#!/bin/bash

# Logger System Build Script - Interactive Compiler Selection Version
# Improved version with interactive compiler selection and better error handling

# Color definitions for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Display banner
echo -e "${BOLD}${BLUE}============================================${NC}"
echo -e "${BOLD}${BLUE}      Logger System Build Script           ${NC}"
echo -e "${BOLD}${BLUE}     Interactive Compiler Selection         ${NC}"
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
    echo "  --lib-only        Build only the core libraries"
    echo "  --samples         Build only the sample applications"
    echo "  --tests           Build and run the unit tests"
    echo ""
    echo -e "${BOLD}Documentation Options:${NC}"
    echo "  --docs            Generate Doxygen documentation"
    echo "  --clean-docs      Clean and regenerate Doxygen documentation"
    echo ""
    echo -e "${BOLD}Logger-Specific Options:${NC}"
    echo "  --standalone      Build in standalone mode without thread_system"
    echo "  --with-thread     Build with thread_system integration"
    echo ""
    echo -e "${BOLD}C++ Compatibility Options:${NC}"
    echo "  --cpp17           Force C++17 mode (disable C++20 features)"
    echo "  --force-fmt       Force fmt library usage even if std::format available"
    echo "  --no-vcpkg        Skip vcpkg and use system libraries only"
    echo "  --no-format       Disable std::format even if supported (legacy)"
    echo ""
    echo -e "${BOLD}Compiler Options:${NC}"
    echo "  --compiler PATH   Use specific compiler (skips interactive selection)"
    echo "  --auto            Auto-select first available compiler (skips interactive)"
    echo ""
    echo -e "${BOLD}General Options:${NC}"
    echo "  --cores N         Use N cores for compilation (default: auto-detect)"
    echo "  --verbose         Show detailed build output"
    echo "  --help            Display this help and exit"
    echo ""
}

# Function to print status messages
print_status() {
    echo -e "${BOLD}${BLUE}[STATUS]${NC} $1"
}

# Function to print success messages
print_success() {
    echo -e "${BOLD}${GREEN}[SUCCESS]${NC} $1"
}

# Function to print error messages
print_error() {
    echo -e "${BOLD}${RED}[ERROR]${NC} $1"
}

# Function to print warning messages
print_warning() {
    echo -e "${BOLD}${YELLOW}[WARNING]${NC} $1"
}

# Function to print info messages
print_info() {
    echo -e "${BOLD}${CYAN}[INFO]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" &> /dev/null
}

