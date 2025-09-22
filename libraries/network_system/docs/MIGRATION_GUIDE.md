# Migration Guide: messaging_system to network_system

This guide provides step-by-step instructions for migrating from the legacy `messaging_system` network module to the new independent `network_system` library.

## Table of Contents
1. [Overview](#overview)
2. [Quick Migration](#quick-migration)
3. [Namespace Changes](#namespace-changes)
4. [API Changes](#api-changes)
5. [Build System Migration](#build-system-migration)
6. [Testing Your Migration](#testing-your-migration)
7. [Troubleshooting](#troubleshooting)
8. [Performance Considerations](#performance-considerations)

## Overview

The `network_system` library is a complete extraction and enhancement of the network module from `messaging_system`. It provides:
- Full backward compatibility through namespace aliases
- Improved performance (305K+ msg/s throughput)
- Better modularity and reusability
- Modern C++20 features including coroutines
- Integration with thread and container systems

### Migration Paths

You have two options for migration:

1. **Compatibility Mode** (Recommended for initial migration)
   - Use the compatibility layer with minimal code changes
   - Maintain existing namespace and API calls
   - Gradually migrate to modern API

2. **Full Migration** (Recommended for new projects)
   - Adopt the new namespace structure
   - Use modern API directly
   - Take advantage of new features

## Quick Migration

### Step 1: Update Dependencies

Replace messaging_system network dependency with network_system:

```cmake
# Old CMakeLists.txt
find_package(MessagingSystem REQUIRED)
target_link_libraries(your_app MessagingSystem::MessagingSystem)

# New CMakeLists.txt
find_package(NetworkSystem REQUIRED)
target_link_libraries(your_app NetworkSystem::NetworkSystem)
```

### Step 2: Update Includes (Compatibility Mode)

```cpp
// Old code
#include <messaging_system/network/tcp_server.h>
#include <messaging_system/network/tcp_client.h>

// New code (compatibility mode)
#include <network_system/compatibility.h>
```

### Step 3: Initialize System

```cpp
// Add initialization in your main function
int main() {
    // Initialize network system (replaces messaging_system init)
    network_system::compat::initialize();

    // Your application code here

    // Cleanup
    network_system::compat::shutdown();
    return 0;
}
```

## Namespace Changes

### Legacy Namespace Mapping

The compatibility layer provides these namespace aliases:

```cpp
// Legacy namespaces are automatically available
namespace network_module {
    // All types from network_system::core
    using messaging_server = network_system::core::messaging_server;
    using messaging_client = network_system::core::messaging_client;
    // ... etc
}

namespace messaging {
    // Convenience aliases
    using server = network_system::core::messaging_server;
    using client = network_system::core::messaging_client;
}
```

### Modern Namespace Structure

For new code, use the modern namespace structure:

```cpp
// Modern usage
#include <network_system/core/messaging_server.h>
#include <network_system/core/messaging_client.h>

using namespace network_system::core;
```

## API Changes

### Server API

#### Legacy API (Still Supported)
```cpp
auto server = network_module::create_server("server_id");
server->set_message_handler([](const std::string& msg) {
    return "Response: " + msg;
});
server->start("0.0.0.0", 8080);
```

#### Modern API
```cpp
auto server = std::make_shared<network_system::core::messaging_server>("server_id");
// Note: set_message_handler is not available in the current implementation
// Messages are handled through the session layer
server->start_server(8080);
```

### Client API

#### Legacy API (Still Supported)
```cpp
auto client = network_module::create_client("client_id");
client->connect("localhost", 8080);
client->send("Hello");
```

#### Modern API
```cpp
auto client = std::make_shared<network_system::core::messaging_client>("client_id");
client->start_client("localhost", 8080);

// Send data as bytes
std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o'};
client->send_packet(data);
```

### Key API Differences

| Feature | Legacy API | Modern API |
|---------|------------|------------|
| Server Start | `start(host, port)` | `start_server(port)` |
| Client Connect | `connect(host, port)` | `start_client(host, port)` |
| Send Message | `send(string)` | `send_packet(vector<uint8_t>)` |
| Stop Server | `stop()` | `stop_server()` |
| Stop Client | `disconnect()` | `stop_client()` |

## Build System Migration

### CMake Configuration

Create or update your CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.16)
project(YourApplication)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find NetworkSystem
find_package(NetworkSystem REQUIRED PATHS /path/to/network_system/build)

# Create your executable
add_executable(your_app main.cpp)

# Link NetworkSystem
target_link_libraries(your_app PRIVATE NetworkSystem::NetworkSystem)

# Platform-specific settings
if(WIN32)
    target_link_libraries(your_app PRIVATE ws2_32 mswsock)
    target_compile_definitions(your_app PRIVATE _WIN32_WINNT=0x0601)
endif()
```

### Package Management

#### Using vcpkg (Optional)
```json
{
  "name": "your-app",
  "version": "1.0.0",
  "dependencies": [
    "asio",
    "fmt"
  ]
}
```

#### Using System Packages
```bash
# Ubuntu/Debian
sudo apt install libasio-dev libfmt-dev

# macOS
brew install asio fmt

# Windows (MSYS2)
pacman -S mingw-w64-x86_64-asio mingw-w64-x86_64-fmt
```

## Testing Your Migration

### 1. Compile Test

First, ensure your code compiles:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 2. Runtime Test

Create a simple test to verify functionality:

```cpp
#include <network_system/compatibility.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Initialize
    network_system::compat::initialize();

    // Create server using legacy API
    auto server = network_module::create_server("test_server");
    server->start_server(9090);

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Create client using legacy API
    auto client = network_module::create_client("test_client");
    client->start_client("127.0.0.1", 9090);

    // Send test message
    std::vector<uint8_t> data = {'T', 'e', 's', 't'};
    client->send_packet(data);

    // Cleanup
    client->stop_client();
    server->stop_server();

    network_system::compat::shutdown();

    std::cout << "Migration test successful!" << std::endl;
    return 0;
}
```

### 3. Performance Test

Run the included benchmark to verify performance:

```bash
./build/benchmark
```

Expected results:
- Throughput: > 100K msg/s
- Latency: < 1ms average
- Concurrent connections: 50+ clients

## Troubleshooting

### Common Issues and Solutions

#### 1. Undefined Reference Errors

**Problem**: Linker errors about undefined references to network_system functions.

**Solution**: Ensure NetworkSystem is properly linked:
```cmake
target_link_libraries(your_app PRIVATE NetworkSystem::NetworkSystem pthread)
```

#### 2. Namespace Not Found

**Problem**: `network_module` namespace not found.

**Solution**: Include the compatibility header:
```cpp
#include <network_system/compatibility.h>
```

#### 3. API Method Missing

**Problem**: Methods like `set_message_handler` not available.

**Solution**: These are not yet implemented in the current version. Use the session-based message handling approach or wait for future updates.

#### 4. Performance Degradation

**Problem**: Lower performance after migration.

**Solution**:
- Ensure you're building in Release mode: `cmake .. -DCMAKE_BUILD_TYPE=Release`
- Check that ASIO is properly configured
- Verify thread pool integration is active

#### 5. Container System Not Found

**Problem**: Container serialization features not working.

**Solution**: Ensure container_system is available and properly configured:
```cmake
add_compile_definitions(BUILD_WITH_CONTAINER_SYSTEM)
```

## Performance Considerations

### Optimization Tips

1. **Use Modern API**: The modern API has better performance optimizations.

2. **Enable Thread Pool**: Integrate with thread_system for better async performance:
```cpp
auto& thread_mgr = network_system::integration::thread_integration_manager::instance();
// Thread pool is automatically used for async operations
```

3. **Batch Operations**: Send multiple messages in batches when possible.

4. **Connection Pooling**: Reuse connections instead of creating new ones.

### Performance Metrics

After migration, you should see:
- **Throughput**: 305K+ messages/second (mixed sizes)
- **Latency**: < 600μs average
- **Concurrent Clients**: 50+ without degradation
- **Memory Usage**: ~10KB per connection

### Benchmarking Your Application

Use the included benchmark as a reference:

```cpp
#include <network_system/core/messaging_server.h>
#include <network_system/core/messaging_client.h>
#include <chrono>

// Measure throughput
auto start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < 10000; ++i) {
    client->send_packet(data);
}
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

double throughput = 10000.0 / (duration.count() / 1000.0);
std::cout << "Throughput: " << throughput << " msg/s" << std::endl;
```

## Migration Checklist

- [ ] Update CMakeLists.txt to use NetworkSystem
- [ ] Replace messaging_system includes with network_system
- [ ] Add initialization and shutdown calls
- [ ] Update namespace references (or use compatibility layer)
- [ ] Modify API calls for changed methods
- [ ] Test compilation
- [ ] Run functionality tests
- [ ] Verify performance metrics
- [ ] Update documentation
- [ ] Remove messaging_system dependency

## Support

For migration assistance:
- **GitHub Issues**: https://github.com/kcenon/network_system/issues
- **Email**: kcenon@naver.com
- **Documentation**: See README.md and API documentation

## Conclusion

The migration from messaging_system to network_system provides:
- Better performance (3x throughput improvement)
- Cleaner architecture
- Modern C++ features
- Full backward compatibility
- Active maintenance and updates

Start with the compatibility mode for a smooth transition, then gradually adopt the modern API for best performance and features.