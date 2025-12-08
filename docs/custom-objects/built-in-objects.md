# Built-in Objects

Documentation for the custom objects included with the system.

## Overview

The haptic system includes several built-in objects that demonstrate the CustomObjectInterface and provide commonly used haptic primitives. These objects serve as both useful tools and examples for creating your own custom objects.

## Basic Geometric Objects

### Sphere Object

**Type**: `sphere`  
**File**: `core/src/objects/sphere_object.h`

A simple sphere with configurable radius and haptic properties.

**Parameters**:
- `radius` (double, default: 0.05) - Sphere radius in meters

**XML Example**:
```xml
<Object id="ball" type="sphere">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="1.0" g="0.0" b="0.0"/>
  <Parameters radius="0.03"/>
</Object>
```

### Box Object

**Type**: `box`  
**File**: `core/src/objects/box_object.h`

A rectangular box with configurable dimensions.

**Parameters**:
- `width` (double, default: 0.1) - Width in meters (X-axis)
- `height` (double, default: 0.1) - Height in meters (Y-axis) 
- `depth` (double, default: 0.1) - Depth in meters (Z-axis)

**XML Example**:
```xml
<Object id="block" type="box">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="0.0" g="1.0" b="0.0"/>
  <Parameters width="0.08" height="0.05" depth="0.12"/>
</Object>
```

### Cylinder Object

**Type**: `cylinder`  
**File**: `core/src/objects/cylinder_object.h`

A cylindrical object with configurable radius and height.

**Parameters**:
- `radius` (double, default: 0.05) - Cylinder radius in meters
- `height` (double, default: 0.1) - Cylinder height in meters

**XML Example**:
```xml
<Object id="pillar" type="cylinder">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="0.0" g="0.0" b="1.0"/>
  <Parameters radius="0.04" height="0.15"/>
</Object>
```

## Advanced Geometric Objects

### Torus Object

**Type**: `torus`  
**File**: `core/src/objects/torus_object.h`

A torus (doughnut shape) with configurable inner and outer radii.

**Parameters**:
- `inner_radius` (double, default: 0.02) - Inner radius in meters
- `outer_radius` (double, default: 0.05) - Outer radius in meters
- `num_sides` (int, default: 16) - Number of sides for mesh resolution
- `num_rings` (int, default: 16) - Number of rings for mesh resolution

**XML Example**:
```xml
<Object id="ring" type="torus">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="1.0" g="1.0" b="0.0"/>
  <Parameters inner_radius="0.03" outer_radius="0.07" num_sides="20" num_rings="20"/>
</Object>
```

### Ellipsoid Object

**Type**: `ellipsoid`  
**File**: `core/src/objects/ellipsoid_object.h`

An ellipsoid with configurable radii along each axis.

**Parameters**:
- `radius_x` (double, default: 0.05) - X-axis radius in meters
- `radius_y` (double, default: 0.05) - Y-axis radius in meters
- `radius_z` (double, default: 0.05) - Z-axis radius in meters
- `num_slices` (int, default: 16) - Number of slices for mesh resolution
- `num_stacks` (int, default: 16) - Number of stacks for mesh resolution

**XML Example**:
```xml
<Object id="oval" type="ellipsoid">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="1.0" g="0.5" b="0.0"/>
  <Parameters radius_x="0.08" radius_y="0.04" radius_z="0.06"/>
</Object>
```

### Line Object

**Type**: `line`  
**File**: `core/src/objects/line_object.h`

A line segment with configurable endpoints and thickness.

**Parameters**:
- `end_x` (double, default: 0.1) - End point X coordinate relative to start
- `end_y` (double, default: 0.0) - End point Y coordinate relative to start
- `end_z` (double, default: 0.0) - End point Z coordinate relative to start
- `thickness` (double, default: 0.005) - Line thickness in meters

**XML Example**:
```xml
<Object id="beam" type="line">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="0.5" g="0.5" b="0.5"/>
  <Parameters end_x="0.15" end_y="0.0" end_z="0.05" thickness="0.008"/>
</Object>
```

## Physics-Enhanced Objects

### Ball Object

**Type**: `ball`  
**File**: `core/src/objects/ball_object.h`

A sphere with realistic physics including gravity, bouncing, and momentum.

**Parameters**:
- `radius` (double, default: 0.05) - Ball radius in meters
- `mass` (double, default: 1.0) - Mass in kg (affects momentum)
- `bounciness` (double, default: 0.5) - Coefficient of restitution (0-1)
- `gravity_scale` (double, default: 1.0) - Gravity multiplier

