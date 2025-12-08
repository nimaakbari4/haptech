# Physics Engine

Comprehensive documentation of the custom physics system, object updates, and integration with the graphics thread.

## Overview

The physics engine handles dynamic object behavior and state updates. Key aspects:
- **Custom Object Physics**: Objects implement their own physics via `CustomObjectInterface`
- **Graphics Thread Integration**: Physics runs at 60 Hz in the graphics/main thread
- **Delta Time**: Frame-independent physics using elapsed time
- **Collision Integration**: Objects query world collisions for realistic interaction
- **Flexible Design**: Each object type controls its own behavior

**Key Files**:
- `core/src/objects/custom_object_interface.h` - Base interface
- `core/src/objects/*.{h,cpp}` - Individual object implementations
- `core/src/simulation_manager.cpp` - Physics update loop

## CustomObjectInterface

### Interface Definition

All custom objects that need physics must implement this interface:

```cpp
class CustomObjectInterface {
public:
    virtual ~CustomObjectInterface() = default;
    
    /**
     * Create the CHAI3D object for this custom object
     * @param params Parameters from XML configuration
     * @param x,y,z Position
     * @param r,g,b Color
     * @return Created CHAI3D object
     */
    virtual cGenericObject* createObject(
        const std::map<std::string, std::string>& params,
        double x, double y, double z,
        double r, double g, double b) = 0;
    
    /**
     * Update physics for this object (called each simulation step)
     * @param object The CHAI3D object to update
     * @param deltaTime Time elapsed since last update (in seconds)
     */
    virtual void updatePhysics(
        cGenericObject* object, 
        double deltaTime) = 0;
    
    /**
     * Get the object type name (used in XML)
     */
    virtual std::string getTypeName() const = 0;
};
```

### Object Registration

Objects register themselves with the `CustomObjectRegistry`:

```cpp
class CustomObjectRegistry {
public:
    using ObjectCreator = std::function<std::unique_ptr<CustomObjectInterface>()>;
    
    // Register a custom object type
    static void registerType(const std::string& typeName, ObjectCreator creator);
    
    // Create an object by type name
    static std::unique_ptr<CustomObjectInterface> createObject(const std::string& typeName);
    
    // Check if a type is registered
    static bool isTypeRegistered(const std::string& typeName);
};
```

### Registration Example

In `core/src/extensions_registry.cpp`:

```cpp
void RegisterAllExtensions() {
    // Register custom objects
    CustomObjectRegistry::registerType("ball", []() {
        return std::make_unique<BallObject>();
    });
    
    CustomObjectRegistry::registerType("spring", []() {
        return std::make_unique<SpringObject>();
    });
    
    CustomObjectRegistry::registerType("box", []() {
        return std::make_unique<BoxObject>();
    });
    
    // ... more objects ...
}
```

## Physics Update Loop

### Graphics Thread Integration

Physics updates happen in the **graphics thread** at ~60 Hz (not in the 1000 Hz haptics thread):

```cpp
void SimulationManager::updateGraphics(void) {
    /////////////////////////////////////////////////////////////////////
    // UPDATE PHYSICS
    /////////////////////////////////////////////////////////////////////
    
    if (trialContext.trialRunning) {
        // Calculate deltaTime for physics updates
        simContext.physicsClock.stop();
        double physicsDeltaTime = simContext.physicsClock.getCurrentTimeSeconds();
        simContext.physicsClock.reset();
        simContext.physicsClock.start();
        
        // Clamp deltaTime to prevent large jumps
        if (physicsDeltaTime > 0.1) {
            physicsDeltaTime = 0.016; // Cap at ~60 Hz equivalent
        } else if (physicsDeltaTime < 0.0001) {
            physicsDeltaTime = 0.016; // Minimum for first frame
        }
        
        // Update physics with proper mutex locking
        {
            std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
            for (const auto& objPair : simContext.objectMap) {
                const std::string& objId = objPair.first;
                cGenericObject* chaiObj = objPair.second.get();
                
                // Look for corresponding custom object interface
                auto interfaceIt = simContext.objectInterfaceMap.find(objId);
                if (interfaceIt != simContext.objectInterfaceMap.end()) {
                    CustomObjectInterface* customInterface = interfaceIt->second.get();
                    if (customInterface && chaiObj) {
                        // Call physics update with deltaTime
                        customInterface->updatePhysics(chaiObj, physicsDeltaTime);
                    }
                }
            }
        }
    }
    
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////
    
    // ... graphics rendering ...
}
```

