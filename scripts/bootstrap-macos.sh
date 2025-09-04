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

echo "[bootstrap] macOS prerequisites via Homebrew"
if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew not found. Installing..."
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  eval "$($(brew --prefix)/bin/brew shellenv)" || true
fi

brew bundle --file "$ROOT_DIR/Brewfile"

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
which grpc_cpp_plugin || { echo "grpc_cpp_plugin not found"; exit 1; }

if $REGEN; then
  echo "[bootstrap] Regenerating protobuf/gRPC (C++ & Python)"
  protoc --proto_path="$ROOT_DIR/proto" \
    --cpp_out="$ROOT_DIR/core/src" \
    --grpc_out="$ROOT_DIR/core/src" \
    --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" \
    "$ROOT_DIR/proto/franka_trial.proto"

  python -m grpc_tools.protoc -I "$ROOT_DIR/proto" \
    --python_out="$ROOT_DIR/python_client" \
    --grpc_python_out="$ROOT_DIR/python_client" \
    "$ROOT_DIR/proto/franka_trial.proto"
fi

if ! $NO_BUILD; then
  echo "[bootstrap] Building core"
  make -C "$ROOT_DIR/core"
fi

echo "[bootstrap] Done. Next steps:"
echo "  1) Run server:   (cd core/bin && ./core)"
echo "  2) Run client:   source .venv/bin/activate && python3 python_client/main.py"

