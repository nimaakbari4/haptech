#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OS="$(uname -s)"

case "$OS" in
  Darwin)
    exec "$ROOT_DIR/scripts/bootstrap-macos.sh" "$@"
    ;;
  Linux)
    exec "$ROOT_DIR/scripts/bootstrap-linux.sh" "$@"
    ;;
  *)
    echo "Unsupported OS: $OS. On Windows, run scripts/bootstrap-windows.ps1 in PowerShell, or use WSL2 and run this script." >&2
    exit 1
    ;;
esac

