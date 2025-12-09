# SimulationManager

Comprehensive documentation for the SimulationManager class, the core of the haptic simulation system.

## Overview

The `SimulationManager` is a static class that manages:
- **Graphics Loop**: 60 Hz rendering and UI updates (main thread)
- **Haptics Loop**: 1000 Hz force computation (dedicated thread)
- **Scene Management**: Objects, camera, lighting
- **Trial Control**: Condition evaluation and state management

**Files**: `core/src/simulation_manager.{h,cpp}`

## Class Structure

```cpp
class SimulationManager {
public:
    // Main entry point
    static bool run();
    
    // Scene manipulation
    static bool reset();
    static bool setBackgroundColor(double r, double g, double b);
    static bool setCamera(double px, double py, double pz,
                          double lx, double ly, double lz,
                          double ux, double uy, double uz);
    static bool addTextOverlay(...);
    static bool clearTextOverlays();
    
    // Object management
    static bool addObject(std::string id, cGenericObject* obj, ...);
    static bool removeObject(std::string id);
    static bool removeAllObjects();
    static bool addObjectFromConfig(...);
    
    // Trial condition checking
    static void checkTrialConditions();
    
    // Shared state
    static SimulationContext simContext;
    static TrialContext trialContext;
    
private:
    // Thread functions
    static void updateGraphics();
    static void updateHaptics();
    static void dataPollingThread();
    
    // Callbacks
    static void keyCallback(...);
    static void windowSizeCallback(...);
    static void errorCallback(...);
    
    // Cleanup
    static void close();
};
```

## SimulationContext

```cpp
struct SimulationContext {
    // Haptics control
    bool hapticsEnabled = true;
    
    // Display settings
    cStereoMode stereoMode = C_STEREO_DISABLED;
    bool fullscreen = false;
    bool mirroredDisplay = false;
    int width, height;
    
    // Scene graph
    cWorld* world;
    cCamera* camera;
    cDirectionalLight* light;
    
    // Haptic device
    cHapticDeviceHandler* handler;
    cGenericHapticDevicePtr hapticDevice;
    cGenericTool* tool;
    
    // Object management
    std::unordered_map<std::string, std::unique_ptr<cGenericObject>> objectMap;
    std::unordered_map<std::string, std::unique_ptr<CustomObjectInterface>> objectInterfaceMap;
    std::mutex objectMapMutex;
    
    // UI
    cLabel* labelRates;
    cLabel* statusText;
    std::vector<cLabel*> customTextOverlays;
    
    // Threading
    bool simulationRunning = true;
    bool simulationFinished = false;
    cThread* hapticsThread;
    
    // Timing
    cFrequencyCounter freqCounterGraphics;
    cFrequencyCounter freqCounterHaptics;
    cPrecisionClock physicsClock;
    
    // Window
    GLFWwindow* window = NULL;
};
```

## TrialContext

```cpp
struct TrialContext {
    bool trialRunning = false;
    std::string trialId;
    double time = 0.0;
    
    // Trial conditions
    std::vector<std::unique_ptr<TrialCondition>> startConditions;
    std::vector<std::unique_ptr<TrialCondition>> endConditions;
    std::unique_ptr<TrialCondition> startConditionMet;
    std::unique_ptr<TrialCondition> endConditionMet;
};
```

## Initialization Sequence

The `run()` method initializes the system in this order:

### 1. OpenGL/GLFW Setup

```cpp
glfwInit();
glfwSetErrorCallback(errorCallback);
// Create window
window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
glfwMakeContextCurrent(window);
glewInit();
```

### 2. Scene Creation

```cpp
world = new cWorld();
world->m_backgroundColor.setBlack();

camera = new cCamera(world);
camera->set(cVector3d(1.0, 0.0, 0.0),   // position
            cVector3d(0.0, 0.0, 0.0),   // lookAt
            cVector3d(0.0, 0.0, 1.0));  // up

light = new cDirectionalLight(world);
light->setDir(-1.0, 0.0, 0.0);
```

### 3. Haptic Device

```cpp
handler = new cHapticDeviceHandler();
handler->getDevice(hapticDevice, 0);  // Get first device

// Create tool with cursor
creatHapticTool();
```

