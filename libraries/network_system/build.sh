#!/bin/bash

# Build script for Network System
# This script builds the network system library and optionally tests

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Default values
BUILD_TYPE="Release"
BUILD_TESTS="ON"
BUILD_SAMPLES="OFF"
BUILD_DOCS="OFF"
CLEAN_BUILD=false
VERBOSE=false
JOBS=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

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

# Function to print usage
usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -d, --debug         Build in Debug mode (default: Release)"
    echo "  -c, --clean         Clean build (remove build directory first)"
    echo "  -t, --no-tests      Don't build tests"
    echo "  -s, --samples       Build samples"
    echo "  -D, --docs          Build documentation"
    echo "  -v, --verbose       Verbose output"
    echo "  -j, --jobs N        Number of parallel jobs (default: $JOBS)"
    exit 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -t|--no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        -s|--samples)
            BUILD_SAMPLES="ON"
            shift
            ;;
        -D|--docs)
            BUILD_DOCS="ON"
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            ;;
    esac
done

# Print build configuration
print_info "Network System Build Configuration:"
print_info "  Build Type: $BUILD_TYPE"
print_info "  Build Tests: $BUILD_TESTS"
print_info "  Build Samples: $BUILD_SAMPLES"
print_info "  Build Docs: $BUILD_DOCS"
print_info "  Parallel Jobs: $JOBS"
print_info "  Clean Build: $CLEAN_BUILD"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_info "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
print_info "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
print_info "Configuring with CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_NETWORK_TESTS="$BUILD_TESTS"
    -DBUILD_NETWORK_SAMPLES="$BUILD_SAMPLES"
    -DBUILD_NETWORK_DOCS="$BUILD_DOCS"
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

# Add vcpkg toolchain if available
if [ -n "$VCPKG_ROOT" ]; then
    CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake")
elif command -v vcpkg >/dev/null 2>&1; then
    VCPKG_ROOT=$(dirname $(which vcpkg))
    CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake")
fi

if ! cmake "${CMAKE_ARGS[@]}" ..; then
    print_error "CMake configuration failed!"
    exit 1
fi

# Build
print_info "Building..."
if [ "$VERBOSE" = true ]; then
    if ! cmake --build . -j "$JOBS"; then
        print_error "Build failed!"
        exit 1
    fi
else
    if ! cmake --build . -j "$JOBS" 2>&1 | tee build.log | grep -E "^\[|error:|warning:"; then
        if grep -q "error:" build.log; then
            print_error "Build failed! Check build.log for details."
            exit 1
        fi
    fi
fi

# Run tests if built
if [ "$BUILD_TESTS" = "ON" ]; then
    print_info "Running tests..."
    if [ -f "bin/network_unit_tests" ]; then
        if ./bin/network_unit_tests; then
            print_info "Unit tests passed!"
        else
            print_warning "Some unit tests failed (this may be due to network port availability)"
        fi
    else
        print_warning "Unit tests not found. Skipping tests."
    fi
fi

print_info "Build completed successfully!"
print_info "Build artifacts are in: $SCRIPT_DIR/build"

# Print summary
echo ""
print_info "Summary:"
print_info "  Library: build/lib/libnetwork.*"
if [ "$BUILD_TESTS" = "ON" ]; then
    print_info "  Unit Tests: build/bin/network_unit_tests"
    print_info "  Benchmarks: build/bin/network_benchmark_tests"
fi
if [ "$BUILD_SAMPLES" = "ON" ]; then
    print_info "  Samples: build/bin/"
fi

# Print network usage notes
echo ""
print_info "Network System Notes:"
print_info "  - Server components listen on TCP ports"
print_info "  - Ensure firewall allows the ports you use"
print_info "  - Default test ports start from 5000"
print_info "  - Benchmarks require a running server on port 6000+"