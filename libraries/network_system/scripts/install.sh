#!/bin/bash

# Network System Installation Script
# Supports Linux, macOS, and Windows (via MSYS2)

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
INSTALL_PREFIX="/usr/local"
BUILD_TYPE="Release"
BUILD_TESTS=OFF
BUILD_SAMPLES=OFF
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Print colored message
print_msg() {
    echo -e "${GREEN}[NetworkSystem]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Show usage
show_help() {
    cat << EOF
Network System Installation Script

Usage: $0 [OPTIONS]

Options:
    -h, --help              Show this help message
    -p, --prefix PATH       Installation prefix (default: /usr/local)
    -t, --build-type TYPE   Build type: Debug, Release, RelWithDebInfo (default: Release)
    --with-tests            Build tests
    --with-samples          Build sample programs
    -j, --jobs N            Number of parallel jobs (default: auto-detect)
    --clean                 Clean build directory before building
    --uninstall             Uninstall Network System

Examples:
    $0                                  # Default installation
    $0 --prefix ~/local --with-tests   # Install to home directory with tests
    $0 --build-type Debug --with-samples # Debug build with samples
    $0 --uninstall                     # Remove installation

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -t|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --with-tests)
            BUILD_TESTS=ON
            shift
            ;;
        --with-samples)
            BUILD_SAMPLES=ON
            shift
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --uninstall)
            UNINSTALL=1
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
    else
        print_error "Unsupported OS: $OSTYPE"
        exit 1
    fi
    print_msg "Detected OS: $OS"
}

# Check dependencies
check_dependencies() {
    print_msg "Checking dependencies..."

    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake not found. Please install CMake 3.16 or later."
        exit 1
    fi

    # Check for compiler
    if ! command -v c++ &> /dev/null && ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        print_error "C++ compiler not found. Please install GCC or Clang."
        exit 1
    fi

    # Check for Ninja (optional but recommended)
    if ! command -v ninja &> /dev/null; then
        print_warning "Ninja not found. Using Make instead (slower builds)."
        CMAKE_GENERATOR="Unix Makefiles"
    else
        CMAKE_GENERATOR="Ninja"
    fi

    # Check for ASIO
    case $OS in
        linux)
            if ! pkg-config --exists asio 2>/dev/null && ! [ -f /usr/include/asio.hpp ]; then
                print_warning "ASIO not found. Install with: sudo apt-get install libasio-dev"
            fi
            ;;
        macos)
            if ! pkg-config --exists asio 2>/dev/null && ! brew list asio &>/dev/null; then
                print_warning "ASIO not found. Install with: brew install asio"
            fi
            ;;
        windows)
            if ! [ -f /mingw64/include/asio.hpp ]; then
                print_warning "ASIO not found. Install with: pacman -S mingw-w64-x86_64-asio"
            fi
            ;;
    esac

    print_msg "Dependencies check complete"
}

# Install system dependencies
install_dependencies() {
    print_msg "Installing system dependencies..."

    case $OS in
        linux)
            if command -v apt-get &> /dev/null; then
                print_msg "Installing dependencies with apt..."
                sudo apt-get update
                sudo apt-get install -y cmake ninja-build libasio-dev libfmt-dev
            elif command -v yum &> /dev/null; then
                print_msg "Installing dependencies with yum..."
                sudo yum install -y cmake ninja-build asio-devel fmt-devel
            elif command -v pacman &> /dev/null; then
                print_msg "Installing dependencies with pacman..."
                sudo pacman -S --needed cmake ninja asio fmt
            fi
            ;;
        macos)
            if command -v brew &> /dev/null; then
                print_msg "Installing dependencies with Homebrew..."
                brew install cmake ninja asio fmt
            else
                print_error "Homebrew not found. Please install Homebrew first."
                exit 1
            fi
            ;;
        windows)
            if command -v pacman &> /dev/null; then
                print_msg "Installing dependencies with MSYS2..."
                pacman -S --needed \
                    mingw-w64-x86_64-cmake \
                    mingw-w64-x86_64-ninja \
                    mingw-w64-x86_64-asio \
                    mingw-w64-x86_64-fmt
            fi
            ;;
    esac

    print_msg "Dependencies installation complete"
}