### Why Graphics Thread?

**Rationale**:
1. **Haptics Performance**: Haptics loop must stay at 1000 Hz for stable force feedback
2. **Visual Sufficient**: 60 Hz is sufficient for smooth visual animation
3. **Collision Queries**: World collision detection is expensive
4. **No Haptic Impact**: Physics doesn't affect force computation directly

**Trade-offs**:
- ✅ Haptics loop stays fast and stable
- ✅ Complex physics doesn't cause force instability
- ⚠️ Physics updates only 60 times per second (not noticeable visually)

## Delta Time Handling

### Concept

Delta time (Δt) enables frame-rate independent physics:

```cpp
// Position update with delta time
position += velocity * deltaTime;

// Velocity update with delta time
velocity += acceleration * deltaTime;
```

**Without delta time**: Objects move faster on fast computers, slower on slow computers  
**With delta time**: Objects move at consistent speed regardless of frame rate

### Implementation

```cpp
void BallObject::updatePhysics(cGenericObject* object, double deltaTime) {
    auto pos = object->getLocalPos();
    
    // Apply gravity (constant acceleration)
    cVector3d gravity(0, 0, -9.81);  // m/s²
    _velocity.add(gravity * _gravityScale * deltaTime);
    
    // Update position (integrate velocity)
    auto newPos = pos + _velocity * deltaTime;
    
    // Handle collisions...
    
    // Apply position
    object->setLocalPos(newPos);
}
```

### Time Step Clamping

```cpp
// Clamp deltaTime to prevent large jumps (e.g., when window is minimized)
if (physicsDeltaTime > 0.1) {
    physicsDeltaTime = 0.016; // Cap at ~60 Hz equivalent
} else if (physicsDeltaTime < 0.0001) {
    physicsDeltaTime = 0.016; // Minimum for first frame
}
```

**Why clamp?**
- Window minimized: Very large Δt causes objects to "teleport"
- First frame: Very small Δt causes division by zero issues
- Paused debugging: Prevents huge jumps when resuming

### Fixed vs Variable Time Step

Haptech uses **variable time step** (actual frame time):

**Advantages**:
- Simple implementation
- Automatic frame rate adaptation
- No catch-up loops needed

**Disadvantages**:
- Less deterministic than fixed time step
- Large Δt can cause instability (mitigated by clamping)

**Alternative - Fixed Time Step**:
```cpp
const double FIXED_DT = 1.0 / 60.0;  // 16.67ms
double accumulator = 0.0;

void updatePhysics() {
    accumulator += realDeltaTime;
    
    while (accumulator >= FIXED_DT) {
        // Physics step with fixed delta time
        updateObjects(FIXED_DT);
        accumulator -= FIXED_DT;
    }
}
```

## Example Physics Implementations

### Bouncing Ball

`core/src/objects/ball_object.{h,cpp}`:

