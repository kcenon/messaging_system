#!/bin/bash

# Dependency installation script for messaging_system

# Set script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Check if vcpkg is installed
VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

if [ ! -d "$VCPKG_ROOT" ]; then
    echo "vcpkg not found at $VCPKG_ROOT"
    echo "Installing vcpkg..."
    
    cd "$HOME"
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    
    if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
        ./bootstrap-vcpkg.sh
    else
        echo "Unsupported OS: $OSTYPE"
        exit 1
    fi
    
    cd "$SCRIPT_DIR"
fi

# Install dependencies
echo "Installing dependencies via vcpkg..."
"$VCPKG_ROOT/vcpkg" install \
    fmt \
    gtest \
    benchmark \
    spdlog \
    libpqxx \
    asio \
    nlohmann-json

echo "Dependencies installed successfully!"