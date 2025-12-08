# XML Configuration

Complete guide to the haptic system's XML configuration system for defining trials, objects, and experimental conditions.

## Overview

The haptic system uses XML configuration files to define experimental trials. These files specify everything needed for a trial: virtual objects, haptic feedback settings, start/end conditions, and data collection parameters.

**Key benefits of XML configuration**:
- **Human readable**: Easy to read and modify
- **Version control friendly**: Text-based files work well with Git
- **Flexible**: Support for simple to complex experimental designs
- **Reusable**: Save and share experimental configurations
- **Validated**: Built-in validation prevents common errors

**What you can configure**:
- **Virtual objects**: Shapes, positions, haptic properties
- **Trial conditions**: When trials start and end
- **Scene settings**: Camera, lighting, visual appearance
- **Data collection**: Sampling rates, file formats
- **User interface**: Visual feedback and display options

## Quick Start

**Use the template file**: Copy [`template-trial.xml`](template-trial.xml) as a starting point for your trials. It includes comprehensive comments and examples for all major features.

**Minimal trial configuration**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<trial>
    <name>simple_trial</name>
    <description>Reach to a sphere target</description>
    
    <!-- Add a sphere target -->
    <objects>
        <sphere id="target">
            <position x="0.1" y="0.0" z="0.0"/>
            <radius>0.03</radius>
            <color r="1.0" g="0.0" b="0.0"/>
        </sphere>
    </objects>
    
    <!-- Start immediately -->
    <start_conditions>
        <time duration_seconds="0.0"/>
    </start_conditions>
    
    <!-- End when target is reached -->
    <end_conditions>
        <position target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </end_conditions>
</trial>
```

**To use this configuration**:
1. Save as `simple_trial.xml` in your config directory
2. Run with Python client: `python main.py simple_trial.xml`
3. The trial will start immediately and end when you reach the red sphere

## Configuration Workflow

**Step 1: Plan your trial**
- What task do participants need to perform?
- What virtual objects are needed?
- When should the trial start and end?
- What data needs to be collected?

**Step 2: Create base structure**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<trial>
    <name>trial_name</name>
    <description>Brief description of the trial</description>
    
    <!-- Configuration sections go here -->
</trial>
```

**Step 3: Add objects**
- Define virtual objects participants interact with
- Set positions, sizes, and haptic properties
- Configure visual appearance

**Step 4: Set trial conditions**
- Define when the trial should start
- Define when the trial should end
- Set up success/failure criteria

**Step 5: Configure scene and data collection**
- Set camera position and lighting
- Configure data collection parameters
- Add any visual feedback elements

**Step 6: Test and validate**
- Load configuration in the haptic system
- Test with a short trial run
- Verify all elements work as expected

## Table of Contents

### Core Documentation
- [Structure Overview](structure-overview.md) - Overall XML configuration structure and elements
- [Trial Conditions](trial-conditions.md) - Configuring start and end conditions for trials
- [Objects](objects.md) - Virtual object configuration and haptic properties
- [Scene Settings](scene-settings.md) - Camera, lighting, and background configuration
- [Data Collection](data-collection.md) - Data collection and logging configuration

### Practical Examples
- [Basic Trial](examples/basic-trial.md) - Simple trial configuration example
- [Multi-Condition Trial](examples/multi-condition.md) - Complex condition configuration example
- [Custom Objects](examples/custom-objects.md) - Using custom objects in XML configuration
- [Data Collection](examples/data-collection.md) - Data collection configuration examples

### Reference Documentation
- [XML Elements Reference](reference/elements.md) - Complete reference for all XML elements and attributes
- [Object Types Reference](reference/object-types.md) - Available object types and their parameters
- [Condition Types Reference](reference/condition-types.md) - Available condition types and their parameters

## Common Configuration Patterns

### Basic Reaching Task
```xml
<trial>
    <name>reaching_task</name>
    
    <objects>
        <sphere id="target">
            <position x="0.15" y="0.0" z="0.0"/>
            <radius>0.025</radius>
            <stiffness>300</stiffness>
        </sphere>
    </objects>
    
    <start_conditions>
        <time duration_seconds="0.0"/>
    </start_conditions>
    
    <end_conditions>
        <position target_x="0.15" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        <time_limit duration_seconds="30.0" priority="2"/>
    </end_conditions>
</trial>
```

