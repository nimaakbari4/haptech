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

### Optional Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `stiffness` | float | 1000 | Haptic stiffness |
| `damping` | float | 50 | Haptic damping |
| `static_friction` | float | 0.1 | Static friction coefficient |
| `dynamic_friction` | float | 0.05 | Dynamic friction coefficient |
| `mass` | float | 0.05 | Object mass (for physics) |

### Example with All Parameters

```xml
<Object id="detailed_sphere" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Color r="1.0" g="0.0" b="0.0"/>
    <Parameters 
        radius="0.04"
        stiffness="1200"
        damping="75"
        static_friction="0.3"
        dynamic_friction="0.2"
        mass="0.08"
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

### Optional Parameters

| Parameter | Type | Default | Description | Units | Range |
|-----------|------|---------|-------------|-------|-------|
| `stiffness` | float | 1000 | Haptic stiffness | N/m | 10 - 5000 |
| `damping` | float | 50 | Haptic damping | Ns/m | 1 - 200 |
| `static_friction` | float | 0.1 | Static friction coefficient | - | 0.0 - 2.0 |
| `dynamic_friction` | float | 0.05 | Dynamic friction coefficient | - | 0.0 - 2.0 |
| `mass` | float | 0.1 | Object mass | kg | 0.001 - 1.0 |

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

### Optional Parameters

| Parameter | Type | Default | Description | Units | Range |
|-----------|------|---------|-------------|-------|-------|
| `stiffness` | float | 1000 | Haptic stiffness | N/m | 10 - 5000 |
| `damping` | float | 50 | Haptic damping | Ns/m | 1 - 200 |
| `static_friction` | float | 0.1 | Static friction coefficient | - | 0.0 - 2.0 |
| `dynamic_friction` | float | 0.05 | Dynamic friction coefficient | - | 0.0 - 2.0 |
| `mass` | float | 0.1 | Object mass | kg | 0.001 - 1.0 |

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
    <Parameters major_radius="0.06" minor_radius="0.02"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `major_radius` | float | Distance from center to tube center |
| `minor_radius` | float | Radius of the tube |

### Optional Parameters

| Parameter | Type | Default | Description | Units | Range |
|-----------|------|---------|-------------|-------|-------|
| `stiffness` | float | 1000 | Haptic stiffness | N/m | 10 - 5000 |
| `damping` | float | 50 | Haptic damping | Ns/m | 1 - 200 |
| `static_friction` | float | 0.1 | Static friction coefficient | - | 0.0 - 2.0 |
| `dynamic_friction` | float | 0.05 | Dynamic friction coefficient | - | 0.0 - 2.0 |

### Torus Size Guidelines

```xml
<!-- Small ring -->
<Parameters major_radius="0.03" minor_radius="0.008"/>

<!-- Medium donut -->
<Parameters major_radius="0.05" minor_radius="0.015"/>

<!-- Large ring -->
<Parameters major_radius="0.1" minor_radius="0.025"/>
```

**Note**: `major_radius` must be greater than `minor_radius` for valid torus geometry.

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

### Optional Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `coils` | int | 8 | Number of spring coils |
| `wire_thickness` | float | 0.002 | Spring wire thickness |
| `compression_limit` | float | 0.8 | Maximum compression ratio |
| `extension_limit` | float | 2.0 | Maximum extension ratio |

### Spring Behavior Examples

```xml
<!-- Soft spring -->
<Parameters 
    radius="0.03" 
    height="0.08" 
    stiffness="200" 
    damping="30"
    coils="10"
/>

<!-- Stiff spring -->
<Parameters 
    radius="0.015" 
    height="0.05" 
    stiffness="2000" 
    damping="150"
    coils="6"
/>

<!-- Long extension spring -->
<Parameters 
    radius="0.02" 
    height="0.12" 
    stiffness="500" 
    damping="80"
    extension_limit="3.0"
/>
```

## Line Objects

### Basic Configuration

```xml
<Object id="guide_line" type="line">
    <Position x="0.0" y="0.0" z="0.0"/>
    <Color r="1.0" g="1.0" b="1.0"/>
    <Parameters start_x="0.0" start_y="0.0" start_z="0.0" 
                end_x="0.1" end_y="0.1" end_z="0.0" 
                thickness="0.002"/>
</Object>
```

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `start_x` | float | Start point X coordinate |
| `start_y` | float | Start point Y coordinate |
| `start_z` | float | Start point Z coordinate |
| `end_x` | float | End point X coordinate |
| `end_y` | float | End point Y coordinate |
| `end_z` | float | End point Z coordinate |
| `thickness` | float | Line thickness |

### Optional Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `stiffness` | float | 1500 | Haptic stiffness |
| `damping` | float | 100 | Haptic damping |
| `tangent_stiffness` | float | 800 | Tangent direction stiffness |

### Line Usage Examples

```xml
<!-- Horizontal guide -->
<Parameters 
    start_x="-0.1" start_y="0.0" start_z="0.0"
    end_x="0.1" end_y="0.0" end_z="0.0"
    thickness="0.003"
