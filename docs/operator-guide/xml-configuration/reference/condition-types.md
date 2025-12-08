# Condition Types Reference

Complete reference for all available condition types used in start and end conditions for haptic trials.

## Overview

Conditions define when trials start and end, and determine success or failure outcomes. Each condition type monitors specific aspects of the trial state and triggers when certain criteria are met.

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

### `time_limit`
Alias for time condition commonly used for trial timeouts.

**Same parameters as `time`**

```xml
<Condition name="time_limit" success="false" priority="1">
    <Parameters duration_seconds="45.0"/>
</Condition>
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

### `position_sequence`
Triggers when multiple positions are reached in order.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------| 
| `positions` | array | Yes | List of target positions |
| `tolerance` | float | Yes | Distance tolerance for each position |
| `reset_on_fail` | bool | No | Reset sequence if position is missed |

#### Usage Example

```xml
<Condition name="position_sequence" success="true" priority="1">
    <Parameters tolerance="0.02" reset_on_fail="true">
        <Position x="0.05" y="0.0" z="0.0"/>
        <Position x="0.1" y="0.05" z="0.0"/>
        <Position x="0.15" y="0.0" z="0.0"/>
    </Parameters>
</Condition>
```

## Velocity Conditions

### `velocity`
Triggers based on movement speed.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `max_velocity` | float | No* | Maximum allowed speed |
| `min_velocity` | float | No* | Minimum required speed |
| `target_velocity` | float | No* | Target speed |
| `tolerance` | float | No | Speed tolerance |
| `duration` | float | No | Time condition must hold |

*At least one velocity parameter is required

#### Usage Examples

**Speed limit**:
```xml
<Condition name="velocity" success="false" priority="2">
    <Parameters max_velocity="0.3"/>  <!-- Fail if faster than 30cm/s -->
</Condition>
```

**Minimum speed requirement**:
```xml
<Condition name="velocity" success="false" priority="3">
    <Parameters 
        min_velocity="0.05" 
        duration="2.0"  <!-- Must move at least 5cm/s for 2 seconds -->
    />
</Condition>
```

**Target speed**:
```xml
<Condition name="velocity" success="true" priority="1">
    <Parameters 
        target_velocity="0.15" 
        tolerance="0.05" 
        duration="1.0"  <!-- Maintain 15±5cm/s for 1 second -->
    />
</Condition>
```

### `acceleration`
Triggers based on movement acceleration.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `max_acceleration` | float | No* | Maximum allowed acceleration |
| `min_acceleration` | float | No* | Minimum required acceleration |
| `duration` | float | No | Time condition must hold |

*At least one acceleration parameter is required

```xml
<Condition name="acceleration" success="false" priority="2">
    <Parameters max_acceleration="2.0"/>  <!-- Fail if acceleration > 2 m/s² -->
</Condition>
```

## Force Conditions

### `force`
Triggers based on applied force magnitude.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `force_threshold` | float | No* | Force magnitude threshold |
| `force_x` | float | No* | Force in X direction |
| `force_y` | float | No* | Force in Y direction |
| `force_z` | float | No* | Force in Z direction |
| `duration` | float | No | Time condition must hold |
| `tolerance` | float | No | Force tolerance |

*At least one force parameter is required

#### Usage Examples

**Force threshold**:
```xml
<Condition name="force" success="true" priority="1">
    <Parameters 
        force_threshold="3.0" 
        duration="1.0"  <!-- Apply 3N force for 1 second -->
    />
</Condition>
```

**Directional force**:
```xml
<Condition name="force" success="true" priority="1">
    <Parameters 
        force_x="2.0" 
        tolerance="0.5" 
        duration="0.5"  <!-- Apply 2±0.5N rightward force for 0.5s -->
    />
</Condition>
```

### `force_limit`
Triggers when force exceeds safety limits.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `max_force` | float | Yes | Maximum allowed force |
| `duration` | float | No | Time before trigger |

```xml
<Condition name="force_limit" success="false" priority="1">
    <Parameters max_force="10.0" duration="0.1"/>
</Condition>
```

## Contact Conditions

### `contact`
Triggers when the haptic device contacts a specific object.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `object_id` | string | Yes | ID of target object |
| `duration` | float | No | Time contact must be maintained |
| `force_threshold` | float | No | Minimum contact force |

#### Usage Examples

**Touch target**:
```xml
<Condition name="contact" success="true" priority="1">
    <Parameters object_id="target_sphere"/>
