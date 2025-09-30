#!/bin/bash

# Dependency installation and update script for messaging_system
# This script handles both external dependencies via vcpkg and library system updates

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Set script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Function to print colored messages
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to check command availability
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Library system definitions with their GitHub repositories
LIBRARY_SYSTEMS="
thread_system|https://github.com/kcenon/thread_system.git
logger_system|https://github.com/kcenon/logger_system.git
monitoring_system|https://github.com/kcenon/monitoring_system.git
container_system|https://github.com/kcenon/container_system.git
database_system|https://github.com/kcenon/database_system.git
network_system|https://github.com/kcenon/network_system.git
"

# Function to update or clone library system
update_library_system() {
    local lib_name=$1
    local lib_url=$2
    local lib_path="libraries/$lib_name"

    print_message "$YELLOW" "Processing $lib_name..."

    # Check if library exists as part of the project
    if [ -d "$lib_path" ]; then
        if [ -d "$lib_path/.git" ]; then
            # It's a git repository, update it
            print_message "$GREEN" "  Updating $lib_name from git..."
            cd "$lib_path"
            git fetch origin
            git pull origin main || git pull origin master
            cd "$SCRIPT_DIR"
        else
            # It's part of the main project
            print_message "$GREEN" "  $lib_name is integrated in the project"
        fi
    else
        # Library doesn't exist, clone it
        print_message "$YELLOW" "  Cloning $lib_name..."
        git clone "$lib_url" "$lib_path"
    fi

    # Check for library-specific dependency script
    if [ -f "$lib_path/dependency.sh" ]; then
        print_message "$YELLOW" "  Installing $lib_name dependencies..."
        cd "$lib_path"
        bash dependency.sh
        cd "$SCRIPT_DIR"
    fi
}

# Function to install vcpkg if needed
install_vcpkg() {
    VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

    if [ ! -d "$VCPKG_ROOT" ]; then
        print_message "$YELLOW" "vcpkg not found at $VCPKG_ROOT"
        print_message "$YELLOW" "Installing vcpkg..."

        cd "$HOME"
        git clone https://github.com/Microsoft/vcpkg.git
        cd vcpkg

        if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
            ./bootstrap-vcpkg.sh
        elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
            ./bootstrap-vcpkg.bat
        else
            print_message "$RED" "Unsupported OS: $OSTYPE"
            exit 1
        fi

        cd "$SCRIPT_DIR"
    else
        print_message "$GREEN" "vcpkg found at $VCPKG_ROOT"
    fi
}

# Function to install external dependencies
install_external_dependencies() {
    print_message "$YELLOW" "\n📦 Installing external dependencies via vcpkg..."

    VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
    VCPKG_CMD="$VCPKG_ROOT/vcpkg"

    if [ ! -f "$VCPKG_CMD" ]; then
        print_message "$RED" "vcpkg executable not found at $VCPKG_CMD"
        exit 1
    fi

    # List of dependencies
    DEPENDENCIES=(
        "fmt"
        "gtest"
        "benchmark"
        "spdlog"
        "libpqxx"
        "asio"
        "nlohmann-json"
    )

    # Install each dependency
    for dep in "${DEPENDENCIES[@]}"; do
        print_message "$YELLOW" "  Installing $dep..."
        "$VCPKG_CMD" install "$dep" || print_message "$RED" "  Failed to install $dep (continuing anyway)"
    done
}

# Function to update library systems
update_library_systems() {
    print_message "$YELLOW" "\n📚 Updating library systems..."

    # Create libraries directory if it doesn't exist
    mkdir -p libraries

    # Update each library system
    echo "$LIBRARY_SYSTEMS" | while IFS='|' read -r lib_name lib_url; do
        # Skip empty lines
        [ -z "$lib_name" ] && continue
        update_library_system "$lib_name" "$lib_url"
    done
}

# Function to build the project
build_project() {
    print_message "$YELLOW" "\n🔨 Building the project..."

    # Create build directory
    mkdir -p build
    cd build

    # Configure with CMake
    print_message "$YELLOW" "  Configuring with CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release

    # Build
    print_message "$YELLOW" "  Building..."
    if command_exists nproc; then
        make -j$(nproc)
    elif command_exists sysctl; then
        make -j$(sysctl -n hw.ncpu)
    else
        make
    fi

    cd "$SCRIPT_DIR"
    print_message "$GREEN" "  Build completed!"
}

# Main script execution
main() {
    print_message "$GREEN" "=========================================="
    print_message "$GREEN" "Messaging System Dependency Manager"
    print_message "$GREEN" "=========================================="

    # Parse command line arguments
    case "${1:-all}" in
        vcpkg)
            install_vcpkg
            ;;
        deps|dependencies)
            install_vcpkg
            install_external_dependencies
            ;;
        libs|libraries)
            update_library_systems
            ;;
        build)
            build_project
            ;;
        all)
            install_vcpkg
            install_external_dependencies
            update_library_systems
            print_message "$GREEN" "\n✅ All dependencies installed and libraries updated successfully!"
            ;;
        help|--help|-h)
            echo "Usage: $0 [command]"
            echo ""
            echo "Commands:"
            echo "  vcpkg       - Install vcpkg package manager"
            echo "  deps        - Install external dependencies via vcpkg"
            echo "  libs        - Update/sync library systems"
            echo "  build       - Build the project"
            echo "  all         - Run all steps (default)"
            echo "  help        - Show this help message"
            ;;
        *)
            print_message "$RED" "Unknown command: $1"
            echo "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"