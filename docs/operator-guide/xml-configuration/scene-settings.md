# Scene Settings

Complete guide to configuring camera, lighting, background, and visual appearance in XML trial configurations.

## Overview

Scene settings control the visual environment for your haptic trials. These settings determine how participants see the virtual world, affecting their spatial understanding and task performance.

**Key components**:
- **Camera**: Viewpoint and perspective 
- **Background**: Colors and environment
- **Text Overlays**: Text HUD elements for the participant conveying instructions or information

## Basic Scene Structure

```xml
<SceneSettings>
    <!-- Camera position and orientation -->
    <Camera>
        <Position x="1.0" y="0.0" z="0.5"/>
        <LookAt x="0.0" y="0.0" z="0.0"/>
        <Up x="0.0" y="0.0" z="1.0"/>
    </Camera>
    
    <!-- Background appearance -->
    <BackgroundColor r="0.2" g="0.2" b="0.2"/>
    
    <!-- Text overlay on screen -->
    <TextOverlay>
        <Text>Move to target position</Text>
        <X>400</X>
        <Y>50</Y>
        <FontSize>32</FontSize>
        <Color r="255" g="0" b="0"/>
    </TextOverlay>
</SceneSettings>
```

## Camera Configuration

The camera defines the participant's view of the virtual world.

### Basic Camera Setup

```xml
<Camera>
    <!-- Where the camera is positioned -->
    <Position x="1.0" y="1.0" z="2.0"/>
    
    <!-- What point the camera looks at -->
    <LookAt x="0.0" y="0.0" z="0.0"/>
    
    <!-- Which direction is "up" -->
    <Up x="0.0" y="1.0" z="0.0"/>
</Camera>
```

### Camera Positioning Examples

#### Overhead View
Great for 2D tasks and path following:

```xml
<Camera>
    <Position x="0.0" y="0.0" z="1.0"/>  <!-- Directly above -->
    <LookAt x="0.0" y="0.0" z="0.0"/>   <!-- Looking down at center -->
    <Up x="0.0" y="1.0" z="0.0"/>      <!-- Y-axis points forward -->
</Camera>
```

#### Side View
Ideal for vertical reaching tasks:

```xml
<Camera>
    <Position x="0.5" y="0.0" z="0.0"/>  <!-- To the right side -->
    <LookAt x="0.0" y="0.0" z="0.0"/>   <!-- Looking at center -->
    <Up x="0.0" y="0.0" z="1.0"/>      <!-- Z-axis points up -->
</Camera>
```

#### Angled Perspective
Natural 3D view for most tasks:

```xml
<Camera>
    <Position x="1.2" y="1.2" z="1.5"/>  <!-- Angled position -->
    <LookAt x="0.0" y="0.0" z="0.0"/>   <!-- Looking at workspace center -->
    <Up x="0.0" y="0.0" z="1.0"/>      <!-- Z-axis points up -->
</Camera>
```

#### First-Person View
Immersive perspective from user's viewpoint:

```xml
<Camera>
    <Position x="-0.5" y="0.0" z="0.0"/>  <!-- Behind user position -->
    <LookAt x="0.2" y="0.0" z="0.0"/>    <!-- Looking forward -->
    <Up x="0.0" y="0.0" z="1.0"/>       <!-- Z-axis points up -->
</Camera>
```

### Camera Guidelines

**Position considerations**:
- Keep camera outside the haptic workspace
- Ensure good view of all important objects
- Avoid positions that create confusing perspectives
- Consider the natural viewing angle for the task

**LookAt target**:
- Usually the center of the workspace (0, 0, 0)
- Can target specific objects for focused views
- Should include all relevant objects in the field of view

**Up vector**:
- Typically (0, 0, 1) for Z-up orientation
- Can be (0, 1, 0) for Y-up orientation
- Determines which direction appears "up" on screen

## Background Settings

### Solid Colors

Simple, distraction-free backgrounds:

```xml
<!-- Dark gray background (common choice) -->
<BackgroundColor r="0.2" g="0.2" b="0.2"/>

<!-- Light blue background -->
<BackgroundColor r="0.7" g="0.8" b="1.0"/>

<!-- Pure black background -->
<BackgroundColor r="0.0" g="0.0" b="0.0"/>

<!-- Neutral white background -->
<BackgroundColor r="1.0" g="1.0" b="1.0"/>
```

