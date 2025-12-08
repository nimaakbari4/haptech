# Hardware Issues

Troubleshooting haptic device problems.

## Device Detection Issues

### Franka Emika Robot

#### Robot Not Detected

**Symptoms:**
- "No Franka robot found on network"
- Server fails to start with device errors
- Connection timeouts

**Diagnostic Steps:**

1. **Check network connectivity**
   ```bash
   ping 172.16.0.2  # Default Franka IP
   ```
   
2. **Verify robot IP address**
   - Check robot's tablet/web interface
   - Common addresses: `172.16.0.2`, `192.168.1.100`
   
3. **Test with Franka tools**
   ```bash
   # If libfranka is installed
   franka_test_connectivity 172.16.0.2
   ```

**Common Solutions:**

1. **Network Configuration**
   - Set computer IP to same subnet as robot
   - Example: Robot `172.16.0.2`, Computer `172.16.0.1`
   - Disable other network adapters during testing

2. **Firewall Issues**
   ```bash
   # Temporarily disable firewall for testing
   sudo ufw disable
   # Re-enable after testing
   sudo ufw enable
   ```

3. **Robot State**
   - Ensure robot is in "Programming Mode"
   - Check that emergency stop is not activated
   - Verify robot is not in protective stop

#### Robot Connection Drops

**Symptoms:**
- "Connection lost during trial"
- Intermittent force feedback
- Robot suddenly stops moving

**Causes & Solutions:**

1. **Network instability**
   - Use dedicated ethernet connection
   - Avoid wireless connections
   - Check cable quality

2. **Real-time violations**
   - Close unnecessary applications
   - Disable power management
   - Use real-time Linux kernel if available

3. **Safety violations**
   - Check position/force limits in configuration
   - Verify workspace boundaries
   - Monitor participant movements

### Force Dimension Devices

#### Device Not Recognized

**Symptoms:**
- "Force Dimension device not found"
- USB device not detected
- Driver errors in system logs

**Diagnostic Steps:**

1. **Check USB connection**
   ```bash
   lsusb | grep -i "force dimension"
   dmesg | grep -i usb  # Check for USB errors
   ```

2. **Verify device permissions**
   ```bash
   ls -la /dev/ttyUSB*  # Check device files
   groups $USER         # Check user groups
   ```

**Solutions:**

1. **Install/Update Drivers**
   - Download latest SDK from Force Dimension
   - Follow installation instructions for your OS
   - Verify installation with vendor test tools

2. **USB Permissions**
   ```bash
   # Add user to dialout group
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```

3. **Device Rules**
   Create udev rule if needed:
   ```bash
   sudo nano /etc/udev/rules.d/99-force-dimension.rules
   ```
   Add:
   ```
   SUBSYSTEM=="usb", ATTRS{idVendor}="1451", GROUP="dialout", MODE="0666"
   ```

#### Poor Force Feedback Quality

**Symptoms:**
- Jittery or noisy force feedback
- Inconsistent force levels
- Device feels "sticky" or "jumpy"

**Solutions:**

1. **Calibration**
   - Run device calibration tool
   - Ensure device is properly zeroed
   - Check for mechanical obstructions

2. **Update Rates**
   - Verify 1000 Hz haptic loop is maintained
   - Check CPU usage during trials
   - Reduce graphics complexity if needed

3. **Environmental Factors**
   - Remove magnetic interference sources
   - Ensure stable power supply
   - Check ambient temperature

## Performance Problems

### Haptic Loop Violations

**Symptoms:**
- "Haptic loop overrun detected"
- Force feedback feels delayed or choppy
- Performance warnings in logs

**Immediate Actions:**
1. Pause current trial
2. Check system load
3. Close unnecessary applications

**Long-term Solutions:**

1. **System Optimization**
   ```bash
   # Check CPU governor
   cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   
   # Set to performance mode
   sudo cpufreq-set -g performance
   ```

2. **Process Priority**
   ```bash
   # Run core server with high priority
   sudo nice -n -10 ./core
   ```

3. **Memory Management**
   - Ensure sufficient RAM available
   - Disable swap if using SSD
   - Close memory-intensive applications

### Position Tracking Issues

**Symptoms:**
- Device position seems incorrect
- Jumpy or erratic movement
- Objects not aligning with device position

**Diagnostic Tests:**