</Condition>
```

**Sustained contact**:
```xml
<Condition name="contact" success="true" priority="1">
    <Parameters 
        object_id="pressure_plate" 
        duration="2.0" 
        force_threshold="1.0"  <!-- Hold contact for 2s with 1N force -->
    />
</Condition>
```

**Avoid obstacle**:
```xml
<Condition name="contact" success="false" priority="2">
    <Parameters object_id="obstacle_box"/>  <!-- Fail if touched -->
</Condition>
```

### `contact_sequence`
Triggers when objects are contacted in a specific order.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `object_sequence` | array | Yes | Ordered list of object IDs |
| `reset_on_wrong` | bool | No | Reset sequence on wrong contact |
| `timeout` | float | No | Time limit for sequence |

```xml
<Condition name="contact_sequence" success="true" priority="1">
    <Parameters reset_on_wrong="true" timeout="30.0">
        <ObjectID>target_1</ObjectID>
        <ObjectID>target_2</ObjectID>
        <ObjectID>target_3</ObjectID>
    </Parameters>
</Condition>
```

## Distance Conditions

### `distance`
Triggers based on distance from objects or positions.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `object_id` | string | No* | Target object ID |
| `target_x` | float | No* | Target X position |
| `target_y` | float | No* | Target Y position |
| `target_z` | float | No* | Target Z position |
| `min_distance` | float | No** | Minimum allowed distance |
| `max_distance` | float | No** | Maximum allowed distance |
| `target_distance` | float | No** | Target distance |
| `tolerance` | float | No | Distance tolerance |

*Either object_id OR target position required
**At least one distance parameter required

#### Usage Examples

**Stay close to object**:
```xml
<Condition name="distance" success="false" priority="2">
    <Parameters 
        object_id="guide_object" 
        max_distance="0.05"  <!-- Fail if more than 5cm away -->
    />
</Condition>
```

**Maintain distance**:
```xml
<Condition name="distance" success="true" priority="1">
    <Parameters 
        target_x="0.1" 
        target_y="0.0" 
        target_z="0.0" 
        target_distance="0.03" 
        tolerance="0.01"  <!-- Stay 3±1cm from position -->
    />
</Condition>
```

## Path Conditions

### `path_following`
Triggers based on following a predefined path.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `path_points` | array | Yes | Ordered path waypoints |
| `tolerance` | float | Yes | Distance tolerance from path |
| `completion_threshold` | float | No | Percentage of path to complete |
| `speed_limit` | float | No | Maximum movement speed |

```xml
<Condition name="path_following" success="true" priority="1">
    <Parameters tolerance="0.02" completion_threshold="0.9">
        <PathPoint x="0.0" y="0.0" z="0.0"/>
        <PathPoint x="0.05" y="0.05" z="0.0"/>
        <PathPoint x="0.1" y="0.0" z="0.0"/>
        <PathPoint x="0.15" y="-0.05" z="0.0"/>
    </Parameters>
</Condition>
```

### `path_deviation`
Triggers when movement deviates too far from expected path.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `reference_path` | array | Yes | Expected path points |
| `max_deviation` | float | Yes | Maximum allowed deviation |
| `smoothing_window` | int | No | Points to average for smoothing |

```xml
<Condition name="path_deviation" success="false" priority="2">
    <Parameters max_deviation="0.03" smoothing_window="5">
        <PathPoint x="0.0" y="0.0" z="0.0"/>
        <PathPoint x="0.1" y="0.0" z="0.0"/>
        <PathPoint x="0.2" y="0.0" z="0.0"/>
    </Parameters>
</Condition>
```

## Region Conditions

### `region_enter`
Triggers when entering a defined 3D region.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `region_type` | string | Yes | "box", "sphere", "cylinder" |
| `center_x` | float | Yes | Region center X |
| `center_y` | float | Yes | Region center Y |
| `center_z` | float | Yes | Region center Z |
| `size_x` | float | No* | Box width or sphere radius |
| `size_y` | float | No* | Box height or cylinder radius |
| `size_z` | float | No* | Box depth or cylinder height |

*Required parameters depend on region_type

#### Usage Examples

**Enter sphere region**:
```xml
<Condition name="region_enter" success="true" priority="1">
    <Parameters 
        region_type="sphere"
        center_x="0.1" center_y="0.0" center_z="0.0"
        size_x="0.04"  <!-- 4cm radius sphere -->
    />
</Condition>
```

**Enter box region**:
```xml
<Condition name="region_enter" success="true" priority="1">
    <Parameters 
        region_type="box"
        center_x="0.0" center_y="0.1" center_z="0.0"
        size_x="0.06" size_y="0.04" size_z="0.02"  <!-- 6x4x2cm box -->
    />
