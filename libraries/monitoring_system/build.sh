#!/bin/bash

# Monitoring System Build Script
# Enhanced version with C++17/C++20 compatibility and threading features

# Color definitions for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Display banner
echo -e "${BOLD}${BLUE}============================================${NC}"
echo -e "${BOLD}${BLUE}    Monitoring System Build Script         ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"

# Display help information
show_help() {
    echo -e "${BOLD}Usage:${NC} $0 [options]"
    echo ""
    echo -e "${BOLD}Build Options:${NC}"
    echo "  --clean           Perform a clean rebuild by removing the build directory"
    echo "  --debug           Build in debug mode (default is release)"
    echo "  --benchmark       Build with benchmarks enabled"
    echo ""
    echo -e "${BOLD}Target Options:${NC}"
    echo "  --all             Build all targets (default)"
    echo "  --lib-only        Build only the core monitoring library"
    echo "  --examples        Build only the example applications"
    echo "  --tests           Build and run the unit tests"
    echo ""
    echo -e "${BOLD}Monitoring-Specific Options:${NC}"
    echo "  --with-thread     Enable thread_system integration"
    echo "  --with-logger     Enable logger_system integration"
    echo "  --no-examples     Disable example programs"
    echo ""
    echo -e "${BOLD}C++ Compatibility Options:${NC}"
    echo "  --cpp17           Force C++17 mode (disable C++20 features)"
    echo "  --force-fmt       Force fmt library usage even if std::format available"
    echo "  --no-vcpkg        Skip vcpkg and use system libraries only"
    echo "  --no-jthread      Disable std::jthread even if supported"
    echo "  --no-concepts     Disable concepts even if supported"
    echo ""
    echo -e "${BOLD}Sanitizer Options:${NC}"
    echo "  --asan            Enable AddressSanitizer"
    echo "  --tsan            Enable ThreadSanitizer"
    echo "  --ubsan           Enable UndefinedBehaviorSanitizer"
    echo ""
    echo -e "${BOLD}Documentation Options:${NC}"
    echo "  --docs            Generate Doxygen documentation"
    echo "  --clean-docs      Clean and regenerate Doxygen documentation"
    echo ""
    echo -e "${BOLD}General Options:${NC}"
    echo "  --cores N         Use N cores for compilation (default: auto-detect)"
    echo "  --verbose         Show detailed build output"
    echo "  --help            Display this help and exit"
    echo ""
    echo -e "${BOLD}Compiler Options:${NC}"
    echo "  --compiler NAME   Select specific compiler (gcc, clang)"
    echo "  --list-compilers  List available compilers and exit"
    echo ""
    echo -e "${BOLD}Examples:${NC}"
    echo "  $0 --clean --debug --tests"
    echo "  $0 --cpp17 --force-fmt --asan"
    echo "  $0 --lib-only --cores 8 --verbose"
}

# Initialize variables with default values
CLEAN_BUILD=0
BUILD_DOCS=0
CLEAN_DOCS=0
BUILD_TYPE=Release
BUILD_BENCHMARKS=0
TARGET=all
BUILD_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
VERBOSE=0
SELECTED_COMPILER=
# Monitoring-specific options
FORCE_CPP17=0
FORCE_FMT=0
NO_VCPKG=0
WITH_THREAD_SYSTEM=0
WITH_LOGGER_SYSTEM=0
NO_EXAMPLES=0
NO_JTHREAD=0
NO_CONCEPTS=0
# Sanitizer options
ENABLE_ASAN=0
ENABLE_TSAN=0
ENABLE_UBSAN=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --debug)
            BUILD_TYPE=Debug
            shift
            ;;
        --benchmark)
            BUILD_BENCHMARKS=1
            shift
            ;;
        --all)
            TARGET=all
            shift
            ;;
        --lib-only)
            TARGET=lib-only
            shift
            ;;
        --examples)
            TARGET=examples
            shift
            ;;
        --tests)
            TARGET=tests
            shift
            ;;
        --with-thread)
            WITH_THREAD_SYSTEM=1
            shift
            ;;
        --with-logger)
            WITH_LOGGER_SYSTEM=1
            shift
            ;;
        --no-examples)
            NO_EXAMPLES=1
            shift
            ;;
        --cpp17)
            FORCE_CPP17=1
            shift
            ;;
        --force-fmt)
            FORCE_FMT=1
            shift
            ;;
        --no-vcpkg)
            NO_VCPKG=1
            shift
            ;;
        --no-jthread)
            NO_JTHREAD=1
            shift
            ;;
        --no-concepts)
            NO_CONCEPTS=1
            shift
            ;;
        --asan)
            ENABLE_ASAN=1
            shift
            ;;
        --tsan)
            ENABLE_TSAN=1
            shift
            ;;
        --ubsan)
            ENABLE_UBSAN=1
            shift
            ;;
        --docs)
            BUILD_DOCS=1
            shift
            ;;
        --clean-docs)
            CLEAN_DOCS=1
            BUILD_DOCS=1
            shift
            ;;
        --cores)
            BUILD_CORES="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --compiler)
            SELECTED_COMPILER="$2"
            shift 2
            ;;
        --list-compilers)
            echo -e "${BOLD}Available compilers:${NC}"
            echo "  gcc    - GNU Compiler Collection"
            echo "  clang  - LLVM Clang Compiler"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Function to print colored messages
