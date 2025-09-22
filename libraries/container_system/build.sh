#!/bin/bash

# Container System Build Script
# This script builds the container system library with all features, tests, and examples

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
BUILD_EXAMPLES="ON"
BUILD_SAMPLES="ON"
BUILD_DOCS="OFF"
MESSAGING_FEATURES="ON"
PERFORMANCE_METRICS="ON"
EXTERNAL_INTEGRATION="ON"
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
    echo "Container System Build Script"
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -d, --debug             Build in Debug mode (default: Release)"
    echo "  -c, --clean             Clean build (remove build directory first)"
    echo "  -t, --no-tests          Don't build tests"
    echo "  -E, --no-examples       Don't build examples"
    echo "  -s, --no-samples        Don't build samples"
    echo "  -D, --docs              Build documentation"
    echo "  --no-messaging          Disable messaging features"
    echo "  --no-metrics            Disable performance metrics"
    echo "  --no-integration        Disable external integration"
    echo "  -v, --verbose           Verbose output"
    echo "  -j, --jobs N            Number of parallel jobs (default: $JOBS)"
    echo ""
    echo "Features (enabled by default):"
    echo "  - Messaging features (messaging integration)"
    echo "  - Performance metrics (monitoring and analytics)"
    echo "  - External integration (callback system)"
    echo "  - Unit tests and integration tests"
    echo "  - Examples and samples"
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
        -E|--no-examples)
            BUILD_EXAMPLES="OFF"
            shift
            ;;
        -s|--no-samples)
            BUILD_SAMPLES="OFF"
            shift
            ;;
        -D|--docs)
            BUILD_DOCS="ON"
            shift
            ;;
        --no-messaging)
            MESSAGING_FEATURES="OFF"
            shift
            ;;
        --no-metrics)
            PERFORMANCE_METRICS="OFF"
            shift
            ;;
        --no-integration)
            EXTERNAL_INTEGRATION="OFF"
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
print_info "Container System Build Configuration:"
print_info "  Build Type: $BUILD_TYPE"
print_info "  Build Tests: $BUILD_TESTS"
print_info "  Build Examples: $BUILD_EXAMPLES"
print_info "  Build Samples: $BUILD_SAMPLES"
print_info "  Build Docs: $BUILD_DOCS"
print_info "  Messaging Features: $MESSAGING_FEATURES"
print_info "  Performance Metrics: $PERFORMANCE_METRICS"
print_info "  External Integration: $EXTERNAL_INTEGRATION"
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
    -DUSE_UNIT_TEST="$BUILD_TESTS"
    -DBUILD_CONTAINER_EXAMPLES="$BUILD_EXAMPLES"
    -DBUILD_CONTAINER_SAMPLES="$BUILD_SAMPLES"
    -DENABLE_MESSAGING_FEATURES="$MESSAGING_FEATURES"
    -DENABLE_PERFORMANCE_METRICS="$PERFORMANCE_METRICS"
    -DENABLE_EXTERNAL_INTEGRATION="$EXTERNAL_INTEGRATION"
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
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

    # Run unit tests
    if [ -f "unit_tests" ]; then
        print_info "Running unit tests..."
        if ./unit_tests; then
            print_info "Unit tests passed!"
        else
            print_error "Unit tests failed!"
            exit 1
        fi
    fi

    # Run integration tests
    if [ -f "test_messaging_integration" ]; then
        print_info "Running integration tests..."
        if ./test_messaging_integration; then
            print_info "Integration tests passed!"
        else
            print_error "Integration tests failed!"
            exit 1
        fi
    fi

    # Run performance tests (but don't fail on performance issues)
    if [ -f "performance_tests" ]; then
        print_info "Running performance tests..."
        if ./performance_tests; then
            print_info "Performance tests completed!"
        else
            print_warning "Performance tests had issues (this is informational only)"
        fi
    fi

    # Run all tests via CTest if available
    if command -v ctest >/dev/null 2>&1; then
        print_info "Running all tests via CTest..."
        if ctest --output-on-failure; then
            print_info "All CTest tests passed!"
        else
            print_warning "Some CTest tests failed"
        fi
    fi
fi

print_info "Build completed successfully!"
print_info "Build artifacts are in: $SCRIPT_DIR/build"

# Print summary
echo ""
print_info "Build Summary:"
print_info "  Library: build/lib/libcontainer_system.*"
if [ "$BUILD_TESTS" = "ON" ]; then
    print_info "  Unit Tests: build/unit_tests"
    print_info "  Integration Tests: build/test_messaging_integration"
    print_info "  Performance Tests: build/performance_tests"
    print_info "  Benchmark Tests: build/benchmark_tests"
fi
if [ "$BUILD_EXAMPLES" = "ON" ]; then
    print_info "  Basic Example: build/examples/basic_container_example"
    print_info "  Advanced Example: build/examples/advanced_container_example"
    print_info "  Real World Scenarios: build/examples/real_world_scenarios"
    if [ "$MESSAGING_FEATURES" = "ON" ]; then
        print_info "  Messaging Example: build/examples/messaging_integration_example"
    fi
fi
if [ "$BUILD_SAMPLES" = "ON" ]; then
    print_info "  Samples: build/samples/"
fi
if [ "$BUILD_DOCS" = "ON" ]; then
    print_info "  Documentation: build/documents/html/index.html"
fi

echo ""
print_info "Quick Start:"
print_info "  Run basic example: ./build/examples/basic_container_example"
if [ "$BUILD_EXAMPLES" = "ON" ]; then
    print_info "  Run advanced demo: ./build/examples/advanced_container_example"
    print_info "  Run scenarios demo: ./build/examples/real_world_scenarios"
fi
if [ "$BUILD_TESTS" = "ON" ]; then
    print_info "  Run all tests: cd build && ctest"
fi