# Function to detect installed compilers
detect_compilers() {
    local compilers=()
    local compiler_names=()
    local compiler_types=()
    
    # Detect GCC variants
    for version in 14 13 12 11 10 9 8 7; do
        if command_exists "gcc-$version"; then
            compilers+=("gcc-$version")
            compiler_names+=("GCC $version")
            compiler_types+=("gcc")
            
            # Check for corresponding g++
            if command_exists "g++-$version"; then
                compilers+=("g++-$version")
                compiler_names+=("G++ $version")
                compiler_types+=("g++")
            fi
        fi
    done
    
    # Check default GCC
    if command_exists "gcc"; then
        local gcc_version=$(gcc --version 2>/dev/null | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -1)
        compilers+=("gcc")
        compiler_names+=("GCC (default) $gcc_version")
        compiler_types+=("gcc")
    fi
    
    # Check default G++
    if command_exists "g++"; then
        local gpp_version=$(g++ --version 2>/dev/null | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -1)
        compilers+=("g++")
        compiler_names+=("G++ (default) $gpp_version")
        compiler_types+=("g++")
    fi
    
    # Detect Clang variants
    for version in 19 18 17 16 15 14 13 12 11 10 9; do
        if command_exists "clang-$version"; then
            compilers+=("clang-$version")
            compiler_names+=("Clang $version")
            compiler_types+=("clang")
            
            # Check for corresponding clang++
            if command_exists "clang++-$version"; then
                compilers+=("clang++-$version")
                compiler_names+=("Clang++ $version")
                compiler_types+=("clang++")
            fi
        fi
    done
    
    # Check default Clang
    if command_exists "clang"; then
        local clang_version=$(clang --version 2>/dev/null | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -1)
        compilers+=("clang")
        compiler_names+=("Clang (default) $clang_version")
        compiler_types+=("clang")
    fi
    
    # Check default Clang++
    if command_exists "clang++"; then
        local clangpp_version=$(clang++ --version 2>/dev/null | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -1)
        compilers+=("clang++")
        compiler_names+=("Clang++ (default) $clangpp_version")
        compiler_types+=("clang++")
    fi
    
    # Check for Intel compilers
    if command_exists "icc"; then
        compilers+=("icc")
        compiler_names+=("Intel C Compiler (icc)")
        compiler_types+=("icc")
    fi
    
    if command_exists "icpc"; then
        compilers+=("icpc")
        compiler_names+=("Intel C++ Compiler (icpc)")
        compiler_types+=("icpc")
    fi
    
    # Check for newer Intel compilers
    if command_exists "icx"; then
        compilers+=("icx")
        compiler_names+=("Intel oneAPI C Compiler (icx)")
        compiler_types+=("icx")
    fi
    
    if command_exists "icpx"; then
        compilers+=("icpx")
        compiler_names+=("Intel oneAPI C++ Compiler (icpx)")
        compiler_types+=("icpx")
    fi
    
    # Check for ARM compilers
    if command_exists "armclang"; then
        compilers+=("armclang")
        compiler_names+=("ARM Compiler (armclang)")
        compiler_types+=("armclang")
    fi
    
    if command_exists "armclang++"; then
        compilers+=("armclang++")
        compiler_names+=("ARM C++ Compiler (armclang++)")
        compiler_types+=("armclang++")
    fi
    
    # macOS specific: Check for Apple Clang
    if [ "$(uname)" == "Darwin" ]; then
        if command_exists "/usr/bin/clang++"; then
            local apple_clang_version=$(/usr/bin/clang++ --version 2>/dev/null | head -n1)
            compilers+=("/usr/bin/clang++")
            compiler_names+=("Apple Clang")
            compiler_types+=("clang++")
        fi
    fi
    
    # Return arrays via global variables
    DETECTED_COMPILERS=("${compilers[@]}")
    DETECTED_COMPILER_NAMES=("${compiler_names[@]}")
    DETECTED_COMPILER_TYPES=("${compiler_types[@]}")
}

