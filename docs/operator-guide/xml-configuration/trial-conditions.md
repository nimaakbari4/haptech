# Trial Conditions

Complete guide to configuring start and end conditions that control when trials begin and end.

## Overview

Trial conditions define the logical rules that determine when a trial starts and when it ends. These conditions are the core of experimental control, allowing you to create precise and reproducible experimental protocols.

**Start conditions**: Define when data collection begins
**End conditions**: Define when data collection stops and determine trial success/failure

**Key concepts**:
- **Priority**: Lower numbers = higher priority (checked first)
- **Success vs. Failure**: End conditions can indicate success or failure
- **Logical combinations**: Use AND, OR, NOT for complex logic
- **Sequences**: Multi-step conditions that must occur in order

## Start Conditions

Start conditions determine when the trial begins and data collection starts.

### Basic Start Conditions

#### Time-Based Start

```xml
<start_conditions>
    <!-- Start immediately when trial is loaded -->
    <time duration_seconds="0.0" priority="1"/>
    
    <!-- Start after 3-second delay -->
    <time duration_seconds="3.0" priority="1"/>
</start_conditions>
```

**Use cases**:
- Immediate start for simple tasks
- Delayed start to allow participant preparation
- Synchronized start with external equipment

#### Position-Based Start

```xml
<start_conditions>
    <!-- Start when haptic device is at origin -->
    <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01" priority="1"/>
    
    <!-- Start when participant reaches starting position -->
    <position target_x="-0.1" target_y="0.0" target_z="0.0" tolerance="0.02" priority="1"/>
</start_conditions>
```

**Parameters**:
- `target_x`, `target_y`, `target_z`: Required position coordinates (meters)
- `tolerance`: Accuracy required to trigger condition (meters)
- `hold_duration`: Time position must be maintained (seconds, optional)

**Use cases**:
- Ensure consistent starting position
- Require participant readiness before trial begins
- Control for initial position variability

#### Velocity-Based Start

```xml
<start_conditions>
    <!-- Start when device is stationary -->
    <velocity max_speed="0.05" priority="1"/>
    
    <!-- Start when participant begins moving -->
    <velocity min_speed="0.1" priority="1"/>
</start_conditions>
```

**Parameters**:
- `max_speed`: Maximum allowed velocity (m/s)
- `min_speed`: Minimum required velocity (m/s)
- `duration`: Time velocity condition must be maintained (seconds)

**Use cases**:
- Ensure participant is ready (stationary)
- Detect movement initiation
- Control movement onset timing

#### Button Press Start

```xml
<start_conditions>
    <!-- Start when participant presses haptic device button -->
    <button_press button_id="0" priority="1"/>
    
    <!-- Start when operator presses keyboard key -->
    <key_press key="space" priority="1"/>
</start_conditions>
```

**Parameters**:
- `button_id`: Haptic device button number (0-2)
- `key`: Keyboard key name ("space", "enter", "a", "1", etc.)

**Use cases**:
- Participant-controlled start timing
- Operator-controlled trial initiation
- Synchronized start with external events

### Advanced Start Conditions

#### Compound Conditions

```xml
<start_conditions>
    <!-- Start when BOTH position and velocity conditions are met -->
    <and priority="1">
        <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01"/>
        <velocity max_speed="0.02" duration="1.0"/>
    </and>
</start_conditions>

<start_conditions>
    <!-- Start when EITHER button is pressed OR time expires -->
    <or priority="1">
        <button_press button_id="0"/>
        <time duration_seconds="10.0"/>
    </or>
</start_conditions>
```

#### Conditional Start with Delay

```xml
<start_conditions>
    <!-- Wait for position, then delay 2 seconds before starting -->
    <sequence priority="1">
        <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01"/>
        <time duration_seconds="2.0"/>
    </sequence>
</start_conditions>
```

## End Conditions

End conditions determine when the trial stops and whether it was successful or not.

### Success Conditions

#### Position Target Reaching

```xml
<end_conditions>
    <!-- Reach target position -->
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Reach object position -->
    <position target_id="sphere_target" tolerance="0.025" priority="1" result="success"/>
    
    <!-- Reach and hold position -->
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02" hold_duration="2.0" priority="1" result="success"/>
</end_conditions>
```

**Parameters**:
- `target_x`, `target_y`, `target_z`: Target coordinates (meters)
- `target_id`: Reference to object defined in `<objects>` section
- `tolerance`: Required accuracy (meters)
- `hold_duration`: Time to maintain position (seconds, optional)
- `result`: "success" or "failure" (default: "success")

**Use cases**:
- Reaching tasks
- Targeting accuracy measurement
- Position maintenance tasks

#### Force Application

```xml
<end_conditions>
    <!-- Apply minimum force -->
    <force_threshold force_newtons="5.0" duration_seconds="1.0" priority="1" result="success"/>
    
    <!-- Apply force in specific direction -->
    <force_direction target_x="1.0" target_y="0.0" target_z="0.0" force_newtons="3.0" tolerance="0.2" priority="1" result="success"/>
</end_conditions>
```

