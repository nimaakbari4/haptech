# System Architecture

Documentation for system architecture and design principles.

## Contents

- [System Overview](overview.md) - High-level architecture, components, and design decisions
- [Data Flow](data-flow.md) - Communication patterns, message flow, and data synchronization
- [Build System](build-system.md) - Build process, dependencies, and compilation

## Quick Links

### Key Concepts

**Client-Server Architecture**: Python client controls C++ real-time haptics server via gRPC

**Multi-threaded Design**:
- Graphics thread (60 Hz) - Rendering and physics
- Haptics thread (1000 Hz) - Force computation
- gRPC thread - Communication and control

**Extensibility System**: Plugin architecture for custom objects and trial conditions

### Core Technologies

- **C++17** - Core server implementation
- **CHAI3D** - Haptic rendering framework
- **gRPC/Protobuf** - Communication protocol
- **Python 3** - Client scripting and data collection
- **OpenGL/GLFW** - Graphics rendering
- **CMake/Make** - Build system

### Important Files

- `core/src/main.cpp` - Application entry point
- `core/src/simulation_manager.{h,cpp}` - Core simulation loop
- `core/src/franka_trial_server.{h,cpp}` - gRPC service
- `core/Makefile` - Build configuration
- `proto/franka_trial.proto` - API definition
- `python_client/main.py` - Client entry point

## Architecture Principles

### Real-Time Performance
The haptics loop must maintain 1000 Hz update rate for stable force rendering. Any code in the haptics thread should be:
- Deterministic
- Lock-free where possible
- Minimal computation
- No I/O operations

### Thread Safety
Shared data structures are protected by mutexes. Object maps use `std::lock_guard` for RAII-style locking.

### Separation of Concerns
- **SimulationManager**: Scene management, rendering, haptics
- **FrankaTrialServer**: Network communication, trial control
- **Custom Objects**: Object-specific behavior and physics
- **Trial Conditions**: Start/end logic

### Extensibility
New functionality can be added without modifying core code:
- Register custom object types
- Register trial condition types
- Specify in XML configuration

## Getting Started

1. Read [System Overview](overview.md) for architecture understanding
2. Review [Data Flow](data-flow.md) for communication patterns
3. Check [Build System](build-system.md) for compilation details
4. Explore [Core Components](../core/README.md) for implementation details
5. Study [Protocols](../protocols/README.md) for API reference