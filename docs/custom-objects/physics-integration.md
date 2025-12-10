# Physics Integration

How to make your custom objects move, respond to forces, and provide realistic haptic feedback.

## Overview

Physics in the haptic system means making objects behave like real-world objects - they can fall due to gravity, bounce when they hit something, or spring back to a rest position. You implement physics through the `updatePhysics()` method, which is called many times per second to update your object's behavior.

**Don't be intimidated by physics!** You can start with simple behaviors and add complexity gradually. Even basic animations like rotation or color changes count as "physics" updates.

## Physics Update Cycle

### How Often Updates Happen

The `updatePhysics()` method is called 60-120 times per second (matching your monitor's refresh rate), but NOT at the haptic rate of 1000 Hz. This separation is important:

- **Haptic rendering** (1000 Hz): Handles the immediate touch feedback - must be extremely fast
- **Physics updates** (60-120 Hz): Handles object movement and behavior - can be more complex

This design lets you implement sophisticated physics without making the haptic feedback feel delayed or jerky.

### The Method You Implement

```cpp
virtual void updatePhysics(cGenericObject* object, double deltaTime) = 0;
```

**Parameters explained**:
- `object`: The 3D object you created in `createObject()` - you'll modify its position, rotation, color, etc.
- `deltaTime`: How much time passed since the last update (typically 0.008-0.016 seconds)

**Key insight**: `deltaTime` varies depending on system performance, so always use it in your calculations to ensure consistent behavior regardless of framerate.

## CHAI3D Haptic Integration

### What Makes Objects "Feel" Different

CHAI3D objects have material properties that determine how they feel when touched with the haptic device:

```cpp
// In your createObject() method
cMesh* mesh = new cMesh();

// Configure how the object feels to touch
mesh->m_material->setStiffness(1000.0);        // Higher = harder surface (N/m)
mesh->m_material->setStaticFriction(0.3);      // Resistance when starting to move
mesh->m_material->setDynamicFriction(0.2);     // Resistance when already sliding  
mesh->m_material->setViscosity(0.1);           // "Thickness" - like moving through honey
mesh->m_material->setTextureLevel(0.5);        // Surface bumpiness/roughness

// Enable haptic feedback on the surface
mesh->m_material->setHapticTriangleSides(true, false);

// Create collision detection - this is what lets the haptic device "feel" the object
double toolRadius = 0.01; // Size of haptic tool tip (1cm)
mesh->createAABBCollisionDetector(toolRadius);
```

**Real-world analogies**:
- **Stiffness**: Difference between touching a rock (high) vs. a sponge (low)
- **Friction**: Difference between touching sandpaper (high) vs. ice (low)  
- **Viscosity**: Difference between moving through air (low) vs. thick syrup (high)

### Changing How Objects Feel Over Time

You can modify material properties during physics updates to create dynamic experiences:

```cpp
void MyObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // Example: Object gets softer as it "heats up"
    double temperature = getCurrentTemperature();
    double dynamicStiffness = baseStiffness * (1.0 - temperature * 0.01);
    object->m_material->setStiffness(dynamicStiffness);
    
    // Example: Object becomes more slippery as it "wears down"
    double wear = getWearLevel();
    object->m_material->setDynamicFriction(0.2 * (1.0 - wear));
}
```

**Research applications**: You could simulate materials that change properties under stress, temperature effects, or wear patterns.
```

## Common Physics Patterns

### 1. Simple Gravity (Making Objects Fall)

```cpp
class FallingObject : public CustomObjectInterface {
private:
    // Store state for each object instance
    struct ObjectState {
        cVector3d velocity{0, 0, 0};  // How fast it's moving (x, y, z)
        double mass = 1.0;            // Object's mass (affects acceleration)
        bool onGround = false;        // Whether it's resting on the ground
    };
    // Map to store state for multiple objects of this type
    std::unordered_map<cGenericObject*, ObjectState> objectStates;
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        auto& state = objectStates[object];  // Get this object's state
        
        if (!state.onGround) {
            // Apply gravity: acceleration = 9.81 m/s² downward
            state.velocity.y() -= 9.81 * deltaTime;
            
            // Update position: newPosition = oldPosition + velocity * time
            cVector3d currentPos = object->getLocalPos();
            cVector3d newPos = currentPos + state.velocity * deltaTime;
            
            // Check if object hits the ground (y = 0)
            double groundLevel = 0.0;
            if (newPos.y() <= groundLevel) {
                newPos.y() = groundLevel;    // Place on ground
                state.velocity.y() = 0.0;    // Stop falling
                state.onGround = true;       // Mark as landed
            }
            
            object->setLocalPos(newPos);
        }
    }
};
```

**What this does**: Objects start falling when created and stop when they hit the ground at y=0.

### 2. Spring Behavior (Objects That Return to Position)

```cpp
class SpringObject : public CustomObjectInterface {
private:
    struct SpringState {
        cVector3d restPosition;    // Where the object "wants" to be
        cVector3d lastPosition;    // Position in the previous frame
        double stiffness;          // How strong the spring force is
        double damping;            // How much the motion dampens (prevents endless bouncing)
    };
    std::unordered_map<cGenericObject*, SpringState> springStates;
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        auto& state = springStates[object];
        cVector3d currentPos = object->getLocalPos();
        
