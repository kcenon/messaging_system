#!/bin/bash

# Thread System Compiler Test Script
# Tests the build system with all available compilers

# Color definitions for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

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

# Display banner
echo -e "${BOLD}${BLUE}============================================${NC}"
echo -e "${BOLD}${BLUE}    Thread System Compiler Test Suite     ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"
echo ""

# Get list of available compilers
print_status "Getting list of available compilers..."
COMPILER_LIST_OUTPUT=$(./build.sh --list-compilers 2>/dev/null | grep -E "^  [0-9]+\)")

if [ -z "$COMPILER_LIST_OUTPUT" ]; then
    print_error "No compilers found or failed to get compiler list"
    exit 1
fi

# Parse compiler names from the output
COMPILERS=()
while IFS= read -r line; do
    # Extract compiler name from lines like "  1) GCC: ..."
    if [[ $line =~ ^\ \ [0-9]+\)\ (.+):\ .* ]]; then
        # For this we need to map the display name to actual compiler command
        case "${BASH_REMATCH[1]}" in
            "GCC")
                COMPILERS+=("g++")
                ;;
            "Clang")
                COMPILERS+=("clang++")
                ;;
            "Apple Clang")
                COMPILERS+=("/usr/bin/clang++")
                ;;
            "GCC-"*)
                # Extract version number
                if [[ $line =~ GCC-([0-9]+) ]]; then
                    COMPILERS+=("g++-${BASH_REMATCH[1]}")
                fi
                ;;
            "Clang-"*)
                # Extract version number
                if [[ $line =~ Clang-([0-9]+) ]]; then
                    COMPILERS+=("clang++-${BASH_REMATCH[1]}")
                fi
                ;;
        esac
    fi
done <<< "$COMPILER_LIST_OUTPUT"

if [ ${#COMPILERS[@]} -eq 0 ]; then
    print_error "Failed to parse compiler list"
    exit 1
fi

print_status "Found ${#COMPILERS[@]} compiler(s): ${COMPILERS[*]}"
echo ""

# Test results tracking
SUCCESSFUL_COMPILERS=()
FAILED_COMPILERS=()
BUILD_FAILED_COMPILERS=()
TEST_FAILED_COMPILERS=()

# Test each compiler
for i in "${!COMPILERS[@]}"; do
    COMPILER="${COMPILERS[i]}"
    COMPILER_NUM=$((i + 1))
    
    echo -e "${BOLD}${CYAN}============================================${NC}"
    echo -e "${BOLD}${CYAN}Testing Compiler $COMPILER_NUM/${#COMPILERS[@]}: $COMPILER${NC}"
    echo -e "${BOLD}${CYAN}============================================${NC}"
    
    # Step 3: Build with clean (reduced cores for stability)
    print_status "Step 3: Building with $COMPILER (clean build)..."
    
    BUILD_SUCCESS=false
    BUILD_OUTPUT_FILE="build_${COMPILER//\//_}_$(date +%s).log"
    
    if ./build.sh --compiler "$COMPILER" --clean --cores 1 > "$BUILD_OUTPUT_FILE" 2>&1; then
        print_success "Build completed successfully"
        BUILD_SUCCESS=true
        rm -f "$BUILD_OUTPUT_FILE"  # Remove log file if build succeeded
    else
        print_error "Build failed with $COMPILER"
        print_warning "Build log saved to: $BUILD_OUTPUT_FILE"
        BUILD_FAILED_COMPILERS+=("$COMPILER")
        
        # For now, we'll continue to next compiler instead of trying to fix
        print_warning "Continuing to next compiler..."
        continue
    fi
    
    # Step 6: Run tests (only if build succeeded)
    if [ "$BUILD_SUCCESS" = true ]; then
        print_status "Step 6: Running tests with $COMPILER..."
        
        TEST_SUCCESS=true
        TEST_OUTPUT_FILE="test_${COMPILER//\//_}_$(date +%s).log"
        
        # Run each test binary directly instead of using build.sh --tests
        cd build || { print_error "Failed to enter build directory"; continue; }
        
        for test_binary in bin/*_unit; do
            if [ -x "$test_binary" ]; then
                test_name=$(basename "$test_binary")
                print_status "Running $test_name..."
                
                if ! ./"$test_binary" >> "../$TEST_OUTPUT_FILE" 2>&1; then
                    print_error "Test $test_name failed"
                    TEST_SUCCESS=false
                fi
            fi
        done
        
        cd .. || { print_error "Failed to return to original directory"; continue; }
        
        if [ "$TEST_SUCCESS" = true ]; then
            print_success "Tests completed successfully"
            rm -f "$TEST_OUTPUT_FILE"  # Remove log file if tests succeeded
        else
            print_error "Tests failed with $COMPILER"
            print_warning "Test log saved to: $TEST_OUTPUT_FILE"
            TEST_FAILED_COMPILERS+=("$COMPILER")
            
            # For now, we'll continue to next compiler instead of trying to fix
            print_warning "Continuing to next compiler..."
            continue
        fi
        
        # If both build and test succeeded
        if [ "$TEST_SUCCESS" = true ]; then
            SUCCESSFUL_COMPILERS+=("$COMPILER")
            print_success "Compiler $COMPILER: ALL TESTS PASSED"
        fi
    fi
    
    echo ""
done

# Final results summary
echo -e "${BOLD}${BLUE}============================================${NC}"
echo -e "${BOLD}${BLUE}           TEST RESULTS SUMMARY            ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"
echo ""

echo -e "${BOLD}Compilers Tested:${NC} ${#COMPILERS[@]}"
echo -e "${BOLD}${GREEN}Successful:${NC} ${#SUCCESSFUL_COMPILERS[@]}"
echo -e "${BOLD}${RED}Build Failed:${NC} ${#BUILD_FAILED_COMPILERS[@]}"
echo -e "${BOLD}${YELLOW}Test Failed:${NC} ${#TEST_FAILED_COMPILERS[@]}"
echo ""

if [ ${#SUCCESSFUL_COMPILERS[@]} -gt 0 ]; then
    echo -e "${BOLD}${GREEN}✅ Successfully tested compilers:${NC}"
    for compiler in "${SUCCESSFUL_COMPILERS[@]}"; do
        echo "   - $compiler"
    done
    echo ""
fi

if [ ${#BUILD_FAILED_COMPILERS[@]} -gt 0 ]; then
    echo -e "${BOLD}${RED}❌ Build failed compilers:${NC}"
    for compiler in "${BUILD_FAILED_COMPILERS[@]}"; do
        echo "   - $compiler"
    done
    echo ""
fi

if [ ${#TEST_FAILED_COMPILERS[@]} -gt 0 ]; then
    echo -e "${BOLD}${YELLOW}⚠️  Test failed compilers:${NC}"
    for compiler in "${TEST_FAILED_COMPILERS[@]}"; do
        echo "   - $compiler"
    done
    echo ""
fi

# Cleanup any remaining log files older than 1 hour
find . -name "build_*.log" -o -name "test_*.log" -mtime +1h -delete 2>/dev/null

# Exit with appropriate code
if [ ${#SUCCESSFUL_COMPILERS[@]} -eq ${#COMPILERS[@]} ]; then
    print_success "All compilers passed successfully!"
    exit 0
elif [ ${#SUCCESSFUL_COMPILERS[@]} -gt 0 ]; then
    print_warning "Some compilers failed, but at least one succeeded"
    exit 1
else
    print_error "All compilers failed!"
    exit 2
fi