# Experiment Scripts

Complete guide to writing automated experiment scripts for haptic research studies.

## Overview

Experiment scripts allow you to automate complex research protocols, run multiple trials in sequence, manage subject sessions, and implement adaptive experimental designs. This automation reduces operator workload and ensures consistent experimental procedures.

**What you can automate**:
- **Multi-trial experiments**: Run sequences of related trials
- **Subject management**: Handle participant data and session organization
- **Adaptive protocols**: Modify trials based on performance
- **Data validation**: Real-time quality control
- **Block randomization**: Counterbalanced experimental designs
- **Progress tracking**: Monitor completion and performance

**Benefits**:
- **Consistency**: Identical procedures across participants
- **Efficiency**: Reduced setup time between trials
- **Reliability**: Automated error handling and recovery
- **Data integrity**: Built-in validation and backup
- **Flexibility**: Easy parameter adjustments

## Basic Script Structure

### Simple Experiment Template

```python
#!/usr/bin/env python3
"""
Basic experiment script template
"""

import os
import sys
import time
import json
from datetime import datetime

# Add the haptech client to path
sys.path.append('/home/peter/haptech/python_client')

from trials import TrialRunner
from settings import load_settings
import util

def run_experiment(subject_id, config_files):
    """Run a sequence of trials for one subject"""
    
    # Initialize
    settings = load_settings()
    runner = TrialRunner(settings)
    
    print(f"Starting experiment for subject: {subject_id}")
    print(f"Trials to run: {len(config_files)}")
    
    results = []
    
    try:
        for i, config_file in enumerate(config_files):
            print(f"\nTrial {i+1}/{len(config_files)}: {config_file}")
            
            # Run trial
            result = runner.run_trial(config_file, subject_id)
            results.append(result)
            
            # Check for success/failure
            if result['success']:
                print(f"✓ Trial completed successfully")
            else:
                print(f"✗ Trial failed: {result.get('message', 'Unknown error')}")
                
                # Decide whether to continue or abort
                response = input("Continue with next trial? (y/n): ")
                if response.lower() != 'y':
                    break
            
            # Brief pause between trials
            time.sleep(2)
    
    except KeyboardInterrupt:
        print("\nExperiment interrupted by user")
    
    except Exception as e:
        print(f"Experiment error: {e}")
    
    finally:
        runner.cleanup()
    
    # Print summary
    successful_trials = sum(1 for r in results if r['success'])
    print(f"\nExperiment Summary:")
    print(f"Completed trials: {len(results)}")
    print(f"Successful trials: {successful_trials}")
    print(f"Success rate: {successful_trials/len(results)*100:.1f}%")
    
    return results

if __name__ == "__main__":
    # Configuration
    SUBJECT_ID = input("Enter subject ID: ")
    CONFIG_FILES = [
        "config/practice_trial.xml",
        "config/baseline_trial.xml", 
        "config/experimental_trial.xml"
    ]
    
    # Run experiment
    results = run_experiment(SUBJECT_ID, CONFIG_FILES)
```

### Enhanced Script with Error Handling

