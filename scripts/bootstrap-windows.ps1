Param(
  [switch]$NoBuild,
  [switch]$Regen,
  [switch]$Ci
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

Write-Host "[bootstrap] Windows prerequisites via winget (or install manually)"
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
  Write-Warning "winget not found. Install Git, CMake, Ninja, Python 3, and Visual Studio Build Tools manually, or use WSL2."
} else {
  winget install -e --id Git.Git --silent | Out-Null
  winget install -e --id Kitware.CMake --silent | Out-Null
  winget install -e --id Ninja-build.Ninja --silent | Out-Null
  winget install -e --id Python.Python.3 --silent | Out-Null
}

Write-Host "[bootstrap] Submodules"
git -C $Root submodule update --init --recursive

Write-Host "[bootstrap] vcpkg (manifest mode)"
if (-not (Test-Path "$Root/.vcpkg")) {
  git -C $Root clone https://github.com/microsoft/vcpkg.git .vcpkg
  & "$Root/.vcpkg/bootstrap-vcpkg.bat"
}

Write-Host "[bootstrap] Python venv + requirements"
$py = "python"
& $py -m venv "$Root/.venv"
& "$Root/.venv/Scripts/Activate.ps1"
pip install -U pip
pip install -r "$Root/python_client/requirements.txt"

Write-Host "[bootstrap] Protobuf tools"
try { protoc --version } catch { Write-Warning "Install protoc or ensure it's on PATH (vcpkg/Visual Studio)." }
try { Get-Command grpc_cpp_plugin | Out-Null } catch { Write-Warning "grpc_cpp_plugin not found; ensure gRPC C++ installed (vcpkg)." }

if ($Regen) {
  Write-Host "[bootstrap] Regenerating protobuf/gRPC (C++ & Python)"
  protoc --proto_path="$Root/proto" `
    --cpp_out="$Root/core/src" `
    --grpc_out="$Root/core/src" `
    --plugin=protoc-gen-grpc="$(Get-Command grpc_cpp_plugin -ErrorAction SilentlyContinue)" `
    "$Root/proto/franka_trial.proto"

  python -m grpc_tools.protoc -I "$Root/proto" `
    --python_out="$Root/python_client" `
    --grpc_python_out="$Root/python_client" `
    "$Root/proto/franka_trial.proto"
}

if (-not $NoBuild) {
  Write-Host "[bootstrap] Building core (recommend WSL2 or provide a CMake preset for MSVC)"
  Write-Host "Run in WSL2 Ubuntu: ./scripts/bootstrap-linux.sh && make -C core"
}

Write-Host "[bootstrap] Done. Next steps:"
Write-Host "  1) Run server:   (cd core/bin && ./core)"
Write-Host "  2) Run client:   .venv\\Scripts\\Activate.ps1; python python_client/main.py"

