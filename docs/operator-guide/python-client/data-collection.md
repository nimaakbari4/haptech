# Data Collection

Comprehensive guide to data logging, file formats, and analysis workflows.

## Overview

The Python client automatically collects and saves multiple types of data during haptic trials:
- **Position data**: Where the haptic device is in 3D space
- **Velocity data**: How fast the participant is moving
- **Force data**: What forces are applied through the haptic device
- **Trial metadata**: Configuration, timing, and outcome information
- **Event logs**: Detailed system events and error messages

## Data Collection Process

### Automatic Collection

**During trials**:
1. Server streams haptic data to client in real-time
2. Client buffers data in memory
3. Timestamps are synchronized between client and server
4. Data collected at graphics frame rate (typically 60-120 Hz)

**At trial end**:
1. All buffered data written to CSV file
2. Trial metadata saved to JSON file
3. System logs saved to text file
4. Files organized in subject/date directory structure

### Data Collection Rate

**Standard rate**: 60-120 Hz (16.7-8.3 milliseconds between samples)
**Haptic rate**: 1000 Hz (internal haptic processing, not logged)
**Configurable**: Rate can be adjusted based on experimental needs

**Rate considerations**:
- Higher rates = more detailed data but larger files
- Lower rates = smaller files but less temporal resolution
- 60 Hz adequate for most human movement analysis
- 120 Hz better for rapid movements or fine motor tasks

## File Organization

### Directory Structure

```
results/
├── Subject1_2024-01-15/           # Subject and date
│   ├── trial001_results.csv      # Primary data file
│   ├── trial001_metadata.json   # Trial configuration and outcome
│   ├── trial001.log             # Detailed system logs
│   ├── trial002_results.csv
│   ├── trial002_metadata.json
│   ├── trial002.log
│   └── ...
├── Subject2_2024-01-15/
├── Subject3_2024-01-16/
└── TEST_2024-01-16/           # Test mode trials
```

**Benefits**:
- **Date organization**: Easy to find sessions by date
- **Subject isolation**: Each participant's data kept separate
- **No overwrites**: Unique filenames prevent data loss
- **Complete records**: Data + metadata + logs for each trial

### File Naming Convention

**Pattern**: `{TrialID}_{type}.{extension}`

**Examples**:
- `reaching_task_001_results.csv`
- `baseline_003_metadata.json`
- `practice_round_5.log`

**Best practices**:
- Use descriptive trial IDs
- Include condition/session information
- Zero-pad numbers for proper sorting (001, 002, 003)
- Avoid spaces and special characters

## Data File Formats

### CSV Results File

**Purpose**: Primary experimental data for analysis

**Format**: Comma-separated values with header row

**Standard columns**:
```csv
timestamp,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z,force_x,force_y,force_z,trial_time
```

**Sample data**:
```csv
timestamp,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z,force_x,force_y,force_z,trial_time
0.016667,0.000000,0.000000,0.000000,0.000,0.000,0.000,0.000,0.000,0.000,0.016667
0.033333,0.001234,0.000567,-0.000234,0.074,0.034,-0.014,0.123,0.056,-0.023,0.033333
0.050000,0.002456,0.001123,-0.000445,0.073,0.033,-0.013,0.145,0.067,-0.034,0.050000
```

**Column descriptions**:
- `timestamp`: Time since data collection start (seconds)
- `pos_x/y/z`: Haptic device position in meters (X=forward, Y=right, Z=up)
- `vel_x/y/z`: Device velocity in meters/second
- `force_x/y/z`: Applied forces in Newtons
- `trial_time`: Alternative timestamp (usually same as timestamp)

**Units**:
- **Position**: Meters (m)
- **Velocity**: Meters per second (m/s)
- **Force**: Newtons (N)
- **Time**: Seconds (s)

### Metadata JSON File

**Purpose**: Trial configuration, timing, and outcome information

