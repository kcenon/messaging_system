#!/bin/bash

# Test script for building messaging_system with modular dependencies

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_modular_test"

# Clean previous build
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Source setup script
source "$SCRIPT_DIR/setup_external_deps.sh"

# Configure with external modular thread_system
cd "$BUILD_DIR"
echo "Configuring messaging_system with external modular dependencies..."
cmake .. \
    -DUSE_INTERNAL_THREAD_SYSTEM=OFF \
    -DThreadSystemCore_ROOT="$MODULAR_ROOT/build/install" \
    -DBUILD_MESSAGING_SAMPLES=OFF \
    -DUSE_UNIT_TEST=OFF

# Build
echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

echo "Build completed successfully!"
echo "Build directory: $BUILD_DIR"