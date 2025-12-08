# Installation

Complete setup instructions for the haptic system.

## System Requirements

### Supported Platforms
- **Linux**: Ubuntu 22.04/24.04 (recommended)
- **macOS**: Supported but may have stability issues
- **Windows**: Via WSL2 with Ubuntu (native Windows not supported)

### Hardware Requirements
- **CPU**: Multi-core processor (Intel/AMD), minimum 4 cores recommended
- **RAM**: 8GB minimum, 16GB recommended for complex experiments
- **GPU**: OpenGL 3.3+ support (integrated graphics acceptable for basic trials)
- **Storage**: 10GB free space for system and experiment data
- **Haptic Device**: 
  - Franka Emika robot arm (primary support)
  - Force Dimension devices (Omega, Delta series)
  - Other devices may work with custom drivers

### Software Dependencies
The following will be installed automatically:
- **C++ Toolchain**: GCC/Clang compiler, Make, CMake
- **Graphics**: OpenGL development libraries, X11 (Linux)
- **Networking**: Protobuf, gRPC for client-server communication
- **Python**: Python 3.8+ with required packages

## Quick Installation

### One-Command Setup

For most users, this single command will set up everything:

```bash
make bootstrap
```

**What this does**:
1. Installs system prerequisites (compilers, libraries)
2. Downloads and builds dependencies (CHAI3D, Protobuf, gRPC)
3. Sets up Python virtual environment
4. Compiles the haptic core server
5. Generates communication protocols
6. Verifies installation

**Expected time**: 15-30 minutes depending on your system and internet connection.

## Detailed Installation Steps

If you prefer to understand each step or need to troubleshoot:

### Step 1: Clone Repository

```bash
git clone https://github.com/Haptech-Capstone/haptech.git
cd haptech
```

### Step 2: Initialize Submodules

```bash
git submodule update --init --recursive
```

This downloads required libraries (CHAI3D, Franka integration, etc.)

### Step 3: Install System Dependencies

**Ubuntu/Debian**:
```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev libx11-dev libxrandr-dev
sudo apt install -y libxi-dev libxinerama-dev libxcursor-dev
sudo apt install -y protobuf-compiler libprotobuf-dev
sudo apt install -y python3 python3-pip python3-venv
```

**macOS** (using Homebrew):
```bash
brew install cmake protobuf grpc
brew install python3
```

### Step 4: Build Core Server

```bash
cd core
make
```

This compiles the C++ haptic server. Look for:
```
✓ Successfully built core/bin/core
```

### Step 5: Setup Python Environment

```bash
cd ..
python3 -m venv .venv
source .venv/bin/activate
pip install -r python_client/requirements.txt
```

### Step 6: Generate Protocol Files

```bash
cd proto
./compile_commands.txt
cd ..
```

This creates the communication files needed for client-server interaction.

## Platform-Specific Instructions

### Ubuntu 24.04 LTS (Recommended)

Ubuntu 24.04 is the primary development platform and should work smoothly:

```bash
git clone https://github.com/Haptech-Capstone/haptech.git
cd haptech
make bootstrap
```

### Ubuntu 22.04 LTS

Slightly older but well-supported:

```bash
# May need newer CMake
sudo apt install -y software-properties-common
sudo add-apt-repository ppa:kitware/cmake
sudo apt update
sudo apt install -y cmake

# Then proceed with normal installation
make bootstrap
```

### macOS (Intel/Apple Silicon)

**Prerequisites**:
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Xcode command line tools
xcode-select --install
```

**Installation**:
```bash
git clone https://github.com/Haptech-Capstone/haptech.git
cd haptech
make bootstrap
```

**Known issues**:
- Core server may crash on startup (being investigated)
- Some haptic devices may not work properly
- Performance may be lower than Linux

### Windows via WSL2

**Setup WSL2**:
1. Enable WSL2 in Windows Features
2. Install Ubuntu from Microsoft Store
3. Launch Ubuntu terminal

**In Ubuntu WSL**:
```bash
# Update package lists
sudo apt update
sudo apt upgrade -y

# Install git if not present
sudo apt install -y git

# Clone and install
git clone https://github.com/Haptech-Capstone/haptech.git
cd haptech
make bootstrap
```

**WSL considerations**:
- X11 forwarding needed for GUI (install VcXsrv or similar)
- USB haptic devices require USB/IP forwarding
- Performance may be slightly reduced

## Verification

### Test Core Server

Verify the haptic server starts correctly:

```bash
cd core/bin
./core
```

You should see:
```
[INFO] Haptic Core Server starting...
[INFO] gRPC server listening on 0.0.0.0:50051
[INFO] Haptic device initialized
[INFO] Ready for trials
```

**Stop the server**: Press `Ctrl+C`

### Test Python Client

Verify the Python client can connect:

```bash
source .venv/bin/activate
cd python_client
python3 main.py
```

This should prompt for trial configuration. Type `example_config.xml` to test.

### Test Communication

**In one terminal (server)**:
```bash
cd core/bin
./core
```

**In another terminal (client)**:
```bash
source .venv/bin/activate
python3 python_client/main.py
```

Enter a test configuration file (e.g., `builtin_objects_test.xml`) and verify the trial runs.

## Configuration

### Client Settings

Edit `python_client/settings.json` to configure:

```json
{
    "config_path": "config",          // Where XML files are stored
    "results_path": "results",       // Where data is saved
    "server_host": "localhost",      // Server IP address
    "server_port": 50051,            // Server port
    "trial_subject": "",              // Default subject name
    "last_configuration_file": ""     // Last used config
}
```

### Server Settings

Server configuration is handled through command-line arguments and XML files. No separate configuration file is needed.

### Haptic Device Setup

**Franka Robot**:
- Ensure robot is in FCI mode
- Network configuration must allow communication
- Safety parameters should be configured

**Force Dimension Devices**:
- Install Force Dimension SDK
- Verify device drivers
- Test device in vendor software first

## Troubleshooting Installation

### Common Issues

**"Command not found: make"**:
```bash
sudo apt install -y build-essential
```

**"No module named grpc"**:
```bash
source .venv/bin/activate
pip install --upgrade -r python_client/requirements.txt
```

**"OpenGL not found"**:
```bash
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev
```

**"Permission denied" when running core**:
```bash
chmod +x core/bin/core
```

**Core server fails to start**:
1. Check if port 50051 is already in use:
   ```bash
   lsof -i :50051
   ```
2. Verify haptic device is connected and drivers are installed
3. Check console output for specific error messages

### Getting Help

1. **Check the logs**: Look for error messages in terminal output
2. **Verify dependencies**: Ensure all required packages are installed
3. **Test components separately**: Try server and client individually
4. **Check hardware**: Verify haptic device works with vendor software
5. **Review documentation**: Check [Troubleshooting](../troubleshooting/README.md) guide

### System-Specific Troubleshooting

**macOS Big Sur/Monterey/Ventura**:
- May need to approve security exceptions for compiled binaries
- Some haptic device drivers require specific macOS versions

**Ubuntu on ARM (Raspberry Pi, etc.)**:
- Compilation may take much longer
- Some dependencies may need manual compilation
- Performance will be significantly reduced

**WSL2**:
- Ensure WSL2 (not WSL1) is being used: `wsl -l -v`
- USB device forwarding may require additional setup
- X11 forwarding needed for graphics: install VcXsrv or X410

## Next Steps

Once installation is complete:
1. Continue to [First Trial](first-trial.md) to run a test experiment
2. Read [Basic Concepts](basic-concepts.md) to understand the system
3. Explore [XML Configuration](../xml-configuration/README.md) to create your own trials

The system is now ready for haptic experiments!