#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Include the headers we need for testing
#include "trial_conditions/trial_condition.h"
#include "trial_conditions/time_condition.h"
#include "trial_conditions/position_condition.h"
#include "trial_conditions/velocity_condition.h"
#include "simulation_manager.h"
#include <gmock/gmock.h>
#include <chai3d.h>

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

// Test fixture for PositionCondition testing
class PositionConditionTest : public ::testing::Test {
protected:
    cGenericTool* mockTool = nullptr;

    void SetUp() override {
        // Create a mock tool for testing
        mockTool = new cGenericTool(nullptr);
        SimulationManager::simContext.tool = mockTool;
        
        // Reset trial context
        SimulationManager::trialContext.time = 0.0;
        SimulationManager::trialContext.startTime = 0.0;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::trialContext.trialId = "";
    }

    void TearDown() override {
        // Clean up
        SimulationManager::simContext.tool = nullptr;
        if (mockTool) {
            delete mockTool;
            mockTool = nullptr;
        }
        
        SimulationManager::trialContext.time = 0.0;
        SimulationManager::trialContext.startTime = 0.0;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::trialContext.trialId = "";
    }

    // Helper method to set the mock tool position
    void setToolPosition(double x, double y, double z) {
        if (mockTool) {
            cVector3d pos(x, y, z);
            mockTool->setDeviceGlobalPos(pos);
        }
    }
};

// Test fixture for VelocityCondition testing  
class VelocityConditionTest : public ::testing::Test {
protected:
    cGenericTool* mockTool = nullptr;

    void SetUp() override {
        // Create a mock tool for testing
        mockTool = new cGenericTool(nullptr);
        SimulationManager::simContext.tool = mockTool;
        
        // Reset trial context
        SimulationManager::trialContext.time = 0.0;
        SimulationManager::trialContext.startTime = 0.0;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::trialContext.trialId = "";
    }

    void TearDown() override {
        // Clean up
        SimulationManager::simContext.tool = nullptr;
        if (mockTool) {
            delete mockTool;
            mockTool = nullptr;
        }
        
        SimulationManager::trialContext.time = 0.0;
        SimulationManager::trialContext.startTime = 0.0;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::trialContext.trialId = "";
    }

