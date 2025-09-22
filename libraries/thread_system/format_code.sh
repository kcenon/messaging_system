#!/bin/bash

# Logger System Code Formatting Script
# Applies clang-format to all C++ source files

# Color definitions
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}===============================================${NC}"
echo -e "${CYAN}    Logger System Code Formatting Script      ${NC}" 
echo -e "${CYAN}===============================================${NC}"

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo -e "${YELLOW}[WARNING]${NC} clang-format not found. Please install it:"
    echo "  macOS: brew install clang-format"
    echo "  Ubuntu: sudo apt install clang-format"
    echo "  CentOS: sudo yum install clang-tools-extra"
    echo ""
    echo -e "${CYAN}[INFO]${NC} .clang-format configuration is already in place."
    echo -e "${CYAN}[INFO]${NC} Run this script again after installing clang-format."
    exit 1
fi

echo -e "${CYAN}[INFO]${NC} Formatting C++ source files..."

# Find and format all C++ files
FORMATTED_COUNT=0

# Format header files
for file in $(find . -name "*.h" -o -name "*.hpp" | grep -v "./build/"); do
    echo -e "${YELLOW}[FORMAT]${NC} $file"
    clang-format -i "$file"
    ((FORMATTED_COUNT++))
done

# Format source files  
for file in $(find . -name "*.cpp" -o -name "*.cc" | grep -v "./build/"); do
    echo -e "${YELLOW}[FORMAT]${NC} $file"
    clang-format -i "$file" 
    ((FORMATTED_COUNT++))
done

echo ""
echo -e "${GREEN}[SUCCESS]${NC} Formatted $FORMATTED_COUNT files"
echo -e "${GREEN}[SUCCESS]${NC} Code formatting completed!"

# Show formatting rules summary
echo ""
echo -e "${CYAN}[INFO]${NC} Applied formatting rules:"
echo "  - Based on Google Style"
echo "  - Indent: 4 spaces"
echo "  - Column limit: 100 characters"
echo "  - Pointer alignment: Left"
echo "  - C++20 standard"
echo ""
echo -e "${CYAN}[INFO]${NC} To check formatting without applying:"
echo "  clang-format --dry-run --Werror <file>"