```cpp
class BallObject : public CustomObjectInterface {
private:
    double _radius;
    double _bounciness;  // Coefficient of restitution
    double _mass;
    double _gravityScale;
    cVector3d _velocity;

public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        auto pos = object->getLocalPos();
        
        // 1. Handle haptic device collisions
        auto hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);
        int collisionEvents = hapticPoint->getNumCollisionEvents();
        
        if (collisionEvents > 0) {
            for (int i = 0; i < collisionEvents; i++) {
                auto collisionEvent = hapticPoint->getCollisionEvent(i);
                
                if (collisionEvent->m_object == object) {
                    // Get collision normal and force
                    auto forceDirection = collisionEvent->m_globalNormal * -1;
                    forceDirection.normalize();
                    
                    double forceMagnitude = hapticPoint->getLastComputedForce().length();
                    forceMagnitude /= _mass;  // F = ma → a = F/m
                    
                    // Apply impulse to velocity
                    _velocity.add(forceDirection * forceMagnitude);
                }
            }
        }
        
        // 2. Apply gravity
        _velocity.add(0, 0, -9.81 * _gravityScale * deltaTime);
        
        // 3. Predict next position
        auto newPos = pos + _velocity * deltaTime;
        
        // 4. Check collisions with world
        cWorld* world = SimulationManager::simContext.world;
        cCollisionRecorder recorder;
        cCollisionSettings settings;
        settings.m_collisionRadius = _radius;
        
        bool collision = world->computeCollisionDetection(pos, newPos, recorder, settings);
        
        if (collision && !recorder.m_collisions.empty()) {
            // Find closest collision (not with self)
            const cCollisionEvent* closestHit = nullptr;
            double closestDist = std::numeric_limits<double>::max();
            
            for (const auto& hit : recorder.m_collisions) {
                if (hit.m_object != object) {
                    double dist = cDistance(pos, hit.m_globalPos);
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestHit = &hit;
                    }
                }
            }
            
            if (closestHit) {
                // Reflect velocity with bounciness
                cVector3d normal = closestHit->m_globalNormal;
                double velDotNormal = cDot(_velocity, normal);
                
                if (velDotNormal < 0) {
                    cVector3d reflection = _velocity - 2.0 * velDotNormal * normal;
                    _velocity = reflection * _bounciness;
                    
                    // Clamp to surface
                    newPos = closestHit->m_globalPos + normal * (_radius + 0.001);
                }
            }
        }
        
        // 5. Apply final position
        object->setLocalPos(newPos);
    }
};
```

### Spring-Damper Object

`core/src/objects/spring_object.{h,cpp}`:

```cpp
class SpringObject : public CustomObjectInterface {
private:
    cVector3d _restPosition;
    double _springConstant;  // k (N/m)
    double _dampingConstant; // b (N·s/m)
    cVector3d _velocity;

public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        auto pos = object->getLocalPos();
        
        // Spring force: F = -k * (x - x0)
        cVector3d displacement = pos - _restPosition;
        cVector3d springForce = -_springConstant * displacement;
        
        // Damping force: F = -b * v
        cVector3d dampingForce = -_dampingConstant * _velocity;
        
        // Total force
        cVector3d totalForce = springForce + dampingForce;
        
        // F = ma → a = F/m (assume unit mass)
        cVector3d acceleration = totalForce;
        
        // Update velocity
        _velocity += acceleration * deltaTime;
        
        // Update position
        auto newPos = pos + _velocity * deltaTime;
        object->setLocalPos(newPos);
    }
};
```

### Pushable Object

```cpp
class PushableObject : public CustomObjectInterface {
private:
    cVector3d _velocity;
    double _mass;
    double _friction;  // Kinetic friction coefficient

public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        auto pos = object->getLocalPos();
        
        // Check for haptic contact
        auto hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);
        int numContacts = hapticPoint->getNumCollisionEvents();
        
        bool inContact = false;
        cVector3d contactForce(0, 0, 0);
        
        for (int i = 0; i < numContacts; i++) {
            auto event = hapticPoint->getCollisionEvent(i);
            if (event->m_object == object) {
                inContact = true;
                contactForce = event->m_globalForce;
                break;
            }
        }
        
        if (inContact) {
            // Apply contact force: F = ma → a = F/m
            cVector3d acceleration = contactForce / _mass;
            _velocity += acceleration * deltaTime;
        }
        
        // Apply friction (opposes motion)
        if (_velocity.length() > 0.001) {
            cVector3d frictionForce = -_friction * _velocity.normalized();
            _velocity += frictionForce * deltaTime;
            
            // Clamp to zero if very small
            if (_velocity.length() < 0.01) {
                _velocity.set(0, 0, 0);
            }
        }
        
        // Update position
        auto newPos = pos + _velocity * deltaTime;
        object->setLocalPos(newPos);
    }
};
```