**Parameters**:
- `force_newtons`: Required force magnitude (Newtons)
- `duration_seconds`: Time to maintain force (seconds)
- `target_x`, `target_y`, `target_z`: Required force direction (unit vector)
- `tolerance`: Directional accuracy (0.0 = exact, 1.0 = any direction)

**Use cases**:
- Force production tasks
- Strength measurement
- Directional force control

#### Movement Pattern Recognition

```xml
<end_conditions>
    <!-- Complete circular movement -->
    <path_pattern type="circle" radius="0.05" center_x="0.1" center_y="0.0" center_z="0.0" completion="0.9" priority="1" result="success"/>
    
    <!-- Trace specific trajectory -->
    <path_match reference_file="target_trajectory.csv" tolerance="0.01" completion="0.8" priority="1" result="success"/>
</end_conditions>
```

**Parameters**:
- `type`: Pattern type ("circle", "line", "spiral", "figure8")
- `completion`: Fraction of pattern that must be completed (0.0-1.0)
- `tolerance`: Maximum deviation from ideal pattern (meters)
- `reference_file`: CSV file with target trajectory coordinates

**Use cases**:
- Drawing tasks
- Movement coordination assessment
- Trajectory following experiments

### Failure Conditions

#### Time Limits

```xml
<end_conditions>
    <!-- Success condition -->
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Failure condition - trial too long -->
    <time_limit duration_seconds="30.0" priority="2" result="failure"/>
</end_conditions>
```

**Use cases**:
- Prevent trials from running indefinitely
- Measure task completion time
- Standardize trial duration

#### Workspace Violations

```xml
<end_conditions>
    <!-- Success condition -->
    <position target_id="target" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Failure condition - left workspace -->
    <workspace_boundary min_x="-0.2" max_x="0.2" min_y="-0.2" max_y="0.2" min_z="-0.1" max_z="0.3" priority="2" result="failure"/>
</end_conditions>
```

**Parameters**:
- `min_x`, `max_x`: Workspace X boundaries (meters)
- `min_y`, `max_y`: Workspace Y boundaries (meters)
- `min_z`, `max_z`: Workspace Z boundaries (meters)

**Use cases**:
- Safety boundaries
- Task area constraints
- Prevent device damage

#### Error Conditions

```xml
<end_conditions>
    <!-- Success condition -->
    <position target_id="target" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Failure condition - excessive force -->
    <force_limit max_force="15.0" priority="2" result="failure"/>
    
    <!-- Failure condition - contact with obstacle -->
    <object_contact object_id="obstacle" priority="2" result="failure"/>
</end_conditions>
```

### Complex End Conditions

#### Sequential Tasks

```xml
<end_conditions>
    <!-- Must reach targets in order -->
    <sequence priority="1" result="success">
        <position target_id="target1" tolerance="0.02" hold_duration="1.0"/>
        <position target_id="target2" tolerance="0.02" hold_duration="1.0"/>
        <position target_id="target3" tolerance="0.02"/>
    </sequence>
    
    <!-- Overall time limit -->
    <time_limit duration_seconds="60.0" priority="2" result="failure"/>
</end_conditions>
```

**Sequence behavior**:
- Conditions must be met in order
- Next condition only becomes active after previous is completed
- Optional `hold_duration` requires maintaining condition
- Sequence fails if any step cannot be completed

#### Alternative Success Conditions

```xml
<end_conditions>
    <!-- Multiple ways to succeed -->
    <or priority="1" result="success">
        <position target_id="target1" tolerance="0.02"/>  <!-- Reach target 1 -->
        <position target_id="target2" tolerance="0.02"/>  <!-- OR reach target 2 -->
        <position target_id="target3" tolerance="0.02"/>  <!-- OR reach target 3 -->
    </or>
    
    <!-- Time limit applies to all -->
    <time_limit duration_seconds="45.0" priority="2" result="failure"/>
</end_conditions>
```

#### Conditional Logic

```xml
<end_conditions>
    <!-- Success requires both position AND force -->
    <and priority="1" result="success">
        <position target_id="target" tolerance="0.02"/>
        <force_threshold force_newtons="2.0" duration_seconds="1.0"/>
    </and>
    
    <!-- Failure if force too high -->
    <force_limit max_force="10.0" priority="2" result="failure"/>
</end_conditions>
```

## Priority System

Conditions are evaluated in priority order (lower numbers first):

```xml
<end_conditions>
    <!-- Checked first -->
    <position target_id="target" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Checked second -->
    <force_limit max_force="15.0" priority="2" result="failure"/>
    
    <!-- Checked third -->
    <time_limit duration_seconds="30.0" priority="3" result="failure"/>
</end_conditions>
```

**Priority guidelines**:
- **Priority 1**: Primary success conditions
- **Priority 2**: Safety and error conditions
- **Priority 3**: Timeout and backup conditions