```python
#!/usr/bin/env python3
"""
Robust experiment script with comprehensive error handling
"""

import os
import sys
import time
import json
import logging
from datetime import datetime
from pathlib import Path

sys.path.append('/home/peter/haptech/python_client')

from trials import TrialRunner
from settings import load_settings
import util

class ExperimentRunner:
    def __init__(self, subject_id, experiment_config):
        self.subject_id = subject_id
        self.config = experiment_config
        self.setup_logging()
        
        # Initialize trial runner
        settings = load_settings()
        self.runner = TrialRunner(settings)
        
        # Experiment state
        self.results = []
        self.current_trial = 0
        self.start_time = datetime.now()
        
    def setup_logging(self):
        """Setup experiment logging"""
        log_dir = Path("experiments/logs")
        log_dir.mkdir(parents=True, exist_ok=True)
        
        log_file = log_dir / f"experiment_{self.subject_id}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
        
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file),
                logging.StreamHandler()
            ]
        )
        
        self.logger = logging.getLogger(__name__)
    
    def save_state(self):
        """Save current experiment state"""
        state = {
            'subject_id': self.subject_id,
            'start_time': self.start_time.isoformat(),
            'current_trial': self.current_trial,
            'results': self.results,
            'config': self.config
        }
        
        state_file = f"experiments/state_{self.subject_id}.json"
        with open(state_file, 'w') as f:
            json.dump(state, f, indent=2)
    
    def load_state(self):
        """Load previous experiment state if available"""
        state_file = f"experiments/state_{self.subject_id}.json"
        
        if os.path.exists(state_file):
            with open(state_file, 'r') as f:
                state = json.load(f)
            
            self.current_trial = state.get('current_trial', 0)
            self.results = state.get('results', [])
            
            self.logger.info(f"Resumed from trial {self.current_trial}")
            return True
        
        return False
    
    def run_experiment(self):
        """Run the complete experiment"""
        self.logger.info(f"Starting experiment for subject: {self.subject_id}")
        
        # Try to resume if previous state exists
        resumed = self.load_state()
        if resumed:
            response = input("Previous session found. Resume? (y/n): ")
            if response.lower() != 'y':
                self.current_trial = 0
                self.results = []
        
        trials = self.config['trials']
        
        try:
            while self.current_trial < len(trials):
                trial_config = trials[self.current_trial]
                
                self.logger.info(f"Starting trial {self.current_trial + 1}/{len(trials)}")
                
                # Run trial with retry logic
                result = self.run_trial_with_retry(trial_config)
                self.results.append(result)
                
                # Save state after each trial
                self.save_state()
                
                self.current_trial += 1
                
                # Inter-trial interval
                if 'inter_trial_interval' in self.config:
                    self.wait_with_feedback(self.config['inter_trial_interval'])
        
        except KeyboardInterrupt:
            self.logger.warning("Experiment interrupted by user")
            self.save_state()
        
        except Exception as e:
            self.logger.error(f"Experiment failed: {e}")
            self.save_state()
            raise
        
        finally:
            self.cleanup()
        
        return self.generate_summary()
    
    def run_trial_with_retry(self, trial_config, max_retries=3):
        """Run a single trial with retry logic"""
        
        for attempt in range(max_retries):
            try:
                result = self.runner.run_trial(
                    trial_config['config_file'], 
                    self.subject_id,
                    trial_id=trial_config.get('trial_id', f"trial_{self.current_trial:03d}")
                )
                
                # Validate result
                if self.validate_trial_result(result, trial_config):
                    return result
                else:
                    self.logger.warning(f"Trial validation failed (attempt {attempt + 1})")
                    
            except Exception as e:
                self.logger.error(f"Trial failed (attempt {attempt + 1}): {e}")
                
                if attempt < max_retries - 1:
                    response = input("Retry trial? (y/n): ")
                    if response.lower() != 'y':
                        break
        
        # All attempts failed
        return {
            'success': False,
            'message': 'Trial failed after all retry attempts',
            'attempts': max_retries
        }
    
    def validate_trial_result(self, result, trial_config):
        """Validate that trial completed properly"""
        
        # Basic success check
        if not result.get('success', False):
            return False
        
        # Check minimum duration
        min_duration = trial_config.get('min_duration', 1.0)
        if result.get('duration', 0) < min_duration:
            self.logger.warning(f"Trial too short: {result.get('duration')} < {min_duration}")
            return False
        
        # Check data file exists
        if 'data_file' in result:
            if not os.path.exists(result['data_file']):
                self.logger.warning(f"Data file missing: {result['data_file']}")
                return False
        
        return True
    
    def wait_with_feedback(self, duration):
        """Wait between trials with countdown"""
        print(f"\nInter-trial interval: {duration} seconds")
        
        for remaining in range(int(duration), 0, -1):
            print(f"Next trial in {remaining} seconds... (press Enter to skip)", end='\r')
            
            # Check for user input (non-blocking)
            import select
            if select.select([sys.stdin], [], [], 1) == ([sys.stdin], [], []):
                input()  # Consume the input
                break
            
        print("\n")
    
    def generate_summary(self):
        """Generate experiment summary"""
        summary = {
            'subject_id': self.subject_id,
            'start_time': self.start_time.isoformat(),
            'end_time': datetime.now().isoformat(),
            'total_trials': len(self.results),
            'successful_trials': sum(1 for r in self.results if r.get('success')),
            'completion_rate': len(self.results) / len(self.config['trials']) * 100,
            'results': self.results
        }
        
        # Save summary
        summary_file = f"experiments/summary_{self.subject_id}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(summary_file, 'w') as f:
            json.dump(summary, f, indent=2)
        
        # Print summary
        print(f"\n{'='*50}")
        print(f"EXPERIMENT SUMMARY")
        print(f"{'='*50}")
        print(f"Subject: {summary['subject_id']}")
        print(f"Trials completed: {summary['total_trials']}/{len(self.config['trials'])}")
        print(f"Success rate: {summary['successful_trials']}/{summary['total_trials']} ({summary['successful_trials']/summary['total_trials']*100:.1f}%)")
        print(f"Completion rate: {summary['completion_rate']:.1f}%")
        print(f"Duration: {datetime.fromisoformat(summary['end_time']) - datetime.fromisoformat(summary['start_time'])}")
        print(f"Summary saved: {summary_file}")
        
        return summary
    
    def cleanup(self):
        """Clean up resources"""
        if hasattr(self, 'runner'):
            self.runner.cleanup()

# Example usage
if __name__ == "__main__":
    # Experiment configuration
    experiment_config = {
        'name': 'Reaching Task Study',
        'version': '1.0',
        'inter_trial_interval': 5,  # seconds
        'trials': [
            {
                'trial_id': 'practice_001',
                'config_file': 'config/practice_trial.xml',
                'min_duration': 2.0
            },
            {
                'trial_id': 'baseline_001', 
                'config_file': 'config/baseline_trial.xml',
                'min_duration': 5.0
            },
            {
                'trial_id': 'experimental_001',
                'config_file': 'config/experimental_trial.xml', 
                'min_duration': 10.0
            }
        ]
    }
    
    # Get subject ID
    subject_id = input("Enter subject ID: ")
    
    # Run experiment
    runner = ExperimentRunner(subject_id, experiment_config)
    summary = runner.run_experiment()
```

