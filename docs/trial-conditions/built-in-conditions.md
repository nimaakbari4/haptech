# Built-in Conditions

Ready-to-use trial conditions for common experimental scenarios. These conditions are already implemented and can be used directly in your XML configuration files.

## TimeCondition

**Purpose**: Ends trials based on elapsed time.

**Common uses**:
- Trial timeouts ("end after 60 seconds if task not completed")
- Immediate start conditions ("start immediately")
- Forced rest periods ("wait 5 seconds before starting")

**XML Configuration**:
```xml
<Condition name="time" success="true" priority="1">
  <Parameters duration_seconds="30.0"/>
</Condition>
```

**Parameters**:
- `duration_seconds` (required): How many seconds to wait before triggering
  - For start conditions: delay before trial begins
  - For end conditions: maximum trial duration

**Examples**:
```xml
<!-- Start immediately -->
<StartConditions>
  <Condition name="time" success="true" priority="1">
    <Parameters duration_seconds="0.0"/>
  </Condition>
</StartConditions>

<!-- End after 45 seconds (timeout/failure) -->
<EndConditions>
  <Condition name="time" success="false" priority="2">
    <Parameters duration_seconds="45.0"/>
  </Condition>
</EndConditions>

<!-- Wait 3 seconds before starting (preparation time) -->
<StartConditions>
  <Condition name="time" success="true" priority="1">
    <Parameters duration_seconds="3.0"/>
  </Condition>
</StartConditions>
```

**Research applications**: Standardizing trial durations, providing consistent timeouts across participants, measuring reaction times.

---

## PositionCondition

**Purpose**: Ends trials when the haptic device reaches a target position.

**Common uses**:
- "Reach the target" tasks
- Workspace boundary enforcement
- Navigation and pointing experiments

**XML Configuration**:
```xml
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.0" target_z="0.05" tolerance="0.02"/>
</Condition>
```

**Parameters**:
- `target_x`, `target_y`, `target_z` (required): Target position coordinates in meters
- `tolerance` (required): How close the device must be to the target (meters)

**Examples**:
```xml
<!-- Success: reach the target within 2cm -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.15" target_y="0.1" target_z="0.0" tolerance="0.02"/>
</Condition>

<!-- Safety: end if device goes too far in any direction -->
<Condition name="position" success="false" priority="1">
  <Parameters target_x="0.5" target_y="0.0" target_z="0.0" tolerance="0.1"/>
</Condition>

<!-- Multiple targets using multiple conditions -->
<Condition name="position" success="true" priority="1">
  <Parameters target_x="0.1" target_y="0.1" target_z="0.0" tolerance="0.015"/>
</Condition>
<Condition name="position" success="true" priority="1">
  <Parameters target_x="-0.1" target_y="0.1" target_z="0.0" tolerance="0.015"/>
</Condition>
```

**Tips**:
- Use tighter tolerances (0.005-0.01m) for precision tasks
- Use looser tolerances (0.02-0.05m) for gross motor tasks
- Consider the haptic device's workspace limits when setting targets

**Research applications**: Motor control studies, target acquisition tasks, spatial navigation experiments, rehabilitation protocols.

---

## VelocityCondition

**Purpose**: Ends trials when the haptic device velocity exceeds a threshold.

**Common uses**:
- Safety conditions ("stop if moving too fast")
- Movement quality assessment
- Preventing ballistic movements

**XML Configuration**:
```xml
<Condition name="velocity" success="false" priority="1">
  <Parameters max_velocity="0.5"/>
</Condition>
```

**Parameters**:
- `max_velocity` (required): Maximum allowed velocity in meters per second

**Examples**:
```xml
<!-- Safety: end trial if participant moves too quickly -->
<Condition name="velocity" success="false" priority="1">
  <Parameters max_velocity="0.3"/>
</Condition>

<!-- Task requirement: must move slowly and deliberately -->
<Condition name="velocity" success="false" priority="2">
  <Parameters max_velocity="0.1"/>
</Condition>

<!-- High-speed movement detection -->
<Condition name="velocity" success="true" priority="1">
  <Parameters max_velocity="1.0"/>
</Condition>
```

