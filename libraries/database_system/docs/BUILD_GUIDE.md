# Database System Build Guide

Comprehensive guide for building the Database System with multi-backend support, connection pooling, and query builders.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Build Configurations](#build-configurations)
- [Database Dependencies](#database-dependencies)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Troubleshooting](#troubleshooting)
- [Advanced Configuration](#advanced-configuration)

## Prerequisites

### System Requirements

- **C++20 compatible compiler**:
  - GCC 10.0+ (Linux)
  - Clang 11.0+ (macOS/Linux)
  - MSVC 2019+ (Windows)
- **CMake 3.16+**
- **Build system**: Make, Ninja (recommended), or Visual Studio
- **Git** (for cloning and vcpkg)

### Optional Dependencies

Database support is optional and can be disabled for testing:

- **PostgreSQL**: libpqxx, libpq, OpenSSL
- **MySQL**: libmysql or mysql-connector-cpp
- **SQLite**: sqlite3
- **MongoDB**: mongo-cxx-driver (mongocxx, bsoncxx)
- **Redis**: hiredis

## Quick Start

### 1. Clone Repository

```bash
git clone https://github.com/kcenon/database_system.git
cd database_system
```

### 2. Basic Build (No External Dependencies)

```bash
# Create build directory
mkdir build && cd build

# Configure with mock implementations
cmake .. -DUSE_POSTGRESQL=OFF -DUSE_MYSQL=OFF -DUSE_SQLITE=OFF -DUSE_MONGODB=OFF -DUSE_REDIS=OFF

# Build
ninja  # or make -j$(nproc)

# Test
./bin/basic_usage
./bin/connection_pool_demo
```

### 3. Full Build with Database Support

```bash
# Install dependencies (see Database Dependencies section)
# Then configure with full support
cmake .. -DUSE_POSTGRESQL=ON -DUSE_MYSQL=ON -DUSE_SQLITE=ON -DUSE_MONGODB=ON -DUSE_REDIS=ON

# Build
ninja
```

## Build Configurations

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `USE_POSTGRESQL` | ON | Enable PostgreSQL support (requires libpqxx) |
| `USE_MYSQL` | OFF | Enable MySQL support (requires libmysql) |
| `USE_SQLITE` | OFF | Enable SQLite support (requires sqlite3) |
| `USE_MONGODB` | OFF | Enable MongoDB support (requires mongocxx) |
| `USE_REDIS` | OFF | Enable Redis support (requires hiredis) |
| `BUILD_DATABASE_SAMPLES` | ON | Build sample programs |
| `USE_UNIT_TEST` | ON | Build unit tests |
| `BUILD_SHARED_LIBS` | OFF | Build as shared library |

### Build Types

```bash
# Debug build (default)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Release with debug info
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Minimum size release
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel
```

### Common Build Scenarios

#### 1. Development Build

```bash
# Full features with debug information
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DUSE_POSTGRESQL=ON \
  -DUSE_MYSQL=ON \
  -DUSE_SQLITE=ON \
  -DBUILD_DATABASE_SAMPLES=ON \
  -DUSE_UNIT_TEST=ON
```

#### 2. Production Build

```bash
# Optimized release with specific databases
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DUSE_POSTGRESQL=ON \
  -DUSE_REDIS=ON \
  -DBUILD_DATABASE_SAMPLES=OFF \
  -DUSE_UNIT_TEST=OFF
```

#### 3. Testing/CI Build

```bash
# Mock implementations only
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DUSE_POSTGRESQL=OFF \
  -DUSE_MYSQL=OFF \
  -DUSE_SQLITE=OFF \
  -DUSE_MONGODB=OFF \
  -DUSE_REDIS=OFF \
  -DBUILD_DATABASE_SAMPLES=ON \
  -DUSE_UNIT_TEST=ON
```

## Database Dependencies

### Using vcpkg (Recommended)

#### Install vcpkg

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Windows
.\bootstrap-vcpkg.bat

# Linux/macOS
./bootstrap-vcpkg.sh
```

#### Install Database Libraries

```bash
# PostgreSQL support
vcpkg install libpqxx openssl

# MySQL support
vcpkg install libmysql

# SQLite support
vcpkg install sqlite3

# MongoDB support
vcpkg install mongo-cxx-driver

# Redis support
vcpkg install hiredis

# Install all at once
vcpkg install libpqxx openssl libmysql sqlite3 mongo-cxx-driver hiredis
```

#### Build with vcpkg

```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DUSE_POSTGRESQL=ON \
  -DUSE_MYSQL=ON \
  -DUSE_SQLITE=ON \
  -DUSE_MONGODB=ON \
  -DUSE_REDIS=ON
```

### Manual Installation

#### Ubuntu/Debian

```bash
# PostgreSQL
sudo apt-get install libpqxx-dev libpq-dev libssl-dev

# MySQL
sudo apt-get install libmysqlclient-dev

# SQLite
sudo apt-get install libsqlite3-dev

# MongoDB
sudo apt-get install libmongocxx-dev libbsoncxx-dev

# Redis
sudo apt-get install libhiredis-dev
```

#### CentOS/RHEL/Fedora

```bash
# PostgreSQL
sudo dnf install libpqxx-devel postgresql-devel openssl-devel

# MySQL
sudo dnf install mysql-devel

# SQLite
sudo dnf install sqlite-devel

# MongoDB
sudo dnf install mongo-cxx-driver-devel

# Redis
sudo dnf install hiredis-devel
```

#### macOS (Homebrew)

```bash
# PostgreSQL
brew install libpqxx postgresql openssl

# MySQL
brew install mysql

# SQLite
brew install sqlite

# MongoDB
brew install mongo-cxx-driver

# Redis
brew install hiredis
```

#### Windows (vcpkg recommended)

For Windows, vcpkg is the recommended approach. Manual installation is complex due to dependency management.

## Platform-Specific Instructions

### Linux

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake ninja-build git

# Install database libraries (see above)

# Build
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja

# Install (optional)
sudo ninja install
```

### macOS

```bash
# Install Xcode command line tools
xcode-select --install

# Install dependencies
brew install cmake ninja

# Install database libraries (see above)

# Build
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja
```

### Windows

#### Using Visual Studio

```batch
# Open Developer Command Prompt

# Build
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

#### Using MSYS2/MinGW

```bash
# Install MSYS2 first

# Install dependencies
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja

# Build
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja
```

## Troubleshooting

### Common Build Issues

#### 1. Missing C++20 Support

**Error**: `error: 'std::variant' is not available before C++17`

**Solution**:
```bash
# Update compiler
# GCC
sudo apt-get install gcc-10 g++-10
export CC=gcc-10 CXX=g++-10

# Clang
sudo apt-get install clang-11
export CC=clang-11 CXX=clang++-11
```

#### 2. Missing Database Libraries

**Error**: `Could NOT find libpqxx (missing: libpqxx_LIBRARY libpqxx_INCLUDE_DIR)`

**Solution**:
```bash
# Disable specific database if not needed
cmake .. -DUSE_POSTGRESQL=OFF

# Or install the library
sudo apt-get install libpqxx-dev

# Or use vcpkg
vcpkg install libpqxx
```

#### 3. CMake Version Too Old

**Error**: `CMake 3.16 or higher is required. You are running version 3.10.2`

**Solution**:
```bash
# Ubuntu/Debian
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt-get update
sudo apt-get install cmake

# Or build from source
wget https://github.com/Kitware/CMake/releases/download/v3.26.0/cmake-3.26.0.tar.gz
tar -xzf cmake-3.26.0.tar.gz
cd cmake-3.26.0
./bootstrap && make -j$(nproc) && sudo make install
```

#### 4. Linking Errors

**Error**: `undefined reference to 'pqxx::connection::connection(...)'`

**Solution**:
```bash
# Make sure all dependencies are found
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON

# Check if libraries are properly linked
ldd bin/basic_usage

# For static linking issues
cmake .. -DBUILD_SHARED_LIBS=OFF
```

#### 5. MongoDB Driver Issues

**Error**: `Could NOT find mongocxx`

**Solution**:
```bash
# Install MongoDB C++ driver manually
git clone https://github.com/mongodb/mongo-cxx-driver.git
cd mongo-cxx-driver
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc) && sudo make install
```

### Build Verification

After successful build, verify installation:

```bash
# Check built files
ls -la bin/
ls -la lib/

# Run tests
ctest --verbose

# Run samples
./bin/basic_usage
./bin/connection_pool_demo
./bin/postgres_advanced  # If PostgreSQL enabled

# Check dependencies
ldd bin/basic_usage  # Linux
otool -L bin/basic_usage  # macOS
```

## Advanced Configuration

### Custom Build Options

#### Disabling Specific Features

```bash
# Minimal build - only core functionality
cmake .. \
  -DUSE_POSTGRESQL=OFF \
  -DUSE_MYSQL=OFF \
  -DUSE_SQLITE=OFF \
  -DUSE_MONGODB=OFF \
  -DUSE_REDIS=OFF \
  -DBUILD_DATABASE_SAMPLES=OFF \
  -DUSE_UNIT_TEST=OFF
```

#### Custom Installation Directory

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/database_system
ninja install
```

#### Cross-Compilation

```bash
# ARM64 cross-compilation example
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=arm64-toolchain.cmake \
  -DUSE_POSTGRESQL=OFF \
  -DUSE_MYSQL=OFF
```

### Environment Variables

```bash
# Custom compiler
export CC=/usr/bin/clang-12
export CXX=/usr/bin/clang++-12

# Custom library paths
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# vcpkg integration
export VCPKG_ROOT=/path/to/vcpkg
export CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

### Performance Optimization

#### Release Builds

```bash
# Maximum optimization
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG"

# Link-time optimization
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

#### Memory Optimization

```bash
# Minimal size build
cmake .. \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_CXX_FLAGS="-Os -flto"
```

### IDE Integration

#### VS Code

```json
// .vscode/settings.json
{
    "cmake.configureSettings": {
        "USE_POSTGRESQL": "ON",
        "USE_MYSQL": "ON",
        "USE_SQLITE": "ON",
        "CMAKE_BUILD_TYPE": "Debug"
    },
    "cmake.buildDirectory": "${workspaceFolder}/build"
}
```

#### CLion

Configure CMake options in Settings → Build, Execution, Deployment → CMake:

```
-DUSE_POSTGRESQL=ON -DUSE_MYSQL=ON -DUSE_SQLITE=ON
```

#### Visual Studio

Use the CMake integration with vcpkg:

```json
// CMakeSettings.json
{
  "configurations": [
    {
      "name": "x64-Release",
      "generator": "Ninja",
      "configurationType": "Release",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "buildRoot": "${projectDir}\\build\\${name}",
      "installRoot": "${projectDir}\\install\\${name}",
      "cmakeCommandArgs": "-DUSE_POSTGRESQL=ON -DUSE_MYSQL=ON",
      "ctestCommandArgs": "",
      "variables": [
        {
          "name": "CMAKE_TOOLCHAIN_FILE",
          "value": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        }
      ]
    }
  ]
}
```

## Continuous Integration

### GitHub Actions Example

```yaml
# .github/workflows/build.yml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    strategy:
      matrix:
        compiler: [gcc-10, clang-11]
        build_type: [Debug, Release]

    steps:
    - uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install ${{ matrix.compiler }} cmake ninja-build

    - name: Configure
      run: |
        mkdir build && cd build
        export CC=${{ matrix.compiler }}
        export CXX=${CC/gcc/g++}
        export CXX=${CXX/clang/clang++}
        cmake .. -GNinja -DCMAKE_BUILD_TYPE=${{ matrix.build_type }}

    - name: Build
      run: |
        cd build
        ninja

    - name: Test
      run: |
        cd build
        ctest --verbose
```

### Docker Build

```dockerfile
# Dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    libpqxx-dev \
    libmysqlclient-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release \
    -DUSE_POSTGRESQL=ON -DUSE_MYSQL=ON -DUSE_SQLITE=ON && \
    ninja

CMD ["./build/bin/basic_usage"]
```

---

For additional help or issues not covered here, please:
1. Check the [troubleshooting section](README.md#troubleshooting) in the main README
2. Search existing [GitHub issues](https://github.com/kcenon/database_system/issues)
3. Create a new issue with your build configuration and error details