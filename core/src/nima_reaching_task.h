#pragma once
#include "trial_conditions/trial_condition.h"
#include <string>
#include <vector>
class NimaReachingTask {
public:
 static void configure(const std::map<std::string,std::string>& params);
 static bool evaluate();
 static void cancel();
 static void clear();
 static bool active();
 static bool successful();
 static bool returning();
 static bool shouldRecord();
 static std::string text();
 static std::vector<std::string> takeEvents();
 static std::string summary();
 static void applyHaptics(double dt);
  static bool readData(chai3d::cVector3d& position,
                      chai3d::cVector3d& velocity,
                      chai3d::cVector3d& force,
                      chai3d::cVector3d* external_force = nullptr, chai3d::cVector3d* angularVelocity = nullptr);
};
class NimaReachingCondition : public TrialConditionInterface {
public:
 NimaReachingCondition(std::map<std::string,std::string>& p,bool ok,int priority):TrialConditionInterface(p,ok,priority){NimaReachingTask::configure(p);}
 bool evaluate() override {bool done=NimaReachingTask::evaluate();if(done)success=NimaReachingTask::successful();return done;}
 std::string getMessage() override {return NimaReachingTask::summary();}
 std::string getName() override {return "nima_reaching";}
};
