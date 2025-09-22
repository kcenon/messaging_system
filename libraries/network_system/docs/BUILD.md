# Build Instructions

## Table of Contents
- [Prerequisites](#prerequisites)
- [Quick Build](#quick-build)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Build Options](#build-options)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Minimum Requirements
- C++20 compatible compiler
  - GCC 11 or later
  - Clang 14 or later
  - MSVC 2022 or later
- CMake 3.16 or later
- ASIO library or Boost.ASIO 1.28+

### Optional Dependencies
- fmt library 10.0+ (falls back to std::format if not available)
- Google Test (for unit tests)
- Google Benchmark (for performance tests)
- Ninja build system (recommended)

## Quick Build

```bash
# Clone the repository
git clone https://github.com/kcenon/network_system.git
cd network_system

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel

# Verify installation
./verify_build
```

## Platform-Specific Instructions

### Ubuntu/Debian

#### Install Dependencies
```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install -y build-essential cmake ninja-build

# Install required libraries
sudo apt install -y libasio-dev libfmt-dev

# Optional: Install Boost (for Boost.ASIO fallback)
sudo apt install -y libboost-all-dev

# Optional: Install test frameworks
sudo apt install -y libgtest-dev libbenchmark-dev
```

#### Build
```bash
mkdir build && cd build
cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_SAMPLES=ON
ninja
```

### macOS

#### Install Dependencies
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja asio fmt boost

# Optional: Install test frameworks
brew install googletest google-benchmark
```

#### Build
```bash
mkdir build && cd build
cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_SAMPLES=ON
ninja
```

### Windows (Visual Studio)

#### Install Dependencies

##### Option 1: vcpkg (Recommended)
```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install asio fmt --triplet x64-windows
```

##### Option 2: Manual Installation
- Download Boost from https://www.boost.org/
- Extract and add to system PATH

#### Build
```powershell
# Create build directory
mkdir build
cd build

# Configure with vcpkg
cmake .. -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="path\to\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

### Windows (MSYS2/MinGW)

#### Install Dependencies
```bash
# Update MSYS2
pacman -Syu

# Install build tools
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja

# Install libraries
pacman -S mingw-w64-x86_64-asio
pacman -S mingw-w64-x86_64-fmt
pacman -S mingw-w64-x86_64-boost
```

#### Build
```bash
mkdir build && cd build
cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_SAMPLES=ON
ninja
```

## Build Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | OFF | Build as shared library |
| `BUILD_TESTS` | ON | Build unit tests |
| `BUILD_SAMPLES` | ON | Build sample applications |
| `BUILD_WITH_CONTAINER_SYSTEM` | ON | Enable container_system integration |
| `BUILD_WITH_THREAD_SYSTEM` | ON | Enable thread_system integration |
| `BUILD_MESSAGING_BRIDGE` | ON | Build messaging_system compatibility bridge |
| `CMAKE_BUILD_TYPE` | Debug | Build configuration (Debug/Release/RelWithDebInfo/MinSizeRel) |

### Example Configurations

#### Minimal Build
```bash
cmake .. -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF
```

#### Full Feature Build
```bash
cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_SAMPLES=ON \
  -DBUILD_WITH_CONTAINER_SYSTEM=ON \
  -DBUILD_WITH_THREAD_SYSTEM=ON \
  -DBUILD_MESSAGING_BRIDGE=ON
```

#### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-g -O0 -fsanitize=address"
```

## Integration

### Using NetworkSystem in Your Project

#### CMake Integration
```cmake
# Find the package
find_package(NetworkSystem REQUIRED)

# Link to your target
target_link_libraries(your_target
  PRIVATE
    NetworkSystem::NetworkSystem
)
```

#### Manual Integration
```cmake
# Add include directory
target_include_directories(your_target
  PRIVATE
    /path/to/network_system/include
)

# Link library
target_link_libraries(your_target
  PRIVATE
    /path/to/network_system/build/libNetworkSystem.a
)
```

## Troubleshooting

### Common Issues

#### ASIO Not Found
```
CMake Error: ASIO not found in standard locations
```

**Solution**: Install ASIO or Boost:
```bash
# Ubuntu/Debian
sudo apt install libasio-dev libboost-all-dev

# macOS
brew install asio boost

# Windows (vcpkg)
vcpkg install asio boost
```

#### pthread Not Found on Windows
```
LINK : fatal error LNK1104: cannot open file 'pthread.lib'
```

**Solution**: This is already fixed in the CMakeLists.txt. pthread is only linked on Unix systems.

#### vcpkg Build Failure
```
Error: run-vcpkg action execution failed
```

**Solution**: The build system automatically falls back to system libraries. Ensure ASIO is installed via your system package manager.

#### C++20 Features Not Available
```
error: 'jthread' is not a member of 'std'
```

**Solution**: Update your compiler:
```bash
# Ubuntu/Debian
sudo apt install g++-11

# Set as default
sudo update-alternatives --config g++
```

### Debug Build Issues

Enable verbose output to diagnose build issues:
```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build . --verbose
```

### Clean Build

If you encounter persistent issues, try a clean build:
```bash
rm -rf build
mkdir build && cd build
cmake .. -G Ninja
ninja
```

## Performance Tuning

### Compiler Optimizations

For maximum performance:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native"
```

### Link-Time Optimization (LTO)
```bash
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## Testing

### Run Tests
```bash
# Run all tests
ctest --output-on-failure

# Run specific test
./tests/unit_tests

# Run with detailed output
ctest -V
```

### Run Benchmarks
```bash
./tests/benchmark_tests
```

## Support

For build issues, please check:
1. [GitHub Issues](https://github.com/kcenon/network_system/issues)
2. [CHANGELOG.md](CHANGELOG.md) for known issues
3. Contact: kcenon@naver.com