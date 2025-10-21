#!/bin/bash

# Messaging System v2.0 Build Script
# Supports CMake presets and Phase 1-3 rebuild architecture

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

# Display banner
echo -e "${BOLD}${BLUE}============================================${NC}"
echo -e "${BOLD}${BLUE}   Messaging System v2.0 Build Script      ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"

# Display help
show_help() {
    echo -e "${BOLD}Usage:${NC} $0 [preset] [options]"
    echo ""
    echo -e "${BOLD}CMake Presets:${NC}"
    echo "  dev-local         ⭐ Local development (recommended, fastest)"
    echo "  release-local     🚀 Local release build (optimized)"
    echo "  default           Production build with find_package"
    echo "  debug             Debug build with all logging"
    echo "  release           Optimized release build"
    echo "  dev-fetchcontent  Development build with FetchContent"
    echo "  asan              AddressSanitizer build"
    echo "  tsan              ThreadSanitizer build"
    echo "  ubsan             UndefinedBehaviorSanitizer build"
    echo "  ci                CI/CD build with pedantic warnings"
    echo "  lockfree          Lock-free data structures enabled"
    echo "  minimal           Minimal build without optional features"
    echo ""
    echo -e "${BOLD}Build Options:${NC}"
    echo "  --clean           Remove build directory before building"
    echo "  --tests           Build and run tests"
    echo "  --examples        Build examples"
    echo "  --benchmarks      Build benchmarks"
    echo "  --verbose         Show detailed build output"
    echo "  --cores N         Use N cores (default: auto-detect)"
    echo ""
    echo -e "${BOLD}Feature Options:${NC}"
    echo "  --lockfree        Enable lock-free implementations"
    echo "  --no-monitoring   Disable monitoring system"
    echo "  --no-logging      Disable logging system"
    echo "  --enable-tls      Enable TLS/SSL support"
    echo ""
    echo -e "${BOLD}Examples:${NC}"
    echo "  $0 dev-local --tests            # Local dev with tests (fastest)"
    echo "  $0 dev-fetchcontent --tests     # Dev build with FetchContent"
    echo "  $0 release --examples           # Release with examples"
    echo "  $0 asan --clean                 # Clean ASAN build"
    echo ""
}

# Status messages
print_status() { echo -e "${BOLD}${BLUE}[STATUS]${NC} $1"; }
print_success() { echo -e "${BOLD}${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${BOLD}${RED}[ERROR]${NC} $1"; }
print_warning() { echo -e "${BOLD}${YELLOW}[WARNING]${NC} $1"; }

# Check command exists
command_exists() {
    command -v "$1" &> /dev/null
}

# Check dependencies
check_dependencies() {
    print_status "Checking build dependencies..."

    local missing_deps=()

    if ! command_exists cmake; then
        missing_deps+=("cmake (>= 3.16)")
    else
        local cmake_version=$(cmake --version | head -n1 | awk '{print $3}')
        print_status "Found CMake $cmake_version"
    fi

    if ! command_exists git; then
        missing_deps+=("git")
    fi

    # Check for C++20 compiler
    if command_exists g++; then
        local gcc_version=$(g++ --version | head -n1 | grep -oE '[0-9]+\.[0-9]+' | head -1)
        print_status "Found g++ $gcc_version"
    elif command_exists clang++; then
        local clang_version=$(clang++ --version | head -n1 | grep -oE '[0-9]+\.[0-9]+' | head -1)
        print_status "Found clang++ $clang_version"
    else
        missing_deps+=("g++ or clang++ with C++20 support")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_warning "Run './dependency.sh' to install dependencies"
        return 1
    fi

    print_success "All core dependencies satisfied"
    return 0
}

# Parse arguments
PRESET="dev-local"
CLEAN_BUILD=0
RUN_TESTS=0
BUILD_EXAMPLES=0
BUILD_BENCHMARKS=0
VERBOSE=0
BUILD_CORES=0
EXTRA_CMAKE_ARGS=""

