# Line Object

The line object is a built-in custom object that creates magnetic lines using CHAI3D's `cCreateCylinder` function. These lines appear as thin cylinders and can guide or constrain robot movement along specific paths by creating magnetic attraction effects.

## XML Configuration

Use the `line` type in your XML configuration:

```xml
<Object id="my_line" type="line">
  <Position x="0.0" y="0.0" z="0.0"/>
  <Orientation roll="0" pitch="0" yaw="0"/>
  <Color r="1.0" g="0.0" b="0.0"/>
  <Parameters 
    point_a_x="0.0" point_a_y="0.0" point_a_z="0.0"
    point_b_x="0.3" point_b_y="0.0" point_b_z="0.0"
    line_width="10.0"
    magnet_max_distance="0.08"
    magnet_max_force="8.0"
    stiffness="600"/>
</Object>
```

## Parameters

### Line Geometry
- `point_a_x`, `point_a_y`, `point_a_z` (double): Starting point of the line (default: 0.0, 0.0, 0.0)
- `point_b_x`, `point_b_y`, `point_b_z` (double): Ending point of the line (default: 0.3, 0.0, 0.0)
- `line_width` (double): Visual thickness of the line (default: 5.0)

### Magnetic Properties
- `magnet_max_distance` (double): Maximum distance for magnetic attraction (default: 0.05)
- `magnet_max_force` (double): Maximum magnetic force applied (default: 5.0)
- `stiffness` (double): Stiffness of the magnetic field (default: 500.0)

## How It Works

The line object creates a thin cylinder mesh that:
1. **Visual Representation**: Appears as a thin cylinder connecting two points
2. **Magnetic Effect**: Creates a magnetic field that attracts the robot tool to the line
3. **Collision Detection**: Uses AABB collision detection for haptic interaction
4. **Orientation**: Automatically rotates to align with the line direction

## Use Cases

- **Path Guidance**: Guide robot movement along specific trajectories
- **Constraint Training**: Train users to follow precise paths
- **Assembly Tasks**: Guide assembly operations along defined lines
- **Surgical Training**: Provide guidance for precise surgical movements

## Example Configurations

### Horizontal Line
```xml
<Parameters 
  point_a_x="-0.2" point_a_y="0.0" point_a_z="0.0"
  point_b_x="0.2" point_b_y="0.0" point_b_z="0.0"
  line_width="8.0"/>
```

### Vertical Line
```xml
<Parameters 
  point_a_x="0.0" point_a_y="0.0" point_a_z="-0.1"
  point_b_x="0.0" point_b_y="0.0" point_b_z="0.1"
  line_width="6.0"/>
```

### Diagonal Line
```xml
<Parameters 
  point_a_x="-0.1" point_a_y="-0.1" point_a_z="0.0"
  point_b_x="0.1" point_b_y="0.1" point_b_z="0.0"
  line_width="10.0"/>
```

## Technical Notes

- The line is implemented as a thin cylinder mesh for reliable visibility
- The cylinder radius is calculated as `line_width * 0.01` for appropriate scaling
- Magnetic effects are applied to the entire cylinder surface
- The line automatically orients itself to connect the two specified points