# Data Collection

Complete guide to configuring data collection and logging in XML trial configurations.

## Overview

Data collection configuration controls what information is recorded during trials. This data is essential for analysis, research validation, and understanding participant performance.

**Key concepts**:
- **Device data**: Haptic device position, force, velocity
- **Object data**: Virtual object interactions and states
- **Trial events**: Start/end times, condition triggers
- **Performance metrics**: Success rates, completion times
- **Sampling rates**: How frequently data is recorded

## Basic Data Collection Structure

```xml
<DataCollection>
    <!-- What device data to record -->
    <Device>
        <Position enabled="true"/>
        <Velocity enabled="true"/>
        <Force enabled="true"/>
    </Device>
    
    <!-- What object interaction data to record -->
    <Objects>
        <Position enabled="true"/>
        <Contact enabled="true"/>
        <Force enabled="true"/>
    </Objects>
    
    <!-- File and timing settings -->
    <Output>
        <SamplingRate>1000</SamplingRate>  <!-- Hz -->
        <Format>csv</Format>
        <Filename>trial_data</Filename>
    </Output>
</DataCollection>
```

## Device Data Collection

Records data from the haptic device itself.

### Position and Motion Data

```xml
<Device>
    <!-- 3D position of haptic device end-effector -->
    <Position enabled="true"/>
    
    <!-- Linear velocity in 3D space -->
    <Velocity enabled="true"/>
    
    <!-- Linear acceleration -->
    <Acceleration enabled="false"/>
    
    <!-- Device orientation (if supported) -->
    <Orientation enabled="false"/>
    
    <!-- Angular velocity -->
    <AngularVelocity enabled="false"/>
</Device>
```

**Position data includes**:
- X, Y, Z coordinates in meters
- Timestamp for each sample
- Reference frame: workspace center at (0,0,0)

**Velocity data includes**:
- Linear velocity in X, Y, Z directions (m/s)
- Speed magnitude
- Direction vector

### Force and Haptic Data

```xml
<Device>
    <!-- Forces applied by haptic device -->
    <Force enabled="true"/>
    
    <!-- Position of haptic proxy/avatar -->
    <ProxyPosition enabled="true"/>
    
    <!-- Torques (if device supports rotation) -->
    <Torque enabled="false"/>
    
    <!-- Device-specific sensor data -->
    <RawSensorData enabled="false"/>
</Device>
```

**Force data includes**:
- Force vector in X, Y, Z directions (Newtons)
- Force magnitude
- Contact state (touching/not touching)

**Proxy position**:
- Position where participant "feels" they are
- May differ from actual device position due to haptic rendering
- Important for understanding haptic interaction

### Complete Device Configuration

```xml
<Device>
    <!-- Essential data for most experiments -->
    <Position enabled="true"/>
    <Velocity enabled="true"/>
    <Force enabled="true"/>
    <ProxyPosition enabled="true"/>
    
    <!-- Optional advanced data -->
    <Acceleration enabled="false"/>
    <Orientation enabled="false"/>
    <AngularVelocity enabled="false"/>
    <Torque enabled="false"/>
</Device>
```

## Object Interaction Data

Records how participants interact with virtual objects.

### Basic Object Data

```xml
<Objects>
    <!-- Position/orientation of objects (if they move) -->
    <Position enabled="true"/>
    <Orientation enabled="true"/>
    
    <!-- Contact detection -->
    <Contact enabled="true"/>
    <ContactPoint enabled="true"/>
    <ContactNormal enabled="true"/>
    
    <!-- Force interactions -->
    <Force enabled="true"/>
    <PenetrationDepth enabled="true"/>
</Objects>
```

### Advanced Object Data

```xml
<Objects>
    <!-- Object movement and physics -->
    <Velocity enabled="true"/>
    <AngularVelocity enabled="true"/>
    
    <!-- Detailed force breakdown -->
    <SpringForce enabled="true"/>    <!-- Force from object stiffness -->
    <DampingForce enabled="true"/>   <!-- Force from object damping -->
    <FrictionForce enabled="true"/>  <!-- Friction forces -->
    
    <!-- Object state changes -->
    <StateChanges enabled="true"/>   <!-- Visibility, color changes -->
    <UserInteractions enabled="true"/>  <!-- Custom interaction events -->
</Objects>
```

### Per-Object Data Collection

Collect data for specific objects only:

```xml
<Objects>
    <!-- Collect data for specific objects -->
    <ObjectFilter>
        <Include id="target_sphere"/>
        <Include id="movable_ball"/>
        <!-- Exclude other objects from data collection -->
    </ObjectFilter>
    
    <!-- What data to collect for included objects -->
    <Position enabled="true"/>
    <Contact enabled="true"/>
    <Force enabled="true"/>
</Objects>
```

## Output Configuration

### File Settings

```xml
<Output>
    <!-- Data file format -->
    <Format>csv</Format>  <!-- csv, json, hdf5 -->
    
    <!-- Base filename (timestamp will be added) -->
    <Filename>trial_data</Filename>
    
    <!-- Output directory -->
    <Directory>results/</Directory>
    
    <!-- File compression -->
    <Compression>none</Compression>  <!-- none, gzip, bzip2 -->
</Output>
```

