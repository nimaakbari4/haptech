#!/bin/bash
# Simple alias script for quick core startup with CPU affinity
exec "$(dirname "$0")/run_core.sh" -c 5,7 "$@"
