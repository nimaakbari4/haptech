# Troubleshooting Custom Objects

Common issues and solutions for custom object development.

## Compilation Issues

### Object Not Found During Registration

**Error**: `Unknown type: my_object` in console output

**Causes**:
1. Object not registered in `ExtensionsRegistry`
2. Registration code not called
3. Typo in type name

**Solutions**:
```cpp
// 1. Verify registration in extensions_registry.cpp
registerCustomObject("my_object", []() -> std::unique_ptr<CustomObjectInterface> {
    return std::make_unique<MyObject>();
});

// 2. Check include in extensions_registry.h
#include "objects/my_object.h"

// 3. Verify type name matches XML
std::string getTypeName() const override { return "my_object"; }  // Must match XML type attribute
```

### Compilation Errors

**Error**: `undefined reference to MyObject::createObject`

**Causes**:
1. Source file not added to Makefile
2. Virtual methods not implemented
3. Header/source mismatch

**Solutions**:
```makefile
# Add to core/Makefile SOURCES
SOURCES += src/objects/my_object.cpp
```

```cpp
// Ensure all virtual methods are implemented
class MyObject : public CustomObjectInterface {
public:
    // Must implement all pure virtual methods
    cGenericObject* createObject(...) override;  // Required
    void updatePhysics(...) override;            // Required  
    std::string getTypeName() const override;    // Required
};
```

### Include Path Issues

**Error**: `fatal error: custom_object_interface.h: No such file or directory`

**Solution**:
```cpp
// Use correct include path from objects directory
#include "custom_object_interface.h"  // Correct
#include "../custom_object_interface.h"  // Wrong
```

## Runtime Issues

### Object Not Appearing in Simulation

**Symptoms**: XML loads without error, but object not visible

**Debugging Steps**:

1. **Check Console Output**:
   ```bash
   # Look for registration messages
   [CustomObjectRegistry] Registered type: my_object
   [DEBUG] Creating object using custom object system: my_object
   ```

2. **Verify Object Creation**:
   ```cpp
   cGenericObject* MyObject::createObject(...) {
       std::cout << "[MyObject] Creating object at (" << x << ", " << y << ", " << z << ")" << std::endl;
       
       cMesh* mesh = new cMesh();
       if (!mesh) {
           std::cerr << "[MyObject] Failed to create mesh!" << std::endl;
           return nullptr;
       }
       
       // Create geometry
       cCreateSphere(mesh, 0.05);
       std::cout << "[MyObject] Created sphere with " << mesh->getNumVertices() << " vertices" << std::endl;
       
       return mesh;
   }
   ```

3. **Check Object Position**:
   ```cpp
   // Verify position is within view
   mesh->setLocalPos(x, y, z);
   std::cout << "[MyObject] Position set to: " << mesh->getLocalPos().str(3) << std::endl;
   ```

### Object Appears But No Haptic Feedback

**Causes**:
1. Missing collision detector
2. Incorrect material properties
3. Object outside haptic workspace

**Solutions**:
```cpp
cGenericObject* MyObject::createObject(...) {
    cMesh* mesh = new cMesh();
    
    // 1. Create collision detector
    double toolRadius = 0.01;
    mesh->createAABBCollisionDetector(toolRadius);
    
    // 2. Set haptic material properties
    mesh->m_material->setStiffness(1000.0);         // Must be > 0
    mesh->m_material->setHapticTriangleSides(true, false);  // Enable front-side haptics
    
    // 3. Verify position is within haptic workspace (~20cm cube)
    if (std::abs(x) > 0.1 || std::abs(y) > 0.1 || std::abs(z) > 0.1) {
        std::cout << "[WARNING] Object may be outside haptic workspace" << std::endl;
    }
    
    return mesh;
}
```

### Physics Updates Not Working

**Symptoms**: `updatePhysics()` called but object doesn't move

