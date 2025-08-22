# haptech
The core functionality of the Haptech project.

## Building the Core

### 1. Initialize Submodules
Before building, make sure to initialize and update the required submodules:

```bash
git submodule update --init --recursive
```

### 2. Build the Project
Navigate to the `core` directory and run:

```bash
cd core
make
```

This will automatically build CHAI3D and its dependencies, then build the core executable in `core/bin/core`.

### 3. Run the Core Executable

```bash
cd bin
./core
```

### Notes
- Ensure you have development packages for system libraries (GL, X11, etc.) installed.
- By default, gRPC, protobuf, and Abseil are expected to be installed globally. For portability, consider integrating these as local dependencies.