# Function to select compiler interactively
select_compiler() {
    if [ ${#DETECTED_COMPILERS[@]} -eq 0 ]; then
        print_error "No C/C++ compilers found on your system!"
        print_info "Please install a compiler (gcc, g++, clang, etc.) and try again."
        return 1
    fi
    
    echo ""
    echo -e "${BOLD}${CYAN}Available Compilers:${NC}"
    echo -e "${BOLD}${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    # Group compilers by type
    local cpp_compilers=()
    local c_compilers=()
    
    for i in "${!DETECTED_COMPILERS[@]}"; do
        local type="${DETECTED_COMPILER_TYPES[$i]}"
        if [[ "$type" == "g++" || "$type" == "clang++" || "$type" == "icpc" || "$type" == "icpx" || "$type" == "armclang++" ]]; then
            cpp_compilers+=($i)
        else
            c_compilers+=($i)
        fi
    done
    
    # Display C++ compilers first (recommended for Logger System)
    if [ ${#cpp_compilers[@]} -gt 0 ]; then
        echo -e "${BOLD}${GREEN}C++ Compilers (Recommended):${NC}"
        local count=1
        for idx in "${cpp_compilers[@]}"; do
            printf "  ${BOLD}%2d)${NC} %-25s ${MAGENTA}[%s]${NC}\n" $count "${DETECTED_COMPILER_NAMES[$idx]}" "${DETECTED_COMPILERS[$idx]}"
            count=$((count + 1))
        done
    fi
    
    # Display C compilers
    if [ ${#c_compilers[@]} -gt 0 ]; then
        echo -e "\n${BOLD}${YELLOW}C Compilers:${NC}"
        local count=$((${#cpp_compilers[@]} + 1))
        for idx in "${c_compilers[@]}"; do
            printf "  ${BOLD}%2d)${NC} %-25s ${MAGENTA}[%s]${NC}\n" $count "${DETECTED_COMPILER_NAMES[$idx]}" "${DETECTED_COMPILERS[$idx]}"
            count=$((count + 1))
        done
    fi
    
    echo -e "${BOLD}${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Get user selection
    local total_compilers=${#DETECTED_COMPILERS[@]}
    local selected_idx=""
    
    while true; do
        echo -n -e "${BOLD}Select a compiler [1-$total_compilers] (${GREEN}recommended: 1${NC}): ${NC}"
        read selected_idx
        
        # Default to first option if empty
        if [ -z "$selected_idx" ]; then
            selected_idx=1
        fi
        
        # Validate input
        if [[ "$selected_idx" =~ ^[0-9]+$ ]] && [ "$selected_idx" -ge 1 ] && [ "$selected_idx" -le "$total_compilers" ]; then
            break
        else
            print_error "Invalid selection. Please enter a number between 1 and $total_compilers."
        fi
    done
    
    # Map selection to actual index
    local actual_idx
    if [ "$selected_idx" -le ${#cpp_compilers[@]} ]; then
        actual_idx=${cpp_compilers[$((selected_idx - 1))]}
    else
        local c_idx=$((selected_idx - ${#cpp_compilers[@]} - 1))
        actual_idx=${c_compilers[$c_idx]}
    fi
    
    SELECTED_COMPILER="${DETECTED_COMPILERS[$actual_idx]}"
    SELECTED_COMPILER_NAME="${DETECTED_COMPILER_NAMES[$actual_idx]}"
    SELECTED_COMPILER_TYPE="${DETECTED_COMPILER_TYPES[$actual_idx]}"
    
    echo ""
    print_success "Selected compiler: ${BOLD}${GREEN}$SELECTED_COMPILER_NAME${NC}"
    
    # Determine C and C++ compilers based on selection
    case "$SELECTED_COMPILER_TYPE" in
        gcc)
            CC_COMPILER="$SELECTED_COMPILER"
            CXX_COMPILER="${SELECTED_COMPILER/gcc/g++}"
            ;;
        g++)
            CXX_COMPILER="$SELECTED_COMPILER"
            CC_COMPILER="${SELECTED_COMPILER/g++/gcc}"
            ;;
        clang)
            CC_COMPILER="$SELECTED_COMPILER"
            CXX_COMPILER="${SELECTED_COMPILER/clang/clang++}"
            ;;
        clang++)
            CXX_COMPILER="$SELECTED_COMPILER"
            CC_COMPILER="${SELECTED_COMPILER/clang++/clang}"
            ;;
        icc)
            CC_COMPILER="$SELECTED_COMPILER"
            CXX_COMPILER="icpc"
            ;;
        icpc)
            CXX_COMPILER="$SELECTED_COMPILER"
            CC_COMPILER="icc"
            ;;
        icx)
            CC_COMPILER="$SELECTED_COMPILER"
            CXX_COMPILER="icpx"
            ;;
        icpx)
            CXX_COMPILER="$SELECTED_COMPILER"
            CC_COMPILER="icx"
            ;;
        armclang)
            CC_COMPILER="$SELECTED_COMPILER"
            CXX_COMPILER="armclang++"
            ;;
        armclang++)
            CXX_COMPILER="$SELECTED_COMPILER"
            CC_COMPILER="armclang"
            ;;
        *)
            CC_COMPILER="$SELECTED_COMPILER"
            CXX_COMPILER="$SELECTED_COMPILER"
            ;;
    esac
    
    # Verify the compilers exist
    if ! command_exists "$CC_COMPILER"; then
        print_warning "C compiler $CC_COMPILER not found, using $SELECTED_COMPILER"
        CC_COMPILER="$SELECTED_COMPILER"
    fi
    
    if ! command_exists "$CXX_COMPILER"; then
        print_warning "C++ compiler $CXX_COMPILER not found, using $SELECTED_COMPILER"
        CXX_COMPILER="$SELECTED_COMPILER"
    fi
    
    print_info "Using C compiler: $CC_COMPILER"
    print_info "Using C++ compiler: $CXX_COMPILER"
    
    return 0
}

# Function to check and install dependencies
check_dependencies() {
    print_status "Checking build dependencies..."
    
    local missing_deps=()
    
    # Check for essential build tools
    for cmd in cmake git; do
        if ! command_exists "$cmd"; then
            missing_deps+=("$cmd")
        fi
    done
    
    # Check for at least one build system (make or ninja)
    if ! command_exists "make" && ! command_exists "ninja"; then
        missing_deps+=("make or ninja")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_warning "Please install missing dependencies before building."
        print_warning "You can use './dependency.sh' to install all required dependencies."
        return 1
    fi
    
    # Check for vcpkg
    if [ ! -d "../vcpkg" ]; then
        print_warning "vcpkg not found in parent directory."
        print_warning "Running dependency script to set up vcpkg..."
        
        if [ -f "./dependency.sh" ]; then
            bash ./dependency.sh
            if [ $? -ne 0 ]; then
                print_error "Failed to run dependency.sh"
                return 1
            fi
        else
            print_error "dependency.sh script not found"
            return 1
        fi
    fi
    
    # Check for doxygen if building docs
    if [ $BUILD_DOCS -eq 1 ] && ! command_exists doxygen; then
        print_warning "Doxygen not found but documentation was requested."
        print_warning "Documentation will not be generated."
        BUILD_DOCS=0
    fi
    
    print_success "All dependencies are satisfied"
    return 0
}

# Process command line arguments
CLEAN_BUILD=0
BUILD_DOCS=0
CLEAN_DOCS=0
BUILD_TYPE="Release"
BUILD_BENCHMARKS=0
TARGET="all"
DISABLE_STD_FORMAT=0
BUILD_CORES=0
VERBOSE=0
SPECIFIC_COMPILER=""
AUTO_SELECT=0
# New C++17/C++20 compatibility options
FORCE_CPP17=0
FORCE_FMT=0
NO_VCPKG=0
STANDALONE_MODE=0
WITH_THREAD=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --benchmark)
            BUILD_BENCHMARKS=1
            shift
            ;;
        --all)
            TARGET="all"
            shift
            ;;
        --lib-only)
            TARGET="lib-only"
            shift
            ;;
        --samples)
            TARGET="samples"
            shift
            ;;
        --tests)
            TARGET="tests"
            shift
            ;;
        --docs)
            BUILD_DOCS=1
            shift
            ;;
        --clean-docs)
            BUILD_DOCS=1
            CLEAN_DOCS=1
            shift
            ;;
        --no-format)
            DISABLE_STD_FORMAT=1
            shift
            ;;
        --standalone)
            STANDALONE_MODE=1
            shift
            ;;
        --with-thread)
            WITH_THREAD=1
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
        --compiler)
            if [ -n "$2" ] && [ "${2:0:1}" != "-" ]; then
                SPECIFIC_COMPILER="$2"
                shift 2
            else
                print_error "Option --compiler requires an argument"
                exit 1
            fi
            ;;
        --auto)
            AUTO_SELECT=1
            shift
            ;;
        --cores)
            if [[ $2 =~ ^[0-9]+$ ]]; then
                BUILD_CORES=$2
                shift 2
            else
                print_error "Option --cores requires a numeric argument"
                exit 1
            fi
            ;;
        --verbose)
            VERBOSE=1
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