**Debugging**:
```cpp
void MyObject::updatePhysics(cGenericObject* object, double deltaTime) {
    static int updateCount = 0;
    if (updateCount++ % 60 == 0) {  // Log every second at 60 FPS
        std::cout << "[MyObject] Physics update: deltaTime=" << deltaTime 
                  << ", position=" << object->getLocalPos().str(3) << std::endl;
    }
    
    // Check if object pointer is valid
    if (!object) {
        std::cerr << "[MyObject] Null object pointer in updatePhysics!" << std::endl;
        return;
    }
    
    // Verify position changes
    static cVector3d lastPos = object->getLocalPos();
    cVector3d currentPos = object->getLocalPos();
    
    // Simple movement test
    currentPos.y() += 0.001;  // Move up slowly
    object->setLocalPos(currentPos);
    
    if ((currentPos - lastPos).length() > 0.0001) {
        std::cout << "[MyObject] Position changed: " << (currentPos - lastPos).str(6) << std::endl;
    }
    lastPos = currentPos;
}
```

## Parameter Issues

### Invalid Parameter Values

**Error**: Object behaves unexpectedly with certain XML parameters

**Debugging**:
```cpp
double MyObject::getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");
    if (it != params.end()) {
        std::cout << "[MyObject] Raw radius parameter: '" << it->second << "'" << std::endl;
        
        try {
            double value = std::stod(it->second);
            std::cout << "[MyObject] Parsed radius value: " << value << std::endl;
            
            if (value <= 0.0) {
                std::cerr << "[MyObject] Invalid radius value: " << value << ", using default" << std::endl;
                return 0.05;
            }
            
            if (value > 1.0) {
                std::cerr << "[MyObject] Radius too large: " << value << ", clamping to 1.0" << std::endl;
                return 1.0;
            }
            
            return value;
        } catch (const std::exception& e) {
            std::cerr << "[MyObject] Parameter parsing error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "[MyObject] No radius parameter found, using default" << std::endl;
    }
    return 0.05;
}
```

### Parameter Not Being Read

**Check XML syntax**:
```xml
<!-- Correct -->
<Parameters radius="0.05" stiffness="1000"/>

<!-- Wrong - not in Parameters element -->
<radius>0.05</radius>
<stiffness>1000</stiffness>

<!-- Wrong - case sensitive -->
<Parameters Radius="0.05" Stiffness="1000"/>
```

## Performance Issues

### Simulation Running Slowly

**Symptoms**: Low frame rate, choppy haptics

**Causes and Solutions**:

1. **Heavy Physics Calculations**:
   ```cpp
   void updatePhysics(cGenericObject* object, double deltaTime) {
       // Avoid expensive operations every frame
       static double accumulator = 0.0;
       accumulator += deltaTime;
       
       if (accumulator > 0.016) {  // Update at ~60Hz instead of every frame
           // Heavy calculation here
           accumulator = 0.0;
       }
   }
   ```

2. **Complex Geometry**:
   ```cpp
   // Use LOD (Level of Detail)
   int getResolution(const std::map<std::string, std::string>& params) const {
       // Lower resolution for better performance
       return std::min(32, std::max(8, resolution));  // Clamp to reasonable range
   }
   ```

3. **Too Many Objects**:
   - Check if creating objects unnecessarily
   - Consider object pooling for dynamic objects

### Memory Leaks

**Symptoms**: Memory usage grows over time

**Common Causes**:
```cpp
cGenericObject* MyObject::createObject(...) {
    cMesh* mesh = new cMesh();
    
    // Don't do this - creates memory leak
    // cMesh* extraMesh = new cMesh();  // Never added to scene or deleted
    
    // Correct - add all created objects to the hierarchy
    cMesh* child = new cMesh();
    mesh->addChild(child);  // Now managed by parent
    
    return mesh;  // SimulationManager will handle deletion
}
```

## State Management Issues

### Multiple Object Instances Interfering

**Problem**: State shared between different object instances

**Wrong**:
```cpp
class BadObject : public CustomObjectInterface {
private:
    static double sharedVelocity;  // Shared between all instances!
};
```

**Correct**:
```cpp
class GoodObject : public CustomObjectInterface {
private:
    std::unordered_map<cGenericObject*, ObjectState> objectStates;  // Per-instance state
};
```

### Thread Safety Issues

**Symptoms**: Crashes or corrupted state when multiple objects exist

**Solution**:
```cpp
class ThreadSafeObject : public CustomObjectInterface {
private:
    mutable std::mutex stateMutex;
    std::unordered_map<cGenericObject*, ObjectState> objectStates;
    
public:
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        std::lock_guard<std::mutex> lock(stateMutex);
        // Thread-safe access to state
    }
};
```

