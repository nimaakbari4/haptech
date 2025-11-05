#ifndef VELOCITY_CONDITION_H
#define VELOCITY_CONDITION_H

#include "trial_condition.h"

/**
 * VelocityCondition - A velocity-based trial end condition
 * Terminates the trial when the haptic device exceeds a maximum velocity
 * Uses SimulationManager::simContext.tool for device velocity data
 */
class VelocityCondition : public TrialConditionInterface {
private:
    double _maxVelocity;
    cVector3d _lastPosition;
    double _lastTime;
    bool _initialized;
    double _maxVelocityReached = 0.0;

public:
    VelocityCondition(std::map<std::string, std::string>& params,
                     bool success, int priority);
    virtual ~VelocityCondition() = default;
    
    virtual bool evaluate() override;
    virtual std::string getMessage() override;
    virtual std::string getName() override;
};

#endif
