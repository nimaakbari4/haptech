# Build System

Comprehensive documentation of the Haptech build system, dependencies, and compilation process.

## Overview

The Haptech build system uses:
- **GNU Make** for orchestration
- **CMake** for CHAI3D and libfranka dependency builds
- **protoc** and **grpc_cpp_plugin** for code generation from `.proto` files
- **g++** (C++17) for compilation
- **Python virtualenv** for client environment isolation

## Dependencies

### System Dependencies

#### Linux (Ubuntu 22.04/24.04)
- **Build Tools**: `build-essential`, `cmake`, `pkg-config`
- **Graphics**: `libgl1-mesa-dev`, `libglu1-mesa-dev`, `libglew-dev`
- **Window System**: `libglfw3-dev`, `libx11-dev`, `libxcursor-dev`, `libxrandr-dev`, `libxinerama-dev`
- **USB**: `libusb-1.0-0-dev`
- **Networking**: `libssl-dev`
- **Math**: `libeigen3-dev`
- **Protobuf/gRPC**: `protobuf-compiler`, `libprotobuf-dev`, `libgrpc++-dev`, `libgrpc-dev`, `protobuf-compiler-grpc`
- **Poco**: `libpoco-dev`
- **Abseil**: `libabsl-dev` (for gRPC support)

#### macOS (via Homebrew)
- **Build Tools**: Xcode Command Line Tools, `cmake`, `pkg-config`
- **Graphics**: `glew`, `glfw`
- **Dependencies**: `eigen`, `protobuf`, `grpc`, `poco`, `openssl`

### Git Submodules
- **chai3d-Franka-integration**: CHAI3D haptics library with Franka robot integration
- **libfranka**: Franka Emika robot control library (embedded in CHAI3D submodule)

### Python Dependencies
Managed via virtualenv in `.venv/`:
- `grpcio`, `grpcio-tools` - gRPC runtime and code generation
- `protobuf` - Protocol buffer support
- Additional packages listed in `python_client/requirements.txt`

## Build Process

### Bootstrap Command

```bash
make bootstrap
```

This single command:
1. Detects platform (Linux/macOS)
2. Installs system dependencies via package manager
3. Initializes git submodules recursively
4. Creates Python virtualenv at `.venv/`
5. Installs Python dependencies
6. Verifies/installs `protoc` and `grpc_cpp_plugin`
7. Generates C++ and Python code from `proto/franka_trial.proto`
8. Builds CHAI3D via CMake
9. Compiles core C++ application

### Build Flags

```bash
make bootstrap ARGS="--no-build"    # Setup only, skip compilation
make bootstrap ARGS="--regen"       # Force regenerate protobuf files
```

### Debug Build

```bash
cd core
make DEBUG=1
```

Enables:
- Debug symbols (`-g`)
- No optimization (`-O0`)
- Frame pointer preservation (`-fno-omit-frame-pointer`)
- `DEBUG` preprocessor macro

## Core Makefile

Location: `core/Makefile`

### Key Variables

```makefile
CHAI3D_DIR = external/chai3d-Franka-integration
SRC_DIR = src
BIN_DIR = bin
CXX = g++
CXXFLAGS = -std=c++17 -DLINUX ...
```

### Compiler Flags

- **Standard**: `-std=c++17`
- **Warnings**: `-Wno-deprecated`
- **Architecture**: `-m64 -fsigned-char`
- **Defines**: `-DLINUX` (Linux only)
- **Include Paths**:
  - CHAI3D source and headers
  - Eigen, GLEW, GLFW
  - libfranka include directories
  - System includes (`/usr/include`, `/usr/local/include`)
  - Project source directories

### Linker Configuration

**Static Libraries**:
- `libchai3d.a` - CHAI3D haptics framework
- `libglfw.a` - Window and input handling

