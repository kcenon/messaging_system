# Platform-Specific Build Guide

## Overview

This guide provides detailed instructions for building the Thread System on different platforms and compilers.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Linux Build Guide](#linux-build-guide)
3. [macOS Build Guide](#macos-build-guide)
4. [Windows Build Guide](#windows-build-guide)
5. [Cross-Platform CMake Options](#cross-platform-cmake-options)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### All Platforms
- CMake 3.16 or later
- C++20 capable compiler
- vcpkg package manager (automatically installed by dependency scripts)
- Git for version control

### Platform-Specific Requirements

| Platform | Compiler | Minimum Version | Notes |
|----------|----------|-----------------|-------|
| Linux | GCC | 9.0+ | Full C++20 support from GCC 10+ |
| Linux | Clang | 10.0+ | Full C++20 support from Clang 12+ |
| macOS | Apple Clang | 12.0+ | Xcode 12+ recommended |
| macOS | Homebrew GCC | 10.0+ | Optional alternative compiler |
| Windows | MSVC | 2019 (16.8+) | Visual Studio 2019/2022 |
| Windows | MinGW-w64 | 10.0+ | MSYS2 environment recommended |
| Windows | Clang | 12.0+ | Via Visual Studio or LLVM |

## Linux Build Guide

### Ubuntu/Debian

```bash
# Install build essentials
sudo apt-get update
sudo apt-get install -y build-essential cmake git curl zip unzip tar

# Install specific compiler (optional)
# For GCC 11:
sudo apt-get install -y gcc-11 g++-11
export CC=gcc-11
export CXX=g++-11

# For Clang 14:
sudo apt-get install -y clang-14
export CC=clang-14
export CXX=clang++-14

# Install dependencies
./dependency.sh

# Build
./build.sh --clean --release -j$(nproc)

# Or manual build
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++-11
cmake --build build -j$(nproc)

# Run tests
cd build && ctest --verbose
```

### Fedora/RHEL/CentOS

```bash
# Install build tools
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Install newer GCC if needed
sudo dnf install gcc-toolset-11
scl enable gcc-toolset-11 bash

# Build process same as above
./dependency.sh
./build.sh --clean --release
```

### Arch Linux

```bash
# Install build tools
sudo pacman -S base-devel cmake git

# Build
./dependency.sh
./build.sh --clean --release
```

## macOS Build Guide

### Using Apple Clang (Default)

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake
brew install cmake

# Install dependencies
./dependency.sh

# Build with Apple Clang
./build.sh --clean --release -j$(sysctl -n hw.ncpu)

# Or manual build
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build -j$(sysctl -n hw.ncpu)
```

### Using Homebrew GCC

```bash
# Install GCC
brew install gcc@12

# Set compiler
export CC=gcc-12
export CXX=g++-12

# Build
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc-12 \
    -DCMAKE_CXX_COMPILER=g++-12
cmake --build build -j$(sysctl -n hw.ncpu)
```

### macOS-Specific Notes

- Tests are disabled by default on macOS due to CI environment limitations
- Use `CMAKE_OSX_DEPLOYMENT_TARGET` to set minimum macOS version
- libiconv is included automatically for character encoding support

## Windows Build Guide

### Visual Studio (MSVC)

```batch
:: Install Visual Studio 2019/2022 with C++ workload

:: Using Developer Command Prompt
:: Install dependencies
dependency.bat

:: Build with Visual Studio
build.bat --clean --release

:: Or manual build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release -j %NUMBER_OF_PROCESSORS%

:: Using specific toolset
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -T v142
```

### MinGW-w64 (MSYS2)

```bash
# Install MSYS2 from https://www.msys2.org/

# In MSYS2 terminal, install tools
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make git

# Add to PATH
export PATH=/mingw64/bin:$PATH

# Build
./dependency.sh
cmake -S . -B build \
    -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Windows with Clang

```batch
:: Install LLVM from https://llvm.org/
:: Or use Visual Studio's Clang

:: Build with Clang
cmake -S . -B build -G "Visual Studio 17 2022" -T ClangCL
cmake --build build --config Release

:: Or with ninja
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

### Windows-Specific Notes

- Use forward slashes (/) or escaped backslashes (\\) in paths
- libiconv is excluded on Windows (not required)
- Consider using `CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS` for DLL builds

## Cross-Platform CMake Options

### Build Configuration

```bash
# Release build (optimized)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Debug build (with debug symbols)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# RelWithDebInfo (optimized with debug info)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo

# MinSizeRel (optimized for size)
cmake -S . -B build -DCMAKE_BUILD_TYPE=MinSizeRel
```

### Feature Flags

```bash
# Build as submodule (no samples/tests)
cmake -S . -B build -DBUILD_THREADSYSTEM_AS_SUBMODULE=ON

# Enable documentation generation
cmake -S . -B build -DBUILD_DOCUMENTATION=ON

# Enable sanitizers (Debug builds)
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_ASAN=ON \
    -DENABLE_UBSAN=ON

# Enable code coverage
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON

# Disable specific C++ features
cmake -S . -B build \
    -DDISABLE_STD_FORMAT=ON \
    -DDISABLE_STD_JTHREAD=ON
```

### Compiler Selection

```bash
# Specify compiler explicitly
cmake -S . -B build \
    -DCMAKE_C_COMPILER=/usr/bin/gcc-11 \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++-11

# Use compiler launcher (ccache)
cmake -S . -B build \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

# Set compiler flags
cmake -S . -B build \
    -DCMAKE_CXX_FLAGS="-march=native -mtune=native"
```

## Troubleshooting

### Common Issues and Solutions

#### vcpkg Installation Fails

```bash
# Clean vcpkg and retry
rm -rf vcpkg
./dependency.sh

# Or manually install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=$PWD/vcpkg
```

#### CMake Cannot Find Compiler

```bash
# Explicitly set compiler path
export CC=/full/path/to/gcc
export CXX=/full/path/to/g++

# Or use CMake variables
cmake -S . -B build \
    -DCMAKE_C_COMPILER=/full/path/to/gcc \
    -DCMAKE_CXX_COMPILER=/full/path/to/g++
```

#### C++20 Features Not Available

```bash
# Check compiler version
g++ --version
clang++ --version

# Force C++ standard
cmake -S . -B build -DCMAKE_CXX_STANDARD=20

# Disable unsupported features
cmake -S . -B build \
    -DDISABLE_STD_FORMAT=ON \
    -DDISABLE_STD_JTHREAD=ON
```

#### Link Errors on Windows

```batch
:: Use static runtime
cmake -S . -B build -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded

:: Or dynamic runtime
cmake -S . -B build -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
```

#### Tests Fail to Build

```bash
# Disable tests
cmake -S . -B build -DBUILD_TESTING=OFF

# Or fix Google Test issues
vcpkg install gtest
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Platform-Specific Environment Variables

```bash
# Linux/macOS
export MAKEFLAGS="-j$(nproc)"
export CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)

# Windows
set CMAKE_BUILD_PARALLEL_LEVEL=%NUMBER_OF_PROCESSORS%
```

## Performance Tips

### Optimization Flags by Platform

#### Linux/macOS with GCC/Clang
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto"
```

#### Windows with MSVC
```batch
cmake -S . -B build -DCMAKE_CXX_FLAGS="/O2 /GL /arch:AVX2"
```

### Link-Time Optimization (LTO)
```bash
# Enable LTO
cmake -S . -B build \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Debug vs Release Performance

| Build Type | Relative Performance | Use Case |
|------------|---------------------|----------|
| Debug | 1x (baseline) | Development, debugging |
| RelWithDebInfo | 5-10x | Profiling with symbols |
| Release | 10-20x | Production deployment |
| MinSizeRel | 8-15x | Embedded systems |

## CI/CD Integration

### GitHub Actions
```yaml
- name: Build Thread System
  run: |
    ./dependency.sh
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    cd build && ctest --verbose
```

### Docker Build
```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    build-essential cmake git curl zip unzip tar
WORKDIR /app
COPY . .
RUN ./dependency.sh && ./build.sh --release
```

## Support Matrix

| Platform | Architecture | Status | Notes |
|----------|-------------|--------|-------|
| Ubuntu 20.04+ | x64, ARM64 | ✅ Fully Supported | Primary development platform |
| macOS 11+ | x64, Apple Silicon | ✅ Fully Supported | Tests disabled in CI |
| Windows 10/11 | x64 | ✅ Fully Supported | MSVC recommended |
| Alpine Linux | x64 | ⚠️ Experimental | musl libc compatibility |
| FreeBSD | x64 | ⚠️ Experimental | Community supported |

## Additional Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [vcpkg Documentation](https://vcpkg.io/)
- [Compiler Support for C++20](https://en.cppreference.com/w/cpp/compiler_support)
- [Thread System README](../README.md)
- [API Reference](./API_REFERENCE.md)

## Changelog

- **2025-09-13**: Initial platform build guide created
- **2025-09-13**: Added troubleshooting section
- **2025-09-13**: Added performance optimization tips