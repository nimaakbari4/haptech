# Structure Overview

Complete guide to the XML configuration file structure, elements, and organization.

## XML File Structure

### Basic Template

Every trial configuration file follows this basic structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<trial>
    <!-- Trial identification -->
    <name>trial_name</name>
    <description>Brief description of what this trial does</description>
    
    <!-- Virtual objects in the scene -->
    <objects>
        <!-- Object definitions go here -->
    </objects>
    
    <!-- When the trial should start -->
    <start_conditions>
        <!-- Start condition definitions go here -->
    </start_conditions>
    
    <!-- When the trial should end -->
    <end_conditions>
        <!-- End condition definitions go here -->
    </end_conditions>
    
    <!-- Optional: Scene appearance settings -->
    <scene_settings>
        <!-- Camera, lighting, background settings -->
    </scene_settings>
    
    <!-- Optional: Data collection configuration -->
    <data_collection>
        <!-- Sampling rates, file settings -->
    </data_collection>
    
    <!-- Optional: User interface settings -->
    <ui_settings>
        <!-- Visual feedback, display options -->
    </ui_settings>
</trial>
```

### Required Elements

**Every trial must have**:
1. `<trial>` root element
2. `<name>` - unique identifier for the trial
3. At least one `<start_conditions>` element
4. At least one `<end_conditions>` element

**Optional but commonly used**:
- `<description>` - human-readable trial description
- `<objects>` - virtual objects in the trial
- `<scene_settings>` - visual and haptic scene configuration
- `<data_collection>` - data recording settings
- `<ui_settings>` - user interface customization

## Element Hierarchy

### Trial Root Element

```xml
<trial version="1.0" schema="haptic_trial_v1">
    <!-- All trial content goes here -->
</trial>
```

**Attributes**:
- `version` (optional): Configuration file format version
- `schema` (optional): Schema identifier for validation

### Trial Identification

```xml
<name>reaching_task_baseline</name>
<description>
    Baseline reaching task with a single sphere target.
    Participant must reach from origin to target position.
</description>
<author>Dr. Jane Smith</author>
<creation_date>2024-01-15</creation_date>
<version>1.2</version>
```

**Elements**:
- `<name>`: Unique trial identifier (required)
- `<description>`: Human-readable description (recommended)
- `<author>`: Trial creator (optional)
- `<creation_date>`: When trial was created (optional)
- `<version>`: Trial version number (optional)

### Objects Section

```xml
<objects>
    <sphere id="target1">
        <position x="0.1" y="0.0" z="0.0"/>
        <radius>0.03</radius>
        <color r="1.0" g="0.0" b="0.0"/>
    </sphere>
    
    <box id="obstacle">
        <position x="0.05" y="0.0" z="0.0"/>
        <size x="0.02" y="0.02" z="0.02"/>
        <color r="0.5" g="0.5" b="0.5"/>
    </box>
</objects>
```

**Structure**:
- Contains multiple object definitions
- Each object has a unique `id` attribute
- Object types: `sphere`, `box`, `cylinder`, `mesh`, `plane`
- Each object type has specific required and optional parameters

### Start Conditions Section

```xml
<start_conditions>
    <time duration_seconds="0.0" priority="1"/>
    <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01" priority="2"/>
</start_conditions>
```

**Structure**:
- Contains one or more start condition definitions
- Each condition has a `priority` attribute (lower number = higher priority)
- Trial starts when **any** start condition is met
- Common conditions: `time`, `position`, `button_press`, `velocity`

### End Conditions Section

```xml
<end_conditions>
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02" priority="1"/>
    <time_limit duration_seconds="30.0" priority="2"/>
    <force_threshold force_newtons="10.0" duration_seconds="0.5" priority="3"/>
</end_conditions>
```

**Structure**:
- Contains one or more end condition definitions
- Each condition has a `priority` attribute
- Trial ends when **any** end condition is met
- Lower priority numbers are checked first
- Success/failure determined by which condition triggered

## Advanced Structure Elements

### Conditional Logic

```xml
<start_conditions>
    <and>
        <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01"/>
        <velocity max_speed="0.05"/>
    </and>
</start_conditions>

<end_conditions>
    <or>
        <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        <position target_x="0.0" target_y="0.1" target_z="0.0" tolerance="0.02"/>
    </or>
