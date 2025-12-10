# Implementation Guide

Step-by-step guide for creating your own custom trial conditions.

## Overview

This guide shows you how to create custom trial conditions from scratch. Don't worry if you're new to C++ programming - we'll explain the key concepts as we go.

### What You'll Learn

**Trial Conditions**: Automated rules that determine when your experiment should start or end
**Interface Implementation**: Following a standard template to ensure your condition works with the system
**Parameter Handling**: Reading configuration values from XML files
**Registration**: Making your condition available for use

### When to Create Custom Conditions

You might need custom conditions for:
- **Complex task completion**: "End when participant completes 5 successful movements"
- **Behavioral patterns**: "End when participant shows specific movement patterns"
- **Multi-modal conditions**: "End when both position AND force criteria are met"

## Step 1: Plan Your Condition

Before coding, clearly define:

1. **What triggers the condition?** (time, position, force, custom logic)
2. **What parameters does it need?** (thresholds, targets, durations)
3. **When should it represent success vs. failure?**
4. **What information should it provide when triggered?**

**Example**: "Force Threshold Condition"
- Triggers when haptic device force exceeds a threshold
- Needs parameters: `max_force`, `duration` (how long to exceed)
- Success = staying under threshold, Failure = exceeding threshold
- Message = actual force value when threshold was exceeded

## Step 2: Create Header File

Create a new header file in `core/src/trial_conditions/` (e.g., `force_condition.h`):

```cpp
#ifndef FORCE_CONDITION_H
#define FORCE_CONDITION_H

#include "trial_condition.h"

/**
 * ForceCondition - Monitors haptic device forces
 * Ends trial when force exceeds a threshold for a specified duration
 */
class ForceCondition : public TrialConditionInterface {
private:
    double _maxForce;           // Maximum allowed force (Newtons)
    double _duration;           // How long force must be exceeded (seconds)
    double _exceedStartTime;    // When force first exceeded threshold
    bool _isExceeding;          // Currently exceeding threshold?
    double _peakForce;          // Highest force reached

public:
    // Constructor - called when condition is created from XML
    ForceCondition(std::map<std::string, std::string>& params,
                   bool success, int priority);
    virtual ~ForceCondition() = default;
    
    // Required interface methods
    virtual bool evaluate() override;        // Check if condition is met
    virtual std::string getMessage() override; // Describe what happened
    virtual std::string getName() override;   // Return condition name

private:
    // Helper methods
    double getCurrentForce();                // Get current haptic force
    void resetExceedanceTracking();         // Reset tracking variables
};

#endif // FORCE_CONDITION_H
```

**Key concepts explained**:
- **Inheritance**: `ForceCondition : public TrialConditionInterface` means our condition follows the standard template
- **Private variables**: Store the condition's internal state
- **Virtual methods**: These MUST be implemented to fulfill the interface contract
- **Helper methods**: Private functions to organize your code

## Step 3: Implement Source File

Create the corresponding source file `force_condition.cpp`:

```cpp
#include "force_condition.h"
#include "../simulation_manager.h"
#include <iostream>

ForceCondition::ForceCondition(std::map<std::string, std::string>& params,
                               bool success, int priority)
    : TrialConditionInterface(params, success, priority),
      _maxForce(10.0),          // Default: 10 Newtons
      _duration(0.5),           // Default: 0.5 seconds
      _exceedStartTime(0.0),
      _isExceeding(false),
      _peakForce(0.0) {
    
    // Read max_force parameter from XML
    auto it = params.find("max_force");
    if (it != params.end()) {
        try {
            _maxForce = std::stod(it->second);
            if (_maxForce < 0.0) {
                std::cerr << "[ForceCondition] max_force must be positive, using default" << std::endl;
                _maxForce = 10.0;
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "[ForceCondition] Invalid max_force parameter: " << it->second << std::endl;
        }
    }
    
    // Read duration parameter from XML
    it = params.find("duration");
    if (it != params.end()) {
        try {
            _duration = std::stod(it->second);
            if (_duration < 0.0) {
                std::cerr << "[ForceCondition] duration must be positive, using default" << std::endl;
                _duration = 0.5;
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "[ForceCondition] Invalid duration parameter: " << it->second << std::endl;
        }
    }
    
    std::cout << "[ForceCondition] Created with max_force=" << _maxForce 
              << ", duration=" << _duration << std::endl;
}

bool ForceCondition::evaluate() {
    double currentForce = getCurrentForce();
    double currentTime = SimulationManager::trialContext.time;
    
    // Track the peak force reached
    if (currentForce > _peakForce) {
        _peakForce = currentForce;
    }
    
    if (currentForce > _maxForce) {
        if (!_isExceeding) {
            // Force just exceeded threshold - start timing
            _exceedStartTime = currentTime;
            _isExceeding = true;
        } else {
            // Force has been exceeding threshold - check duration
            double exceedDuration = currentTime - _exceedStartTime;
            if (exceedDuration >= _duration) {
                return true;  // Condition met - end trial
            }
        }
    } else {
        // Force is below threshold - reset tracking
        if (_isExceeding) {
            resetExceedanceTracking();
        }
    }
    
    return false;  // Condition not met - continue trial
}

std::string ForceCondition::getMessage() {
    double currentForce = getCurrentForce();
    return "Force condition triggered: " + std::to_string(currentForce) + 
           "N exceeded maximum " + std::to_string(_maxForce) + 
           "N for " + std::to_string(_duration) + " seconds. Peak force: " + 
           std::to_string(_peakForce) + "N";
}

std::string ForceCondition::getName() {
    return "ForceCondition";
}

double ForceCondition::getCurrentForce() {
    // Get the haptic device's current force
    cVector3d force;
    SimulationManager::simContext.hapticDevice->getForce(force);
    return force.length();  // Return magnitude of force vector
}

void ForceCondition::resetExceedanceTracking() {
    _isExceeding = false;
    _exceedStartTime = 0.0;
}
```