        // Calculate how far the object is from its rest position
        cVector3d displacement = currentPos - state.restPosition;
        
        // Spring force: F = -kx (Hooke's Law)
        // The further from rest, the stronger the force pulling back
        cVector3d springForce = -state.stiffness * displacement;
        
        // Damping force: F = -cv (opposes motion to prevent endless oscillation)
        cVector3d velocity = (currentPos - state.lastPosition) / deltaTime;
        cVector3d dampingForce = -state.damping * velocity;
        
        // Combine forces
        cVector3d totalForce = springForce + dampingForce;
        
        // Apply force to update motion (simplified physics integration)
        double mass = 1.0;
        cVector3d acceleration = totalForce / mass;  // F = ma, so a = F/m
        cVector3d newVelocity = velocity + acceleration * deltaTime;
        cVector3d newPosition = currentPos + newVelocity * deltaTime;
        
        object->setLocalPos(newPosition);
        state.lastPosition = currentPos;  // Remember position for next frame
    }
};
```

**What this does**: Objects act like they're connected to their starting position by an invisible spring - push them away and they'll oscillate back to the rest position.

**Research applications**: Modeling elastic materials, studying motor control with unstable targets, simulating tissue deformation.

### 3. Bouncing Objects (Collision Detection and Response)

```cpp
class BounceObject : public CustomObjectInterface {
private:
    struct BounceState {
        cVector3d velocity{0, 0, 0};  // Object's movement speed and direction
        double radius;                // Object's size (for collision detection)
        double bounciness;            // How much energy is retained after bouncing (0-1)
    };
    std::unordered_map<cGenericObject*, BounceState> bounceStates;
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        auto& state = bounceStates[object];
        
        // Apply gravity (makes object fall)
        state.velocity.y() -= 9.81 * deltaTime;
        
        // Calculate where object would be next frame
        cVector3d currentPos = object->getLocalPos();
        cVector3d newPos = currentPos + state.velocity * deltaTime;
        
        // Define boundaries of our "room" (invisible walls)
        double minX = -0.5, maxX = 0.5;  // Left and right walls
        double minZ = -0.5, maxZ = 0.5;  // Front and back walls  
        double groundY = 0.0;            // Floor level
        
        // Check collision with left/right walls
        if (newPos.x() - state.radius < minX) {        // Hit left wall
            newPos.x() = minX + state.radius;          // Place just touching wall
            state.velocity.x() = -state.velocity.x() * state.bounciness;  // Reverse and dampen
        } else if (newPos.x() + state.radius > maxX) { // Hit right wall
            newPos.x() = maxX - state.radius;
            state.velocity.x() = -state.velocity.x() * state.bounciness;
        }
        