</end_conditions>
```

**Logical operators**:
- `<and>`: All child conditions must be met
- `<or>`: Any child condition must be met
- `<not>`: Condition must NOT be met
- Can be nested for complex logic

### Sequence Conditions

```xml
<end_conditions>
    <sequence>
        <position target_id="target1" tolerance="0.02" hold_duration="1.0"/>
        <position target_id="target2" tolerance="0.02" hold_duration="1.0"/>
        <position target_id="target3" tolerance="0.02"/>
    </sequence>
</end_conditions>
```

**Sequence behavior**:
- Conditions must be met in order
- Each condition must be satisfied before moving to next
- Optional `hold_duration` requires condition to be met continuously
- Trial succeeds when all sequence conditions are completed

### Timed Conditions

```xml
<start_conditions>
    <delay duration_seconds="3.0">
        <position target_x="0.0" target_y="0.0" target_z="0.0" tolerance="0.01"/>
    </delay>
</start_conditions>

<end_conditions>
    <timeout duration_seconds="5.0">
        <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </timeout>
</end_conditions>
```

**Timing wrappers**:
- `<delay>`: Wait specified time before checking child condition
- `<timeout>`: Child condition must be met within time limit
- `<hold>`: Condition must be maintained for specified duration

## Data Types and Units

### Position Coordinates

```xml
<position x="0.1" y="0.0" z="0.0"/>  <!-- Meters -->
```

**Units**: Meters (m)
**Coordinate system**: 
- X: Forward/backward (positive = away from user)
- Y: Left/right (positive = right)
- Z: Up/down (positive = up)
**Workspace bounds**: Approximately [-0.3, 0.3] in all dimensions

### Colors

```xml
<color r="1.0" g="0.5" b="0.0"/>     <!-- RGB values 0.0-1.0 -->
<color name="red"/>                   <!-- Named colors -->
<color hex="#FF8000"/>                <!-- Hex color codes -->
```

**Color formats**:
- **RGB components**: Values from 0.0 to 1.0
- **Named colors**: red, green, blue, yellow, cyan, magenta, white, black, gray
- **Hex codes**: Standard 6-digit hex format (#RRGGBB)

### Physical Properties

```xml
<stiffness>300</stiffness>           <!-- N/m (Newtons per meter) -->
<damping>50</damping>                <!-- N⋅s/m (Newton-seconds per meter) -->
<friction static="0.3" kinetic="0.2"/> <!-- Dimensionless (0.0-1.0) -->
<mass>0.5</mass>                     <!-- kg (Kilograms) -->
```

**Units**:
- **Stiffness**: N/m (Newtons per meter)
- **Damping**: N⋅s/m (Newton-seconds per meter)
- **Friction**: Dimensionless (0.0 = no friction, 1.0 = maximum friction)
- **Mass**: kg (Kilograms)

### Time Values

```xml
<duration_seconds>5.0</duration_seconds>   <!-- Seconds -->
<hold_time>2.5</hold_time>                 <!-- Seconds -->
<timeout>30</timeout>                      <!-- Seconds -->
```

**Units**: Always seconds
**Precision**: Decimal values supported (e.g., 0.1, 2.5, 10.0)

## Validation and Error Handling

### XML Schema Validation

The system validates XML files against a schema to catch common errors:

**Valid XML**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<trial>
    <name>test_trial</name>
    <start_conditions>
        <time duration_seconds="0.0"/>
    </start_conditions>
    <end_conditions>
        <time_limit duration_seconds="10.0"/>
    </end_conditions>
</trial>
```

**Common validation errors**:
```xml
<!-- Missing required elements -->
<trial>
    <name>incomplete_trial</name>
    <!-- ERROR: Missing start_conditions and end_conditions -->
</trial>

<!-- Invalid attribute values -->
<sphere id="target">
    <position x="invalid" y="0.0" z="0.0"/>  <!-- ERROR: x must be numeric -->
    <radius>-0.1</radius>                     <!-- ERROR: radius must be positive -->
</sphere>

<!-- Unclosed tags -->
<start_conditions>
    <time duration_seconds="0.0"/>
    <!-- ERROR: Missing </start_conditions> -->
```

### Runtime Validation

The system also validates configurations at runtime:

**Position bounds checking**:
```xml
<sphere id="target">
    <position x="1.0" y="0.0" z="0.0"/>  <!-- WARNING: Outside haptic workspace -->
</sphere>
```

**Logical condition checking**:
```xml
<end_conditions>
    <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="-0.01"/>
    <!-- ERROR: Tolerance cannot be negative -->
</end_conditions>
```

**Object reference checking**:
```xml
<end_conditions>
    <position target_id="missing_object" tolerance="0.02"/>
    <!-- ERROR: Object 'missing_object' not defined in objects section -->
</end_conditions>
```

