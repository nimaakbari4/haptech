#include "time_condition.h"
#include "../simulation_manager.h"
#include <iostream>

TimeCondition::TimeCondition(std::map<std::string, std::string>& params,
                            bool success, int priority)
    : TrialConditionInterface(params, success, priority), _durationSeconds(0) {
    auto it = params.find("duration_seconds");
    if (it != params.end()) {
        try {
            _durationSeconds = std::stod(it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[TimeCondition] Invalid duration_seconds parameter: " << it->second << std::endl;
            _durationSeconds = 0;
        }
    } else {
        std::cerr << "[TimeCondition] Missing duration_seconds parameter" << std::endl;
    }
}

bool TimeCondition::evaluate() {    
    // Use the trial context time instead of wall clock time
    double elapsedTime = SimulationManager::trialContext.time - SimulationManager::trialContext.startTime;
    return elapsedTime >= _durationSeconds;
}

std::string TimeCondition::getMessage() {
    double elapsedTime = SimulationManager::trialContext.time - SimulationManager::trialContext.startTime;
    return "Time condition reached: " + std::to_string(elapsedTime) + "/" + std::to_string(_durationSeconds) + " seconds";
}

std::string TimeCondition::getName() {
    return "TimeCondition";
}
