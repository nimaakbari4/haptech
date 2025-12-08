# Command Line Interface

Complete guide to using the Python client from the command line.

## Overview

The haptic system's Python client provides both **interactive mode** (default) and **command-line tools** for different use cases:

- **Interactive Mode**: Guided prompts for running trials (recommended for most users)
- **Configuration Validation**: Command-line tool to test XML configurations
- **Programmatic Interface**: Python module for custom experiment scripts

## Basic Usage

### Interactive Mode (Default)

The standard way to run haptic trials:

```bash
cd /path/to/haptech/python_client
python main.py
```

**What happens:**
1. Prompts for trial configuration file
2. Asks for participant information (if not test mode)
3. Generates unique trial ID
4. Runs the trial with real-time feedback
5. Saves results and metadata

**Example session:**
```
$ python main.py
Trial configuration file: example_config.xml
Trial subject: P001
Trial ID: reaching_trial_01

Starting trial...

[INFO] Connected to haptic server
[INFO] Loading configuration...
[INFO] Trial started successfully
[INFO] Participant can begin

... (trial runs) ...

[INFO] Trial completed successfully
Trial Duration: 2m 34.5s

==============================================
             TRIAL SUMMARY
==============================================
│ Subject     │ P001                         │
│ Trial ID    │ reaching_trial_01            │
│ Duration    │ 2m 34.5s                    │
│ Config File │ example_config.xml           │
│ Results CSV │ results/P001_2024-12-07/reaching_trial_01_results.csv │
│ Metadata    │ results/P001_2024-12-07/reaching_trial_01_metadata.json │
==============================================
```

## Configuration File Management

### Selecting Configuration Files

**First time:** Enter complete filename
```
Trial configuration file: pong_demo.xml
```

**Subsequent runs:** Press Enter to reuse last configuration
```
Last configuration file: 'pong_demo.xml'
Press Enter to continue or enter a new file: [Enter]
```

**Change configuration:** Type new filename
```
Last configuration file: 'pong_demo.xml'
Press Enter to continue or enter a new file: reaching_task.xml
```

### Configuration File Locations

**Default search path:** `./config/` directory

**Available configurations:**
```bash
ls config/
# Shows all .xml files in config directory
```

**Example configurations included:**
- `example_config.xml` - Basic reaching task
- `pong_demo.xml` - Interactive pong game
- `cursor_test.xml` - Simple cursor movement test
- `custom_objects_test.xml` - Test custom object types

## Participant and Trial Management

### Subject Information

**First time:** Enter subject ID
```
Trial subject: P001
```

**Subsequent runs:** Press Enter to continue with same subject
```
Last trial subject: 'P001'
Press Enter to continue or enter a new subject: [Enter]
```

**New subject:** Type new subject ID
```
Last trial subject: 'P001'
Press Enter to continue or enter a new subject: P002
```

### Trial IDs

**Always prompted:** Enter unique trial identifier
```
Trial ID: reaching_trial_01
```

**Recommendations:**
- Use descriptive names: `reaching_trial_01`, `pong_practice`, `baseline_test`
- Include condition information: `high_force_trial`, `no_feedback_condition`
- Use consistent naming: `condition_A_trial_01`, `condition_A_trial_02`

### Test Mode

**Automatic test mode:** When XML contains `<Test>true</Test>`
- Subject automatically set to "TEST"
- Trial ID auto-generated with timestamp
- No data files saved (testing only)
- Simplified output

```xml
<TrialConfig>
  <Test>true</Test>
  <!-- rest of configuration -->
</TrialConfig>
```

## Configuration Validation Tool

### XML Parser Command Line

**Test configuration files without running trials:**

```bash
cd /path/to/haptech/python_client
python parser.py config/example_config.xml
```

**What it does:**
- Parses and validates XML structure
- Displays all configuration settings
- Shows any parsing errors
- Tests parameter validation

