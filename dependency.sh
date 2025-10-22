#!/bin/bash

# Messaging System v2.0 Dependency Installation Script
# Installs required dependencies for building the messaging system

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
echo -e "${BOLD}${BLUE}  Messaging System v2.0 Dependency Setup   ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"
echo ""

# Status messages
print_status() { echo -e "${BOLD}${BLUE}[STATUS]${NC} $1"; }
print_success() { echo -e "${BOLD}${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${BOLD}${RED}[ERROR]${NC} $1"; }
print_warning() { echo -e "${BOLD}${YELLOW}[WARNING]${NC} $1"; }

# Check if command exists
command_exists() {
    command -v "$1" &> /dev/null
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ -f /etc/os-release ]; then
            . /etc/os-release
            OS=$ID
            OS_VERSION=$VERSION_ID
        else
            OS="unknown-linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        OS_VERSION=$(sw_vers -productVersion)
    else
        OS="unknown"
    fi

    print_status "Detected OS: $OS $OS_VERSION"
}

# Install core dependencies (cmake, compiler, git)
install_core_dependencies() {
    print_status "Installing core dependencies..."

    case $OS in
        ubuntu|debian)
            print_status "Using apt package manager"
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                cmake \
                git \
                ninja-build \
                gcc-11 \
                g++-11 \
                pkg-config

            # Set g++-11 as default if available
            if command_exists g++-11; then
                sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100
                sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100
            fi
            ;;

        fedora|rhel|centos)
            print_status "Using dnf/yum package manager"
            sudo dnf install -y \
                gcc \
                gcc-c++ \
                cmake \
                git \
                ninja-build \
                pkg-config \
                || sudo yum install -y \
                gcc \
                gcc-c++ \
                cmake \
                git \
                ninja-build \
                pkg-config
            ;;

        arch|manjaro)
            print_status "Using pacman package manager"
            sudo pacman -Sy --noconfirm \
                base-devel \
                cmake \
                git \
                ninja \
                gcc \
                pkg-config
            ;;

        macos)
            print_status "Using Homebrew package manager"
            if ! command_exists brew; then
                print_error "Homebrew not found. Install from https://brew.sh"
                exit 1
            fi

            brew install \
                cmake \
                git \
                ninja \
                pkg-config

            # Install Xcode Command Line Tools if not present
            if ! xcode-select -p &> /dev/null; then
                print_status "Installing Xcode Command Line Tools..."
                xcode-select --install
            fi
            ;;

        *)
            print_error "Unsupported OS: $OS"
            print_warning "Please install manually: cmake (>=3.16), g++/clang++ with C++20 support, git, ninja"
            exit 1
            ;;
    esac

    print_success "Core dependencies installed"
}

# Install optional dependencies
install_optional_dependencies() {
    print_status "Installing optional dependencies..."

    case $OS in
        ubuntu|debian)
            sudo apt-get install -y \
                libyaml-cpp-dev \
                libpq-dev \
                libssl-dev \
                libfmt-dev
            ;;

        fedora|rhel|centos)
            sudo dnf install -y \
                yaml-cpp-devel \
                postgresql-devel \
                openssl-devel \
                fmt-devel \
                || sudo yum install -y \
                yaml-cpp-devel \
                postgresql-devel \
                openssl-devel
            ;;

        arch|manjaro)
            sudo pacman -Sy --noconfirm \
                yaml-cpp \
                postgresql-libs \
                openssl \
                fmt
            ;;

        macos)
            brew install \
                yaml-cpp \
                postgresql \
                openssl \
                fmt
            ;;

        *)
            print_warning "Cannot install optional dependencies on $OS"
            ;;
    esac

    print_success "Optional dependencies installed"
}

# Check external systems
check_external_systems() {
    print_status "Checking external systems availability..."

    local systems=(
        "CommonSystem:https://github.com/kcenon/common_system.git"
        "ThreadSystem:https://github.com/kcenon/thread_system.git"
        "LoggerSystem:https://github.com/kcenon/logger_system.git"
        "MonitoringSystem:https://github.com/kcenon/monitoring_system.git"
        "ContainerSystem:https://github.com/kcenon/container_system.git"
        "DatabaseSystem:https://github.com/kcenon/database_system.git"
        "NetworkSystem:https://github.com/kcenon/network_system.git"
    )

    for system_info in "${systems[@]}"; do
        local name="${system_info%%:*}"
        local url="${system_info#*:}"

        if git ls-remote "$url" HEAD &> /dev/null; then
            print_success "  ✓ $name accessible"
        else
            print_warning "  ✗ $name not accessible"
        fi
    done
}

