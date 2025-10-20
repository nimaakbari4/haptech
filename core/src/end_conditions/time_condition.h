#ifndef TIME_CONDITION_H
#define TIME_CONDITION_H

#include "trial_end_condition.h"

/**
 * TimeCondition - A simple time-based trial end condition
 * Terminates the trial after a specified duration has elapsed
 * Uses SimulationManager::trialContext.time for timing
 */
class TimeCondition : public TrialEndCondition {
private:
    double _durationSeconds;

public:
    TimeCondition(std::map<std::string, std::string>& params,
                  bool success, int priority);
    virtual ~TimeCondition() = default;
    
    virtual bool evaluate() override;
    virtual std::string getMessage() override;
    virtual std::string getName() override;
};

#endif
