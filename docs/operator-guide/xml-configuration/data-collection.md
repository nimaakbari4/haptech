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
        <Orientation enabled="true"/>
        <AngularVelocity enabled="true"/>
        <ProxyPosition enabled="true"/>
    </Device>
    
    <!-- What object interaction data to record -->
    <Objects>
        <Position enabled="true"/>
        <Orientation enabled="true"/>
        <Contact enabled="true"/>
        <ContactPoint enabled="true"/>
        <ContactNormal enabled="true"/>
        <PenetrationDepth enabled="true"/>
    </Objects>
</DataCollection>
```

## Device Data Collection

Records data from the haptic device itself.

### Position and Motion Data

**Position data includes**:
- X, Y, Z coordinates in meters
- Timestamp for each sample
- Reference frame: workspace center at (0,0,0)

**Velocity data includes**:
- Linear velocity in X, Y, Z directions (m/s)
- Speed magnitude
- Direction vector

### Force and Haptic Data

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
    <Position enabled="true"/>
    <Velocity enabled="true"/>
    <Force enabled="true"/>
    <ProxyPosition enabled="true"/>
    <Orientation enabled="true"/>
    <AngularVelocity enabled="true"/>
</Device>
```

## Object Interaction Data

Records how participants interact with virtual objects.

### Basic Object Data

```xml
<Objects>
    <!-- Position/orientation of objects -->
    <Position enabled="true"/>
    <Orientation enabled="true"/>
    
    <!-- Contact detection -->
    <Contact enabled="true"/>
    <ContactPoint enabled="true"/>
    <ContactNormal enabled="true"/>
    <PenetrationDepth enabled="true"/>
</Objects>
```

**Contact data includes**:
- Contact - boolean (0 = false, or 1 = true) indicating if the cursor is in contact with that object for a specific point in time
- ContactPoint - Vector indicating the position where the cursor came in contact
- ContactNormal - Normal vector of the collision
- PenetrationDepth - Scalar distance, how much the cursor penetrated the surface of the object

## Data Format and Output

Data collection is automatically handled by the system:

**Default format**: CSV files with timestamped data
**Sampling rate**: Configurable up to 1000Hz (matches haptic update rate)
**File naming**: Automatic based on trial configuration and timestamp
**Storage location**: Results directory specified in Python client settings

**CSV output includes**:
- Timestamp column (seconds since trial start)
- Device position (X, Y, Z coordinates)
- Device velocity (if enabled)
- Device orientation (if enabled)
- Force data (X, Y, Z components)
- Object contact information

## Performance Considerations

Data collection is optimized for real-time haptic applications:

**Automatic optimization**:
- Efficient CSV writing during trials
- Minimal impact on haptic update rates
- Streamlined data buffering
- Optimized for 1000Hz haptic loop

**File management**:
- Automatic timestamped filenames
- Results stored in configured directory
- CSV format for easy analysis
- File size depends on enabled data channels and trial duration

**Best practices**:
- Enable only needed data channels
- Monitor system performance during long trials
- Use consistent data collection settings across experiments

## Troubleshooting

### Common Issues

**Missing data columns**:
```
Problem: Expected data columns don't appear in CSV
Solution: Check that data collection options are enabled="true" in XML configuration
```

**No object interaction data**:
```
Problem: Object contact data shows no values
Solution: Ensure objects are being contacted during trial, verify object IDs match, verify object actually has a collision detector
```

**Performance impact**:
```
Problem: Haptic system becomes slow during data collection
Solution: Reduce enabled data channels, check system resources
```

**Empty or incomplete files**:
```
Problem: Data files are empty or missing samples
Solution: Verify trial runs to completion, check file permissions in results directory, check that DataCollection element in XML is set up correctly
```

### Validation Steps

1. **Test collection**: Run short trial to verify data is being recorded
2. **Check headers**: Ensure CSV headers match enabled data collection options
3. **Verify data**: Confirm non-zero values appear for expected interactions
4. **File integrity**: Check that files can be opened in Excel/MATLAB/Python
5. **Performance**: Monitor system performance during collection

## Best Practices

### Data Collection Strategy
- **Start minimal**: Begin with basic position and contact data
- **Add gradually**: Include additional data types as needed for your analysis
- **Test thoroughly**: Verify data quality with short test trials
- **Document settings**: Keep record of collection configuration for each experiment

### Configuration Management
- **Consistent settings**: Use the same data collection configuration across related trials
- **Essential data**: Focus on data directly relevant to your research questions
- **Performance balance**: Consider system performance when enabling many data channels
- **Backup configurations**: Save working XML configurations for reuse

### Data Management
- **Systematic naming**: Use consistent filename conventions
- **Organize by experiment**: Create directory structure for different studies
- **Backup important data**: Keep copies of critical experimental data
- **Documentation**: Include metadata about experimental conditions in filenames or logs

## Next Steps

- **Trial Conditions**: Learn how to trigger data collection with [trial conditions](trial-conditions.md)
- **Object Interactions**: Configure [objects](objects.md) for optimal data collection
- **Analysis Tools**: Explore data analysis examples in the examples directory
- **Advanced Features**: Set up real-time data streaming and processing