**Example output:**
```
Start Conditions:
 - home_position (success=false, priority=0)
   tolerance: 0.01
   x: 0.0
   y: 0.0
   z: 0.0

End Conditions:
 - position (success=true, priority=1)
   tolerance: 0.01
   x: 0.1
   y: 0.2
   z: 0.3
 - time (success=false, priority=0)
   value: 20.0

HUD Messages:
 Start: Move the cursor to the target as quickly and accurately as possible.
 Success: Well done!
 Failure: Try again.

Scene Settings:
 Background color: RGB(0.20, 0.20, 0.20) = RGB(51, 51, 51)
 Camera pos: Position(x=1.0, y=1.0, z=2.0)
 Camera look-at: Position(x=0.0, y=0.0, z=0.0)
 Camera up: Position(x=0.0, y=1.0, z=0.0)

Objects:
 • [sphere] target
    Position: Position(x=0.1, y=0.2, z=0.3)
    Orientation: Position(x=0.0, y=0.0, z=0.0)
    Color: RGB(0.80, 0.20, 0.20) = RGB(204, 51, 51)
    Parameters:
      radius: 0.05

Data Collection Configuration:
 Device:
  Position: True
  Velocity: True
  Angular Velocity: True
  Orientation: True
  Proxy Position: True
  Force: True
 Objects:
  Position: True
  Orientation: True
  Contact: True
  Contact Point: True
  Contact Normal: True
  Penetration Depth: True
  Force: True
  Spring Force: True
  Damping Force: True
```

**Common validation errors:**
```bash
# Missing required elements
$ python parser.py config/broken_config.xml
xml.etree.ElementTree.ParseError: no element found: line 1, column 0

# Invalid parameter values
ValueError: Invalid radius for object 'sphere1' (must be > 0)

# Missing files
FileNotFoundError: [Errno 2] No such file or directory: 'config/missing.xml'
```

## Settings Configuration

### settings.json File

**Location:** `python_client/settings.json`

**Auto-generated defaults:**
```json
{
    "config_path": "config",
    "results_path": "results", 
    "server_host": "localhost",
    "server_port": 50051
}
```

**Configuration options:**

| Setting | Description | Default | Example |
|---------|-------------|---------|---------|
| `config_path` | Directory containing XML configurations | `"config"` | `"../shared/configs"` |
| `results_path` | Directory for saving trial results | `"results"` | `"/data/haptic_results"` |
| `server_host` | Haptic core server address | `"localhost"` | `"192.168.1.100"` |
| `server_port` | Server port number | `50051` | `50051` |
| `trial_subject` | Last used subject ID | *auto-saved* | `"P001"` |
| `last_configuration_file` | Last used config file | *auto-saved* | `"pong_demo.xml"` |

### Modifying Settings

**Direct editing:**
```bash
nano settings.json
```

**Settings are automatically updated when:**
- You change trial subject during interactive session
- You select a different configuration file
- Values are remembered for next session

**Example custom settings:**
```json
{
    "config_path": "/home/researcher/haptic_configs",
    "results_path": "/data/experiment_results",
    "server_host": "haptic-server.lab.edu",
    "server_port": 50051,
    "trial_subject": "P023",
    "last_configuration_file": "custom_reaching_task.xml"
}
```

## Output and Data Files

### File Organization

**Automatic directory structure:**
```
results/
├── SubjectID_YYYY-MM-DD/
│   ├── trialID_results.csv
│   ├── trialID_metadata.json
│   └── trialID.log
└── SubjectID_YYYY-MM-DD/
    ├── trialID_results.csv
    └── ...
```

**Example:**
```
results/
├── P001_2024-12-07/
│   ├── reaching_trial_01_results.csv
│   ├── reaching_trial_01_metadata.json
│   ├── reaching_trial_02_results.csv
│   └── reaching_trial_02_metadata.json
└── P002_2024-12-07/
    ├── baseline_test_results.csv
    └── baseline_test_metadata.json
```

### CSV Data Files

**Automatic CSV headers based on data collection configuration:**
```csv
t,device_pos_x,device_pos_y,device_pos_z,proxy_pos_x,proxy_pos_y,proxy_pos_z,force_x,force_y,force_z,device_vel_x,device_vel_y,device_vel_z,device_ang_vel_x,device_ang_vel_y,device_ang_vel_z,device_quat_x,device_quat_y,device_quat_z,device_quat_w
0.001,-0.123,0.045,0.234,-0.122,0.046,0.235,0.45,-0.12,2.34,...
```

**Customizable data collection via XML:**
```xml
<DataCollection>
  <Device>
    <Position enabled="true"/>
    <Velocity enabled="false"/>
    <Force enabled="true"/>
    <!-- Only position and force will be recorded -->
  </Device>
</DataCollection>
```

### Metadata Files

