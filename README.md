# Messaging System

A comprehensive C++20 messaging system with container, database, and network modules, built on top of the high-performance thread_system framework.

## Features

- **Container Module**: Type-safe, thread-safe data containers with SIMD optimization
- **Database Module**: PostgreSQL integration with connection pooling
- **Network Module**: High-performance TCP messaging with coroutine support
- **Thread System**: Lock-free thread pools, typed job scheduling, and hazard pointers
- **Python Bindings**: Full Python API for all modules

## Dependencies

- C++20 compatible compiler
- CMake 3.16+
- vcpkg (for dependency management)
- PostgreSQL (for database module)
- Python 3.7+ (for Python bindings)

## Thread System Integration

This messaging system now includes the latest thread_system with:

- **Lock-free Components**: High-performance lock-free queues and thread pools
- **Typed Thread Pools**: Priority-based job scheduling (RealTime, Batch, Background)
- **Hazard Pointers**: Safe memory reclamation for lock-free data structures
- **Performance**: 2.14x throughput improvement with lock-free implementations

## Building

```bash
# Install dependencies
./dependency.sh

# Build the project
./build.sh

# Or manually with CMake
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Modules

### Container
- Thread-safe containers with variant value types
- SIMD-optimized operations
- Support for bool, numeric, string, bytes, and nested containers

### Database
- PostgreSQL connection management
- Prepared statement support
- Connection pooling
- Async query execution

### Network
- TCP client/server implementation
- Coroutine-based asynchronous I/O
- Pipeline processing for message handling
- Session management

### Thread System
- Standard and lock-free thread pools
- Type-based job prioritization
- Comprehensive logging system
- Performance monitoring

## Usage

```cpp
// Example: Using the messaging system with lock-free thread pool
#include <container/container.h>
#include <network/messaging_server.h>
#include <thread_pool/core/lockfree_thread_pool.h>

int main() {
    // Create a lock-free thread pool for message processing
    auto pool = std::make_shared<thread_pool_module::lockfree_thread_pool>();
    pool->start();
    
    // Create messaging server
    network::messaging_server server(8080);
    server.set_thread_pool(pool);
    server.start();
    
    // Process messages...
    
    return 0;
}
```

## Python Bindings

```python
from messaging_system import container, network

# Create a container
c = container.Container()
c.set_value("key", 42)

# Start a server
server = network.MessagingServer(8080)
server.start()
```

## Performance

With the integrated lock-free thread_system:
- **Throughput**: Up to 2.48M jobs/second (lock-free) vs 1.16M (mutex-based)
- **Latency**: Sub-microsecond job scheduling
- **Scalability**: Linear scaling up to 16+ threads
- **Memory**: Efficient hazard pointer-based reclamation

## License

BSD 3-Clause License (see LICENSE file)