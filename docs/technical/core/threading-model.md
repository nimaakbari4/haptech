# Threading Model

Comprehensive documentation of the multi-threaded architecture, synchronization, and performance considerations.

## Overview

Haptech uses a **3-thread architecture** for parallel execution:

1. **Main/Graphics Thread** - 60 Hz rendering and physics (normal priority)
2. **Haptics Thread** - 1000 Hz force computation (high priority)
3. **gRPC Server Thread** - Asynchronous trial requests (background priority)

**Key Design Goals**:
- Haptics loop maintains stable 1000 Hz for force feedback quality
- Graphics rendering at 60 Hz for smooth visuals
- Network communication doesn't block simulation
- Thread-safe access to shared resources

## Thread Architecture

### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         Process: core                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌────────────────────────────────────────────────────────┐     │
│  │  Main Thread (Graphics + Physics)                      │     │
│  │  Priority: NORMAL                                      │     │
│  │  Frequency: ~60 Hz (vsync)                             │     │
│  ├────────────────────────────────────────────────────────┤     │
│  │  • OpenGL rendering                                    │     │
│  │  • Window management (GLFW)                            │     │
│  │  • Physics updates (CustomObjectInterface)             │     │
│  │  • UI text overlays                                    │     │
│  │  • Condition checking (trial start/end)                │     │
│  │  • Keyboard input                                      │     │
│  └────────────────────────────────────────────────────────┘     │
│                           │                                     │
│                           │ Shared Data:                        │
│                           │ - simContext.world                  │
│                           │ - simContext.objectMap              │
│                           ↓ - simContext.tool                   │
│  ┌────────────────────────────────────────────────────────┐     │
│  │  Haptics Thread (Force Feedback)                       │     │
│  │  Priority: HIGH (CTHREAD_PRIORITY_HAPTICS)             │     │
│  │  Frequency: 1000 Hz                                    │     │
│  ├────────────────────────────────────────────────────────┤     │
│  │  • Read device position                                │     │
│  │  • Update world transforms                             │     │
│  │  • Collision detection (tool vs world)                 │     │
│  │  • Proxy algorithm (god-object)                        │     │
│  │  • Force computation (spring-damper)                   │     │
│  │  • Apply force to device                               │     │
│  └────────────────────────────────────────────────────────┘     │
│                           │                                     │
│                           │                                     │
│                           ↓                                     │
│  ┌────────────────────────────────────────────────────────┐     │
│  │  gRPC Server Thread (Network Communication)            │     │
│  │  Priority: NORMAL (background)                         │     │
│  │  Frequency: Event-driven                               │     │
│  ├────────────────────────────────────────────────────────┤     │
│  │  • Listen for client connections (port 50051)          │     │
│  │  • Parse TrialRequest messages                         │     │
│  │  • Configure scene via SimulationManager API           │     │
│  │  • Stream TrialDataPoint messages                      │     │
│  │  • Handle client disconnection                         │     │
│  └────────────────────────────────────────────────────────┘     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Thread Creation

```cpp
// In SimulationManager::run()

// 1. Main thread (current thread)
//    - Initializes GLFW, OpenGL, CHAI3D
//    - Enters graphics loop (main event loop)

// 2. Create haptics thread (high priority)
simContext.hapticsThread = new cThread();
simContext.hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

// 3. gRPC server thread (background, started in main.cpp)
std::thread serverThread(runServer);
serverThread.detach();  // Run independently
```

## Common Issues and Solutions

### Haptics Thread Starvation

**Problem**: Haptics frequency drops below 1000 Hz

**Causes**:
- Heavy operations in haptics loop
- Too many objects (collision detection slow)
- Graphics thread hogging CPU

**Solutions**:
```cpp
// 1. Profile and optimize haptics loop
// 2. Reduce object count or use LOD
// 3. Ensure high thread priority
// 4. Pin to dedicated CPU core
```

### Race Conditions

**Problem**: Unpredictable behavior, crashes, corrupted data

**Symptoms**:
- Crashes only under heavy load
- Different behavior each run
- Invalid pointers

**Solutions**:
```cpp
// 1. Identify shared data
// 2. Add mutex protection
// 3. Use thread sanitizer (TSan)
g++ -fsanitize=thread -g simulation_manager.cpp
```

### Deadlocks

**Problem**: Threads freeze waiting for each other

**Detection**:
```cpp
// Timeout-based detection
std::unique_lock<std::mutex> lock(mutex, std::defer_lock);
if (!lock.try_lock_for(std::chrono::seconds(5))) {
    std::cout << "DEADLOCK: Timeout waiting for mutex" << std::endl;
}
```

**Prevention**:
```cpp
// 1. Always acquire locks in same order
// 2. Use std::lock for multiple mutexes
// 3. Keep lock duration minimal
// 4. Avoid calling external code while holding lock
```

### Graphics Stutter

**Problem**: Occasional frame drops, stuttering

**Causes**:
- Physics updates take too long
- Object map locked by other thread
- OpenGL driver synchronization

**Solutions**:
```cpp
// 1. Limit physics computation per frame
const double MAX_PHYSICS_TIME = 0.010;  // 10ms max
cPrecisionClock timer;
timer.start();
for (auto* obj : objects) {
    obj->updatePhysics(dt);
    if (timer.getCurrentTimeSeconds() > MAX_PHYSICS_TIME) {
        break;  // Continue next frame
    }
}

// 2. Use double buffering for object states
// 3. Reduce shadow map updates
```

## Thread Lifecycle

### Startup Sequence

```
1. Main thread starts
2. Initialize GLFW, OpenGL, CHAI3D
3. Create haptics thread (starts immediately)
4. Start gRPC server (separate thread)
5. Enter main graphics loop
```

```cpp
int main() {
    // 1. Start gRPC server in background
    std::thread serverThread(runServer);
    serverThread.detach();
    
    // 2. Initialize and run simulation (creates haptics thread)
    SimulationManager::run();
    
    // 3. Main loop runs until window closed
    // 4. Cleanup in SimulationManager::close()
    
    return 0;
}
```

### Shutdown Sequence

```
1. User closes window (or Ctrl+C)
2. Main loop exits
3. SimulationManager::close() called
4. Disable haptics, ramp force to zero
5. Set simulationRunning = false
6. Haptics thread exits loop
7. Wait for haptics thread to finish
8. Cleanup resources
9. gRPC server stops (may take time)
10. Process exits
```

```cpp
void SimulationManager::close(void) {
    // 1. Disable haptics
    simContext.hapticsEnabled = false;
    
    // 2. Ramp force to zero (safety)
    rampDeviceForceToZero(100);
    
    // 3. Remove all objects
    removeAllObjects();
    
    // 4. Stop simulation
    simContext.simulationRunning = false;
    
    // 5. Wait for haptics thread
    while (!simContext.simulationFinished) {
        cSleepMs(100);
    }
    
    // 6. Cleanup
    destroyHapticTool();
    delete simContext.hapticsThread;
    delete simContext.handler;
    delete simContext.world;
}
```

## See Also

- [Physics Engine](physics-engine.md) - Physics in graphics thread
- [Simulation Manager](simulation-manager.md) - Overall coordination
- [gRPC API](../protocols/grpc-api.md) - Network thread communication