/>

<!-- Curved path approximation -->
<Parameters 
    start_x="0.0" start_y="0.0" start_z="0.0"
    end_x="0.05" end_y="0.05" end_z="0.02"
    thickness="0.002"
    stiffness="800"
    tangent_stiffness="400"
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

### Optional Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `stiffness` | float | 1000 | Haptic stiffness |
| `damping` | float | 50 | Haptic damping |
| `mass` | float | 0.05 | Object mass |

### Ellipsoid Shape Examples

```xml
<!-- Elongated oval (cigar shape) -->
<Parameters radius_x="0.06" radius_y="0.015" radius_z="0.015"/>

<!-- Flattened oval (pancake shape) -->
<Parameters radius_x="0.04" radius_y="0.04" radius_z="0.01"/>

<!-- Asymmetric shape -->
<Parameters radius_x="0.05" radius_y="0.03" radius_z="0.025"/>
```

## Universal Optional Elements

All object types support these optional elements:

### Visibility Control

```xml
<Visible>false</Visible>  <!-- Object starts hidden -->
```

### Transparency

```xml
<Transparency>0.5</Transparency>  <!-- 0.0 = opaque, 1.0 = transparent -->
```

### Material Properties

```xml
<Material>
    <Ambient r="0.2" g="0.2" b="0.2"/>
    <Diffuse r="0.8" g="0.8" b="0.8"/>
    <Specular r="0.5" g="0.5" b="0.5"/>
    <Shininess>50.0</Shininess>
</Material>
```

### Texture Mapping

```xml
<Texture>
    <File>textures/surface.jpg</File>
    <Scale>1.0</Scale>
    <Rotation>0.0</Rotation>
</Texture>
```

## Object Interaction Properties

### Collision Detection

All objects support collision detection with these properties:

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `collision_enabled` | bool | true | Enable/disable collision |
| `collision_margin` | float | 0.001 | Collision detection tolerance |
| `collision_response` | string | "haptic" | "haptic", "visual", "both", "none" |

```xml
<Parameters 
    radius="0.03"
    collision_enabled="true"
    collision_margin="0.002"
    collision_response="haptic"
/>
```

### Physics Simulation

For dynamic objects (mainly `ball` type):

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `gravity_enabled` | bool | true | Enable gravity effects |
| `kinematic` | bool | false | Object moved by script only |
| `fixed_rotation` | bool | false | Prevent rotation |
| `continuous_collision` | bool | false | High-precision collision |

```xml
<Parameters 
    radius="0.025"
    mass="0.1"
    gravity_enabled="true"
    kinematic="false"
    continuous_collision="true"
/>
```

## Performance Considerations

### Complexity Levels

**Low complexity** (best performance):
- `sphere`, `box` with default parameters
- No physics simulation
- Simple collision detection

**Medium complexity**:
- `cylinder`, `torus`, `ellipsoid`
- Basic physics properties
- Standard collision detection

**High complexity** (may impact performance):
- `ball` with full physics
- `spring` with detailed simulation
- Complex `line` paths
- Advanced material properties

### Optimization Guidelines

1. **Use simple shapes** when possible
2. **Limit dynamic objects** (balls with physics)
3. **Avoid excessive detail** in object parameters
4. **Test performance** with your target hardware
5. **Consider LOD** (level of detail) for distant objects

## Best Practices

### Object Sizing
- **Minimum size**: 2mm radius/width for reliable haptic feedback
- **Maximum size**: Stay within workspace bounds (±30cm)
- **Proportional**: Scale objects appropriately for the task

### Parameter Selection
- **Start with defaults**: Begin with default stiffness/damping
- **Adjust gradually**: Make small parameter changes
- **Test thoroughly**: Verify haptic feel with target users
- **Document changes**: Record parameter modifications

### Naming Conventions
- Use descriptive IDs: `target_sphere`, `obstacle_box`
- Include size indicators: `small_target`, `large_barrier`
- Group related objects: `waypoint_1`, `waypoint_2`

### Common Parameter Ranges

**Stiffness values**:
- Soft objects: 100-500 N/m
- Normal objects: 500-1500 N/m
- Hard objects: 1500-3000 N/m
- Very hard: 3000+ N/m

**Damping values**:
- Low damping: 10-30 Ns/m
- Normal damping: 30-100 Ns/m
- High damping: 100-200 Ns/m

**Object sizes**:
- Small targets: 1-3cm radius
- Medium objects: 3-6cm radius
- Large objects: 6-10cm radius