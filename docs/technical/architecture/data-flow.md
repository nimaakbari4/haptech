# Data Flow

Comprehensive documentation of data flow between system components.

## Overview

The Haptech system uses a client-server architecture with bidirectional streaming over gRPC. The C++ core server maintains real-time haptic rendering while streaming data to the Python client for logging and analysis.

## Communication Architecture

```
Python Client                     C++ Core Server
     │                                   │
     │  gRPC Connection                 │
     │  localhost:50051                 │
     │                                   │
     ├──── TrialRequest ──────────────►│
     │                                   │
     │                         [Parse config]
     │                         [Setup scene]
     │                         [Start trial]
     │                                   │
     │◄─── StatusUpdate(STARTED) ──────│
     │                                   │
     │                         [Haptics loop]
     │                         [Collect data]
     │                                   │
     │◄──── TrialDataPoint ───────────│
     │◄──── TrialDataPoint ───────────│
     │◄──── TrialDataPoint ───────────│
     │        ...                          │
     │                                   │
     │                    [End condition met]
     │                                   │
     │◄── StatusUpdate(COMPLETED) ─────│
     │                                   │
[Save CSV]                                │
[Close stream]                            │
```

## Trial Execution Flow

### 1. Trial Configuration Phase

```
Client                          Server
  │                               │
  ├─ Load XML config              │
  ├─ Parse objects/conditions     │
  ├─ Build TrialRequest           │
  │                               │
  ├──── TrialRequest ─────────►│
  │                               ├─ Validate request
  │                               ├─ Check simulation running
  │                               ├─ Check no trial in progress
  │                               │
  │◄─── ERROR if invalid ──────│
```

**TrialRequest Contents**:
- `trial_id` - Unique identifier
- `objects[]` - Scene objects configuration
- `startConditions[]` - When to start data collection
- `endConditions[]` - When to end trial
- `sceneSettings` - Camera, background, text overlays
- `dataCollection` - Sample rate configuration

### 2. Scene Setup Phase

```
Server Thread Flow:
  │
  ├─ Disable haptics
  ├─ Clear existing scene
  ├─ Remove all objects
  ├─ Clear tool state
  │
  ├─ Set background color
  ├─ Configure camera position/orientation
  ├─ Add text overlays
  │
  ├─ Create objects:
  │   │
  │   ├─ For each object in request:
  │   │   ├─ Lookup type in registry
  │   │   ├─ Create CHAI3D object
  │   │   ├─ Set position/rotation
  │   │   ├─ Configure material properties
  │   │   ├─ Add to world
  │   │   └─ Store in objectMap
  │   │
  │
  ├─ Create trial conditions:
  │   ├─ Start conditions
  │   └─ End conditions
  │
  ├─ Enable haptics gradually
  ├─ Mark trial as running
  │
  ├──── StatusUpdate(STARTED) ───► Client
```

### 3. Data Collection Phase

**Parallel Threads**:

```
Graphics Thread (60 Hz)          Haptics Thread (1000 Hz)        gRPC Thread
      │                                  │                         │
      ├─ Update physics                 ├─ Read device pos          │
      ├─ Render scene                   ├─ Compute forces           │
      ├─ Update UI                      ├─ Apply to device          │
      │                                  │                         │
      │                                  │                         ├─ Check conditions
      │                                  │                         ├─ Collect data
      │                                  │                         ├─ Build DataPoint
      │                                  │                         ├───► Client
      │                                  │                         │
      │                                  │                         ├─ Check end condition
      │                                  │                         │
      │                                  │                    [Condition met]
      │                                  │                         │
      │                                  │                         ├─ Stop trial
      │                                  │                         ├───► Client
```

**TrialDataPoint Contents** (at configured sample rate, e.g., 500 Hz):
- `timestamp` - Trial time (seconds)
- `devicePosition` - Raw device position (x, y, z)
- `proxyPosition` - Virtual proxy position (x, y, z)
- `force` - Computed force vector (x, y, z)
- `deviceVelocity` - Device velocity
- `deviceOrientation` - Quaternion (x, y, z, w)
- `objects[]` - State of each object (position, velocity, custom data)
- Extended data: contacts, trial state, diagnostics, etc.

### 4. Trial Completion Phase

```
Server                           Client
  │                                │
  ├─ End condition met              │
  ├─ Mark trial as complete         │
  ├─ Disable haptics                │
  ├─ Ramp forces to zero            │
  │                                │
  ├─── StatusUpdate(COMPLETED) ──►├─ Write CSV
  │                                ├─ Save metadata
  ├─ Close gRPC stream              ├─ Close files
  │                                │
  ├─ Keep scene visible             │
  ├─ Wait for next trial            │
```

## Internal Data Flow

### Haptics Loop Data Flow

```
1. Read Device State
   │
   ├─ cHapticDevice::getPosition()
   ├─ cHapticDevice::getVelocity()
   ├─ cHapticDevice::getRotation()
   │
   ▼
2. Update Tool from Device
   │
   ├─ cGenericTool::updateFromDevice()
   ├─ Map device space → workspace
   ├─ Update haptic points
   │
   ▼
3. Compute World Transforms
   │
   ├─ cWorld::computeGlobalPositions()
   ├─ Update object transforms
   ├─ Update tool position
   │
   ▼
4. Compute Interaction Forces
   │
   ├─ cGenericTool::computeInteractionForces()
   ├─ Collision detection
   ├─ Proxy algorithm
   ├─ Force = stiffness * (device - proxy)
   │
   ▼
5. Apply Forces to Device
   │
   ├─ cGenericTool::applyToDevice()
   ├─ cHapticDevice::setForce()
   └─ Send to hardware
```

