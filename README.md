# haptech
The core functionality of the Haptech project.

## Supported platforms
- macOS (Sort of works, may be broken on core launch)
- Linux (Ubuntu 22.04/24.04)
- Windows via WSL2 (Ubuntu). Native Windows builds are not supported.

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
```bash
cd core/bin
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
