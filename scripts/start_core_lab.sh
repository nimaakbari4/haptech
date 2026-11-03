#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export LIBGL_ALWAYS_SOFTWARE=1
export LD_LIBRARY_PATH="$ROOT_DIR/core/external/chai3d-Franka-integration/build/external/libfranka:/opt/openrobots/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

cd "$ROOT_DIR"
exec "$ROOT_DIR/core/bin/core" "$@"
