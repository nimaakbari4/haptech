"""Hardware-free lifecycle regression tests.

Run with: python3 core/tests/test_shutdown_lifecycle.py

Compile the production main, server coordinator, GUI-exit tail, and cleanup
functions against small substitutes for gRPC, graphics, and the robot device.
This checks ordering and startup races; it does not validate hardware or the
full gRPC/graphics build.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def block(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


HEADER = r"""
#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <cassert>
struct cVector3d {
    double x=0;
    cVector3d(double a=0, double=0, double=0):x(a){}
    double length() const { return x; }
    std::string str(int) const { return std::to_string(x); }
};
inline cVector3d operator*(double a, const cVector3d& v) { return {a*v.x}; }
void cSleepMs(int);
struct Object { virtual ~Object()=default; };
struct Device {
    bool close();
    bool setForce(cVector3d);
    void getForce(cVector3d& v) { v={1}; }
};
struct Tool : Object {
    std::shared_ptr<Device> device;
    bool stop() { return device->close(); }
};
struct World {
    std::set<Object*> children;
    void removeChild(Object* p) { children.erase(p); }
    ~World();
};
struct Worker {
    std::thread thread;
    ~Worker() { if(thread.joinable()) thread.join(); }
};
struct Handler {};
struct SimulationContext {
    std::atomic<bool> hapticsEnabled{true}, simulationRunning{false};
    std::atomic<bool> simulationFinished{true}, dataThreadRunning{false};
    Worker* hapticsThread=nullptr;
    Worker* dataThread=nullptr;
    Tool* tool=nullptr;
    World* world=nullptr;
    Handler* handler=nullptr;
    std::shared_ptr<Device> hapticDevice;
    std::map<std::string,std::unique_ptr<Object>> objectMap;
    std::map<std::string,std::unique_ptr<Object>> objectInterfaceMap;
    std::mutex objectMapMutex;
    void* window=nullptr;
};
class SimulationManager {
public:
    static SimulationContext simContext;
    static void setDebugMode(bool) {}
    static bool run();
    static void close();
    static void rampDeviceForceToZero(int);
    static void destroyHapticTool();
};
class ExtensionsRegistry {
public:
    static void registerAllExtensions() {}
};
void runServer();
void shutdownServer();
void glfwDestroyWindow(void*);
void glfwTerminate();
"""

STUBS = r"""
#include "simulation_manager.h"
SimulationContext SimulationManager::simContext;
std::string mode=std::getenv("SHUTDOWN_CASE") ? std::getenv("SHUTDOWN_CASE") : "idle";
std::atomic<int> deviceCloses{0}, worldDeletes{0}, rpcCompletions{0}, rampWrites{0};
std::atomic<bool> serviceAlive{false}, serverDeleted{false};
std::mutex readyMutex;
std::condition_variable readyCv;
bool serverBuilt=false;
namespace grpc {
struct Server {
    std::mutex mutex;
    std::condition_variable cv;
    bool cancelling=false, drained=false;
    std::thread rpc;
    void startRpc() {
        rpc=std::thread([this] {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock,[this]{return cancelling;});
            assert(SimulationManager::simContext.world != nullptr);
            ++rpcCompletions;
        });
    }
    template<class T> void Shutdown(const T&) {
        { std::lock_guard<std::mutex> lock(mutex); cancelling=true; }
        cv.notify_all();
        if(rpc.joinable()) rpc.join();
        { std::lock_guard<std::mutex> lock(mutex); drained=true; }
        cv.notify_all();
    }
    void Wait() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock,[this]{return drained;});
    }
    ~Server() {
        assert(serviceAlive);
        assert(!rpc.joinable());
        serverDeleted=true;
    }
};
inline int InsecureServerCredentials() { return 0; }
}
using grpc::Server;
std::atomic<Server*> builtServer{nullptr};
struct FrankaTrialServiceImpl {
    FrankaTrialServiceImpl() { serviceAlive=true; }
    ~FrankaTrialServiceImpl() { serviceAlive=false; }
};
struct ServerBuilder {
    void AddListeningPort(const std::string&,int) {}
    void RegisterService(FrankaTrialServiceImpl*) {}
    std::unique_ptr<Server> BuildAndStart() {
        if(mode=="late-server" || mode=="early-failure")
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        auto server=mode=="bind-failure" ? nullptr : std::make_unique<Server>();
        builtServer=server.get();
        { std::lock_guard<std::mutex> lock(readyMutex); serverBuilt=true; }
        readyCv.notify_all();
        return server;
    }
};
void cSleepMs(int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(n>1 ? 1 : n));
}
bool Device::close() {
    assert(SimulationManager::simContext.simulationFinished);
    ++deviceCloses;
    return true;
}
bool Device::setForce(cVector3d) {
    if(deviceCloses) return false;
    assert(SimulationManager::simContext.simulationFinished);
    ++rampWrites;
    return true;
}
World::~World() {
    assert(children.empty());
    assert(deviceCloses==1);
    assert(SimulationManager::simContext.simulationFinished);
    if(mode=="active-rpc") assert(rpcCompletions==1);
    ++worldDeletes;
}
void glfwDestroyWindow(void*) { assert(worldDeletes==1); }
void glfwTerminate() {
    assert(worldDeletes==1);
    if(mode=="repeat") {
        shutdownServer();
        SimulationManager::close();
    }
}
void verify() {
    if(mode=="early-failure") {
        assert(deviceCloses==0 && worldDeletes==0);
    } else {
        assert(deviceCloses==1 && worldDeletes==1);
        assert(rampWrites==21);
    }
    assert(mode=="bind-failure" || serverDeleted);
    std::cout << "SHUTDOWN_VERIFIED" << std::endl;
}
struct Verifier { Verifier() { std::atexit(verify); } } verifier;
"""


class ShutdownTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not shutil.which("g++"):
            raise unittest.SkipTest("g++ is required")
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        path = Path(cls.temp.name)
        main = (ROOT / "core/src/main.cpp").read_text()
        server = (ROOT / "core/src/franka_trial_server.cpp").read_text()
        sim = (ROOT / "core/src/simulation_manager.cpp").read_text()
        (path / "simulation_manager.h").write_text(HEADER)
        for name in ("extensions_registry.h", "franka_trial_server.h"):
            (path / name).write_text('#pragma once\n#include "simulation_manager.h"\n')
        (path / "main.cpp").write_text(main)

        # Execute the actual exit tail, including the call order before GL teardown.
        exit_tail = sim[sim.index("    // Drain requests"):sim.index("\nvoid SimulationManager::setDebugMode")]
        assert exit_tail.rstrip().endswith("}")
        startup = sim[sim.index("    // Publish startup state"):sim.index("    // create a thread which starts")]
        worker_start = block(sim, "void SimulationManager::updateHaptics")
        assert "simContext.simulationRunning = true;" not in worker_start
        run_stub = r"""