# First argument might be a preset
if [[ $# -gt 0 ]] && [[ ! $1 =~ ^-- ]]; then
    PRESET="$1"
    shift
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --tests)
            RUN_TESTS=1
            shift
            ;;
        --examples)
            BUILD_EXAMPLES=1
            shift
            ;;
        --benchmarks)
            BUILD_BENCHMARKS=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --cores)
            BUILD_CORES=$2
            shift 2
            ;;
        --lockfree)
            EXTRA_CMAKE_ARGS+=" -DMESSAGING_USE_LOCKFREE=ON"
            shift
            ;;
        --no-monitoring)
            EXTRA_CMAKE_ARGS+=" -DMESSAGING_ENABLE_MONITORING=OFF"
            shift
            ;;
        --no-logging)
            EXTRA_CMAKE_ARGS+=" -DMESSAGING_ENABLE_LOGGING=OFF"
            shift
            ;;
        --enable-tls)
            EXTRA_CMAKE_ARGS+=" -DMESSAGING_ENABLE_TLS=ON"
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Set build cores
if [ $BUILD_CORES -eq 0 ]; then
    if command_exists nproc; then
        BUILD_CORES=$(nproc)
    elif [ "$(uname)" == "Darwin" ]; then
        BUILD_CORES=$(sysctl -n hw.ncpu)
    else
        BUILD_CORES=4
    fi
fi

print_status "Using $BUILD_CORES cores for compilation"

# Check dependencies
check_dependencies || exit 1

# Clean if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    print_status "Cleaning build directories..."
    rm -rf build build-* _builds
    print_success "Build directories cleaned"
fi

# Configure with preset
print_status "Configuring with preset: $PRESET"
cmake --preset $PRESET $EXTRA_CMAKE_ARGS

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "Configuration complete"

# Build
print_status "Building project..."

# Determine build preset name (some configure presets have different build preset names)
BUILD_PRESET="$PRESET"
case $PRESET in
    dev-local) BUILD_PRESET="dev-local" ;;
    dev-fetchcontent) BUILD_PRESET="dev" ;;
esac

if [ $VERBOSE -eq 1 ]; then
    cmake --build --preset $BUILD_PRESET --parallel $BUILD_CORES -- VERBOSE=1
else
    cmake --build --preset $BUILD_PRESET --parallel $BUILD_CORES
fi

if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

print_success "Build complete"

# Run tests if requested
if [ $RUN_TESTS -eq 1 ]; then
    print_status "Running tests..."
    ctest --preset default --output-on-failure

    if [ $? -eq 0 ]; then
        print_success "All tests passed"
    else
        print_error "Some tests failed"
        exit 1
    fi
fi

# Success summary
echo ""
echo -e "${BOLD}${GREEN}============================================${NC}"
echo -e "${BOLD}${GREEN}       Build Completed Successfully        ${NC}"
echo -e "${BOLD}${GREEN}============================================${NC}"
echo ""
echo -e "${CYAN}Configuration:${NC}"
echo -e "  Preset: $PRESET"
echo -e "  Cores: $BUILD_CORES"
[ $RUN_TESTS -eq 1 ] && echo -e "  Tests: Run"
[ $BUILD_EXAMPLES -eq 1 ] && echo -e "  Examples: Built"
[ $BUILD_BENCHMARKS -eq 1 ] && echo -e "  Benchmarks: Built"
echo ""

# Show available binaries
BUILD_DIR="build-local"
case $PRESET in
    dev-local) BUILD_DIR="build-local" ;;
    dev-fetchcontent) BUILD_DIR="build-dev" ;;
    debug) BUILD_DIR="build-debug" ;;
    release) BUILD_DIR="build-release" ;;
    asan) BUILD_DIR="build-asan" ;;
    tsan) BUILD_DIR="build-tsan" ;;
    ubsan) BUILD_DIR="build-ubsan" ;;
    ci) BUILD_DIR="build-ci" ;;
    lockfree) BUILD_DIR="build-lockfree" ;;
    minimal) BUILD_DIR="build-minimal" ;;
esac

if [ -d "$BUILD_DIR/bin" ]; then
    echo -e "${CYAN}Available binaries in $BUILD_DIR/bin:${NC}"
    ls -lh $BUILD_DIR/bin/ 2>/dev/null || echo "  (none yet)"
    echo ""
fi

echo -e "${CYAN}Quick commands:${NC}"
echo -e "  Run main:     ./$BUILD_DIR/bin/messaging_system"
echo -e "  Run tests:    ctest --test-dir $BUILD_DIR"
echo -e "  Run example:  ./$BUILD_DIR/bin/basic_messaging"
echo ""

exit 0
