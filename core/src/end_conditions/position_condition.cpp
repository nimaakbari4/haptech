#include "position_condition.h"
#include "../simulation_manager.h"
#include <iostream>
#include <cmath>

PositionCondition::PositionCondition(std::map<std::string, std::string>& params,
                                   bool success, int priority)
    : TrialEndCondition(params, success, priority), _targetX(0), _targetY(0), _targetZ(0), _tolerance(0.05) {
    
    // Parse target position
    auto x_it = params.find("target_x");
    if (x_it != params.end()) {
        try {
            _targetX = std::stod(x_it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[PositionCondition] Invalid target_x parameter: " << x_it->second << std::endl;
        }
    }
    
    auto y_it = params.find("target_y");
    if (y_it != params.end()) {
        try {
            _targetY = std::stod(y_it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[PositionCondition] Invalid target_y parameter: " << y_it->second << std::endl;
        }
    }
    
    auto z_it = params.find("target_z");
    if (z_it != params.end()) {
        try {
            _targetZ = std::stod(z_it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[PositionCondition] Invalid target_z parameter: " << z_it->second << std::endl;
        }
    }
    
    // Parse tolerance
    auto tol_it = params.find("tolerance");
    if (tol_it != params.end()) {
        try {
            _tolerance = std::stod(tol_it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[PositionCondition] Invalid tolerance parameter: " << tol_it->second << std::endl;
        }
    }
}

bool PositionCondition::evaluate() {
    // Get current device position from simulation manager
    cVector3d devicePos = SimulationManager::simContext.tool->getDeviceGlobalPos();
    
    // Calculate distance to target
    double dx = devicePos.x() - _targetX;
    double dy = devicePos.y() - _targetY;
    double dz = devicePos.z() - _targetZ;
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    return distance <= _tolerance;
}

std::string PositionCondition::getMessage() {
    cVector3d devicePos = SimulationManager::simContext.tool->getDeviceGlobalPos();
    double dx = devicePos.x() - _targetX;
    double dy = devicePos.y() - _targetY;
    double dz = devicePos.z() - _targetZ;
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    return "Target position reached: distance=" + std::to_string(distance) + 
           " (tolerance=" + std::to_string(_tolerance) + ")";
}

std::string PositionCondition::getName() {
    return "PositionCondition";
}