## Configuration File Organization

### Single Trial File

```xml
<?xml version="1.0" encoding="UTF-8"?>
<trial>
    <name>simple_reaching</name>
    <description>Basic reaching task</description>
    
    <objects>
        <sphere id="target">
            <position x="0.1" y="0.0" z="0.0"/>
            <radius>0.03</radius>
        </sphere>
    </objects>
    
    <start_conditions>
        <time duration_seconds="0.0"/>
    </start_conditions>
    
    <end_conditions>
        <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </end_conditions>
</trial>
```

### Multiple Trial Variants

For related trials, create separate files with descriptive names:

```
config/
├── reaching_baseline.xml          # Basic reaching task
├── reaching_small_target.xml      # Same task, smaller target
├── reaching_large_target.xml      # Same task, larger target
├── reaching_with_obstacles.xml    # Same task, added obstacles
└── reaching_force_feedback.xml    # Same task, different haptic feedback
```

### Template-Based Organization

Create templates for common patterns:

```xml
<!-- template_reaching_task.xml -->
<?xml version="1.0" encoding="UTF-8"?>
<trial>
    <name>TEMPLATE_NAME</name>
    <description>TEMPLATE_DESCRIPTION</description>
    
    <objects>
        <sphere id="target">
            <position x="TARGET_X" y="TARGET_Y" z="TARGET_Z"/>
            <radius>TARGET_RADIUS</radius>
            <color r="TARGET_COLOR_R" g="TARGET_COLOR_G" b="TARGET_COLOR_B"/>
        </sphere>
    </objects>
    
    <start_conditions>
        <time duration_seconds="0.0"/>
    </start_conditions>
    
    <end_conditions>
        <position target_x="TARGET_X" target_y="TARGET_Y" target_z="TARGET_Z" tolerance="TARGET_TOLERANCE"/>
        <time_limit duration_seconds="TIME_LIMIT"/>
    </end_conditions>
</trial>
```

Then generate specific trials by replacing template variables:
- `TARGET_X`, `TARGET_Y`, `TARGET_Z` → specific coordinates
- `TARGET_RADIUS` → target size
- `TARGET_COLOR_R`, `TARGET_COLOR_G`, `TARGET_COLOR_B` → color values
- `TARGET_TOLERANCE` → accuracy requirement
- `TIME_LIMIT` → maximum trial duration

## Best Practices

### Readability

**Use descriptive names and comments**:
```xml
<trial>
    <name>motor_learning_day1_baseline</name>
    <description>
        Day 1 baseline measurement for motor learning study.
        Participant reaches to targets at 3 distances.
    </description>
    
    <!-- Primary target at medium distance -->
    <objects>
        <sphere id="primary_target">
            <position x="0.15" y="0.0" z="0.0"/>  <!-- 15cm forward -->
            <radius>0.025</radius>                 <!-- 2.5cm radius -->
            <color name="red"/>
        </sphere>
    </objects>
</trial>
```

**Organize elements logically**:
```xml
<trial>
    <!-- 1. Trial information -->
    <name>trial_name</name>
    <description>trial_description</description>
    
    <!-- 2. Scene objects -->
    <objects>
        <!-- Objects in order of importance -->
    </objects>
    
    <!-- 3. Trial timing -->
    <start_conditions>
        <!-- Most important conditions first -->
    </start_conditions>
    
    <end_conditions>
        <!-- Success conditions first, failure conditions last -->
    </end_conditions>
    
    <!-- 4. Optional settings -->
    <scene_settings>
        <!-- Scene configuration -->
    </scene_settings>
</trial>
```

### Maintainability

**Use consistent naming**:
- Object IDs: `target1`, `target2`, `obstacle_main`, `visual_cue`
- Trial names: `task_condition_version` format
- File names: match trial names with `.xml` extension

**Document parameter choices**:
```xml
<sphere id="target">
    <position x="0.12" y="0.0" z="0.0"/>  <!-- Based on pilot study reach distances -->
    <radius>0.03</radius>                  <!-- Size for 85% success rate -->
    <stiffness>400</stiffness>             <!-- Firm but comfortable haptic feedback -->
</sphere>
```

**Version control friendly**:
- Use consistent indentation (2 or 4 spaces)
- Keep line lengths reasonable
- Add meaningful commit messages when updating configurations

This structure overview provides the foundation for understanding how XML trial configurations are organized and validated. Next, explore the specific sections for detailed information about objects, conditions, and advanced features.