1. **Basic Movement Test**
   - Move device slowly in known directions
   - Check if visual feedback matches movement
   - Test all axes (X, Y, Z)

2. **Calibration Verification**
   - Return device to known position
   - Check if system reports correct coordinates
   - Test workspace boundaries

**Solutions:**

1. **Re-calibration**
   - Follow device-specific calibration procedure
   - Ensure device is properly homed
   - Check for mechanical drift

2. **Coordinate System**
   - Verify coordinate system matches expectations
   - Check transformation matrices in configuration
   - Test with simple movement patterns

3. **Encoder Issues**
   - Check for encoder errors in device logs
   - Look for mechanical wear or damage
   - Contact manufacturer if hardware issue suspected

## Safety Issues

### Emergency Stop Procedures

**When to use emergency stop:**
- Unexpected device movement
- Participant discomfort or injury risk
- System malfunction
- Any unsafe condition

**Emergency stop methods:**
1. **Hardware E-stop**: Red button on device (if available)
2. **Software kill**: `Ctrl+C` in server terminal
3. **Power disconnect**: Unplug device power cable
4. **Network disconnect**: Unplug ethernet cable

**After emergency stop:**
1. Ensure participant safety
2. Document what happened
3. Check device for damage
4. Review logs for cause
5. Don't restart until issue is resolved

### Force Limit Violations

**Symptoms:**
- "Force limit exceeded" warnings
- Device suddenly stops providing force
- Protective stop activation

**Immediate Actions:**
1. Stop trial immediately
2. Check participant condition
3. Review force settings in configuration

**Prevention:**

1. **Conservative Force Limits**
   ```xml
   <SafetyLimits>
     <MaxForce>5.0</MaxForce>  <!-- Newtons -->
     <MaxVelocity>0.5</MaxVelocity>  <!-- m/s -->
   </SafetyLimits>
   ```

2. **Gradual Force Introduction**
   - Start with low forces during testing
   - Gradually increase based on comfort
   - Always test with researcher first

3. **Participant Training**
   - Explain emergency stop procedures
   - Practice basic movements
   - Start with simple trials

### Position Limit Violations

**Symptoms:**
- Device stops at workspace boundary
- "Position limit exceeded" errors
- Unexpected movement restrictions

**Solutions:**

1. **Check Workspace Configuration**
   ```xml
   <WorkspaceLimits>
     <Min x="-0.3" y="-0.3" z="0.0"/>
     <Max x="0.3" y="0.3" z="0.4"/>
   </WorkspaceLimits>
   ```

2. **Device Calibration**
   - Verify home position is correct
   - Check device coordinate system
   - Re-calibrate if necessary

3. **Trial Design**
   - Ensure targets are within workspace
   - Add buffer zones around limits
   - Test all movements before participant trials

## Maintenance

### Regular Maintenance Tasks

**Daily (if in use):**
- Visual inspection for damage
- Check cable connections
- Verify emergency stops work
- Clean device surfaces

**Weekly:**
- Check calibration accuracy
- Test all safety systems
- Review error logs
- Update software if needed

**Monthly:**
- Deep clean device
- Check mechanical wear
- Verify force calibration
- Update documentation

### Mechanical Issues

**Common Problems:**

1. **Excessive Play or Backlash**
   - Check for worn bearings
   - Inspect belt tension
   - Look for loose mechanical connections

2. **Unusual Noises**
   - Grinding: Possible bearing wear
   - Clicking: Loose parts or debris
   - Whining: Belt or motor issues

3. **Stick-Slip Motion**
   - Clean and lubricate as per manufacturer
   - Check for cable interference
   - Verify power supply stability

**When to Contact Manufacturer:**
- Any mechanical damage
- Persistent calibration issues
- Safety system failures
- Performance degradation despite troubleshooting
- Warranty or service questions

### Preventive Measures

1. **Environmental Control**
   - Keep device clean and dust-free
   - Maintain stable temperature
   - Avoid magnetic interference
   - Provide adequate ventilation

2. **Proper Usage**
   - Train all users on proper operation
   - Follow manufacturer guidelines
   - Don't exceed force/speed limits
   - Use appropriate safety margins

3. **Documentation**
   - Log all issues and resolutions
   - Keep maintenance records
   - Document configuration changes
   - Track device usage hours