print_info() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to set compiler
set_compiler() {
    if [[ -n "$SELECTED_COMPILER" ]]; then
        case "$SELECTED_COMPILER" in
            gcc)
                if command_exists gcc; then
                    export CC=gcc
                    export CXX=g++
                    print_info "Using GCC compiler"
                else
                    print_error "GCC not found"
                    exit 1
                fi
                ;;
            clang)
                if command_exists clang; then
                    export CC=clang
                    export CXX=clang++
                    print_info "Using Clang compiler"
                else
                    print_error "Clang not found"
                    exit 1
                fi
                ;;
            *)
                print_error "Unsupported compiler: $SELECTED_COMPILER"
                exit 1
                ;;
        esac
    fi
}

# Function to build documentation
build_documentation() {
    print_info "Building documentation..."
    
    if [[ $CLEAN_DOCS -eq 1 ]] && [[ -d "docs/html" ]]; then
        print_info "Cleaning existing documentation..."
        rm -rf docs/html
    fi
    
    if command_exists doxygen; then
        doxygen Doxyfile
        print_success "Documentation generated in docs/html/"
    else
        print_warning "Doxygen not found. Please install doxygen to generate documentation."
    fi
}

# Main build function
build_monitoring_system() {
    print_info "Starting Monitoring System build..."
    
    # Set compiler
    set_compiler
    
    # Clean build if requested
    if [[ $CLEAN_BUILD -eq 1 ]] && [[ -d "build" ]]; then
        print_info "Cleaning build directory..."
        rm -rf build
    fi
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Prepare CMake arguments
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    
    # Monitoring-specific options
    if [[ $FORCE_CPP17 -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DFORCE_CPP17=ON"
        print_info "Forcing C++17 mode - C++20 features will be disabled"
    fi
    
    if [[ $FORCE_FMT -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_FORCE_CPP17_FORMAT=ON"
        print_info "Forcing fmt library usage over std::format"
    fi
    
    if [[ $WITH_THREAD_SYSTEM -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_USE_THREAD_SYSTEM=ON"
        print_info "Enabling thread_system integration"
    fi
    
    if [[ $WITH_LOGGER_SYSTEM -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_USE_LOGGER_SYSTEM=ON"
        print_info "Enabling logger_system integration"
    fi
    
    if [[ $BUILD_BENCHMARKS -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_BUILD_BENCHMARKS=ON"
    fi
    
    # Target-specific options
    case $TARGET in
        lib-only)
            CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_BUILD_EXAMPLES=OFF -DMONITORING_BUILD_TESTS=OFF"
            ;;
        examples)
            CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_BUILD_EXAMPLES=ON -DMONITORING_BUILD_TESTS=OFF"
            ;;
        tests)
            CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_BUILD_TESTS=ON"
            ;;
    esac
    
    if [[ $NO_EXAMPLES -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DMONITORING_BUILD_EXAMPLES=OFF"
    fi
    
    # Sanitizer options
    if [[ $ENABLE_ASAN -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DENABLE_ASAN=ON"
        print_info "Enabling AddressSanitizer"
    fi
    
    if [[ $ENABLE_TSAN -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DENABLE_TSAN=ON"
        print_info "Enabling ThreadSanitizer"
    fi
    
    if [[ $ENABLE_UBSAN -eq 1 ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DENABLE_UBSAN=ON"
        print_info "Enabling UndefinedBehaviorSanitizer"
    fi
    
    # Configure
    print_info "Configuring build with: $CMAKE_ARGS"
    if [[ $VERBOSE -eq 1 ]]; then
        cmake $CMAKE_ARGS ..
    else
        cmake $CMAKE_ARGS .. > /dev/null
    fi
    
    if [[ $? -ne 0 ]]; then
        print_error "Configuration failed"
        exit 1
    fi
    
    print_success "Configuration completed"
    
    # Build
    print_info "Building with $BUILD_CORES cores..."
    if [[ $VERBOSE -eq 1 ]]; then
        make -j$BUILD_CORES
    else
        make -j$BUILD_CORES > /dev/null
    fi
    
    if [[ $? -ne 0 ]]; then
        print_error "Build failed"
        exit 1
    fi
    
    print_success "Build completed successfully!"
    
    # Run tests if requested
    if [[ $TARGET == "tests" ]]; then
        print_info "Running tests..."
        ctest --verbose
        
        if [[ $? -eq 0 ]]; then
            print_success "All tests passed!"
        else
            print_warning "Some tests failed. Check the output above."
        fi
    fi
    
    cd ..
}

# Main execution
print_info "Monitoring System Build Configuration:"
print_info "  Build Type: $BUILD_TYPE"
print_info "  Target: $TARGET"
print_info "  Cores: $BUILD_CORES"
print_info "  Force C++17: $([ $FORCE_CPP17 -eq 1 ] && echo 'Yes' || echo 'No')"
print_info "  Force fmt: $([ $FORCE_FMT -eq 1 ] && echo 'Yes' || echo 'No')"
print_info "  Thread System: $([ $WITH_THREAD_SYSTEM -eq 1 ] && echo 'Yes' || echo 'No')"
print_info "  Logger System: $([ $WITH_LOGGER_SYSTEM -eq 1 ] && echo 'Yes' || echo 'No')"
print_info "  AddressSanitizer: $([ $ENABLE_ASAN -eq 1 ] && echo 'Yes' || echo 'No')"
print_info "  ThreadSanitizer: $([ $ENABLE_TSAN -eq 1 ] && echo 'Yes' || echo 'No')"

echo ""

# Build documentation if requested
if [[ $BUILD_DOCS -eq 1 ]]; then
    build_documentation
    echo ""
fi

# Build the project
build_monitoring_system

print_success "Monitoring System build script completed!"