### Rotating Object

```cpp
class RotatingObject : public CustomObjectInterface {
private:
    double _angularVelocity;  // rad/s
    cVector3d _rotationAxis;

public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        // Get current rotation
        cMatrix3d rot = object->getLocalRot();
        
        // Compute rotation delta
        double angleRad = _angularVelocity * deltaTime;
        
        // Create rotation matrix for delta
        cMatrix3d deltaRot;
        deltaRot.setAxisAngleRotationRad(_rotationAxis, angleRad);
        
        // Apply rotation
        cMatrix3d newRot = deltaRot * rot;
        object->setLocalRot(newRot);
    }
};
```

## Collision Detection for Physics

### World Collision Query

Objects can query collisions with the entire world:

```cpp
cWorld* world = SimulationManager::simContext.world;
cCollisionRecorder recorder;
cCollisionSettings settings;

// Configure collision detection
settings.m_ignoreShapes = false;
settings.m_checkForNearestCollisionOnly = false;
settings.m_adjustObjectMotion = false;
settings.m_collisionRadius = objectRadius;
settings.m_checkHapticObjects = true;

// Ray-cast from current position to next position
bool collision = world->computeCollisionDetection(
    currentPos,
    nextPos,
    recorder,
    settings
);

if (collision && !recorder.m_collisions.empty()) {
    // Process collisions
    for (const auto& hit : recorder.m_collisions) {
        if (hit.m_object != self) {
            cVector3d point = hit.m_globalPos;
            cVector3d normal = hit.m_globalNormal;
            // Handle collision response...
        }
    }
}
```

### Collision Settings

```cpp
cCollisionSettings settings;

// Collision sphere radius (makes detection more robust)
settings.m_collisionRadius = 0.02;

// Check only nearest collision (faster)
settings.m_checkForNearestCollisionOnly = true;

// Don't adjust object positions automatically
settings.m_adjustObjectMotion = false;

// Include haptic tool in detection
settings.m_checkHapticObjects = true;

// Don't ignore shape primitives
settings.m_ignoreShapes = false;
```

### Haptic Tool Collision

Check if object is being touched by the haptic cursor:

```cpp
cHapticPoint* hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);
int numContacts = hapticPoint->getNumCollisionEvents();

for (int i = 0; i < numContacts; i++) {
    cCollisionEvent* event = hapticPoint->getCollisionEvent(i);
    
    if (event->m_object == myObject) {
        // Object is being touched
        cVector3d contactPoint = event->m_globalPos;
        cVector3d contactNormal = event->m_globalNormal;
        cVector3d contactForce = event->m_globalForce;
        
        // Or get from haptic point directly:
        cVector3d force = hapticPoint->getLastComputedForce();
        
        // Apply physics response...
    }
}
```

### Constraint Solving

Keep object constrained to a surface or path:

```cpp
void updatePhysics(cGenericObject* object, double deltaTime) {
    // Update position
    cVector3d newPos = pos + velocity * deltaTime;
    
    // Constrain to plane (z = 0)
    newPos.z(0);
    
    // Or constrain to sphere surface
    double radius = 1.0;
    cVector3d direction = newPos.normalized();
    newPos = direction * radius;
    
    // Or constrain to path
    // ... project to nearest point on path ...
    
    object->setLocalPos(newPos);
}
```

## Performance Optimization

### Object Sleeping

Disable physics for stationary objects:

