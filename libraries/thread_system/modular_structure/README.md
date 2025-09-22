# Modular Thread System Structure

This directory contains the new modular structure for the thread_system project.

## Directory Structure

```
modular_structure/
├── core/               # Core thread_system (minimal dependencies)
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── thread_system/
│   │       ├── interfaces/
│   │       │   ├── logger_interface.h
│   │       │   ├── monitoring_interface.h
│   │       │   └── thread_context.h
│   │       ├── thread_base/
│   │       ├── thread_pool/
│   │       └── typed_thread_pool/
│   └── src/
│       ├── interfaces/
│       ├── thread_base/
│       ├── thread_pool/
│       └── typed_thread_pool/
│
└── optional/          # Optional integrations (separate repositories)
    ├── logger_integration/
    └── monitoring_integration/
```

## Module Dependencies

### Core Module (thread_system)
- **No external dependencies** (except standard library and vcpkg packages)
- Provides interfaces for logger and monitoring
- Contains all threading functionality

### Optional Modules
- **logger_system**: Implements logger_interface
- **monitoring_system**: Implements monitoring_interface
- Both depend on core thread_system for interfaces

## Build Configuration

Each module will have its own:
- CMakeLists.txt with proper export targets
- Package configuration for find_package
- Version information
- CI/CD pipeline

## Migration Plan

1. **Core Module Setup** (Current)
   - Extract core threading components
   - Set up interface headers
   - Configure CMake exports

2. **Optional Module Templates**
   - Create integration templates
   - Document integration patterns
   - Provide migration examples

3. **Compatibility Layer**
   - Maintain backward compatibility
   - Provide migration headers
   - Gradual deprecation notices