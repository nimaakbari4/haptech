#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Include the headers we need for testing
#include "trial_conditions/trial_condition.h"
#include "trial_conditions/time_condition.h"
#include "simulation_manager.h"

// Test fixture for TimeCondition testing
class TimeConditionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset the actual SimulationManager state before each test
        SimulationManager::trialContext.time = 0.0;
        SimulationManager::trialContext.startTime = 0.0;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::trialContext.trialId = "";
    }

    void TearDown() override {
        // Clean up after each test
        SimulationManager::trialContext.time = 0.0;
        SimulationManager::trialContext.startTime = 0.0;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::trialContext.trialId = "";
    }

    // Helper method to set simulation time directly (since TimeCondition accesses static members)
    void setSimulationTime(double currentTime, double startTime) {
        SimulationManager::trialContext.time = currentTime;
        SimulationManager::trialContext.startTime = startTime;
    }
};

// Tests using direct manipulation of static members
TEST_F(TimeConditionTest, CreatesWithValidDuration) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "5.0";
    
    TimeCondition condition(params, true, 1);
    
    EXPECT_EQ(condition.getName(), "TimeCondition");
}

TEST_F(TimeConditionTest, EvaluatesFalseWhenTimeNotElapsed) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "10.0";
    
    TimeCondition condition(params, true, 1);
    
    // Use our helper to set simulation time
    setSimulationTime(5.0, 0.0);  // 5 seconds elapsed, need 10
    
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(TimeConditionTest, EvaluatesTrueWhenTimeElapsed) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "10.0";
    
    TimeCondition condition(params, true, 1);
    
    // Set simulation time so exactly 10 seconds have elapsed
    setSimulationTime(10.0, 0.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(TimeConditionTest, EvaluatesTrueWhenTimeExceeded) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "5.0";
    
    TimeCondition condition(params, true, 1);
    
    // Set simulation time so 6.5 seconds have elapsed (more than 5.0 needed)
    setSimulationTime(8.5, 2.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(TimeConditionTest, HandlesZeroDuration) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "0.0";
    
    TimeCondition condition(params, true, 1);
    
    // Even with no time elapsed, should be true for zero duration
    setSimulationTime(0.0, 0.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(TimeConditionTest, HandlesInvalidDuration) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "invalid_number";
    
    TimeCondition condition(params, true, 1);
    
    // Should default to 0.0 duration and be immediately satisfied
    setSimulationTime(1.0, 0.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(TimeConditionTest, GeneratesCorrectMessage) {
    std::map<std::string, std::string> params;
    params["duration_seconds"] = "10.0";
    
    TimeCondition condition(params, true, 1);
    
    // Set time to 7.5 seconds elapsed
    setSimulationTime(7.5, 0.0);
    
    std::string message = condition.getMessage();
    
    // Verify the message contains the expected time values
    EXPECT_NE(message.find("7.5"), std::string::npos);
    EXPECT_NE(message.find("10"), std::string::npos);
    EXPECT_NE(message.find("Time condition reached"), std::string::npos);
}