#!/bin/bash

# Database System Dependency Management Script
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
echo -e "${BOLD}${BLUE}    Database System Dependency Installer   ${NC}"
echo -e "${BOLD}${BLUE}============================================${NC}"

# Display help information
show_help() {
    echo -e "${BOLD}Usage:${NC} $0 [options]"
    echo ""
    echo -e "${BOLD}Installation Options:${NC}"
    echo "  --system-only     Install only system dependencies (no vcpkg)"
    echo "  --vcpkg-only      Install only vcpkg and its packages"
    echo "  --all             Install all dependencies (default)"
    echo ""
    echo -e "${BOLD}Database Options:${NC}"
    echo "  --with-postgresql Install PostgreSQL development libraries"
    echo "  --with-mysql      Install MySQL development libraries"
    echo "  --with-sqlite     Install SQLite development libraries"
    echo "  --no-databases    Skip database-specific dependencies"
    echo ""
    echo -e "${BOLD}Development Tools:${NC}"
    echo "  --with-docs       Install documentation tools (Doxygen, Sphinx)"
    echo "  --with-debug      Install debugging tools (GDB, Valgrind)"
    echo "  --with-profiling  Install profiling tools"
    echo "  --minimal         Install only essential dependencies"
    echo ""
    echo -e "${BOLD}General Options:${NC}"
    echo "  --force           Force reinstallation of all packages"
    echo "  --dry-run         Show what would be installed without installing"
    echo "  --verbose         Show detailed installation output"
    echo "  --help            Display this help and exit"
    echo ""
    echo -e "${BOLD}Examples:${NC}"
    echo "  $0                                    # Install all dependencies"
    echo "  $0 --with-postgresql --with-docs     # Install with PostgreSQL and docs"
    echo "  $0 --minimal --vcpkg-only            # Minimal vcpkg installation"
    echo "  $0 --dry-run                         # Preview installation"
}

# Default values
INSTALL_SYSTEM=true
INSTALL_VCPKG=true
INSTALL_POSTGRESQL=false
INSTALL_MYSQL=false
INSTALL_SQLITE=false
INSTALL_DOCS=false
INSTALL_DEBUG=false
INSTALL_PROFILING=false
MINIMAL=false
FORCE=false
DRY_RUN=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --system-only)
            INSTALL_SYSTEM=true
            INSTALL_VCPKG=false
            shift
            ;;
        --vcpkg-only)
            INSTALL_SYSTEM=false
            INSTALL_VCPKG=true
            shift
            ;;
        --all)
            INSTALL_SYSTEM=true
            INSTALL_VCPKG=true
            shift
            ;;
        --with-postgresql)
            INSTALL_POSTGRESQL=true
            shift
            ;;
        --with-mysql)
            INSTALL_MYSQL=true
            shift
            ;;
        --with-sqlite)
            INSTALL_SQLITE=true
            shift
            ;;
        --no-databases)
            INSTALL_POSTGRESQL=false
            INSTALL_MYSQL=false
            INSTALL_SQLITE=false
            shift
            ;;
        --with-docs)
            INSTALL_DOCS=true
            shift
            ;;
        --with-debug)
            INSTALL_DEBUG=true
            shift
            ;;
        --with-profiling)
            INSTALL_PROFILING=true
            shift
            ;;
        --minimal)
            MINIMAL=true
            shift
            ;;
        --force)
            FORCE=true
            shift
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Function to run command with dry-run support
run_command() {
    local cmd="$1"
    local description="$2"

    if [[ "$DRY_RUN" == true ]]; then
        echo -e "${CYAN}[DRY RUN]${NC} Would run: $cmd"
        return 0
    fi

    if [[ "$VERBOSE" == true ]]; then
        echo -e "${BLUE}Running:${NC} $cmd"
    fi

    eval "$cmd"
    local result=$?

    if [[ $result -eq 0 ]]; then
        echo -e "${GREEN}✅ $description${NC}"
    else
        echo -e "${RED}❌ Failed: $description${NC}"
        return $result
    fi
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        PACKAGE_MANAGER="brew"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get &> /dev/null; then
            OS="ubuntu"
            PACKAGE_MANAGER="apt"
        elif command -v yum &> /dev/null; then
            OS="centos"
            PACKAGE_MANAGER="yum"
        elif command -v pacman &> /dev/null; then
            OS="arch"
            PACKAGE_MANAGER="pacman"
        else
            echo -e "${RED}❌ Unsupported Linux distribution${NC}"
            exit 1
        fi
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
        PACKAGE_MANAGER="choco"
    else
        echo -e "${RED}❌ Unsupported operating system: $OSTYPE${NC}"
        exit 1
    fi

    echo -e "${CYAN}📋 Detected OS: ${BOLD}$OS${NC} (Package manager: $PACKAGE_MANAGER)"
}

