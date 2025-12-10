# Data Formats

Comprehensive reference for all Protocol Buffer message structures used in the Haptech system.

## Overview

This document provides detailed specifications for all protobuf messages. Messages are organized by category:

- **Service Messages**: Request and response structures
- **Data Collection**: Real-time sensor/state data
- **Configuration**: Scene and experiment setup
- **Primitives**: Basic data types (vectors, quaternions, etc.)

**Source**: `proto/franka_trial.proto`

## Service Messages

### TrialRequest

Complete trial configuration sent from client to server.

```protobuf
message TrialRequest {
    string trial_id = 1;
    repeated Object objects = 2;
    repeated TrialCondition startConditions = 3;
    repeated TrialCondition endConditions = 4;
    SceneSettings sceneSettings = 5;
    DataCollectionConfig dataCollection = 6;
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `trial_id` | string | Yes | Unique identifier for this trial |
| `objects` | Object[] | Yes | Objects to add to scene |
| `startConditions` | TrialCondition[] | Yes | Conditions to begin data collection |
| `endConditions` | TrialCondition[] | Yes | Conditions to end trial |
| `sceneSettings` | SceneSettings | Yes | Visual environment configuration |
| `dataCollection` | DataCollectionConfig | No | Data field selection |

### TrialResponseStream

Polymorphic response message (either status or data).

```protobuf
message TrialResponseStream {
    oneof payload {
        TrialStatusUpdate statusUpdate = 1;
        TrialDataPoint dataPoint = 2;
    }
}
```

**Usage**: Check which field is set using `HasField()` or `WhichOneof()`

**Python Example**:
```python
if response.HasField('statusUpdate'):
    # Handle status
elif response.HasField('dataPoint'):
    # Handle data
```

### TrialStatusUpdate

Lifecycle status messages.

```protobuf
message TrialStatusUpdate {
    TrialStatus status = 1;
    string message = 2;
}

