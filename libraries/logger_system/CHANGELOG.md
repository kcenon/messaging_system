# Changelog

All notable changes to the Logger System will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial implementation of logger system
- Asynchronous and synchronous logging modes
- Console writer with color support
- Thread-safe operations
- Integration with Thread System via logger_interface
- CMake build system and packaging
- Comprehensive documentation
- GitHub Actions CI/CD pipeline

### Changed
- Nothing yet

### Deprecated
- Nothing yet

### Removed
- Nothing yet

### Fixed
- Nothing yet

### Security
- Nothing yet

## [1.0.0] - 2025-01-12

### Added
- **Core Features**
  - High-performance logger implementation
  - Lock-free asynchronous logging mode
  - Synchronous logging mode for immediate output
  - Six log levels: trace, debug, info, warning, error, critical
  - Log level filtering
  - Source location tracking (file, line, function)

- **Writers**
  - Console writer with automatic color detection
  - Base writer class for custom implementations
  - Thread-safe writer management
  - Multiple writer support

- **Integration**
  - Thread System logger_interface implementation
  - Service container integration
  - Direct usage as standalone library

- **Performance**
  - Configurable buffer sizes
  - Batch processing in async mode
  - Minimal allocation strategy
  - Lock-free enqueue operations (planned)

- **Build System**
  - CMake 3.16+ support
  - FetchContent integration
  - Package installation support
  - Cross-platform compatibility (Linux, macOS, Windows)

- **Documentation**
  - Getting started guide
  - Architecture overview
  - API reference
  - Performance guide
  - Custom writer tutorial
  - Contributing guidelines

- **Testing**
  - Unit test framework
  - Benchmark framework
  - CI/CD with GitHub Actions

### Known Issues
- File writer not yet implemented (example provided in docs)
- Lock-free queue uses mutex (true lock-free planned)
- No log rotation built-in

## Future Releases

### [1.1.0] - Planned
- File writer with rotation support
- True lock-free queue implementation
- Structured logging (JSON format)
- Network writer
- Syslog writer

### [1.2.0] - Planned
- Log aggregation features
- Sampling for high-frequency logs
- Runtime configuration
- Hot-reload of configuration

### [2.0.0] - Future
- C++23 features
- Compile-time format checking
- Zero-allocation mode
- Plugin system for writers

---

[Unreleased]: https://github.com/kcenon/logger_system/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/kcenon/logger_system/releases/tag/v1.0.0