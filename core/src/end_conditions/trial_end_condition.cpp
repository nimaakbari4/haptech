#include "trial_end_condition.h"
#include "../simulation_manager.h"
#include <iostream>

// Define the static member variable
std::map<std::string, EndConditionRegistry::ConditionCreator> EndConditionRegistry::creators_;


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