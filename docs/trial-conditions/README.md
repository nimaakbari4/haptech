# Trial Conditions

Trial conditions define when your haptic experiment should start and end. Think of them as the "rules" that determine when a participant has completed a task successfully or failed.

## What are Trial Conditions?

Trial conditions are automatic checks that run continuously during your experiment. They monitor things like:
- **Time**: "End the trial after 30 seconds"
- **Position**: "End when the participant touches the target"
- **Velocity**: "End if they move too fast (safety condition)"
- **Custom logic**: Any condition you can program

## Why Use Conditions?

Conditions let you:
- **Automate experiments**: No need to manually stop trials
- **Ensure safety**: Automatically stop if something goes wrong  
- **Standardize protocols**: Consistent trial termination across participants
- **Collect precise data**: Know exactly when success/failure occurred

## How Conditions Work

Every trial has two types of conditions:
1. **Start Conditions**: When to begin the actual trial (often just "start immediately")
2. **End Conditions**: When to stop the trial (success, failure, timeout, etc.)

Conditions are checked many times per second. When a condition is met, the trial ends with either "success" or "failure" depending on how you configured it.

## Table of Contents

### Getting Started
- [Interface Specification](interface-specification.md) - Technical details of how conditions work
- [Implementation Guide](implementation-guide.md) - Step-by-step guide to create custom conditions
- [Built-in Conditions](built-in-conditions.md) - Ready-to-use conditions for common scenarios

### Advanced Topics  
- [Priority System](priority-system.md) - How multiple conditions interact
- [Evaluation Lifecycle](evaluation-lifecycle.md) - When and how conditions are checked
- [Troubleshooting](troubleshooting.md) - Common issues and solutions

## Quick Start

Most researchers will use existing conditions in their XML configuration:

```xml
<StartConditions>
  <Condition name="time" success="true" priority="1">
    <Parameters duration_seconds="0.0"/>
  </Condition>
</StartConditions>

<EndConditions>
  <!-- Success: reach the target -->
  <Condition name="position" success="true" priority="1">
    <Parameters target_x="0.1" target_y="0.0" target_z="0.0" tolerance="0.02"/>
  </Condition>
  
  <!-- Failure: timeout after 60 seconds -->
  <Condition name="time" success="false" priority="2">
    <Parameters duration_seconds="60.0"/>
  </Condition>
</EndConditions>
```

For custom conditions, see the [Implementation Guide](implementation-guide.md).