# Install system dependencies
install_system_dependencies() {
    if [[ "$INSTALL_SYSTEM" != true ]]; then
        return 0
    fi

    echo -e "${BLUE}📦 Installing system dependencies...${NC}"

    case "$OS" in
        "macos")
            # Essential build tools
            if [[ "$MINIMAL" != true ]]; then
                run_command "brew install cmake ninja pkg-config curl wget git" "Essential build tools"
                run_command "brew install llvm" "LLVM/Clang compiler"
            else
                run_command "brew install cmake ninja" "Minimal build tools"
            fi

            # Database libraries
            if [[ "$INSTALL_POSTGRESQL" == true ]]; then
                run_command "brew install postgresql libpq" "PostgreSQL development libraries"
            fi
            if [[ "$INSTALL_MYSQL" == true ]]; then
                run_command "brew install mysql mysql-client" "MySQL development libraries"
            fi
            if [[ "$INSTALL_SQLITE" == true ]]; then
                run_command "brew install sqlite" "SQLite development libraries"
            fi

            # Development tools
            if [[ "$INSTALL_DEBUG" == true ]]; then
                run_command "brew install gdb lldb" "Debugging tools"
            fi
            if [[ "$INSTALL_PROFILING" == true ]]; then
                run_command "brew install valgrind gperftools" "Profiling tools"
            fi
            if [[ "$INSTALL_DOCS" == true ]]; then
                run_command "brew install doxygen graphviz sphinx-doc" "Documentation tools"
            fi
            ;;

        "ubuntu")
            # Update package list
            run_command "sudo apt update" "Package list update"

            # Essential build tools
            if [[ "$MINIMAL" != true ]]; then
                run_command "sudo apt install -y build-essential cmake ninja-build pkg-config curl wget git" "Essential build tools"
                run_command "sudo apt install -y clang llvm libc++-dev libc++abi-dev" "Clang/LLVM compiler"
                run_command "sudo apt install -y gdb autoconf automake autoconf-archive" "Development tools"
            else
                run_command "sudo apt install -y build-essential cmake ninja-build" "Minimal build tools"
            fi

            # Database libraries
            if [[ "$INSTALL_POSTGRESQL" == true ]]; then
                run_command "sudo apt install -y libpq-dev postgresql-server-dev-all" "PostgreSQL development libraries"
            fi
            if [[ "$INSTALL_MYSQL" == true ]]; then
                run_command "sudo apt install -y libmysqlclient-dev" "MySQL development libraries"
            fi
            if [[ "$INSTALL_SQLITE" == true ]]; then
                run_command "sudo apt install -y libsqlite3-dev" "SQLite development libraries"
            fi

            # Development tools
            if [[ "$INSTALL_DEBUG" == true ]]; then
                run_command "sudo apt install -y gdb valgrind" "Debugging tools"
            fi
            if [[ "$INSTALL_PROFILING" == true ]]; then
                run_command "sudo apt install -y google-perftools libgoogle-perftools-dev" "Profiling tools"
            fi
            if [[ "$INSTALL_DOCS" == true ]]; then
                run_command "sudo apt install -y doxygen graphviz python3-sphinx" "Documentation tools"
            fi
            ;;

        "windows")
            echo -e "${YELLOW}⚠️  Windows dependency installation via chocolatey${NC}"
            run_command "choco install cmake ninja pkgconfiglite git" "Essential build tools"

            if [[ "$INSTALL_POSTGRESQL" == true ]]; then
                run_command "choco install postgresql" "PostgreSQL"
            fi
            if [[ "$INSTALL_MYSQL" == true ]]; then
                run_command "choco install mysql" "MySQL"
            fi
            if [[ "$INSTALL_DOCS" == true ]]; then
                run_command "choco install doxygen.install graphviz" "Documentation tools"
            fi
            ;;
    esac
}

# Install vcpkg and packages
install_vcpkg() {
    if [[ "$INSTALL_VCPKG" != true ]]; then
        return 0
    fi

    echo -e "${BLUE}📦 Setting up vcpkg package manager...${NC}"

    # Check if vcpkg already exists
    if [[ -d "vcpkg" && "$FORCE" != true ]]; then
        echo -e "${YELLOW}⚠️  vcpkg directory already exists, updating...${NC}"
        cd vcpkg
        run_command "git pull" "Update vcpkg"
        cd ..
    else
        if [[ "$FORCE" == true && -d "vcpkg" ]]; then
            run_command "rm -rf vcpkg" "Remove existing vcpkg"
        fi
        run_command "git clone https://github.com/microsoft/vcpkg.git" "Clone vcpkg repository"
    fi

    # Bootstrap vcpkg
    cd vcpkg
    if [[ "$OS" == "windows" ]]; then
        run_command "./bootstrap-vcpkg.bat" "Bootstrap vcpkg (Windows)"
    else
        run_command "./bootstrap-vcpkg.sh" "Bootstrap vcpkg (Unix)"
    fi

    # Integrate vcpkg
    run_command "./vcpkg integrate install" "Integrate vcpkg with build system"

    # Install C++ packages based on vcpkg.json if it exists
    if [[ -f "../vcpkg.json" ]]; then
        echo -e "${CYAN}📋 Installing packages from vcpkg.json...${NC}"
        run_command "./vcpkg install" "Install packages from manifest"
    else
        echo -e "${CYAN}📋 Installing essential C++ packages...${NC}"

        # Essential packages
        run_command "./vcpkg install gtest" "Google Test framework"
        run_command "./vcpkg install fmt" "fmt formatting library"
        run_command "./vcpkg install spdlog" "spdlog logging library"

        # Database packages
        if [[ "$INSTALL_POSTGRESQL" == true ]]; then
            run_command "./vcpkg install libpqxx" "PostgreSQL C++ library"
        fi
        if [[ "$INSTALL_MYSQL" == true ]]; then
            run_command "./vcpkg install libmysql" "MySQL C++ library"
        fi
        if [[ "$INSTALL_SQLITE" == true ]]; then
            run_command "./vcpkg install sqlite3" "SQLite3 library"
        fi

        # Additional useful packages
        if [[ "$MINIMAL" != true ]]; then
            run_command "./vcpkg install nlohmann-json" "JSON library"
            run_command "./vcpkg install catch2" "Catch2 testing framework"
        fi
    fi

    cd ..
}

