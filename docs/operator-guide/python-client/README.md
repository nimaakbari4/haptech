# Python Client

Complete guide to using the Python client for haptic experiments.

## Overview

The Python client is your primary interface for running haptic experiments. It handles:
- **Trial management**: Starting, monitoring, and ending experimental trials
- **Data collection**: Recording participant movements, forces, and trial outcomes
- **Configuration**: Loading XML files and sending parameters to the haptic server
- **User interface**: Providing feedback to operators and participants

## Client Architecture

```
┌─────────────────────────────────────────────┐
│                Python Client                │
├────────────────┬───────────────┬───────────────┤
│  main.py        │ settings.json  │  config/ dir   │
│                │                │               │
│ • Trial control │ • Server host   │ • XML files     │
│ • Data logging  │ • Paths config  │ • Trial setups  │
│ • User prompts  │ • Last used     │ • Examples      │
└────────────────┴───────────────┴───────────────┘
                        │
                        │ gRPC Communication
                        ▼
              ┌─────────────────┐
              │   Core Server   │
              │      (C++)      │
              └─────────────────┘
```

## Core Files

### main.py
**Purpose**: Primary client application

**Functions**:
- Prompts for trial configuration and participant information
- Establishes connection with haptic server
- Manages trial lifecycle (start, monitor, end)
- Handles data logging and file management
- Provides user feedback and error handling

### settings.json
**Purpose**: Client configuration

**Contains**:
```json
{
    "config_path": "config",              // Where XML files are stored
    "results_path": "results",           // Where data is saved
    "server_host": "localhost",          // Server IP address
    "server_port": 50051,                // Server port
    "trial_subject": "last_subject",     // Default subject name
    "last_configuration_file": "last.xml" // Last used config
}
```

### config/ Directory
**Purpose**: Storage for XML trial configurations

**Contents**:
- Example configurations (`builtin_objects_test.xml`, `pong_demo.xml`, etc.)
- Custom experimental setups
- Template files for new experiments

### results/ Directory
**Purpose**: Output location for collected data

**Organization**:
```
results/
├── Subject1_2024-01-15/
│   ├── trial001_results.csv
│   ├── trial001_metadata.json
│   ├── trial001.log
│   ├── trial002_results.csv
│   └── ...
├── Subject2_2024-01-15/
└── Subject3_2024-01-16/
```

## Basic Usage

### Standard Trial Session

1. **Start the client**:
   ```bash
   source .venv/bin/activate
   python3 python_client/main.py
   ```

2. **Follow prompts**:
   ```
   Trial configuration file: builtin_objects_test.xml
   Trial subject: John_Doe
   Trial ID: reaching_task_001
   ```

3. **Monitor trial**:
   - Watch console output for trial progress
   - Observe graphics window (if enabled)
   - Note any error messages or warnings

4. **Trial completion**:
   - Client automatically saves data when trial ends
   - Files saved to `results/{subject}_{date}/` directory
   - Client returns to prompt or exits

### Quick Testing

For rapid testing or demos:

```bash
# Use test mode (skips subject/ID prompts)
python3 python_client/main.py --test

# Or edit config to set test: true
```

## Configuration Management

### Using settings.json

**Edit default settings**:
```bash
nano python_client/settings.json
```

**Common modifications**:
```json
{
    "server_host": "192.168.1.100",  // Remote server
    "trial_subject": "Pilot_Study",  // Default subject
    "config_path": "my_configs",     // Custom config directory
    "results_path": "/data/haptic"   // Custom results location
}
```

### Default Values

The client remembers your last choices:
- **Configuration file**: Prompts with last used file
- **Subject name**: Suggests previous subject
- **Paths**: Uses settings.json values

### Multiple Configurations

Manage different experimental setups:

```bash
# Organize configs by experiment
config/
├── experiment1/
│   ├── condition_A.xml
│   └── condition_B.xml
├── experiment2/
└── templates/

# Update config_path in settings.json as needed
```

## Data Collection Features

### Automatic File Organization

**Directory structure**:
```
results/
└── {Subject}_{YYYY-MM-DD}/
    ├── {TrialID}_results.csv      // Raw data
    ├── {TrialID}_metadata.json   // Trial info
    └── {TrialID}.log             // Detailed logs
```

**Benefits**:
- **Organized by date**: Easy to find recent sessions
- **Subject separation**: Each participant gets their own folder
- **No overwrites**: Unique filenames prevent data loss
- **Complete records**: Data + metadata + logs for each trial

### Data File Contents

**CSV Results** (`{TrialID}_results.csv`):
```csv
timestamp,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z,force_x,force_y,force_z,trial_time
0.016667,0.000,0.000,0.000,0.0,0.0,0.0,0.0,0.0,0.0,0.016667
0.033333,0.001,0.000,0.000,0.1,0.0,0.0,0.5,0.0,0.0,0.033333
...
```