bool SimulationManager::run() {
    if(mode=="early-failure") return false;
    auto& ctx=simContext;
    ctx.hapticDevice=std::make_shared<Device>();
    ctx.handler=new Handler();
    ctx.world=new World();
    ctx.objectMap["object"]=std::make_unique<Object>();
    ctx.world->children.insert(ctx.objectMap["object"].get());
    ctx.tool=new Tool();
    ctx.tool->device=ctx.hapticDevice;
    ctx.world->children.insert(ctx.tool);
    std::atexit(close);
"""
        run_stub += startup
        run_stub += r"""
    ctx.hapticsThread=new Worker();
    ctx.hapticsThread->thread=std::thread([] {
        if(mode=="late-worker")
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        while(SimulationManager::simContext.simulationRunning)
            std::this_thread::yield();
        SimulationManager::simContext.simulationFinished=true;
    });
    if(mode!="late-server") {
        std::unique_lock<std::mutex> lock(readyMutex);
        readyCv.wait(lock,[]{return serverBuilt;});
    }
    if(mode=="active-rpc") builtServer.load()->startRpc();
"""
        run_stub += exit_tail
        production = "\n".join([
            block(server, "namespace {"),
            block(server, "void shutdownServer()"),
            block(server, "void runServer()"),
            block(sim, "void SimulationManager::close(void)"),
            block(sim, "void SimulationManager::destroyHapticTool()"),
            block(sim, "void SimulationManager::rampDeviceForceToZero(int durationMs)"),
        ])
        (path / "runtime.cpp").write_text(STUBS + "\n" + production + "\n" + run_stub)
        cls.binary = path / "shutdown-test"
        subprocess.run([
            "g++", "-std=c++17", "-pthread", "-Wall", "-Wextra",
            str(path / "main.cpp"), str(path / "runtime.cpp"),
            "-o", str(cls.binary),
        ], check=True)

    def check_case(self, case, exit_code=0):
        env = dict(os.environ, SHUTDOWN_CASE=case)
        result = subprocess.run(
            [str(self.binary)], env=env, capture_output=True, text=True, timeout=3,
        )
        self.assertEqual(result.returncode, exit_code, result.stdout + result.stderr)
        self.assertIn("SHUTDOWN_VERIFIED", result.stdout)

    def test_idle_gui_close_exits_and_stops_device(self):
        self.check_case("idle")

    def test_active_rpc_finishes_before_scene_is_freed(self):
        self.check_case("active-rpc")

    def test_gui_closes_before_server_finishes_starting(self):
        self.check_case("late-server")

    def test_worker_starts_after_stop_request(self):
        self.check_case("late-worker")

    def test_repeated_shutdown_and_atexit_are_harmless(self):
        self.check_case("repeat")

    def test_graphics_startup_failure_still_joins_server(self):
        self.check_case("early-failure", 1)

    def test_port_bind_failure_does_not_block_shutdown(self):
        self.check_case("bind-failure")


if __name__ == "__main__":
    unittest.main()