    // Helper method to set the mock tool position and time
    void setToolPositionAndTime(double x, double y, double z, double time) {
        if (mockTool) {
            cVector3d pos(x, y, z);
            mockTool->setDeviceGlobalPos(pos);
        }
        SimulationManager::trialContext.time = time;
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

// PositionCondition Tests
TEST_F(PositionConditionTest, CreatesWithValidParameters) {
    std::map<std::string, std::string> params;
    params["target_x"] = "1.0";
    params["target_y"] = "2.0";
    params["target_z"] = "3.0";
    params["tolerance"] = "0.1";
    
    PositionCondition condition(params, true, 1);
    
    EXPECT_EQ(condition.getName(), "PositionCondition");
}

TEST_F(PositionConditionTest, EvaluatesFalseWhenToolNotInitialized) {
    std::map<std::string, std::string> params;
    params["target_x"] = "1.0";
    
    PositionCondition condition(params, true, 1);
    
    // Set tool to null to simulate uninitialized state
    SimulationManager::simContext.tool = nullptr;
    
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(PositionConditionTest, EvaluatesFalseWhenFarFromTarget) {
    std::map<std::string, std::string> params;
    params["target_x"] = "1.0";
    params["target_y"] = "2.0";
    params["target_z"] = "3.0";
    params["tolerance"] = "0.1";
    
    PositionCondition condition(params, true, 1);
    
    // Set tool position far from target
    setToolPosition(0.0, 0.0, 0.0);
    
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(PositionConditionTest, EvaluatesTrueWhenWithinTolerance) {
    std::map<std::string, std::string> params;
    params["target_x"] = "1.0";
    params["target_y"] = "2.0";
    params["target_z"] = "3.0";
    params["tolerance"] = "0.1";
    
    PositionCondition condition(params, true, 1);
    
    // Set tool position within tolerance (0.05 away from target)
    setToolPosition(1.03, 2.04, 3.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(PositionConditionTest, EvaluatesTrueWhenExactlyAtTarget) {
    std::map<std::string, std::string> params;
    params["target_x"] = "5.0";
    params["target_y"] = "-2.0";
    params["target_z"] = "1.5";
    params["tolerance"] = "0.01";
    
    PositionCondition condition(params, true, 1);
    
    // Set tool position exactly at target
    setToolPosition(5.0, -2.0, 1.5);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(PositionConditionTest, HandlesDefaultValues) {
    std::map<std::string, std::string> params;
    // No parameters provided - should use defaults (0,0,0) with tolerance 0.05
    
    PositionCondition condition(params, true, 1);
    
    // Set position at origin
    setToolPosition(0.0, 0.0, 0.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(PositionConditionTest, HandlesInvalidParameters) {
    std::map<std::string, std::string> params;
    params["target_x"] = "invalid_number";
    params["target_y"] = "invalid_also";
    params["target_z"] = "nope";
    params["tolerance"] = "not_a_number";
    
    PositionCondition condition(params, true, 1);
    
    // Should use default values - target (0,0,0) with tolerance 0.05
    // Set position at (0.02, 0.0, 0.0) - distance = 0.02 which should be < 0.05 tolerance
    setToolPosition(0.02, 0.0, 0.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(PositionConditionTest, GeneratesCorrectMessage) {
    std::map<std::string, std::string> params;
    params["target_x"] = "1.0";
    params["target_y"] = "2.0";
    params["target_z"] = "3.0";
    params["tolerance"] = "0.1";
    
    PositionCondition condition(params, true, 1);
    
    setToolPosition(1.05, 2.0, 3.0);
    
    std::string message = condition.getMessage();
    
    // Verify the message contains distance and tolerance info
    EXPECT_NE(message.find("distance="), std::string::npos);
    EXPECT_NE(message.find("tolerance="), std::string::npos);
    EXPECT_NE(message.find("0.1"), std::string::npos);
}

TEST_F(PositionConditionTest, GeneratesMessageWhenToolNotInitialized) {
    std::map<std::string, std::string> params;
    params["target_x"] = "1.0";
    
    PositionCondition condition(params, true, 1);
    
    // Set tool to null
    SimulationManager::simContext.tool = nullptr;
    
    std::string message = condition.getMessage();
    
    EXPECT_NE(message.find("Tool not initialized"), std::string::npos);
}

// VelocityCondition Tests
TEST_F(VelocityConditionTest, CreatesWithValidParameters) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    EXPECT_EQ(condition.getName(), "VelocityCondition");
}

TEST_F(VelocityConditionTest, EvaluatesFalseWhenToolNotInitialized) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    // Set tool to null to simulate uninitialized state
    SimulationManager::simContext.tool = nullptr;
    
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(VelocityConditionTest, EvaluatesFalseOnFirstCall) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    
    // Should return false on first evaluation (no previous position to compare)
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(VelocityConditionTest, EvaluatesFalseWhenVelocityBelowLimit) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "1.0";  // 1 m/s limit
    
    VelocityCondition condition(params, true, 1);
    
    // First call to initialize
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    
    // Second call with slow movement (0.1 m in 1 second = 0.1 m/s)
    setToolPositionAndTime(0.1, 0.0, 0.0, 2.0);
    
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(VelocityConditionTest, EvaluatesTrueWhenVelocityExceedsLimit) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";  // 0.5 m/s limit
    
    VelocityCondition condition(params, true, 1);
    
    // First call to initialize
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    
    // Second call with fast movement (1.0 m in 1 second = 1.0 m/s > 0.5 m/s limit)
    setToolPositionAndTime(1.0, 0.0, 0.0, 2.0);
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(VelocityConditionTest, EvaluatesFalseWhenTimeDeltaZero) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    // First call to initialize
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    
    // Second call with same time (dt = 0)
    setToolPositionAndTime(1.0, 0.0, 0.0, 1.0);  // Same time
    
    EXPECT_FALSE(condition.evaluate());
}

TEST_F(VelocityConditionTest, HandlesDefaultMaxVelocity) {
    std::map<std::string, std::string> params;
    // No max_velocity parameter - should use default 0.1
    
    VelocityCondition condition(params, true, 1);
    
    // First call to initialize
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    
    // Second call with movement that exceeds default 0.1 m/s
    setToolPositionAndTime(0.2, 0.0, 0.0, 2.0);  // 0.2 m/s > 0.1 m/s default
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(VelocityConditionTest, HandlesInvalidMaxVelocityParameter) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "not_a_number";
    
    VelocityCondition condition(params, true, 1);
    
    // Should still work with default value
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    
    setToolPositionAndTime(0.2, 0.0, 0.0, 2.0);  // Should exceed default 0.1 m/s
    
    EXPECT_TRUE(condition.evaluate());
}

TEST_F(VelocityConditionTest, GeneratesCorrectMessage) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    // Initialize and trigger velocity limit
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    setToolPositionAndTime(1.0, 0.0, 0.0, 2.0);
    condition.evaluate();
    
    std::string message = condition.getMessage();
    
    // Verify the message contains velocity limit info
    EXPECT_NE(message.find("Velocity limit exceeded"), std::string::npos);
    EXPECT_NE(message.find("0.5"), std::string::npos);
}

TEST_F(VelocityConditionTest, GeneratesMessageWhenToolNotInitialized) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    // Set tool to null
    SimulationManager::simContext.tool = nullptr;
    
    std::string message = condition.getMessage();
    
    EXPECT_NE(message.find("Tool not initialized"), std::string::npos);
}

TEST_F(VelocityConditionTest, Calculates3DVelocityCorrectly) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.9";  // Set limit lower to ensure we exceed it
    
    VelocityCondition condition(params, true, 1);
    
    // Initialize at origin
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    condition.evaluate();
    
    // Move diagonally: sqrt(0.6^2 + 0.8^2) = sqrt(0.36 + 0.64) = sqrt(1.0) = 1.0 m in 1 second = 1.0 m/s
    // This is > 0.9 m/s limit
    setToolPositionAndTime(0.6, 0.8, 0.0, 2.0);
    
    EXPECT_TRUE(condition.evaluate());  // Should exceed the 0.9 m/s limit
}

TEST_F(VelocityConditionTest, GeneratesMessageWhenNotInitialized) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    // Call getMessage before evaluate() is called (condition not initialized)
    std::string message = condition.getMessage();
    
    // Should return the uninitialized message path
    EXPECT_NE(message.find("Velocity limit exceeded: max=0.5"), std::string::npos);
}

TEST_F(VelocityConditionTest, GeneratesMessageWithVelocityCalculation) {
    std::map<std::string, std::string> params;
    params["max_velocity"] = "0.5";
    
    VelocityCondition condition(params, true, 1);
    
    // Initialize the condition with a starting position
    setToolPositionAndTime(0.0, 0.0, 0.0, 1.0);
    bool result1 = condition.evaluate();
    EXPECT_FALSE(result1); // First evaluation should not trigger
    
    // Move rapidly to exceed velocity limit (distance = sqrt(3), time = 1, so velocity = sqrt(3) ≈ 1.73 > 0.5)
    setToolPositionAndTime(1.0, 1.0, 1.0, 2.0);
    bool result2 = condition.evaluate();
    EXPECT_TRUE(result2); // This should exceed the velocity limit and set _maxVelocityReached
    
    // Set SimulationManager to a state where getMessage will do the full calculation
    // The _initialized flag should be true now, and _lastTime should be 2.0, _lastPosition should be (1,1,1)
    // Set current state to some later position and time so dt > 0 and we get to the full calculation
    setToolPositionAndTime(2.0, 2.0, 2.0, 3.0);
    
    // Now call getMessage() - should hit the full velocity calculation path (lines 80-86)
    std::string message = condition.getMessage();
    
    // Verify the message contains the velocity information in "exceeded > limit" format
    EXPECT_NE(message.find("Velocity limit exceeded:"), std::string::npos);
    EXPECT_NE(message.find(">"), std::string::npos);
    EXPECT_NE(message.find("0.5"), std::string::npos);
}