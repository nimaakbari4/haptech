# CustomObjectInterface Specification

The `CustomObjectInterface` is like a template or contract that all custom haptic objects must follow. Think of it as a standard set of questions that every custom object must be able to answer, such as "How do you create yourself?" and "How do you behave over time?"

## What is an Interface?

In programming, an interface defines what methods (functions) a class must implement, but not how to implement them. It's like saying "every car must have a method to start the engine" without specifying whether it's a key, button, or voice command.

For haptic objects, every custom object must be able to:
1. Create its 3D representation (`createObject`)
2. Update its behavior over time (`updatePhysics`) 
3. Identify itself by name (`getTypeName`)

## Interface Overview

Here's the complete interface that your custom object must implement:

```cpp
class CustomObjectInterface {
public:
    // Virtual destructor - ensures proper cleanup when objects are destroyed
    virtual ~CustomObjectInterface() = default;
    
    // Creates the 3D object with specified position, color, and custom parameters
    virtual cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                        double x, double y, double z,
                                        double r, double g, double b) = 0;
    
    // Updates object behavior every simulation frame
    virtual void updatePhysics(cGenericObject* object, double deltaTime) = 0;
    
    // Returns the string name used to identify this object type in XML
    virtual std::string getTypeName() const = 0;
};
```

**Note**: The `= 0` at the end means these are "pure virtual" methods - you MUST implement them in your custom class.

## Required Methods

### `createObject()`

**Purpose**: Creates and sets up the 3D object that users will see and feel.

**Parameters**:
- `params` - A collection of custom settings from your XML file (like `<Parameters size="0.05" stiffness="1000"/>`)
- `x, y, z` - Where to place the object in 3D space (in meters)
- `r, g, b` - What color to make the object (values between 0.0 and 1.0)

**Returns**: A pointer to the created CHAI3D object (the actual 3D thing that gets rendered)

**What this method should do**:
- Create the 3D shape (sphere, box, custom mesh, etc.)
- Set its position and color
- Configure how it feels to touch (stiffness, friction, etc.)
- Apply any custom settings from the XML parameters
- Return a pointer to the complete, ready-to-use object

**Example**:
```cpp
cGenericObject* SphereObject::createObject(const std::map<std::string, std::string>& params,
                                          double x, double y, double z,
                                          double r, double g, double b) {
    // Step 1: Create a mesh to hold our 3D shape
    cMesh* sphere = new cMesh();
    
    // Step 2: Create the actual sphere geometry
    // getRadius() is a helper method that reads the "radius" parameter from XML
    cCreateSphere(sphere, getRadius(params));
    
    // Step 3: Position and color the sphere
    sphere->setLocalPos(x, y, z);           // Place it in 3D space
    sphere->m_material->setColor(r, g, b);   // Set its color
    
    // Step 4: Configure haptic properties (how it feels to touch)
    sphere->m_material->setStiffness(1000);         // How hard/rigid it feels
    sphere->m_material->setStaticFriction(0.3);     // Friction when not moving
    sphere->m_material->setDynamicFriction(0.2);    // Friction when sliding
    
    // Step 5: Enable haptic collision detection
    // This allows the haptic device to "feel" when it touches the sphere
    sphere->createAABBCollisionDetector(toolRadius);
    
    return sphere;  // Return the complete object
}
```

### `updatePhysics()`

**Purpose**: Called repeatedly during simulation to update the object's behavior and animation.

**Parameters**:
- `object` - The 3D object that was created by your `createObject()` method
- `deltaTime` - How much time has passed since the last update (in seconds)

**What this method should do**:
- Update the object's position, rotation, or other properties
- Implement custom behaviors like gravity, springs, or collisions
- Handle animations and state changes
- Modify haptic properties if they change over time

**Important**: This method is called many times per second (typically 60-120 times), so keep the calculations simple and fast.

**Example**:
```cpp
void BallObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // Simple gravity simulation
    
    // Step 1: Apply gravity to the ball's velocity
    // 9.81 is Earth's gravity in m/s², _gravityScale lets us adjust it
    _velocity.y() -= 9.81 * _gravityScale * deltaTime;
    
    // Step 2: Update the ball's position based on its velocity
    cVector3d currentPos = object->getLocalPos();        // Get current position
    cVector3d newPos = currentPos + _velocity * deltaTime; // Calculate new position
    
    // Step 3: Handle collision with the ground
    if (newPos.y() < _radius) {  // If ball hits ground (y = 0)
        newPos.y() = _radius;                           // Place ball on ground
        _velocity.y() = -_velocity.y() * _bounciness;   // Reverse and reduce velocity
    }
    
    // Step 4: Apply the new position
    object->setLocalPos(newPos);
}
```

### `getTypeName()`

**Purpose**: Returns the string identifier used in XML configuration.

**Returns**: String type name that matches the XML `type` attribute

**Example**:
```cpp
std::string SphereObject::getTypeName() const {
    return "sphere";
}
```

## Registration

Custom objects must be registered in the `ExtensionsRegistry` to be available for use:

```cpp
// In ExtensionsRegistry::registerAllExtensions()
registerCustomObject("sphere", []() -> std::unique_ptr<CustomObjectInterface> {
    return std::make_unique<SphereObject>();
});
```

## Parameter Handling

Objects receive configuration settings from XML files through the `params` map. Here are common patterns for reading these parameters safely:

```cpp
// Reading a number parameter with a default value
double getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");  // Look for "radius" in the parameters
    if (it != params.end()) {         // If we found it
        try {
            return std::stod(it->second); // Convert string to number
        } catch (const std::invalid_argument&) {
            // If conversion fails (e.g., radius="abc"), print error and use default
            std::cerr << "Invalid radius parameter: " << it->second << std::endl;
        }
    }
    return 0.05; // Default value if parameter is missing or invalid
}

// Reading a yes/no parameter
bool isVisible(const std::map<std::string, std::string>& params) const {
    auto it = params.find("visible");
    return (it != params.end() && it->second == "true");
}
```

**XML Example**: These helper methods would read parameters like:
```xml
<Parameters radius="0.08" visible="true" stiffness="1500"/>
```
```

## Memory Management

**Don't worry too much about this section** - the system handles most memory management automatically. Here are the key points:

- The system takes ownership of objects you create with `createObject()`
- You don't need to manually delete these objects
- If your object allocates additional memory (rare), clean it up in the destructor
- Follow the "RAII" principle: if you create something, make sure it gets properly destroyed

## Thread Safety

**For most simple objects, you can ignore this section.** But if you're sharing data between methods or using complex state:

- `createObject()` is called once when setting up the scene
- `updatePhysics()` is called repeatedly during simulation
- If both methods access the same variables, you may need synchronization
- Avoid time-consuming operations in `updatePhysics()` as it can make haptic feedback jerky