**What this code does**:
1. **Constructor**: Reads parameters from XML and validates them
2. **evaluate()**: Checks current force, tracks when threshold is exceeded, returns true when duration is reached
3. **getMessage()**: Provides detailed information about what happened
4. **Helper methods**: Keep the main logic clean and organized

## Step 4: Register Your Condition

Add your condition to the extensions registry. In `extensions_registry.cpp`, add:

```cpp
// Add include at the top
#include "trial_conditions/force_condition.h"

// In ExtensionsRegistry::registerAllExtensions()
TrialConditionRegistry::registerCondition("force", 
    [](std::map<std::string, std::string>& params, bool success, int priority) 
    -> std::unique_ptr<TrialConditionInterface> {
        return std::make_unique<ForceCondition>(params, success, priority);
    });
```

**What this does**: Makes your condition available with the name "force" in XML files.

## Step 5: Update Build System

Add your source file to the Makefile in the `core/` directory:

```makefile
# Add to the SOURCES variable
SOURCES += src/trial_conditions/force_condition.cpp
```

## Step 6: Build and Test

1. **Build the core**:
   ```bash
   cd core
   make clean
   make
   ```

2. **Create test XML configuration**:
   ```xml
   <TrialConfig>
     <StartConditions>
       <Condition name="time" success="true" priority="1">
         <Parameters duration_seconds="0.0"/>
       </Condition>
     </StartConditions>
     
     <EndConditions>
       <!-- Success: complete task without excessive force -->
       <Condition name="time" success="true" priority="2">
         <Parameters duration_seconds="30.0"/>
       </Condition>
       
       <!-- Failure: apply too much force -->
       <Condition name="force" success="false" priority="1">
         <Parameters max_force="15.0" duration="2.0"/>
       </Condition>
     </EndConditions>
     
     <!-- Rest of your trial configuration... -->
   </TrialConfig>
   ```

3. **Test your condition**:
   ```bash
   # Start the server
   cd core/bin && ./core
   
   # In another terminal, run a trial
   source .venv/bin/activate
   python3 python_client/main.py config/my_force_test.xml test_subject test_trial
   ```

## Advanced Examples

### Multi-Parameter Condition

```cpp
class ComboCondition : public TrialConditionInterface {
private:
    double _targetX, _targetY, _targetZ;     // Target position
    double _tolerance;                       // Position tolerance
    double _minHoldTime;                     // How long to hold position
    double _maxForce;                        // Maximum allowed force
    
    bool _inTarget;                          // Currently at target?
    double _targetEntryTime;                 // When target was reached
    
public:
    bool evaluate() override {
        cVector3d currentPos = SimulationManager::simContext.tool->getDeviceLocalPos();
        double currentForce = getCurrentForce();
        double currentTime = SimulationManager::trialContext.time;
        
        // Check if force is too high (safety condition)
        if (currentForce > _maxForce) {
            return true;  // End trial due to excessive force
        }
        
        // Check if at target position
        cVector3d target(_targetX, _targetY, _targetZ);
        double distance = (currentPos - target).length();
        
        if (distance <= _tolerance) {
            if (!_inTarget) {
                _inTarget = true;
                _targetEntryTime = currentTime;
            } else {
                // Check if held position long enough
                double holdTime = currentTime - _targetEntryTime;
                if (holdTime >= _minHoldTime) {
                    return true;  // Success!
                }
            }
        } else {
            _inTarget = false;  // Left target area
        }
        
        return false;  // Keep going
    }
};
```

## Common Patterns

### State Tracking
```cpp
// Track whether something has happened
bool _eventOccurred = false;

// Track timing
double _eventStartTime = 0.0;
double _lastEventTime = 0.0;

// Track counts
int _successCount = 0;
int _requiredSuccesses = 5;
```

### Parameter Validation
```cpp
double getValidatedParam(const std::string& name, double defaultValue, 
                        double minValue, double maxValue) {
    auto it = params.find(name);
    if (it != params.end()) {
        try {
            double value = std::stod(it->second);
            if (value >= minValue && value <= maxValue) {
                return value;
            } else {
                std::cerr << "Parameter " << name << "=" << value 
                         << " outside valid range [" << minValue << ", " 
                         << maxValue << "], using default" << std::endl;
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid " << name << " parameter: " << it->second << std::endl;
        }
    }
    return defaultValue;
}
```

## Best Practices

1. **Start simple**: Begin with basic functionality, add complexity gradually
2. **Validate parameters**: Check for missing, invalid, or out-of-range values
3. **Provide clear messages**: Help researchers understand what happened
4. **Handle edge cases**: What happens with extreme parameters or unusual conditions?
5. **Test thoroughly**: Try different parameter combinations and scenarios
6. **Document your condition**: Explain what it does, what parameters it needs, and how to use it
7. **Consider performance**: `evaluate()` is called frequently, keep it efficient

With these tools, you can create sophisticated trial conditions that precisely control your haptic experiments and collect the data you need for your research.