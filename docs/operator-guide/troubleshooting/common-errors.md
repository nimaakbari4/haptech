# Common Errors

Frequent error messages and their solutions.

## Server Connection Errors

### "Connection refused" or "Failed to connect to server"

**Symptoms:**
```
[ERROR] Failed to connect to haptic server at localhost:50051
[ERROR] Connection refused
```

**Causes & Solutions:**

1. **Core server not running**
   - **Solution**: Start the core server first
   ```bash
   cd /path/to/haptech/core/bin
   ./core
   ```

2. **Wrong server address**
   - **Check**: Verify `settings.json` has correct `server_host` and `server_port`
   - **Common mistake**: Using `127.0.0.1` instead of `localhost` or vice versa
   
3. **Firewall blocking connection**
   - **Solution**: Allow port 50051 through firewall
   ```bash
   sudo ufw allow 50051
   ```

4. **Server crashed**
   - **Check**: Look for error messages in server terminal
   - **Solution**: Restart server, check hardware connections

### "Service unavailable"

**Symptoms:**
```
[ERROR] RPC failed with status UNAVAILABLE
```

**Causes & Solutions:**

1. **Server overloaded**
   - **Solution**: Wait a few seconds and retry
   - **Prevention**: Close unnecessary applications

2. **Network congestion**
   - **Solution**: Use localhost instead of network address for local setups

## XML Configuration Errors

### "Failed to parse configuration file"

**Symptoms:**
```
[ERROR] XML parsing failed: Expected '<', found 'T'
[ERROR] Configuration validation failed
```

**Common causes:**

1. **Missing closing tags**
   ```xml
   <!-- Wrong -->
   <Position x="1.0" y="2.0" z="3.0"
   
   <!-- Correct -->
   <Position x="1.0" y="2.0" z="3.0"/>
   ```

2. **Invalid attribute values**
   ```xml
   <!-- Wrong: missing quotes -->
   <Position x=1.0 y=2.0 z=3.0/>
   
   <!-- Correct -->
   <Position x="1.0" y="2.0" z="3.0"/>
   ```

3. **Unsupported parameters**
   - **Check**: Parameter names match exactly (case-sensitive)
   - **Reference**: See [XML Reference](../xml-configuration/reference/) for valid parameters

### "Required element missing"

**Symptoms:**
```
[ERROR] Required element 'HomePosition' not found
[ERROR] TrialConfig must contain EndConditions
```

**Solution**: Check XML structure includes all required elements:
```xml
<TrialConfig>
  <HomePosition>...</HomePosition>     <!-- Required -->
  <EndConditions>...</EndConditions>   <!-- Required -->
  <SceneSettings>...</SceneSettings>   <!-- Optional -->
</TrialConfig>
```

## Haptic Device Errors

### "Device not found" or "Failed to initialize device"

**Symptoms:**
```
[ERROR] No haptic device detected
[ERROR] Failed to initialize Franka robot
[ERROR] Device initialization timeout
```

**Solutions:**

1. **Check physical connections**
   - Ensure all cables are securely connected
   - Verify power is on
   - Check USB/Ethernet connections

2. **Check device permissions**
   ```bash
   # Add user to dialout group (for USB devices)
   sudo usermod -a -G dialout $USER
   # Log out and log back in
   ```

3. **Device already in use**
   - **Check**: Another application using the device
   - **Solution**: Close other haptic applications
   ```bash
   # Check for processes using the device
   lsof | grep -i franka
   ```

4. **Driver issues**
   - **Solution**: Reinstall device drivers
   - **For Franka**: Check libfranka installation
   - **For Force Dimension**: Verify SDK installation

### "Device safety violation"

**Symptoms:**
```
[ERROR] Safety limits exceeded
[ERROR] Emergency stop activated
[ERROR] Position limit violation
```

**Immediate action**: Device has been stopped for safety

**Solutions:**
1. **Check workspace limits** in XML configuration
2. **Verify home position** is within device range
3. **Reset device** and restart core server
4. **Calibrate device** if position tracking seems off

## Data Collection Errors

### "Failed to create results file"

**Symptoms:**
```
[ERROR] Permission denied writing to /results/trial_data.csv
[ERROR] Directory does not exist: /invalid/path/
```

**Solutions:**

1. **Check directory permissions**
   ```bash
   # Create results directory with correct permissions
   mkdir -p /path/to/results
   chmod 755 /path/to/results
   ```

2. **Update settings.json**
   ```json
   {
     "results_path": "/valid/writable/path/results"
   }
   ```

3. **Disk space**
   ```bash
   df -h  # Check available disk space
   ```

### "Data logging stopped unexpectedly"

**Symptoms:**
- CSV file exists but is incomplete
- Missing data points during trial

**Causes & Solutions:**

1. **High CPU usage**
   - **Solution**: Close unnecessary applications
   - **Check**: `top` or `htop` for CPU usage

2. **Network lag** (if using remote server)
   - **Solution**: Use local server for data collection
   - **Alternative**: Reduce data collection frequency

3. **Storage performance**
   - **Solution**: Use SSD for results directory
   - **Alternative**: Reduce data collection resolution

## Python Client Errors

### "Module not found" errors

**Symptoms:**
```
ModuleNotFoundError: No module named 'franka_trial_pb2'
ImportError: cannot import name 'xml_parser' from 'parser'
```

**Solutions:**

1. **Regenerate protobuf files**
   ```bash
   cd /path/to/haptech
   make proto
   ```

2. **Check Python environment**
   ```bash
   # Activate virtual environment if using one
   source venv/bin/activate
   
   # Install requirements
   pip install -r python_client/requirements.txt
   ```

3. **Path issues**
   - **Solution**: Run Python client from correct directory
   ```bash
   cd /path/to/haptech/python_client
   python main.py
   ```

### "Settings file not found"

**Symptoms:**
```
[ERROR] settings.json not found or invalid
[ERROR] Need to specify 'server_host' in settings.json
```

**Solution**: Create or fix `settings.json`:
```json
{
  "server_host": "localhost",
  "server_port": "50051",
  "config_path": "./config",
  "results_path": "./results"
}
```

## Performance Issues

### "Trial running slowly" or "Laggy haptic feedback"

**Quick checks:**

1. **CPU usage**: Should be < 80% during trials
   ```bash
   top -p $(pgrep core)
   ```

2. **System load**: Check for background tasks
   ```bash
   htop
   ```

3. **Memory usage**: Ensure sufficient free RAM
   ```bash
   free -h
   ```

**Solutions**: See [Performance Issues](performance-issues.md) for detailed optimization

## Getting Help

### Before asking for help:

1. **Check log files** for detailed error messages
2. **Try restarting** both server and client
3. **Verify basic setup** with a simple configuration
4. **Test hardware** with manufacturer's tools

### Information to include when reporting issues:

- **Exact error message** (copy and paste)
- **Steps to reproduce** the problem
- **System information** (OS, hardware)
- **Configuration file** that causes the issue
- **Log output** from both server and client

### Log locations:
- **Server logs**: Console output where core server was started
- **Client logs**: Console output where Python client was run
- **System logs**: `/var/log/syslog` (Linux) for system-level issues