# Set number of cores to use for building
if [ $BUILD_CORES -eq 0 ]; then
    if command_exists nproc; then
        BUILD_CORES=$(nproc)
    elif [ "$(uname)" == "Darwin" ]; then
        BUILD_CORES=$(sysctl -n hw.ncpu)
        # macOS specific: Force single thread to avoid jobserver issues
        print_warning "macOS detected: Using single-threaded build to avoid jobserver issues"
        BUILD_CORES=1
    else
        # Default to 2 if we can't detect
        BUILD_CORES=2
    fi
fi

print_status "Using $BUILD_CORES cores for compilation"

# Store original directory
ORIGINAL_DIR=$(pwd)

# Check for platform-specific settings
if [ "$(uname)" == "Linux" ]; then
    if [ $(uname -m) == "aarch64" ]; then
        export VCPKG_FORCE_SYSTEM_BINARIES=arm
        print_status "Detected ARM64 platform, setting VCPKG_FORCE_SYSTEM_BINARIES=arm"
    fi
fi

# Check dependencies before proceeding
check_dependencies
if [ $? -ne 0 ]; then
    print_error "Failed dependency check. Exiting."
    exit 1
fi

# Detect available compilers
print_status "Detecting available compilers..."
detect_compilers

# Select compiler
if [ -n "$SPECIFIC_COMPILER" ]; then
    # Use specified compiler
    if command_exists "$SPECIFIC_COMPILER"; then
        CC_COMPILER="$SPECIFIC_COMPILER"
        CXX_COMPILER="$SPECIFIC_COMPILER"
        print_success "Using specified compiler: $SPECIFIC_COMPILER"
    else
        print_error "Specified compiler '$SPECIFIC_COMPILER' not found!"
        exit 1
    fi