**Special Cursor Flag**: You can choose whether to use our own cursor implementation (cToolMesh) or a built-in CHAI3D one. When the flag below in `simulation_manager.cpp` is set to true, it will use our cursor implementation which can be configured in XML. If false, it will use the built-in CHAI3D cToolCursor. Note that you will have to re-build the core using make when you change this.

```cpp
bool SimulationManager::useMeshCursorByDefault = true;
```

### 4. UI Widgets

```cpp
labelRates = new cLabel(font);
camera->m_frontLayer->addChild(labelRates);

statusText = new cLabel(biggerFont);
camera->m_frontLayer->addChild(statusText);
```

### 5. Start Threads

```cpp
physicsClock.start();

hapticsThread = new cThread();
hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

simulationRunning = true;
atexit(close);
```

### 6. Main Graphics Loop

```cpp
while (!glfwWindowShouldClose(window)) {
    updateGraphics();
    glfwSwapBuffers(window);
    glfwPollEvents();
    freqCounterGraphics.signal(1);
}
```

## Graphics Thread (Main Thread)

**Frequency**: ~60 Hz (vsync limited)

### Update Cycle

```cpp
void updateGraphics() {
    // 1. Update physics (moved from haptics thread)
    if (trialContext.trialRunning) {
        double deltaTime = physicsClock.getCurrentTimeSeconds();
        physicsClock.reset();
        
        // Clamp deltaTime
        if (deltaTime > 0.1) deltaTime = 0.016;
        else if (deltaTime < 0.0001) deltaTime = 0.016;
        
        // Update custom object physics
        std::lock_guard<std::mutex> lock(objectMapMutex);
        for (auto& [id, chaiObj] : objectMap) {
            auto it = objectInterfaceMap.find(id);
            if (it != objectInterfaceMap.end()) {
                it->second->updatePhysics(chaiObj.get(), deltaTime);
            }
        }
    }
    
    // 2. Update UI widgets
    labelRates->setText(
        cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz"
    );
    
    if (trialContext.trialRunning) {
        statusText->setText(
            "Trial running: " + trialContext.trialId + 
            " | t = " + cStr(trialContext.time, 4)
        );
    } else {
        statusText->setText("Waiting for a trial to begin");
    }
    
    // 3. Render scene
    world->updateShadowMaps(false, mirroredDisplay);
    camera->renderView(framebufferWidth, framebufferHeight);
    glFinish();
}
```

## Haptics Thread

**Frequency**: 1000 Hz (1ms cycle)
**Priority**: High (CTHREAD_PRIORITY_HAPTICS)

### Update Cycle

```cpp
void updateHaptics() {
    cPrecisionClock clock;
    clock.start();
    
    while (simulationRunning) {
        // Wait if haptics disabled
        if (!hapticsEnabled) {
            cSleepMs(100);
            continue;
        }
        
        // 1. Update world transforms
        world->computeGlobalPositions(true);
        
        // 2. Update tool from device
        tool->updateFromDevice();
        
        // 3. Compute interaction forces
        tool->computeInteractionForces();
        
        // 4. Apply forces to device
        tool->applyToDevice();
        
        // 5. Update frequency counter
        freqCounterHaptics.signal(1);
        
        // 6. Sleep to maintain 1kHz
        cSleepMs(1000 / HAPTICS_LOOP_FREQUENCY);
    }
}
```

### Why Physics Moved to Graphics Thread

Originally, physics updates were in the haptics loop. This caused:
- Haptics loop slowdown (missed 1kHz target)
- Force rendering instability
- Jittery haptic feedback

**Solution**: Move physics to graphics thread (60 Hz sufficient for visual updates)

## Scene Management

### Adding Objects

```cpp
bool addObjectFromConfig(const std::string& id, 
                        const std::string& type,
                        double px, double py, double pz,
                        double rx, double ry, double rz,
                        double r, double g, double b,
                        const std::map<std::string, std::string>& params) {
    // 1. Create via registry
    auto customObj = CustomObjectRegistry::createObject(type);
    cGenericObject* chaiObj = customObj->createObject(params, px, py, pz, r, g, b);
    
    // 2. Set position and rotation
    chaiObj->setLocalPos(px, py, pz);
    cMatrix3d rotMatrix;
    rotMatrix.setExtrinsicEulerRotationRad(rx, ry, rz, C_EULER_ORDER_XYZ);
    chaiObj->setLocalRot(rotMatrix);
    
    // 3. Configure material
    double maxStiffness = hapticDevice->getSpecifications().m_maxLinearStiffness / workspaceScale;
    chaiObj->m_material->setStiffness(maxStiffness);
    
    // 4. Add to world and maps
    world->addChild(chaiObj);
    std::lock_guard<std::mutex> lock(objectMapMutex);
    objectMap[id] = std::unique_ptr<cGenericObject>(chaiObj);
    objectInterfaceMap[id] = std::move(customObj);
}
```