## Advanced Experiment Features

### Randomized Trial Orders

```python
def randomize_trials(trials, block_size=None, seed=None):
    """Randomize trial order with optional blocking"""
    import random
    
    if seed:
        random.seed(seed)
    
    if block_size:
        # Block randomization
        randomized = []
        for i in range(0, len(trials), block_size):
            block = trials[i:i+block_size]
            random.shuffle(block)
            randomized.extend(block)
        return randomized
    else:
        # Complete randomization
        randomized = trials.copy()
        random.shuffle(randomized)
        return randomized

# Example usage
trials = [
    {'config_file': 'condition_A.xml', 'condition': 'A'},
    {'config_file': 'condition_B.xml', 'condition': 'B'},
    {'config_file': 'condition_C.xml', 'condition': 'C'},
] * 10  # 30 trials total (10 of each condition)

# Block randomization (3 trials per block)
randomized_trials = randomize_trials(trials, block_size=3, seed=12345)
```

### Adaptive Experiments

```python
class AdaptiveExperiment(ExperimentRunner):
    def __init__(self, subject_id, initial_config):
        super().__init__(subject_id, initial_config)
        self.performance_history = []
        self.difficulty_level = 0.5  # Start at medium difficulty
    
    def adapt_difficulty(self, recent_results):
        """Adjust difficulty based on recent performance"""
        recent_success_rate = sum(r.get('success', False) for r in recent_results) / len(recent_results)
        
        # Target 70-80% success rate
        if recent_success_rate > 0.8:
            # Too easy, increase difficulty
            self.difficulty_level = min(1.0, self.difficulty_level + 0.1)
            print(f"Increasing difficulty to {self.difficulty_level:.1f}")
        elif recent_success_rate < 0.7:
            # Too hard, decrease difficulty  
            self.difficulty_level = max(0.0, self.difficulty_level - 0.1)
            print(f"Decreasing difficulty to {self.difficulty_level:.1f}")
    
    def generate_next_trial(self):
        """Generate next trial based on current difficulty"""
        
        # Adjust target size based on difficulty
        base_size = 0.03  # 3cm radius
        size_range = 0.02  # ±2cm
        target_size = base_size + (0.5 - self.difficulty_level) * size_range
        
        # Adjust target distance based on difficulty  
        base_distance = 0.15  # 15cm
        distance_range = 0.10  # ±10cm
        target_distance = base_distance + self.difficulty_level * distance_range
        
        return {
            'config_file': 'adaptive_trial_template.xml',
            'parameters': {
                'target_size': target_size,
                'target_distance': target_distance,
                'difficulty': self.difficulty_level
            }
        }
    
    def run_adaptive_experiment(self, total_trials=50, adaptation_window=5):
        """Run experiment with adaptive difficulty"""
        
        self.logger.info(f"Starting adaptive experiment: {total_trials} trials")
        
        for trial_num in range(total_trials):
            # Adapt difficulty every few trials
            if trial_num > 0 and trial_num % adaptation_window == 0:
                recent_results = self.results[-adaptation_window:]
                self.adapt_difficulty(recent_results)
            
            # Generate next trial
            trial_config = self.generate_next_trial()
            trial_config['trial_id'] = f"adaptive_{trial_num:03d}"
            
            # Run trial
            result = self.run_trial_with_retry(trial_config)
            result['difficulty'] = self.difficulty_level
            self.results.append(result)
            
            # Save state
            self.save_state()
            
            self.logger.info(f"Trial {trial_num + 1} complete (difficulty: {self.difficulty_level:.2f})")
        
        return self.generate_summary()
```

