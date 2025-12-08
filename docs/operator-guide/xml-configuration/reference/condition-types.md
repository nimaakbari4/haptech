# Condition Types Reference

Complete reference for all available condition types used in start and end conditions for haptic trials.

## Overview

Conditions define when trials start and end, and determine success or failure outcomes. Each condition type monitors specific aspects of the trial state and triggers when certain criteria are met.

**Available condition types**:
- **time**: Time-based conditions and delays
- **position**: 3D position-based conditions
- **velocity**: Speed and movement-based conditions

**Condition structure**:
- **Type**: The kind of condition to monitor
- **Success/Failure**: Whether triggering indicates success or failure
- **Priority**: Order of evaluation (1 = highest priority)
- **Parameters**: Condition-specific thresholds and settings

**Common usage**:
- **Start conditions**: When to begin data collection
- **End conditions**: When to stop and determine outcome
- **Multiple conditions**: Combine different types for complex logic

## Time Conditions

### `time`
Basic time-based condition that triggers after a specified duration.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `duration_seconds` | float | Yes | Time to wait |

#### Usage Examples

**Immediate start**:
```xml
<StartConditions>
    <Condition name="time" success="true" priority="1">
        <Parameters duration_seconds="0.0"/>
    </Condition>
</StartConditions>
```

**Delayed start**:
```xml
<StartConditions>
    <Condition name="time" success="true" priority="1">
        <Parameters duration_seconds="3.0"/>  <!-- Start after 3 seconds -->
    </Condition>
</StartConditions>
```

**Time limit**:
```xml
<EndConditions>
    <Condition name="time" success="false" priority="1">
        <Parameters duration_seconds="30.0"/>  <!-- 30 second limit -->
    </Condition>
</EndConditions>
```

## Position Conditions

### `position`
Triggers when the haptic device reaches a specific 3D position.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------| 
| `target_x` | float | Yes | Target X coordinate |
| `target_y` | float | Yes | Target Y coordinate |
| `target_z` | float | Yes | Target Z coordinate |
| `tolerance` | float | Yes | Distance tolerance |

#### Usage Examples

**Reach target**:
```xml
<Condition name="position" success="true" priority="1">
    <Parameters 
        target_x="0.1" 
        target_y="0.0" 
        target_z="0.0" 
        tolerance="0.02"  <!-- 2cm tolerance -->
    />
</Condition>
```

**Start from home**:
```xml
<StartConditions>
    <Condition name="position" success="true" priority="1">
        <Parameters 
            target_x="-0.1" 
            target_y="0.0" 
            target_z="0.0" 
            tolerance="0.03"
        />
    </Condition>
</StartConditions>
```

**Avoid forbidden zone**:
```xml
<EndConditions>
    <Condition name="position" success="false" priority="2">
        <Parameters 
            target_x="0.0" 
            target_y="0.2" 
            target_z="0.0" 
            tolerance="0.05"  <!-- Fail if within 5cm of this point -->
        />
    </Condition>
</EndConditions>
```

## Velocity Conditions

### `velocity`
Triggers based on movement speed.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `max_velocity` | float | Yes | Maximum allowed speed (m/s) |

#### Usage Examples

**Speed limit**:
```xml
<Condition name="velocity" success="false" priority="2">
    <Parameters max_velocity="0.3"/>  <!-- Fail if faster than 30cm/s -->
</Condition>
```

**Note**: Only `max_velocity` parameter is currently supported. Other velocity parameters like `min_velocity` and `target_velocity` are not available.

## Complete Trial Examples

### Basic Reaching Task
```xml
<TrialConfig>
    <StartConditions>
        <Condition name="time" success="true" priority="1">
            <Parameters duration_seconds="0.0"/>  <!-- Start immediately -->
        </Condition>
    </StartConditions>
    
    <EndConditions>
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.15" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        </Condition>
        <Condition name="time" success="false" priority="2">
            <Parameters duration_seconds="30.0"/>  <!-- Time limit -->
        </Condition>
    </EndConditions>
</TrialConfig>
```

### Speed-Controlled Task
```xml
<TrialConfig>
    <StartConditions>
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="-0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        </Condition>
    </StartConditions>
    
    <EndConditions>
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        </Condition>
        <Condition name="velocity" success="false" priority="2">
            <Parameters max_velocity="0.2"/>  <!-- Fail if too fast -->
        </Condition>
        <Condition name="time" success="false" priority="3">
            <Parameters duration_seconds="45.0"/>
        </Condition>
    </EndConditions>
</TrialConfig>
```

### Delayed Start Task
```xml
<TrialConfig>
    <StartConditions>
        <Condition name="time" success="true" priority="1">
            <Parameters duration_seconds="5.0"/>  <!-- Start after 5 seconds -->
        </Condition>
    </StartConditions>
    
    <EndConditions>
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.0" target_y="0.1" target_z="0.05" tolerance="0.03"/>
        </Condition>
        <Condition name="time" success="false" priority="2">
            <Parameters duration_seconds="60.0"/>
        </Condition>
    </EndConditions>
</TrialConfig>
```

## Condition Best Practices

### Priority Guidelines

1. **Primary success conditions**: Priority 1
2. **Movement constraints (velocity)**: Priority 2  
3. **Timeout conditions**: Priority 3

### Parameter Selection

**Position tolerances**:
- Precise tasks: 0.01-0.02m
- Normal tasks: 0.02-0.03m
- Coarse tasks: 0.03-0.05m

**Time limits**:
- Practice trials: 60-120 seconds
- Experimental trials: 30-60 seconds
- Quick tasks: 10-30 seconds

**Velocity limits**:
- Slow controlled movements: 0.1-0.2 m/s
- Normal movements: 0.2-0.5 m/s
- Note: Only max_velocity is supported

### Common Issues

**Unreachable conditions**:
- Make sure position targets are within haptic workspace (±0.3m)
- Set reasonable tolerances for task difficulty
- Include appropriate time limits for all trials

**Performance problems**:
- Use appropriate velocity limits to encourage smooth movement
- Don't set tolerances too small for the task requirements
- Always test conditions with pilot runs

## Troubleshooting

**Condition not triggering**:
- Check parameter values are within expected ranges
- Verify position coordinates are reachable
- Ensure velocity limits are appropriate for the task

**Trial ending unexpectedly**:
- Check condition priorities are set correctly
- Verify success/failure flags match intended behavior
- Review time limit settings

**XML validation errors**:
- Ensure all required parameters are included
- Check parameter names match exactly (case-sensitive)
- Verify condition names are spelled correctly