```cpp
class SleepableObject : public CustomObjectInterface {
private:
    bool _sleeping;
    double _sleepThreshold;
    double _timeAtRest;

public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        // Check if moving
        if (_velocity.length() < _sleepThreshold) {
            _timeAtRest += deltaTime;
            if (_timeAtRest > 1.0) {  // 1 second at rest
                _sleeping = true;
            }
        } else {
            _timeAtRest = 0.0;
            _sleeping = false;
        }
        
        // Skip physics if sleeping
        if (_sleeping) {
            return;
        }
        
        // Normal physics...
    }
    
    void wake() {
        _sleeping = false;
        _timeAtRest = 0.0;
    }
};
```

## Thread Safety

### Mutex Protection

Physics updates must be thread-safe since they access shared data:

```cpp
void SimulationManager::updateGraphics(void) {
    // Lock mutex before accessing object maps
    {
        std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
        
        for (const auto& objPair : simContext.objectMap) {
            // Safe to access objects here
            CustomObjectInterface* customInterface = ...;
            if (customInterface) {
                customInterface->updatePhysics(object, deltaTime);
            }
        }
    }
    // Mutex automatically released
}
```

### Avoiding Deadlocks

**Good Practice**:
```cpp
// Hold lock for minimal time
{
    std::lock_guard<std::mutex> lock(mutex);
    // Quick read/write
    auto objects = getObjects();
}

// Process outside lock
for (auto* obj : objects) {
    obj->updatePhysics(dt);
}
```

**Bad Practice**:
```cpp
// DON'T: Hold lock during expensive operations
std::lock_guard<std::mutex> lock(mutex);
for (auto* obj : objects) {
    obj->updatePhysics(dt);  // May take long time!
    // Other threads blocked entire time
}
```

## Common Issues and Solutions

### Objects Moving Too Fast

**Problem**: High velocity objects pass through walls

**Solutions**:
```cpp
// 1. Continuous collision detection (multiple substeps)
int substeps = 5;
double subDt = deltaTime / substeps;
for (int i = 0; i < substeps; i++) {
    updatePosition(subDt);
    checkCollisions();
}

// 2. Velocity clamping
double maxVelocity = 10.0;  // m/s
if (_velocity.length() > maxVelocity) {
    _velocity = _velocity.normalized() * maxVelocity;
}

// 3. Use collision radius
settings.m_collisionRadius = objectSize * 1.5;
```

### Jittery Motion

**Problem**: Objects jitter or vibrate

**Solutions**:
```cpp
// 1. Add damping
_velocity *= 0.98;  // 2% damping per frame

// 2. Smooth interpolation
_velocity = lerp(_velocity, targetVelocity, 0.1);

// 3. Deadzone for small velocities
if (_velocity.length() < 0.01) {
    _velocity.set(0, 0, 0);
}
```

### Energy Gain (Objects Speed Up)

**Problem**: Objects gain energy over time (incorrect physics)

**Solutions**:
```cpp
// 1. Proper time integration
// Use Verlet or RK4 instead of basic Euler

// 2. Energy dissipation
_velocity *= 0.999;  // Slight damping

// 3. Conservation checks
double energy = 0.5 * mass * velocity.lengthSq() + mass * g * height;
// Clamp if exceeds initial energy
```

### Collision Tunneling

**Problem**: Fast objects pass through thin walls

**Solutions**:
```cpp
// 1. Swept collision detection
// Check entire path, not just endpoints

// 2. Thicker collision geometry
mesh->createAABBCollisionDetector(radius * 2.0);

// 3. Velocity limits
const double MAX_VELOCITY = 5.0;
velocity.clamp(MAX_VELOCITY);
```

## See Also

- [Threading Model](threading-model.md) - Thread synchronization
- [Custom Objects](../../custom-objects/README.md) - Implementing custom objects
- [Simulation Manager](simulation-manager.md) - Overall coordination