### Multi-Session Experiments

```python
class MultiSessionExperiment:
    def __init__(self, study_config):
        self.config = study_config
        self.study_dir = Path(f"experiments/{study_config['study_name']}")
        self.study_dir.mkdir(parents=True, exist_ok=True)
        
        # Load or initialize study state
        self.load_study_state()
    
    def load_study_state(self):
        """Load study progress across sessions"""
        state_file = self.study_dir / "study_state.json"
        
        if state_file.exists():
            with open(state_file, 'r') as f:
                self.state = json.load(f)
        else:
            self.state = {
                'participants': {},
                'completed_sessions': 0,
                'start_date': datetime.now().isoformat()
            }
    
    def save_study_state(self):
        """Save study progress"""
        state_file = self.study_dir / "study_state.json"
        with open(state_file, 'w') as f:
            json.dump(self.state, f, indent=2)
    
    def register_participant(self, participant_id, condition):
        """Register new participant"""
        if participant_id not in self.state['participants']:
            self.state['participants'][participant_id] = {
                'condition': condition,
                'sessions_completed': 0,
                'total_sessions': self.config['sessions_per_participant'],
                'enrollment_date': datetime.now().isoformat()
            }
            self.save_study_state()
            print(f"Registered participant {participant_id} in condition {condition}")
    
    def run_session(self, participant_id):
        """Run one session for a participant"""
        if participant_id not in self.state['participants']:
            raise ValueError(f"Participant {participant_id} not registered")
        
        participant = self.state['participants'][participant_id]
        session_num = participant['sessions_completed'] + 1
        
        print(f"Running session {session_num} for {participant_id}")
        
        # Get session configuration
        session_config = self.config['session_configs'][session_num - 1]
        condition = participant['condition']
        
        # Modify config based on condition
        if condition in session_config['condition_modifications']:
            modifications = session_config['condition_modifications'][condition]
            session_config.update(modifications)
        
        # Run session
        session_runner = ExperimentRunner(
            f"{participant_id}_session_{session_num:02d}",
            session_config
        )
        
        summary = session_runner.run_experiment()
        
        # Update participant state
        participant['sessions_completed'] = session_num
        participant[f'session_{session_num}_date'] = datetime.now().isoformat()
        participant[f'session_{session_num}_summary'] = summary
        
        self.state['completed_sessions'] += 1
        self.save_study_state()
        
        print(f"Session {session_num} completed for {participant_id}")
        
        # Check if participant is complete
        if session_num >= participant['total_sessions']:
            print(f"Participant {participant_id} has completed all sessions!")
        
        return summary
    
    def get_study_progress(self):
        """Get overall study progress"""
        total_participants = len(self.state['participants'])
        completed_participants = sum(
            1 for p in self.state['participants'].values() 
            if p['sessions_completed'] >= p['total_sessions']
        )
        
        total_sessions = sum(p['total_sessions'] for p in self.state['participants'].values())
        completed_sessions = sum(p['sessions_completed'] for p in self.state['participants'].values())
        
        return {
            'total_participants': total_participants,
            'completed_participants': completed_participants,
            'total_sessions': total_sessions,
            'completed_sessions': completed_sessions,
            'completion_rate': completed_sessions / total_sessions * 100 if total_sessions > 0 else 0
        }

# Example study configuration
study_config = {
    'study_name': 'Motor_Learning_Study',
    'sessions_per_participant': 5,
    'session_configs': [
        {  # Session 1: Baseline
            'name': 'Baseline',
            'inter_trial_interval': 3,
            'trials': [
                {'config_file': 'baseline_trial.xml', 'repetitions': 20}
            ],
            'condition_modifications': {
                'control': {},
                'feedback': {'enable_visual_feedback': True}
            }
        },
        {  # Session 2-4: Training
            'name': 'Training',
            'inter_trial_interval': 2,
            'trials': [
                {'config_file': 'training_trial.xml', 'repetitions': 50}
            ],
            'condition_modifications': {
                'control': {'feedback_type': 'minimal'},
                'feedback': {'feedback_type': 'enhanced'}
            }
        },
        {  # Session 5: Test
            'name': 'Test',
            'inter_trial_interval': 3,
            'trials': [
                {'config_file': 'test_trial.xml', 'repetitions': 30}
            ],
            'condition_modifications': {
                'control': {},
                'feedback': {}
            }
        }
    ]
}

# Usage example
study = MultiSessionExperiment(study_config)

# Register participants
study.register_participant("P001", "control")
study.register_participant("P002", "feedback")

# Run sessions
study.run_session("P001")  # P001's first session
study.run_session("P002")  # P002's first session

# Check progress
progress = study.get_study_progress()
print(f"Study progress: {progress['completion_rate']:.1f}%")
```

