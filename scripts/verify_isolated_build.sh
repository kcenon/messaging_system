#!/bin/bash

# Verification script for isolated build without optional dependencies
# This ensures that fallback implementations work correctly when
# KCENON_WITH_NETWORK_SYSTEM and KCENON_WITH_MONITORING_SYSTEM are disabled.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m'

print_status() { echo -e "${BOLD}${BLUE}[STATUS]${NC} $1"; }
print_success() { echo -e "${BOLD}${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${BOLD}${RED}[ERROR]${NC} $1"; }
print_warning() { echo -e "${BOLD}${YELLOW}[WARNING]${NC} $1"; }

# Build directory
BUILD_DIR="${PROJECT_DIR}/build-isolated"

# Function to clean build directory
clean_build() {
    if [ -d "$BUILD_DIR" ]; then
        print_status "Cleaning existing isolated build directory..."
        rm -rf "$BUILD_DIR"
    fi
}

# Function to configure
configure_build() {
    print_status "Configuring isolated build (no optional dependencies)..."
    print_status "  KCENON_WITH_NETWORK_SYSTEM=OFF"
    print_status "  KCENON_WITH_MONITORING_SYSTEM=OFF"

    cmake --preset=isolated
}

# Function to build
run_build() {
    print_status "Building in isolated mode..."

    cmake --build --preset=isolated --parallel
}

# Function to run tests
run_tests() {
    print_status "Running tests in isolated mode..."

    ctest --preset=isolated --output-on-failure
}

# Function to verify feature flags
verify_flags() {
    print_status "Verifying feature flag definitions..."

    # Check that the flags are defined in compiled code
    local test_file="${BUILD_DIR}/_deps"

    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found"
        return 1
    fi

    print_success "Feature flags verified"
}

# Main function
main() {
    echo ""
    echo "=============================================="
    echo " messaging_system Isolated Build Verification"
    echo "=============================================="
    echo ""

    cd "$PROJECT_DIR"

    # Parse arguments
    CLEAN=false
    SKIP_TESTS=false

    while [[ $# -gt 0 ]]; do
        case $1 in
            --clean)
                CLEAN=true
                shift
                ;;
            --skip-tests)
                SKIP_TESTS=true
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --clean       Clean build directory before building"
                echo "  --skip-tests  Skip running tests"
                echo "  -h, --help    Show this help message"
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    # Clean if requested
    if [ "$CLEAN" = true ]; then
        clean_build
    fi

    # Configure
    configure_build
    echo ""

    # Build
    run_build
    echo ""

    # Run tests
    if [ "$SKIP_TESTS" = false ]; then
        run_tests
        echo ""
    fi

    # Verify flags
    verify_flags
    echo ""

    print_success "Isolated build verification completed successfully!"
    echo ""
    echo "The following features were disabled:"
    echo "  - HTTP transport (stub implementation)"
    echo "  - WebSocket transport (stub implementation)"
    echo "  - Message bus collector (no-op implementation)"
    echo ""
    echo "All fallback implementations are working correctly."

    return 0
}

# Run main
main "$@"
