# Developer Guide

## Quick Start

This guide helps developers get up and running with the Messaging System quickly and efficiently.

### Prerequisites

Before starting, ensure you have:
- C++20 compatible compiler (GCC 10+, Clang 12+, or MSVC 2019+)
- CMake 3.16 or later
- Git
- PostgreSQL 12+ (optional, for database features)
- Python 3.8+ (optional, for Python bindings)

### 1. Clone and Setup

```bash
# Clone the repository
git clone <repository-url> messaging_system
cd messaging_system

# Initialize submodules (vcpkg)
git submodule update --init --recursive

# Install system dependencies
./scripts/dependency.sh
```

### 2. Build the Project

```bash
# Quick build with default settings
./scripts/build.sh

# Build with specific options
./scripts/build.sh --tests --release --parallel 8

# Manual CMake build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --parallel
```

### 3. Your First Application

Create a simple messaging application:

```cpp
// simple_app.cpp
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/services/network/network_service.h>
#include <iostream>

using namespace kcenon::messaging;

int main() {
    // Initialize message bus
    core::message_bus_config config;
    config.worker_threads = 4;
    core::message_bus bus(config);
    bus.initialize();

    // Subscribe to messages
    bus.subscribe("greeting", [](const core::message& msg) {
        std::cout << "Received: " << msg.get_payload().get<std::string>("text") << std::endl;
        return core::message_status::processed;
    });

    // Publish a message
    core::message_payload payload;
    payload.set("text", "Hello, Messaging System!");
    bus.publish("greeting", payload);

    // Keep running
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bus.shutdown();
    return 0;
}
```

Compile and run:
```bash
g++ -std=c++20 simple_app.cpp -lmessaging_core -lmessaging_services -o simple_app
./simple_app
```

## Development Setup

### IDE Configuration

#### Visual Studio Code

`.vscode/settings.json`:
```json
{
    "cmake.configureSettings": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_TOOLCHAIN_FILE": "${workspaceFolder}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    },
    "C_Cpp.default.cppStandard": "c++20",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/libraries/*/include",
        "${workspaceFolder}/application_layer/include"
    ]
}
```

#### CLion

1. Open project and set CMake toolchain file
2. Add build configurations for Debug/Release
3. Set C++20 standard in CMake settings

#### Visual Studio

1. Open folder as CMake project
2. Configure CMake settings to use vcpkg toolchain
3. Select x64-Debug or x64-Release configuration

### Development Dependencies

Install development tools:

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    valgrind \
    clang-format \
    clang-tidy \
    doxygen \
    graphviz

# macOS
brew install \
    cmake \
    ninja \
    llvm \
    doxygen \
    graphviz

# Windows (with Chocolatey)
choco install \
    cmake \
    ninja \
    llvm \
    doxygen.install
```

## Project Structure

### Directory Layout

```
messaging_system/
├── libraries/                  # Core system libraries
│   ├── container_system/       # Data serialization
│   ├── database_system/        # Database connectivity
│   ├── network_system/         # Network communication
│   ├── thread_system/          # Concurrency primitives
│   ├── logger_system/          # Logging infrastructure
│   └── monitoring_system/      # Metrics and monitoring
├── application_layer/          # Application services
│   ├── include/               # Public headers
│   ├── src/                   # Implementation
│   ├── samples/               # Example applications
│   ├── tests/                 # Unit and integration tests
│   └── python_bindings/       # Python API
├── test/                      # System-wide tests
├── scripts/                   # Build and utility scripts
├── cmake/                     # CMake modules
└── docs/                      # Documentation
```

### Module Organization

Each module follows this structure:
```
module_name/
├── include/                   # Public headers
│   └── module_name/
│       ├── module.h
│       └── types.h
├── src/                       # Implementation
│   ├── module.cpp
│   └── internal/
│       └── helpers.cpp
├── tests/                     # Module tests
│   ├── unit/
│   └── integration/
├── benchmarks/                # Performance tests
└── CMakeLists.txt
```

## Coding Standards

### C++ Style Guide

#### Naming Conventions

```cpp
// Classes: PascalCase
class MessageProcessor {
public:
    // Methods: snake_case
    void process_message(const Message& msg);

    // Member variables: snake_case with underscore suffix
    size_t message_count_;

    // Constants: UPPER_SNAKE_CASE
    static constexpr size_t MAX_BUFFER_SIZE = 1024;
};

// Functions: snake_case
void send_notification(const std::string& message);

// Namespaces: lowercase
namespace messaging {
namespace core {
    // ...
}
}