**Physics**: The ball falls under gravity, bounces off the ground, and maintains velocity between contacts.

**XML Example**:
```xml
<Object id="bouncy_ball" type="ball">
  <Position x="0.0" y="0.2" z="0.0"/>
  <Color r="1.0" g="0.2" b="0.2"/>
  <Parameters radius="0.04" mass="0.5" bounciness="0.8" gravity_scale="1.5"/>
</Object>
```

## Example Custom Objects

### Spring Object

**Type**: `spring`  
**File**: `core/src/objects/spring_object.h`

A spring that provides force feedback based on compression/extension.

**Parameters**:
- `stiffness` (double, default: 1000.0) - Spring constant (N/m)
- `damping` (double, default: 10.0) - Damping coefficient
- `radius` (double, default: 0.02) - Spring coil radius
- `height` (double, default: 0.1) - Relaxed spring height

**Physics**: Generates force proportional to displacement from rest position.

**XML Example**:
```xml
<Object id="shock_absorber" type="spring">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="0.7" g="0.7" b="0.7"/>
  <Parameters stiffness="1500.0" damping="15.0" radius="0.025" height="0.12"/>
</Object>
```

### Cup Object

**Type**: `cup`  
**File**: `core/src/objects/cup_object.h`

A hollow cup/container with configurable dimensions.

**Parameters**:
- `inner_radius` (double, default: 0.04) - Inner radius in meters
- `outer_radius` (double, default: 0.05) - Outer radius in meters  
- `height` (double, default: 0.08) - Cup height in meters
- `thickness` (double, default: 0.01) - Wall thickness in meters

**XML Example**:
```xml
<Object id="container" type="cup">
  <Position x="0.0" y="0.05" z="0.0"/>
  <Color r="0.8" g="0.6" b="0.4"/>
  <Parameters inner_radius="0.035" outer_radius="0.045" height="0.1" thickness="0.01"/>
</Object>
```

## Game Objects

### Pong Ball Object

**Type**: `pong_ball`  
**File**: `core/src/objects/pong_ball_object.h`

A ball specifically designed for pong-style games with controlled physics.

**Parameters**:
- `radius` (double, default: 0.02) - Ball radius
- `speed` (double, default: 0.1) - Movement speed in m/s
- `bounce_damping` (double, default: 0.95) - Velocity retention on bounce

**XML Example**:
```xml
<Object id="game_ball" type="pong_ball">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="1.0" g="1.0" b="1.0"/>
  <Parameters radius="0.015" speed="0.12" bounce_damping="0.9"/>
</Object>
```

### Pong Paddle Object

**Type**: `pong_paddle`  
**File**: `core/src/objects/pong_paddle_object.h`

A paddle for pong-style games that can interact with the pong ball.

**Parameters**:
- `width` (double, default: 0.1) - Paddle width
- `height` (double, default: 0.02) - Paddle height
- `depth` (double, default: 0.01) - Paddle thickness

**XML Example**:
```xml
<Object id="player_paddle" type="pong_paddle">
  <Position x="0.0" y="0.1" z="0.2"/>
  <Color r="0.2" g="0.8" b="0.2"/>
  <Parameters width="0.12" height="0.025" depth="0.008"/>
</Object>
```

## Usage Notes

### Common Parameters

All objects support these standard properties set via XML:
- **Position**: `x`, `y`, `z` coordinates in meters
- **Orientation**: Rotation in degrees around `x`, `y`, `z` axes
- **Color**: RGB values (0.0-1.0 range)

### Haptic Properties

Most objects automatically configure reasonable haptic properties:
- **Stiffness**: Typically 1000 N/m for solid feel
- **Friction**: Static (0.3) and dynamic (0.2) friction coefficients
- **Collision Detection**: AABB collision detection enabled by default

### Performance Considerations

- Complex geometry (high polygon count) may affect haptic rendering performance
- Physics-enabled objects (ball, spring) use more CPU in `updatePhysics()`
- Use appropriate mesh resolution for your needs - higher resolution looks better but costs more

### Extending Built-in Objects

You can use built-in objects as starting points for your own custom objects:
1. Copy the header and source files
2. Rename the class and change the type name
3. Modify the behavior as needed
4. Register with a new type name

This approach is especially useful for creating variations of existing objects with specialized behavior.