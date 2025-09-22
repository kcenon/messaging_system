# Dependency Management Guide

## Overview

The `dependency.sh` script is a comprehensive dependency management tool for the messaging_system project. It handles both external dependencies (via vcpkg) and internal library system updates.

## Features

- **Automated vcpkg installation**: Installs and configures the vcpkg package manager
- **External dependency management**: Installs required third-party libraries
- **Library system updates**: Manages and updates all internal library systems
- **Cross-platform support**: Works on Linux, macOS, and Windows (MSYS2/Cygwin)
- **Modular commands**: Flexible command structure for specific tasks
- **Colored output**: Enhanced readability with color-coded messages
- **Build integration**: Can build the entire project after dependency installation

## Usage

```bash
./dependency.sh [command]
```

### Commands

| Command | Description |
|---------|-------------|
| `vcpkg` | Install vcpkg package manager only |
| `deps` or `dependencies` | Install external dependencies via vcpkg |
| `libs` or `libraries` | Update/sync internal library systems |
| `build` | Build the project using CMake |
| `all` | Run all steps (default if no command specified) |
| `help` | Show usage information |

### Examples

```bash
# Install everything and update all libraries (default)
./dependency.sh

# Install only external dependencies
./dependency.sh deps

# Update only library systems
./dependency.sh libs

# Build the project
./dependency.sh build

# Show help
./dependency.sh help
```

## Library Systems

The script manages the following library systems:

- **thread_system**: Threading and concurrency utilities
- **logger_system**: Comprehensive logging framework
- **monitoring_system**: System monitoring and metrics
- **container_system**: Custom container implementations
- **database_system**: Database connectivity and ORM
- **network_system**: Network communication utilities

Each library system can be:
- **Integrated**: Part of the main project repository
- **External**: Separate git repository that can be updated independently

## External Dependencies

The script installs the following external dependencies via vcpkg:

- **fmt**: Modern formatting library
- **gtest**: Google Test framework
- **benchmark**: Microbenchmarking support
- **spdlog**: Fast C++ logging library
- **libpqxx**: PostgreSQL C++ client API
- **asio**: Asynchronous I/O library
- **nlohmann-json**: JSON for Modern C++

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `VCPKG_ROOT` | Path to vcpkg installation | `$HOME/vcpkg` |

## Requirements

- **Bash**: Version 3.2 or higher (compatible with macOS default)
- **Git**: For cloning repositories
- **CMake**: For building the project
- **C++ Compiler**: GCC, Clang, or MSVC

## Troubleshooting

### vcpkg Installation Fails

If vcpkg installation fails, you can manually install it:

```bash
cd $HOME
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # On Linux/macOS
./bootstrap-vcpkg.bat  # On Windows
```

### Library Update Issues

If a library system fails to update:

1. Check if the library directory exists: `ls -la libraries/`
2. Verify git connectivity: `git ls-remote https://github.com/kcenon/[library_name].git`
3. Manually update if needed: `cd libraries/[library_name] && git pull`

### Build Failures

If the build fails after dependency installation:

1. Clean the build directory: `rm -rf build/`
2. Re-run CMake configuration: `cmake -B build -DCMAKE_BUILD_TYPE=Release`
3. Build with verbose output: `cmake --build build --verbose`

## Contributing

When adding new library systems or dependencies:

1. Update the `LIBRARY_SYSTEMS` variable in `dependency.sh`
2. Add new vcpkg dependencies to the `DEPENDENCIES` array
3. Test the script on all supported platforms
4. Update this documentation

## License

This dependency management system is part of the messaging_system project and follows the same BSD 3-Clause License.