// Enums: PascalCase for type, UPPER_SNAKE_CASE for values
enum class MessageType {
    DATA_UPDATE,
    HEARTBEAT,
    ERROR_REPORT
};

// Templates: PascalCase for type parameters
template<typename MessageHandler>
class Router {
    // ...
};
```

#### File Organization

```cpp
// header_file.h
#pragma once

// System includes
#include <memory>
#include <string>
#include <vector>

// Third-party includes
#include <boost/asio.hpp>

// Project includes
#include "messaging/core/types.h"

namespace messaging {

// Forward declarations
class MessageBus;

// Type aliases
using MessagePtr = std::shared_ptr<Message>;

// Constants
constexpr size_t DEFAULT_QUEUE_SIZE = 1000;

// Class definition
class MessageProcessor {
    // Public interface first
public:
    MessageProcessor();
    ~MessageProcessor();

    void process(const Message& msg);

    // Private implementation
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace messaging
```

#### Best Practices

1. **RAII (Resource Acquisition Is Initialization)**
```cpp
class FileHandler {
    std::unique_ptr<std::FILE, decltype(&std::fclose)> file_;
public:
    FileHandler(const std::string& path)
        : file_(std::fopen(path.c_str(), "r"), &std::fclose) {
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }
    // Automatic cleanup on destruction
};
```

2. **Move Semantics**
```cpp
class LargeData {
    std::vector<uint8_t> data_;
public:
    // Move constructor
    LargeData(LargeData&& other) noexcept
        : data_(std::move(other.data_)) {}

    // Move assignment
    LargeData& operator=(LargeData&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }
};
```

3. **Const Correctness**
```cpp
class Container {
    std::vector<int> data_;
public:
    // Const methods don't modify state
    size_t size() const { return data_.size(); }

    // Const and non-const overloads
    int& operator[](size_t idx) { return data_[idx]; }
    const int& operator[](size_t idx) const { return data_[idx]; }
};
```

4. **Smart Pointers**
```cpp
// Use unique_ptr for single ownership
std::unique_ptr<Connection> create_connection();

// Use shared_ptr for shared ownership
class Session : public std::enable_shared_from_this<Session> {
public:
    void start() {
        auto self = shared_from_this();
        async_read([self](auto ec, auto bytes) {
            // Session kept alive during async operation
        });
    }
};

// Use weak_ptr to break cycles
class Node {
    std::shared_ptr<Node> child_;
    std::weak_ptr<Node> parent_;  // Breaks circular reference
};
```

5. **Error Handling**
```cpp
// Use exceptions for exceptional conditions
void critical_operation() {
    if (!precondition_met()) {
        throw std::logic_error("Precondition failed");
    }
}

// Use optional for values that might not exist
std::optional<Config> load_config(const std::string& path) {
    if (!file_exists(path)) {
        return std::nullopt;
    }
    return Config::from_file(path);
}

// Use expected/result for operations that can fail
template<typename T>
using Result = std::expected<T, std::error_code>;

Result<Data> parse_data(const std::string& input) {
    if (input.empty()) {
        return std::unexpected(ErrorCode::InvalidInput);
    }
    return Data{input};
}
```

### Documentation Standards

#### Code Documentation

```cpp
/**
 * @brief Processes incoming messages and routes them to appropriate handlers
 *
 * This class implements a high-performance message processor using lock-free
 * queues and thread pools for concurrent processing.
 *
 * @tparam Handler Type of the message handler callable
 *
 * @code
 * MessageProcessor processor;
 * processor.register_handler("topic", [](const Message& msg) {
 *     std::cout << "Received: " << msg.content() << std::endl;
 * });
 * processor.start();
 * @endcode
 */
template<typename Handler>
class MessageProcessor {
public:
    /**
     * @brief Register a handler for a specific topic
     *
     * @param topic Topic to subscribe to (supports wildcards)
     * @param handler Callable to process messages
     * @return true if registration successful, false otherwise
     *
     * @throws std::invalid_argument if topic is empty
     * @throws std::runtime_error if processor is not initialized
     */
    bool register_handler(const std::string& topic, Handler handler);
};
```

#### Comment Guidelines

```cpp
// Good: Explains WHY
// Use lock-free queue to avoid contention in high-throughput scenarios
queue_.push_lock_free(item);

// Bad: Explains WHAT (obvious from code)
// Push item to queue
queue_.push(item);

// Good: Complex algorithm explanation
// Binary search with interpolation for better cache locality.
// We estimate the position based on value distribution, reducing
// comparisons from O(log n) to O(log log n) in the average case.
size_t pos = interpolation_search(data, target);

// Good: TODO with context
// TODO(john): Optimize for small arrays (<100 elements) using linear search
// Benchmarks show 20% improvement for small datasets
```

## Testing Guidelines

### Unit Testing

#### Test Structure

```cpp
#include <gtest/gtest.h>
#include "messaging/core/message.h"

// Test fixture for shared setup
class MessageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common initialization
        message_ = std::make_unique<Message>();
    }

