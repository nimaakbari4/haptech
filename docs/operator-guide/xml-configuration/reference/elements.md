# XML Elements Reference

Complete reference for all XML elements and attributes available in haptic trial configurations.

## Document Structure

### Root Element

#### `<TrialConfig>`
Root element for all trial configurations.

**Attributes**: None
**Required**: Yes
**Children**: All trial configuration elements

```xml
<TrialConfig>
    <!-- All trial configuration goes here -->
</TrialConfig>
```

## Trial Identification

### `<Test>`
Optional flag to mark configuration as test/experimental.

**Content**: "true" or "false"
**Required**: No
**Example**:
```xml
<Test>true</Test>
```

## Condition Elements

### `<StartConditions>`
Defines when the trial begins.

**Attributes**: None
**Required**: No (defaults to immediate start)
**Children**: One or more `<Condition>` elements

```xml
<StartConditions>
    <Condition name="time" success="true" priority="1">
        <Parameters duration_seconds="2.0"/>
    </Condition>
</StartConditions>
```

### `<EndConditions>`
Defines when the trial ends and success/failure criteria.

**Attributes**: None
**Required**: Yes
**Children**: One or more `<Condition>` elements

```xml
<EndConditions>
    <Condition name="position" success="true" priority="1">
        <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </Condition>
</EndConditions>
```

### `<Condition>`
Individual condition definition.

**Attributes**:
- `name` (required): Condition type (see [Condition Types Reference](condition-types.md))
- `success` (required): "true" for success condition, "false" for failure condition
- `priority` (required): Integer priority (1 = highest priority)

**Children**: `<Parameters>` element with condition-specific parameters

```xml
<Condition name="time" success="false" priority="2">
    <Parameters duration_seconds="30.0"/>
</Condition>
```

### `<Parameters>`
Contains condition-specific parameters as attributes.

**Attributes**: Vary by condition type
**Required**: Usually yes
**Children**: None (parameters are attributes)

```xml
<!-- Position condition parameters -->
<Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>

<!-- Time condition parameters -->
<Parameters duration_seconds="30.0"/>

<!-- Velocity condition parameters -->
<Parameters max_velocity="0.5"/>
```

## Object Elements

### `<Objects>`
Container for all virtual objects in the trial.

**Attributes**: None
**Required**: No (but trials usually need objects)
**Children**: One or more `<Object>` elements

```xml
<Objects>
    <Object id="target" type="sphere">
        <!-- Object configuration -->
    </Object>
</Objects>
```

### `<Object>`
Defines a single virtual object.

**Attributes**:
- `id` (required): Unique identifier for the object
- `type` (required): Object type (box, sphere, ball, torus, spring, etc.)

**Children**: `<Position>`, `<Orientation>`, `<Color>`, `<Parameters>`

```xml
<Object id="target_sphere" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>
    <Orientation x="0" y="0" z="0"/>
    <Color r="1.0" g="0.0" b="0.0"/>
    <Parameters radius="0.03"/>
</Object>
```

### `<Position>`
3D position in workspace coordinates.

**Attributes**:
- `x` (required): X-coordinate in meters
- `y` (required): Y-coordinate in meters
- `z` (required): Z-coordinate in meters

**Children**: None

```xml
<Position x="0.1" y="0.0" z="0.0"/>
```

### `<Orientation>`
3D orientation using Euler angles in degrees.

**Attributes**:
- `x` (required): Rotation around X-axis in degrees
- `y` (required): Rotation around Y-axis in degrees
- `z` (required): Rotation around Z-axis in degrees

**Children**: None

```xml
<Orientation x="0" y="0" z="45"/>
```

### `<Color>`
RGB color specification.

**Attributes**:
- `r` (required): Red component (0.0-1.0)
- `g` (required): Green component (0.0-1.0)
- `b` (required): Blue component (0.0-1.0)

**Children**: None

```xml
<Color r="1.0" g="0.0" b="0.0"/>  <!-- Red -->
```

### `<Parameters>`
Object-specific parameters (varies by object type).

**Attributes**: Vary by object type
**Children**: None

```xml
<!-- Sphere parameters -->
<Parameters radius="0.03"/>

<!-- Box parameters -->
<Parameters width="0.05" height="0.05" depth="0.05"/>

<!-- Ball parameters -->
<Parameters radius="0.04" gravity_scale="0.0" mass="100.0" bounciness="1.0"/>

<!-- Torus parameters -->
<Parameters inner_radius="0.03" outer_radius="0.1" num_sides="16" num_rings="20"/>

<!-- Spring parameters -->
<Parameters radius="0.08" height="0.20" stiffness="1000.0" damping="100.0"/>
```

## Scene Configuration

### `<SceneSettings>`
Container for scene appearance configuration.

**Attributes**: None
**Required**: No
**Children**: `<Camera>`, `<BackgroundColor>`, `<Lighting>`, etc.

