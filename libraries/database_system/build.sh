#!/bin/bash

# Database System Build Script
# Advanced C++20 Database System with Multi-Backend Support

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
echo -e "${BOLD}${BLUE}      Database System Build Script         ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"

# Display help information
show_help() {
    echo -e "${BOLD}Usage:${NC} $0 [options]"
    echo ""
    echo -e "${BOLD}Build Options:${NC}"
    echo "  --clean           Perform a clean rebuild by removing the build directory"
    echo "  --debug           Build in debug mode (default is release)"
    echo "  --release         Build in release mode (default)"
    echo ""
    echo -e "${BOLD}Target Options:${NC}"
    echo "  --all             Build all targets (default)"
    echo "  --lib-only        Build only the database library"
    echo "  --samples         Build only the sample applications"
    echo "  --tests           Build and run the unit tests"
    echo ""
    echo -e "${BOLD}Database Options:${NC}"
    echo "  --with-postgresql Enable PostgreSQL support (default)"
    echo "  --no-postgresql   Disable PostgreSQL support"
    echo "  --with-mysql      Enable MySQL support (future)"
    echo "  --with-sqlite     Enable SQLite support (future)"
    echo ""
    echo -e "${BOLD}Feature Options:${NC}"
    echo "  --no-vcpkg        Skip vcpkg and use system libraries only"
    echo "  --use-system-deps Use system-installed dependencies"
    echo ""
    echo -e "${BOLD}General Options:${NC}"
    echo "  --cores N         Use N cores for compilation (default: auto-detect)"
    echo "  --verbose         Show detailed build output"
    echo "  --install         Install after building"
    echo "  --prefix PATH     Installation prefix (default: /usr/local)"
    echo "  --help            Display this help and exit"
    echo ""
    echo -e "${BOLD}Compiler Options:${NC}"
    echo "  --gcc             Use GCC compiler"
    echo "  --clang           Use Clang compiler"
    echo "  --compiler PATH   Use specific compiler"
    echo ""
    echo -e "${BOLD}Examples:${NC}"
    echo "  $0                                    # Default build (Release, with PostgreSQL)"
    echo "  $0 --debug --tests                   # Debug build with tests"
    echo "  $0 --no-postgresql --lib-only        # Library only without PostgreSQL"
    echo "  $0 --clean --release --install       # Clean release build with install"
}

# Default values
BUILD_TYPE="Release"
BUILD_TARGET="all"
CMAKE_ARGS=""
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")
VERBOSE=false
CLEAN=false
USE_VCPKG=true
POSTGRESQL=true
MYSQL=false
SQLITE=false
INSTALL=false
PREFIX="/usr/local"
GENERATOR=""

# Detect system and set defaults
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    CORES=$(sysctl -n hw.ncpu)
    GENERATOR="Ninja"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    CORES=$(nproc)
    GENERATOR="Ninja"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    # Windows
    CORES=$NUMBER_OF_PROCESSORS
    GENERATOR="Visual Studio 17 2022"
fi

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --all)
            BUILD_TARGET="all"
            shift
            ;;
        --lib-only)
            BUILD_TARGET="database"
            CMAKE_ARGS="$CMAKE_ARGS -DBUILD_DATABASE_SAMPLES=OFF -DUSE_UNIT_TEST=OFF"
            shift
            ;;
        --samples)
            BUILD_TARGET="samples"
            CMAKE_ARGS="$CMAKE_ARGS -DBUILD_DATABASE_SAMPLES=ON -DUSE_UNIT_TEST=OFF"
            shift
            ;;
        --tests)
            BUILD_TARGET="tests"
            CMAKE_ARGS="$CMAKE_ARGS -DUSE_UNIT_TEST=ON"
            shift
            ;;
        --with-postgresql)
            POSTGRESQL=true
            shift
            ;;
        --no-postgresql)
            POSTGRESQL=false
            shift
            ;;
        --with-mysql)
            MYSQL=true
            shift
            ;;
        --with-sqlite)
            SQLITE=true
            shift
            ;;
        --no-vcpkg)
            USE_VCPKG=false
            shift
            ;;
        --use-system-deps)
            USE_VCPKG=false
            shift
            ;;
        --cores)
            CORES="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --prefix)
            PREFIX="$2"
            shift 2
            ;;
        --gcc)
            CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++"
            shift
            ;;
        --clang)
            CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
            shift
            ;;
        --compiler)
            CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_COMPILER=$2"
            shift 2
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Validation
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}❌ CMake is not installed${NC}"
    exit 1
fi

# Check for Ninja if needed
if [[ "$GENERATOR" == "Ninja" ]] && ! command -v ninja &> /dev/null; then
    echo -e "${YELLOW}⚠️  Ninja not found, using default generator${NC}"
    GENERATOR=""
fi

# Setup build directory
BUILD_DIR="build"
if [[ "$BUILD_TYPE" == "Debug" ]]; then
    BUILD_DIR="build_debug"
fi

