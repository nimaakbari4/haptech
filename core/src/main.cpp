#include "simulation_manager.h"
#include "franka_trial_server.h"
#include <thread>
#include <chrono>
#include <iostream>

int main() {
    SimulationManager::setDebugMode(true);

    // Launch test thread (so that simulation is on main thread)
    std::thread serverThread(runServer);

    // Run simulation on main thread
    SimulationManager::run();

    // Join test thread after simulation ends
    serverThread.join();

    return 0;
}
