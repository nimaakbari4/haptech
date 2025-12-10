# Implementation Guide

Step-by-step guide for creating your own custom haptic objects.

## Overview

This guide walks you through creating a custom haptic object from scratch.

Creating a custom object involves:
1. Creating header and source files
2. Implementing the `CustomObjectInterface` (following a standard template)
3. Registering your object in the extensions registry (making it available )
4. Testing with XML configuration

## Step 1: Create Header File

Create a new header file in `core/src/objects/` (e.g., `my_object.h`):

```cpp
#ifndef MY_OBJECT_H
#define MY_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Custom haptic object example
 * This class inherits from CustomObjectInterface, which means it must
 * implement certain required methods (like filling in a template)
 */
class MyObject : public CustomObjectInterface {
public:
    MyObject() = default;           // Constructor - called when creating the object
    virtual ~MyObject() = default;  // Destructor - called when destroying the object
    
    // Required methods from CustomObjectInterface
    // The 'override' keyword ensures we're implementing the interface correctly
    
    // This method creates the 3D object and sets its properties
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    // This method is called every frame to update the object's behavior
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    // This returns the name used in XML files to identify this object type
    std::string getTypeName() const override { return "my_object"; }

private:
    // Helper methods for reading configuration parameters from XML
    // The 'params' map contains key-value pairs like "size" -> "0.05"
    double getSize(const std::map<std::string, std::string>& params) const;
    double getStiffness(const std::map<std::string, std::string>& params) const;
    
    // Variables to store object state between updates
    // The underscore prefix is a common C++ convention for private variables
    double _customState = 0.0;
};

#endif // MY_OBJECT_H
```

## Step 2: Implement Source File

Create the corresponding source file `my_object.cpp`:

```cpp
#include "my_object.h"
#include <iostream>

cGenericObject* MyObject::createObject(const std::map<std::string, std::string>& params,
                                      double x, double y, double z,
                                      double r, double g, double b) {
    // Create the base mesh object (this will hold our 3D shape)
    cMesh* mesh = new cMesh();
    
    // Create the 3D geometry - in this example, a box
    // We get the size from the XML configuration parameters
    double size = getSize(params);
    cCreateBox(mesh, size, size, size);  // Creates a cube with equal sides
    
    // Set position
    mesh->setLocalPos(x, y, z);
    
    // Set color
    mesh->m_material->setColor(r, g, b);
    
    // Configure haptic properties (how the object feels to touch)
    double stiffness = getStiffness(params);
    mesh->m_material->setStiffness(stiffness);        // How rigid/hard the surface feels
    mesh->m_material->setStaticFriction(0.3);        // Resistance when starting to slide
    mesh->m_material->setDynamicFriction(0.2);       // Resistance when already sliding
    
    // Enable haptic rendering (allow the haptic device to "feel" this object)
    mesh->m_material->setHapticTriangleSides(true, false);
    
    // Create collision detector - this detects when the haptic tool touches the object
    double toolRadius = 0.01; // Size of the haptic tool tip (1cm radius)
    mesh->createAABBCollisionDetector(toolRadius);
    
    std::cout << "[MyObject] Created object with size=" << size 
              << ", stiffness=" << stiffness << std::endl;
    
    return mesh;
}

void MyObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // This method is called every frame to update the object
    // deltaTime tells us how much time has passed since the last update
    
    // Example: Rotate the object slowly around the Y-axis
    _customState += deltaTime;  // Keep track of total elapsed time
    double angle = _customState * 0.5; // Rotate at 0.5 radians per second
    
    // Create a rotation matrix and apply it to the object
    cMatrix3d rotation;
    rotation.setRotationAxisAngle(cVector3d(0, 1, 0), angle);  // Rotate around Y-axis
    object->setLocalRot(rotation);
    
    // Example: Make the color pulse using a sine wave
    double intensity = 0.5 + 0.3 * sin(_customState * 2.0);  // Oscillates between 0.2 and 0.8
    object->m_material->setColor(intensity, 0.2, 0.8);       // Red component pulses
}

double MyObject::getSize(const std::map<std::string, std::string>& params) const {
    // Look for a "size" parameter in the XML configuration
    auto it = params.find("size");
    if (it != params.end()) {  // If we found the parameter
        try {
            // Convert the string value to a number
            double value = std::stod(it->second);  // std::stod = "string to double"
            if (value > 0.0) return value;  // Make sure it's a positive number
        } catch (const std::invalid_argument& e) {
            // If the conversion fails (e.g., "size=abc"), print an error
            std::cerr << "[MyObject] Invalid size parameter: " << it->second << std::endl;
        }
    }
    return 0.1; // Default size if parameter is missing or invalid
}

double MyObject::getStiffness(const std::map<std::string, std::string>& params) const {
    auto it = params.find("stiffness");
    if (it != params.end()) {
        try {
            double value = std::stod(it->second);
            if (value >= 0.0) return value;
        } catch (const std::invalid_argument& e) {
            std::cerr << "[MyObject] Invalid stiffness parameter: " << it->second << std::endl;
        }
    }
    return 1000.0; // Default stiffness
}
```

