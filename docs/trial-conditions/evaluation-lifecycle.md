# Evaluation Lifecycle

Understanding when and how trial conditions are checked during your experiment.

## Overview

Trial conditions are continuously monitored throughout your experiment to automatically determine when trials should start and end. This document explains the timing and order of these checks.

## When Conditions Are Evaluated

### Frequency
Conditions are evaluated at the **graphics frame rate** (typically 60-120 Hz), which means:
- `evaluate()` is called 60-120 times per second
- Each evaluation takes approximately 8-16 milliseconds
- This is frequent enough to catch rapid events but not so frequent as to interfere with haptic rendering

### Why Not Haptic Rate?
The haptic system runs at 1000 Hz (every 1 millisecond) to provide smooth force feedback. Running condition evaluation at this rate would:
- Interfere with haptic smoothness
- Consume too much computational resources
- Provide unnecessarily high temporal resolution for most experimental needs

**Practical impact**: Events lasting less than ~10 milliseconds might not be detected. For most human movement research, this is not a limitation.

## Trial Lifecycle Phases

### 1. Pre-Trial Setup
```
┌─────────────────┐
│ Load XML Config │
│ Create Objects  │
│ Init Conditions │
└─────────────────┘
```
**What happens**:
- XML configuration is parsed
- Objects and conditions are created
- Start and end conditions are initialized with their parameters
- Scene is set up but trial hasn't started yet

**Condition state**: Created but not being evaluated

### 2. Start Condition Evaluation
```
┌─────────────────┐
│ Check Start     │
│ Conditions      │
│ Every Frame     │
└─────────────────┘
```
**What happens**:
- Only start conditions are evaluated
- Scene is visible but trial timing hasn't begun
- Once any start condition returns `true`, the trial officially begins

**Common patterns**:
```xml
<!-- Start immediately -->
<StartConditions>
  <Condition name="time" success="true" priority="1">
    <Parameters duration_seconds="0.0"/>
  </Condition>
</StartConditions>

<!-- Wait for participant to reach starting position -->
<StartConditions>
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.03"/>
  </Condition>
</StartConditions>
```

### 3. Active Trial Phase
```
┌─────────────────┐
│ Check End       │
│ Conditions      │
│ Every Frame     │
│ Log Data        │
└─────────────────┘
```
**What happens**:
- End conditions are evaluated continuously
- Trial timing is active
- Data is being collected
- Participants can interact with objects
- Trial continues until any end condition returns `true`

**Timeline tracking**:
```cpp
// During active trial phase:
double elapsedTime = SimulationManager::trialContext.time - SimulationManager::trialContext.startTime;
```

### 4. Trial Termination
```
┌─────────────────┐
│ Condition Met   │
│ Stop Evaluation │
│ Log Results     │
│ Cleanup         │
└─────────────────┘
```
**What happens**:
- The triggering condition's `getMessage()` is called
- Trial result (success/failure) is determined
- Data collection stops
- Results are logged and sent to client
- Scene may be reset or cleaned up

## Evaluation Order and Priority

### Within Each Frame
Conditions are evaluated in **priority order** (lowest number = highest priority):

```xml
<EndConditions>
  <!-- Evaluated first (priority 1) -->
  <Condition name="velocity" success="false" priority="1">
    <Parameters max_velocity="0.5"/>
  </Condition>
  
  <!-- Evaluated second (priority 2) -->
  <Condition name="position" success="true" priority="2">
    <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
  </Condition>
  
  <!-- Evaluated last (priority 3) -->
  <Condition name="time" success="false" priority="3">
    <Parameters duration_seconds="60.0"/>
  </Condition>
</EndConditions>
```

**Evaluation process each frame**:
1. Check velocity condition (priority 1)
   - If `true`: End trial with failure (excessive velocity)
   - If `false`: Continue to next condition
2. Check position condition (priority 2)
   - If `true`: End trial with success (target reached)
   - If `false`: Continue to next condition
3. Check time condition (priority 3)
   - If `true`: End trial with failure (timeout)
   - If `false`: Continue trial

### Same Priority Handling
If multiple conditions have the same priority, they're evaluated in the order they appear in the XML. The **first one to return `true`** ends the trial.

