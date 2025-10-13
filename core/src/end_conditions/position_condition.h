#ifndef POSITION_CONDITION_H
#define POSITION_CONDITION_H

#include "trial_end_condition.h"

/**
 * PositionCondition - A position-based trial end condition
 * Terminates the trial when the haptic device reaches a target position
 * Uses SimulationManager::simContext.tool for device position data
 */
class PositionCondition : public TrialEndCondition {
private:
    double _targetX, _targetY, _targetZ;
    double _tolerance;

public:
    PositionCondition(std::map<std::string, std::string>& params,
                     bool success, int priority);
    virtual ~PositionCondition() = default;
    
    virtual bool evaluate() override;
    virtual std::string getMessage() override;
    virtual std::string getName() override;
};

#endif