**Structure**:
```json
{
  "trial_info": {
    "subject": "John_Doe",
    "trial_id": "reaching_task_001", 
    "configuration_file": "reaching_task.xml",
    "start_time": "2024-01-15T14:30:00.000Z",
    "end_time": "2024-01-15T14:30:45.234Z",
    "duration_seconds": 45.234,
    "result": "success"
  },
  "configuration": {
    "objects": [
      {
        "id": "target_sphere",
        "type": "sphere",
        "position": {"x": 0.1, "y": 0.0, "z": 0.0},
        "parameters": {"radius": "0.03"}
      }
    ],
    "start_conditions": [
      {
        "name": "time",
        "success": true,
        "priority": 1,
        "parameters": {"duration_seconds": "0.0"}
      }
    ],
    "end_conditions": [
      {
        "name": "position",
        "success": true, 
        "priority": 1,
        "parameters": {
          "target_x": "0.1",
          "target_y": "0.0", 
          "target_z": "0.0",
          "tolerance": "0.02"
        }
      }
    ]
  },
  "trial_outcome": {
    "end_condition": "position",
    "end_condition_priority": 1,
    "success": true,
    "message": "Target reached at position (0.098, 0.002, 0.001)",
    "final_position": {"x": 0.098, "y": 0.002, "z": 0.001},
    "target_distance": 0.0034,
    "trial_duration": 45.234
  },
  "data_collection": {
    "sampling_rate_hz": 60,
    "total_samples": 2714,
    "data_file": "reaching_task_001_results.csv",
    "log_file": "reaching_task_001.log"
  }
}
```

**Uses**:
- **Analysis**: Understanding trial parameters
- **Replication**: Exactly reproduce experimental conditions
- **Quality control**: Verify trials completed as intended
- **Data validation**: Check for inconsistencies or errors

### Log File

**Purpose**: Detailed system events and debugging information

**Format**: Timestamped text entries

**Sample content**:
```
2024-01-15 14:30:00.123 INFO: Trial started: reaching_task_001
2024-01-15 14:30:00.145 INFO: Connected to haptic server at localhost:50051
2024-01-15 14:30:00.167 INFO: Loaded configuration: reaching_task.xml
2024-01-15 14:30:00.189 INFO: Created object: target_sphere at (0.100, 0.000, 0.000)
2024-01-15 14:30:00.234 INFO: Trial conditions initialized
2024-01-15 14:30:00.256 INFO: Data collection started
2024-01-15 14:30:05.123 INFO: Start condition met: time (0.0 seconds)
2024-01-15 14:30:05.145 INFO: Trial phase: active
2024-01-15 14:30:12.678 WARNING: Velocity spike detected: 0.456 m/s
2024-01-15 14:30:25.432 INFO: Participant approaching target (distance: 0.034m)
2024-01-15 14:30:45.234 INFO: End condition met: position (target reached)
2024-01-15 14:30:45.245 INFO: Trial result: success
2024-01-15 14:30:45.267 INFO: Data saved to: reaching_task_001_results.csv
2024-01-15 14:30:45.289 INFO: Trial completed successfully
```

**Information types**:
- **INFO**: Normal trial events
- **WARNING**: Non-critical issues or unusual events
- **ERROR**: Serious problems requiring attention
- **DEBUG**: Detailed system information (if enabled)

## Data Analysis Workflows

### Basic Analysis in Python

**Load CSV data**:
```python
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Load trial data
data = pd.read_csv('results/Subject1_2024-01-15/trial001_results.csv')

# Basic information
print(f"Trial duration: {data['timestamp'].max():.2f} seconds")
print(f"Data points: {len(data)}")
print(f"Sampling rate: {1/data['timestamp'].diff().mean():.1f} Hz")
```

**Position analysis**:
```python
# Calculate distance from origin
data['distance'] = np.sqrt(data['pos_x']**2 + data['pos_y']**2 + data['pos_z']**2)

# Calculate movement speed
data['speed'] = np.sqrt(data['vel_x']**2 + data['vel_y']**2 + data['vel_z']**2)

# Plot trajectory
plt.figure(figsize=(12, 4))

plt.subplot(131)
plt.plot(data['timestamp'], data['pos_x'], label='X')
plt.plot(data['timestamp'], data['pos_y'], label='Y')
plt.plot(data['timestamp'], data['pos_z'], label='Z')
plt.xlabel('Time (s)')
plt.ylabel('Position (m)')
plt.legend()
plt.title('Position vs Time')

plt.subplot(132)
plt.plot(data['timestamp'], data['speed'])
plt.xlabel('Time (s)')
plt.ylabel('Speed (m/s)')
plt.title('Movement Speed')

plt.subplot(133)
ax = plt.gca(projection='3d')
ax.plot(data['pos_x'], data['pos_y'], data['pos_z'])
ax.set_xlabel('X Position (m)')
ax.set_ylabel('Y Position (m)')
ax.set_zlabel('Z Position (m)')
plt.title('3D Trajectory')

plt.tight_layout()
plt.show()
```