### Text Overlays

Add instructional text to the screen:

```xml
<TextOverlay>
    <Text>Reach the green target sphere</Text>
    <X>400</X>          <!-- Horizontal position in pixels -->
    <Y>50</Y>           <!-- Vertical position in pixels -->
    <FontSize>32</FontSize>
    <Color r="255" g="255" b="255"/>  <!-- White text (RGB 0-255) -->
</TextOverlay>
```

### Background Best Practices

**Contrast considerations**:
- Choose colors that contrast well with your objects
- Avoid colors that are too bright or distracting
- Consider colorblind accessibility

**Common color choices**:
- **Dark gray (0.2, 0.2, 0.2)**: Professional, reduces eye strain
- **Light gray (0.8, 0.8, 0.8)**: Clean, good for dark objects
- **Dark blue (0.1, 0.1, 0.3)**: Calming, good contrast
- **Black (0.0, 0.0, 0.0)**: High contrast, minimal distraction

## Lighting

Currently, lighting configuration is handled automatically by the system. The virtual environment uses default ambient lighting that provides good visibility for all objects.

**Default lighting features**:
- Even illumination from all directions
- Optimized for haptic task visibility
- No shadows or complex lighting effects
- Consistent across all trials

**Note**: Advanced lighting configuration (directional lights, shadows, etc.) is not currently supported in XML configuration files. The system uses built-in lighting optimized for haptic applications.

## Complete Scene Examples

### Minimalist Setup
Clean and distraction-free:

```xml
<SceneSettings>
    <Camera>
        <Position x="0.8" y="0.8" z="1.2"/>
        <LookAt x="0.0" y="0.0" z="0.0"/>
        <Up x="0.0" y="0.0" z="1.0"/>
    </Camera>
    
    <BackgroundColor r="0.95" g="0.95" b="0.95"/>  <!-- Light gray -->
</SceneSettings>
```

### Standard Setup
Balanced view with text instructions:

```xml
<SceneSettings>
    <Camera>
        <Position x="1.2" y="1.0" z="1.8"/>
        <LookAt x="0.0" y="0.0" z="0.0"/>
        <Up x="0.0" y="0.0" z="1.0"/>
    </Camera>
    
    <BackgroundColor r="0.25" g="0.25" b="0.25"/>  <!-- Dark gray -->
    
    <TextOverlay>
        <Text>Reach the target as quickly as possible</Text>
        <X>400</X>
        <Y>50</Y>
        <FontSize>24</FontSize>
        <Color r="255" g="255" b="255"/>  <!-- White text -->
    </TextOverlay>
</SceneSettings>
```

### Overhead Task View
Ideal for 2D navigation tasks:

```xml
<SceneSettings>
    <Camera>
        <Position x="0.0" y="0.0" z="1.5"/>   <!-- Directly overhead -->
        <LookAt x="0.0" y="0.0" z="0.0"/>
        <Up x="0.0" y="1.0" z="0.0"/>       <!-- Y-axis points up -->
    </Camera>
    
    <BackgroundColor r="0.1" g="0.15" b="0.2"/>  <!-- Dark blue -->
    
    <TextOverlay>
        <Text>Navigate to each waypoint in order</Text>
        <X>300</X>
        <Y>30</Y>
        <FontSize>28</FontSize>
        <Color r="255" g="255" b="0"/>  <!-- Yellow text -->
    </TextOverlay>
</SceneSettings>
```

### Close-up Precision View
For fine manipulation tasks:

```xml
<SceneSettings>
    <Camera>
        <Position x="0.3" y="0.0" z="0.2"/>   <!-- Close side view -->
        <LookAt x="0.1" y="0.0" z="0.0"/>   <!-- Focus on work area -->
        <Up x="0.0" y="0.0" z="1.0"/>
    </Camera>
    
    <BackgroundColor r="0.0" g="0.0" b="0.0"/>  <!-- Pure black -->
    
    <TextOverlay>
        <Text>Apply precise force to the object</Text>
        <X>200</X>
        <Y>40</Y>
        <FontSize>20</FontSize>
        <Color r="0" g="255" b="255"/>  <!-- Cyan text -->
    </TextOverlay>
</SceneSettings>
```