# Objects

Complete guide to configuring virtual objects in XML trial configurations.

## Overview

Virtual objects define the haptic and visual elements that participants interact with during trials. Objects can provide haptic feedback when touched, serve as targets or obstacles, and help create realistic experimental environments.

**Key concepts**:
- **Object types**: Different shapes (sphere, box, cylinder, ball, torus)
- **Positioning**: 3D coordinates and orientation in space
- **Haptic properties**: Stiffness, damping, friction for realistic touch
- **Visual appearance**: Colors, transparency, visibility
- **Physical properties**: Mass, elasticity for dynamic behavior

## Basic Object Structure

All objects share this basic XML structure:

```xml
<Objects>
    <Object id="unique_name" type="object_type">
        <Position x="0.1" y="0.0" z="0.0"/>
        <Orientation roll="0" pitch="0" yaw="0"/>
        <Color r="1.0" g="0.0" b="0.0"/>
        <Parameters><!-- Object-specific parameters --></Parameters>
    </Object>
</Objects>
```

### Required Elements
- **id**: Unique identifier for the object (used in conditions and scripts)
- **type**: Object shape type (sphere, box, cylinder, ball, torus)
- **Position**: 3D coordinates in meters
- **Parameters**: Object-specific size and properties

### Optional Elements
- **Orientation**: Rotation in degrees (roll, pitch, yaw)
- **Color**: RGB values from 0.0 to 1.0
- **Visibility**: Whether object is initially visible
- **Physics properties**: Stiffness, damping, mass, friction

## Object Types

### Sphere

Perfect for target objects and simple haptic feedback.

```xml
<Object id="target_sphere" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Color r="1.0" g="0.0" b="0.0"/>  <!-- Red -->
    <Parameters radius="0.03"/>  <!-- 3cm radius -->
</Object>
```

**Parameters**:
- `radius`: Sphere radius in meters (required)
- `stiffness`: Haptic stiffness (N/m, default: 1000)
- `damping`: Haptic damping (Ns/m, default: 50)

### Box

Rectangular objects, useful for obstacles and platforms.

```xml
<Object id="obstacle_box" type="box">
    <Position x="0.0" y="0.2" z="0.0"/>
    <Orientation roll="0" pitch="0" yaw="45"/>  <!-- Rotated 45 degrees -->
    <Color r="0.0" g="0.0" b="1.0"/>  <!-- Blue -->
    <Parameters width="0.05" height="0.05" depth="0.05"/>  <!-- 5cm cube -->
</Object>
```

**Parameters**:
- `width`: Width along X-axis in meters (required)
- `height`: Height along Y-axis in meters (required) 
- `depth`: Depth along Z-axis in meters (required)
- `stiffness`: Haptic stiffness (N/m, default: 1000)
- `damping`: Haptic damping (Ns/m, default: 50)

### Cylinder

Cylindrical objects for varied haptic experiences.

```xml
<Object id="cylinder_target" type="cylinder">
    <Position x="-0.1" y="0.0" z="0.0"/>
    <Color r="0.0" g="1.0" b="0.0"/>  <!-- Green -->
    <Parameters radius="0.04" height="0.08"/>  <!-- 4cm radius, 8cm tall -->
</Object>
```

**Parameters**:
- `radius`: Cylinder radius in meters (required)
- `height`: Cylinder height in meters (required)
- `stiffness`: Haptic stiffness (N/m, default: 1000)
- `damping`: Haptic damping (Ns/m, default: 50)

### Ball

Dynamic spheres that can move and respond to forces.

```xml
<Object id="movable_ball" type="ball">
    <Position x="0.0" y="0.0" z="0.1"/>
    <Color r="1.0" g="1.0" b="0.0"/>  <!-- Yellow -->
    <Parameters radius="0.025" mass="0.1" elasticity="0.8"/>
</Object>
```

**Parameters**:
- `radius`: Ball radius in meters (required)
- `mass`: Ball mass in kg (default: 0.05)
- `elasticity`: Bounce factor 0.0-1.0 (default: 0.5)
- `friction`: Rolling friction (default: 0.1)

### Torus

Donut-shaped objects for advanced interactions.

```xml
<Object id="ring_target" type="torus">
    <Position x="0.0" y="0.1" z="0.0"/>
    <Color r="0.8" g="0.0" b="0.8"/>  <!-- Purple -->
    <Parameters major_radius="0.06" minor_radius="0.02"/>
</Object>
```

**Parameters**:
- `major_radius`: Distance from center to tube center in meters (required)
- `minor_radius`: Radius of the tube in meters (required)
- `stiffness`: Haptic stiffness (N/m, default: 1000)
- `damping`: Haptic damping (Ns/m, default: 50)