**JSON format with complete trial information:**
```json
{
    "trial_subject": "P001",
    "trial_id": "reaching_trial_01",
    "configuration_file": "example_config.xml",
    "started_at": "2024-12-07 14:30:15.123456",
    "ended_at": "2024-12-07 14:32:49.654321",
    "trial_duration": 154.53,
    "configuration": {
        "start_conditions": { ... },
        "end_conditions": { ... },
        "scene_settings": { ... },
        "objects": [ ... ]
    }
}
```

### Log Files

**Per-trial log files with detailed information:**
```
2024-12-07 14:30:15,123 INFO: Connected to haptic server at localhost:50051
2024-12-07 14:30:15,456 INFO: Loading configuration 'example_config.xml'
2024-12-07 14:30:16,789 INFO: Trial 'reaching_trial_01' started successfully
2024-12-07 14:32:49,012 INFO: End condition 'position' triggered (success=true)
2024-12-07 14:32:49,345 INFO: Trial 'reaching_trial_01' completed successfully
```

## Error Handling and Recovery

### Connection Errors

**Server not running:**
```
[ERROR] Failed to connect to haptic server at localhost:50051
[ERROR] Connection refused
```
**Solution:** Start the core server first

**Wrong server address:**
```
[ERROR] RPC failed with status UNAVAILABLE
```
**Solution:** Check `settings.json` server configuration

### Configuration Errors

**File not found:**
```
Trial configuration file: nonexistent.xml
FileNotFoundError: [Errno 2] No such file or directory: 'config/nonexistent.xml'
```
**Solution:** Check filename and path

**Invalid XML:**
```
[ERROR] XML parsing failed: Expected '<', found 'T'
```
**Solution:** Validate XML syntax with parser tool

### Data Collection Errors

**Permission denied:**
```
[ERROR] Permission denied writing to /results/trial_data.csv
```
**Solution:** Check directory permissions or change `results_path`

**Disk space:**
```
[ERROR] No space left on device
```
**Solution:** Free disk space or change `results_path`

## Advanced Usage

### Environment Variables

**Override settings via environment:**
```bash
export HAPTIC_SERVER_HOST=192.168.1.100
export HAPTIC_RESULTS_PATH=/data/experiments
python main.py
```

**Supported variables:**
- `HAPTIC_SERVER_HOST` - Override `server_host`
- `HAPTIC_SERVER_PORT` - Override `server_port`  
- `HAPTIC_CONFIG_PATH` - Override `config_path`
- `HAPTIC_RESULTS_PATH` - Override `results_path`

### Batch Processing

**Run multiple trials with script:**
```bash
#!/bin/bash
# run_experiment.sh

for trial in {1..10}; do
    echo "Running trial $trial..."
    python3 -c "
import main
# Set config programmatically
main.run_trial_batch('pong_demo.xml', 'P001', 'trial_$trial')
"
done
```

### Integration with Other Tools

**CSV processing with pandas:**
```python
import pandas as pd
data = pd.read_csv('results/P001_2024-12-07/trial_01_results.csv')
print(data.describe())
```

**Automated analysis:**
```bash
# Process all trials for a subject
python analyze_results.py results/P001_2024-12-07/
```

## Tips and Best Practices

### Efficient Workflow

1. **Test configurations first:**
   ```bash
   python parser.py config/new_experiment.xml
   ```

2. **Use descriptive trial IDs:**
   - Include condition: `high_force_01`
   - Include block: `practice_trial_03`
   - Include timestamp if needed: `baseline_20241207_1430`

3. **Organize results:**
   - One subject per day per directory
   - Consistent naming conventions
   - Regular backups of results

### Configuration Management

1. **Version control XML files:**
   ```bash
   git add config/
   git commit -m "Add new reaching task configuration"
   ```

2. **Test mode for development:**
   - Add `<Test>true</Test>` to XML
   - No data files created
   - Quick iteration

3. **Backup configurations:**
   ```bash
   cp config/working_experiment.xml config/backup/
   ```

### Data Management

1. **Regular data exports:**
   ```bash
   # Archive completed sessions
   tar -czf P001_session1.tar.gz results/P001_2024-12-07/
   ```

2. **Verify data integrity:**
   ```bash
   # Check CSV files are complete
   wc -l results/*/trial_*_results.csv
   ```

3. **Monitor disk space:**
   ```bash
   df -h  # Check available space
   du -sh results/  # Check results directory size
   ```

This command-line interface provides both interactive ease-of-use and programmatic flexibility for haptic research experiments.