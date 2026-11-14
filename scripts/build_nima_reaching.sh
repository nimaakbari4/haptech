#!/usr/bin/env bash
set -euo pipefail
PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
make -C "$PROJECT_DIR/nima_reaching" test
make -C "$PROJECT_DIR/nima_reaching"
