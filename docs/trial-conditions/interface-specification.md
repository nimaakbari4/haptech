# TrialConditionInterface Specification

The `TrialConditionInterface` is a template that all trial conditions must follow. This ensures consistent behavior and makes it easy to create new types of conditions.

## What is a Trial Condition?

A trial condition is a piece of code that:
1. **Monitors** something during the experiment (time, position, velocity, etc.)
2. **Decides** when a specific event has occurred ("target reached", "time expired", etc.)
3. **Reports** what happened and whether it represents success or failure

## Interface Overview

Every custom condition must implement these three methods:

```cpp
class TrialConditionInterface {
public:
    // Configuration from XML
    std::map<std::string, std::string> params;  // Parameters from XML <Parameters/>
    bool success;                                // Whether this condition represents success
    int priority;                               // Which condition "wins" if multiple trigger
    
    // Required methods you must implement:
    
    /**
     * Check if this condition is currently met
     * Called many times per second during the trial
     * Return true when your condition is satisfied
     */
    virtual bool evaluate() = 0;
    
    /**
     * Provide a human-readable message about what happened
     * Used for logging and displaying results to participants
     */
    virtual std::string getMessage() = 0;
    
    /**
     * Return the name of this condition type
     * Used for identification and debugging
     */
    virtual std::string getName() = 0;
};
```

## Required Methods

### `evaluate()`

**Purpose**: Check whether your condition is currently satisfied.

**When called**: Continuously during the trial (typically 60-120 times per second)

**Return value**: 
- `true` = condition is met, end the trial
- `false` = condition not met, keep checking

**Example**:
```cpp
bool TimeCondition::evaluate() {
    // Get how long the trial has been running
    double elapsedTime = SimulationManager::trialContext.time - SimulationManager::trialContext.startTime;
    
    // Check if enough time has passed
    return elapsedTime >= _durationSeconds;
}
```

**Important**: Keep this method fast! It's called very frequently and any delays will affect trial performance.

### `getMessage()`

**Purpose**: Provide information about what happened when the condition was met.

**When called**: After `evaluate()` returns `true`, to log the result

**Return value**: A descriptive string explaining what occurred

**Example**:
```cpp
std::string PositionCondition::getMessage() {
    cVector3d currentPos = SimulationManager::simContext.tool->getDeviceLocalPos();
    return "Target reached at position (" + 
           std::to_string(currentPos.x()) + ", " +
           std::to_string(currentPos.y()) + ", " +
           std::to_string(currentPos.z()) + ")";
}
```

### `getName()`

**Purpose**: Identify this type of condition for debugging and logging.

**Return value**: A simple string name for this condition type

**Example**:
```cpp
std::string TimeCondition::getName() {
    return "TimeCondition";
}
```

## Configuration Parameters

Conditions receive their configuration through the `params` map, which contains key-value pairs from the XML configuration:

**XML Configuration**:
```xml
<Condition name="time" success="true" priority="1">
  <Parameters duration_seconds="30.0" warning_time="25.0"/>
</Condition>
```

**Accessing Parameters in Code**:
```cpp
TimeCondition::TimeCondition(std::map<std::string, std::string>& params,
                            bool success, int priority)
    : TrialConditionInterface(params, success, priority) {
    
    // Read the duration_seconds parameter
    auto it = params.find("duration_seconds");
    if (it != params.end()) {
        try {
            _durationSeconds = std::stod(it->second);  // Convert string to double
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid duration_seconds: " << it->second << std::endl;
            _durationSeconds = 0.0;  // Default value
        }
    } else {
        std::cerr << "Missing duration_seconds parameter" << std::endl;
        _durationSeconds = 0.0;
    }
}
```

**Common Parameter Patterns**:
```cpp
// Reading numeric parameters with defaults
double getDoubleParam(const std::string& name, double defaultValue = 0.0) {
    auto it = params.find(name);
    if (it != params.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid " << name << ": " << it->second << std::endl;
        }
    }
    return defaultValue;
}

// Reading boolean parameters
bool getBoolParam(const std::string& name, bool defaultValue = false) {
    auto it = params.find(name);
    if (it != params.end()) {
        return (it->second == "true" || it->second == "1");
    }
    return defaultValue;
}
```

## Success vs. Failure Conditions

The `success` flag determines what happens when the condition is met:

- **success = true**: Trial ends with successful completion
- **success = false**: Trial ends with failure/timeout

**Example**: A typical experiment might have:
```xml
<!-- Success condition: reach the target -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
</Condition>

<!-- Failure condition: take too long -->
<Condition name="time" success="false" priority="2">
  <Parameters duration_seconds="30.0"/>
</Condition>
```

## Priority System

When multiple conditions are met simultaneously, the one with the **lower priority number** takes precedence:

- `priority="1"` = highest priority (most important)
- `priority="2"` = medium priority  
- `priority="3"` = lowest priority

This is useful for safety conditions that should always override task conditions.

## Registration

Custom conditions must be registered to be available for use:

```cpp
// In your condition's source file or registration module
TrialConditionRegistry::registerCondition("my_condition", 
    [](std::map<std::string, std::string>& params, bool success, int priority) 
    -> std::unique_ptr<TrialConditionInterface> {
        return std::make_unique<MyCondition>(params, success, priority);
    });
```

After registration, you can use it in XML:
```xml
<Condition name="my_condition" success="true" priority="1">
  <Parameters my_param="value"/>
</Condition>
```

## Accessing Simulation Data

Conditions can access live simulation data through the `SimulationManager`:

```cpp
// Get haptic device position
cVector3d devicePos = SimulationManager::simContext.tool->getDeviceLocalPos();

// Get trial timing information
double currentTime = SimulationManager::trialContext.time;
double trialStartTime = SimulationManager::trialContext.startTime;
double elapsedTime = currentTime - trialStartTime;

// Access objects in the scene (if needed)
// (More advanced - see simulation manager documentation)
```

## Thread Safety

Conditions are evaluated from the simulation thread. Keep in mind:

- Don't perform expensive calculations in `evaluate()`
- Avoid accessing external resources that might block
- Use appropriate synchronization if sharing data between conditions

## Best Practices

1. **Keep `evaluate()` fast**: It's called many times per second
2. **Provide clear messages**: Help researchers understand what happened
3. **Handle missing parameters gracefully**: Use sensible defaults
4. **Test edge cases**: What if parameters are invalid or extreme?
5. **Document your conditions**: Explain what they do and what parameters they need