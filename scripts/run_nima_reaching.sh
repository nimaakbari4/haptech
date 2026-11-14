#!/usr/bin/env bash
set -euo pipefail
PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
export LIBGL_ALWAYS_SOFTWARE="${LIBGL_ALWAYS_SOFTWARE:-1}"
export LD_LIBRARY_PATH="$PROJECT_DIR/core/external/chai3d-Franka-integration/build/external/libfranka:/opt/openrobots/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$PROJECT_DIR"
exec "$PROJECT_DIR/core/bin/core" "$@"
