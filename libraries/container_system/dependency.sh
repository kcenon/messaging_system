#!/bin/bash

# Container System Dependency Installation Script
# This script installs all required dependencies for the Container System module

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    # Detect Linux distribution
    if [ -f /etc/debian_version ]; then
        DISTRO="debian"
    elif [ -f /etc/redhat-release ]; then
        DISTRO="redhat"
    elif [ -f /etc/arch-release ]; then
        DISTRO="arch"
    else
        DISTRO="unknown"
    fi
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    OS="windows"
fi

# Function to print colored output
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check and install vcpkg
install_vcpkg() {
    print_step "Checking for vcpkg..."
    
    if command_exists vcpkg; then
        print_info "vcpkg is already installed"
        return 0
    fi
    
    # Check common vcpkg locations
    VCPKG_PATHS=(
        "$HOME/vcpkg"
        "/opt/vcpkg"
        "/usr/local/vcpkg"
        "$HOME/tools/vcpkg"
    )
    
    for path in "${VCPKG_PATHS[@]}"; do
        if [ -f "$path/vcpkg" ]; then
            print_info "Found vcpkg at $path"
            export PATH="$path:$PATH"
            return 0
        fi
    done
    
    print_warning "vcpkg not found. Would you like to install it? (y/n)"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        print_info "Installing vcpkg..."
        VCPKG_ROOT="$HOME/vcpkg"
        git clone https://github.com/Microsoft/vcpkg.git "$VCPKG_ROOT"
        cd "$VCPKG_ROOT"
        ./bootstrap-vcpkg.sh
        export PATH="$VCPKG_ROOT:$PATH"
        cd "$SCRIPT_DIR"
        print_info "vcpkg installed at $VCPKG_ROOT"
        print_warning "Add $VCPKG_ROOT to your PATH for future use"
    else
        print_error "vcpkg is required for dependency management"
        return 1
    fi
}

# Function to install system dependencies
install_system_deps() {
    print_step "Installing system dependencies..."
    
    case "$OS" in
        "macos")
            if ! command_exists brew; then
                print_error "Homebrew is required on macOS. Please install from https://brew.sh"
                exit 1
            fi
            
            print_info "Updating Homebrew..."
            brew update
            
            print_info "Installing dependencies..."
            brew install cmake ninja pkg-config
            
            # Optional tools
            if ! command_exists doxygen; then
                print_info "Installing documentation tools..."
                brew install doxygen graphviz
            fi
            ;;
            
        "linux")
            case "$DISTRO" in
                "debian")
                    print_info "Installing dependencies for Debian/Ubuntu..."
                    sudo apt-get update
                    sudo apt-get install -y \
                        build-essential \
                        cmake \
                        ninja-build \
                        pkg-config \
                        git
                    
                    # Optional tools
                    sudo apt-get install -y doxygen graphviz
                    ;;
                    
                "redhat")
                    print_info "Installing dependencies for RedHat/CentOS/Fedora..."
                    sudo yum install -y \
                        gcc \
                        gcc-c++ \
                        cmake \
                        ninja-build \
                        pkgconfig \
                        git
                    
                    # Optional tools
                    sudo yum install -y doxygen graphviz
                    ;;
                    
                "arch")
                    print_info "Installing dependencies for Arch Linux..."
                    sudo pacman -S --needed \
                        base-devel \
                        cmake \
                        ninja \
                        pkg-config \
                        git
                    
                    # Optional tools
                    sudo pacman -S --needed doxygen graphviz
                    ;;
                    
                *)
                    print_error "Unknown Linux distribution. Please install manually:"
                    print_info "  - C++ compiler (gcc/clang)"
                    print_info "  - CMake (>= 3.16)"
                    print_info "  - Ninja (optional)"
                    print_info "  - pkg-config"
                    exit 1
                    ;;
            esac
            ;;
            
        "windows")
            print_info "On Windows, please ensure you have:"
            print_info "  - Visual Studio 2019 or later with C++ workload"
            print_info "  - CMake (>= 3.16)"
            print_info "  - Git"
            print_info "  - vcpkg (will be checked next)"
            ;;
            
        *)
            print_error "Unknown operating system: $OS"
            exit 1
            ;;
    esac
}

# Function to install C++ dependencies via vcpkg
install_cpp_deps() {
    print_step "Installing C++ dependencies..."
    
    # Check if vcpkg.json exists
    if [ ! -f "vcpkg.json" ]; then
        print_warning "vcpkg.json not found. Creating default configuration..."
        cat > vcpkg.json << 'EOF'
{
    "name": "container-system",
    "description": "Advanced C++20 Container System with Thread-Safe Operations and Messaging Integration",
    "dependencies": [
        "fmt",
        "gtest",
        "benchmark"
    ]
}
EOF
    fi
    
    # Install dependencies
    print_info "Installing vcpkg dependencies..."
    if ! vcpkg install; then
        print_error "Failed to install vcpkg dependencies"
        return 1
    fi
    
    print_info "C++ dependencies installed successfully"
}

# Function to install test framework
install_test_framework() {
    print_step "Checking test frameworks..."
    
    # Check if already installed via vcpkg
    if vcpkg list | grep -q "gtest"; then
        print_info "Google Test is already installed via vcpkg"
    else
        print_warning "Installing Google Test..."
        vcpkg install gtest
    fi
    
    if vcpkg list | grep -q "benchmark"; then
        print_info "Google Benchmark is already installed via vcpkg"
    else
        print_warning "Installing Google Benchmark..."
        vcpkg install benchmark
    fi
}

# Main installation process
main() {
    print_info "Container System Dependency Installer"
    print_info "OS: $OS"
    if [ "$OS" = "linux" ]; then
        print_info "Distribution: $DISTRO"
    fi
    echo ""
    
    # Install system dependencies
    install_system_deps
    
    # Install vcpkg
    if ! install_vcpkg; then
        print_error "Failed to set up vcpkg"
        exit 1
    fi
    
    # Install C++ dependencies
    if ! install_cpp_deps; then
        print_error "Failed to install C++ dependencies"
        exit 1
    fi
    
    # Install test framework
    install_test_framework
    
    echo ""
    print_info "All dependencies installed successfully!"
    print_info "You can now build the project with ./build.sh"
    
    # Print vcpkg integration instructions
    echo ""
    print_step "For CMake integration, use one of these methods:"
    print_info "1. Set CMAKE_TOOLCHAIN_FILE:"
    print_info "   cmake -DCMAKE_TOOLCHAIN_FILE=$(vcpkg integrate install | grep -o '[^ ]*\.cmake') .."
    print_info "2. Or export VCPKG_ROOT:"
    print_info "   export VCPKG_ROOT=$(dirname $(which vcpkg))"
}

# Run main function
main