## Data Collection Integration

### Real-time Data Monitoring

```python
class MonitoredExperiment(ExperimentRunner):
    def __init__(self, subject_id, experiment_config):
        super().__init__(subject_id, experiment_config)
        self.data_monitor = RealTimeDataMonitor()
        self.quality_thresholds = {
            'max_velocity': 2.0,      # m/s
            'max_force': 15.0,        # N
            'min_samples': 100,       # minimum data points
            'position_bound': 0.5     # workspace bounds (m)
        }
    
    def monitor_trial_quality(self, trial_data):
        """Monitor data quality during trial"""
        warnings = []
        
        # Check velocity spikes
        velocities = np.sqrt(trial_data['vel_x']**2 + trial_data['vel_y']**2 + trial_data['vel_z']**2)
        if velocities.max() > self.quality_thresholds['max_velocity']:
            warnings.append(f"High velocity detected: {velocities.max():.2f} m/s")
        
        # Check force levels
        forces = np.sqrt(trial_data['force_x']**2 + trial_data['force_y']**2 + trial_data['force_z']**2)
        if forces.max() > self.quality_thresholds['max_force']:
            warnings.append(f"High force detected: {forces.max():.2f} N")
        
        # Check data completeness
        if len(trial_data) < self.quality_thresholds['min_samples']:
            warnings.append(f"Insufficient data: {len(trial_data)} samples")
        
        # Check workspace bounds
        positions = trial_data[['pos_x', 'pos_y', 'pos_z']].abs()
        if (positions > self.quality_thresholds['position_bound']).any().any():
            warnings.append("Position outside workspace bounds")
        
        return warnings
    
    def run_monitored_trial(self, trial_config):
        """Run trial with real-time monitoring"""
        
        # Start monitoring
        self.data_monitor.start()
        
        try:
            # Run trial
            result = super().run_trial_with_retry(trial_config)
            
            # Get collected data
            trial_data = self.data_monitor.get_data()
            
            # Check data quality
            warnings = self.monitor_trial_quality(trial_data)
            
            if warnings:
                print("Data quality warnings:")
                for warning in warnings:
                    print(f"  ⚠️ {warning}")
                
                # Ask if trial should be repeated
                response = input("Repeat trial due to data quality issues? (y/n): ")
                if response.lower() == 'y':
                    return self.run_monitored_trial(trial_config)
            
            # Add quality info to result
            result['data_quality'] = {
                'warnings': warnings,
                'quality_score': self.calculate_quality_score(trial_data)
            }
            
        finally:
            self.data_monitor.stop()
        
        return result
    
    def calculate_quality_score(self, trial_data):
        """Calculate overall data quality score (0-100)"""
        score = 100
        
        # Penalize for various issues
        velocities = np.sqrt(trial_data['vel_x']**2 + trial_data['vel_y']**2 + trial_data['vel_z']**2)
        if velocities.max() > self.quality_thresholds['max_velocity']:
            score -= 20
        
        forces = np.sqrt(trial_data['force_x']**2 + trial_data['force_y']**2 + trial_data['force_z']**2)
        if forces.max() > self.quality_thresholds['max_force']:
            score -= 15
        
        if len(trial_data) < self.quality_thresholds['min_samples']:
            score -= 30
        
        positions = trial_data[['pos_x', 'pos_y', 'pos_z']].abs()
        if (positions > self.quality_thresholds['position_bound']).any().any():
            score -= 25
        
        return max(0, score)
```