## Step 3: Register Your Object

Add your object to the extensions registry in `extensions_registry.cpp`:

```cpp
// Add include at the top of the file
#include "objects/my_object.h"

// In the ExtensionsRegistry::registerAllExtensions() method
// This registers your object with the name "my_object" - this is what you'll
// use in XML files. The lambda function (the [] part) creates a new instance
// of your object when needed.
registerCustomObject("my_object", []() -> std::unique_ptr<CustomObjectInterface> {
    return std::make_unique<MyObject>();  // Creates a new MyObject
});
```

Also add the include to `extensions_registry.h`:

```cpp
#include "objects/my_object.h"
```

## Step 4: Update Build System

Add your source file to the Makefile in the `core/` directory. Add `my_object.cpp` to the `SOURCES` variable:

```makefile
SOURCES += src/objects/my_object.cpp
```

## Step 5: Build and Test

1. **Build the core**:
   ```bash
   cd core
   make clean
   make
   ```

2. **Create test XML configuration**:
   ```xml
   <TrialConfig>
     <Objects>
       <Object id="test_object" type="my_object">
         <Position x="0.0" y="0.1" z="0.0"/>
         <Orientation x="0" y="0" z="0"/>
         <Color r="1.0" g="0.2" b="0.8"/>
         <Parameters size="0.05" stiffness="1500"/>
       </Object>
     </Objects>
     <!-- Add other required XML elements... -->
   </TrialConfig>
   ```

3. **Test your object**:
   ```bash
   # Start the server
   cd core/bin && ./core
   
   # In another terminal, run a trial
   source .venv/bin/activate
   python3 python_client/main.py config/my_test.xml test_subject test_trial
   ```

## Common Patterns

### Complex Geometry

For more complex shapes, you can load meshes from files:

```cpp
// Load from file
bool success = mesh->loadFromFile("path/to/model.obj");
if (!success) {
    std::cerr << "Failed to load mesh" << std::endl;
    // Fallback to primitive shape
    cCreateSphere(mesh, 0.05);
}
```

### Multi-Part Objects

Create compound objects with multiple meshes:

```cpp
cGenericObject* MyComplexObject::createObject(...) {
    // Create parent object
    cGenericObject* parent = new cGenericObject();
    parent->setLocalPos(x, y, z);
    
    // Create child meshes
    cMesh* part1 = new cMesh();
    cCreateBox(part1, 0.1, 0.1, 0.1);
    part1->setLocalPos(0.05, 0, 0);
    parent->addChild(part1);
    
    cMesh* part2 = new cMesh();
    cCreateSphere(part2, 0.03);
    part2->setLocalPos(-0.05, 0, 0);
    parent->addChild(part2);
    
    return parent;
}
```

### State Management

For objects that need to maintain state between updates:

```cpp
class StatefulObject : public CustomObjectInterface {
private:
    struct ObjectState {
        cVector3d velocity{0, 0, 0};
        double energy = 100.0;
        bool active = true;
    };
    
    // Use object pointer as key to maintain per-instance state
    std::unordered_map<cGenericObject*, ObjectState> _objectStates;
    mutable std::mutex _stateMutex;
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        std::lock_guard<std::mutex> lock(_stateMutex);
        
        auto& state = _objectStates[object];
        if (state.active) {
            // Update state...
            state.velocity.y() -= 9.81 * deltaTime;
            
            cVector3d newPos = object->getLocalPos() + state.velocity * deltaTime;
            object->setLocalPos(newPos);
        }
    }
};
```

## Best Practices

1. **Parameter Validation**: Always validate input parameters and provide sensible defaults
2. **Error Handling**: Log errors clearly but don't crash the simulation
3. **Performance**: Keep `updatePhysics()` lightweight - it's called at 1000Hz
4. **Memory Management**: Let CHAI3D and SimulationManager handle object cleanup
5. **Documentation**: Comment your parameters and their expected ranges
6. **Testing**: Create simple test configurations to verify your object works correctly

## Debugging Tips

- Use `std::cout` for debugging output during development
- Check the console output for registration messages
- Verify your object appears in the simulation window
- Test parameter parsing with various XML configurations
- Use simple geometry initially, then add complexity