# Verify installation
verify_installation() {
    echo -e "${BLUE}🔍 Verifying installation...${NC}"

    # Check essential tools
    local tools=("cmake" "ninja")
    if [[ "$OS" != "windows" ]]; then
        tools+=("pkg-config")
    fi

    for tool in "${tools[@]}"; do
        if command -v "$tool" &> /dev/null; then
            echo -e "${GREEN}✅ $tool is available${NC}"
        else
            echo -e "${RED}❌ $tool is not available${NC}"
        fi
    done

    # Check vcpkg
    if [[ -d "vcpkg" ]]; then
        echo -e "${GREEN}✅ vcpkg is installed${NC}"
        if [[ "$VERBOSE" == true ]]; then
            cd vcpkg
            echo -e "${CYAN}📋 Installed vcpkg packages:${NC}"
            ./vcpkg list 2>/dev/null || echo "No packages installed yet"
            cd ..
        fi
    else
        echo -e "${YELLOW}⚠️  vcpkg is not installed${NC}"
    fi

    # Check database libraries
    if [[ "$INSTALL_POSTGRESQL" == true ]]; then
        if pkg-config --exists libpq 2>/dev/null || [[ "$OS" == "macos" && -d "/opt/homebrew/opt/libpq" ]]; then
            echo -e "${GREEN}✅ PostgreSQL development libraries are available${NC}"
        else
            echo -e "${YELLOW}⚠️  PostgreSQL development libraries may not be available${NC}"
        fi
    fi
}

# Main execution
main() {
    echo -e "${CYAN}📋 Dependency Installation Configuration:${NC}"
    echo -e "  OS: ${BOLD}$OS${NC}"
    echo -e "  Package Manager: ${BOLD}$PACKAGE_MANAGER${NC}"
    echo -e "  System Dependencies: ${BOLD}$([ "$INSTALL_SYSTEM" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  vcpkg: ${BOLD}$([ "$INSTALL_VCPKG" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  PostgreSQL: ${BOLD}$([ "$INSTALL_POSTGRESQL" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  MySQL: ${BOLD}$([ "$INSTALL_MYSQL" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  SQLite: ${BOLD}$([ "$INSTALL_SQLITE" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  Documentation: ${BOLD}$([ "$INSTALL_DOCS" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  Debugging Tools: ${BOLD}$([ "$INSTALL_DEBUG" = true ] && echo "YES" || echo "NO")${NC}"
    echo -e "  Minimal Install: ${BOLD}$([ "$MINIMAL" = true ] && echo "YES" || echo "NO")${NC}"
    if [[ "$DRY_RUN" == true ]]; then
        echo -e "  ${YELLOW}🔍 DRY RUN MODE - No actual installation${NC}"
    fi
    echo ""

    # Install dependencies
    install_system_dependencies
    install_vcpkg

    if [[ "$DRY_RUN" != true ]]; then
        verify_installation

        echo ""
        echo -e "${GREEN}✅ Dependency installation completed!${NC}"
        echo -e "${CYAN}📁 vcpkg location: $(pwd)/vcpkg${NC}"
        echo ""
        echo -e "${CYAN}🎯 Next steps:${NC}"
        echo "  1. Run ./build.sh to build the project"
        echo "  2. Use CMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake for CMake"
        echo "  3. Check build.sh --help for build options"
    else
        echo ""
        echo -e "${CYAN}🔍 Dry run completed. Use without --dry-run to install.${NC}"
    fi
}

# Check if running as root (not recommended)
if [[ $EUID -eq 0 && "$OS" != "windows" ]]; then
    echo -e "${YELLOW}⚠️  Running as root is not recommended for development${NC}"
    echo -e "${YELLOW}   Consider running as a regular user${NC}"
    echo ""
fi

# Detect OS and run main
detect_os
main

exit 0