echo -e "${CYAN}📋 Build Configuration:${NC}"
echo -e "  Build Type: ${BOLD}$BUILD_TYPE${NC}"
echo -e "  Build Target: ${BOLD}$BUILD_TARGET${NC}"
echo -e "  PostgreSQL: ${BOLD}$([ "$POSTGRESQL" = true ] && echo "ON" || echo "OFF")${NC}"
echo -e "  MySQL: ${BOLD}$([ "$MYSQL" = true ] && echo "ON" || echo "OFF")${NC}"
echo -e "  SQLite: ${BOLD}$([ "$SQLITE" = true ] && echo "ON" || echo "OFF")${NC}"
echo -e "  Use vcpkg: ${BOLD}$([ "$USE_VCPKG" = true ] && echo "YES" || echo "NO")${NC}"
echo -e "  Cores: ${BOLD}$CORES${NC}"
echo -e "  Generator: ${BOLD}${GENERATOR:-Default}${NC}"
echo ""

# Clean build directory if requested
if [[ "$CLEAN" == true ]]; then
    echo -e "${YELLOW}🧹 Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure CMake options
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
CMAKE_ARGS="$CMAKE_ARGS -DUSE_POSTGRESQL=$([ "$POSTGRESQL" = true ] && echo "ON" || echo "OFF")"
CMAKE_ARGS="$CMAKE_ARGS -DUSE_MYSQL=$([ "$MYSQL" = true ] && echo "ON" || echo "OFF")"
CMAKE_ARGS="$CMAKE_ARGS -DUSE_SQLITE=$([ "$SQLITE" = true ] && echo "ON" || echo "OFF")"
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

if [[ "$INSTALL" == true ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=$PREFIX"
fi

if [[ "$USE_VCPKG" == true ]]; then
    # Check for vcpkg
    VCPKG_ROOT=""
    if [[ -d "vcpkg" ]]; then
        VCPKG_ROOT="$(pwd)/vcpkg"
    elif [[ -d "../vcpkg" ]]; then
        VCPKG_ROOT="$(pwd)/../vcpkg"
    elif [[ -n "$VCPKG_ROOT" ]]; then
        # Use environment variable
        true
    else
        echo -e "${YELLOW}⚠️  vcpkg not found, will try to install...${NC}"
        if ./dependency.sh; then
            VCPKG_ROOT="$(pwd)/vcpkg"
        else
            echo -e "${RED}❌ Failed to install vcpkg${NC}"
            USE_VCPKG=false
        fi
    fi

    if [[ "$USE_VCPKG" == true && -n "$VCPKG_ROOT" ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        echo -e "${GREEN}✅ Using vcpkg from: $VCPKG_ROOT${NC}"
    fi
fi

if [[ -n "$GENERATOR" ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -G \"$GENERATOR\""
fi

# Configure
echo -e "${BLUE}⚙️  Configuring...${NC}"
if [[ "$VERBOSE" == true ]]; then
    echo "CMake command: cmake -B $BUILD_DIR $CMAKE_ARGS"
fi

cd "$BUILD_DIR" || exit 1
if ! eval "cmake .. $CMAKE_ARGS"; then
    echo -e "${RED}❌ Configuration failed${NC}"
    exit 1
fi
cd ..

# Build
echo -e "${BLUE}🔨 Building...${NC}"
BUILD_ARGS="--config $BUILD_TYPE --parallel $CORES"

if [[ "$VERBOSE" == true ]]; then
    BUILD_ARGS="$BUILD_ARGS --verbose"
fi

if [[ "$BUILD_TARGET" != "all" ]]; then
    BUILD_ARGS="$BUILD_ARGS --target $BUILD_TARGET"
fi

if ! cmake --build "$BUILD_DIR" $BUILD_ARGS; then
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

# Run tests if requested
if [[ "$BUILD_TARGET" == "tests" || "$BUILD_TARGET" == "all" ]]; then
    echo -e "${BLUE}🧪 Running tests...${NC}"
    cd "$BUILD_DIR"
    if ! ctest --output-on-failure -C "$BUILD_TYPE"; then
        echo -e "${RED}❌ Tests failed${NC}"
        cd ..
        exit 1
    fi
    cd ..
fi

# Install if requested
if [[ "$INSTALL" == true ]]; then
    echo -e "${BLUE}📦 Installing...${NC}"
    if ! cmake --install "$BUILD_DIR" --config "$BUILD_TYPE"; then
        echo -e "${RED}❌ Installation failed${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}✅ Build completed successfully!${NC}"
echo -e "${CYAN}📁 Build artifacts location: $(pwd)/$BUILD_DIR${NC}"

if [[ "$BUILD_TARGET" == "all" || "$BUILD_TARGET" == "samples" ]]; then
    echo -e "${CYAN}🎯 Sample programs:${NC}"
    find "$BUILD_DIR" -name "*_usage*" -o -name "*_demo*" -o -name "*_example*" | head -5
fi

if [[ "$INSTALL" == true ]]; then
    echo -e "${CYAN}📦 Installed to: $PREFIX${NC}"
fi