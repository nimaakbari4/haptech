# gRPC API

Comprehensive documentation of the FrankaTrialService gRPC API, including request/response formats and usage examples.

## Service Definition

```protobuf
syntax = "proto3";

service FrankaTrialService {
    rpc RunTrial(TrialRequest) returns (stream TrialResponseStream);
}
```

**Endpoint**: Single RPC method for running experimental trials  
**Pattern**: Request/streaming-response (client sends one request, server streams back multiple responses)  
**Protocol**: gRPC over HTTP/2  
**Default Port**: 50051

## RunTrial RPC

### Method Signature

```protobuf
rpc RunTrial(TrialRequest) returns (stream TrialResponseStream);
```

**Purpose**: Configure scene, run experimental trial, and stream data back to client

**Request**: `TrialRequest` - Complete trial configuration  
**Response**: `stream TrialResponseStream` - Status updates and data points

### Request Flow

1. Client sends `TrialRequest` with full configuration
2. Server validates request
3. Server configures scene (objects, camera, conditions)
4. Server sends `STARTED` status update
5. Server enables haptics and begins simulation
6. Server streams `TrialDataPoint` messages
7. Server monitors end conditions
8. Server sends `COMPLETED` or `ERROR` status update
9. Stream closes

### Response Stream Format

```protobuf
message TrialResponseStream {
    oneof payload {
        TrialStatusUpdate statusUpdate = 1;
        TrialDataPoint dataPoint = 2;
    }
}
```

**Stream Contents**:
- **Status Updates**: Trial lifecycle events (started, error, completed)
- **Data Points**: Continuous sensor/state data during trial

## TrialRequest

### Message Structure

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

### Fields

#### trial_id (string)

**Purpose**: Unique identifier for this trial  
**Required**: Yes (should be non-empty)  
**Format**: Any string (e.g., "trial_001", "test_2024-01-15_001")

**Example**:
```python
request.trial_id = "experiment_alpha_trial_042"
```

#### objects (repeated Object)