**Force analysis**:
```python
# Calculate force magnitude
data['force_magnitude'] = np.sqrt(data['force_x']**2 + data['force_y']**2 + data['force_z']**2)

# Force statistics
print(f"Mean force: {data['force_magnitude'].mean():.3f} N")
print(f"Max force: {data['force_magnitude'].max():.3f} N")
print(f"Force > 1N: {(data['force_magnitude'] > 1.0).sum()} samples")

# Plot forces
plt.figure(figsize=(10, 6))
plt.subplot(211)
plt.plot(data['timestamp'], data['force_x'], label='Force X')
plt.plot(data['timestamp'], data['force_y'], label='Force Y')
plt.plot(data['timestamp'], data['force_z'], label='Force Z')
plt.ylabel('Force (N)')
plt.legend()
plt.title('Force Components')

plt.subplot(212)
plt.plot(data['timestamp'], data['force_magnitude'], color='red')
plt.xlabel('Time (s)')
plt.ylabel('Force Magnitude (N)')
plt.title('Total Force Magnitude')
plt.tight_layout()
plt.show()
```

### Batch Analysis

**Process multiple trials**:
```python
import os
import glob
import json

def analyze_session(subject_dir):
    """Analyze all trials for one subject session"""
    results = []
    
    # Find all CSV files
    csv_files = glob.glob(os.path.join(subject_dir, '*_results.csv'))
    
    for csv_file in csv_files:
        # Load data
        data = pd.read_csv(csv_file)
        
        # Load metadata
        metadata_file = csv_file.replace('_results.csv', '_metadata.json')
        with open(metadata_file, 'r') as f:
            metadata = json.load(f)
        
        # Calculate metrics
        trial_result = {
            'trial_id': metadata['trial_info']['trial_id'],
            'duration': metadata['trial_info']['duration_seconds'],
            'success': metadata['trial_outcome']['success'],
            'max_speed': np.sqrt(data['vel_x']**2 + data['vel_y']**2 + data['vel_z']**2).max(),
            'mean_force': np.sqrt(data['force_x']**2 + data['force_y']**2 + data['force_z']**2).mean(),
            'path_length': calculate_path_length(data),
            'target_distance': metadata['trial_outcome'].get('target_distance', np.nan)
        }
        
        results.append(trial_result)
    
    return pd.DataFrame(results)

def calculate_path_length(data):
    """Calculate total path length traveled"""
    dx = data['pos_x'].diff()
    dy = data['pos_y'].diff()
    dz = data['pos_z'].diff()
    distances = np.sqrt(dx**2 + dy**2 + dz**2)
    return distances.sum()

# Analyze all sessions
results_dir = 'python_client/results'
session_dirs = glob.glob(os.path.join(results_dir, '*_2024-*'))

all_results = []
for session_dir in session_dirs:
    subject = os.path.basename(session_dir).split('_')[0]
    session_results = analyze_session(session_dir)
    session_results['subject'] = subject
    all_results.append(session_results)

# Combine all results
combined_results = pd.concat(all_results, ignore_index=True)

# Summary statistics
print("\nOverall Results Summary:")
print(f"Total trials: {len(combined_results)}")
print(f"Success rate: {combined_results['success'].mean():.2%}")
print(f"Mean trial duration: {combined_results['duration'].mean():.2f} seconds")
print(f"Mean movement speed: {combined_results['max_speed'].mean():.3f} m/s")
```

### Export for Statistical Analysis

**Save processed data**:
```python
# Export to CSV for R, SPSS, etc.
combined_results.to_csv('analysis_summary.csv', index=False)

# Export individual trial metrics
for subject in combined_results['subject'].unique():
    subject_data = combined_results[combined_results['subject'] == subject]
    subject_data.to_csv(f'subject_{subject}_summary.csv', index=False)
```

**R analysis example**:
```r
# Load data in R
library(tidyverse)

data <- read.csv("analysis_summary.csv")

# Basic statistics
summary(data)

# Success rate by subject
data %>% 
  group_by(subject) %>% 
  summarise(
    trials = n(),
    success_rate = mean(success),
    mean_duration = mean(duration),
    mean_speed = mean(max_speed)
  )

# Statistical tests
t.test(duration ~ success, data = data)
anova(lm(max_speed ~ subject, data = data))
```

## Data Quality Control

### Validation Checks

