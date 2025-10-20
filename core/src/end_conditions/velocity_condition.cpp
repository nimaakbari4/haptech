#include "velocity_condition.h"
#include "../simulation_manager.h"
#include <iostream>
#include <cmath>

VelocityCondition::VelocityCondition(std::map<std::string, std::string>& params,
                                   bool success, int priority)
    : TrialEndCondition(params, success, priority), _maxVelocity(0.1), _lastPosition(0, 0, 0), _lastTime(0), _initialized(false) {
    
    // Parse max velocity parameter
    auto vel_it = params.find("max_velocity");
    if (vel_it != params.end()) {
        try {
            _maxVelocity = std::stod(vel_it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[VelocityCondition] Invalid max_velocity parameter: " << vel_it->second << std::endl;
        }
    } else {
        std::cerr << "[VelocityCondition] Missing max_velocity parameter" << std::endl;
    }
}

bool VelocityCondition::evaluate() {
    // Get current device position and time
    cVector3d currentPos = SimulationManager::simContext.tool->getDeviceGlobalPos();
    double currentTime = SimulationManager::trialContext.time;
    
    // Initialize on first call
    if (!_initialized) {
        _lastPosition = currentPos;
        _lastTime = currentTime;
        _initialized = true;
        return false; // Can't calculate velocity on first frame
    }
    
    // Calculate time delta
    double dt = currentTime - _lastTime;
    if (dt <= 0) {
        return false; // Avoid division by zero
    }
    
    // Calculate velocity
    double dx = currentPos.x() - _lastPosition.x();
    double dy = currentPos.y() - _lastPosition.y();
    double dz = currentPos.z() - _lastPosition.z();
    double velocity = std::sqrt(dx*dx + dy*dy + dz*dz) / dt;
    
    // Update for next frame
    _lastPosition = currentPos;
    _lastTime = currentTime;
    
    return velocity > _maxVelocity;
}

std::string VelocityCondition::getMessage() {
    cVector3d currentPos = SimulationManager::simContext.tool->getDeviceGlobalPos();
    double currentTime = SimulationManager::trialContext.time;
    
    if (!_initialized) {
        return "Velocity limit exceeded: max=" + std::to_string(_maxVelocity);
    }
    
    double dt = currentTime - _lastTime;
    if (dt <= 0) {
        return "Velocity limit exceeded: max=" + std::to_string(_maxVelocity);
    }
    
    double dx = currentPos.x() - _lastPosition.x();
    double dy = currentPos.y() - _lastPosition.y();
    double dz = currentPos.z() - _lastPosition.z();
    double velocity = std::sqrt(dx*dx + dy*dy + dz*dz) / dt;
    
    return "Velocity limit exceeded: " + std::to_string(velocity) + 
           " > " + std::to_string(_maxVelocity);
}

std::string VelocityCondition::getName() {
    return "VelocityCondition";
}