elif [ $AUTO_SELECT -eq 1 ]; then
    # Auto-select first available compiler
    if [ ${#DETECTED_COMPILERS[@]} -gt 0 ]; then
        SELECTED_COMPILER="${DETECTED_COMPILERS[0]}"
        SELECTED_COMPILER_NAME="${DETECTED_COMPILER_NAMES[0]}"
        CC_COMPILER="$SELECTED_COMPILER"
        CXX_COMPILER="$SELECTED_COMPILER"
        print_success "Auto-selected compiler: $SELECTED_COMPILER_NAME"
    else
        print_error "No compilers found for auto-selection!"
        exit 1
    fi
else
    # Interactive compiler selection
    select_compiler
    if [ $? -ne 0 ]; then
        print_error "Compiler selection failed. Exiting."
        exit 1
    fi
fi

# Clean build if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    print_status "Performing clean build..."
    rm -rf build
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    print_status "Creating build directory..."
    mkdir -p build
fi

# Prepare CMake arguments
CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake"
CMAKE_ARGS+=" -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
CMAKE_ARGS+=" -DCMAKE_C_COMPILER=$CC_COMPILER"
CMAKE_ARGS+=" -DCMAKE_CXX_COMPILER=$CXX_COMPILER"

# Add feature flags based on options
if [ $DISABLE_STD_FORMAT -eq 1 ]; then
    CMAKE_ARGS+=" -DUSE_STD_FORMAT=OFF"
fi

if [ $BUILD_BENCHMARKS -eq 1 ]; then
    CMAKE_ARGS+=" -DBUILD_BENCHMARKS=ON"
fi

# New C++17/C++20 compatibility options
if [ $FORCE_CPP17 -eq 1 ]; then
    CMAKE_ARGS+=" -DFORCE_CPP17=ON"
    print_info "Forcing C++17 mode - C++20 features will be disabled"
fi

if [ $FORCE_FMT -eq 1 ]; then
    CMAKE_ARGS+=" -DLOGGER_FORCE_CPP17_FORMAT=ON"
    print_info "Forcing fmt library usage over std::format"
fi

if [ $NO_VCPKG -eq 1 ]; then
    CMAKE_ARGS+=" -DNO_VCPKG=ON"
    # Remove vcpkg toolchain if disabling vcpkg
    CMAKE_ARGS=$(echo "$CMAKE_ARGS" | sed 's/-DCMAKE_TOOLCHAIN_FILE=[^ ]*//')
    print_info "Skipping vcpkg - using system libraries only"
fi

if [ $STANDALONE_MODE -eq 1 ]; then
    CMAKE_ARGS+=" -DLOGGER_STANDALONE=ON"
    print_info "Building in standalone mode without thread_system"
elif [ $WITH_THREAD -eq 1 ]; then
    CMAKE_ARGS+=" -DUSE_THREAD_SYSTEM=ON"
    print_info "Building with thread_system integration"
fi

# Set build targets based on option
if [ "$TARGET" == "lib-only" ]; then
    CMAKE_ARGS+=" -DBUILD_SAMPLES=OFF -DBUILD_TESTS=OFF"
elif [ "$TARGET" == "samples" ]; then
    CMAKE_ARGS+=" -DBUILD_SAMPLES=ON -DBUILD_TESTS=OFF"
elif [ "$TARGET" == "tests" ]; then
    CMAKE_ARGS+=" -DBUILD_SAMPLES=OFF -DBUILD_TESTS=ON"
else
    # Default "all" target - disable samples to avoid API compatibility issues
    CMAKE_ARGS+=" -DBUILD_SAMPLES=OFF -DBUILD_TESTS=OFF"
fi

# Enter build directory
cd build || { print_error "Failed to enter build directory"; exit 1; }

# Run CMake configuration
print_status "Configuring project with CMake..."
print_info "Compiler: C=$CC_COMPILER, C++=$CXX_COMPILER"

if [ $VERBOSE -eq 1 ]; then
    echo "CMake command: cmake .. $CMAKE_ARGS"
fi

if [ "$(uname)" == "Darwin" ]; then
    # On macOS, use local modified GTest config
    cmake .. $CMAKE_ARGS -DCMAKE_PREFIX_PATH="$(pwd)"
else
    cmake .. $CMAKE_ARGS
fi

# Check if CMake configuration was successful
if [ $? -ne 0 ]; then
    print_error "CMake configuration failed. See the output above for details."
    cd "$ORIGINAL_DIR"
    exit 1
fi

# Build the project
print_status "Building project in $BUILD_TYPE mode..."

# Detect build system (Ninja or Make)
if [ -f "build.ninja" ]; then
    BUILD_COMMAND="ninja"
    if [ $VERBOSE -eq 1 ]; then
        BUILD_ARGS="-v"
    else
        BUILD_ARGS=""
    fi
elif [ -f "Makefile" ]; then
    BUILD_COMMAND="make"
    if [ $VERBOSE -eq 1 ]; then
        BUILD_ARGS="VERBOSE=1"
    else
        BUILD_ARGS=""
    fi
else
    print_error "No build system files found (neither build.ninja nor Makefile)"
    cd "$ORIGINAL_DIR"
    exit 1
fi

print_status "Using build system: $BUILD_COMMAND"
print_status "Building with compiler: $CC_COMPILER / $CXX_COMPILER"

# Run build with appropriate cores
$BUILD_COMMAND -j$BUILD_CORES $BUILD_ARGS

# Check if build was successful
if [ $? -ne 0 ]; then
    print_error "Build failed. See the output above for details."
    cd "$ORIGINAL_DIR"
    exit 1
fi

# Run tests if requested
if [ "$TARGET" == "tests" ]; then
    print_status "Running tests..."
    
    # Run tests with CTest
    if command_exists ctest; then
        ctest --output-on-failure
        if [ $? -eq 0 ]; then
            print_success "All tests passed!"
        else
            print_error "Some tests failed. See the output above for details."
        fi
    else
        # Try running test executables directly
        test_failed=0
        for test in ./bin/*_test ./tests/*_test; do
            if [ -x "$test" ]; then
                print_status "Running $(basename $test)..."
                $test
                if [ $? -ne 0 ]; then
                    print_error "Test $(basename $test) failed"
                    test_failed=1
                else
                    print_success "Test $(basename $test) passed"
                fi
            fi
        done
        
        if [ $test_failed -eq 0 ]; then
            print_success "All tests passed!"
        else
            print_error "Some tests failed. See the output above for details."
        fi
    fi
fi

# Return to original directory
cd "$ORIGINAL_DIR"

# Show success message
print_success "Build completed successfully!"

# Generate documentation if requested
if [ $BUILD_DOCS -eq 1 ]; then
    print_status "Generating Doxygen documentation..."
    
    # Create docs directory if it doesn't exist
    if [ ! -d "docs" ]; then
        mkdir -p docs
    fi
    
    # Clean docs if requested
    if [ $CLEAN_DOCS -eq 1 ]; then
        print_status "Cleaning documentation directory..."
        rm -rf docs/*
    fi
    
    # Check if doxygen is installed
    if ! command_exists doxygen; then
        print_error "Doxygen is not installed. Please install it to generate documentation."
        exit 1
    fi
    
    # Run doxygen
    doxygen Doxyfile
    
    # Check if documentation generation was successful
    if [ $? -ne 0 ]; then
        print_error "Documentation generation failed. See the output above for details."
    else
        print_success "Documentation generated successfully in the docs directory!"
    fi
fi

# Final success message
echo ""
echo -e "${BOLD}${GREEN}============================================${NC}"
echo -e "${BOLD}${GREEN}      Logger System Build Complete         ${NC}"
echo -e "${BOLD}${GREEN}============================================${NC}"

if [ -d "build/bin" ]; then
    echo -e "${CYAN}Available executables:${NC}"
    ls -la build/bin/
fi

echo ""
echo -e "${CYAN}Build Configuration:${NC}"
echo -e "  ${BOLD}Build type:${NC} $BUILD_TYPE"
echo -e "  ${BOLD}Target:${NC} $TARGET"
echo -e "  ${BOLD}Compiler:${NC} $CC_COMPILER / $CXX_COMPILER"
echo -e "  ${BOLD}Cores used:${NC} $BUILD_CORES"

if [ $BUILD_BENCHMARKS -eq 1 ]; then
    echo -e "  ${BOLD}Benchmarks:${NC} Enabled"
fi

if [ $BUILD_DOCS -eq 1 ]; then
    echo -e "  ${BOLD}Documentation:${NC} Generated"
fi

exit 0