**Typical velocity ranges**:
- **Very slow/precise**: 0.01-0.05 m/s
- **Slow/controlled**: 0.05-0.2 m/s  
- **Normal**: 0.2-0.5 m/s
- **Fast**: 0.5-1.0 m/s
- **Very fast/safety limit**: 1.0+ m/s

**Research applications**: Motor learning studies, movement smoothness analysis, safety monitoring, tremor detection.

---

## Combining Conditions

### Success and Failure Conditions Together

```xml
<EndConditions>
  <!-- Primary success condition -->
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.015"/>
  </Condition>
  
  <!-- Safety condition (higher priority) -->
  <Condition name="velocity" success="false" priority="1">
    <Parameters max_velocity="0.4"/>
  </Condition>
  
  <!-- Timeout condition (lower priority) -->
  <Condition name="time" success="false" priority="3">
    <Parameters duration_seconds="60.0"/>
  </Condition>
</EndConditions>
```

**Priority explanation**:
- Velocity and position both have priority 1 - either can end the trial immediately
- Time has priority 3 - only triggers if neither other condition is met

### Multiple Success Criteria

```xml
<!-- Reach any of three targets -->
<EndConditions>
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.1" target_y="0.1" target_z="0.0" tolerance="0.02"/>
  </Condition>
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.0" target_y="0.1" target_z="0.1" tolerance="0.02"/>
  </Condition>
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="-0.1" target_y="0.1" target_z="0.0" tolerance="0.02"/>
  </Condition>
  
  <!-- Timeout -->
  <Condition name="time" success="false" priority="2">
    <Parameters duration_seconds="30.0"/>
  </Condition>
</EndConditions>
```

## Best Practices

### Condition Design
1. **Always include a timeout**: Prevent trials from running indefinitely
2. **Use appropriate priorities**: Safety conditions should have high priority (low numbers)
3. **Test parameter ranges**: Make sure thresholds are achievable but challenging
4. **Consider participant variability**: What's easy for one person might be impossible for another

### Parameter Selection
1. **Position tolerance**: 
   - Start with 2-3cm for new participants
   - Reduce to 1cm or less for skilled participants
   - Consider the haptic device's resolution (typically ~0.1mm)

2. **Time limits**:
   - Practice trials: 60-120 seconds (learning time)
   - Experimental trials: 30-60 seconds (focused performance)
   - Baseline measurements: 10-20 seconds (quick assessment)

3. **Velocity limits**:
   - Precision tasks: 0.05-0.2 m/s
   - Normal tasks: 0.2-0.5 m/s
   - Safety limits: 0.5-1.0 m/s

### Experimental Design
1. **Start simple**: Use basic time and position conditions first
2. **Pilot test**: Try conditions with a few participants before full data collection
3. **Document parameters**: Keep notes on what values work well for your task
4. **Consider individual differences**: Some participants may need adjusted parameters

### Common Configurations

**Reaching task**:
```xml
<StartConditions>
  <Condition name="time" success="true" priority="1">
    <Parameters duration_seconds="0.0"/>
  </Condition>
</StartConditions>
<EndConditions>
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.12" target_y="0.08" target_z="0.0" tolerance="0.02"/>
  </Condition>
  <Condition name="time" success="false" priority="2">
    <Parameters duration_seconds="45.0"/>
  </Condition>
</EndConditions>
```

**Speed-accuracy task**:
```xml
<EndConditions>
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.01"/>
  </Condition>
  <Condition name="velocity" success="false" priority="1">
    <Parameters max_velocity="0.15"/>
  </Condition>
  <Condition name="time" success="false" priority="3">
    <Parameters duration_seconds="30.0"/>
  </Condition>
</EndConditions>
```

**Safety-critical task**:
```xml
<EndConditions>
  <Condition name="velocity" success="false" priority="1">
    <Parameters max_velocity="0.5"/>
  </Condition>
  <Condition name="position" success="false" priority="1">
    <Parameters target_x="0.3" target_y="0.0" target_z="0.0" tolerance="0.05"/>
  </Condition>
  <Condition name="position" success="true" priority="2">
    <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
  </Condition>
  <Condition name="time" success="false" priority="3">
    <Parameters duration_seconds="60.0"/>
  </Condition>
</EndConditions>
```

These built-in conditions should cover most common experimental scenarios. For more complex requirements, see the [Implementation Guide](implementation-guide.md) for creating custom conditions.