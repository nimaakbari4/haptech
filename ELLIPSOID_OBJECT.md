# Ellipsoid Object

The ellipsoid object is a built-in custom object that creates ellipsoidal shapes using CHAI3D's `cCreateEllipsoid` function.

## XML Configuration

Use the `ellipsoid` type in your XML configuration:

```xml
<Object id="my_ellipsoid" type="ellipsoid">
  <Position x="0.0" y="0.0" z="0.0"/>
  <Orientation roll="0" pitch="0" yaw="0"/>
  <Color r="1.0" g="0.0" b="0.0"/>
  <Parameters 
    radius_x="0.05" 
    radius_y="0.03" 
    radius_z="0.08" 
    slices="32" 
    stacks="16"/>
</Object>
```

## Parameters

- `radius_x` (double): Radius along the X-axis (default: 0.05)
- `radius_y` (double): Radius along the Y-axis (default: 0.05)  
- `radius_z` (double): Radius along the Z-axis (default: 0.05)
- `slices` (unsigned int): Number of subdivisions around the Z-axis, like lines of longitude (default: 32)
- `stacks` (unsigned int): Number of subdivisions along the Z-axis, like lines of latitude (default: 16)

## Examples

### Sphere-like ellipsoid (all radii equal)
```xml
<Parameters radius_x="0.05" radius_y="0.05" radius_z="0.05"/>
```

### Prolate ellipsoid (elongated along Z-axis)
```xml
<Parameters radius_x="0.03" radius_y="0.03" radius_z="0.08"/>
```

### Oblate ellipsoid (flattened along Z-axis)
```xml
<Parameters radius_x="0.06" radius_y="0.06" radius_z="0.02"/>
```

### Triaxial ellipsoid (all radii different)
```xml
<Parameters radius_x="0.08" radius_y="0.04" radius_z="0.06"/>
```

## Quality Settings

Higher `slices` and `stacks` values create smoother ellipsoids but require more computational resources:

- **Low quality**: `slices="8" stacks="6"` (fast, low detail)
- **Medium quality**: `slices="32" stacks="16"` (balanced)
- **High quality**: `slices="64" stacks="32"` (smooth, slower)

## Test Configuration

A complete test configuration is available in `python_client/ellipsoid_test.xml` with various ellipsoid shapes and sizes for testing.
