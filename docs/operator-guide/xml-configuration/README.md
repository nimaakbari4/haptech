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

**To use this configuration**:
1. Save as `simple_trial.xml` in your config directory
2. Run with Python client: `python main.py simple_trial.xml`
3. The trial will end when you reach the red sphere

## Configuration Workflow

**Step 1: Plan your trial**
- What task do participants need to perform?
- What virtual objects are needed?
- When should the trial start and end?
- What data needs to be collected?

**Step 2: Create base structure**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<TrialConfig>
    <!-- Configuration sections go here -->
</TrialConfig>
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

### Reference Documentation
- [XML Elements Reference](reference/elements.md) - Complete reference for all XML elements and attributes
- [Object Types Reference](reference/object-types.md) - Available object types and their parameters
- [Condition Types Reference](reference/condition-types.md) - Available condition types and their parameters

## Common Configuration Patterns

### Basic Reaching Task
```xml
<TrialConfig>
    <Objects>
        <Object id="target" type="sphere">
            <Position x="0.15" y="0.0" z="0.0"/>
            <Orientation x="0" y="0" z="0"/>
            <Color r="0.0" g="1.0" b="0.0"/>
            <Parameters radius="0.025"/>
        </Object>
    </Objects>
    
    <StartConditions>
        <Condition name="time" success="true" priority="1">
            <Parameters duration_seconds="0.0"/>
        </Condition>
    </StartConditions>
    
    <EndConditions>
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.15" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        </Condition>
        <Condition name="time" success="false" priority="2">
            <Parameters duration_seconds="30.0"/>
        </Condition>
    </EndConditions>
</TrialConfig>
```

### Velocity Control Task
```xml
<TrialConfig>
    <Objects>
        <Object id="target" type="sphere">
            <Position x="0.1" y="0.0" z="0.0"/>
            <Orientation x="0" y="0" z="0"/>
            <Color r="0.0" g="0.0" b="1.0"/>
            <Parameters radius="0.03"/>
        </Object>
    </Objects>
    
    <StartConditions>
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.05" target_y="0.0" target_z="0.0" tolerance="0.01"/>
        </Condition>
    </StartConditions>
    
    <EndConditions>
        <!-- Reach target -->
        <Condition name="position" success="true" priority="1">
            <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
        </Condition>
        <!-- Fail if moving too fast -->
        <Condition name="velocity" success="false" priority="2">
            <Parameters max_velocity="0.3"/>
        </Condition>
        <!-- Time limit -->
        <Condition name="time" success="false" priority="3">
            <Parameters duration_seconds="45.0"/>
        </Condition>
    </EndConditions>
</TrialConfig>
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
- **Object IDs**: `object_type_descriptor` (e.g., `target_sphere`, `obstacle_box`)

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
Error: XML parse error at line 15: unclosed element 'Objects'
Solution: Check that all XML tags are properly closed
```

**Object positioning errors**:
```
Warning: Object 'target_sphere' positioned outside haptic workspace
Solution: Verify x, y, z coordinates are within [-0.3, 0.3] range
```

**Unreachable conditions**:
```
Warning: End condition may be unreachable
Solution: Check position tolerances and time limits are reasonable
```

**Missing required elements**:
```
Error: TrialConfig must contain at least one EndCondition
Solution: Add appropriate end conditions to your configuration
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