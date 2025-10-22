#!/bin/bash

# Automated build script that temporarily moves local systems
# to avoid target conflicts with FetchContent

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SOURCES_DIR="$(dirname "$PROJECT_DIR")"

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

# Systems that may conflict
SYSTEMS=(
    "common_system"
    "thread_system"
    "logger_system"
    "monitoring_system"
    "container_system"
    "database_system"
    "network_system"
)

MOVED_SYSTEMS=()

# Function to backup systems
backup_systems() {
    print_status "Backing up local systems that may conflict with FetchContent..."

    cd "$SOURCES_DIR"

    for sys in "${SYSTEMS[@]}"; do
        if [ -d "$sys" ] && [ ! -d "${sys}.build_backup" ]; then
            print_status "  Moving $sys to ${sys}.build_backup"
            mv "$sys" "${sys}.build_backup"
            MOVED_SYSTEMS+=("$sys")
        fi
    done

    if [ ${#MOVED_SYSTEMS[@]} -eq 0 ]; then
        print_status "  No systems needed to be moved"
    else
        print_success "  Backed up ${#MOVED_SYSTEMS[@]} systems"
    fi
}

# Function to restore systems
restore_systems() {
    print_status "Restoring local systems..."

    cd "$SOURCES_DIR"

    for sys in "${MOVED_SYSTEMS[@]}"; do
        if [ -d "${sys}.build_backup" ]; then
            print_status "  Restoring $sys"
            mv "${sys}.build_backup" "$sys"
        fi
    done

    print_success "  Restored ${#MOVED_SYSTEMS[@]} systems"
}

# Trap to ensure systems are restored on exit
trap restore_systems EXIT

# Main build process
main() {
    print_status "Starting automated FetchContent build..."
    echo ""

    # Backup conflicting systems
    backup_systems
    echo ""

    # Run the build
    print_status "Running build.sh..."
    cd "$PROJECT_DIR"

    if ./build.sh "$@"; then
        echo ""
        print_success "Build completed successfully!"
        return 0
    else
        echo ""
        print_error "Build failed. Check errors above."
        return 1
    fi
}

# Run main
main "$@"
EXIT_CODE=$?

echo ""
print_status "Cleanup will restore systems automatically..."

exit $EXIT_CODE