**Shared Libraries**:
- `libfranka.so.0.13.3` - Franka robot control
- `libgrpc++`, `libgrpc`, `libgpr` - gRPC runtime
- `libprotobuf` - Protocol buffer runtime
- `libabsl_synchronization` - Abseil synchronization (required by gRPC)
- `libPocoFoundation`, `libPocoNet`, `libPocoNetSSL`, `libPocoCrypto` - Poco libraries
- OpenGL: `libGL`, `libGLU`
- X11: `libX11`, `libXcursor`, `libXrandr`, `libXinerama`
- System: `libpthread`, `librt`, `libdl`, `libssl`, `libcrypto`, `libz`, `libcares`, `libusb-1.0`

### Platform-Specific Handling

**macOS Adjustments**:
- Add Homebrew include/lib paths (`/opt/homebrew`)
- Enable deprecated C++17 binders for old Eigen: `-D_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS`
- Remove Linux-specific libraries (X11, librt)
- Link macOS frameworks: `Cocoa`, `IOKit`, `CoreVideo`, `OpenGL`, etc.

### Build Targets

- **all** (default): Generate protobuf code, build CHAI3D, compile core
- **cmake_chai3d_build**: Build CHAI3D library via CMake
- **generate_proto**: Generate C++ stubs from `.proto` file
- **clean**: Remove `bin/` directory and build artifacts
- **proto_clean**: Remove generated protobuf C++ files

### Object File Compilation

Sources are organized in:
- `src/*.cpp` → `bin/*.o`
- `src/objects/*.cpp` → `bin/objects/*.o`
- `src/trial_conditions/*.cpp` → `bin/trial_conditions/*.o`
- Generated protobuf files → `bin/franka_trial.pb.o`, `bin/franka_trial.grpc.pb.o`

### Final Linking

```makefile
$(TARGET): $(OBJECTS)
    $(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)
```

Links all object files with libraries to produce `bin/core` executable.

## Protocol Buffer Code Generation

### C++ Generation

```bash
protoc --cpp_out=core/src --grpc_out=core/src \
    --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
    proto/franka_trial.proto
```

Generates:
- `core/src/franka_trial.pb.h`, `core/src/franka_trial.pb.cc` - Message definitions
- `core/src/franka_trial.grpc.pb.h`, `core/src/franka_trial.grpc.pb.cc` - Service stubs

### Python Generation

```bash
python -m grpc_tools.protoc -I. \
    --python_out=python_client \
    --grpc_python_out=python_client \
    proto/franka_trial.proto
```

Generates:
- `python_client/franka_trial_pb2.py` - Message classes
- `python_client/franka_trial_pb2_grpc.py` - Service stubs

## Common Build Issues

### Linker Errors

**Problem**: `undefined reference to symbol '_ZN4absl...'`
**Solution**: Add `-labsl_synchronization` to `LDLIBS` in Makefile

**Problem**: `DSO missing from command line`
**Solution**: Ensure all required libraries are explicitly linked

### GLFW Initialization Failure

**Problem**: `Error: X11: Failed to open display`
**Solution** (WSL2): Install VcXsrv on Windows, set `DISPLAY` variable

### Protobuf Version Mismatch

**Problem**: `generated_message_table_driven.h: No such file or directory`
**Solution**: Regenerate protobuf files matching installed protobuf version:
```bash
make bootstrap ARGS="--regen --no-build"
```

### Submodule Issues

**Problem**: Missing submodule files or fetch failures
**Solution**:
```bash
git submodule sync --recursive
git submodule update --init --recursive
```

## Performance Considerations

### Optimization Levels
- **Release**: Default compiler optimization (typically `-O2` or `-O3`)
- **Debug**: `-O0` for debugging, slower execution

### Parallel Compilation

```bash
make -j$(nproc)  # Use all CPU cores
```

### Incremental Builds

Makefile automatically detects changed files and recompiles only what's needed.

## Clean Builds

```bash
cd core
make clean       # Remove compiled objects and binaries
make proto_clean # Remove generated protobuf files
```

For complete rebuild:
```bash
cd core
make clean proto_clean
cd ..
make bootstrap
```