**Evaluation rules**:
1. Conditions evaluated in priority order
2. First condition to trigger determines trial outcome
3. Lower priority conditions ignored once higher priority condition triggers
4. Multiple conditions with same priority evaluated simultaneously

## Condition Parameters Reference

### Common Parameters

| Parameter | Type | Units | Description |
|-----------|------|-------|-------------|
| `priority` | integer | - | Evaluation order (1 = highest priority) |
| `result` | string | - | "success" or "failure" |
| `tolerance` | float | meters | Position accuracy requirement |
| `duration_seconds` | float | seconds | Time-based parameter |
| `hold_duration` | float | seconds | Time to maintain condition |

### Position Condition Parameters

| Parameter | Type | Units | Description |
|-----------|------|-------|-------------|
| `target_x` | float | meters | X coordinate target |
| `target_y` | float | meters | Y coordinate target |
| `target_z` | float | meters | Z coordinate target |
| `target_id` | string | - | Reference to object in scene |
| `tolerance` | float | meters | Required accuracy |
| `hold_duration` | float | seconds | Time to maintain position |

### Force Condition Parameters

| Parameter | Type | Units | Description |
|-----------|------|-------|-------------|
| `force_newtons` | float | Newtons | Required force magnitude |
| `max_force` | float | Newtons | Maximum allowed force |
| `duration_seconds` | float | seconds | Time to maintain force |
| `target_x`, `target_y`, `target_z` | float | - | Force direction (unit vector) |

### Velocity Condition Parameters

| Parameter | Type | Units | Description |
|-----------|------|-------|-------------|
| `max_speed` | float | m/s | Maximum allowed speed |
| `min_speed` | float | m/s | Minimum required speed |
| `duration` | float | seconds | Time to maintain velocity condition |

## Best Practices

### Design Principles

**Clear success criteria**:
```xml
<end_conditions>
    <!-- Clear, specific success condition -->
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Reasonable time limit -->
    <time_limit duration_seconds="30.0" priority="2" result="failure"/>
</end_conditions>
```

**Appropriate tolerances**:
- **Fine motor tasks**: 0.005-0.01 m tolerance
- **Gross motor tasks**: 0.02-0.05 m tolerance
- **Exploratory tasks**: 0.03-0.10 m tolerance

**Reasonable time limits**:
```xml
<!-- Based on pilot testing -->
<time_limit duration_seconds="45.0" priority="2" result="failure"/>  <!-- 95th percentile completion time + buffer -->
```

### Safety Considerations

**Always include safety conditions**:
```xml
<end_conditions>
    <!-- Primary success condition -->
    <position target_id="target" tolerance="0.02" priority="1" result="success"/>
    
    <!-- Safety conditions -->
    <force_limit max_force="20.0" priority="2" result="failure"/>
    <workspace_boundary min_x="-0.25" max_x="0.25" min_y="-0.25" max_y="0.25" min_z="-0.05" max_z="0.35" priority="2" result="failure"/>
    
    <!-- Time limit -->
    <time_limit duration_seconds="60.0" priority="3" result="failure"/>
</end_conditions>
```

### Testing and Validation

**Test conditions with pilot data**:
1. Run pilot trials with generous tolerances
2. Analyze completion rates and times
3. Adjust tolerances for target difficulty
4. Validate conditions work as expected

**Document condition rationale**:
```xml
<end_conditions>
    <!-- Based on pilot study: 80% success rate with 0.02m tolerance -->
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02" priority="1" result="success"/>
    
    <!-- 95th percentile completion time was 28s, added 10s buffer -->
    <time_limit duration_seconds="38.0" priority="2" result="failure"/>
</end_conditions>
```

### Common Patterns

**Simple reaching task**:
```xml
<start_conditions>
    <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01" priority="1"/>
</start_conditions>

<end_conditions>
    <position target_x="0.15" target_y="0.0" target_z="0.0" tolerance="0.025" priority="1" result="success"/>
    <time_limit duration_seconds="30.0" priority="2" result="failure"/>
</end_conditions>
```

**Force production task**:
```xml
<start_conditions>
    <position target_id="force_target" tolerance="0.01" priority="1"/>
</start_conditions>

<end_conditions>
    <force_threshold force_newtons="8.0" duration_seconds="2.0" priority="1" result="success"/>
    <force_limit max_force="25.0" priority="2" result="failure"/>
    <time_limit duration_seconds="20.0" priority="3" result="failure"/>
</end_conditions>
```

**Multi-target sequence**:
```xml
<start_conditions>
    <time duration_seconds="0.0" priority="1"/>
</start_conditions>

<end_conditions>
    <sequence priority="1" result="success">
        <position target_id="target1" tolerance="0.02"/>
        <position target_id="target2" tolerance="0.02"/>
        <position target_id="target3" tolerance="0.02"/>
    </sequence>
    <time_limit duration_seconds="90.0" priority="2" result="failure"/>
</end_conditions>
```

This comprehensive guide covers all aspects of trial condition configuration. The conditions system provides precise control over experimental timing and success criteria, enabling sophisticated research protocols while maintaining safety and data quality.