# Build Network System
build_network_system() {
    print_msg "Building Network System..."

    # Clean build directory if requested
    if [[ -n "$CLEAN_BUILD" ]] && [[ -d "build" ]]; then
        print_msg "Cleaning build directory..."
        rm -rf build
    fi

    # Create build directory
    mkdir -p build
    cd build

    # Configure
    print_msg "Configuring with CMake..."
    cmake .. \
        -G "$CMAKE_GENERATOR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DBUILD_TESTS="$BUILD_TESTS" \
        -DBUILD_SAMPLES="$BUILD_SAMPLES" \
        -DBUILD_SHARED_LIBS=OFF

    # Build
    print_msg "Building with $PARALLEL_JOBS parallel jobs..."
    cmake --build . --parallel "$PARALLEL_JOBS"

    # Run tests if enabled
    if [[ "$BUILD_TESTS" == "ON" ]]; then
        print_msg "Running tests..."
        ctest --output-on-failure || print_warning "Some tests failed"
    fi

    cd ..
    print_msg "Build complete"
}

# Install Network System
install_network_system() {
    print_msg "Installing Network System to $INSTALL_PREFIX..."

    cd build

    # Install
    if [[ "$OS" == "windows" ]] || [[ -w "$INSTALL_PREFIX" ]]; then
        cmake --install .
    else
        sudo cmake --install .
    fi

    # Create pkg-config file
    PKG_CONFIG_DIR="$INSTALL_PREFIX/lib/pkgconfig"
    if [[ "$OS" != "windows" ]]; then
        mkdir -p "$PKG_CONFIG_DIR" 2>/dev/null || sudo mkdir -p "$PKG_CONFIG_DIR"

        cat > network_system.pc << EOF
prefix=$INSTALL_PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: NetworkSystem
Description: High-performance asynchronous network library
Version: Development
Requires:
Libs: -L\${libdir} -lNetworkSystem -pthread
Cflags: -I\${includedir}
EOF

        if [[ -w "$PKG_CONFIG_DIR" ]]; then
            mv network_system.pc "$PKG_CONFIG_DIR/"
        else
            sudo mv network_system.pc "$PKG_CONFIG_DIR/"
        fi
    fi

    cd ..

    print_msg "Installation complete!"
    print_msg "To use Network System in your CMake project:"
    echo "    find_package(NetworkSystem REQUIRED)"
    echo "    target_link_libraries(your_app NetworkSystem::NetworkSystem)"
}

# Uninstall Network System
uninstall_network_system() {
    print_msg "Uninstalling Network System from $INSTALL_PREFIX..."

    # Remove installed files
    if [[ -f "build/install_manifest.txt" ]]; then
        while IFS= read -r file; do
            if [[ -f "$file" ]]; then
                print_msg "Removing $file"
                rm -f "$file" 2>/dev/null || sudo rm -f "$file"
            fi
        done < "build/install_manifest.txt"
    fi

    # Remove directories
    for dir in \
        "$INSTALL_PREFIX/include/network_system" \
        "$INSTALL_PREFIX/lib/cmake/NetworkSystem" \
        "$INSTALL_PREFIX/share/doc/NetworkSystem"
    do
        if [[ -d "$dir" ]]; then
            print_msg "Removing $dir"
            rmdir "$dir" 2>/dev/null || sudo rmdir "$dir" 2>/dev/null || true
        fi
    done

    # Remove pkg-config file
    PKG_CONFIG_FILE="$INSTALL_PREFIX/lib/pkgconfig/network_system.pc"
    if [[ -f "$PKG_CONFIG_FILE" ]]; then
        rm -f "$PKG_CONFIG_FILE" 2>/dev/null || sudo rm -f "$PKG_CONFIG_FILE"
    fi

    print_msg "Uninstall complete"
}

# Main execution
main() {
    echo "========================================"
    echo "   Network System Installation Script   "
    echo "========================================"
    echo

    detect_os

    if [[ -n "$UNINSTALL" ]]; then
        uninstall_network_system
        exit 0
    fi

    check_dependencies

    # Ask to install dependencies
    read -p "Do you want to install missing dependencies? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_dependencies
    fi

    build_network_system
    install_network_system

    echo
    print_msg "Network System has been successfully installed!"
    print_msg "Installation prefix: $INSTALL_PREFIX"
    print_msg "Build type: $BUILD_TYPE"
}

# Run main function
main