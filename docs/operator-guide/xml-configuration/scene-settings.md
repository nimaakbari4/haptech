# Scene Settings

Complete guide to configuring camera, lighting, background, and visual appearance in XML trial configurations.

## Overview

Scene settings control the visual environment for your haptic trials. These settings determine how participants see the virtual world, affecting their spatial understanding and task performance.

**Key components**:
- **Camera**: Viewpoint and perspective 
- **Background**: Colors and environment
- **Lighting**: Illumination and shadows
- **Visual effects**: Transparency, materials, post-processing

## Basic Scene Structure

```xml
<SceneSettings>
    <!-- Camera position and orientation -->
    <Camera>
        <Position x="1.0" y="1.0" z="2.0"/>
        <LookAt x="0.0" y="0.0" z="0.0"/>
        <Up x="0.0" y="1.0" z="0.0"/>
    </Camera>
    
    <!-- Background appearance -->
    <BackgroundColor r="0.2" g="0.2" b="0.2"/>
    
    <!-- Optional: Advanced lighting -->
    <Lighting>
        <AmbientLight intensity="0.3"/>
        <DirectionalLight intensity="0.8"/>
    </Lighting>
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

### Gradient Backgrounds

```xml
<Background>
    <Gradient>
        <TopColor r="0.4" g="0.6" b="1.0"/>     <!-- Light blue top -->
        <BottomColor r="0.1" g="0.2" b="0.4"/>  <!-- Dark blue bottom -->
    </Gradient>
</Background>
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

## Lighting Configuration

### Basic Lighting

Simple ambient lighting (usually sufficient):

```xml
<Lighting>
    <!-- Even illumination from all directions -->
    <AmbientLight intensity="0.7"/>
</Lighting>
```

### Advanced Lighting

Directional lighting for depth perception:

```xml
<Lighting>
    <!-- Base ambient light -->
    <AmbientLight intensity="0.3"/>
    
    <!-- Main directional light -->
    <DirectionalLight intensity="0.8">
        <Direction x="-0.5" y="-0.3" z="-1.0"/>
        <Color r="1.0" g="1.0" b="0.9"/>
    </DirectionalLight>
    
    <!-- Optional: Fill light -->
    <DirectionalLight intensity="0.3">
        <Direction x="0.5" y="0.2" z="-0.8"/>
        <Color r="0.8" g="0.9" b="1.0"/>  <!-- Slight blue tint -->
    </DirectionalLight>
</Lighting>
```

### Lighting Types

#### Ambient Light
Provides even illumination without shadows:

```xml
<AmbientLight intensity="0.5"/>  <!-- 0.0 = dark, 1.0 = full -->
```

#### Directional Light
Simulates sunlight with shadows:

```xml
<DirectionalLight intensity="0.8">
    <Direction x="-1.0" y="-1.0" z="-2.0"/>  <!-- From upper-left -->
    <Color r="1.0" g="1.0" b="0.9"/>         <!-- Warm white -->
</DirectionalLight>
```

#### Point Light
Localized light source:

```xml
<PointLight intensity="1.0">
    <Position x="0.5" y="0.5" z="1.0"/>
    <Color r="1.0" g="0.9" b="0.8"/>  <!-- Warm light -->
    <Attenuation constant="1.0" linear="0.1" quadratic="0.01"/>
</PointLight>
```

## Visual Effects

### Material Properties

Control how objects interact with light:

```xml
<MaterialSettings>
    <DefaultMaterial>
        <Ambient r="0.2" g="0.2" b="0.2"/>
        <Diffuse r="0.8" g="0.8" b="0.8"/>
        <Specular r="0.5" g="0.5" b="0.5"/>
        <Shininess value="50.0"/>
    </DefaultMaterial>
</MaterialSettings>
```

### Shadows

Enable shadows for better depth perception:

```xml
<RenderSettings>
    <Shadows enabled="true" quality="medium"/>
    <ShadowColor r="0.1" g="0.1" b="0.1"/>  <!-- Dark gray shadows -->
</RenderSettings>
```

### Anti-aliasing

Smooth edges for better visual quality:

```xml
<RenderSettings>
    <Antialiasing enabled="true" samples="4"/>
</RenderSettings>
```

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
    
    <Lighting>
        <AmbientLight intensity="0.8"/>
    </Lighting>
