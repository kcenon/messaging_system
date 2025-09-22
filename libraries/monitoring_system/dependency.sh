#!/bin/bash

# Monitoring System Dependency Installation Script

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Display banner
echo -e "${BOLD}${BLUE}============================================${NC}"
echo -e "${BOLD}${BLUE}  Monitoring System Dependency Installer   ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"

# Function to print messages
print_status() {
    echo -e "${BOLD}${BLUE}[STATUS]${NC} $1"
}

print_success() {
    echo -e "${BOLD}${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${BOLD}${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${BOLD}${YELLOW}[WARNING]${NC} $1"
}

# Store original directory
ORIGINAL_DIR=$(pwd)

# Detect OS and install system dependencies
print_status "Detecting operating system..."

if [ "$(uname)" == "Darwin" ]; then
    print_status "macOS detected. Installing dependencies via Homebrew..."
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        print_error "Homebrew is not installed. Please install it first:"
        echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    # Update Homebrew
    print_status "Updating Homebrew..."
    brew update
    
    # Install dependencies
    print_status "Installing build tools..."
    brew install pkg-config cmake doxygen ninja
    brew install autoconf automake autoconf-archive python3
    
    print_success "macOS dependencies installed successfully"
    
elif [ "$(uname)" == "Linux" ]; then
    print_status "Linux detected. Installing dependencies via apt..."
    
    # Check if running as root or with sudo
    if [ "$EUID" -ne 0 ]; then
        print_warning "This script needs sudo privileges to install packages."
        print_status "Re-running with sudo..."
        exec sudo "$0" "$@"
    fi
    
    # Update package list
    print_status "Updating package list..."
    apt update
    
    # Upgrade existing packages
    print_status "Upgrading existing packages..."
    apt upgrade -y
    
    # Install build essentials
    print_status "Installing build essentials..."
    apt install -y cmake build-essential gdb doxygen
    
    # Install additional tools
    print_status "Installing additional tools..."
    apt-get install -y curl zip unzip tar
    apt-get install -y pkg-config ninja-build python3
    apt-get install -y autoconf automake autoconf-archive
    
    # Check for ARM64 architecture
    if [ $(uname -m) == "aarch64" ]; then
        export VCPKG_FORCE_SYSTEM_BINARIES=arm
        print_status "ARM64 platform detected, setting VCPKG_FORCE_SYSTEM_BINARIES=arm"
    fi
    
    print_success "Linux dependencies installed successfully"
    
else
    print_error "Unsupported operating system: $(uname)"
    exit 1
fi

# Check if doxygen is installed
print_status "Checking doxygen installation..."
if command -v doxygen &> /dev/null; then
    DOXYGEN_VERSION=$(doxygen --version)
    print_success "Doxygen $DOXYGEN_VERSION is installed"
else
    print_warning "Doxygen is not installed. Documentation generation will not be available."
fi

# Setup vcpkg
print_status "Setting up vcpkg..."

# Go to parent directory
cd ..

# Check if vcpkg already exists
if [ ! -d "./vcpkg/" ]; then
    print_status "Cloning vcpkg repository..."
    git clone https://github.com/microsoft/vcpkg.git
    if [ $? -ne 0 ]; then
        print_error "Failed to clone vcpkg repository"
        cd "$ORIGINAL_DIR"
        exit 1
    fi
else
    print_status "vcpkg directory already exists, updating..."
fi

# Enter vcpkg directory
cd vcpkg

# Update vcpkg
print_status "Updating vcpkg..."
git pull

# Bootstrap vcpkg
print_status "Bootstrapping vcpkg..."
if [ "$(uname)" == "Darwin" ] || [ "$(uname)" == "Linux" ]; then
    ./bootstrap-vcpkg.sh
    if [ $? -ne 0 ]; then
        print_error "Failed to bootstrap vcpkg"
        cd "$ORIGINAL_DIR"
        exit 1
    fi
else
    print_error "Unsupported platform for vcpkg bootstrap"
    cd "$ORIGINAL_DIR"
    exit 1
fi

print_success "vcpkg setup completed successfully"

# Return to original directory
cd "$ORIGINAL_DIR"

# Install vcpkg packages for Monitoring System
print_status "Installing vcpkg packages for Monitoring System..."

# Create vcpkg response file if it doesn't exist
if [ ! -f "vcpkg.json" ]; then
    print_status "Creating vcpkg.json..."
    cat > vcpkg.json << 'EOF'
{
  "name": "monitoring-system",
  "version": "1.0.0",
  "description": "Real-time performance monitoring system",
  "dependencies": [
    "gtest",
    "benchmark"
  ]
}
EOF
fi

# Install dependencies via vcpkg
print_status "Installing vcpkg dependencies..."
../vcpkg/vcpkg install

if [ $? -eq 0 ]; then
    print_success "All vcpkg dependencies installed successfully"
else
    print_error "Failed to install some vcpkg dependencies"
    exit 1
fi

# Display summary
echo -e "\n${BOLD}${GREEN}============================================${NC}"
echo -e "${BOLD}${GREEN}     Dependency Installation Complete      ${NC}"
echo -e "${BOLD}${GREEN}============================================${NC}"

print_status "Summary of installed components:"
echo -e "  ${CYAN}Build Tools:${NC} cmake, ninja/make, pkg-config"
echo -e "  ${CYAN}Compilers:${NC} g++/clang++ (system default)"
echo -e "  ${CYAN}Documentation:${NC} doxygen"
echo -e "  ${CYAN}Package Manager:${NC} vcpkg"
echo -e "  ${CYAN}C++ Libraries:${NC} gtest, benchmark"

print_status "Next steps:"
echo "  1. Run './build.sh' to build the Monitoring System"
echo "  2. Run './build.sh --help' to see all build options"
echo "  3. Run './build.sh --tests' to build and run tests"

exit 0