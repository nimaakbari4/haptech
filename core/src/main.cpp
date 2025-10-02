#include "simulation_manager.h"
#include "franka_trial_server.h"
#include "researcher_objects_registry.h"
#include <thread>
#include <chrono>
#include <iostream>

#define C_ENABLE_FRANKA_DEVICE_SUPPORT

int main() {
    SimulationManager::setDebugMode(false);
    
    // Register researcher custom object types
    ResearcherObjectsRegistry::registerAllObjects();

    // Launch test thread (so that simulation is on main thread)
    std::thread serverThread(runServer);

    // Run simulation on main thread
    SimulationManager::run();

    // Join test thread after simulation ends
    serverThread.join();

    return 0;
}
