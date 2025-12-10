# System Overview

High-level architecture and design of the Haptech haptic research platform.

## System Purpose

Haptech is a haptic research platform for conducting psychophysics experiments with haptic devices. It provides:
- Real-time haptic rendering and force feedback
- 3D visual environment with OpenGL
- Trial-based experimental workflow
- Data collection and streaming
- Extensible object and condition system
- Remote control via gRPC API

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Python Client                            │
│  (Configuration, Control, Data Collection)                   │
└────────────────────┬────────────────────────────────────────┘
                     │ gRPC (localhost:50051)
                     │ TrialRequest → TrialResponseStream
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    C++ Core Server                           │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  FrankaTrialServer (gRPC Service - Background Thread)│  │
│  └──────────────┬───────────────────────────────────────┘  │
│                 │                                            │
│  ┌──────────────▼───────────────────────────────────────┐  │
│  │         SimulationManager                            │  │
│  │  - Main Graphics Thread (60 Hz)                      │  │
│  │  - Haptics Thread (1000 Hz)                          │  │
│  │  - Scene Management                                  │  │
│  │  - Object Lifecycle                                  │  │
│  └──────┬────────────┬─────────────┬───────────────────┘  │
│         │            │             │                        │
│  ┌──────▼─────┐ ┌───▼──────┐ ┌───▼──────────┐            │
│  │  CHAI3D    │ │ Custom   │ │ Trial        │            │
│  │  Graphics  │ │ Objects  │ │ Conditions   │            │
│  │  & Haptics │ │ Registry │ │ Registry     │            │
│  └────────────┘ └──────────┘ └──────────────┘            │
└────────┬──────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────┐
│  Haptic Device      │
│  (Touch, Geomagic,  │
│   or Simulation)    │
└─────────────────────┘
```

## Core Components

### Python Client
- **Location**: `python_client/`
- **Purpose**: Trial configuration, execution control, data collection
- **Key Files**:
  - `main.py` - CLI entry point
  - `trials.py` - Trial execution logic
  - `parser.py` - XML configuration parser
  - `util.py` - Protobuf message builders
  - `settings.py` - Client configuration

### C++ Core Server
- **Location**: `core/`
- **Purpose**: Real-time haptics rendering, scene management, gRPC service
- **Executable**: `core/bin/core`
- **Main Components**:
  - `main.cpp` - Entry point, launches server and simulation threads
  - `SimulationManager` - Graphics/haptics loop, scene control
  - `FrankaTrialServer` - gRPC service implementation
  - Object/Condition registries - Extensibility system

### CHAI3D Library
- **Location**: `core/external/chai3d-Franka-integration/` (submodule)
- **Purpose**: Haptic rendering, collision detection, device abstraction
- **Provides**:
  - `cWorld`, `cCamera`, `cLight` - Scene graph
  - `cGenericHapticDevice` - Device abstraction
  - `cGenericTool` - Haptic tool/cursor
  - Physics and collision detection

### Protocol Definitions
- **Location**: `proto/franka_trial.proto`
- **Purpose**: Define gRPC service and message formats
- **Generated Code**:
  - C++: `core/src/franka_trial.{pb,grpc.pb}.{h,cc}`
  - Python: `python_client/franka_trial_pb2{,_grpc}.py`

## Thread Architecture

### Main Thread (Graphics)
- **Frequency**: ~60 Hz (vsync)
- **Responsibilities**:
  - Render 3D scene with OpenGL
  - Update UI overlays and status text
  - Handle keyboard input
  - Update physics for custom objects
  - Process window events (GLFW)

### Haptics Thread
- **Frequency**: 1000 Hz
- **Responsibilities**:
  - Update haptic device position
  - Compute collision forces with proxy algorithm
  - Apply forces to device
  - High-priority real-time loop

### gRPC Server Thread
- **Purpose**: Handle client connections and trial requests
- **Responsibilities**:
  - Parse trial configuration
  - Configure scene (objects, camera, colors)
  - Stream data back to client
  - Monitor trial conditions

## Data Flow

See [Data Flow](data-flow.md) for detailed diagrams and descriptions.

**Summary**:
1. Client sends `TrialRequest` with configuration
2. Server configures scene and starts trial
3. Haptics thread computes forces at 1kHz
4. Server streams `TrialDataPoint` messages at configurable rate
5. Client saves data to CSV
6. Trial ends when end condition met
7. Server sends `TrialStatusUpdate` with completion status

## Extensibility

### Custom Objects
New haptic objects can be added by:
1. Implementing `CustomObjectInterface`
2. Registering in `ExtensionsRegistry::registerAllExtensions()`
3. Specifying object type in XML configuration

See [Custom Objects Documentation](../custom-objects/README.md)

### Trial Conditions
New trial start/end conditions can be added by:
1. Inheriting from `TrialCondition`
2. Implementing `evaluate()` method
3. Registering in `ExtensionsRegistry::registerAllExtensions()`
4. Specifying condition type in XML configuration

See [Trial Conditions Documentation](../trial-conditions/README.md)

## Configuration System

Trials are configured via XML files:

```xml
<trial>
  <sceneSettings>
    <camera position="1,0,0" lookAt="0,0,0" up="0,0,1"/>
    <backgroundColor r="0.0" g="0.0" b="0.0"/>
    <textOverlay text="Trial 1" x="10" y="30" fontSize="24"/>
  </sceneSettings>
  
  <objects>
    <object id="sphere1" type="sphere" position="0,0,0">
      <param name="radius" value="0.05"/>
    </object>
  </objects>
  
  <startConditions>
    <condition type="time" value="1.0"/>
  </startConditions>
  
  <endConditions>
    <condition type="time" value="10.0"/>
  </endConditions>
  
  <dataCollection sampleRate="500"/>
</trial>
```

## Platform Support

### Linux (Recommended)
- Ubuntu 22.04 and 24.04 fully supported
- Native X11 support
- Best performance and stability

### Windows via WSL2
- Requires X server (VcXsrv or WSLg)
- Additional setup for display forwarding
- Supported but requires extra configuration

### macOS
- Experimental support
- Some known issues with CHAI3D integration
- May require additional troubleshooting

## Performance Characteristics

- **Haptics Loop**: 1000 Hz (1ms cycle time)
- **Graphics Loop**: 60 Hz (vsync)
- **Data Streaming**: Configurable (typically 100-500 Hz)
- **Physics Updates**: 60 Hz (graphics thread)
- **Maximum Stiffness**: Device-dependent, typically ~3000 N/m
- **Workspace**: Device-dependent, typically ~160mm cube

## Safety Features

- Force saturation limits
- Gradual force ramping on enable/disable
- Thread-safe object management
- Mutex protection for shared data structures
- Haptics disable during scene modifications

## Key Design Decisions

### Why C++ Core?
- Real-time haptics require deterministic performance
- Low-latency force computation (<1ms)
- CHAI3D library is C++
- Direct hardware access

### Why Python Client?
- Rapid prototyping of experiments
- Easy XML parsing and configuration
- Simple data analysis integration
- User-friendly scripting

### Why gRPC?
- Language-agnostic communication
- Efficient binary serialization (protobuf)
- Streaming support for real-time data
- Auto-generated client/server stubs
- Type-safe API

### Why Split Physics to Graphics Thread?
- Haptics thread must maintain 1kHz for stability
- Physics updates (60 Hz) sufficient for visual feedback
- Reduces haptics loop computational burden
- Improves force rendering consistency