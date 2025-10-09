#include "trial_end_condition.h"
#include "../simulation_manager.h"
#include <iostream>

// Define the static member variable
std::map<std::string, EndConditionRegistry::ConditionCreator> EndConditionRegistry::creators_;

// TimeCondition implementation
TimeCondition::TimeCondition(std::map<std::string, std::string>& params,
                            bool success, int priority)
    : TrialEndCondition(params, success, priority), _durationSeconds(0) {
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
    double elapsedTime = SimulationManager::trialContext.time;
    return elapsedTime >= _durationSeconds;
}

std::string TimeCondition::getMessage() {
    double elapsedTime = SimulationManager::trialContext.time;
    return "Time limit reached: " + std::to_string(elapsedTime) + "/" + std::to_string(_durationSeconds) + " seconds";
}

std::string TimeCondition::getName() {
    return "TimeCondition";
}

void EndConditionRegistry::registerCondition(const std::string& name, ConditionCreator creator) {
    creators_[name] = creator;
    std::cout << "[EndConditionRegistry] Registered end condition: " << name << std::endl;    
}

std::unique_ptr<TrialEndCondition> EndConditionRegistry::createCondition(
                            const std::string& name, std::map<std::string, std::string>& params,
                            bool success, int priority) {
    auto it = creators_.find(name);
    if (it != creators_.end()) {
        return it->second(params, success, priority);
    }
    
    std::cout << "[EndConditionRegistry] Unknown condition name: " << name << std::endl;
    return nullptr;
}

bool EndConditionRegistry::isConditionRegistered(const std::string& name) {
    return creators_.find(name) != creators_.end();
}