# Messaging System Build Guide

Comprehensive guide for building the messaging system on different platforms with various configurations.

## Table of Contents

- [System Requirements](#system-requirements)
- [Quick Start](#quick-start)
- [Dependency Installation](#dependency-installation)
- [Build Configuration](#build-configuration)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Build Options](#build-options)
- [Troubleshooting](#troubleshooting)
- [Advanced Build Topics](#advanced-build-topics)

## System Requirements

### Minimum Requirements

| Component | Requirement | Notes |
|-----------|-------------|-------|
| **OS** | Linux, macOS 10.15+, Windows 10+ | 64-bit required |
| **CPU** | x86_64 or ARM64 | ARM64 support is experimental |
| **Memory** | 4GB RAM | 8GB+ recommended for development |
| **Disk** | 2GB free space | Additional space for dependencies |

### Compiler Requirements

| Platform | Compiler | Version | C++ Standard |
|----------|----------|---------|--------------|
| **Linux** | GCC | 10.0+ | C++20 |
| **Linux** | Clang | 12.0+ | C++20 |
| **macOS** | Xcode/Clang | 13.0+ | C++20 |
| **Windows** | MSVC | 2019+ | C++20 |
| **Windows** | MinGW-w64 | 10.0+ | C++20 |

### Build Tools

- **CMake**: 3.16 or later
- **vcpkg**: Included as submodule (no separate installation needed)
- **Git**: 2.25 or later
- **Python**: 3.8+ (optional, for Python bindings)

## Quick Start

### 1. Clone Repository

```bash
git clone <repository-url> messaging_system
cd messaging_system
git submodule update --init --recursive
```

### 2. Install Dependencies

```bash
# Automated installation (recommended)
./dependency.sh

# Or platform-specific manual installation (see below)
```

### 3. Build Project

```bash
# Standard build
./build.sh

# Build with tests
./build.sh --tests

# Clean build  
./build.sh --clean
```

### 4. Verify Installation

```bash
# Run tests to verify build
cd build/bin
./container_test
./database_test
./network_test
./integration_test
```

## Dependency Installation

### Automated Installation

The `dependency.sh` script automatically detects your platform and installs required dependencies:

```bash
# Install all dependencies
./dependency.sh

# Install specific components only
./dependency.sh --no-postgresql  # Skip PostgreSQL
./dependency.sh --no-python     # Skip Python
./dependency.sh --minimal       # Install only essential components
```

### Manual Installation

#### Ubuntu/Debian

```bash
# Update package manager
sudo apt update

# Essential build tools
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    curl \
    zip \
    unzip \
    tar

# PostgreSQL development files
sudo apt install -y \
    postgresql-server-dev-all \
    libpq-dev

# Python development (optional)
sudo apt install -y \
    python3-dev \
    python3-pip \
    python3-venv

# Additional utilities
sudo apt install -y \
    doxygen \
    graphviz \
    valgrind
```

#### CentOS/RHEL/Fedora

```bash
# Essential build tools (CentOS/RHEL)
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake \
    git \
    pkg-config \
    curl \
    zip \
    unzip

# Essential build tools (Fedora)
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y \
    cmake \
    git \
    pkg-config \
    curl \
    zip \
    unzip

# PostgreSQL development
sudo yum install -y postgresql-devel  # CentOS/RHEL
sudo dnf install -y postgresql-devel  # Fedora

# Python development
sudo yum install -y python3-devel     # CentOS/RHEL  
sudo dnf install -y python3-devel     # Fedora
```

#### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install \
    cmake \
    postgresql \
    pkg-config \
    doxygen \
    graphviz

# Python (usually pre-installed, but can upgrade)
brew install python@3.11
```

#### Windows

**Option 1: Using Visual Studio**

1. Install Visual Studio 2019 or later with C++ development tools
2. Install Git for Windows
3. Install CMake for Windows
4. Install PostgreSQL for Windows (optional)

**Option 2: Using MSYS2/MinGW-w64**

```bash
# Install MSYS2 from https://www.msys2.org/

# Update package database
pacman -Syu

# Install build tools
pacman -S --needed \
    base-devel \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-postgresql \
    git
```

## Build Configuration

### CMake Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug, Release, RelWithDebInfo) |
| `USE_UNIT_TEST` | OFF | Enable unit test building |
| `BUILD_MESSAGING_SAMPLES` | OFF | Build sample applications |
| `ENABLE_PYTHON_BINDINGS` | ON | Build Python bindings |
| `ENABLE_DATABASE_MODULE` | ON | Build database module |
| `ENABLE_NETWORK_MODULE` | ON | Build network module |
| `USE_LOCK_FREE_IMPL` | ON | Use lock-free implementations |
| `ENABLE_SIMD_OPTIMIZATION` | ON | Enable SIMD optimizations |

### vcpkg Configuration

The project uses vcpkg for dependency management. Key dependencies:

```json
{
  "dependencies": [
    "asio",
    "benchmark", 
    "fmt",
    "gtest",
    "libpq",
    "libpqxx",
    "nlohmann-json",
    "spdlog"
  ]
}
```

## Platform-Specific Instructions

### Linux

#### Standard Build

```bash
# Configure and build
mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_UNIT_TEST=ON

make -j$(nproc)
```

#### With Clang

```bash
# Set compiler to Clang
export CC=clang
export CXX=clang++

mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++

make -j$(nproc)
```

#### Debug Build with Sanitizers

```bash
mkdir build_debug && cd build_debug
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=undefined" \
    -DUSE_UNIT_TEST=ON

make -j$(nproc)
```

### macOS

#### Xcode Build

```bash
# Generate Xcode project
mkdir build_xcode && cd build_xcode
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -G Xcode \
    -DCMAKE_BUILD_TYPE=Release

# Build with Xcode
xcodebuild -configuration Release
```

#### Command Line Build

```bash
# Standard build
mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release

make -j$(sysctl -n hw.ncpu)
```

#### Universal Binary (Intel + Apple Silicon)

```bash
mkdir build_universal && cd build_universal
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_BUILD_TYPE=Release

make -j$(sysctl -n hw.ncpu)
```

### Windows

#### Visual Studio

```bash
# Generate Visual Studio solution
mkdir build && cd build
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -G "Visual Studio 16 2019" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release

# Build with MSBuild
msbuild messaging_system.sln /p:Configuration=Release /m
```

#### Visual Studio Developer Command Prompt

```bash
# Set up environment
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"

# Configure and build
mkdir build && cd build
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release --parallel
```

#### MinGW-w64

```bash
# Using MSYS2 environment
mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release

mingw32-make -j$(nproc)
```

## Build Options

### Using Build Script

The `build.sh` script provides convenient build options:

```bash
# Basic options
./build.sh                 # Standard Release build
./build.sh --debug         # Debug build
./build.sh --clean         # Clean build (removes build directory)
./build.sh --tests         # Build with tests enabled

# Module selection
./build.sh --no-database   # Build without database module
./build.sh --no-network    # Build without network module  
./build.sh --no-python     # Build without Python bindings

# Advanced options
./build.sh --lockfree      # Use lock-free implementations
./build.sh --no-simd       # Disable SIMD optimizations
./build.sh --static        # Static linking
./build.sh --verbose       # Verbose build output

# Combination examples
./build.sh --debug --tests --verbose
./build.sh --clean --no-database --static
```

### Manual CMake Configuration

For more control, configure CMake manually:

```bash
mkdir build && cd build

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_UNIT_TEST=ON \
    -DBUILD_MESSAGING_SAMPLES=ON \
    -DENABLE_PYTHON_BINDINGS=OFF \
    -DUSE_LOCK_FREE_IMPL=ON \
    -DENABLE_SIMD_OPTIMIZATION=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/local/messaging_system

cmake --build . --config Release --parallel
```

### Build Types

#### Release Build (Default)

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```
- Optimized for performance
- No debug symbols
- Assertions disabled
- Recommended for production

#### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
- No optimizations
- Full debug symbols
- Assertions enabled
- Recommended for development

#### RelWithDebInfo Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```
- Optimized with debug symbols
- Assertions disabled
- Good for profiling

#### MinSizeRel Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel
```
- Optimized for size
- Minimal debug information
- Good for embedded systems

## Troubleshooting

### Common Build Issues

#### vcpkg Bootstrap Issues

```bash
# Error: vcpkg not bootstrapped
# Solution: Bootstrap vcpkg manually
cd vcpkg
./bootstrap-vcpkg.sh      # Linux/macOS
.\bootstrap-vcpkg.bat     # Windows
```

#### Missing Dependencies

```bash
# Error: Package 'xyz' not found
# Solution: Install dependency manually
vcpkg install xyz
vcpkg integrate install
```

#### Compiler Not Found

```bash
# Error: Could not find compiler
# Solution: Set compiler explicitly
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10

# Or in CMake
cmake .. -DCMAKE_C_COMPILER=/usr/bin/gcc-10 -DCMAKE_CXX_COMPILER=/usr/bin/g++-10
```

#### PostgreSQL Not Found

```bash
# Error: Could NOT find PostgreSQL
# Solution: Install PostgreSQL development files
sudo apt install libpq-dev              # Ubuntu/Debian
sudo yum install postgresql-devel        # CentOS/RHEL
brew install postgresql                  # macOS

# Or disable database module
cmake .. -DENABLE_DATABASE_MODULE=OFF
```

#### Python Bindings Issues

```bash
# Error: Could NOT find Python3
# Solution: Install Python development files
sudo apt install python3-dev python3-pip    # Ubuntu/Debian
brew install python@3.11                     # macOS

# Or disable Python bindings
cmake .. -DENABLE_PYTHON_BINDINGS=OFF
```

### Memory and Performance Issues

#### Out of Memory During Build

```bash
# Reduce parallel build jobs
make -j2    # Instead of -j$(nproc)

# Or use single-threaded build
make
```

#### Slow Build Times

```bash
# Enable precompiled headers (if supported)
cmake .. -DUSE_PRECOMPILED_HEADERS=ON

# Use faster linker
cmake .. -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=gold"    # Linux
cmake .. -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"     # Clang

# Enable ccache for faster rebuilds
cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

### Platform-Specific Issues

#### Linux

```bash
# Error: GLIBC version too old
# Solution: Use newer compiler or static linking
cmake .. -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"

# Error: Missing development headers
sudo apt install linux-libc-dev build-essential
```

#### macOS

```bash
# Error: Xcode Command Line Tools not found
xcode-select --install

# Error: macOS SDK not found
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
```

#### Windows

```bash
# Error: MSBuild not found
# Solution: Use Visual Studio Developer Command Prompt

# Error: vcvars64.bat not found
# Solution: Install Visual Studio with C++ tools
```

### Debugging Build Issues

#### Verbose Build Output

```bash
# CMake verbose configuration
cmake .. --debug-output

# Make verbose build
make VERBOSE=1

# MSBuild verbose
msbuild /verbosity:detailed
```

#### CMake Cache Issues

```bash
# Clear CMake cache
rm -rf CMakeCache.txt CMakeFiles/

# Or delete entire build directory
rm -rf build/
mkdir build && cd build
```

#### Dependency Issues

```bash
# Check vcpkg installed packages
vcpkg list

# Reinstall specific package
vcpkg remove asio
vcpkg install asio

# Update all packages
vcpkg upgrade --no-dry-run
```

## Advanced Build Topics

### Cross-Compilation

#### ARM64 Linux

```bash
# Install cross-compilation tools
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Configure for ARM64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=arm64-linux \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
```

#### Windows from Linux

```bash
# Install MinGW-w64
sudo apt install mingw-w64

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
```

### Custom vcpkg Triplets

Create custom triplet file `custom-triplet.cmake`:

```cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
```

Use custom triplet:

```bash
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=custom-triplet
```

### Static Linking

```bash
# Enable static linking
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-linux-static \
    -DCMAKE_BUILD_TYPE=Release

# For specific libraries only
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
```

### Profile-Guided Optimization (PGO)

```bash
# Step 1: Build with instrumentation
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-fprofile-generate"

make -j$(nproc)

# Step 2: Run representative workload
cd bin
./container_test
./database_test
./network_test

# Step 3: Rebuild with optimization
cd ..
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-fprofile-use"

make -j$(nproc)
```

### Link Time Optimization (LTO)

```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Custom Installation

```bash
# Configure installation prefix
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/opt/messaging_system \
    -DCMAKE_BUILD_TYPE=Release

# Build and install
make -j$(nproc)
sudo make install

# Create package
make package
```

### Development Build Setup

For frequent development builds, create a development script:

```bash
#!/bin/bash
# dev-build.sh

set -e

BUILD_DIR="build_dev"
SOURCE_DIR="$(pwd)"

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with development settings
cmake $SOURCE_DIR \
    -DCMAKE_TOOLCHAIN_FILE=$SOURCE_DIR/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DUSE_UNIT_TEST=ON \
    -DBUILD_MESSAGING_SAMPLES=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra -g3 -O0"

# Build
make -j$(nproc)

# Run tests
ctest --output-on-failure

echo "Development build complete!"
```

## Performance Optimization

### Compiler Optimizations

```bash
# Maximum optimization
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG"

# Size optimization
cmake .. \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_CXX_FLAGS="-Os -flto"

# Debug-friendly optimization
cmake .. \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_CXX_FLAGS="-O2 -g"
```

### SIMD Optimizations

```bash
# Enable specific SIMD instructions
cmake .. \
    -DCMAKE_CXX_FLAGS="-msse4.2 -mavx2"    # x86_64
    
cmake .. \
    -DCMAKE_CXX_FLAGS="-march=armv8-a+simd" # ARM64
```

### Memory Optimization

```bash
# Reduce memory usage during build
cmake .. \
    -DCMAKE_CXX_FLAGS="-pipe -ggdb1"

# Use gold linker for faster linking
cmake .. \
    -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=gold"
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        build_type: [Debug, Release]
        
    runs-on: ${{ matrix.os }}
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Install dependencies
      run: ./dependency.sh
      
    - name: Configure CMake
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} \
          -DUSE_UNIT_TEST=ON
          
    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}
      
    - name: Test
      run: ctest --test-dir build --output-on-failure
```

### Docker Build

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpq-dev \
    python3-dev

WORKDIR /app
COPY . .

RUN git submodule update --init --recursive
RUN ./dependency.sh
RUN ./build.sh --tests

CMD ["./build/bin/integration_test"]
```

## License

BSD 3-Clause License - see main project LICENSE file.