    void TearDown() override {
        // Cleanup if needed
        message_.reset();
    }

    std::unique_ptr<Message> message_;
};

// Basic test
TEST_F(MessageTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(message_->type(), MessageType::UNDEFINED);
    EXPECT_TRUE(message_->payload().empty());
}

// Parameterized test
class MessageSerializationTest
    : public MessageTest,
      public ::testing::WithParamInterface<SerializationFormat> {};

TEST_P(MessageSerializationTest, SerializeDeserializeRoundTrip) {
    auto format = GetParam();
    message_->set_content("test data");

    auto serialized = message_->serialize(format);
    auto deserialized = Message::deserialize(serialized, format);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->content(), "test data");
}

INSTANTIATE_TEST_SUITE_P(
    Formats,
    MessageSerializationTest,
    ::testing::Values(
        SerializationFormat::BINARY,
        SerializationFormat::JSON,
        SerializationFormat::PROTOBUF
    )
);

// Death test for assertions
TEST_F(MessageTest, NullPayloadCausesDeath) {
    ASSERT_DEATH(message_->set_payload(nullptr), "Assertion.*null");
}
```

#### Test Coverage

Aim for minimum 80% code coverage:

```bash
# Build with coverage
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
make

# Run tests
ctest

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info

# HTML report
genhtml coverage.info --output-directory coverage_report
```

### Integration Testing

```cpp
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start services
        server_ = std::make_unique<Server>(8080);
        server_->start();

        client_ = std::make_unique<Client>();
        client_->connect("localhost", 8080);
    }

    void TearDown() override {
        client_->disconnect();
        server_->stop();
    }

    std::unique_ptr<Server> server_;
    std::unique_ptr<Client> client_;
};

TEST_F(IntegrationTest, ClientServerCommunication) {
    // Send request
    auto request = create_test_request();
    auto future = client_->send_async(request);

    // Wait for response
    auto response = future.get();
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status(), Status::SUCCESS);
}
```

### Performance Testing

```cpp
#include <benchmark/benchmark.h>

static void BM_MessageSerialization(benchmark::State& state) {
    Message msg;
    msg.set_content(std::string(state.range(0), 'x'));

    for (auto _ : state) {
        auto serialized = msg.serialize();
        benchmark::DoNotOptimize(serialized);
    }

    state.SetBytesProcessed(state.iterations() * state.range(0));
}

BENCHMARK(BM_MessageSerialization)->Range(8, 8<<10);

