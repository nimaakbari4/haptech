# Troubleshooting Trial Conditions

Common issues and solutions when working with trial conditions.

## Quick Diagnostics

### Condition Never Triggers

**Symptoms**: Trial runs indefinitely, expected condition doesn't end it
**Most likely causes**: Parameter values, condition logic, timing issues

**Quick checks**:
1. Verify parameter values in XML are reasonable
2. Check that condition name matches registered name
3. Ensure required parameters are present
4. Test with simplified parameter values

**Example debugging**:
```xml
<!-- Original (not working) -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.001" target_y="0.001" target_z="0.001" tolerance="0.0001"/>
</Condition>

<!-- Test version (easier to reach) -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.05"/>
</Condition>
```

### Condition Triggers Immediately

**Symptoms**: Trial ends right after starting
**Most likely causes**: Threshold too low, initialization issues, wrong parameters

**Quick checks**:
1. Check if start position already meets end condition
2. Verify threshold values make sense
3. Look for parameter parsing errors in console output

**Example fixes**:
```xml
<!-- Problem: Time condition with 0 duration -->
<Condition name="time" success="false" priority="1">
  <Parameters duration_seconds="0.0"/>  <!-- Triggers immediately! -->
</Condition>

<!-- Solution: Reasonable duration -->
<Condition name="time" success="false" priority="1">
  <Parameters duration_seconds="30.0"/>
</Condition>
```

### Wrong Condition Ends Trial

**Symptoms**: Unexpected condition terminates trial (e.g., timeout instead of success)
**Most likely causes**: Priority issues, multiple conditions triggering

**Quick checks**:
1. Review priority numbers (lower = higher priority)
2. Check if higher-priority conditions are being met first
3. Verify success/failure flags are correct

## Common Issues by Condition Type

### TimeCondition Problems

**Issue**: "Time condition triggers at wrong time"
```xml
<Condition name="time" success="true" priority="1">
  <Parameters duration_seconds="30.0"/>
</Condition>
```

**Possible causes**:
- **Missing parameter**: `duration_seconds` not specified
- **Invalid value**: Non-numeric or negative duration
- **Wrong timing reference**: Condition uses trial time, not wall clock time

**Solutions**:
```xml
<!-- Correct time condition -->
<Condition name="time" success="true" priority="1">
  <Parameters duration_seconds="30.0"/>
</Condition>

<!-- For immediate start -->
<Condition name="time" success="true" priority="1">
  <Parameters duration_seconds="0.0"/>
</Condition>
```

**Debug output**: Look for console messages like:
```
[TimeCondition] Missing duration_seconds parameter
[TimeCondition] Invalid duration_seconds parameter: abc
```

### PositionCondition Problems

**Issue**: "Position condition never triggers despite reaching target"

**Common causes**:
1. **Tolerance too small**: Target is achievable but tolerance is unrealistic
2. **Wrong coordinate system**: Target position outside haptic workspace
3. **Typo in coordinates**: Wrong axis or sign

**Diagnostic steps**:
```xml
<!-- Test with very loose tolerance -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.1"/>
</Condition>
```

**Common fixes**:
```xml
<!-- Too tight (hard to reach) -->
<Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.001"/>

<!-- Better (achievable) -->
<Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>

<!-- Check workspace limits -->
<Parameters target_x="0.15" target_y="0.1" target_z="0.05" tolerance="0.02"/>
<!-- Make sure this position is within your haptic device's workspace -->
```

**Debug technique**: Add temporary large tolerance to verify target is reachable, then gradually reduce.

### VelocityCondition Problems

**Issue**: "Velocity condition triggers too easily/never triggers"

**Common causes**:
1. **Unrealistic thresholds**: Too high or too low for human movement
2. **Noise sensitivity**: Brief velocity spikes triggering condition
3. **Calculation timing**: Velocity computed from position differences

**Typical velocity ranges**:
- **Very slow**: 0.01-0.05 m/s (precision tasks)
- **Slow**: 0.05-0.2 m/s (controlled movement) 
- **Normal**: 0.2-0.5 m/s (comfortable movement)
- **Fast**: 0.5-1.0 m/s (rapid movement)
- **Very fast**: 1.0+ m/s (safety limit)