### Graphics Loop Data Flow

```
1. Update Physics (Custom Objects)
   │
   ├─ For each object with CustomObjectInterface:
   ├─   CustomObject::updatePhysics(deltaTime)
   ├─   Update position/velocity
   ├─   Apply forces/constraints
   │
   ▼
2. Update UI Widgets
   │
   ├─ Update frequency counters
   ├─ Update status text
   ├─ Update trial time display
   │
   ▼
3. Render Scene
   │
   ├─ Update shadow maps
   ├─ cCamera::renderView()
   ├─ Render all objects
   ├─ Render UI overlay
   │
   ▼
4. Swap Buffers
   │
   ├─ glfwSwapBuffers()
   └─ Display frame
```

### Object Lifecycle

```
Create:
  ├─ CustomObjectRegistry::createObject(type)
  ├─ CustomObject::createObject(params)
  ├─ Returns cGenericObject*
  ├─ SimulationManager::addObject()
  ├─ world->addChild()
  └─ Store in objectMap

Update:
  ├─ [Graphics Thread]
  ├─   CustomObject::updatePhysics()
  │
  ├─ [Haptics Thread]
  └─   cGenericTool::computeInteractionForces()
        └─ Collision with object

Destroy:
  ├─ SimulationManager::removeObject(id)
  ├─ Move to (10000, 10000, 10000) [far away]
  ├─ [Later, graphics thread]
  ├─ world->removeChild()
  └─ unique_ptr destruction
```

## Data Synchronization

### Thread Safety

**Mutex-Protected Data**:
- `objectMapMutex` - Protects `objectMap` and `objectInterfaceMap`
- Used when:
  - Adding objects (gRPC thread)
  - Removing objects (gRPC thread)
  - Updating physics (graphics thread)
  - Accessing object state (any thread)

**Lock-Free Data** (atomic or single-writer):
- `hapticsEnabled` - Boolean flag
- `simulationRunning` - Boolean flag
- `trialRunning` - Boolean flag
- Frequency counters - Thread-safe internally

### Data Collection Timing

```
Haptics: 1000 Hz ─────────▼─────────▼─────────▼─────────▼
                       Force computation

Graphics: 60 Hz  ────────────────▼─────────────────▼
                       Physics update

Data Stream: 500 Hz ─────▼─────▼─────▼─────▼─────▼
                       Sample & send

Data collected at specified rate from most recent values
```

## Message Flow Sequences

### Successful Trial

```
Client                 Server
  │                      │
  ├─── TrialRequest ───►│
  │                      ├─ Setup scene
  │◄─ StatusUpdate(STARTED)│
  │                      ├─ Wait for start condition
  │                      .
  │                      .
  │◄─── DataPoint ─────│ (start condition met)
  │◄─── DataPoint ─────│
  │◄─── DataPoint ─────│
  │       ...            │
  │                      ├─ End condition met
  │◄ StatusUpdate(COMPLETED)│
  │                      │
```

### Error Handling

```
Client                 Server
  │                      │
  ├─── TrialRequest ───►│
  │                      ├─ Validation fails
  │◄─ StatusUpdate(ERROR) │
  │   "Invalid object type"│
  │                      │
  [Stream closed]         │
```

### Client Cancellation

```
Client                 Server
  │                      │
  ├─── TrialRequest ───►│
  │◄─ StatusUpdate(STARTED)│
  │◄─── DataPoint ─────│
  │                      │
  [Client disconnects]   │
  │                      ├─ Detect cancellation
  │                      ├─ Clean up trial
  │                      ├─ Disable haptics
  │                      ├─ Reset scene
```

## Data Persistence

### CSV Output Format

```
timestamp,device_x,device_y,device_z,proxy_x,proxy_y,proxy_z,force_x,force_y,force_z,...
0.002,0.05,0.03,0.02,0.05,0.03,0.02,0.0,0.0,0.0,...
0.004,0.051,0.031,0.021,0.051,0.031,0.021,0.1,-0.2,0.0,...
...
```

### Metadata JSON Format

```json
{
  "trial_id": "trial_001",
  "subject": "participant_01",
  "date": "2025-11-11",
  "configuration": {
    "objects": [...],
    "start_conditions": [...],
    "end_conditions": [...]
  },
  "duration": 12.5,
  "samples": 6250
}
```

## Performance Considerations

### Bandwidth
- Each `TrialDataPoint` ≈ 200-500 bytes (protobuf binary)
- At 500 Hz: ~100-250 KB/s
- Over localhost: negligible latency (<1ms)

### Latency
- Device → Force: <1ms (haptics loop)
- Data collection → Client: ~2-5ms (gRPC + serialization)
- Total device-to-log: <10ms

### Buffer Management
- gRPC handles streaming buffering
- Client writes to disk asynchronously
- No data loss under normal conditions