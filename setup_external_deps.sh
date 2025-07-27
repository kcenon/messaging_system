#!/bin/bash

# Setup script for external modular dependencies
# This script sets up paths for the improved modular systems

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULAR_ROOT="${SCRIPT_DIR}/../thread_system/modular_structure"

# Check if modular thread_system exists
if [ ! -d "$MODULAR_ROOT" ]; then
    echo "Error: Modular thread_system not found at $MODULAR_ROOT"
    echo "Please ensure the improved thread_system is available at ../thread_system/modular_structure"
    exit 1
fi

# Build modular thread_system if not already built
if [ ! -d "$MODULAR_ROOT/build" ]; then
    echo "Building modular thread_system..."
    cd "$MODULAR_ROOT"
    ./build.sh
    cd "$SCRIPT_DIR"
fi

# Export CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH="$MODULAR_ROOT/build/install:$CMAKE_PREFIX_PATH"

echo "External dependencies configured:"
echo "  ThreadSystemCore: $MODULAR_ROOT/build/install"
echo ""
echo "To build messaging_system with external modular dependencies:"
echo "  mkdir -p build && cd build"
echo "  cmake .. -DUSE_INTERNAL_THREAD_SYSTEM=OFF -DThreadSystemCore_ROOT=$MODULAR_ROOT/build/install"
echo "  make"