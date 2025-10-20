#ifndef TRIAL_END_CONDITION_H
#define TRIAL_END_CONDITION_H

#include "chai3d.h"
#include <map>

using namespace chai3d;

// Forward declaration to avoid circular include
class SimulationManager;

class TrialEndCondition {
public:
    std::map<std::string, std::string> params;
    bool success;
    int priority;

    TrialEndCondition(std::map<std::string, std::string>& params,
                    bool success, int priority)
        : params(params), success(success), priority(priority) {}
    virtual ~TrialEndCondition() = default;
    
    /**
     * Evaluate the condition
     */
    virtual bool evaluate() = 0;

    /**
     * Get the message to stream back upon satisfaction of this condition
    */
    virtual std::string getMessage() = 0;

    /**
     * Get the name of this condition
    */
    virtual std::string getName() = 0;
};


class EndConditionRegistry {
public:
    using ConditionCreator = std::function<std::unique_ptr<TrialEndCondition>(
        std::map<std::string, std::string>&, bool, int)>;
    
    /**
     * Register a condition
     */
    static void registerCondition(const std::string& name, ConditionCreator creator);
    
    /**
     * Create an condition by type name
     */
    static std::unique_ptr<TrialEndCondition> createCondition(const std::string& name, 
                                                std::map<std::string, std::string>& params,
                                                bool success, int priority);
    
    /**
     * Check if a condition is registered
     */
    static bool isConditionRegistered(const std::string& name);

private:
    static std::map<std::string, ConditionCreator> creators_;
};

#endif