### Automated Analysis Pipeline

```python
class AnalysisIntegratedExperiment(ExperimentRunner):
    def __init__(self, subject_id, experiment_config):
        super().__init__(subject_id, experiment_config)
        self.analysis_pipeline = []
        
        # Add default analyses
        self.add_analysis('basic_metrics', self.calculate_basic_metrics)
        self.add_analysis('movement_efficiency', self.calculate_movement_efficiency)
        self.add_analysis('error_metrics', self.calculate_error_metrics)
    
    def add_analysis(self, name, function):
        """Add analysis function to pipeline"""
        self.analysis_pipeline.append((name, function))
    
    def calculate_basic_metrics(self, trial_data, metadata):
        """Calculate basic performance metrics"""
        # Movement time
        movement_time = trial_data['timestamp'].max()
        
        # Path length
        dx = trial_data['pos_x'].diff().fillna(0)
        dy = trial_data['pos_y'].diff().fillna(0) 
        dz = trial_data['pos_z'].diff().fillna(0)
        path_length = np.sqrt(dx**2 + dy**2 + dz**2).sum()
        
        # Peak velocity
        velocities = np.sqrt(trial_data['vel_x']**2 + trial_data['vel_y']**2 + trial_data['vel_z']**2)
        peak_velocity = velocities.max()
        mean_velocity = velocities.mean()
        
        return {
            'movement_time': movement_time,
            'path_length': path_length,
            'peak_velocity': peak_velocity,
            'mean_velocity': mean_velocity
        }
    
    def calculate_movement_efficiency(self, trial_data, metadata):
        """Calculate movement efficiency metrics"""
        
        # Get start and end positions
        start_pos = trial_data[['pos_x', 'pos_y', 'pos_z']].iloc[0].values
        end_pos = trial_data[['pos_x', 'pos_y', 'pos_z']].iloc[-1].values
        
        # Straight-line distance
        straight_distance = np.linalg.norm(end_pos - start_pos)
        
        # Actual path length
        dx = trial_data['pos_x'].diff().fillna(0)
        dy = trial_data['pos_y'].diff().fillna(0)
        dz = trial_data['pos_z'].diff().fillna(0)
        actual_distance = np.sqrt(dx**2 + dy**2 + dz**2).sum()
        
        # Efficiency metrics
        path_efficiency = straight_distance / actual_distance if actual_distance > 0 else 0
        
        # Velocity profile smoothness (number of velocity peaks)
        velocities = np.sqrt(trial_data['vel_x']**2 + trial_data['vel_y']**2 + trial_data['vel_z']**2)
        velocity_peaks = len(find_peaks(velocities, height=velocities.mean())[0])
        
        return {
            'straight_distance': straight_distance,
            'actual_distance': actual_distance,
            'path_efficiency': path_efficiency,
            'velocity_peaks': velocity_peaks,
            'movement_smoothness': 1.0 / (velocity_peaks + 1)  # Higher = smoother
        }
    
    def calculate_error_metrics(self, trial_data, metadata):
        """Calculate error and accuracy metrics"""
        
        # Target position from metadata
        target_pos = None
        if 'trial_outcome' in metadata and 'final_position' in metadata['trial_outcome']:
            target_info = metadata['configuration']['end_conditions'][0]['parameters']
            target_pos = np.array([
                float(target_info.get('target_x', 0)),
                float(target_info.get('target_y', 0)),
                float(target_info.get('target_z', 0))
            ])
        
        if target_pos is not None:
            # Final position error
            final_pos = trial_data[['pos_x', 'pos_y', 'pos_z']].iloc[-1].values
            final_error = np.linalg.norm(final_pos - target_pos)
            
            # Continuous error during movement
            positions = trial_data[['pos_x', 'pos_y', 'pos_z']].values
            errors = np.linalg.norm(positions - target_pos, axis=1)
            
            return {
                'final_error': final_error,
                'mean_error': errors.mean(),
                'max_error': errors.max(),
                'error_variance': errors.var()
            }
        
        return {}
    
    def run_trial_with_analysis(self, trial_config):
        """Run trial and perform immediate analysis"""
        
        # Run trial
        result = super().run_trial_with_retry(trial_config)
        
        if result.get('success') and 'data_file' in result:
            # Load trial data
            trial_data = pd.read_csv(result['data_file'])
            metadata_file = result['data_file'].replace('_results.csv', '_metadata.json')
            
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
            
            # Run analysis pipeline
            analysis_results = {}
            
            for analysis_name, analysis_func in self.analysis_pipeline:
                try:
                    analysis_result = analysis_func(trial_data, metadata)
                    analysis_results[analysis_name] = analysis_result
                    
                except Exception as e:
                    self.logger.warning(f"Analysis {analysis_name} failed: {e}")
                    analysis_results[analysis_name] = {'error': str(e)}
            
            # Save analysis results
            analysis_file = result['data_file'].replace('_results.csv', '_analysis.json')
            with open(analysis_file, 'w') as f:
                json.dump(analysis_results, f, indent=2)
            
            # Add to result
            result['analysis'] = analysis_results
            result['analysis_file'] = analysis_file
            
            # Print quick summary
            if 'basic_metrics' in analysis_results:
                metrics = analysis_results['basic_metrics']
                print(f"  📊 Movement time: {metrics.get('movement_time', 0):.2f}s")
                print(f"  📊 Peak velocity: {metrics.get('peak_velocity', 0):.3f}m/s")
                print(f"  📊 Path length: {metrics.get('path_length', 0):.3f}m")
            
            if 'error_metrics' in analysis_results and analysis_results['error_metrics']:
                errors = analysis_results['error_metrics']
                if 'final_error' in errors:
                    print(f"  🎯 Final error: {errors['final_error']:.4f}m")
        
        return result

# Find peaks function (simplified version)
def find_peaks(data, height=None):
    """Simple peak detection"""
    peaks = []
    for i in range(1, len(data) - 1):
        if data[i] > data[i-1] and data[i] > data[i+1]:
            if height is None or data[i] >= height:
                peaks.append(i)
    return peaks, None
```