        // Check collision with floor
        if (newPos.y() - state.radius < groundY) {     // Hit ground
            newPos.y() = groundY + state.radius;
            state.velocity.y() = -state.velocity.y() * state.bounciness;
        }
        
        // Check collision with front/back walls  
        if (newPos.z() - state.radius < minZ) {        // Hit front wall
            newPos.z() = minZ + state.radius;
            state.velocity.z() = -state.velocity.z() * state.bounciness;
        } else if (newPos.z() + state.radius > maxZ) { // Hit back wall
            newPos.z() = maxZ - state.radius;
            state.velocity.z() = -state.velocity.z() * state.bounciness;
        }
        
        object->setLocalPos(newPos);  // Apply the final position
    }
};
```

**What this does**: Objects bounce around inside an invisible box, losing some energy with each bounce (controlled by the `bounciness` parameter).

**Research applications**: Studying hand-eye coordination, reaction time experiments, physics learning simulations.
```

## Advanced Physics Concepts

### 4. Force Fields (Invisible Forces)

You can make objects respond to invisible forces in space, like gravity or magnetism:

```cpp
class ForceFieldObject : public CustomObjectInterface {
private:
    // Calculate force at any position in space
    cVector3d calculateGradientForce(const cVector3d& position) {
        // Example: Create an attractive force toward the center
        cVector3d center(0, 0, 0);           // Center point of force field
        cVector3d direction = position - center;  // Vector from center to object
        double distance = direction.length();     // How far from center
        
        if (distance > 0.001) {  // Avoid division by zero
            direction.normalize();  // Make direction a unit vector
            // Inverse square law (like real gravity or magnetism)
            double strength = 100.0 / (distance * distance);
            return -direction * strength; // Negative = attractive force
        }
        return cVector3d(0, 0, 0);  // No force if at exact center
    }
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        cVector3d position = object->getLocalPos();
        cVector3d force = calculateGradientForce(position);
        
        // Apply force to change velocity (F = ma, so a = F/m)
        double mass = 1.0;
        cVector3d acceleration = force / mass;
        
        // Update velocity and position...
        // (You'd add this to your existing velocity/position update code)
    }
};
```

**What this does**: Objects are attracted toward the center point with a force that gets stronger as they get closer.

**Research applications**: Studying motor adaptation to novel force fields, force learning experiments.
```

### 5. Objects That Affect Each Other

Sometimes you want multiple objects to interact with each other:

```cpp
class ConnectedObjects : public CustomObjectInterface {
private:
    // Static list shared by all objects of this type
    static std::vector<cGenericObject*> allConnectedObjects;
    
    cVector3d calculateInteractionForces(cGenericObject* thisObject) {
        cVector3d totalForce(0, 0, 0);
        cVector3d thisPos = thisObject->getLocalPos();
        
        // Check interaction with every other connected object
        for (auto* other : allConnectedObjects) {
            if (other != thisObject) {  // Don't interact with yourself
                cVector3d otherPos = other->getLocalPos();
                cVector3d direction = otherPos - thisPos;
                double distance = direction.length();
                
                // Example: Objects push each other away when too close
                if (distance < 0.1 && distance > 0.001) {
                    direction.normalize();
                    double repulsion = 10.0 / (distance * distance);
                    totalForce -= direction * repulsion;  // Push away
                }
            }
        }
        return totalForce;
    }
};
```

**What this does**: Objects repel each other when they get too close, like magnets with the same polarity.

**Research applications**: Swarm behavior studies, personal space research, multi-agent coordination.
```

## Haptic Force Feedback

**Note**: For most applications, you won't need to directly control haptic forces. CHAI3D automatically generates appropriate forces based on your object's material properties and geometry.

### Modifying How Objects Feel During Updates

The most common way to create dynamic haptic effects is by changing material properties:

