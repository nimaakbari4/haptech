#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

NO_BUILD=false
REGEN=false
CI_MODE=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-build) NO_BUILD=true; shift;;
    --regen) REGEN=true; shift;;
    --ci) CI_MODE=true; shift;;
    *) echo "Unknown flag: $1"; exit 2;;
  esac
done

export DEBIAN_FRONTEND=noninteractive || true
if command -v apt-get >/dev/null 2>&1; then
  echo "[bootstrap] Installing packages via apt"
  sudo apt-get update
  
  # Try to install packages with fallbacks for different Ubuntu versions
  echo "[bootstrap] Installing core build tools..."
  sudo apt-get install -y \
    git curl ca-certificates zip unzip tar \
    build-essential cmake ninja-build pkg-config \
    protobuf-compiler libprotobuf-dev \
    libgrpc++-dev \
    libeigen3-dev libglew-dev libglfw3-dev libusb-1.0-0-dev \
    mesa-common-dev libglu1-mesa-dev \
    python3-venv python3-pip \
    libpoco-dev libssl-dev libcrypto++-dev \
    libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev \
    libasound2-dev libpulse-dev \
    protobuf-compiler-grpc || true
  
  # Try to install abseil (package name varies by Ubuntu version)
  echo "[bootstrap] Installing abseil..."
  if sudo apt-get install -y abseil-cpp-dev 2>/dev/null; then
    echo "abseil-cpp-dev installed successfully"
  elif sudo apt-get install -y libabsl-dev 2>/dev/null; then
    echo "libabsl-dev installed successfully"
  else
    echo "Warning: abseil not found in system packages, will use vcpkg version"
  fi
  
  # Try to install grpc-proto (package name varies by Ubuntu version)
  echo "[bootstrap] Installing gRPC proto..."
  if sudo apt-get install -y grpc-proto 2>/dev/null; then
    echo "grpc-proto installed successfully"
  elif sudo apt-get install -y libgrpc-dev 2>/dev/null; then
    echo "libgrpc-dev installed successfully"
  else
    echo "Warning: gRPC proto not found in system packages, will use vcpkg version"
  fi

  # Ensure grpc_cpp_plugin is available (apt first, then vcpkg fallback)
  echo "[bootstrap] Ensuring grpc_cpp_plugin is available..."
  if ! command -v grpc_cpp_plugin >/dev/null 2>&1; then
    if sudo apt-get install -y protobuf-compiler-grpc 2>/dev/null; then
      echo "protobuf-compiler-grpc installed successfully"
    else
      echo "protobuf-compiler-grpc not available; installing gRPC via vcpkg to obtain plugin"
      echo "[bootstrap] vcpkg (ensure bootstrapped)"
      if [[ ! -d "$ROOT_DIR/.vcpkg" ]]; then
        git -C "$ROOT_DIR" clone https://github.com/microsoft/vcpkg.git .vcpkg
        "$ROOT_DIR/.vcpkg/bootstrap-vcpkg.sh"
      fi
      "$ROOT_DIR/.vcpkg/vcpkg" install grpc
      VCPKG_PLUGIN="$ROOT_DIR/.vcpkg/installed/x64-linux/tools/grpc/grpc_cpp_plugin"
      if [[ -x "$VCPKG_PLUGIN" ]]; then
        export PATH="$ROOT_DIR/.vcpkg/installed/x64-linux/tools/grpc:$PATH"
        echo "Using vcpkg grpc_cpp_plugin at $VCPKG_PLUGIN"
      else
        echo "Warning: grpc_cpp_plugin still not found after vcpkg install" >&2
      fi
    fi
  fi
  
else
  echo "[bootstrap] Please install prerequisites for your distro (apt, dnf, yum)." >&2
fi

if [[ -f "$ROOT_DIR/.gitmodules" ]]; then
  echo "[bootstrap] Submodules"
  git -C "$ROOT_DIR" submodule sync --recursive
  git -C "$ROOT_DIR" submodule update --init --recursive
else
  echo "[bootstrap] No .gitmodules found; skipping submodule update"
fi

echo "[bootstrap] vcpkg (manifest mode)"
if [[ ! -d "$ROOT_DIR/.vcpkg" ]]; then
  git -C "$ROOT_DIR" clone https://github.com/microsoft/vcpkg.git .vcpkg
  "$ROOT_DIR/.vcpkg/bootstrap-vcpkg.sh"
fi

echo "[bootstrap] Python venv + requirements"
PY=python3
"$PY" -m venv "$ROOT_DIR/.venv"
source "$ROOT_DIR/.venv/bin/activate"
pip install -U pip
pip install -r "$ROOT_DIR/python_client/requirements.txt"

echo "[bootstrap] Verify protoc and grpc_cpp_plugin"
protoc --version
which grpc_cpp_plugin || echo "(Note) grpc_cpp_plugin path not required when using system libgrpc++-dev"

# Auto-regenerate C++ protobuf if generated headers are from a newer protoc
if grep -q 'google/protobuf/runtime_version.h' "$ROOT_DIR/core/src/franka_trial.pb.h" 2>/dev/null; then
  echo "[bootstrap] Detected stale protobuf headers (runtime_version.h). Regenerating C++ stubs..."
  protoc --proto_path="$ROOT_DIR/proto" \
    --cpp_out="$ROOT_DIR/core/src" \
    --grpc_out="$ROOT_DIR/core/src" \
    --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin || echo grpc_cpp_plugin)" \
    "$ROOT_DIR/proto/franka_trial.proto"
fi

if $REGEN; then
  echo "[bootstrap] Regenerating protobuf/gRPC (C++ & Python)"
  protoc --proto_path="$ROOT_DIR/proto" \
    --cpp_out="$ROOT_DIR/core/src" \
    --grpc_out="$ROOT_DIR/core/src" \
    --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin || echo grpc_cpp_plugin)" \
    "$ROOT_DIR/proto/franka_trial.proto"

  python -m grpc_tools.protoc -I "$ROOT_DIR/proto" \
    --python_out="$ROOT_DIR/python_client" \
    --grpc_python_out="$ROOT_DIR/python_client" \
    "$ROOT_DIR/proto/franka_trial.proto"
fi

if ! $NO_BUILD; then
  echo "[bootstrap] Building core"
  make -C "$ROOT_DIR/core"
  
  echo "[bootstrap] Installing Franka library system-wide"
  if [[ -f "$ROOT_DIR/core/external/chai3d-Franka-integration/build/external/libfranka/libfranka.so.0.13.3" ]]; then
    sudo cp "$ROOT_DIR/core/external/chai3d-Franka-integration/build/external/libfranka/libfranka.so.0.13.3" /usr/local/lib/
    sudo ldconfig
    echo "Franka library installed successfully"
  else
    echo "Warning: Franka library not found, you may need to set LD_LIBRARY_PATH manually"
  fi
fi

echo "[bootstrap] Done. Next steps:"
echo "  1) Run server:   (cd core/bin && ./core)"
echo "  2) Run client:   source .venv/bin/activate && python3 python_client/main.py"