**Solutions**:
```xml
<!-- Too sensitive (triggers on small movements) -->
<Parameters max_velocity="0.01"/>

<!-- More reasonable -->
<Parameters max_velocity="0.3"/>

<!-- Safety condition -->
<Parameters max_velocity="0.8"/>
```

## Parameter Issues

### Missing Parameters

**Symptoms**: Console errors, condition uses default values
**Solution**: Check XML spelling and ensure all required parameters are present

```xml
<!-- Missing tolerance parameter -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.0" target_z="0.0"/>
  <!-- tolerance missing! -->
</Condition>

<!-- Complete parameters -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
</Condition>
```

### Invalid Parameter Values

**Symptoms**: Console error messages, condition doesn't behave as expected

**Common issues**:
- Non-numeric values: `duration_seconds="thirty"`
- Negative values where positive expected: `tolerance="-0.1"`
- Out-of-range values: `max_velocity="1000.0"`

**Debug approach**:
1. Check console output for parsing error messages
2. Verify parameter types (numbers vs. strings)
3. Validate parameter ranges make physical sense

### Parameter Units

**Issue**: Condition behaves differently than expected due to unit confusion

**Haptic system units**:
- **Position**: meters (m)
- **Velocity**: meters per second (m/s)
- **Time**: seconds (s)
- **Force**: Newtons (N)

**Common mistakes**:
```xml
<!-- Wrong: Using centimeters instead of meters -->
<Parameters target_x="10.0" target_y="0.0" target_z="0.0" tolerance="2.0"/>

<!-- Correct: Using meters -->
<Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>

<!-- Wrong: Using milliseconds instead of seconds -->
<Parameters duration_seconds="5000"/>

<!-- Correct: Using seconds -->
<Parameters duration_seconds="5.0"/>
```

## Priority and Multiple Conditions

### Unexpected Priority Behavior

**Issue**: Wrong condition ends trial despite priority settings

**Debug steps**:
1. **List all conditions with priorities**:
   ```xml
   <!-- Safety: priority 1 (highest) -->
   <Condition name="velocity" success="false" priority="1">
   
   <!-- Task: priority 2 -->
   <Condition name="position" success="true" priority="2">
   
   <!-- Timeout: priority 3 (lowest) -->
   <Condition name="time" success="false" priority="3">
   ```

2. **Check which condition actually triggered**: Look at trial result message
3. **Verify priority numbers**: Lower numbers = higher priority

**Common fix**: Adjust priorities to match intended behavior
```xml
<!-- Problem: Safety condition has low priority -->
<Condition name="velocity" success="false" priority="3">  <!-- Wrong! -->
<Condition name="position" success="true" priority="1">

<!-- Solution: Safety condition has high priority -->
<Condition name="velocity" success="false" priority="1">  <!-- Correct! -->
<Condition name="position" success="true" priority="2">
```

### Simultaneous Condition Triggering

**Issue**: Unclear which condition should win when multiple trigger at once

**Solution**: Use distinct priorities and understand evaluation order
```xml
<!-- Ambiguous: Same priority -->
<Condition name="position" success="true" priority="1">
<Condition name="velocity" success="false" priority="1">

<!-- Clear: Different priorities -->
<Condition name="velocity" success="false" priority="1">  <!-- Safety first -->
<Condition name="position" success="true" priority="2">   <!-- Then task -->
```

## Performance Issues

### Slow Condition Evaluation

**Symptoms**: Jerky haptic feedback, delayed responses, low frame rate

**Likely causes**:
1. Expensive calculations in `evaluate()` method
2. File I/O or network operations in condition
3. Too many conditions being evaluated

**Solutions**:
- Keep `evaluate()` methods simple and fast
- Pre-calculate expensive values in constructor
- Cache results when possible
- Remove unnecessary conditions

### Memory Issues

**Symptoms**: Increasing memory usage, system slowdown over time

**Likely causes**:
1. Conditions storing too much historical data
2. Memory leaks in custom conditions
3. Inefficient data structures

**Prevention**:
- Limit stored data in conditions
- Use appropriate data structures
- Clean up resources in destructors

