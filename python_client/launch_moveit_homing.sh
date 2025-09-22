#!/bin/bash
# Launch script for MoveIt-based Franka robot homing system

echo "Starting MoveIt-based Franka robot homing system..."

# Check if ROS is running
if ! pgrep -x "roscore" > /dev/null; then
    echo "Starting ROS core..."
    roscore &
    sleep 3
fi

# Check if required ROS packages are available
echo "Checking for required ROS packages..."

# Check for franka_control
if ! rospack find franka_control > /dev/null 2>&1; then
    echo "Error: franka_control package not found"
    echo "Please install: sudo apt install ros-melodic-franka-ros"
    exit 1
fi

# Check for panda_moveit_config
#if ! rospack find panda_moveit_config > /dev/null 2>&1; then
#    echo "Error: panda_moveit_config package not found"
#    echo "Please install: sudo apt install ros-melodic-panda-moveit-config"
#    exit 1
#fi

# Check for moveit_commander
#if ! python3 -c "import moveit_commander" 2>/dev/null; then
#    echo "Error: moveit_commander Python package not found"
#    echo "Please install: sudo apt install ros-melodic-moveit"
#    exit 1
#fi

echo "All required packages found!"

# Get robot IP from user if not set
if [ -z "$FRANKA_IP" ]; then
    read -p "Enter Franka robot IP address (default: 172.16.0.2): " FRANKA_IP
    FRANKA_IP=${FRANKA_IP:-172.16.0.2}
fi

echo "Using robot IP: $FRANKA_IP"

# Launch franka_control
echo "Starting franka_control..."
roslaunch franka_control franka_control.launch robot_ip:=$FRANKA_IP &
FRANKA_PID=$!

# Wait for franka_control to start
echo "Waiting for franka_control to start..."
sleep 10

# Launch MoveIt
echo "Starting MoveIt move_group..."
roslaunch panda_moveit_config move_group.launch &
MOVEIT_PID=$!

# Wait for MoveIt to start
echo "Waiting for MoveIt to start..."
sleep 10

# Start the gRPC service
echo "Starting MoveIt-based gRPC homing service..."
python3 moveit_grpc_service.py &
GRPC_PID=$!

echo ""
echo "MoveIt-based homing system is running!"
echo "Robot IP: $FRANKA_IP"
echo "gRPC Service: localhost:50051"
echo ""
echo "Process IDs:"
echo "  franka_control: $FRANKA_PID"
echo "  move_group: $MOVEIT_PID"
echo "  gRPC service: $GRPC_PID"
echo ""
echo "To test the system:"
echo "  python3 test_moveit_homing.py"
echo ""
echo "To stop the system, press Ctrl+C"

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "Shutting down MoveIt-based homing system..."
    kill $GRPC_PID 2>/dev/null
    kill $MOVEIT_PID 2>/dev/null
    kill $FRANKA_PID 2>/dev/null
    echo "System stopped."
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Wait for processes
wait