### Sampling Configuration

```xml
<Output>
    <!-- Data collection frequency -->
    <SamplingRate>1000</SamplingRate>  <!-- Hz (samples per second) -->
    
    <!-- Buffer settings for performance -->
    <BufferSize>1000</BufferSize>  <!-- Number of samples to buffer -->
    
    <!-- Automatic file splitting -->
    <MaxFileSize>100MB</MaxFileSize>
    <MaxDuration>600</MaxDuration>  <!-- seconds -->
</Output>
```

### Data Format Options

#### CSV Format (Recommended for beginners)
```xml
<Format>csv</Format>
```
- **Pros**: Easy to read, works with Excel, MATLAB, Python
- **Cons**: Larger file sizes, slower for very high-frequency data
- **Best for**: Most research applications, moderate data rates

#### JSON Format
```xml
<Format>json</Format>
```
- **Pros**: Human readable, preserves data structure
- **Cons**: Large file sizes, slower parsing
- **Best for**: Complex data structures, debugging

#### HDF5 Format
```xml
<Format>hdf5</Format>
```
- **Pros**: Compact, fast, handles large datasets
- **Cons**: Requires special tools to read
- **Best for**: High-frequency data, large experiments

## Sampling Rate Guidelines

### Recommended Rates by Task Type

**Precision manipulation**: 1000-2000 Hz
```xml
<SamplingRate>1000</SamplingRate>  <!-- 1000 samples/second -->
```

**General reaching**: 500-1000 Hz
```xml
<SamplingRate>500</SamplingRate>
```

**Slow movement tasks**: 100-500 Hz
```xml
<SamplingRate>200</SamplingRate>
```

**Event-based tasks**: 50-100 Hz
```xml
<SamplingRate>100</SamplingRate>
```

### Performance Considerations

- **Higher rates**: More detailed data, larger files, more processing
- **Lower rates**: Smaller files, less detail, may miss rapid events
- **Balance**: Choose rate appropriate for your analysis needs
- **Haptic stability**: Don't compromise haptic update rate for data collection

## Complete Configuration Examples

### Minimal Data Collection
Basic data for simple reaching tasks:

```xml
<DataCollection>
    <Device>
        <Position enabled="true"/>
        <Force enabled="true"/>
    </Device>
    
    <Objects>
        <Contact enabled="true"/>
    </Objects>
    
    <Output>
        <SamplingRate>500</SamplingRate>
        <Format>csv</Format>
        <Filename>reaching_data</Filename>
    </Output>
</DataCollection>
```

### Comprehensive Data Collection
Detailed data for research analysis:

```xml
<DataCollection>
    <Device>
        <Position enabled="true"/>
        <Velocity enabled="true"/>
        <Acceleration enabled="true"/>
        <Force enabled="true"/>
        <ProxyPosition enabled="true"/>
    </Device>
    
    <Objects>
        <Position enabled="true"/>
        <Contact enabled="true"/>
        <ContactPoint enabled="true"/>
        <ContactNormal enabled="true"/>
        <Force enabled="true"/>
        <SpringForce enabled="true"/>
        <DampingForce enabled="true"/>
        <PenetrationDepth enabled="true"/>
    </Objects>
    
    <Output>
        <SamplingRate>1000</SamplingRate>
        <Format>hdf5</Format>
        <Filename>detailed_trial_data</Filename>
        <Directory>results/detailed/</Directory>
        <Compression>gzip</Compression>
    </Output>
</DataCollection>
```

### Performance-Optimized Collection
Focused data for real-time analysis:

```xml
<DataCollection>
    <!-- Only essential data -->
    <Device>
        <Position enabled="true"/>
        <Velocity enabled="false"/>  <!-- Calculate from position if needed -->
        <Force enabled="true"/>
        <ProxyPosition enabled="false"/>
    </Device>
    
    <!-- Focus on specific objects -->
    <Objects>
        <ObjectFilter>
            <Include id="target_1"/>
            <Include id="target_2"/>
        </ObjectFilter>
        
        <Contact enabled="true"/>
        <Position enabled="false"/>  <!-- Objects don't move -->
        <Force enabled="false"/>    <!-- Device force is sufficient -->
    </Objects>
    
    <Output>
        <SamplingRate>500</SamplingRate>  <!-- Moderate rate -->
        <Format>csv</Format>
        <BufferSize>500</BufferSize>     <!-- Smaller buffer -->
    </Output>
</DataCollection>
```

### Multi-Trial Experiment Collection
Organized data for batch experiments:

