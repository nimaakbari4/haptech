#!/usr/bin/env python3
"""
MoveIt-based homing functionality for Franka robot.
This module provides safe, motion-planned homing using MoveIt's Python interface.
"""

import sys
import rospy
import moveit_commander
import moveit_msgs.msg
import geometry_msgs.msg
from math import pi
import copy


class FrankaHoming:
    """
    Franka robot homing using MoveIt motion planning.
    Provides safe, collision-aware homing to predefined positions.
    """
    
    def __init__(self):
        """Initialize MoveIt interface for Franka robot."""
        # Initialize moveit_commander and rospy
        moveit_commander.roscpp_initialize(sys.argv)
        rospy.init_node('franka_homing', anonymous=True)
        
        # Instantiate RobotCommander object
        self.robot = moveit_commander.RobotCommander()
        
        # Instantiate PlanningSceneInterface object
        self.scene = moveit_commander.PlanningSceneInterface()
        
        # Instantiate MoveGroupCommander for the panda_arm group
        self.group_name = "panda_arm"
        self.move_group = moveit_commander.MoveGroupCommander(self.group_name)
        
        # Create a DisplayTrajectory ROS publisher for visualization
        self.display_trajectory_publisher = rospy.Publisher(
            '/move_group/display_planned_path',
            moveit_msgs.msg.DisplayTrajectory,
            queue_size=20
        )
        
        # Define safe home positions (joint angles in radians)
        self.home_positions = {
            'default': [0, -pi/4, 0, -3*pi/4, 0, pi/2, pi/4],
            'ready': [0, -pi/6, 0, -2*pi/3, 0, pi/2, pi/4],
            'safe': [0, -pi/8, 0, -pi/2, 0, pi/3, pi/6],
            # Note: For Panda, joint 4 (index 3) cannot be 0 rad; keep it slightly within limits
            'zero': [0.0, 0.0, 0.0, -0.10, 0.0, 0.0, 0.0]
        }
        
        # Set planning parameters for safe, smooth motion
        self.move_group.set_planning_time(10.0)  # Allow up to 10 seconds for planning
        self.move_group.set_num_planning_attempts(5)  # Try up to 5 planning attempts
        self.move_group.set_max_velocity_scaling_factor(0.3)  # 30% of max velocity
        self.move_group.set_max_acceleration_scaling_factor(0.3)  # 30% of max acceleration
        
        print("FrankaHoming initialized successfully")
        print(f"Planning frame: {self.move_group.get_planning_frame()}")
        print(f"End effector link: {self.move_group.get_end_effector_link()}")
        print(f"Available planning groups: {self.robot.get_group_names()}")
    
    def get_current_joint_values(self):
        """Get current joint values of the robot."""
        return self.move_group.get_current_joint_values()
    
    def get_current_pose(self):
        """Get current pose of the end effector."""
        return self.move_group.get_current_pose().pose
    
    def is_at_home(self, home_name='default', tolerance=0.1):
        """
        Check if robot is already at the specified home position.
        
        Args:
            home_name (str): Name of home position to check
            tolerance (float): Tolerance for considering positions equal (radians)
        
        Returns:
            bool: True if robot is at home position within tolerance
        """
        if home_name not in self.home_positions:
            print(f"Warning: Unknown home position '{home_name}'")
            return False
        
        current_joints = self.get_current_joint_values()
        target_joints = self.home_positions[home_name]
        
        # Check if all joints are within tolerance
        for i, (current, target) in enumerate(zip(current_joints, target_joints)):
            if abs(current - target) > tolerance:
                return False
        
        return True
    
    def plan_to_home(self, home_name='default'):
        """
        Plan a trajectory to the specified home position.
        
        Args:
            home_name (str): Name of home position to plan to
        
        Returns:
            tuple: (success, plan) where success is bool and plan is the trajectory
        """
        if home_name not in self.home_positions:
            print(f"Error: Unknown home position '{home_name}'")
            return False, None
        
        print(f"Planning to home position: {home_name}")
        
        # Get target joint values
        joint_goal = self.home_positions[home_name]
        
        # Set the joint target
        self.move_group.set_joint_value_target(joint_goal)
        
        # Plan the trajectory
        plan = self.move_group.plan()
        
        # Check if planning was successful
        if not plan[0]:  # plan[0] is the success flag
            print(f"Failed to plan trajectory to {home_name}")
            return False, None
        
        print(f"Successfully planned trajectory to {home_name}")
        return True, plan[1]  # plan[1] is the actual trajectory
    
    def execute_home(self, home_name='default', wait=True):
        """
        Execute homing to the specified position.
        
        Args:
            home_name (str): Name of home position to move to
            wait (bool): Whether to wait for execution to complete
        
        Returns:
            bool: True if execution was successful
        """
        if home_name not in self.home_positions:
            print(f"Error: Unknown home position '{home_name}'")
            return False
        
        # Check if already at home
        if self.is_at_home(home_name):
            print(f"Robot is already at {home_name} home position")
            return True
        
        print(f"Executing homing to {home_name}...")
        
        # Get target joint values
        joint_goal = self.home_positions[home_name]
        
        # Set the joint target
        self.move_group.set_joint_value_target(joint_goal)
        
        # Execute the motion
        success = self.move_group.go(wait=wait)
        
        if success:
            print(f"Successfully moved to {home_name} home position")
            # Stop any residual movement
            self.move_group.stop()
            # Clear targets
            self.move_group.clear_pose_targets()
        else:
            print(f"Failed to execute homing to {home_name}")
        
        return success
    
    def plan_and_execute_home(self, home_name='default', visualize=False):
        """
        Plan and execute homing with optional visualization.
        
        Args:
            home_name (str): Name of home position to move to
            visualize (bool): Whether to visualize the planned trajectory
        
        Returns:
            bool: True if execution was successful
        """
        if home_name not in self.home_positions:
            print(f"Error: Unknown home position '{home_name}'")
            return False
        
        # Check if already at home
        if self.is_at_home(home_name):
            print(f"Robot is already at {home_name} home position")
            return True
        
        print(f"Planning and executing homing to {home_name}...")
        
        # Plan the trajectory
        success, plan = self.plan_to_home(home_name)
        if not success:
            return False
        
        # Visualize the plan if requested
        if visualize:
            self.visualize_trajectory(plan)
            input("Press Enter to execute the planned trajectory...")
        
        # Execute the plan
        success = self.move_group.execute(plan, wait=True)
        
        if success:
            print(f"Successfully executed homing to {home_name}")
            # Stop any residual movement
            self.move_group.stop()
            # Clear targets
            self.move_group.clear_pose_targets()
        else:
            print(f"Failed to execute homing to {home_name}")
        
        return success
    
    def visualize_trajectory(self, plan):
        """
        Visualize a planned trajectory in RViz.
        
        Args:
            plan: The planned trajectory to visualize
        """
        display_trajectory = moveit_msgs.msg.DisplayTrajectory()
        display_trajectory.trajectory_start = self.robot.get_current_state()
        display_trajectory.trajectory.append(plan)
        
        # Publish the trajectory
        self.display_trajectory_publisher.publish(display_trajectory)
        print("Trajectory published for visualization in RViz")
    
    def add_home_position(self, name, joint_values):
        """
        Add a new home position.
        
        Args:
            name (str): Name of the home position
            joint_values (list): List of 7 joint angles in radians
        """
        if len(joint_values) != 7:
            print("Error: Home position must have exactly 7 joint values")
            return False
        
        self.home_positions[name] = joint_values
        print(f"Added home position '{name}' with values: {joint_values}")
        return True
    
    def list_home_positions(self):
        """List all available home positions."""
        print("Available home positions:")
        for name, values in self.home_positions.items():
            print(f"  {name}: {[f'{v:.3f}' for v in values]}")
    
    def get_robot_info(self):
        """Print detailed robot information."""
        print("=== Robot Information ===")
        print(f"Planning frame: {self.move_group.get_planning_frame()}")
        print(f"End effector link: {self.move_group.get_end_effector_link()}")
        print(f"Available planning groups: {self.robot.get_group_names()}")
        print(f"Current joint values: {[f'{v:.3f}' for v in self.get_current_joint_values()]}")
        print(f"Current pose: {self.get_current_pose()}")


def main():
    """Example usage of the FrankaHoming class."""
    try:
        # Initialize homing
        homing = FrankaHoming()
        
        # Print robot information
        homing.get_robot_info()
        
        # List available home positions
        homing.list_home_positions()
        
        # Check if already at home
        if homing.is_at_home('default'):
            print("Robot is already at default home position")
        else:
            print("Robot is not at home position")
        
        # Execute homing to default position
        print("\nExecuting homing to default position...")
        success = homing.execute_home('default')
        
        if success:
            print("Homing completed successfully!")
        else:
            print("Homing failed!")
        
    except rospy.ROSInterruptException:
        print("ROS interrupted")
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()