**Automated validation**:
```python
def validate_trial_data(csv_file, metadata_file):
    """Check trial data for common issues"""
    issues = []
    
    # Load data
    data = pd.read_csv(csv_file)
    with open(metadata_file, 'r') as f:
        metadata = json.load(f)
    
    # Check data completeness
    if len(data) < 10:
        issues.append("Very short trial (< 10 samples)")
    
    # Check for missing values
    if data.isnull().any().any():
        issues.append("Missing values detected")
    
    # Check position bounds
    pos_range = 0.5  # Typical workspace
    if (abs(data[['pos_x', 'pos_y', 'pos_z']]) > pos_range).any().any():
        issues.append("Position outside expected workspace")
    
    # Check velocity bounds
    speeds = np.sqrt(data['vel_x']**2 + data['vel_y']**2 + data['vel_z']**2)
    if speeds.max() > 2.0:  # 2 m/s threshold
        issues.append(f"Extreme velocity detected: {speeds.max():.2f} m/s")
    
    # Check force bounds
    forces = np.sqrt(data['force_x']**2 + data['force_y']**2 + data['force_z']**2)
    if forces.max() > 10.0:  # 10 N threshold
        issues.append(f"High force detected: {forces.max():.2f} N")
    
    # Check trial duration vs. metadata
    data_duration = data['timestamp'].max()
    metadata_duration = metadata['trial_info']['duration_seconds']
    if abs(data_duration - metadata_duration) > 0.1:
        issues.append("Duration mismatch between data and metadata")
    
    return issues

# Validate all trials
for csv_file in glob.glob('results/*/*.csv'):
    metadata_file = csv_file.replace('_results.csv', '_metadata.json')
    if os.path.exists(metadata_file):
        issues = validate_trial_data(csv_file, metadata_file)
        if issues:
            print(f"\nIssues in {os.path.basename(csv_file)}:")
            for issue in issues:
                print(f"  - {issue}")
```

### Data Backup and Archival

**Backup strategy**:
```bash
#!/bin/bash
# backup_data.sh

SOURCE_DIR="python_client/results"
BACKUP_DIR="/backup/haptic_data"
DATE=$(date +%Y%m%d)

# Create backup with date stamp
tar -czf "${BACKUP_DIR}/haptic_data_${DATE}.tar.gz" "$SOURCE_DIR"

# Keep only last 30 days of backups
find "$BACKUP_DIR" -name "haptic_data_*.tar.gz" -mtime +30 -delete

echo "Backup completed: haptic_data_${DATE}.tar.gz"
```

**Long-term storage**:
- Compress older data files
- Store metadata separately for quick access
- Maintain file index/catalog
- Regular integrity checks

## Advanced Data Collection

### Custom Data Streams

For specialized experiments, you can extend data collection:

```python
# Example: Add custom calculated fields
def enhance_data_collection(data_stream):
    """Add derived measures to data stream"""
    
    # Calculate distance to target
    target_pos = np.array([0.1, 0.0, 0.0])  # Target position
    current_pos = data_stream[['pos_x', 'pos_y', 'pos_z']].values
    data_stream['target_distance'] = np.linalg.norm(current_pos - target_pos, axis=1)
    
    # Calculate smoothed velocity
    window_size = 5
    data_stream['velocity_smooth'] = data_stream['speed'].rolling(window_size, center=True).mean()
    
    # Calculate movement direction
    data_stream['direction_x'] = data_stream['vel_x'] / data_stream['speed']
    data_stream['direction_y'] = data_stream['vel_y'] / data_stream['speed']
    data_stream['direction_z'] = data_stream['vel_z'] / data_stream['speed']
    
    return data_stream
```

### Real-time Data Processing

```python
def real_time_feedback(current_data):
    """Provide real-time feedback during trials"""
    
    # Calculate current performance metrics
    speed = np.sqrt(current_data['vel_x']**2 + current_data['vel_y']**2 + current_data['vel_z']**2)
    target_distance = np.sqrt(current_data['pos_x']**2 + current_data['pos_y']**2 + current_data['pos_z']**2)
    
    # Provide feedback
    if speed > 0.5:
        print("WARNING: Moving too fast")
    
    if target_distance < 0.05:
        print("APPROACHING TARGET")
    
    return speed, target_distance
```

The data collection system provides comprehensive, high-quality datasets for haptic research. Understanding these formats and analysis workflows enables effective experimental data processing and scientific insights.