```cpp
void TemperatureObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // Simulate how temperature affects the object's feel
    double temperature = getCurrentTemperature();  // Your custom temperature calculation
    
    if (temperature > 50.0) {
        // Hot object feels softer and more viscous (like heated plastic)
        object->m_material->setStiffness(500.0);     // Reduced stiffness
        object->m_material->setViscosity(0.3);       // Increased viscosity
        object->m_material->setColor(1.0, 0.0, 0.0); // Red for hot
    } else if (temperature < 0.0) {
        // Cold object feels harder and more slippery (like ice)
        object->m_material->setStiffness(2000.0);    // Increased stiffness
        object->m_material->setDynamicFriction(0.1); // Reduced friction
        object->m_material->setColor(0.0, 0.0, 1.0); // Blue for cold
    }
}
```

**Research applications**: Temperature perception studies, material property learning, cross-modal sensory research.
```

## Performance Considerations

### Keep Physics Updates Simple

Since `updatePhysics()` is called 60-120 times per second, it needs to be fast:

**Good practices:**
- Use simple calculations when possible
- Store expensive computations and reuse them
- Use Euler integration (simple position updates) rather than complex physics solvers
- Avoid complex loops or recursive calculations

**Example of efficient state management:**
```cpp
class EfficientObject : public CustomObjectInterface {
private:
    // Store object data in arrays for better performance
    std::vector<cGenericObject*> objects;
    std::vector<ObjectState> states;
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        // Find which object this is (do this lookup only once)
        auto it = std::find(objects.begin(), objects.end(), object);
        if (it != objects.end()) {
            size_t index = std::distance(objects.begin(), it);
            updateSingleObject(states[index], deltaTime);  // Update just this one
        }
    }
    
private:
    void updateSingleObject(ObjectState& state, double deltaTime) {
        // Efficient per-object calculations here
    }
};
```
```

## Using External Physics Engines (Advanced)

**Skip this section unless you need very complex physics.** For most research applications, the built-in physics methods shown above are sufficient.

If you need advanced physics (complex collisions, fluid dynamics, etc.), you can integrate external physics engines like Bullet Physics:

```cpp
class BulletPhysicsObject : public CustomObjectInterface {
private:
    btDiscreteDynamicsWorld* physicsWorld;  // External physics simulation
    std::unordered_map<cGenericObject*, btRigidBody*> rigidBodies;  // Physics objects
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        // Step the external physics simulation
        physicsWorld->stepSimulation(deltaTime);
        
        // Copy results back to our CHAI3D object
        btRigidBody* body = rigidBodies[object];
        if (body) {
            btTransform transform = body->getWorldTransform();
            
            // Convert external physics position to CHAI3D position
            cVector3d position(transform.getOrigin().x(),
                              transform.getOrigin().y(), 
                              transform.getOrigin().z());
            object->setLocalPos(position);
            
            // Also convert rotation if needed...
        }
    }
};
```

**When you might need this**: Complex multi-body simulations, realistic fluid interactions, sophisticated collision scenarios.
```

## Best Practices for Research

1. **Start Simple**: Begin with basic movement (rotation, translation) before adding complex physics

2. **Test Physics Parameters**: 
   - Try different stiffness values to see what feels right (100-3000 is typical)
   - Adjust friction to match real materials (0.1 for slippery, 0.8 for rough)
   - Use damping to prevent unrealistic oscillations

3. **Validate with Real Physics**: 
   - Does gravity feel like 9.81 m/s²?
   - Do bounces look realistic?
   - Are object interactions intuitive?

4. **Consider Your Research Goals**:
   - **Motor learning**: Focus on consistent, predictable physics
   - **Perception studies**: Ensure haptic properties match visual appearance
   - **Skill training**: Prioritize realistic material properties

5. **Handle Edge Cases**: 
   - What happens if objects move very fast?
   - What if parameters are set to extreme values?
   - How do objects behave at simulation boundaries?

6. **Provide Clear Documentation**: Future researchers (including yourself) will thank you for explaining:
   - What physics model you used
   - What parameter ranges work well
   - Any known limitations or quirks

7. **Test Thoroughly**: Physics bugs can be subtle and may only appear under specific conditions

The physics integration system gives you powerful tools for creating engaging and realistic haptic experiences. Remember: you can create meaningful research applications with simple physics - don't feel obligated to implement complex simulations unless your research specifically requires them.