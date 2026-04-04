#!/usr/bin/env bash

OPENCL=OFF; VULKAN=OFF
for arg in "$@"; do
    case "$arg" in
        --opencl) OPENCL=ON ;;
        --vulkan) VULKAN=ON ;;
        *) echo "Usage: bash build.sh [--opencl] [--vulkan]"; exit 1 ;;
    esac
done

mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release -DINDK_OPENCL_SUPPORT=$OPENCL -DINDK_VULKAN_SUPPORT=$VULKAN ..
make install