### Removing Objects

**Safe Deletion Strategy**: Objects can't be deleted immediately (may be accessed by haptics thread)

```cpp
bool removeObject(std::string id) {
    std::lock_guard<std::mutex> lock(objectMapMutex);
    
    // Move object far away (effectively removes from haptics)
    auto it = objectMap.find(id);
    if (it != objectMap.end()) {
        it->second->setLocalPos(10000, 10000, 10000);
    }
    
    // Later (when safe), actually remove:
    // world->removeChild(obj);
    // objectMap.erase(id);
}
```

## Trial Management

### Reset Sequence

```cpp
bool reset() {
    // 1. Disable haptics
    hapticsEnabled = false;
    
    // 2. Clear tool state (drop contacts)
    tool->initialize();
    cSleepMs(100);
    
    // 3. Clear scene
    setBackgroundColor(0.0, 0.0, 0.0);
    clearTextOverlays();
    removeAllObjects();
    
    // 4. Reset camera
    setCamera(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    
    // 5. Clear conditions
    startConditions.clear();
    endConditions.clear();
    
    // 6. Ramp forces down
    rampDeviceForceToZero(150); // 150ms ramp
    
    // 7. Keep haptics disabled until next trial
}
```

### Condition Checking

```cpp
void checkTrialConditions() {
    // Check start conditions
    if (trialRunning && startConditionMet == nullptr) {
        for (auto& condition : startConditions) {
            if (condition->evaluate()) {
                startConditionMet = std::move(condition);
                break;
            }
        }
    }
    
    // Check end conditions
    if (trialRunning && endConditionMet == nullptr) {
        for (auto& condition : endConditions) {
            if (condition->evaluate()) {
                endConditionMet = std::move(condition);
                break;
            }
        }
    }
}
```

## Thread Safety

### Mutex Protection

```cpp
// When accessing object maps from any thread:
std::lock_guard<std::mutex> lock(objectMapMutex);
// ... access objectMap or objectInterfaceMap ...
// Automatically unlocked when lock goes out of scope
```

### Atomic Flags

```cpp
// Simple boolean flags (no mutex needed)
bool hapticsEnabled;       // Control haptics loop
bool simulationRunning;    // Control all threads
bool trialRunning;         // Trial state
```

## Cleanup and Shutdown

```cpp
void close() {
    // 1. Disable haptics
    hapticsEnabled = false;
    
    // 2. Ramp forces to zero
    rampDeviceForceToZero(100);
    
    // 3. Remove objects
    removeAllObjects();
    
    // 4. Stop threads
    simulationRunning = false;
    while (!simulationFinished) {
        cSleepMs(100);
    }
    
    // 5. Cleanup CHAI3D objects
    delete tool;
    delete camera;
    delete light;
    delete world;
    delete handler;
    delete hapticsThread;
}
```

## Performance Metrics

- **Graphics Loop**: Typically 60 Hz (vsync)
- **Haptics Loop**: Consistently 1000 Hz when running
- **Object Updates**: 60 Hz (graphics thread)
- **Collision Detection**: 1000 Hz (haptics thread)
- **UI Updates**: 60 Hz

## Common Issues

### Haptics Loop Slowdown
**Symptom**: Haptics frequency drops below 1000 Hz
**Causes**:
- Heavy computation in haptics thread
- Too many objects with complex collision geometry
- Lock contention on objectMapMutex

**Solutions**:
- Move non-critical updates to graphics thread
- Optimize collision detectors
- Minimize mutex lock duration

### Force Discontinuities
**Symptom**: Sudden jumps in force, unstable haptics
**Causes**:
- Objects added/removed without disabling haptics
- Abrupt force changes
- Thread synchronization issues

**Solutions**:
- Always disable haptics before scene changes
- Use force ramping (rampDeviceForceToZero)
- Properly mutex-protect shared data