## Best Practices

### Script Organization

**File structure**:
```
experiments/
├── scripts/
│   ├── basic_experiment.py         # Simple experiment template
│   ├── adaptive_experiment.py      # Adaptive difficulty
│   ├── multi_session_study.py      # Multi-session management
│   └── analysis_experiment.py      # With integrated analysis
├── configs/
│   ├── study_configs/             # Study-level configurations
│   ├── session_configs/           # Session-level configurations  
│   └── trial_configs/             # Individual trial configs
├── logs/                          # Experiment logs
├── state/                         # Saved experiment states
├── results/                       # Analysis results
└── templates/                     # Script templates
```

**Code organization principles**:
- **Modular design**: Separate data collection, analysis, and reporting
- **Configuration driven**: Use external config files for parameters
- **Error resilient**: Handle failures gracefully with recovery options
- **State preservation**: Save progress to allow resumption
- **Logging**: Comprehensive logging for debugging and audit trails

### Testing and Validation

**Test your scripts**:
```python
def test_experiment_script():
    """Test experiment script with mock data"""
    
    # Use test configuration
    test_config = {
        'trials': [
            {'config_file': 'config/test_trial.xml', 'trial_id': 'test_001'}
        ]
    }
    
    # Run with test subject
    runner = ExperimentRunner("TEST_SUBJECT", test_config)
    
    # Override trial runner for testing
    class MockTrialRunner:
        def run_trial(self, config_file, subject_id, trial_id=None):
            return {
                'success': True,
                'duration': 10.0,
                'data_file': 'test_data.csv',
                'message': 'Mock trial completed'
            }
        
        def cleanup(self):
            pass
    
    runner.runner = MockTrialRunner()
    
    # Run test
    summary = runner.run_experiment()
    
    assert summary['total_trials'] == 1
    assert summary['successful_trials'] == 1
    
    print("✓ Script test passed")

# Run tests before real experiments
if __name__ == "__main__":
    if "--test" in sys.argv:
        test_experiment_script()
    else:
        # Normal experiment code
        pass
```