</SceneSettings>
```

### Professional Setup
Balanced lighting with depth:

```xml
<SceneSettings>
    <Camera>
        <Position x="1.2" y="1.0" z="1.8"/>
        <LookAt x="0.0" y="0.0" z="0.0"/>
        <Up x="0.0" y="0.0" z="1.0"/>
    </Camera>
    
    <BackgroundColor r="0.25" g="0.25" b="0.25"/>  <!-- Dark gray -->
    
    <Lighting>
        <AmbientLight intensity="0.4"/>
        <DirectionalLight intensity="0.7">
            <Direction x="-0.6" y="-0.4" z="-1.0"/>
            <Color r="1.0" g="1.0" b="0.95"/>
        </DirectionalLight>
    </Lighting>
    
    <RenderSettings>
        <Shadows enabled="true" quality="medium"/>
        <Antialiasing enabled="true" samples="4"/>
    </RenderSettings>
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
    
    <Lighting>
        <AmbientLight intensity="0.9"/>  <!-- Bright even lighting -->
    </Lighting>
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
    
    <Lighting>
        <AmbientLight intensity="0.3"/>
        <PointLight intensity="1.2">
            <Position x="0.2" y="0.1" z="0.3"/>
            <Color r="1.0" g="1.0" b="1.0"/>
        </PointLight>
    </Lighting>
</SceneSettings>
```

## Task-Specific Recommendations

### Reaching Tasks
- **Camera**: Angled perspective to show depth
- **Background**: Medium gray for good contrast
- **Lighting**: Ambient with subtle directional light

### Path Following
- **Camera**: Overhead view for clear path visibility
- **Background**: Dark to make path elements stand out
- **Lighting**: Bright ambient for clear visibility

### Precision Manipulation
- **Camera**: Close side or angled view
- **Background**: High contrast (black or white)
- **Lighting**: Bright, focused lighting

### Force Application
- **Camera**: Side view to show force direction
- **Background**: Neutral gray
- **Lighting**: Directional to show object deformation

### Multi-target Selection
- **Camera**: Wide-angle view to show all targets
- **Background**: Low distraction color
- **Lighting**: Even ambient lighting

## Performance Considerations

### Rendering Optimization

```xml
<RenderSettings>
    <!-- Reduce quality for better performance -->
    <Shadows enabled="false"/>
    <Antialiasing enabled="false"/>
    <LOD enabled="true"/>  <!-- Level of detail -->
    
    <!-- Or maintain quality with selective settings -->
    <Shadows enabled="true" quality="low"/>
    <Antialiasing enabled="true" samples="2"/>
</RenderSettings>
```

### Performance Guidelines
- **Simple lighting**: Use mostly ambient light for best performance
- **Minimal effects**: Avoid complex shadows and reflections for real-time trials
- **Stable framerate**: Prioritize smooth haptic update rate over visual quality
- **Test thoroughly**: Verify performance on target hardware

## Troubleshooting

### Common Issues

**Camera not showing objects**:
```
Problem: Black screen or objects not visible
Solution: Check camera Position and LookAt coordinates, verify objects are within view
```

**Poor visibility**:
```
Problem: Objects are too dark or washed out
Solution: Adjust lighting intensity, check background contrast
```

**Confusing perspective**:
```
Problem: Difficult to judge object positions
Solution: Try different camera angles, add subtle shadows for depth
```

**Performance issues**:
```
Problem: Slow rendering, choppy haptics
Solution: Reduce lighting complexity, disable shadows, simplify effects
```

### Validation Checklist

- **Camera position**: Is the camera outside the haptic workspace?
- **Field of view**: Are all important objects visible?
- **Contrast**: Can objects be clearly distinguished from background?
- **Lighting**: Is the scene adequately lit without being too bright?
- **Performance**: Does the scene maintain smooth haptic update rates?

## Best Practices

### Visual Design
- **Consistency**: Use similar lighting across related trials
- **Contrast**: Ensure adequate contrast between objects and background
- **Simplicity**: Avoid unnecessary visual complexity
- **Accessibility**: Consider colorblind and visually impaired users

### Camera Placement
- **Natural angles**: Choose viewpoints that feel intuitive
- **Task appropriate**: Match camera angle to task requirements
- **Avoid extremes**: Don't use angles that cause confusion
- **Test thoroughly**: View the scene from the participant's perspective

### Lighting Strategy
- **Start simple**: Begin with ambient lighting only
- **Add gradually**: Introduce directional lighting if needed for depth
- **Performance first**: Prioritize haptic performance over visual effects
- **Consistent mood**: Match lighting to experimental goals

## Next Steps

- **Objects**: Learn how lighting affects [object appearance](objects.md)
- **Trial Conditions**: Configure [conditions](trial-conditions.md) for your visual setup
- **Data Collection**: Set up [data logging](data-collection.md) for scene-related metrics
- **Examples**: Browse scene examples in the examples directory