```xml
<DataCollection>
    <Device>
        <Position enabled="true"/>
        <Velocity enabled="true"/>
        <Force enabled="true"/>
    </Device>
    
    <Objects>
        <Contact enabled="true"/>
        <ContactPoint enabled="true"/>
        <Force enabled="true"/>
    </Objects>
    
    <Output>
        <SamplingRate>1000</SamplingRate>
        <Format>csv</Format>
        
        <!-- Automatic file organization -->
        <Filename>trial_{trial_number}_{participant_id}</Filename>
        <Directory>results/{date}/{participant_id}/</Directory>
        
        <!-- Auto-split files for each trial -->
        <SplitByTrial>true</SplitByTrial>
    </Output>
    
    <!-- Trial metadata -->
    <Metadata>
        <ParticipantID enabled="true"/>
        <TrialNumber enabled="true"/>
        <Timestamp enabled="true"/>
        <TrialConditions enabled="true"/>
    </Metadata>
</DataCollection>
```

## Data Analysis Preparation

### Column Headers
CSV files include descriptive headers:

```csv
Timestamp,DevicePos_X,DevicePos_Y,DevicePos_Z,DeviceVel_X,DeviceVel_Y,DeviceVel_Z,Force_X,Force_Y,Force_Z,Contact_target_sphere,ContactPoint_X,ContactPoint_Y,ContactPoint_Z
0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,false,,,
0.001,0.001,0.000,0.000,1.000,0.000,0.000,0.000,0.000,0.000,false,,,
...
```

### Units and Coordinate System
- **Position**: Meters, workspace center at (0,0,0)
- **Velocity**: Meters per second
- **Force**: Newtons
- **Time**: Seconds since trial start
- **Angles**: Degrees (orientation data)

### Common Analysis Workflows

**Python/Pandas**:
```python
import pandas as pd
data = pd.read_csv('trial_data_20231207_143022.csv')
positions = data[['DevicePos_X', 'DevicePos_Y', 'DevicePos_Z']]
velocities = data[['DeviceVel_X', 'DeviceVel_Y', 'DeviceVel_Z']]
```

**MATLAB**:
```matlab
data = readtable('trial_data_20231207_143022.csv');
positions = data{:, {'DevicePos_X', 'DevicePos_Y', 'DevicePos_Z'}};
velocities = data{:, {'DeviceVel_X', 'DeviceVel_Y', 'DeviceVel_Z'}};
```

**R**:
```r
data <- read.csv('trial_data_20231207_143022.csv')
positions <- data[c('DevicePos_X', 'DevicePos_Y', 'DevicePos_Z')]
velocities <- data[c('DeviceVel_X', 'DeviceVel_Y', 'DeviceVel_Z')]
```

## Performance and Storage

### File Size Estimation

Approximate file sizes for 60-second trial:

**Minimal data (position + contact)** @ 500Hz:
- CSV: ~2 MB
- HDF5: ~0.5 MB

**Comprehensive data** @ 1000Hz:
- CSV: ~15 MB
- HDF5: ~3 MB

**High-frequency detailed data** @ 2000Hz:
- CSV: ~50 MB
- HDF5: ~10 MB

### Storage Management

```xml
<Output>
    <!-- Automatic cleanup of old files -->
    <RetentionPolicy>
        <MaxAge>30</MaxAge>  <!-- days -->
        <MaxSize>10GB</MaxSize>
    </RetentionPolicy>
    
    <!-- Compression for long-term storage -->
    <Compression>gzip</Compression>
    <CompressionLevel>6</CompressionLevel>  <!-- 1-9, higher = more compression -->
</Output>
```

## Troubleshooting

### Common Issues

**Large file sizes**:
```
Problem: Data files are too large
Solution: Reduce sampling rate, disable unnecessary data channels, use HDF5 format
```

**Missing data**:
```
Problem: Some data channels show no values
Solution: Check that objects are being contacted, verify enabled settings
```

**Performance impact**:
```
Problem: Haptic system becomes slow during data collection
Solution: Reduce sampling rate, increase buffer size, use faster storage
```

**File corruption**:
```
Problem: Data files are incomplete or corrupted
Solution: Check disk space, increase buffer size, use file splitting
```

### Validation Steps

1. **Test collection**: Run short trial to verify data is being recorded
2. **Check headers**: Ensure CSV headers match expected data
3. **Verify sampling**: Confirm data rate matches configuration
4. **File integrity**: Check that files can be opened and read
5. **Performance**: Monitor system performance during collection

## Best Practices

### Data Collection Strategy
- **Start minimal**: Begin with basic position and contact data
- **Add gradually**: Include additional data types as needed
- **Test thoroughly**: Verify data quality before running full experiments
- **Document settings**: Keep record of collection configuration for each experiment

### Performance Optimization
- **Appropriate sampling**: Don't collect more data than you need
- **Efficient formats**: Use HDF5 for high-frequency or large datasets
- **Buffering**: Use appropriate buffer sizes for your system
- **Storage location**: Use fast storage (SSD) for real-time collection

### Data Management
- **Consistent naming**: Use systematic filename conventions
- **Organize by experiment**: Create directory structure for different studies
- **Backup important data**: Keep copies of critical experimental data
- **Documentation**: Include metadata about experimental conditions

## Next Steps

- **Trial Conditions**: Learn how to trigger data collection with [trial conditions](trial-conditions.md)
- **Object Interactions**: Configure [objects](objects.md) for optimal data collection
- **Analysis Tools**: Explore data analysis examples in the examples directory
- **Advanced Features**: Set up real-time data streaming and processing