## Positioning and Orientation

### Coordinate System

The haptic workspace uses a right-handed coordinate system:
- **X-axis**: Left (-) to Right (+)
- **Y-axis**: Back (-) to Front (+) 
- **Z-axis**: Down (-) to Up (+)

**Workspace limits**: Typically [-0.3, 0.3] meters in each direction

### Position Examples

```xml
<!-- Object directly in front of user -->
<Position x="0.0" y="0.15" z="0.0"/>

<!-- Object to the right and above -->
<Position x="0.1" y="0.0" z="0.05"/>

<!-- Object behind and to the left -->
<Position x="-0.08" y="-0.12" z="0.0"/>
```

### Orientation (Euler Angles)

```xml
<!-- No rotation -->
<Orientation roll="0" pitch="0" yaw="0"/>

<!-- Rotated 45 degrees around Z-axis -->
<Orientation roll="0" pitch="0" yaw="45"/>

<!-- Tilted forward 30 degrees -->
<Orientation roll="0" pitch="30" yaw="0"/>
```

## Visual Properties

### Colors

RGB values from 0.0 (black) to 1.0 (full intensity):

```xml
<!-- Primary colors -->
<Color r="1.0" g="0.0" b="0.0"/>  <!-- Red -->
<Color r="0.0" g="1.0" b="0.0"/>  <!-- Green -->
<Color r="0.0" g="0.0" b="1.0"/>  <!-- Blue -->

<!-- Mixed colors -->
<Color r="1.0" g="1.0" b="0.0"/>  <!-- Yellow -->
<Color r="0.5" g="0.0" b="0.5"/>  <!-- Purple -->
<Color r="0.8" g="0.8" b="0.8"/>  <!-- Light gray -->
```

### Visibility Control

```xml
<!-- Object starts invisible, can be shown later -->
<Object id="hidden_target" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Color r="1.0" g="0.0" b="0.0"/>
    <Parameters radius="0.03"/>
    <Visible>false</Visible>
</Object>
```

## Haptic Properties

### Stiffness

Controls how "hard" an object feels:
- **Low (100-500 N/m)**: Soft, spongy feel
- **Medium (500-1500 N/m)**: Normal solid objects
- **High (1500+ N/m)**: Very rigid, hard surfaces

```xml
<Parameters radius="0.03" stiffness="1200" damping="75"/>
```

### Damping

Controls resistance to motion:
- **Low (10-30 Ns/m)**: Little resistance, easy to move through
- **Medium (30-100 Ns/m)**: Normal resistance
- **High (100+ Ns/m)**: High resistance, viscous feel

### Advanced Haptic Properties

```xml
<Object id="complex_object" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Parameters 
        radius="0.04"
        stiffness="800"     <!-- Medium stiffness -->
        damping="60"       <!-- Medium damping -->
        static_friction="0.3"  <!-- Surface friction -->
        dynamic_friction="0.2" <!-- Moving friction -->
        texture_stiffness="200" <!-- Surface texture -->
    />
</Object>
```

## Multiple Objects

### Object Arrays

Create multiple similar objects efficiently:

```xml
<Objects>
    <!-- Row of targets -->
    <Object id="target_1" type="sphere">
        <Position x="0.05" y="0.1" z="0.0"/>
        <Color r="1.0" g="0.0" b="0.0"/>
        <Parameters radius="0.02"/>
    </Object>
    
    <Object id="target_2" type="sphere">
        <Position x="0.1" y="0.1" z="0.0"/>
        <Color r="1.0" g="0.0" b="0.0"/>
        <Parameters radius="0.02"/>
    </Object>
    
    <Object id="target_3" type="sphere">
        <Position x="0.15" y="0.1" z="0.0"/>
        <Color r="1.0" g="0.0" b="0.0"/>
        <Parameters radius="0.02"/>
    </Object>
</Objects>
```

### Object Interactions

Combine different object types:

```xml
<Objects>
    <!-- Target to reach -->
    <Object id="goal" type="sphere">
        <Position x="0.15" y="0.0" z="0.0"/>
        <Color r="0.0" g="1.0" b="0.0"/>
        <Parameters radius="0.03" stiffness="500"/>
    </Object>
    
    <!-- Obstacle to avoid -->
    <Object id="barrier" type="box">
        <Position x="0.075" y="0.0" z="0.0"/>
        <Color r="1.0" g="0.0" b="0.0"/>
        <Parameters width="0.02" height="0.06" depth="0.08" stiffness="2000"/>
    </Object>
    
    <!-- Platform to rest on -->
    <Object id="platform" type="box">
        <Position x="0.0" y="0.0" z="-0.04"/>
        <Color r="0.5" g="0.5" b="0.5"/>
        <Parameters width="0.3" height="0.3" depth="0.02" stiffness="1500"/>
    </Object>
</Objects>
```