# Setup local external systems (optional)
setup_local_external_systems() {
    print_status "Do you want to clone external systems locally for development?"
    echo "This will clone all 7 external systems to ../common_system, ../thread_system, etc."
    read -p "Clone external systems? [y/N]: " -n 1 -r
    echo

    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "Skipping local external system setup"
        return
    fi

    local parent_dir="$(dirname "$(pwd)")"
    print_status "Cloning to $parent_dir"

    cd "$parent_dir" || exit 1

    local systems=(
        "common_system"
        "thread_system"
        "logger_system"
        "monitoring_system"
        "container_system"
        "database_system"
        "network_system"
    )

    for system in "${systems[@]}"; do
        if [ -d "$system" ]; then
            print_warning "  $system already exists, skipping"
        else
            print_status "  Cloning $system..."
            git clone "https://github.com/kcenon/${system}.git"

            if [ $? -eq 0 ]; then
                print_success "  ✓ $system cloned"

                # Run dependency script if it exists
                if [ -f "$system/dependency.sh" ]; then
                    print_status "  Installing $system dependencies..."
                    cd "$system"
                    chmod +x dependency.sh
                    ./dependency.sh
                    cd "$parent_dir"
                fi
            else
                print_error "  ✗ Failed to clone $system"
            fi
        fi
    done

    cd - > /dev/null
}

# Verify installation
verify_installation() {
    print_status "Verifying installation..."

    local all_ok=true

    # Check CMake version
    if command_exists cmake; then
        local cmake_version=$(cmake --version | head -n1 | awk '{print $3}')
        local required_version="3.16"

        if [ "$(printf '%s\n' "$required_version" "$cmake_version" | sort -V | head -n1)" = "$required_version" ]; then
            print_success "  ✓ CMake $cmake_version (>= $required_version)"
        else
            print_error "  ✗ CMake $cmake_version is too old (need >= $required_version)"
            all_ok=false
        fi
    else
        print_error "  ✗ CMake not found"
        all_ok=false
    fi

    # Check C++20 compiler
    if command_exists g++; then
        local gcc_version=$(g++ --version | head -n1 | grep -oE '[0-9]+\.[0-9]+' | head -1)
        print_success "  ✓ g++ $gcc_version"
    elif command_exists clang++; then
        local clang_version=$(clang++ --version | head -n1 | grep -oE '[0-9]+\.[0-9]+' | head -1)
        print_success "  ✓ clang++ $clang_version"
    else
        print_error "  ✗ No C++20 compatible compiler found"
        all_ok=false
    fi

    # Check git
    if command_exists git; then
        local git_version=$(git --version | awk '{print $3}')
        print_success "  ✓ git $git_version"
    else
        print_error "  ✗ git not found"
        all_ok=false
    fi

    # Check ninja (optional but recommended)
    if command_exists ninja; then
        local ninja_version=$(ninja --version)
        print_success "  ✓ ninja $ninja_version"
    else
        print_warning "  ! ninja not found (optional, but recommended for faster builds)"
    fi

    # Check yaml-cpp (optional)
    if pkg-config --exists yaml-cpp; then
        local yaml_version=$(pkg-config --modversion yaml-cpp)
        print_success "  ✓ yaml-cpp $yaml_version"
    else
        print_warning "  ! yaml-cpp not found (ConfigLoader will be disabled)"
    fi

    echo ""
    if [ "$all_ok" = true ]; then
        print_success "All required dependencies verified!"
        return 0
    else
        print_error "Some required dependencies are missing"
        return 1
    fi
}

# Main installation flow
main() {
    detect_os
    echo ""

    install_core_dependencies
    echo ""

    install_optional_dependencies
    echo ""

    check_external_systems
    echo ""

    setup_local_external_systems
    echo ""

    verify_installation
    echo ""

    # Print next steps
    echo -e "${BOLD}${GREEN}============================================${NC}"
    echo -e "${BOLD}${GREEN}        Installation Complete!             ${NC}"
    echo -e "${BOLD}${GREEN}============================================${NC}"
    echo ""
    echo -e "${CYAN}Next steps:${NC}"
    echo "  1. Build the project:"
    echo "     ./build.sh dev-fetchcontent --tests"
    echo ""
    echo "  2. Or use a different preset:"
    echo "     ./build.sh release --examples"
    echo ""
    echo "  3. Run tests:"
    echo "     ctest --preset default"
    echo ""
    echo "  4. See available presets:"
    echo "     cmake --list-presets"
    echo ""
    echo -e "${CYAN}Build presets available:${NC}"
    echo "  default           - Production with find_package"
    echo "  dev-fetchcontent  - Development with FetchContent"
    echo "  debug             - Debug build"
    echo "  release           - Optimized release"
    echo "  asan/tsan/ubsan   - Sanitizer builds"
    echo ""
}

# Run main installation
main

exit 0
