# First Trial

Step-by-step guide to running your first haptic trial.

## Prerequisites

Before starting, ensure:
- [Installation](installation.md) is complete and verified
- Haptic device is connected and configured
- You understand [Basic Concepts](basic-concepts.md)

## Overview

This guide will walk you through:
1. Starting the haptic server
2. Launching the Python client
3. Running a test trial
4. Understanding the results
5. Troubleshooting common issues

**Expected time**: 10-15 minutes

## Step 1: Start the Core Server

### Launch Terminal 1 - Core Server

Open a terminal and navigate to the core directory:

```bash
cd /path/to/haptech/core/bin
./core
```

### Expected Output

You should see something like:

```
[INFO] Haptech Core Server v1.0
[INFO] Initializing haptic device...
[INFO] Device found: Franka Emika Robot
[INFO] Haptic device initialized successfully
[INFO] Starting physics engine...
[INFO] gRPC server listening on 0.0.0.0:50051
[INFO] Server ready for connections
[INFO] Waiting for trial configuration...
```

**If successful**: Leave this terminal running (don't close it)

**If there are errors**: See [Troubleshooting](#troubleshooting) section below

## Step 2: Start the Python Client

### Launch Terminal 2 - Python Client

Open a **new terminal** (keep the server terminal running):

```bash
cd /path/to/haptech
source .venv/bin/activate
python3 python_client/main.py
```

### Client Startup Sequence

You'll see prompts like:

```
Trial configuration file: 
```

**Enter**: `sandbox.xml` (this is a safe test configuration)

```
Trial subject: 
```

**Enter**: Your name or `test_user`

```
Trial ID: 
```

**Enter**: `first_trial_001`

### Connection and Trial Start

The client will:
1. Connect to the haptic server
2. Load the XML configuration
3. Setup the virtual scene
4. Start the trial

You should see:

```
[INFO] Connected to haptic server at localhost:50051
[INFO] Loading configuration: builtin_objects_test.xml
[INFO] Creating objects: sphere, box, cylinder, ball, torus
[INFO] Setting up scene...
[INFO] Trial started successfully
[INFO] Data logging to: results/test_user_2024-01-15/first_trial_001_results.csv
```

## Step 3: Experience the Trial

### What You'll See

A graphics window should open showing:
- Multiple colored objects (sphere, box, cylinder, etc.)
- A cursor representing the haptic device position
- Text overlay with trial information

### What You'll Feel

**If you have a haptic device connected**:
- Move the haptic device around
- You should feel resistance when touching the virtual objects
- Different objects may have different textures and stiffness

**If no haptic device**:
- The trial will still run
- You can observe the visual components
- Mouse cursor may show device position

### Trial Duration

The test configuration runs for 60 seconds, giving you time to:
- Explore different objects
- Get familiar with the haptic sensations
- Understand how the system responds

### Trial End

The trial will end automatically after 60 seconds. You'll see:

```
[INFO] Trial ended: Time condition reached (60.0 seconds)
[INFO] Trial result: Success
[INFO] Data saved to: results/test_user_2024-01-15/first_trial_001_results.csv
[INFO] Metadata saved to: results/test_user_2024-01-15/first_trial_001_metadata.json
```

## Step 4: Review the Results

### Data Files

Navigate to the results directory:

```bash
cd python_client/results
ls -la
```

You should see a folder named `{subject}_{date}/` containing:
- `first_trial_001_results.csv` - Trial data
- `first_trial_001_metadata.json` - Trial configuration and metadata
- `first_trial_001.log` - Detailed system logs

### Examine the CSV Data

Open the CSV file (or view first few lines):

```bash
head results/{subject}_{date}/first_trial_001_results.csv
```

**Expected columns**:
```csv
timestamp,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z,force_x,force_y,force_z,trial_time
0.016667,0.000000,0.000000,0.000000,0.0,0.0,0.0,0.0,0.0,0.0,0.016667
0.033333,0.001234,0.000567,-0.000234,0.074,0.034,-0.014,0.12,0.05,-0.02,0.033333
...
```

**Data explanation**:
- `timestamp`: Time since trial start (seconds)
- `pos_x/y/z`: Haptic device position (meters)
- `vel_x/y/z`: Device velocity (meters/second)
- `force_x/y/z`: Applied forces (Newtons)
- `trial_time`: Alternative timestamp

### Check the Metadata

```bash
cat results/{subject}_{date}/first_trial_001_metadata.json
```

This contains:
- Trial configuration parameters
- Start and end conditions
- Object definitions
- Trial outcome information

## Step 5: Stop the System

### Stop the Client

In the Python client terminal:
- The client should have automatically exited after the trial
- If still running, press `Ctrl+C`

### Stop the Server

In the core server terminal:
- Press `Ctrl+C` to gracefully shut down
- You should see:

```
[INFO] Shutdown signal received
[INFO] Cleaning up haptic device...
[INFO] Server shutdown complete
```

## Understanding What Happened

### The Test Configuration

`sandbox.xml` creates a simple scene with:
- **Multiple objects**: Sphere, box, cylinder, ball, torus
- **Different properties**: Various sizes, colors, and haptic properties
- **Data collection**: Position, velocity, and force data every ~16ms

### System Interaction

1. **Python client** loaded the XML and sent configuration to server
2. **Core server** created the virtual objects and started haptic rendering
3. **During trial**: Server provided haptic feedback while client logged data
4. **At end**: Client saved all collected data and metadata

### What the Data Shows

The CSV file contains a complete record of:
- Where you moved the haptic device
- How fast you were moving
- What forces were applied to your hand
- Precise timing of all measurements

This data can be analyzed to understand participant behavior, performance, learning, etc.

## Next Steps

### Try Different Configurations

Explore other example configurations:

```bash
python3 python_client/main.py
# Try: pong_demo.xml, home_robot.xml, cursor_test.xml
```

### Modify Parameters

Edit configuration files in `python_client/config/` to:
- Change object positions and properties
- Adjust trial duration
- Add different end conditions
- Modify data collection settings

### Create Custom Trials

Once comfortable with the system:
1. Read [XML Configuration](../xml-configuration/README.md) guide
2. Design trials specific to your research
3. Apply [Best Practices](../best-practices/README.md) for effective experiments

## Troubleshooting

### Server Won't Start

**Error**: `[ERROR] Failed to initialize haptic device`

**Solutions**:
1. Check haptic device connection (USB/Ethernet)
2. Verify device drivers are installed
3. Ensure device is not in use by other software
4. Check device-specific setup requirements

**Error**: `[ERROR] Port 50051 already in use`

**Solutions**:
1. Kill any existing server processes: `pkill core`
2. Check for other programs using the port: `lsof -i :50051`
3. Wait a moment and try again

### Client Connection Failed

**Error**: `[ERROR] Failed to connect to server`

**Solutions**:
1. Verify core server is running
2. Check server output for error messages
3. Verify network settings in `python_client/settings.json`
4. Try restarting both client and server

### No Haptic Feedback

**Problem**: Visual trial works but no force feedback

**Solutions**:
1. Check haptic device connection and status
2. Verify device drivers and software
3. Test device with manufacturer's software first
4. Check force limits and safety settings

### Graphics Issues

**Problem**: No graphics window or rendering errors

**Solutions**:
1. **Linux**: Verify OpenGL libraries: `sudo apt install libgl1-mesa-dev`
2. **macOS**: Check security permissions for the application
3. **WSL**: Install X11 server (VcXsrv) and set DISPLAY variable
4. Update graphics drivers

### Configuration File Not Found

**Error**: `[ERROR] Configuration file not found: xyz.xml`

**Solutions**:
1. Check file exists in `python_client/config/`
2. Verify filename spelling (case-sensitive)
3. Use relative path from config directory
4. Try a known-good file like `builtin_objects_test.xml`

### Data Not Saved

**Problem**: Trial runs but no output files created

**Solutions**:
1. Check permissions on `python_client/results/` directory
2. Verify disk space availability
3. Look for error messages in console output
4. Check `python_client/settings.json` for correct results path

### Poor Haptic Performance

**Problem**: Jerky or delayed force feedback

**Solutions**:
1. Close unnecessary applications to free CPU
2. Check system performance during trials
3. Verify haptic device USB/network connection quality
4. Consider simplifying trial configuration

## Getting Help

If you continue to have issues:

1. **Check logs**: Look at console output and log files for error messages
2. **Try simpler configurations**: Start with basic examples before complex trials
3. **Verify hardware**: Test haptic device with manufacturer software
4. **Review documentation**: Check relevant sections in [Troubleshooting](../troubleshooting/README.md)

## Success!

If you've successfully run a trial and collected data, you're ready to:
- Configure custom experiments using [XML Configuration](../xml-configuration/README.md)
- Learn advanced features in [Python Client](../python-client/README.md)
- Apply [Best Practices](../best-practices/README.md) for research

Congratulations on running your first haptic trial!
