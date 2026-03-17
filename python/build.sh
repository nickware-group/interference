#!/usr/bin/env bash

OPENCL=OFF; VULKAN=OFF
for arg in "$@"; do
    case "$arg" in
        --opencl) OPENCL=ON ;;
        --vulkan) VULKAN=ON ;;
        *) echo "Usage: bash build.sh [--opencl] [--vulkan]"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/.venv"

if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
fi
source "$VENV_DIR/bin/activate"

pip install --upgrade pip setuptools wheel -q
export SKBUILD_CMAKE_ARGS="-DINDK_OPENCL_SUPPORT=$OPENCL;-DINDK_VULKAN_SUPPORT=$VULKAN"
pip install "$SCRIPT_DIR" -v

echo ""
echo "Test:"
echo "  source $VENV_DIR/bin/activate"
echo "  cd ./samples"
echo "  python -c \"import indk; print(indk.__version__)\""