**Metadata JSON** (`{TrialID}_metadata.json`):
```json
{
  "trial_info": {
    "subject": "John_Doe",
    "trial_id": "reaching_task_001",
    "start_time": "2024-01-15T14:30:00Z",
    "end_time": "2024-01-15T14:30:45Z",
    "duration_seconds": 45.2,
    "result": "success"
  },
  "configuration": {
    "file": "reaching_task.xml",
    "objects": [...],
    "conditions": [...]
  },
  "trial_outcome": {
    "end_condition": "position",
    "message": "Target reached at (0.098, 0.002, 0.001)"
  }
}
```

**Log File** (`{TrialID}.log`):
```
2024-01-15 14:30:00 INFO: Trial started
2024-01-15 14:30:05 INFO: Participant approaching target
2024-01-15 14:30:12 WARNING: Velocity spike detected: 0.45 m/s
2024-01-15 14:30:45 INFO: Trial ended successfully
```

### Real-time Monitoring

During trials, the client displays:

```
[INFO] Trial: reaching_task_001 | Subject: John_Doe
[INFO] Duration: 00:15 | Target: (0.10, 0.00, 0.00)
[INFO] Current position: (0.045, 0.012, 0.003)
[INFO] Distance to target: 0.057m
```

## Error Handling

### Connection Issues

**Server not available**:
```
[ERROR] Failed to connect to haptic server at localhost:50051
[INFO] Please ensure the core server is running
[INFO] Try: cd core/bin && ./core
```

**Network problems**:
```
[ERROR] Connection lost during trial
[INFO] Attempting to reconnect...
[INFO] Trial data saved up to disconnection point
```

### Configuration Errors

**File not found**:
```
[ERROR] Configuration file not found: missing_file.xml
[INFO] Available configurations:
[INFO]   - builtin_objects_test.xml
[INFO]   - pong_demo.xml
[INFO]   - reaching_task.xml
```

**Invalid XML**:
```
[ERROR] Failed to parse configuration: Invalid XML syntax
[INFO] Check line 15: Missing closing tag </Object>
```

### Data Saving Issues

**Permission problems**:
```
[ERROR] Cannot write to results directory
[INFO] Check permissions on: /path/to/results
[INFO] Try: chmod 755 /path/to/results
```

**Disk space**:
```
[ERROR] Failed to save data: No space left on device
[INFO] Free up disk space or change results_path in settings.json
```

## Advanced Features

### Test Mode

Skip user prompts for automated testing:

```bash
# Enable test mode in configuration XML:
<TrialConfig test="true">
  <!-- Trial automatically uses TEST subject and timestamp ID -->
</TrialConfig>

# Or use command line flag (if implemented):
python3 main.py --test
```

### Custom Scripts

Extend the client for specific needs:

```python
# Custom experiment script
import main
import settings

# Override default settings
settings.change_setting("trial_subject", "Experiment1_Pilot")
settings.change_setting("config_path", "experiment1_configs")

# Run multiple trials automatically
configurations = ["condition_A.xml", "condition_B.xml", "condition_C.xml"]
for i, config in enumerate(configurations):
    settings.change_setting("last_configuration_file", config)
    trial_id = f"pilot_trial_{i+1:03d}"
    # Run trial with specific parameters
    main.run_trial_with_config(config, "Pilot_Subject", trial_id)
```

### Remote Operation

Run client on different machine from server:

```json
// settings.json
{
    "server_host": "192.168.1.100",  // IP of haptic server
    "server_port": 50051,
    "results_path": "./remote_results"
}
```

**Network requirements**:
- Port 50051 accessible between machines
- Low latency network connection
- Synchronized clocks for timestamps

### Batch Processing

Process multiple participants efficiently:

```bash
# Create script for batch trials
echo '#!/bin/bash
for subject in Subject1 Subject2 Subject3; do
  for trial in 001 002 003; do
    echo "Running trial ${trial} for ${subject}"
    python3 main.py --subject $subject --trial ${subject}_trial_${trial} --config reaching_task.xml
  done
done' > batch_trials.sh

chmod +x batch_trials.sh
./batch_trials.sh
```

## Table of Contents

For detailed information on specific aspects:

- [Command Line Interface](command-line-interface.md) - CLI options and automation
- [Data Collection](data-collection.md) - Data formats and analysis
- [Experiment Scripts](experiment-scripts.md) - Custom automation and batch processing

## Best Practices

1. **Test configurations first**: Always run test trials before data collection
2. **Monitor console output**: Watch for warnings or errors during trials
3. **Backup data regularly**: Copy results directory to secure storage
4. **Use descriptive names**: Clear subject and trial IDs aid later analysis
5. **Document changes**: Keep notes when modifying configurations
6. **Check disk space**: Ensure adequate storage for data collection sessions

The Python client provides a robust, user-friendly interface for conducting haptic experiments. With these tools, you can efficiently collect high-quality data for your research.