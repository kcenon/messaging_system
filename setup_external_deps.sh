#!/bin/bash

# Setup script for external modular dependencies
# This script sets up paths for the improved modular systems

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THREAD_SYSTEM_ROOT="${SCRIPT_DIR}/../thread_system/modular_structure"
LOGGER_SYSTEM_ROOT="${SCRIPT_DIR}/../logger_system"
MONITORING_SYSTEM_ROOT="${SCRIPT_DIR}/../monitoring_system"

# Check if modular thread_system exists
if [ ! -d "$THREAD_SYSTEM_ROOT" ]; then
    echo "Error: Modular thread_system not found at $THREAD_SYSTEM_ROOT"
    echo "Please ensure the improved thread_system is available at ../thread_system/modular_structure"
    exit 1
fi

# Check if logger_system exists
if [ ! -d "$LOGGER_SYSTEM_ROOT" ]; then
    echo "Error: Logger system not found at $LOGGER_SYSTEM_ROOT"
    echo "Please ensure the logger_system is available at ../logger_system"
    exit 1
fi

# Check if monitoring_system exists
if [ ! -d "$MONITORING_SYSTEM_ROOT" ]; then
    echo "Error: Monitoring system not found at $MONITORING_SYSTEM_ROOT"
    echo "Please ensure the monitoring_system is available at ../monitoring_system"
    exit 1
fi

# Build modular thread_system if not already built
if [ ! -d "$THREAD_SYSTEM_ROOT/core/build" ]; then
    echo "Building modular thread_system..."
    cd "$THREAD_SYSTEM_ROOT/core"
    
    # Create and run build
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
    make install
    
    cd "$SCRIPT_DIR"
fi

# Build logger_system if not already built
if [ ! -d "$LOGGER_SYSTEM_ROOT/build" ]; then
    echo "Building logger_system..."
    cd "$LOGGER_SYSTEM_ROOT"
    
    # Set up vcpkg if needed
    if [ -f dependency.sh ]; then
        ./dependency.sh
    fi
    
    ./build.sh
    cd "$SCRIPT_DIR"
fi

# Build monitoring_system if not already built
if [ ! -d "$MONITORING_SYSTEM_ROOT/build" ]; then
    echo "Building monitoring_system..."
    cd "$MONITORING_SYSTEM_ROOT"
    
    # Set up vcpkg if needed
    if [ -f dependency.sh ]; then
        ./dependency.sh
    fi
    
    ./build.sh
    cd "$SCRIPT_DIR"
fi

# Export CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH="$THREAD_SYSTEM_ROOT/core/build/install:$LOGGER_SYSTEM_ROOT/build/install:$MONITORING_SYSTEM_ROOT/build/install:$CMAKE_PREFIX_PATH"

echo "External dependencies configured:"
echo "  ThreadSystemCore: $THREAD_SYSTEM_ROOT/core/build/install"
echo "  LoggerSystem: $LOGGER_SYSTEM_ROOT/build/install"
echo "  MonitoringSystem: $MONITORING_SYSTEM_ROOT/build/install"
echo ""
echo "To build messaging_system with external modular dependencies:"
echo "  mkdir -p build && cd build"
echo "  cmake .. -DUSE_INTERNAL_THREAD_SYSTEM=OFF -DUSE_EXTERNAL_LOGGER_SYSTEM=ON -DUSE_EXTERNAL_MONITORING_SYSTEM=ON \\"
echo "    -DThreadSystemCore_ROOT=$THREAD_SYSTEM_ROOT/core/build/install \\"
echo "    -DLoggerSystem_ROOT=$LOGGER_SYSTEM_ROOT/build/install \\"
echo "    -DMonitoringSystem_ROOT=$MONITORING_SYSTEM_ROOT/build/install"
echo "  make"