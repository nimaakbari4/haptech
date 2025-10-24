# Built-in Objects as Custom Objects

## Summary

This refactoring moved the built-in object creation logic from `SimulationManager::addObjectFromConfig()` into separate custom object classes. This provides better code organization and allows the built-in objects to be extended and customized like any other custom object.

## Changes Made

### New Custom Object Classes Created

1. **SphereObject** (`src/objects/sphere_object.h/cpp`)
   - Type: `"sphere"`
   - Parameters: `radius` (required)
   - Creates a basic sphere mesh

2. **BoxObject** (`src/objects/box_object.h/cpp`)
   - Type: `"box"`  
   - Parameters: `width`, `height`, `depth` (all required)
   - Creates a rectangular box mesh

3. **CylinderObject** (`src/objects/cylinder_object.h/cpp`)
   - Type: `"cylinder"`
   - Parameters: `radius`, `height` (both required)
   - Creates a cylindrical mesh

4. **BallObject** (`src/objects/ball_object.h/cpp`)
   - Type: `"ball"`
   - Parameters: `radius` (required), `bounciness` (optional, default 0.5)
   - Creates a sphere with customizable bounciness via damping

### Registry Updates

- Updated `extensions_registry.h` to include the new object headers
- Updated `extensions_registry.cpp` to register all four new object types in `registerAllExtensions()`

### SimulationManager Simplification

- Simplified `SimulationManager::addObjectFromConfig()` to only use the custom object system
- Removed the fallback built-in object creation logic since these are now registered as custom objects
- All object types now go through the same unified custom object creation path

## Benefits

1. **Consistency**: All objects (built-in and custom) now use the same creation system
2. **Extensibility**: Built-in objects can now be easily extended or customized
3. **Maintainability**: Object creation logic is separated into focused, single-responsibility classes
4. **Flexibility**: Each object type can have its own specific parameters and error handling

## Usage

The XML configuration syntax remains unchanged. Objects are still created with:

```xml
<object id="my_sphere" type="sphere" x="0" y="0" z="0" r="1" g="0" b="0">
    <param name="radius" value="0.1"/>
</object>

<object id="my_box" type="box" x="0.2" y="0" z="0" r="0" g="1" b="0">
    <param name="width" value="0.1"/>
    <param name="height" value="0.1"/>
    <param name="depth" value="0.1"/>
</object>

<object id="my_ball" type="ball" x="0" y="0.2" z="0" r="0" g="0" b="1">
    <param name="radius" value="0.05"/>
    <param name="bounciness" value="0.8"/>
</object>
```

The functionality is identical, but the underlying implementation is now more modular and extensible.