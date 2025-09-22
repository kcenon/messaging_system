#!/bin/bash

# Script to run code coverage analysis for thread_system

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Thread System Code Coverage Analysis${NC}"
echo "======================================"

# Check if lcov is installed
if ! command -v lcov &> /dev/null; then
    echo -e "${RED}Error: lcov is not installed${NC}"
    echo "Please install lcov:"
    echo "  macOS: brew install lcov"
    echo "  Linux: apt-get install lcov"
    exit 1
fi

# Check if genhtml is installed
if ! command -v genhtml &> /dev/null; then
    echo -e "${RED}Error: genhtml is not installed${NC}"
    echo "Please install lcov (includes genhtml):"
    echo "  macOS: brew install lcov"
    echo "  Linux: apt-get install lcov"
    exit 1
fi

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Create build directory for coverage
BUILD_DIR="${PROJECT_ROOT}/build-coverage"
COVERAGE_DIR="${BUILD_DIR}/coverage"

echo "Project root: ${PROJECT_ROOT}"
echo "Build directory: ${BUILD_DIR}"
echo ""

# Clean previous build
if [ -d "${BUILD_DIR}" ]; then
    echo -e "${YELLOW}Cleaning previous coverage build...${NC}"
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure with coverage enabled
echo -e "${GREEN}Configuring with coverage enabled...${NC}"
cmake -DENABLE_COVERAGE=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
      "${PROJECT_ROOT}"

# Build the project
echo -e "${GREEN}Building project...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

# Run tests to generate coverage data
echo -e "${GREEN}Running tests...${NC}"
ctest --output-on-failure || true

# Generate coverage report
echo -e "${GREEN}Generating coverage report...${NC}"
cmake --build . --target coverage

# Check if report was generated
if [ -f "${COVERAGE_DIR}/all/index.html" ]; then
    echo ""
    echo -e "${GREEN}Coverage report generated successfully!${NC}"
    echo "Report location: ${COVERAGE_DIR}/all/index.html"
    
    # Try to open in browser
    if command -v open &> /dev/null; then
        echo -e "${YELLOW}Opening report in browser...${NC}"
        open "${COVERAGE_DIR}/all/index.html"
    elif command -v xdg-open &> /dev/null; then
        echo -e "${YELLOW}Opening report in browser...${NC}"
        xdg-open "${COVERAGE_DIR}/all/index.html"
    else
        echo "To view the report, open: ${COVERAGE_DIR}/all/index.html"
    fi
else
    echo -e "${RED}Error: Coverage report was not generated${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}Coverage analysis complete!${NC}"