## XML Configuration Problems

### Malformed XML

**Symptoms**: Conditions not loading, parsing errors

**Common issues**:
```xml
<!-- Missing closing tag -->
<Condition name="time" success="true" priority="1">
  <Parameters duration_seconds="30.0"/>
<!-- Missing </Condition> -->

<!-- Mismatched tags -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1"/>
</Conditon>  <!-- Typo in closing tag -->

<!-- Missing quotes -->
<Condition name=time success="true" priority="1">
```

**Solution**: Validate XML syntax using an XML validator or IDE

### Case Sensitivity

**Issue**: Condition names, parameter names, or values with wrong capitalization

```xml
<!-- Wrong case -->
<Condition name="Time" success="true">  <!-- Should be "time" -->
<Parameters Duration_Seconds="30.0"/>   <!-- Should be "duration_seconds" -->

<!-- Correct -->
<Condition name="time" success="true">
<Parameters duration_seconds="30.0"/>
```

### Boolean Values

**Issue**: Incorrect boolean parameter values

```xml
<!-- These don't work -->
<Condition success="True">    <!-- Should be "true" -->
<Condition success="FALSE">   <!-- Should be "false" -->
<Condition success="1">       <!-- Should be "true" -->

<!-- Correct -->
<Condition success="true">
<Condition success="false">
```

## Custom Condition Issues

### Registration Problems

**Symptoms**: "Unknown condition type" errors

**Checklist**:
1. Condition is registered in `ExtensionsRegistry::registerAllExtensions()`
2. Registration name matches XML `name` attribute
3. Include file is added to extensions registry
4. Source file is added to Makefile
5. Code compiles without errors

### Interface Implementation

**Common mistakes**:
1. **Missing `override` keyword**: Can lead to subtle bugs
2. **Wrong method signatures**: Must match interface exactly
3. **Forgot to implement required methods**: Pure virtual methods must be implemented

```cpp
// Wrong: Missing override, wrong signature
bool evaluate() { return true; }
string getMessage() { return ""; }  // Wrong return type

// Correct: Proper interface implementation
bool evaluate() override { return true; }
std::string getMessage() override { return "Message"; }
std::string getName() override { return "MyCondition"; }
```

## Debug Techniques

### Console Output

Add debug prints to understand condition behavior:
```cpp
bool MyCondition::evaluate() {
    double currentValue = getCurrentValue();
    
    // Debug output (remove for production)
    static int count = 0;
    if (++count % 60 == 0) {  // Print every second (60 FPS)
        std::cout << "[MyCondition] Value: " << currentValue 
                  << ", Threshold: " << _threshold << std::endl;
    }
    
    return currentValue >= _threshold;
}
```

### Simplified Testing

Create minimal test configurations:
```xml
<!-- Test individual condition in isolation -->
<EndConditions>
  <Condition name="time" success="true" priority="1">
    <Parameters duration_seconds="5.0"/>  <!-- Short test -->
  </Condition>
</EndConditions>
```

### Parameter Sweeping

Test different parameter ranges systematically:
```xml
<!-- Test 1: Very loose tolerance -->
<Parameters tolerance="0.1"/>

<!-- Test 2: Moderate tolerance -->
<Parameters tolerance="0.05"/>

<!-- Test 3: Tight tolerance -->
<Parameters tolerance="0.01"/>
```

## Getting Help

### Log Analysis

Save and analyze trial logs:
1. Look for condition trigger messages
2. Check parameter parsing warnings
3. Note timing patterns

### Community Resources

1. **Documentation**: Re-read relevant sections
2. **Code examples**: Study working configurations
3. **Console output**: Error messages often point to specific issues
4. **Incremental testing**: Start simple, add complexity gradually

### Reporting Issues

When asking for help, include:
1. **XML configuration**: The specific condition causing problems
2. **Expected behavior**: What should happen
3. **Actual behavior**: What actually happens
4. **Console output**: Any error messages
5. **System info**: Haptic device, operating system, etc.

Remember: Most condition issues stem from parameter values or XML configuration rather than bugs in the underlying system. Start with the simplest possible test case and gradually add complexity until you isolate the problem.