## Visual Issues

### Object Appears Black/Wrong Color

**Causes**:
1. No lighting setup
2. Normal vectors incorrect
3. Material properties not set

**Solutions**:
```cpp
cGenericObject* MyObject::createObject(...) {
    cMesh* mesh = new cMesh();
    cCreateSphere(mesh, radius);
    
    // Set color explicitly
    mesh->m_material->setColor(r, g, b);
    
    // Ensure proper lighting
    mesh->setUseMaterial(true);
    mesh->setUseTransparency(false);
    
    // Compute normals for proper lighting
    mesh->computeAllNormals();
    
    return mesh;
}
```

### Object Flickers or Disappears

**Causes**:
1. Z-fighting with other objects
2. Normals facing wrong direction
3. Object moving outside view frustum

**Solutions**:
```cpp
// 1. Slight position offset to avoid z-fighting
cVector3d offset(0.001, 0, 0);
mesh->setLocalPos(x + offset.x(), y + offset.y(), z + offset.z());

// 2. Check normal direction
mesh->computeAllNormals();
if (normalsInward) {
    mesh->reverseAllNormals();
}

// 3. Keep object in reasonable bounds
cVector3d pos = object->getLocalPos();
if (pos.length() > 10.0) {  // Too far from origin
    object->setLocalPos(cVector3d(0, 0, 0));  // Reset position
}
```

## Debugging Tools

### Logging Helper

```cpp
class ObjectLogger {
public:
    static void logObjectInfo(cGenericObject* obj, const std::string& context) {
        if (!obj) {
            std::cout << "[" << context << "] NULL object pointer" << std::endl;
            return;
        }
        
        std::cout << "[" << context << "] Object info:" << std::endl;
        std::cout << "  Position: " << obj->getLocalPos().str(3) << std::endl;
        std::cout << "  Vertices: " << obj->getNumVertices() << std::endl;
        std::cout << "  Children: " << obj->getNumChildren() << std::endl;
        
        if (obj->m_material) {
            std::cout << "  Stiffness: " << obj->m_material->getStiffness() << std::endl;
            std::cout << "  Color: " << obj->m_material->getColor().str(3) << std::endl;
        }
    }
};
```

### Performance Profiling

```cpp
class SimpleProfiler {
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::string name;
    
public:
    SimpleProfiler(const std::string& n) : name(n), startTime(std::chrono::high_resolution_clock::now()) {}
    
    ~SimpleProfiler() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        std::cout << "[PROFILE] " << name << " took " << duration.count() << " μs" << std::endl;
    }
};

// Usage
void updatePhysics(cGenericObject* object, double deltaTime) override {
    SimpleProfiler prof("MyObject::updatePhysics");
    // ... your code ...
}
```

## Getting Help

### Collecting Debug Information

When reporting issues, include:

1. **Console Output**: Complete log from application start
2. **XML Configuration**: The exact XML that causes problems
3. **Object Code**: Relevant parts of your custom object implementation
4. **System Info**: OS, compiler version, hardware
5. **Reproduction Steps**: Minimal steps to recreate the issue

### Testing Methodology

```cpp
// Create minimal test case
class TestObject : public CustomObjectInterface {
public:
    cGenericObject* createObject(...) override {
        std::cout << "[TEST] Creating simple test object" << std::endl;
        cMesh* mesh = new cMesh();
        cCreateSphere(mesh, 0.05);
        mesh->setLocalPos(x, y, z);
        mesh->m_material->setColor(1.0, 0.0, 0.0);  // Bright red
        mesh->m_material->setStiffness(1000.0);
        mesh->createAABBCollisionDetector(0.01);
        return mesh;
    }
    
    void updatePhysics(cGenericObject* object, double deltaTime) override {
        // Minimal physics - just log
        static int count = 0;
        if (++count % 60 == 0) {
            std::cout << "[TEST] Physics update called" << std::endl;
        }
    }
    
    std::string getTypeName() const override { return "test"; }
};
```

By following these troubleshooting steps and debugging techniques, you should be able to identify and resolve most custom object development issues. Remember to start simple and gradually add complexity once basic functionality is working.