## Common Patterns

### Simple Target Task

```xml
<!-- Single sphere target -->
<Objects>
    <Object id="target" type="sphere">
        <Position x="0.12" y="0.0" z="0.0"/>
        <Color r="0.0" g="1.0" b="0.0"/>  <!-- Green target -->
        <Parameters radius="0.025" stiffness="800"/>
    </Object>
</Objects>
```

### Path Following

```xml
<!-- Series of waypoint spheres -->
<Objects>
    <Object id="waypoint_1" type="sphere">
        <Position x="0.05" y="0.0" z="0.0"/>
        <Color r="1.0" g="1.0" b="0.0"/>  <!-- Yellow -->
        <Parameters radius="0.015"/>
    </Object>
    
    <Object id="waypoint_2" type="sphere">
        <Position x="0.1" y="0.05" z="0.0"/>
        <Color r="1.0" g="1.0" b="0.0"/>
        <Parameters radius="0.015"/>
    </Object>
    
    <Object id="final_target" type="sphere">
        <Position x="0.15" y="0.1" z="0.0"/>
        <Color r="0.0" g="1.0" b="0.0"/>  <!-- Green finish -->
        <Parameters radius="0.02"/>
    </Object>
</Objects>
```

### Obstacle Avoidance

```xml
<!-- Movable ball with obstacles -->
<Objects>
    <!-- Ball to push -->
    <Object id="ball" type="ball">
        <Position x="0.0" y="0.0" z="0.0"/>
        <Color r="0.0" g="0.5" b="1.0"/>
        <Parameters radius="0.02" mass="0.05" elasticity="0.6"/>
    </Object>
    
    <!-- Obstacles -->
    <Object id="wall_1" type="box">
        <Position x="0.05" y="0.0" z="0.0"/>
        <Color r="0.8" g="0.0" b="0.0"/>
        <Parameters width="0.01" height="0.05" depth="0.1" stiffness="3000"/>
    </Object>
    
    <!-- Goal zone -->
    <Object id="goal_zone" type="box">
        <Position x="0.15" y="0.0" z="0.0"/>
        <Color r="0.0" g="0.8" b="0.0"/>
        <Parameters width="0.04" height="0.04" depth="0.02" stiffness="200"/>
    </Object>
</Objects>
```

## Best Practices

### Object Positioning
- **Stay within workspace**: Keep objects within [-0.3, 0.3] meters
- **Comfortable reach**: Place targets at comfortable reaching distances
- **Clear spacing**: Leave adequate space between objects (>3cm)
- **Height considerations**: Avoid positions that require awkward hand positions

### Haptic Properties
- **Start with defaults**: Use default stiffness/damping first, then adjust
- **Test thoroughly**: Physical properties greatly affect user experience
- **Consider object size**: Larger objects can have lower stiffness
- **Match expectations**: Hard-looking objects should feel hard

### Visual Design
- **High contrast**: Use colors that stand out from background
- **Consistent coding**: Use consistent colors for similar object types
- **Accessibility**: Consider colorblind users (avoid red/green only)
- **Size visibility**: Ensure objects are large enough to see clearly

### Performance
- **Limit object count**: Keep total objects under 20 for best performance
- **Optimize complexity**: Simple shapes perform better than complex ones
- **LOD considerations**: Use appropriately sized objects for the task

## Troubleshooting

### Common Issues

**Object not visible**:
```
Problem: Object defined but not appearing
Solution: Check Position is within workspace bounds, verify Color values
```

**Poor haptic feedback**:
```
Problem: Object feels wrong when touched
Solution: Adjust stiffness and damping parameters, check object size
```

**Objects overlapping**:
```
Problem: Multiple objects occupy same space
Solution: Check Position coordinates, ensure adequate spacing
```

**Performance issues**:
```
Problem: System running slowly with objects
Solution: Reduce object count, simplify haptic properties, check for unnecessary objects
```

### Validation Tips

- **Position check**: Verify all coordinates are within [-0.3, 0.3]
- **Size check**: Ensure object dimensions are reasonable (1mm to 20cm)
- **Color check**: Confirm RGB values are between 0.0 and 1.0
- **ID uniqueness**: Verify all object IDs are unique within the trial
- **Required parameters**: Check that all object types have their required parameters

## Next Steps

- **Trial Conditions**: Learn how to use objects in [trial conditions](trial-conditions.md)
- **Scene Settings**: Configure [scene appearance](scene-settings.md) to complement your objects
- **Data Collection**: Set up [data collection](data-collection.md) to track object interactions
- **Examples**: Browse practical examples in the examples directory