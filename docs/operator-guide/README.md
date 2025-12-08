# Operator Guide

Complete user guide for researchers and lab operators using the haptic system.

## Who This Guide Is For

This guide is designed for:
- **Researchers** conducting haptic experiments
- **Lab operators** running trials with participants
- **Students** learning to use the haptic system
- **Anyone** who needs to configure and run haptic trials

**What you'll learn**: How to set up, configure, and run haptic experiments from start to finish.

**Prerequisites**: Basic computer skills, familiarity with experimental procedures. No programming experience required for basic operation.

## System Overview

The haptic system consists of:
- **Core Server** (C++): Handles haptic rendering and physics simulation
- **Python Client**: Manages trials, collects data, and provides user interface  
- **XML Configuration**: Defines trials, objects, and experimental parameters
- **Haptic Device**: Provides force feedback (typically a Franka robot or Force Dimension device)

## Quick Start

1. **Setup**: Follow the [Installation](getting-started/installation.md) guide
2. **First Trial**: Run through [First Trial](getting-started/first-trial.md) to verify everything works
3. **Configure**: Create XML files using [XML Configuration](xml-configuration/README.md) guides
4. **Run Experiments**: Use [Python Client](python-client/README.md) to collect data
5. **Optimize**: Apply [Best Practices](best-practices/README.md) for smooth operation

## Table of Contents

### Getting Started
- [Getting Started Overview](getting-started/README.md) - Essential first steps
- [Installation](getting-started/installation.md) - Setup and installation instructions
- [First Trial](getting-started/first-trial.md) - Guide for running your first haptic trial
- [Basic Concepts](getting-started/basic-concepts.md) - Core concepts overview for the haptic system

### Python Client
- [Python Client Overview](python-client/README.md) - Running trials and collecting data
- [Command Line Interface](python-client/command-line-interface.md) - CLI options and usage
- [Data Collection](python-client/data-collection.md) - Data logging and CSV output configuration
- [Experiment Scripts](python-client/experiment-scripts.md) - Writing experiment automation scripts

### XML Configuration
- [XML Configuration Overview](xml-configuration/README.md) - Creating and modifying trial configurations
- [Structure Overview](xml-configuration/structure-overview.md) - Understanding XML file organization
- [Trial Conditions](xml-configuration/trial-conditions.md) - Defining when trials start and end
- [Objects](xml-configuration/objects.md) - Configuring haptic objects and scenes
- [Scene Settings](xml-configuration/scene-settings.md) - Camera, lighting, and visual settings
- [Data Collection](xml-configuration/data-collection.md) - Specifying what data to record
- [Examples](xml-configuration/examples/) - Complete example configurations
- [Reference](xml-configuration/reference/) - Complete parameter reference

### Best Practices
- [Best Practices Overview](best-practices/README.md) - Guidelines for effective experiments
- [Trial Design](best-practices/trial-design.md) - Designing effective haptic experiments
- [Data Analysis](best-practices/data-analysis.md) - Working with collected data
- [Performance Tips](best-practices/performance-tips.md) - Optimizing system performance
- [Safety Guidelines](best-practices/safety-guidelines.md) - Ensuring participant and equipment safety

### Troubleshooting
- [Troubleshooting Overview](troubleshooting/README.md) - Solving common problems
- [Common Errors](troubleshooting/common-errors.md) - Frequent issues and solutions
- [Hardware Issues](troubleshooting/hardware-issues.md) - Haptic device problems
- [Performance Issues](troubleshooting/performance-issues.md) - Lag, jitter, and stability problems

## Typical Workflow

1. **Plan Experiment**
   - Define research questions and hypotheses
   - Design trial structure and conditions
   - Choose appropriate haptic objects and interactions

2. **Create Configuration**
   - Write XML configuration file
   - Define trial conditions (start/end criteria)
   - Configure data collection parameters
   - Test with simple trials

3. **Setup Session**
   - Start haptic core server
   - Calibrate haptic device
   - Verify safety settings
   - Test configuration with practice trials

4. **Run Experiment**
   - Launch python client
   - Collect participant data
   - Monitor system performance
   - Handle any issues that arise

5. **Analyze Results**
   - Export CSV data
   - Process trial results
   - Generate reports and visualizations

## Support and Resources

- **Documentation**: This guide covers most common use cases
- **Example Files**: Check `python_client/config/` for working examples
- **Log Files**: Trial logs contain detailed information for debugging
- **Console Output**: Real-time feedback during trials

This guide will help you become proficient with the haptic system, from basic operation to advanced experimental configurations.