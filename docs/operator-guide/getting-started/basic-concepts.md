# Basic Concepts

Core concepts and terminology for the haptic system.

## System Architecture

### Client-Server Model

The haptic system uses a **client-server architecture**:

```
┌─────────────────┐    gRPC/Network    ┌─────────────────┐
│  Python Client  │◄──────────────────►│   Core Server   │
│                 │                    │      (C++)      │
│ • Trial control │                    │ • Haptic render │
│ • Data logging  │                    │ • Physics sim   │
│ • User interface│                    │ • Device control│
└─────────────────┘                    └─────────────────┘
                                                 │
                                                 │ USB/Ethernet
                                                 ▼
                                       ┌─────────────────┐
                                       │ Haptic Device   │
                                       │ (Robot/Force)   │
                                       └─────────────────┘
```

**Why this design?**
- **Performance**: C++ core handles time-critical haptic rendering (1000 Hz)
- **Flexibility**: Python client makes experiments easy to configure and modify
- **Safety**: Server can enforce safety limits independent of client
- **Scalability**: Multiple clients can connect to one server

### Core Server (C++)

**Purpose**: Real-time haptic simulation and device control

**Responsibilities**:
- **Haptic Rendering**: Generate forces at 1000 Hz (every 1 millisecond)
- **Physics Simulation**: Update object positions, collisions, dynamics
- **Device Control**: Interface with haptic hardware (robots, force devices)
- **Safety Monitoring**: Enforce position/force limits to protect equipment and users

**Key characteristics**:
- Runs continuously during experiments
- Must maintain precise timing for smooth haptic feedback
- Handles all low-level hardware communication

### Python Client

**Purpose**: Experiment control and data collection

**Responsibilities**:
- **Trial Management**: Start/stop trials, manage experimental conditions
- **Data Logging**: Record participant movements, forces, trial outcomes
- **User Interface**: Provide feedback to operators and participants
- **Configuration**: Load XML files and send parameters to server

**Key characteristics**:
- Started for each experimental session
- Handles file I/O, data processing, user interaction
- Can be customized for specific experimental needs

## Core Concepts

### Trials

A **trial** is a single instance of a haptic task:

```
Trial Lifecycle:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│    Setup    │───►│    Start    │───►│   Active    │───►│     End     │
│             │    │ Conditions  │    │   Trial     │    │ Conditions  │
│ • Load XML  │    │             │    │             │    │             │
│ • Create    │    │ • Wait for  │    │ • Haptic    │    │ • Target    │
│   objects   │    │   trigger   │    │   feedback  │    │   reached   │
│ • Setup     │    │ • Show      │    │ • Physics   │    │ • Timeout   │
│   scene     │    │   ready     │    │ • Data log  │    │ • Error     │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
```

**Trial Components**:
- **Objects**: Things participants interact with (spheres, boxes, custom shapes)
- **Conditions**: Rules for when trials start and end
- **Data**: Information collected during the trial
- **Scene**: Visual environment (lighting, camera, background)

### XML Configuration

**Purpose**: Define experimental parameters in human-readable format

**Structure**:
```xml
<TrialConfig>
    <StartConditions>...</StartConditions>   <!-- When to begin trial -->
    <EndConditions>...</EndConditions>       <!-- When to end trial -->
    <Objects>...</Objects>                   <!-- What's in the scene -->
    <SceneSettings>...</SceneSettings>       <!-- Visual appearance -->
    <DataCollection>...</DataCollection>     <!-- What to record -->
</TrialConfig>
```

**Benefits**:
- **No coding required**: Configure experiments without programming
- **Version control**: Track changes to experimental parameters
- **Sharing**: Easy to share configurations between researchers
- **Documentation**: Self-documenting experimental setup

### Haptic Objects

**Definition**: 3D objects that provide force feedback when touched

**Types**:
- **Built-in**: Spheres, boxes, cylinders (ready to use)
- **Custom**: Researcher-defined objects with specific behaviors
- **Imported**: 3D models loaded from files

**Properties**:
- **Position**: Where in 3D space (x, y, z coordinates)
- **Appearance**: Color, size, texture
- **Haptic feel**: Stiffness, friction, texture
- **Behavior**: How it responds to touch (static, dynamic, physics)

**Example**:
```xml
<Object id="target_sphere" type="sphere">
    <Position x="0.1" y="0.0" z="0.0"/>     <!-- 10cm forward -->
    <Color r="1.0" g="0.0" b="0.0"/>        <!-- Red color -->
    <Parameters radius="0.03"/>              <!-- 3cm radius -->
</Object>
```

### Trial Conditions

**Purpose**: Automatically determine when trials start and end

**Start Conditions**: When does the actual trial begin?
- **Time-based**: "Start after 3 seconds" (preparation time)
- **Position-based**: "Start when participant reaches starting position"
- **Immediate**: "Start right away"

**End Conditions**: When should the trial terminate?
- **Success**: "End when target is reached"
- **Failure**: "End if time limit exceeded"
- **Safety**: "End if force/velocity limits exceeded"

**Example**:
```xml
<EndConditions>
    <!-- Success: reach target within 2cm -->
    <Condition name="position" success="true" priority="1">
        <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
    </Condition>
    
    <!-- Failure: timeout after 30 seconds -->
    <Condition name="time" success="false" priority="2">
        <Parameters duration_seconds="30.0"/>
    </Condition>
</EndConditions>
```

