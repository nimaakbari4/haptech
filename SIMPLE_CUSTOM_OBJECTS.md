# Simple Custom Objects for Haptech

This document explains how researchers can add custom object types to the Haptech simulation system.

## How It Works

The system extends the existing `addObjectFromConfig` function in `simulation_manager.cpp`. Researchers can add new object types by simply adding new `else if` blocks to the existing function.

## Current Custom Object Types

### Ball Object
- **Type**: `ball`
- **Parameters**: `radius` (required), `bounciness` (optional, 0.0-1.0)
- **Description**: A bouncy sphere with configurable physics

### Cup Object  
- **Type**: `cup`
- **Parameters**: `outer_radius` (required), `height` (required)
- **Description**: A hollow cylinder for catching objects

### Spring Object
- **Type**: `spring`
- **Parameters**: `radius` (required), `height` (required), `stiffness` (optional)
- **Description**: A spring-like object with configurable stiffness

## How to Add Your Own Custom Object

### Step 1: Add the Object Type
In `core/src/simulation_manager.cpp`, find the `addObjectFromConfig` function and add a new `else if` block:

```cpp
else if (type == "your_object_type") {
    // Get required parameters
    auto param1It = sizeParams.find("param1");
    if (param1It == sizeParams.end()) {
        std::cout << "[ERROR] Your object requires 'param1' parameter." << std::endl;
        return false;
    }
    
    // Create your object geometry
    cCreateSphere(mesh.get(), param1It->second); // or cCreateBox, cCreateCylinder, etc.
    mesh->createAABBCollisionDetector(param1It->second);
    
    // Set optional parameters
    auto optionalIt = sizeParams.find("optional_param");
    if (optionalIt != sizeParams.end()) {
        // Configure your object based on the parameter
        mesh->m_material->setStiffness(optionalIt->second);
    }
}
```

### Step 2: Use in XML Configuration
```xml
<Object id="my_object" type="your_object_type">
  <Position x="0.0" y="0.0" z="0.0"/>
  <Size param1="0.1"/>
  <Parameters>
    <Parameter name="optional_param" value="1000.0"/>
  </Parameters>
  <Color r="1.0" g="0.0" b="0.0"/>
</Object>
```

## Example: Adding a "Wobble" Object

Here's how you'd add a wobbling object:

```cpp
else if (type == "wobble") {
    auto radiusIt = sizeParams.find("radius");
    if (radiusIt == sizeParams.end()) {
        std::cout << "[ERROR] Wobble requires 'radius' parameter." << std::endl;
        return false;
    }
    
    cCreateSphere(mesh.get(), radiusIt->second);
    mesh->createAABBCollisionDetector(radiusIt->second);
    
    // Set wobble properties
    auto wobbleIt = sizeParams.find("wobble_strength");
    if (wobbleIt != sizeParams.end()) {
        mesh->m_material->setDamping(10.0 * wobbleIt->second); // More wobble = less damping
    }
}
```

Then use it in XML:
```xml
<Object id="wobbly_ball" type="wobble">
  <Position x="0.0" y="0.0" z="0.0"/>
  <Size radius="0.05"/>
  <Parameters>
    <Parameter name="wobble_strength" value="2.0"/>
  </Parameters>
  <Color r="1.0" g="1.0" b="0.0"/>
</Object>
```

## Benefits of This Approach

1. **Simple**: Just add one `else if` block
2. **No new files**: Uses existing architecture
3. **Easy to understand**: Follows the same pattern as built-in objects
4. **Flexible**: Can add any parameters you need
5. **Integrated**: Works with existing XML system

## Testing Your Custom Objects

1. Add your object type to `simulation_manager.cpp`
2. Create an XML configuration file
3. Compile: `cd core && make`
4. Run: `cd core/bin && ./run_core.sh`
5. Test with Python client using your XML file

## Tips for Researchers

- **Start simple**: Begin with basic geometry (sphere, box, cylinder)
- **Add parameters gradually**: Start with one parameter, then add more
- **Use descriptive names**: Make your object type names clear
- **Test thoroughly**: Make sure your objects work with the haptic system
- **Document parameters**: Add comments explaining what each parameter does

This system makes it easy for researchers to extend the simulation without complex architecture changes!
