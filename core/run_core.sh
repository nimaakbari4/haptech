#!/bin/bash
# Wrapper script to run the core executable with proper library path
export LD_LIBRARY_PATH="$(dirname "$0")/external/chai3d-Franka-integration/build/external/libfranka:$LD_LIBRARY_PATH"
exec "$(dirname "$0")/bin/core" "$@"