### Data Collection

**Purpose**: Record participant behavior for analysis

**Common data types**:
- **Position**: Where the haptic device is in 3D space
- **Velocity**: How fast the participant is moving
- **Force**: What forces are applied to the participant
- **Trial events**: When conditions are met, errors occur, etc.
- **Timing**: Precise timestamps for all measurements

**Output format**: CSV files with columns for each measurement

**Example data**:
```csv
timestamp,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z,force_x,force_y,force_z
0.001,0.000,0.000,0.000,0.0,0.0,0.0,0.0,0.0,0.0
0.002,0.001,0.000,0.000,1.0,0.0,0.0,0.5,0.0,0.0
...
```

## Coordinate Systems

### 3D Space Coordinates

The haptic system uses a **right-handed coordinate system**:

```
        Z ↑ (up)
          │
          │
          │
          └────────► Y (right)
         ╱
        ╱
       ↙ X (forward)
```

**Conventions**:
- **X-axis**: Forward/backward (positive = forward toward participant)
- **Y-axis**: Left/right (positive = right)
- **Z-axis**: Up/down (positive = up)
- **Units**: Meters (m)

**Typical workspace**: Most haptic devices work within ±0.2m in each direction

**Example positions**:
```xml
<Position x="0.1"  y="0.0"  z="0.0"/>   <!-- 10cm forward -->
<Position x="0.0"  y="0.1"  z="0.0"/>   <!-- 10cm right -->
<Position x="0.0"  y="0.0"  z="0.1"/>   <!-- 10cm up -->
<Position x="0.05" y="0.05" z="0.05"/>  <!-- Diagonal -->
```

### Rotations

Rotations use **roll-pitch-yaw** convention:
- **Roll**: Rotation around X-axis (forward)
- **Pitch**: Rotation around Y-axis (right)
- **Yaw**: Rotation around Z-axis (up)
- **Units**: Degrees

## Timing and Performance

### Haptic Rate (1000 Hz)

**Critical constraint**: Haptic feedback must be updated every **1 millisecond**

**Why this matters**:
- Human touch is extremely sensitive to timing
- Delays > 1ms cause noticeable degradation in feel
- Forces must be calculated and applied consistently

**System design impact**:
- Core server prioritizes haptic timing over everything else
- Complex calculations are done at lower rates (60-120 Hz)
- Safety systems must react within haptic constraints

### Graphics Rate (60-120 Hz)

**Visual updates**: Screen refreshes at monitor rate (typically 60-120 Hz)

**Physics updates**: Object movement, collision detection at graphics rate

**Data logging**: Recorded at graphics rate or lower (adjustable)

### Network Communication

**Client-server**: Communication as needed (typically 1-10 Hz)

**Trial control**: Start/stop commands sent when needed

**Data transfer**: Bulk data sent at end of trials

## Safety Concepts

### Hardware Limits

**Position limits**: Prevent device from moving outside safe workspace
**Force limits**: Limit maximum force output to protect users
**Velocity limits**: Prevent dangerous rapid movements
**Emergency stops**: Immediate halt of all motion

### Software Safety

**Watchdog timers**: Detect when systems become unresponsive
**Condition monitoring**: Automatically end trials if problems detected
**Graceful degradation**: System continues operating safely even with errors
**Logging**: All safety events recorded for analysis

### Participant Safety

**Force limitations**: Maximum forces set well below harmful levels
**Workspace boundaries**: Trials designed within comfortable reach
**Emergency procedures**: Clear protocols for stopping experiments
**Monitoring**: Continuous observation during trials

## Common Terminology

**Haptic**: Related to touch and force feedback
**DOF**: Degrees of Freedom (how many ways something can move)
**End-effector**: The part of the device the user holds/touches
**Workspace**: The 3D region where the device can operate
**Stiffness**: How rigid a virtual object feels
**Damping**: Resistance to motion (like moving through thick fluid)
**Friction**: Resistance when sliding along surfaces
**Force feedback**: Providing touch sensations through applied forces

**Trial**: A single experimental task
**Session**: A complete experiment with one participant
**Configuration**: XML file defining trial parameters
**Condition**: Rule determining trial start/end
**Object**: Virtual thing in the haptic environment
**Client**: Python program controlling experiments
**Server**: C++ program handling haptic rendering

## Understanding the Workflow

### Typical Experimental Session

1. **Preparation**
   - Start haptic server
   - Load experimental configuration
   - Calibrate haptic device
   - Verify safety settings

2. **Practice Trials**
   - Familiarize participant with system
   - Verify everything works correctly
   - Adjust parameters if needed

3. **Data Collection**
   - Run experimental trials
   - Monitor participant performance
   - Handle any technical issues
   - Log all relevant data

4. **Completion**
   - Save all data files
   - Shut down systems safely
   - Process and analyze results

### File Organization

```
haptech/
├── core/                    # C++ server code
│   ├── bin/core            # Compiled haptic server
│   └── src/                # Source code
├── python_client/          # Python client
│   ├── config/             # XML configuration files
│   ├── results/            # Data output directory
│   ├── main.py            # Main client program
│   └── settings.json      # Client configuration
└── docs/                   # Documentation
```

With these basic concepts, you're ready to start using the haptic system effectively. Next, try running your [First Trial](first-trial.md) to see these concepts in action!