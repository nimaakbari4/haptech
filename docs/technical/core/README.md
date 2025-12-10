# Core System Documentation

Comprehensive documentation for the C++ haptic system core components. This covers everything the HapTech team has implemented ourselves. For CHAI3D specific queries, [see the official CHAI3D documentation](https://www.chai3d.org/download/doc/html/wrapper-overview.html).

## Contents

- [SimulationManager](simulation-manager.md) - Main simulation loop, scene management, and graphics/haptics coordination
- [Physics Engine](physics-engine.md) - Physics simulation and custom object updates
- [Threading Model](threading-model.md) - Multi-threaded architecture and synchronization

## Quick Reference

### Main Components

| Component | File | Purpose |
|-----------|------|----------|
| Entry Point | `main.cpp` | Launch server and simulation threads |
| Simulation | `simulation_manager.{h,cpp}` | Graphics/haptics loop, scene control |
| gRPC Service | `franka_trial_server.{h,cpp}` | Network API, trial execution |
| Tool Management | `tool_mesh.{h,cpp}` | Haptic cursor/tool configuration |
| Object Registry | `extensions_registry.{h,cpp}` | Object and condition registration |
| Custom Objects | `objects/` directory | Extensible object implementations |
| Trial Conditions | `trial_conditions/` directory | Start/end condition logic |

### Core Data Structures

**SimulationContext** - Shared state for rendering and haptics:
- CHAI3D world, camera, lighting
- Haptic device and tool
- Object maps and scene graph
- Thread synchronization primitives
- Frequency counters

**TrialContext** - Trial-specific state:
- Trial ID and timing
- Start/end conditions
- Condition evaluation state

### Thread Responsibilities

| Thread | Frequency | Priority | Responsibilities |
|--------|-----------|----------|------------------|
| Main (Graphics) | 60 Hz | Normal | Rendering, physics, UI updates |
| Haptics | 1000 Hz | High | Force computation, device I/O |
| gRPC Server | Event-driven | Normal | Client communication, trial setup |

### Key Concepts

**Proxy Algorithm**: CHAI3D's haptic rendering uses a virtual proxy that follows the device but can't penetrate objects. Force = stiffness × (device position - proxy position).

**Object Lifecycle**: Objects are created on gRPC thread, updated by graphics/haptics threads, and deleted safely via mutex-protected maps.

**Trial Flow**:
1. Client sends TrialRequest
2. Server configures scene
3. Haptics enabled gradually
4. Data streamed to client
5. End condition triggers completion
6. Forces ramped down safely

## File Organization

```
core/src/
  main.cpp                        # Entry point
  simulation_manager.{h,cpp}      # Core simulation
  franka_trial_server.{h,cpp}     # gRPC service
  tool_mesh.{h,cpp}               # Haptic tool
  extensions_registry.{h,cpp}     # Plugin system
  
  objects/                        # Custom object implementations
    custom_object_interface.{h,cpp}  # Base interface
    sphere_object.{h,cpp}            # Built-in sphere
    box_object.{h,cpp}               # Built-in box
    cylinder_object.{h,cpp}          # Built-in cylinder
    ... (see directory listing)
  
  trial_conditions/               # Condition implementations
    trial_condition.{h,cpp}          # Base class
    time_condition.{h,cpp}           # Time-based
    position_condition.{h,cpp}       # Position-based
    velocity_condition.{h,cpp}       # Velocity-based
  
  franka_trial.{pb,grpc.pb}.{h,cc} # Generated protobuf
```

## Building

```bash
cd core
make              # Release build
make DEBUG=1      # Debug build
make clean        # Clean build artifacts
```

## Running

```bash
cd core/bin
./core
```

The server listens on `localhost:50051` by default.

## Development Guidelines

### Adding Custom Objects

1. Create new class inheriting from `CustomObjectInterface`
2. Implement `createObject()` and `updatePhysics()`
3. Register in `ExtensionsRegistry::registerAllExtensions()`
4. Reference type name in XML configuration

See [Custom Objects Documentation](../../custom-objects/README.md)

### Adding Trial Conditions

1. Create new class inheriting from `TrialCondition`
2. Implement `evaluate()` method
3. Register in `ExtensionsRegistry::registerAllExtensions()`
4. Specify condition type in XML

See [Trial Conditions Documentation](../../trial-conditions/README.md)

### Thread Safety Rules

- Always use `std::lock_guard` for `objectMapMutex` when accessing object maps
- Never block haptics thread with I/O or long computations
- Disable haptics before modifying scene graph
- Use atomic operations for simple flags

### Performance Tips

- Keep haptics loop under 1ms
- Move heavy computation to graphics thread
- Use CHAI3D collision detectors efficiently
- Minimize allocations in haptics loop
- Profile with `DEBUG=1` build and `-pg` flag