**Purpose**: Objects to place in the scene  
**Required**: No (can have empty scene)  
**See**: [Object Message](#object-message)

**Example**:
```python
obj = request.objects.add()
obj.id = "target_sphere"
obj.type = "sphere"
obj.position.x = 0.0
obj.position.y = 0.0
obj.position.z = 0.0
obj.color.r = 1.0
obj.color.g = 0.0
obj.color.b = 0.0
obj.params.add(key="radius", value="0.05")
```

#### startConditions (repeated TrialCondition)

**Purpose**: Conditions that must be met to begin data collection  
**Required**: No (trial starts immediately if empty)  
**See**: [TrialCondition Message](#trialcondition-message)

**Example**:
```python
# Start when user enters target region
condition = request.startConditions.add()
condition.name = "position"
condition.params.add(key="target_x", value="0.0")
condition.params.add(key="target_y", value="0.0")
condition.params.add(key="target_z", value="0.1")
condition.params.add(key="tolerance", value="0.02")
```

#### endConditions (repeated TrialCondition)

**Purpose**: Conditions that signal trial completion  
**Required**: Yes (at least one, otherwise trial never ends)  
**See**: [TrialCondition Message](#trialcondition-message)

**Example**:
```python
# End after 10 seconds
condition = request.endConditions.add()
condition.name = "time"
condition.params.add(key="duration", value="10.0")
```

#### sceneSettings (SceneSettings)

**Purpose**: Configure visual environment (camera, background, overlays)  
**Required**: No (uses defaults if omitted)  
**See**: [SceneSettings Message](#scenesettings-message)

**Example**:
```python
request.sceneSettings.backgroundColor.r = 0.1
request.sceneSettings.backgroundColor.g = 0.1
request.sceneSettings.backgroundColor.b = 0.1

request.sceneSettings.camera.position.x = 1.0
request.sceneSettings.camera.position.y = 0.0
request.sceneSettings.camera.position.z = 0.0
request.sceneSettings.camera.lookAt.x = 0.0
request.sceneSettings.camera.lookAt.y = 0.0
request.sceneSettings.camera.lookAt.z = 0.0
request.sceneSettings.camera.up.z = 1.0
```

#### dataCollection (DataCollectionConfig)

**Purpose**: Specify which data fields to include in stream  
**Required**: No (all fields included by default)  
**See**: [DataCollectionConfig Message](#datacollectionconfig-message)

**Example**:
```python
# Collect only essential data
request.dataCollection.device.position = True
request.dataCollection.device.force = True
request.dataCollection.objects.position = True
```

## TrialResponseStream

### Message Structure

```protobuf
message TrialResponseStream {
    oneof payload {
        TrialStatusUpdate statusUpdate = 1;
        TrialDataPoint dataPoint = 2;
    }
}
```

**Usage**: Check which field is set using `HasField` (C++) or `WhichOneof` (Python)

### Handling Responses

```python
for response in stub.RunTrial(request):
    if response.HasField('statusUpdate'):
        # Handle status update
        status = response.statusUpdate
        print(f"{status.status}: {status.message}")
        
    elif response.HasField('dataPoint'):
        # Handle data point
        data = response.dataPoint
        # Write to CSV, plot, etc.
```

## TrialStatusUpdate

### Message Structure

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

### Status Types

#### STARTED

**When**: Scene successfully configured, haptics enabled, data collection beginning  
**Message**: Usually "Trial started"

**Example**:
```json
{
  "statusUpdate": {
    "status": "STARTED",
    "message": "Trial started: experiment_001"
  }
}
```

#### ERROR

**When**: Configuration failed, validation error, or trial already running  
**Message**: Describes the error

**Common Errors**:
- "There is already a trial running."
- "Simulation is not running yet."
- "Failed to create object: unknown_type"
- "Invalid camera parameters"

**Example**:
```json
{
  "statusUpdate": {
    "status": "ERROR",
    "message": "Object type 'invalid_shape' not registered"
  }
}
```

#### COMPLETED

**When**: End condition satisfied, data collection finished  
**Message**: Usually includes end condition details

**Example**:
```json
{
  "statusUpdate": {
    "status": "COMPLETED",
    "message": "Trial completed: time limit reached"
  }
}
```

## TrialDataPoint

### Message Structure (Essential Fields)

```protobuf
message TrialDataPoint {
    double timestamp = 1;
    Vector3 devicePosition = 2;
    Vector3 proxyPosition = 3;
    Vector3 force = 4;
    Vector3 deviceVelocity = 5;
    Vector3 deviceAngularVelocity = 6;
    Quaternion deviceOrientation = 7;
    repeated ObjectState objects = 8;
    // ... many optional fields ...
}
```

**See**: [data-formats.md](data-formats.md) for complete field reference

### Key Fields

#### timestamp (double)

**Purpose**: Time since trial start (seconds)  
**Range**: 0.0 to trial duration  
**Precision**: ~0.001s (1ms)

#### devicePosition (Vector3)

**Purpose**: Haptic device position in world coordinates  
**Units**: Meters  
**Frame**: World/scene frame

#### proxyPosition (Vector3)

**Purpose**: Haptic proxy (constrained cursor) position  
**Units**: Meters  
**Note**: May differ from devicePosition when touching objects

#### force (Vector3)

**Purpose**: Rendered force applied to device  
**Units**: Newtons  
**Note**: Spring-damper force between device and proxy

#### objects (repeated ObjectState)

**Purpose**: State of all objects in scene at this timestamp  
**See**: [ObjectState Message](#objectstate-message)

## Supporting Messages

### Object Message

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

**Fields**:
- `id`: Unique identifier (e.g., "sphere_1")
- `type`: Object type from registry (e.g., "sphere", "box", "ball")
- `position`: World position (meters)
- `orientation`: Euler angles (degrees) - XYZ order
- `color`: RGB color (0.0 to 1.0)
- `params`: Key-value parameters (e.g., radius, mass, stiffness)

**Example**:
```python
obj = Object()
obj.id = "bouncy_ball"
obj.type = "ball"
obj.position.x = 0.0
obj.position.y = 0.0
obj.position.z = 0.1
obj.color.r = 1.0
obj.color.g = 0.5
obj.color.b = 0.0
obj.params.add(key="radius", value="0.03")
obj.params.add(key="bounciness", value="0.8")
obj.params.add(key="mass", value="0.1")
```

### TrialCondition Message

```protobuf
message TrialCondition {
    string name = 1;
    bool success = 2;
    int32 priority = 3;
    repeated ObjectParameter params = 4;
}
```

**Fields**:
- `name`: Condition type (e.g., "time", "position", "velocity")
- `success`: Expected value (true = success condition, false = failure condition)
- `priority`: Order to check (lower = higher priority)
- `params`: Condition-specific parameters

**Common Condition Types**:
- `time`: Duration-based (param: `duration`)
- `position`: Target region (params: `target_x/y/z`, `tolerance`)
- `velocity`: Speed threshold (params: `max_velocity`, `min_velocity`)
- `contact`: Object touch (params: `object_id`)

**Example - Time Condition**:
```python
condition = TrialCondition()
condition.name = "time"
condition.success = True
condition.priority = 1
condition.params.add(key="duration", value="10.0")
```

**Example - Position Condition**:
```python
condition = TrialCondition()
condition.name = "position"
condition.success = True
condition.priority = 1
condition.params.add(key="target_x", value="0.0")
condition.params.add(key="target_y", value="0.0")
condition.params.add(key="target_z", value="0.1")
condition.params.add(key="tolerance", value="0.02")
```

### SceneSettings Message

```protobuf
message SceneSettings {
    Color backgroundColor = 1;
    Camera camera = 2;
    repeated TextOverlay textOverlays = 3;
}
```

**Fields**:
- `backgroundColor`: RGB color (0.0 to 1.0)
- `camera`: Camera position and orientation
- `textOverlays`: On-screen text labels

**Example**:
```python
settings = SceneSettings()

# Dark background
settings.backgroundColor.r = 0.1
settings.backgroundColor.g = 0.1
settings.backgroundColor.b = 0.1

# Camera 1 meter away, looking at origin
settings.camera.position.x = 1.0
settings.camera.position.y = 0.0
settings.camera.position.z = 0.0
settings.camera.lookAt.x = 0.0
settings.camera.lookAt.y = 0.0
settings.camera.lookAt.z = 0.0
settings.camera.up.x = 0.0
settings.camera.up.y = 0.0
settings.camera.up.z = 1.0

# Add instruction text
overlay = settings.textOverlays.add()
overlay.text = "Move to the target sphere"
overlay.x = 10
overlay.y = 50
overlay.fontSize = 24
overlay.color.r = 1.0
overlay.color.g = 1.0
overlay.color.b = 1.0
```

### Camera Message

```protobuf
message Camera {
    Vector3 position = 1;
    Vector3 lookAt = 2;
    Vector3 up = 3;
}
```

**Fields**:
- `position`: Camera location (world coordinates)
- `lookAt`: Point camera is facing
- `up`: Up direction vector (usually [0, 0, 1])

**Defaults**:
- `position`: [1.0, 0.0, 0.0]
- `lookAt`: [0.0, 0.0, 0.0]
- `up`: [0.0, 0.0, 1.0]

### TextOverlay Message

```protobuf
message TextOverlay {
    string text = 1;
    double x = 2;           // pixels from left
    double y = 3;           // pixels from top
    double fontSize = 4;    // points (20, 32, 40)
    Color color = 5;        // RGB
}
```

**Coordinate System**: Screen space (0,0 = top-left corner)

**Font Sizes**: Limited to CHAI3D preset sizes (20, 32, 40 points)

**Example**:
```python
overlay = TextOverlay()
overlay.text = "Trial 5 / 10"
overlay.x = 10      # 10 pixels from left
overlay.y = 100     # 100 pixels from top
overlay.fontSize = 32
overlay.color.r = 0.0
overlay.color.g = 1.0
overlay.color.b = 0.0
```

### DataCollectionConfig Message

```protobuf
message DataCollectionConfig {
    DeviceDataConfig device = 1;
    ObjectDataConfig objects = 2;
}

message DeviceDataConfig {
    bool position = 1;
    bool velocity = 2;
    bool angular_velocity = 3;
    bool orientation = 4;
    bool proxy_position = 5;
    bool force = 6;
}

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

**Purpose**: Reduce bandwidth by excluding unnecessary fields

**Default**: All fields enabled (if omitted)

**Example - Minimal Data**:
```python
config = DataCollectionConfig()
config.device.position = True
config.device.force = True
config.objects.position = True
# All other fields implicitly False
```

### ObjectState Message

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

**Purpose**: Runtime state of objects in scene (included in TrialDataPoint)

**Fields**:
- `id`: Object identifier
- `position`: Current world position
- `orientation`: Current rotation (quaternion)
- `type`: Object type string
- `color`: Current color (may change during trial)
- `active`: Whether object is enabled
- `collisionEnabled`: Whether collision detection is active

## Error Handling

### Common Errors

#### Trial Already Running

```
Status: ERROR
Message: "There is already a trial running."
```

**Cause**: Previous trial hasn't finished  
**Solution**: Wait for previous trial to complete or restart server

#### Invalid Object Type

```
Status: ERROR
Message: "Unknown object type: invalid_shape"
```

**Cause**: Object type not registered  
**Solution**: Check available types in `extensions_registry.cpp`

#### Missing End Condition

**Symptom**: Trial never completes, stream stays open indefinitely  
**Cause**: No end condition specified or condition never satisfied  
**Solution**: Always add at least one end condition (e.g., time limit)

## Performance Tips

1. **Reduce Data Rate**: Use `DataCollectionConfig` to exclude unnecessary fields
2. **Batch Processing**: Buffer data points before writing to disk
3. **Compression**: Enable gRPC compression for large messages
4. **Connection Reuse**: Keep channel open for multiple trials

## See Also

- [Data Formats](data-formats.md) - Complete message reference
- [Protocols README](README.md) - Protocol overview
- [Franka Trial Server](../core/franka-trial-server.md) - Server implementation
- [Python Client Guide](../../operator-guide/python-client/README.md) - Client usage