### Multiple Target Sequence
```xml
<trial>
    <name>sequence_task</name>
    
    <objects>
        <sphere id="target1">
            <position x="0.1" y="0.0" z="0.0"/>
            <radius>0.02</radius>
            <color r="1.0" g="0.0" b="0.0"/>
        </sphere>
        <sphere id="target2">
            <position x="0.0" y="0.1" z="0.0"/>
            <radius>0.02</radius>
            <color r="0.0" g="1.0" b="0.0"/>
            <visible>false</visible>  <!-- Hidden initially -->
        </sphere>
    </objects>
    
    <start_conditions>
        <time duration_seconds="0.0"/>
    </start_conditions>
    
    <end_conditions>
        <!-- Reach target1, then target2 appears -->
        <sequence>
            <position target_id="target1" tolerance="0.02"/>
            <position target_id="target2" tolerance="0.02"/>
        </sequence>
    </end_conditions>
</trial>
```

### Force Application Task
```xml
<trial>
    <name>force_task</name>
    
    <objects>
        <box id="pushable_object">
            <position x="0.1" y="0.0" z="0.0"/>
            <size x="0.05" y="0.05" z="0.05"/>
            <stiffness>500</stiffness>
            <damping>50</damping>
            <mass>0.2</mass>
        </box>
    </objects>
    
    <start_conditions>
        <position target_x="0.05" target_y="0.0" target_z="0.0" tolerance="0.01"/>
    </start_conditions>
    
    <end_conditions>
        <force_threshold force_newtons="5.0" duration_seconds="2.0"/>
        <time_limit duration_seconds="45.0" priority="2"/>
    </end_conditions>
</trial>
```

## Configuration Best Practices

### File Organization
```
config/
├── trials/           # Individual trial configs
│   ├── practice/     # Practice trials
│   ├── baseline/     # Baseline measurements
│   ├── experimental/ # Main experimental trials
│   └── test/         # Testing and debugging
├── objects/          # Reusable object definitions
├── scenes/           # Scene templates
└── templates/        # Base templates for new trials
```

### Naming Conventions
- **Files**: `task_condition_version.xml` (e.g., `reaching_baseline_v1.xml`)
- **Trial names**: `descriptive_name` (e.g., `reaching_task_small_target`)
- **Object IDs**: `object_type_descriptor` (e.g., `sphere_target`, `box_obstacle`)

### Version Control
- Use descriptive commit messages when changing configurations
- Tag important configuration versions
- Keep backup copies of working configurations
- Document changes in trial descriptions

### Validation
- Always test new configurations with short trial runs
- Verify object positions are within haptic workspace
- Check that conditions can be realistically achieved
- Validate data collection settings match analysis requirements

### Reusability
- Create template files for common experimental paradigms
- Use consistent object names across related trials
- Document configuration parameters and their effects
- Share working configurations with research team

## Troubleshooting

### Common Configuration Errors

**Invalid XML syntax**:
```
Error: XML parse error at line 15: unclosed element 'objects'
Solution: Check that all XML tags are properly closed
```

**Object positioning errors**:
```
Warning: Object 'target' positioned outside haptic workspace
Solution: Verify x, y, z coordinates are within [-0.3, 0.3] range
```

**Unreachable conditions**:
```
Warning: End condition may be unreachable
Solution: Check position tolerances and time limits are reasonable
```

**Missing required elements**:
```
Error: Trial must contain at least one start_condition
Solution: Add appropriate start conditions to your configuration
```

### Getting Help
- Check the [troubleshooting guide](../troubleshooting/) for common issues
- Review example configurations in the `examples/` directory
- Use the configuration validator: `python validate_config.py your_trial.xml`
- Consult the complete reference documentation for detailed parameter information

## Next Steps

1. **New users**: Start with the [`template-trial.xml`](template-trial.xml) file and modify it for your needs
2. **Understanding structure**: Read [Structure Overview](structure-overview.md) to understand the XML format
3. **Creating conditions**: Review [Trial Conditions](trial-conditions.md) to set up start and end conditions
4. **Adding objects**: Read [Objects](objects.md) to learn about virtual object configuration
5. **Practical examples**: Browse the [examples](examples/) directory for real trial configurations
6. **Advanced features**: Explore [Scene Settings](scene-settings.md) and [Data Collection](data-collection.md) for complete control