```xml
<!-- Both have priority 1 - either can end the trial -->
<Condition name="position" success="true" priority="1">...</Condition>
<Condition name="velocity" success="false" priority="1">...</Condition>
```

## Performance Considerations

### Computational Cost
Since conditions are evaluated frequently, keep them efficient:

**Good** - Simple calculations:
```cpp
bool TimeCondition::evaluate() {
    double elapsed = SimulationManager::trialContext.time - SimulationManager::trialContext.startTime;
    return elapsed >= _durationSeconds;  // Simple comparison
}
```

**Avoid** - Expensive operations:
```cpp
bool BadCondition::evaluate() {
    // Don't do these in evaluate():
    readFromFile();           // File I/O
    complexMathOperation();   // Heavy calculations
    networkRequest();         // Network operations
    sleepOrWait();           // Blocking operations
}
```

### Memory Usage
Conditions should maintain minimal state:

```cpp
class EfficientCondition : public TrialConditionInterface {
private:
    // Good: Store only necessary state
    double _threshold;
    bool _triggered;
    
    // Avoid: Large data structures updated every frame
    // std::vector<double> _allValues;  // Grows continuously
    // std::map<int, ComplexObject> _cache;  // Large memory footprint
};
```

## Timing Precision

### Resolution
With 60 Hz evaluation, timing resolution is approximately **16.7 milliseconds**:
- Events shorter than this may be missed
- Timing measurements are accurate to within ~17ms
- For most human movement research, this precision is sufficient

### High-Precision Requirements
If you need sub-millisecond timing precision:
1. Use the haptic thread (advanced, requires careful programming)
2. Log timestamps and analyze offline
3. Consider hardware-based triggering for critical events

### Example Timing Analysis
```cpp
// In your condition's evaluate() method:
void MyCondition::evaluate() {
    double currentTime = SimulationManager::trialContext.time;
    
    // Log high-precision timestamps for later analysis
    if (eventDetected()) {
        logTimestamp("event_detected", currentTime);
        return true;
    }
}
```

## Debugging Evaluation Issues

### Common Problems

1. **Condition never triggers**:
   - Check parameter values (thresholds too high/low?)
   - Verify condition logic in `evaluate()`
   - Add debug output to see current values

2. **Condition triggers immediately**:
   - Check for initialization issues
   - Verify parameter parsing
   - Ensure threshold values are reasonable

3. **Inconsistent triggering**:
   - Look for race conditions between multiple conditions
   - Check priority settings
   - Verify state management

### Debug Techniques

```cpp
bool MyCondition::evaluate() {
    double currentValue = getCurrentValue();
    
    // Add debug output (remove for production)
    static int frameCount = 0;
    if (++frameCount % 60 == 0) {  // Print once per second
        std::cout << "[MyCondition] Current value: " << currentValue 
                  << ", threshold: " << _threshold << std::endl;
    }
    
    return currentValue >= _threshold;
}
```

### Logging for Analysis

```cpp
std::string MyCondition::getMessage() {
    // Include detailed information for post-experiment analysis
    return "Condition triggered at time " + 
           std::to_string(SimulationManager::trialContext.time) +
           ", value=" + std::to_string(_currentValue) +
           ", threshold=" + std::to_string(_threshold) +
           ", participant_pos=(" + 
           std::to_string(_triggerPosition.x()) + "," +
           std::to_string(_triggerPosition.y()) + "," +
           std::to_string(_triggerPosition.z()) + ")";
}
```

## Best Practices

1. **Design for the evaluation frequency**: Remember conditions are checked 60-120 times per second
2. **Use appropriate priorities**: Critical conditions (safety) should have high priority (low numbers)
3. **Keep evaluate() fast**: Avoid expensive computations
4. **Test timing sensitivity**: Ensure your conditions work reliably at the evaluation frequency
5. **Log sufficient detail**: Include enough information in messages for post-experiment analysis
6. **Consider participant variability**: Some participants may trigger conditions differently

Understanding the evaluation lifecycle helps you design robust experimental protocols and debug issues when conditions don't behave as expected.