```xml
<SceneSettings>
    <Camera><!-- Camera configuration --></Camera>
    <BackgroundColor r="0.2" g="0.2" b="0.2"/>
</SceneSettings>
```

### `<Camera>`
Defines camera viewpoint.

**Attributes**: None
**Required**: No (uses default if omitted)
**Children**: `<Position>`, `<LookAt>`, `<Up>`

```xml
<Camera>
    <Position x="1.0" y="1.0" z="2.0"/>
    <LookAt x="0.0" y="0.0" z="0.0"/>
    <Up x="0.0" y="0.0" z="1.0"/>
</Camera>
```

### `<LookAt>`
Defines camera target point.

**Attributes**:
- `x` (required): X-coordinate of look target
- `y` (required): Y-coordinate of look target  
- `z` (required): Z-coordinate of look target

**Children**: None

```xml
<LookAt x="0.0" y="0.0" z="0.0"/>
```

### `<Up>`
Defines camera up direction.

**Attributes**:
- `x` (required): X-component of up vector
- `y` (required): Y-component of up vector
- `z` (required): Z-component of up vector

**Children**: None

```xml
<Up x="0.0" y="0.0" z="1.0"/>
```

### `<BackgroundColor>`
Defines background color.

**Attributes**:
- `r` (required): Red component (0.0-1.0)
- `g` (required): Green component (0.0-1.0)
- `b` (required): Blue component (0.0-1.0)

**Children**: None

```xml
<BackgroundColor r="0.2" g="0.2" b="0.2"/>
```

### `<TextOverlay>`
Defines text overlay displayed on screen.

**Attributes**: None
**Required**: No
**Children**: `<Text>`, `<X>`, `<Y>`, `<FontSize>`, `<Color>`

```xml
<TextOverlay>
    <Text>Explore the sandbox for up to 2 minutes</Text>
    <X>0</X>
    <Y>50</Y>
    <FontSize>32</FontSize>
    <Color r="0" g="128" b="0"/>
</TextOverlay>
```

#### `<Text>`
The text content to display.

**Content**: Text string
**Required**: Yes

#### `<X>` and `<Y>`
Screen position for the text overlay.

**Content**: Integer pixel coordinates
**Required**: Yes

#### `<FontSize>`
Size of the text font.

**Content**: Integer font size
**Required**: Yes

## Common Object Types

### Available Object Types
Based on sandbox.xml, the following object types are supported:

- `box` - Rectangular objects for platforms, obstacles, targets
- `sphere` - Spherical objects for targets and simple interactions  
- `ball` - Dynamic spheres with physics simulation
- `torus` - Donut-shaped objects with inner/outer radius
- `spring` - Spring objects for force feedback

### Object Type Examples

**Box (ground platform)**:
```xml
<Object id="ground" type="box">
    <Position x="0.0" y="-0.15" z="-0.8"/>
    <Orientation x="0" y="0" z="0"/>
    <Parameters width="1.0" height="1.0" depth="1.0"/>
    <Color r="0.5" g="0.5" b="0.5"/>
</Object>
```

**Ball (pushable object)**:
```xml
<Object id="pushable_ball" type="ball">
    <Position x="0.0" y="-0.4" z="0.0"/>
    <Orientation x="0" y="0" z="0"/>
    <Parameters radius="0.04" gravity_scale="0.0" mass="100.0" bounciness="1.0"/>
    <Color r="0.2" g="0.8" b="1.0"/>
</Object>
```

**Torus (ring obstacle)**:
```xml
<Object id="ring_obstacle" type="torus">
    <Position x="0.0" y="0.25" z="0.0"/>
    <Orientation x="90" y="45" z="20"/>
    <Parameters inner_radius="0.03" outer_radius="0.1" num_sides="16" num_rings="20"/>
    <Color r="0.0" g="0.0" b="1.0"/>
</Object>
```

**Spring (obstacle)**:
```xml
<Object id="spring_obstacle" type="spring">
    <Position x="-0.2" y="-0.2" z="0.2"/>
    <Orientation x="20" y="30" z="10"/>
    <Parameters radius="0.08" height="0.20" stiffness="1000.0" damping="100.0"/>
    <Color r="1.0" g="1.0" b="0.0"/>
</Object>
```

## Validation Rules

### Required Elements
- Root element `<TrialConfig>`
- At least one `<EndCondition>`
- For objects: `id`, `type`, `<Position>`, `<Orientation>`, `<Parameters>`, `<Color>`
- For conditions: `name`, `success`, `priority`

### Attribute Constraints
- Position coordinates: typically within haptic workspace bounds
- Color values: [0.0, 1.0] for `<Color>` elements, [0, 255] for `<TextOverlay>` colors
- Priority values: positive integers
- Object IDs: must be unique within trial
- Orientation values: degrees (can be any real number)

### Common Validation Errors
- Missing required attributes
- Duplicate object IDs  
- Invalid coordinate ranges
- Color values outside valid ranges
- Invalid condition type names
- Missing condition parameters
- Malformed XML structure