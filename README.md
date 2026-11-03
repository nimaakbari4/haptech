# haptech
The core functionality of the Haptech project.

## Supported platforms
- macOS (Sort of works, may be broken on core launch)
- Linux (Ubuntu 22.04/24.04)
- Windows via WSL2 (Ubuntu). Native Windows builds are not supported.

## Clone the repository

Clone the project together with its required submodules:

```bash
git clone --recurse-submodules https://github.com/nimaakbari4/haptech.git
cd haptech
```

If you already cloned the repository, initialize its submodules:

```bash
cd haptech
git submodule update --init --recursive
```

## Prerequisites for physical Franka FR3 operation

- A real-time Linux kernel and permission to use real-time scheduling.
  A generic kernel can be used for compilation.
- Robot firmware compatible with the pinned libfranka version, 0.21.3.
- Pinocchio and its dependencies installed before building.
  The build searches `/opt/openrobots` and `/usr`.
  The bootstrap script does not install Pinocchio.
- The correct end-effector profile configured in Franka Desk.

WSL2 instructions are for software development, not physical robot control.

The updated fork still requires a full build and physical robot verification.

## One‑command setup

```bash
make bootstrap
```

What this does:
- Installs system prerequisites (C/C++ toolchain, OpenGL/X11 dev, Protobuf, gRPC, Poco, etc.)
- Initializes git submodules
- Bootstraps vcpkg (manifest mode)
- Creates Python virtualenv and installs `python_client` requirements
- Verifies `protoc` and `grpc_cpp_plugin` (installs plugin or falls back to vcpkg as needed)
- Regenerates protobuf/gRPC C++ stubs if stale
- Builds `core/bin/core`

Flags (pass via `ARGS`):
- `make bootstrap ARGS="--no-build"`  # install/setup only
- `make bootstrap ARGS="--regen"`     # force regenerate protobuf/gRPC stubs (C++ + Python)

## Windows (via WSL2)
Use WSL2 with Ubuntu, then run the same command inside the distro:
```bash
make bootstrap
```

## Manual build
```bash
git submodule update --init --recursive
cd core && make
```

## Running
Server (C++):
(From root directory)
```bash
./scripts/start_core_lab.sh
```

Client (Python):
```bash
source .venv/bin/activate
python3 python_client/main.py
```

## Regenerate protobuf/gRPC
On demand:
```bash
make bootstrap ARGS="--regen --no-build"
```

## Troubleshooting
- If `protoc-gen-grpc`/`grpc_cpp_plugin` is missing, the bootstrap will install `protobuf-compiler-grpc` or use vcpkg to provide the plugin automatically.
- If the Python client can’t import `grpc`, make sure you’re using the project venv: `source .venv/bin/activate` then `pip install -r python_client/requirements.txt`.
- Eigen binder errors (e.g., `std::binder1st/binder2nd` not found) when compiling with C++17: ensure the system Eigen is used (installed by bootstrap) and avoid picking up any vendored/ancient Eigen from submodules.
- Protobuf header not found (e.g., `generated_message_table_driven.h`): your generated C++ stubs are out of sync with the installed Protobuf version. Regenerate with `make bootstrap ARGS="--regen --no-build"`.
- Submodules out of sync or fetch failures: run `git submodule sync --recursive && git submodule update --init --recursive`.