### Performance Monitoring

**Resource usage monitoring**:
```python
import psutil
import threading
import time

class ResourceMonitor:
    def __init__(self):
        self.monitoring = False
        self.data = []
    
    def start_monitoring(self):
        self.monitoring = True
        self.monitor_thread = threading.Thread(target=self._monitor_loop)
        self.monitor_thread.start()
    
    def stop_monitoring(self):
        self.monitoring = False
        if hasattr(self, 'monitor_thread'):
            self.monitor_thread.join()
    
    def _monitor_loop(self):
        while self.monitoring:
            data_point = {
                'timestamp': time.time(),
                'cpu_percent': psutil.cpu_percent(),
                'memory_percent': psutil.virtual_memory().percent,
                'disk_usage': psutil.disk_usage('/').percent
            }
            self.data.append(data_point)
            time.sleep(1)  # Monitor every second
    
    def get_summary(self):
        if not self.data:
            return {}
        
        cpu_values = [d['cpu_percent'] for d in self.data]
        memory_values = [d['memory_percent'] for d in self.data]
        
        return {
            'duration': self.data[-1]['timestamp'] - self.data[0]['timestamp'],
            'avg_cpu': sum(cpu_values) / len(cpu_values),
            'max_cpu': max(cpu_values),
            'avg_memory': sum(memory_values) / len(memory_values),
            'max_memory': max(memory_values)
        }

# Use in experiments
class MonitoredExperimentRunner(ExperimentRunner):
    def __init__(self, subject_id, experiment_config):
        super().__init__(subject_id, experiment_config)
        self.resource_monitor = ResourceMonitor()
    
    def run_experiment(self):
        # Start resource monitoring
        self.resource_monitor.start_monitoring()
        
        try:
            # Run experiment
            summary = super().run_experiment()
            
            # Add resource usage to summary
            resource_summary = self.resource_monitor.get_summary()
            summary['resource_usage'] = resource_summary
            
            print(f"\n📈 Resource Usage:")
            print(f"  CPU: {resource_summary.get('avg_cpu', 0):.1f}% avg, {resource_summary.get('max_cpu', 0):.1f}% max")
            print(f"  Memory: {resource_summary.get('avg_memory', 0):.1f}% avg, {resource_summary.get('max_memory', 0):.1f}% max")
            
            return summary
            
        finally:
            self.resource_monitor.stop_monitoring()
```

This comprehensive guide covers all aspects of experiment script automation, from basic templates to advanced features like adaptive experiments and real-time analysis. The scripts provide robust error handling, state management, and data quality monitoring to ensure reliable research data collection.