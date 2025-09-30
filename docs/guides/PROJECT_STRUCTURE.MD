# Project Structure Guide

This document describes the organization and purpose of each directory in the messaging_system project.

## Overview

The project follows a modular, purpose-driven structure that separates concerns and makes navigation intuitive.

## Root Directory Structure

```
messaging_system/
├── application_layer/      # Application service layer
├── cmake/                  # CMake modules and configuration
├── docs/                   # All documentation
├── examples/               # Usage examples
├── include/                # Public C++ headers
├── libraries/              # Core system libraries
├── scripts/                # Build and utility scripts
├── src/                    # Implementation files
├── test/                   # Unit and integration tests
├── tests/                  # Additional test suites
├── tools/                  # Development tools
├── CMakeLists.txt          # Main build configuration
├── LICENSE                 # Project license
├── README.md               # Main project documentation
└── vcpkg.json              # External dependencies
```

## Directory Purposes

### `/application_layer`
Application-level integration and services.

```
application_layer/
├── include/            # Application headers
├── python_bindings/    # Python API bindings
├── samples/            # Production-ready examples
├── src/                # Application implementation
└── tests/              # Application tests
```

**Purpose**: High-level messaging services that integrate all core libraries into cohesive applications.

### `/cmake`
CMake build system configuration files.

```
cmake/
├── modules/            # Custom CMake modules
└── *.cmake             # Configuration scripts
```

**Purpose**: Shared CMake functions, macros, and configuration for consistent builds across platforms.

### `/docs`
Comprehensive project documentation.

```
docs/
├── api/                # API reference documentation
├── guides/             # User guides and tutorials
│   ├── ARCHITECTURE.md
│   ├── DEPENDENCIES.md
│   ├── PERFORMANCE.md
│   └── testing.md
├── libraries/          # Library-specific documentation
│   ├── container_system.md
│   ├── logger_system.md
│   ├── monitoring_system.md
│   ├── network_system.md
│   └── thread_system.md
├── samples/            # Sample application guides
│   └── README.md
├── application_layer.md
├── API_REFERENCE.md
├── DEPLOYMENT_GUIDE.md
├── DESIGN_PATTERNS.md
├── DEVELOPER_GUIDE.md
├── GETTING_STARTED.md
├── README.md
└── TROUBLESHOOTING.md
```

**Purpose**: Centralized documentation hub for all project aspects.

### `/examples`
Standalone usage examples demonstrating integration patterns.

**Purpose**: Quick-start code samples showing how to use the messaging system in various scenarios.

### `/include`
Public C++ header files for external consumers.

```
include/
└── kcenon/
    └── messaging/
        ├── result_adapter.h
        └── unified_messaging_system.h
```

**Purpose**: Clean public API headers that external projects can include.

### `/libraries`
Core system libraries - the foundation of the messaging system.

```
libraries/
├── container_system/       # Type-safe data containers
├── logger_system/          # High-performance logging
├── monitoring_system/      # System monitoring and metrics
├── network_system/         # Asynchronous TCP messaging
└── thread_system/          # Lock-free thread pools
```

**Purpose**: Modular, reusable libraries that can be used independently or as part of the unified system.

Each library follows a consistent structure:
```
library_name/
├── core/               # Core functionality
├── internal/           # Internal implementation details
├── docs/               # Library-specific documentation
├── tests/              # Library unit tests
├── CMakeLists.txt      # Build configuration
└── README.md           # Library documentation
```

### `/scripts`
Build and utility scripts for development and deployment.

```
scripts/
├── build.bat               # Windows build script
├── build.sh                # Unix build script
├── dependency.sh           # Dependency management
├── setup_external_deps.sh  # External dependency setup
└── test_modular_build.sh   # Modular build testing
```

**Purpose**: Automation scripts for common development tasks. All scripts are maintained in this central location.

**Usage**:
```bash
# Build the project
./scripts/build.sh

# Install dependencies
./scripts/dependency.sh

# Run tests
./scripts/build.sh --tests
```

### `/src`
Implementation files for the unified messaging system.

```
src/
└── unified_messaging_system.cpp
```

**Purpose**: Source code that integrates all libraries into the unified messaging system.

### `/test` and `/tests`
Test suites and test infrastructure.

```
test/
└── unittest/           # Unit tests
    ├── CMakeLists.txt
    └── README.md

tests/
└── unified_messaging_test.cpp
```

**Purpose**: Comprehensive testing at different levels:
- **Unit tests**: Test individual components in isolation
- **Integration tests**: Test component interactions
- **System tests**: End-to-end testing

### `/tools`
Development and maintenance tools.

**Purpose**: Helper tools for development, such as code generators, analyzers, or deployment utilities.

## File Organization Principles

### 1. Separation of Concerns
- **Source code** (`/src`, `/libraries`): Implementation
- **Headers** (`/include`): Public API
- **Documentation** (`/docs`): All documentation
- **Scripts** (`/scripts`): Build and automation
- **Tests** (`/test`, `/tests`): Validation

### 2. Modular Design
Each library is self-contained with its own:
- CMakeLists.txt
- Documentation
- Tests
- Headers and implementation

### 3. Clear Entry Points
- **For users**: Start with `/docs/GETTING_STARTED.md`
- **For developers**: Check `/docs/DEVELOPER_GUIDE.md`
- **For examples**: Browse `/application_layer/samples/`
- **For building**: Run `./scripts/build.sh`

## Build Output

Build artifacts are generated in the `build/` directory (gitignored):

```
build/
├── bin/                # Executable files
├── lib/                # Static/shared libraries
└── CMakeFiles/         # CMake generated files
```

## Navigation Tips

### Finding Documentation
1. **General docs**: `/docs/`
2. **Library-specific**: `/docs/libraries/`
3. **Sample explanations**: `/docs/samples/`
4. **Build guides**: `/docs/guides/`

### Finding Code
1. **Public API**: `/include/kcenon/`
2. **Implementation**: `/src/` and `/libraries/*/`
3. **Examples**: `/application_layer/samples/`
4. **Tests**: `/test/` and `/tests/`

### Building and Testing
1. **Build scripts**: `/scripts/`
2. **CMake config**: `CMakeLists.txt` (root and library-specific)
3. **Test execution**: Via `./scripts/build.sh --tests`

## Best Practices

### When Adding New Features
1. Add implementation to appropriate `/libraries/` or `/src/`
2. Add public headers to `/include/`
3. Add tests to `/test/` or library-specific test directory
4. Document in `/docs/`
5. Add usage example to `/application_layer/samples/` if appropriate

### When Modifying Structure
1. Update this guide (`/docs/guides/PROJECT_STRUCTURE.md`)
2. Update main `README.md` if user-facing
3. Update `CMakeLists.txt` as needed
4. Verify all documentation links still work

### When Adding Dependencies
1. Add to `vcpkg.json` for external dependencies
2. Update `/docs/guides/DEPENDENCIES.md`
3. Modify `/scripts/dependency.sh` if special setup needed

## Migration Notes

### Recent Changes
- **Scripts moved**: All build scripts moved from root to `/scripts/`
- **Docs reorganized**: All documentation consolidated in `/docs/`
- **Guides categorized**: Documentation split into logical subdirectories

### Backward Compatibility
- Old documentation paths may still exist in library subdirectories
- Use centralized `/docs/` for all new documentation
- Script paths updated in all documentation

## See Also

- [Dependencies Guide](DEPENDENCIES.md) - External dependency management
- [Developer Guide](../DEVELOPER_GUIDE.md) - Development workflows
- [Architecture Guide](ARCHITECTURE.md) - System architecture overview