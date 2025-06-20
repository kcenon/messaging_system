[![CodeFactor](https://www.codefactor.io/repository/github/kcenon/messaging_system/badge)](https://www.codefactor.io/repository/github/kcenon/messaging_system)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/cacfad55304e44c3af21d3a6b5bcb4dd)](https://www.codacy.com/gh/kcenon/messaging_system/dashboard?utm_source=github.com&utm_medium=referral&utm_content=kcenon/messaging_system&utm_campaign=Badge_Grade)

# Messaging System

A high-performance, modern C++20 networking framework designed to simplify TCP-based communication systems. This project provides comprehensive threading, container serialization, and asynchronous networking capabilities.

## Overview

The Messaging System is a production-ready framework that empowers developers to build scalable client-server applications without the typical complexity of manual socket management and protocol implementation. Built with modern C++20 features and offering full Python compatibility.

## Features

- **High-Performance Networking**: Asynchronous TCP client/server with non-blocking I/O
- **Type-Safe Containers**: Serializable data containers with variant-based value storage
- **Thread System**: Lock-free job queue and thread pool management
- **Database Integration**: PostgreSQL support for persistent message storage
- **Cross-Language Support**: Full Python implementation with protocol compatibility
- **SIMD Optimizations**: Hardware-accelerated data processing
- **Error Handling**: Comprehensive error reporting and recovery mechanisms
- **Memory Safety**: RAII principles and smart pointers throughout

## Project Structure

```
messaging_system/
├── container/                    # Serializable data containers
│   ├── core/                     # Core container and value classes
│   ├── values/                   # Specialized value implementations
│   └── internal/                 # Thread-safe and SIMD optimizations
├── network/                      # Networking components
│   ├── core/                     # Client and server implementations
│   ├── session/                  # Session management
│   └── internal/                 # Protocol and pipeline handling
├── database/                     # Database integration
│   ├── postgres_manager.h/cpp    # PostgreSQL support
│   └── database_manager.h/cpp    # Generic database interface
├── python/                       # Python implementation
│   ├── messaging_system/         # Main Python package
│   ├── pyproject.toml            # Python packaging
│   └── setup.py                  # Installation script
├── samples/                      # Example applications
│   └── basic_demo/               # Basic functionality demonstration
├── unittest/                     # Unit tests (Google Test)
├── thread_system/                # Thread management (submodule)
├── CMakeLists.txt               # Main build configuration
├── vcpkg.json                   # C++ dependencies
├── build.sh                     # Build script
└── dependency.sh               # Dependency installation
```

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.16 or higher
- vcpkg package manager
- Python 3.7+ (for Python bindings)
- PostgreSQL (for database features)

## Building

### Using Build Scripts

```bash
# Install dependencies
./dependency.sh        # Linux/macOS
./dependency.bat       # Windows

# Build the project
./build.sh             # Linux/macOS
./build.bat            # Windows
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure with vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build . --config Release
```

## Usage Examples

### Basic Server Example

```cpp
#include "network/core/messaging_server.h"
#include "container/core/container.h"

// Create and configure server
auto server = std::make_shared<network_module::messaging_server>("my_server");

// Set up event handlers
server->on_new_client([](const std::string& client_id) {
    std::cout << "Client connected: " << client_id << std::endl;
});

server->on_message([&server](const std::string& client_id, const auto& message) {
    // Echo response
    server->send_to_client(client_id, message);
});

// Start server
server->start_server(8080);
```

### Basic Client Example

```cpp
#include "network/core/messaging_client.h"
#include "container/core/container.h"

// Create client
auto client = std::make_shared<network_module::messaging_client>("client_001");

// Connect to server
client->start_client("localhost", 8080);

// Create and send message
auto container = std::make_shared<container_module::value_container>();
container->set_message_type("greeting");
container->add(std::make_shared<container_module::value>(
    "text", container_module::value_types::string_value, "Hello, World!"
));

client->send_packet(container->serialize());
```

### Python Example

```python
# Install Python package
pip install -e python/

# Server
from messaging_system.network import MessagingServer

def handle_message(client_id: str, message):
    print(f"Received from {client_id}: {message}")

server = MessagingServer(recv_callback=handle_message)
server.start("0.0.0.0", 8080)

# Client
from messaging_system.network import MessagingClient
from messaging_system.container import Container

client = MessagingClient("client_001", "key")
client.start("localhost", 8080)

message = Container()
message.create(message_type="test")
client.send_packet(message)
```

## Testing

### Running Tests

```bash
# C++ unit tests
cd build
ctest --verbose

# Python tests
cd python
python -m pytest tests/

# Run samples
./build/bin/samples/basic_demo
python -m messaging_system.samples.simple_server
```

## Key Components

### Container System
- **`value_container`**: High-level message container with routing
- **`value`**: Type-safe polymorphic value storage  
- **Supported types**: null, bool, integers, floats, strings, binary, nested containers
- **Thread-safe operations**: Lock-free access patterns
- **SIMD optimizations**: Hardware-accelerated processing

### Network System  
- **`messaging_server`**: Multi-client asynchronous TCP server
- **`messaging_client`**: Robust TCP client with reconnection
- **Frame-based protocol**: Reliable packet transmission
- **Event-driven architecture**: Callback-based network events
- **Session management**: Per-client state tracking

### Database Integration
- **`postgres_manager`**: PostgreSQL-specific implementation
- **Container persistence**: Direct serialization support
- **Transaction support**: ACID-compliant operations
- **Connection pooling**: Efficient resource management

### Thread System Integration
- **Lock-free job queue**: High-performance task processing
- **Type-based scheduling**: Priority-aware job execution
- **Performance monitoring**: Real-time metrics collection

## License

This project is licensed under the BSD 3-Clause License - see the LICENSE file for details.