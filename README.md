# haptech
The core functionality of the Haptech project.

## Building the Core

### One‑command setup (macOS/Linux)

```bash
make bootstrap
```

This installs prerequisites, initializes submodules, bootstraps vcpkg, sets up a Python venv, optionally regenerates protobuf/gRPC stubs, and builds `core/bin/core`.

Flags:
- pass Make variable `ARGS` to the script, e.g.:
  - `make bootstrap ARGS="--no-build"`
  - `make bootstrap ARGS="--regen"`

Windows:
- Use PowerShell: `powershell -ExecutionPolicy Bypass -File scripts/bootstrap-windows.ps1`
- Or use WSL2 (Ubuntu) and run `scripts/bootstrap-linux.sh`.

### Manual build
```bash
git submodule update --init --recursive
cd core && make
```

### Run the Core Executable
```bash
cd core/bin
./core
```