enum TrialStatus {
    STARTED = 0;
    ERROR = 1;
    COMPLETED = 2;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `status` | TrialStatus | Event type (STARTED/ERROR/COMPLETED) |
| `message` | string | Human-readable description |

**Status Values**:
- `STARTED`: Trial configured successfully, data collection beginning
- `ERROR`: Configuration failed or runtime error occurred
- `COMPLETED`: End condition satisfied, trial finished

## Data Collection Messages

### TrialDataPoint

Primary data structure streamed at ~100 Hz during trial execution.

```protobuf
message TrialDataPoint {
    // Core fields (always present)
    double timestamp = 1;
    Vector3 devicePosition = 2;
    Vector3 proxyPosition = 3;
    Vector3 force = 4;
    
    // Device kinematics
    Vector3 deviceVelocity = 5;
    Vector3 deviceAngularVelocity = 6;
    Quaternion deviceOrientation = 7;
    
    // Scene state
    repeated ObjectState objects = 8;
    
    // Extended data (optional)
    Vector3 deviceAcceleration = 9;
    Vector3 measuredForce = 10;
    uint64 messageIndex = 11;
    double wallTimestamp = 12;
    
    // Robot kinematics (if using robot)
    repeated double robotQ = 13;
    repeated double robotDQ = 14;
    repeated double robotTau = 15;
    Pose eePose = 16;
    Twist eeVelocity = 17;
    
    // Force breakdown
    Vector3 springForce = 18;
    Vector3 dampingForce = 19;
    bool forceSaturated = 20;
    bool maxForceReached = 21;
    
    // Collision info
    repeated Contact contacts = 22;
    
    // State information
    TrialState trial = 23;
    HapticsState haptics = 24;
    HomingInfo homing = 25;
    
    // Diagnostics
    string diagnostics = 26;
    repeated string reflexFlags = 27;
    int32 droppedFrames = 28;
    int32 queueDepth = 29;
    string cpuAffinity = 30;
    string threadPriority = 31;
    uint64 loopOverrunCount = 32;
    double latencyComputeMs = 33;
    double latencyApplyMs = 34;
    double latencySendMs = 35;
    
    // Unit metadata
    string coordinateFrame = 36;
    string unitsLinear = 37;
    string unitsForce = 38;
    string unitsTime = 39;
}
```

#### Core Fields

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `timestamp` | double | seconds | Time since trial start |
| `devicePosition` | Vector3 | meters | Haptic device position (world frame) |
| `proxyPosition` | Vector3 | meters | Haptic proxy position (constrained) |
| `force` | Vector3 | Newtons | Rendered force (applied to device) |

#### Device Kinematics

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `deviceVelocity` | Vector3 | m/s | Linear velocity |
| `deviceAngularVelocity` | Vector3 | rad/s | Rotational velocity |
| `deviceOrientation` | Quaternion | - | Device orientation (x,y,z,w) |

#### Extended Data

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `deviceAcceleration` | Vector3 | m/s² | Linear acceleration |
| `measuredForce` | Vector3 | N | Actual force measured by device |
| `messageIndex` | uint64 | - | Monotonic message counter |
| `wallTimestamp` | double | seconds | Unix epoch timestamp |

#### Robot Kinematics (Franka Specific)

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `robotQ` | double[7] | radians | Joint positions |
| `robotDQ` | double[7] | rad/s | Joint velocities |
| `robotTau` | double[7] | N·m | Joint torques |
| `eePose` | Pose | - | End-effector pose |
| `eeVelocity` | Twist | - | End-effector velocity |

#### Force Breakdown

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `springForce` | Vector3 | N | Spring component of force |
| `dampingForce` | Vector3 | N | Damping component of force |
| `forceSaturated` | bool | - | Whether force hit device limit |
| `maxForceReached` | bool | - | Whether max force was reached |

#### State Information

| Field | Type | Description |
|-------|------|-------------|
| `objects` | ObjectState[] | State of all scene objects |
| `contacts` | Contact[] | Active collision contacts |
| `trial` | TrialState | Trial progress information |
| `haptics` | HapticsState | Haptics system state |
| `homing` | HomingInfo | Homing guidance state |

#### Diagnostics

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `diagnostics` | string | - | Diagnostic messages |
| `reflexFlags` | string[] | - | Safety reflex triggers |
| `droppedFrames` | int32 | - | Dropped haptics cycles |
| `queueDepth` | int32 | - | Message queue depth |
| `cpuAffinity` | string | - | CPU core assignment |
| `threadPriority` | string | - | Thread priority level |
| `loopOverrunCount` | uint64 | - | Haptics loop overruns |
| `latencyComputeMs` | double | ms | Force computation latency |
| `latencyApplyMs` | double | ms | Force application latency |
| `latencySendMs` | double | ms | Message send latency |

#### Unit Metadata

| Field | Type | Example | Description |
|-------|------|---------|-------------|
| `coordinateFrame` | string | "world" | Reference frame name |
| `unitsLinear` | string | "m" | Linear unit abbreviation |
| `unitsForce` | string | "N" | Force unit abbreviation |
| `unitsTime` | string | "s" | Time unit abbreviation |

### ObjectState

Runtime state of an object in the scene.

```protobuf
message ObjectState {
    string id = 1;
    Vector3 position = 2;
    Quaternion orientation = 3;
    string type = 4;
    Color color = 5;
    bool active = 6;
    bool collisionEnabled = 7;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `id` | string | - | Object identifier |
| `position` | Vector3 | meters | World position |
| `orientation` | Quaternion | - | World rotation (x,y,z,w) |
| `type` | string | - | Object type name |
| `color` | Color | - | RGB color |
| `active` | bool | - | Whether object is enabled |
| `collisionEnabled` | bool | - | Whether collision detection active |

### Contact

Collision contact information.

```protobuf
message Contact {
    string objectId = 1;
    Vector3 point = 2;
    Vector3 normal = 3;
    double penetrationDepth = 4;
    string frictionState = 5;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `objectId` | string | - | Object involved in contact |
| `point` | Vector3 | meters | Contact point (world frame) |
| `normal` | Vector3 | - | Contact normal (unit vector) |
| `penetrationDepth` | double | meters | How deep proxy penetrated |
| `frictionState` | string | - | Friction mode ("static"/"kinetic") |

### TrialState

Trial progress information.

```protobuf
message TrialState {
    string trialId = 1;
    double elapsedTime = 2;
    bool trialRunning = 3;
    bool reachedGoal = 4;
    double distanceToGoal = 5;
    double goalTolerance = 6;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `trialId` | string | - | Current trial identifier |
| `elapsedTime` | double | seconds | Time since trial start |
| `trialRunning` | bool | - | Whether trial is active |
| `reachedGoal` | bool | - | Whether goal reached |
| `distanceToGoal` | double | meters | Distance to goal position |
| `goalTolerance` | double | meters | Goal tolerance radius |

### HapticsState

Haptics system state.

```protobuf
message HapticsState {
    bool hapticsEnabled = 1;
    double workspaceScaleFactor = 2;
    double toolRadius = 3;
    bool waitForSmallForce = 4;
    double loopFrequencyHz = 5;
    double dt = 6;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `hapticsEnabled` | bool | - | Whether haptics loop running |
| `workspaceScaleFactor` | double | - | Virtual/physical workspace ratio |
| `toolRadius` | double | meters | Haptic proxy radius |
| `waitForSmallForce` | bool | - | Waiting for small force before enabling |
| `loopFrequencyHz` | double | Hz | Actual haptics loop frequency |
| `dt` | double | seconds | Haptics loop time step |

### HomingInfo

Homing guidance state (if active).

```protobuf
message HomingInfo {
    bool homingActive = 1;
    Vector3 target = 2;
    double tolerance = 3;
    double elapsed = 4;
    double springConstant = 5;
    double dampingConstant = 6;
    double maxForce = 7;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `homingActive` | bool | - | Whether homing is active |
| `target` | Vector3 | meters | Target position |
| `tolerance` | double | meters | Distance tolerance |
| `elapsed` | double | seconds | Time since homing started |
| `springConstant` | double | N/m | Homing spring stiffness |
| `dampingConstant` | double | N·s/m | Homing damping |
| `maxForce` | double | N | Maximum homing force |

## Configuration Messages

### Object

Object configuration for scene setup.

```protobuf
message Object {
    string id = 1;
    string type = 2;
    Vector3 position = 3;
    Vector3 orientation = 4;  // roll, pitch, yaw in degrees
    Color color = 5;
    repeated ObjectParameter params = 6;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `id` | string | - | Unique object identifier |
| `type` | string | - | Object type (e.g., "sphere", "box", "ball") |
| `position` | Vector3 | meters | Initial world position |
| `orientation` | Vector3 | degrees | Euler angles (roll, pitch, yaw) |
| `color` | Color | - | RGB color (0.0-1.0) |
| `params` | ObjectParameter[] | - | Type-specific parameters |

**Common Object Types**:
- `sphere` - Basic sphere (params: `radius`)
- `box` - Rectangular box (params: `size_x`, `size_y`, `size_z`)
- `ball` - Bouncing ball (params: `radius`, `bounciness`, `mass`)
- `spring` - Spring-damper (params: `spring_constant`, `damping`)
- `ellipsoid` - Ellipsoid shape (params: `radius_x`, `radius_y`, `radius_z`)
- `cup` - Cup/bowl shape (params: `radius`, `height`, `thickness`)
- `line` - Line segment (params: `length`, `thickness`)

### ObjectParameter

Key-value parameter for object configuration.

```protobuf
message ObjectParameter {
    string key = 1;
    string value = 2;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `key` | string | Parameter name |
| `value` | string | Parameter value (parsed by object) |

**Example**:
```python
param = ObjectParameter()
param.key = "radius"
param.value = "0.05"  # 5cm radius
```

### TrialCondition

Condition for trial start/end.

```protobuf
message TrialCondition {
    string name = 1;
    bool success = 2;
    int32 priority = 3;
    repeated ObjectParameter params = 4;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Condition type |
| `success` | bool | Expected outcome (true=success, false=failure) |
| `priority` | int32 | Evaluation order (lower = higher priority) |
| `params` | ObjectParameter[] | Condition-specific parameters |

**Built-in Condition Types**:

#### time
End after specified duration.
- `duration` (double): Time limit in seconds

#### position
Reach target position.
- `target_x`, `target_y`, `target_z` (double): Target position (meters)
- `tolerance` (double): Distance tolerance (meters)

#### velocity
Speed threshold.
- `max_velocity` (double): Maximum speed (m/s)
- `min_velocity` (double): Minimum speed (m/s)

#### contact
Touch specific object.
- `object_id` (string): ID of object to touch

### SceneSettings

Visual environment configuration.

```protobuf
message SceneSettings {
    Color backgroundColor = 1;
    Camera camera = 2;
    repeated TextOverlay textOverlays = 3;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `backgroundColor` | Color | Scene background color |
| `camera` | Camera | Camera position and orientation |
| `textOverlays` | TextOverlay[] | On-screen text labels |

### Camera

Camera configuration.

```protobuf
message Camera {
    Vector3 position = 1;
    Vector3 lookAt = 2;
    Vector3 up = 3;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `position` | Vector3 | meters | Camera location (world frame) |
| `lookAt` | Vector3 | meters | Point camera is facing |
| `up` | Vector3 | - | Up direction (unit vector) |

**Default**:
- `position`: [1.0, 0.0, 0.0]
- `lookAt`: [0.0, 0.0, 0.0]
- `up`: [0.0, 0.0, 1.0]

### TextOverlay

On-screen text display.

```protobuf
message TextOverlay {
    string text = 1;
    double x = 2;
    double y = 3;
    double fontSize = 4;
    Color color = 5;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `text` | string | - | Text content |
| `x` | double | pixels | X position (from left edge) |
| `y` | double | pixels | Y position (from top edge) |
| `fontSize` | double | points | Font size (20, 32, or 40) |
| `color` | Color | - | Text color (RGB 0.0-1.0) |

**Coordinate System**: Screen space (0,0 = top-left corner)

**Font Sizes**: Limited to CHAI3D presets: 20, 32, 40 points

### DataCollectionConfig

Data field selection for bandwidth optimization.

```protobuf
message DataCollectionConfig {
    DeviceDataConfig device = 1;
    ObjectDataConfig objects = 2;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `device` | DeviceDataConfig | Device data fields to include |
| `objects` | ObjectDataConfig | Object data fields to include |

### DeviceDataConfig

Device data field selection.

```protobuf
message DeviceDataConfig {
    bool position = 1;
    bool velocity = 2;
    bool angular_velocity = 3;
    bool orientation = 4;
    bool proxy_position = 5;
    bool force = 6;
}
```

**All fields default to false if not explicitly set.**

### ObjectDataConfig

Object data field selection.

```protobuf
message ObjectDataConfig {
    bool position = 1;
    bool orientation = 2;
    bool contact = 3;
    bool contact_point = 4;
    bool contact_normal = 5;
    bool penetration_depth = 6;
    bool force = 7;
    bool spring_force = 8;
    bool damping_force = 9;
}
```

**All fields default to false if not explicitly set.**

## Primitive Data Types

### Vector3

3D vector (position, velocity, force, etc.).

```protobuf
message Vector3 {
    double x = 1;
    double y = 2;
    double z = 3;
}
```

**Usage**:
```python
vec = Vector3()
vec.x = 1.0
vec.y = 0.0
vec.z = 0.5
```

**Common Interpretations**:
- **Position**: World coordinates (meters)
- **Velocity**: m/s
- **Force**: Newtons
- **Acceleration**: m/s²

### Quaternion

Orientation representation (x, y, z, w).

```protobuf
message Quaternion {
    double x = 1;
    double y = 2;
    double z = 3;
    double w = 4;
}
```

**Format**: Hamilton convention (w is scalar component)

**Identity** (no rotation):
```python
q = Quaternion()
q.x = 0.0
q.y = 0.0
q.z = 0.0
q.w = 1.0
```

**Normalization**: Should always satisfy: x² + y² + z² + w² = 1

### Color

RGB color (0.0 to 1.0 range).

```protobuf
message Color {
    double r = 1;
    double g = 2;
    double b = 3;
}
```

**Range**: [0.0, 1.0] for each component

**Common Colors**:
```python
# Red
color = Color(r=1.0, g=0.0, b=0.0)

# Green
color = Color(r=0.0, g=1.0, b=0.0)

# Blue
color = Color(r=0.0, g=0.0, b=1.0)

# White
color = Color(r=1.0, g=1.0, b=1.0)

# Black
color = Color(r=0.0, g=0.0, b=0.0)

# Gray (50%)
color = Color(r=0.5, g=0.5, b=0.5)
```

### Pose

Combined position and orientation.

```protobuf
message Pose {
    Vector3 position = 1;
    Quaternion orientation = 2;
}
```

**Usage**: Represent complete 6-DOF pose (position + rotation)

### Twist

Velocity representation (linear + angular).

```protobuf
message Twist {
    Vector3 linear = 1;
    Vector3 angular = 2;
}
```

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `linear` | Vector3 | m/s | Linear velocity |
| `angular` | Vector3 | rad/s | Angular velocity |

## Version Compatibility

### Adding New Fields

✅ **Safe**: Always backward compatible

```protobuf
message TrialDataPoint {
    double timestamp = 1;
    // ...existing fields...
    
    // NEW FIELD: Old clients ignore it
    double newMetric = 50;
}
```

### Removing Fields

✅ **Safe**: Mark as reserved

```protobuf
message TrialDataPoint {
    double timestamp = 1;
    // repeated OldType oldField = 2;  // REMOVED
    reserved 2;  // Prevent field number reuse
}
```

### Renaming Fields

✅ **Safe**: Field names don't affect serialization

```protobuf
message TrialDataPoint {
    double timestamp = 1;  // Was: time_since_start
}
```

### Changing Field Types

❌ **Unsafe**: Breaks compatibility

```protobuf
// DON'T DO THIS:
// int32 timeout = 5;
double timeout = 5;  // Type change breaks clients!
```

## See Also

- [gRPC API](grpc-api.md) - API usage and examples
- [Protocols README](README.md) - Protocol overview
- [Python Client Guide](../../operator-guide/python-client/README.md) - Client implementation