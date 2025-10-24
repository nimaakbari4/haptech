#include "trial_condition.h"
#include "../simulation_manager.h"
#include <iostream>

// Define the static member variable
std::map<std::string, TrialConditionRegistry::ConditionCreator> TrialConditionRegistry::creators_;

void TrialConditionRegistry::registerCondition(const std::string& name, ConditionCreator creator) {
    creators_[name] = creator;
    std::cout << "[TrialConditionRegistry] Registered trial condition: " << name << std::endl;    
}

std::unique_ptr<TrialConditionInterface> TrialConditionRegistry::createCondition(
                            const std::string& name, std::map<std::string, std::string>& params,
                            bool success, int priority) {
    auto it = creators_.find(name);
    if (it != creators_.end()) {
        return it->second(params, success, priority);
    }
    
    std::cout << "[TrialConditionRegistry] Unknown condition name: " << name << std::endl;
    return nullptr;
}

bool TrialConditionRegistry::isConditionRegistered(const std::string& name) {
    return creators_.find(name) != creators_.end();
}