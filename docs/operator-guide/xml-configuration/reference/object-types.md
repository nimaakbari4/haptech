# Object Types Reference

Complete reference for all available object types and their parameters in haptic trial configurations.

## Overview

The haptic system supports several built-in object types, each with specific geometric properties and haptic behaviors. Each object type has required and optional parameters that control its appearance and physical properties.

**Available object types**:
- `sphere` - Perfect spheres for targets and simple interactions
- `box` - Rectangular objects for obstacles and platforms
- `cylinder` - Cylindrical objects for varied haptic experiences
- `ball` - Dynamic spheres with physics simulation
- `torus` - Donut-shaped objects for advanced interactions
- `spring` - Spring objects for force feedback
- `line` - Linear objects for path guidance
- `ellipsoid` - Elliptical shapes for asymmetric objects

## Sphere Objects

### Basic Configuration

```xml
<Object id="target_sphere" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Color r="1.0" g="0.0" b="0.0"/>
    <Parameters radius="0.03"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `radius` | float | Sphere radius |

### Example with All Parameters

```xml
<Object id="detailed_sphere" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Color r="1.0" g="0.0" b="0.0"/>
    <Parameters 
        radius="0.04"
    />
</Object>
```

## Box Objects

### Basic Configuration

```xml
<Object id="obstacle_box" type="box">
    <Position x="0.0" y="0.1" z="0.0"/>
    <Color r="0.0" g="0.0" b="1.0"/>
    <Parameters width="0.05" height="0.05" depth="0.05"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `width` | float | Width along X-axis |
| `height` | float | Height along Y-axis |
| `depth` | float | Depth along Z-axis |

### Common Box Configurations

```xml
<!-- Small cube -->
<Parameters width="0.03" height="0.03" depth="0.03"/>

<!-- Platform/shelf -->
<Parameters width="0.15" height="0.02" depth="0.1"/>

<!-- Vertical wall -->
<Parameters width="0.01" height="0.1" depth="0.15"/>

<!-- Thin barrier -->
<Parameters width="0.2" height="0.005" depth="0.1"/>
```

## Cylinder Objects

### Basic Configuration

```xml
<Object id="cylinder_target" type="cylinder">
    <Position x="-0.1" y="0.0" z="0.0"/>
    <Color r="0.0" g="1.0" b="0.0"/>
    <Parameters radius="0.04" height="0.08"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `radius` | float | Cylinder radius |
| `height` | float | Cylinder height |

### Common Cylinder Configurations

```xml
<!-- Tall thin post -->
<Parameters radius="0.01" height="0.15"/>

<!-- Short wide disk -->
<Parameters radius="0.08" height="0.02"/>

<!-- Medium rod -->
<Parameters radius="0.02" height="0.1"/>
```

## Ball Objects

### Basic Configuration

```xml
<Object id="movable_ball" type="ball">
    <Position x="0.0" y="0.0" z="0.05"/>
    <Color r="1.0" g="1.0" b="0.0"/>
    <Parameters radius="0.025" mass="0.1" elasticity="0.8"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `radius` | float | Ball radius |

### Optional Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `mass` | float | 0.05 | Ball mass |
| `elasticity` | float | 0.5 | Bounce factor |
| `friction` | float | 0.1 | Rolling friction |
| `air_resistance` | float | 0.01 | Air resistance coefficient |
| `restitution` | float | 0.8 | Collision restitution |
| `linear_damping` | float | 0.1 | Linear motion damping |
| `angular_damping` | float | 0.1 | Rotational damping |

### Ball Physics Examples

```xml
<!-- Bouncy ball -->
<Parameters 
    radius="0.03" 
    mass="0.02" 
    elasticity="0.9" 
    friction="0.05"
    restitution="0.9"
/>

<!-- Heavy ball -->
<Parameters 
    radius="0.04" 
    mass="0.3" 
    elasticity="0.3" 
    friction="0.3"
    linear_damping="0.2"
/>

<!-- Low-friction ball -->
<Parameters 
    radius="0.025" 
    mass="0.05" 
    friction="0.01"
    air_resistance="0.005"
/>
```

## Torus Objects

### Basic Configuration

```xml
<Object id="ring_target" type="torus">
    <Position x="0.0" y="0.1" z="0.0"/>
    <Color r="0.8" g="0.0" b="0.8"/>
    <Parameters outer_radius="0.06" inner_radius="0.02"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `inner_radius` | float | Distance from center to torus center |
| `outer_radius` | float | Radius of the torus |

### Torus Size Guidelines

```xml
<!-- Small ring -->
<Parameters outer_radius="0.03" inner_radius="0.008"/>

<!-- Medium donut -->
<Parameters outer_radius="0.05" inner_radius="0.015"/>

<!-- Large ring -->
<Parameters outer_radius="0.1" inner_radius="0.025"/>
```

**Note**: `outer_radius` must be greater than `inner_radius` for valid torus geometry.

## Spring Objects

### Basic Configuration

```xml
<Object id="spring_damper" type="spring">
    <Position x="0.0" y="0.0" z="0.0"/>
    <Color r="0.5" g="0.5" b="0.5"/>
    <Parameters radius="0.02" height="0.1" stiffness="800" damping="100"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `radius` | float | Spring coil radius |
| `height` | float | Spring length |
| `stiffness` | float | Spring constant |
| `damping` | float | Damping coefficient |

### Spring Behavior Examples

```xml
<!-- Soft spring -->
<Parameters 
    radius="0.03" 
    height="0.08" 
    stiffness="200" 
    damping="30"
/>

<!-- Stiff spring -->
<Parameters 
    radius="0.015" 
    height="0.05" 
    stiffness="2000" 
    damping="150"
/>

<!-- Long extension spring -->
<Parameters 
    radius="0.02" 
    height="0.12" 
    stiffness="500" 
    damping="80"
/>
```

## Ellipsoid Objects

### Basic Configuration

```xml
<Object id="oval_target" type="ellipsoid">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Color r="0.0" g="0.8" b="0.8"/>
    <Parameters radius_x="0.04" radius_y="0.02" radius_z="0.03"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `radius_x` | float | Radius along X-axis |
| `radius_y` | float | Radius along Y-axis |
| `radius_z` | float | Radius along Z-axis |

### Ellipsoid Shape Examples

```xml
<!-- Elongated oval (cigar shape) -->
<Parameters radius_x="0.06" radius_y="0.015" radius_z="0.015"/>

<!-- Flattened oval (pancake shape) -->
<Parameters radius_x="0.04" radius_y="0.04" radius_z="0.01"/>

<!-- Asymmetric shape -->
<Parameters radius_x="0.05" radius_y="0.03" radius_z="0.025"/>
```