static void BM_LockFreeQueue(benchmark::State& state) {
    LockFreeQueue<int> queue(10000);

    for (auto _ : state) {
        queue.push(42);
        int value;
        queue.pop(value);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_LockFreeQueue)->Threads(1)->Threads(2)->Threads(4)->Threads(8);
```

Run benchmarks:
```bash
./build/benchmarks/message_benchmark --benchmark_format=json > results.json
```

## Debugging

### Using GDB

```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Run with GDB
gdb ./build/bin/my_application

# Common GDB commands
(gdb) break MessageProcessor::process  # Set breakpoint
(gdb) run                              # Start program
(gdb) backtrace                        # Show call stack
(gdb) print message_                   # Inspect variable
(gdb) watch message_count_             # Watch variable changes
(gdb) thread apply all backtrace       # All threads' stacks
```

### Using Valgrind

Memory leak detection:
```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./build/bin/my_application
```

Race condition detection:
```bash
valgrind --tool=helgrind ./build/bin/my_application
```

### Using AddressSanitizer

```bash
# Build with AddressSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
make

# Run with ASAN options
ASAN_OPTIONS=detect_leaks=1:halt_on_error=0 ./build/bin/my_application
```

### Using ThreadSanitizer

```bash
# Build with ThreadSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-fsanitize=thread"
make

# Run with TSAN options
TSAN_OPTIONS=halt_on_error=0:history_size=7 ./build/bin/my_application
```

## Performance Profiling

### Using perf

```bash
# Record performance data
perf record -g ./build/bin/my_application

# Generate report
perf report

# Generate flame graph
perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > flame.svg
```

### Using Instruments (macOS)

```bash
# Time Profiler
instruments -t "Time Profiler" -D trace.trace ./build/bin/my_application

# Allocations
instruments -t "Allocations" -D alloc.trace ./build/bin/my_application
```

### Using Intel VTune

```bash
# Hotspot analysis
vtune -collect hotspots -result-dir vtune_results ./build/bin/my_application

# Threading analysis
vtune -collect threading -result-dir vtune_threading ./build/bin/my_application
```

## Contributing Guidelines

### Workflow

1. **Fork** the repository
2. **Create** a feature branch
```bash
git checkout -b feature/amazing-feature
```

3. **Make** changes following coding standards
4. **Test** your changes
```bash
./scripts/build.sh --tests
ctest --output-on-failure
```

5. **Format** code
```bash
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

6. **Commit** with descriptive message
```bash
git commit -m "feat(network): add WebSocket support

- Implement WebSocket protocol handler
- Add connection upgrade mechanism
- Include unit tests and documentation"
```

7. **Push** to your fork
```bash
git push origin feature/amazing-feature
```

8. **Create** Pull Request with:
   - Clear description of changes
   - Link to related issues
   - Test results
   - Performance impact (if applicable)

### Commit Message Format

Follow Conventional Commits specification:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test additions or fixes
- `build`: Build system changes
- `ci`: CI/CD changes
- `chore`: Other changes

Example:
```
feat(container): add SIMD optimization for numeric operations

Implement ARM NEON and x86 AVX vectorization for bulk numeric
operations in containers. This provides ~3x speedup for large
arrays.

Closes #123
```

### Code Review Checklist

Before submitting PR, ensure:

- [ ] Code follows project style guide
- [ ] All tests pass
- [ ] New tests added for new features
- [ ] Documentation updated
- [ ] No compiler warnings
- [ ] No memory leaks (checked with Valgrind/ASAN)
- [ ] Performance impact assessed
- [ ] Security implications considered
- [ ] Breaking changes documented
- [ ] Changelog updated

## Build System

### CMake Configuration

Basic configuration options:

```cmake
# Build type
-DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo|MinSizeRel

# Feature flags
-DUSE_UNIT_TEST=ON|OFF
-DUSE_BENCHMARK=ON|OFF
-DUSE_DATABASE=ON|OFF
-DUSE_PYTHON_BINDING=ON|OFF

# Optimization flags
-DENABLE_LTO=ON|OFF           # Link-time optimization
-DENABLE_SIMD=ON|OFF          # SIMD optimizations
-DENABLE_SANITIZERS=ON|OFF    # AddressSanitizer/ThreadSanitizer

# Custom paths
-DCMAKE_INSTALL_PREFIX=/custom/path
-DVCPKG_ROOT=/path/to/vcpkg
```

### Creating New Modules

1. Create module structure:
```bash
mkdir -p libraries/my_module/{include/my_module,src,tests}
```

2. Create CMakeLists.txt:
```cmake
# libraries/my_module/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(my_module)

# Define library
add_library(my_module
    src/implementation.cpp
)

# Include directories
target_include_directories(my_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(my_module
    PUBLIC
        container_system
        thread_system
)

# Set properties
set_target_properties(my_module PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Tests
if(USE_UNIT_TEST)
    add_subdirectory(tests)
endif()
```

3. Add to main CMakeLists.txt:
```cmake
add_subdirectory(libraries/my_module)
```

## Deployment

### Docker Deployment

```dockerfile
# Dockerfile
FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    postgresql-client

# Build application
WORKDIR /app
COPY . .
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --parallel

FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libpq5 \
    && rm -rf /var/lib/apt/lists/*

# Copy application
COPY --from=builder /app/build/bin/* /usr/local/bin/
COPY --from=builder /app/build/lib/* /usr/local/lib/

# Configuration
ENV LD_LIBRARY_PATH=/usr/local/lib
EXPOSE 8080

CMD ["/usr/local/bin/messaging_server"]
```

Build and run:
```bash
docker build -t messaging-system .
docker run -p 8080:8080 messaging-system
```

### Kubernetes Deployment

```yaml
# deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: messaging-system
spec:
  replicas: 3
  selector:
    matchLabels:
      app: messaging-system
  template:
    metadata:
      labels:
        app: messaging-system
    spec:
      containers:
      - name: messaging
        image: messaging-system:latest
        ports:
        - containerPort: 8080
        env:
        - name: WORKER_THREADS
          value: "8"
        - name: DB_HOST
          valueFrom:
            secretKeyRef:
              name: db-secret
              key: host
        resources:
          requests:
            memory: "1Gi"
            cpu: "500m"
          limits:
            memory: "2Gi"
            cpu: "1000m"
        livenessProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /ready
            port: 8080
          initialDelaySeconds: 5
          periodSeconds: 5
---
apiVersion: v1
kind: Service
metadata:
  name: messaging-service
spec:
  selector:
    app: messaging-system
  ports:
  - port: 8080
    targetPort: 8080
  type: LoadBalancer
```

Deploy:
```bash
kubectl apply -f deployment.yaml
kubectl get pods -l app=messaging-system
kubectl logs -f deployment/messaging-system
```

## Security Best Practices

### Input Validation

```cpp
class InputValidator {
public:
    static bool validate_message(const Message& msg) {
        // Check size limits
        if (msg.size() > MAX_MESSAGE_SIZE) {
            return false;
        }

        // Validate UTF-8 encoding
        if (!is_valid_utf8(msg.content())) {
            return false;
        }

        // Check for injection attacks
        if (contains_sql_injection(msg.content())) {
            return false;
        }

        return true;
    }

private:
    static constexpr size_t MAX_MESSAGE_SIZE = 1024 * 1024;  // 1MB
};
```

### Secure Communication

```cpp
class SecureChannel {
    SSL_CTX* ctx_;
    SSL* ssl_;

public:
    SecureChannel() {
        // Initialize OpenSSL
        SSL_library_init();
        SSL_load_error_strings();

        // Create context with TLS 1.3
        ctx_ = SSL_CTX_new(TLS_method());
        SSL_CTX_set_min_proto_version(ctx_, TLS1_3_VERSION);

        // Load certificates
        SSL_CTX_use_certificate_file(ctx_, "cert.pem", SSL_FILETYPE_PEM);
        SSL_CTX_use_PrivateKey_file(ctx_, "key.pem", SSL_FILETYPE_PEM);

        // Verify peer certificate
        SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER, nullptr);
    }
};
```

### Rate Limiting

```cpp
class RateLimiter {
    struct ClientInfo {
        std::atomic<size_t> request_count{0};
        std::chrono::steady_clock::time_point window_start;
    };

    std::unordered_map<std::string, ClientInfo> clients_;
    std::mutex mutex_;
    size_t max_requests_per_window_ = 100;
    std::chrono::seconds window_duration_{60};

public:
    bool allow_request(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();

        auto& info = clients_[client_id];

        // Reset window if expired
        if (now - info.window_start > window_duration_) {
            info.request_count = 0;
            info.window_start = now;
        }

        // Check rate limit
        if (info.request_count >= max_requests_per_window_) {
            return false;
        }

        info.request_count++;
        return true;
    }
};
```

## Troubleshooting Common Issues

### Build Issues

**Problem**: CMake cannot find dependencies
```bash
CMake Error: Could not find PostgreSQL
```

**Solution**:
```bash
# Install PostgreSQL development files
sudo apt-get install libpq-dev  # Ubuntu/Debian
brew install postgresql         # macOS

# Or disable PostgreSQL support
cmake .. -DUSE_DATABASE=OFF
```

**Problem**: Compilation errors with C++20 features
```bash
error: 'concepts' is not a namespace-name
```

**Solution**:
```bash
# Update compiler
sudo apt-get install gcc-11 g++-11
export CXX=g++-11

# Or use Clang
export CXX=clang++-12
```

### Runtime Issues

**Problem**: Segmentation fault on startup
```bash
Segmentation fault (core dumped)
```

**Debugging steps**:
```bash
# Generate core dump
ulimit -c unlimited
./my_application

# Analyze core dump
gdb ./my_application core
(gdb) bt
(gdb) frame 0
(gdb) print <variable>
```

**Problem**: High memory usage
```bash
# Monitor memory usage
valgrind --tool=massif ./my_application
ms_print massif.out.<pid>

# Or use built-in monitoring
MESSAGING_ENABLE_MEMORY_TRACKING=1 ./my_application
```

### Performance Issues

**Problem**: Poor throughput
```bash
# Profile CPU usage
perf top -p <pid>

# Check thread contention
perf record -e sched:sched_switch -p <pid>
perf report

# Monitor lock contention
MESSAGING_ENABLE_LOCK_PROFILING=1 ./my_application
```

**Solution**: Tune configuration
```ini
[performance]
worker_threads = 16          # Increase threads
queue_size = 100000          # Larger queues
batch_size = 100            # Process in batches
enable_cpu_affinity = true  # Pin threads to CPUs
```