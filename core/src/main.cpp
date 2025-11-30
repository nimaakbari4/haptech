#include "simulation_manager.h"
#include "franka_trial_server.h"
#include "extensions_registry.h"
#include <thread>
#include <chrono>
#include <iostream>

#define C_ENABLE_FRANKA_DEVICE_SUPPORT

int main() {
    SimulationManager::setDebugMode(true);
    
    // Register all extensions (custom objects, trial conditions, etc.)
    ExtensionsRegistry::registerAllExtensions();

    // Ensure custom objects and conditions are registered before server handles requests
    ExtensionsRegistry::registerAllExtensions();

    // Launch test thread (so that simulation is on main thread)
    std::thread serverThread(runServer);

    // Run simulation on main thread
    SimulationManager::run();

    // Join test thread after simulation ends
    serverThread.join();

    return 0;
}