</Condition>
```

### `region_exit`
Triggers when leaving a defined region.

**Same parameters as `region_enter`**

```xml
<Condition name="region_exit" success="false" priority="2">
    <Parameters 
        region_type="sphere"
        center_x="0.0" center_y="0.0" center_z="0.0"
        size_x="0.15"  <!-- Fail if leave 15cm workspace -->
    />
</Condition>
```

## Performance Conditions

### `accuracy`
Triggers based on movement accuracy metrics.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `target_accuracy` | float | Yes | Required accuracy percentage |
| `measurement_type` | string | Yes | "rmse", "max_error", "mean_error" |
| `reference_path` | array | No | Reference for accuracy calculation |
| `window_size` | int | No | Samples to include in calculation |

```xml
<Condition name="accuracy" success="true" priority="1">
    <Parameters 
        target_accuracy="0.9" 
        measurement_type="rmse"
        window_size="100"
    />
</Condition>
```

### `efficiency`
Triggers based on movement efficiency metrics.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `min_efficiency` | float | Yes | Minimum efficiency ratio |
| `measurement_type` | string | Yes | "path_ratio", "time_ratio" |
| `reference_time` | float | No | Expected completion time |
| `reference_distance` | float | No | Expected path length |

```xml
<Condition name="efficiency" success="true" priority="1">
    <Parameters 
        min_efficiency="0.8" 
        measurement_type="path_ratio"
        reference_distance="0.2"
    />
</Condition>
```

## Logical Conditions

### `and`
Triggers when ALL child conditions are true.

```xml
<Condition name="and" success="true" priority="1">
    <Condition name="position" success="true" priority="1">
        <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </Condition>
    <Condition name="force" success="true" priority="2">
        <Parameters force_threshold="2.0" duration="1.0"/>
    </Condition>
</Condition>
```

### `or`
Triggers when ANY child condition is true.

```xml
<Condition name="or" success="true" priority="1">
    <Condition name="contact" success="true" priority="1">
        <Parameters object_id="target_1"/>
    </Condition>
    <Condition name="contact" success="true" priority="2">
        <Parameters object_id="target_2"/>
    </Condition>
</Condition>
```

### `not`
Triggers when child condition is NOT true.

```xml
<Condition name="not" success="false" priority="2">
    <Condition name="contact" success="true" priority="1">
        <Parameters object_id="forbidden_object"/>
    </Condition>
</Condition>
```

## Custom Conditions

### `script`
Triggers based on custom script evaluation.

#### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `script_file` | string | No* | Path to script file |
| `script_code` | string | No* | Inline script code |
| `update_rate` | float | No | Script evaluation frequency (Hz) |

*Either script_file OR script_code required

```xml
<Condition name="script" success="true" priority="1">
    <Parameters 
        script_code="return device.position.distance(target) < 0.02 and device.velocity.magnitude() < 0.1"
        update_rate="100"
    />
</Condition>
```

## Condition Best Practices

### Priority Guidelines

1. **Safety conditions** (force limits, workspace bounds): Priority 1
2. **Primary success conditions**: Priority 2-3
3. **Secondary conditions**: Priority 4-5
4. **Timeout conditions**: Lowest priority

### Parameter Selection

**Position tolerances**:
- Precise tasks: 0.005-0.01m
- Normal tasks: 0.01-0.02m
- Coarse tasks: 0.02-0.05m

**Time limits**:
- Practice trials: 60-120 seconds
- Experimental trials: 30-60 seconds
- Quick tasks: 10-30 seconds

**Velocity limits**:
- Slow movements: 0.05-0.1 m/s
- Normal movements: 0.1-0.5 m/s
- Fast movements: 0.5-1.0 m/s

### Common Patterns

**Basic reaching task**:
```xml
<EndConditions>
    <Condition name="position" success="true" priority="1">
        <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </Condition>
    <Condition name="time_limit" success="false" priority="2">
        <Parameters duration_seconds="30.0"/>
    </Condition>
</EndConditions>
```

**Speed-accuracy tradeoff**:
```xml
<EndConditions>
    <Condition name="and" success="true" priority="1">
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.01"/>
        </Condition>
        <Condition name="velocity" success="true" priority="2">
            <Parameters max_velocity="0.2"/>
        </Condition>
    </Condition>
    <Condition name="time_limit" success="false" priority="2">
        <Parameters duration_seconds="45.0"/>
    </Condition>
</EndConditions>
```