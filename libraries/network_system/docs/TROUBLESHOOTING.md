# Troubleshooting Guide

This guide provides solutions for common issues, debugging techniques, and problem resolution strategies for the Network System.

## Table of Contents

- [Common Issues and Solutions](#common-issues-and-solutions)
- [Debugging Techniques](#debugging-techniques)
- [Performance Problems](#performance-problems)
- [Connection Issues](#connection-issues)
- [Memory Leaks](#memory-leaks)
- [Build Problems](#build-problems)
- [Platform-Specific Issues](#platform-specific-issues)
- [Error Reference](#error-reference)
- [Support Resources](#support-resources)

## Common Issues and Solutions

### Server Won't Start

#### Symptoms
- Server process exits immediately
- No log output
- Service fails to initialize

#### Diagnosis Steps
1. Check system logs
```bash
# Linux/macOS
tail -f /var/log/syslog
journalctl -u network_service -f

# Windows Event Viewer
eventvwr.msc
```

2. Run in debug mode
```bash
./network_server --debug --foreground
```

3. Verify configuration
```bash
./network_server --validate-config
```

#### Common Causes and Solutions

**Port Already in Use**
```bash
# Check if port is in use
lsof -i :8080  # Linux/macOS
netstat -ano | findstr :8080  # Windows

# Solution: Kill the process or use different port
kill -9 <PID>
# Or change configuration
export NETWORK_SERVER_PORT=8081
```

**Permission Denied**
```bash
# Check file permissions
ls -la /etc/network_system/

# Fix permissions
sudo chown -R network_service:network_service /etc/network_system/
sudo chmod 600 /etc/network_system/server.key
```

**Missing Dependencies**
```bash
# Check library dependencies
ldd network_server  # Linux
otool -L network_server  # macOS
dumpbin /DEPENDENTS network_server.exe  # Windows

# Install missing libraries
sudo apt-get install libssl1.1  # Ubuntu/Debian
brew install openssl  # macOS
```

### High CPU Usage

#### Symptoms
- CPU usage consistently above 80%
- System becomes unresponsive
- Increased response times

#### Diagnosis
```bash
# Monitor CPU usage by thread
top -H -p $(pidof network_server)

# Profile CPU usage
perf top -p $(pidof network_server)

# Check thread activity
gdb -p $(pidof network_server)
(gdb) info threads
(gdb) thread apply all bt
```

#### Solutions

**Infinite Loop in Code**
```cpp
// Check for loops without exit conditions
void process_messages() {
    while (running_) {  // Ensure proper exit condition
        if (!message_queue_.empty()) {
            process_message(message_queue_.pop());
        } else {
            // Add sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
```

**Excessive Logging**
```cpp
// Reduce log level in production
logger.set_level(LogLevel::WARNING);

// Use conditional logging
if (logger.should_log(LogLevel::DEBUG)) {
    logger.debug("Expensive debug info: {}", generate_debug_info());
}
```

**Lock Contention**
```cpp
// Use lock-free data structures
std::atomic<int> counter{0};
counter.fetch_add(1, std::memory_order_relaxed);

// Or use read-write locks
std::shared_mutex mutex;
// For reads
std::shared_lock lock(mutex);
// For writes
std::unique_lock lock(mutex);
```

### Memory Issues

#### Symptoms
- Increasing memory usage over time
- Out of memory errors
- System swap usage increases

#### Diagnosis
```bash
# Monitor memory usage
valgrind --leak-check=full --track-origins=yes ./network_server

# Use AddressSanitizer
g++ -fsanitize=address -g network_server.cpp
ASAN_OPTIONS=detect_leaks=1 ./network_server

# Memory profiling
heaptrack ./network_server
heaptrack_gui heaptrack.network_server.*.gz
```

#### Solutions
See [Memory Leaks](#memory-leaks) section for detailed solutions.

## Debugging Techniques

### Core Dump Analysis

#### Enable Core Dumps
```bash
# Enable core dumps
ulimit -c unlimited

# Set core dump pattern
echo "/tmp/core.%e.%p.%t" | sudo tee /proc/sys/kernel/core_pattern
```

#### Analyze Core Dump
```bash
# Open core dump in GDB
gdb network_server core.12345

# Basic commands
(gdb) bt                    # Backtrace
(gdb) info threads          # List all threads
(gdb) thread 3              # Switch to thread 3
(gdb) frame 2               # Switch to frame 2
(gdb) info locals           # Show local variables
(gdb) print variable_name   # Print variable value
```

### Logging and Tracing

#### Enable Debug Logging
```cpp
// Configure detailed logging
LogConfig config;
config.level = LogLevel::TRACE;
config.include_thread_id = true;
config.include_timestamp = true;
config.include_source_location = true;
logger.configure(config);
```

#### Trace Execution Flow
```cpp
class TraceGuard {
    std::string function_name_;
    std::chrono::steady_clock::time_point start_;

public:
    TraceGuard(const std::string& name)
        : function_name_(name), start_(std::chrono::steady_clock::now()) {
        LOG_TRACE("Entering: {}", function_name_);
    }

    ~TraceGuard() {
        auto duration = std::chrono::steady_clock::now() - start_;
        LOG_TRACE("Exiting: {} ({}ms)", function_name_,
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }
};

// Usage
void process_request(const Request& req) {
    TraceGuard guard(__FUNCTION__);
    // Function implementation
}
```

### Network Packet Capture

```bash
# Capture network traffic
sudo tcpdump -i any -w network.pcap port 8080

# Analyze with Wireshark
wireshark network.pcap

# Quick analysis with tcpdump
tcpdump -r network.pcap -nn -A

# Monitor specific connections
sudo netstat -antp | grep network_server
sudo ss -antp | grep network_server
```

### Strace/DTrace Analysis

```bash
# Trace system calls
strace -f -e network -p $(pidof network_server)

# Trace file operations
strace -f -e file -p $(pidof network_server)

# Count system calls
strace -c -p $(pidof network_server)

# macOS dtrace
sudo dtrace -n 'syscall:::entry /pid == $target/ { @[probefunc] = count(); }' -p $(pgrep network_server)
```

## Performance Problems

### Slow Response Times

#### Diagnosis
```cpp
class PerformanceProfiler {
    struct Timing {
        std::chrono::microseconds parse_time;
        std::chrono::microseconds process_time;
        std::chrono::microseconds send_time;
    };

    void profile_request(const Request& req) {
        auto start = std::chrono::steady_clock::now();

        // Parse phase
        auto parsed = parse_request(req);
        auto parse_end = std::chrono::steady_clock::now();

        // Process phase
        auto result = process_request(parsed);
        auto process_end = std::chrono::steady_clock::now();

        // Send phase
        send_response(result);
        auto send_end = std::chrono::steady_clock::now();

        // Log timings
        LOG_INFO("Request timing - Parse: {}μs, Process: {}μs, Send: {}μs",
            duration_cast<microseconds>(parse_end - start).count(),
            duration_cast<microseconds>(process_end - parse_end).count(),
            duration_cast<microseconds>(send_end - process_end).count());
    }
};
```

#### Common Causes

**Database Queries**
```cpp
// Add query caching
class QueryCache {
    std::unordered_map<std::string, CachedResult> cache_;

    Result execute_query(const std::string& query) {
        auto it = cache_.find(query);
        if (it != cache_.end() && !it->second.is_expired()) {
            return it->second.result;
        }

        auto result = db_.execute(query);
        cache_[query] = {result, std::chrono::steady_clock::now()};
        return result;
    }
};
```

**Synchronous I/O**
```cpp
// Convert to async I/O
class AsyncIOHandler {
    std::future<std::string> read_file_async(const std::string& path) {
        return std::async(std::launch::async, [path]() {
            std::ifstream file(path);
            return std::string(std::istreambuf_iterator<char>(file), {});
        });
    }
};
```

**Lock Contention**
```cpp
// Use fine-grained locking
class ConnectionManager {
    struct ConnectionBucket {
        mutable std::mutex mutex;
        std::vector<Connection> connections;
    };

    static constexpr size_t BUCKET_COUNT = 16;
    std::array<ConnectionBucket, BUCKET_COUNT> buckets_;

    size_t get_bucket_index(ConnectionId id) {
        return std::hash<ConnectionId>{}(id) % BUCKET_COUNT;
    }

    Connection& get_connection(ConnectionId id) {
        auto& bucket = buckets_[get_bucket_index(id)];
        std::lock_guard lock(bucket.mutex);
        // Find connection in bucket
    }
};
```

### Throughput Issues

#### Diagnosis
```bash
# Network throughput test
iperf3 -s  # Server side
iperf3 -c server_ip -t 60 -P 10  # Client side

# Application throughput test
wrk -t12 -c400 -d30s --latency http://localhost:8080/
```

#### Solutions

**Increase Buffer Sizes**
```cpp
// Socket buffer tuning
int send_buffer_size = 1048576;  // 1MB
setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF,
    &send_buffer_size, sizeof(send_buffer_size));

int recv_buffer_size = 1048576;  // 1MB
setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF,
    &recv_buffer_size, sizeof(recv_buffer_size));
```

**Batch Processing**
```cpp
class BatchProcessor {
    std::vector<Request> batch_;
    std::mutex batch_mutex_;
    std::condition_variable cv_;

    void add_request(Request req) {
        {
            std::lock_guard lock(batch_mutex_);
            batch_.push_back(std::move(req));
        }

        if (batch_.size() >= BATCH_SIZE) {
            cv_.notify_one();
        }
    }

    void process_batch() {
        std::unique_lock lock(batch_mutex_);
        cv_.wait(lock, [this] {
            return batch_.size() >= BATCH_SIZE || shutdown_;
        });

        auto local_batch = std::move(batch_);
        batch_.clear();
        lock.unlock();

        // Process batch without holding lock
        process_all(local_batch);
    }
};
```

## Connection Issues

### Connection Refused

#### Diagnosis
```bash
# Check if server is listening
netstat -tlnp | grep 8080
ss -tlnp | grep 8080

# Test connectivity
telnet localhost 8080
nc -zv localhost 8080

# Check firewall rules
sudo iptables -L -n -v
sudo ufw status verbose
```

#### Solutions

**Firewall Blocking**
```bash
# Allow port in firewall
sudo ufw allow 8080/tcp
sudo iptables -A INPUT -p tcp --dport 8080 -j ACCEPT
```

**Bind Address Issues**
```cpp
// Bind to all interfaces
address.sin_addr.s_addr = INADDR_ANY;

// Or specific interface
inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
```

### Connection Timeouts

#### Diagnosis
```cpp
class TimeoutDiagnostics {
    void log_timeout_details(const Connection& conn) {
        LOG_ERROR("Connection timeout: {}", conn.id);
        LOG_ERROR("  State: {}", conn.state);
        LOG_ERROR("  Last activity: {}ms ago",
            duration_since(conn.last_activity));
        LOG_ERROR("  Pending writes: {}", conn.write_buffer.size());
        LOG_ERROR("  RTT: {}ms", conn.round_trip_time);
    }
};
```

#### Solutions

**Adjust Timeout Values**
```cpp
// Increase timeouts for slow networks
struct TimeoutConfig {
    std::chrono::seconds connect_timeout{30};
    std::chrono::seconds read_timeout{60};
    std::chrono::seconds write_timeout{60};
    std::chrono::seconds keep_alive_timeout{300};
};
```

**Implement Keep-Alive**
```cpp
// TCP keep-alive
int enable = 1;
setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

int idle = 60;  // Start keep-alive after 60 seconds
setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));

int interval = 10;  // Send keep-alive every 10 seconds
setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));

int count = 6;  // Send 6 keep-alive probes
setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
```

### Connection Drops

#### Diagnosis
```bash
# Monitor connection states
watch 'netstat -tan | grep :8080 | awk "{print \$6}" | sort | uniq -c'

# Check for network errors
netstat -s | grep -i error
ip -s link show
```

#### Solutions

**Handle Partial Writes**
```cpp
ssize_t send_all(int socket, const char* buffer, size_t length) {
    size_t total_sent = 0;

    while (total_sent < length) {
        ssize_t sent = send(socket, buffer + total_sent,
            length - total_sent, MSG_NOSIGNAL);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Wait for socket to become writable
                continue;
            }
            return -1;  // Error
        }

        total_sent += sent;
    }

    return total_sent;
}
```

## Memory Leaks

### Detection

#### Using Valgrind
```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./network_server
```

#### Using AddressSanitizer
```bash
# Compile with AddressSanitizer
g++ -fsanitize=address -fno-omit-frame-pointer -g -O1 network_server.cpp

# Run with leak detection
ASAN_OPTIONS=detect_leaks=1:check_initialization_order=1 ./network_server
```

#### Using HeapTrack
```bash
heaptrack ./network_server
heaptrack_gui heaptrack.network_server.*.gz
```

### Common Memory Leak Patterns

#### Missing Delete
```cpp
// Problem
void process() {
    Buffer* buffer = new Buffer(1024);
    // Missing delete
}

// Solution
void process() {
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(1024);
    // Automatic cleanup
}
```

#### Circular References
```cpp
// Problem
class Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;  // Creates cycle
};

// Solution
class Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;  // Breaks cycle
};
```

#### Container Leaks
```cpp
// Problem
std::vector<Resource*> resources;
resources.push_back(new Resource());
// Vector cleared but resources not deleted

// Solution
std::vector<std::unique_ptr<Resource>> resources;
resources.push_back(std::make_unique<Resource>());
// Automatic cleanup when vector is cleared
```

### Memory Leak Prevention

```cpp
// RAII wrapper for C resources
template<typename T, typename Deleter>
class RAIIWrapper {
    T* resource_;
    Deleter deleter_;

public:
    explicit RAIIWrapper(T* resource, Deleter deleter)
        : resource_(resource), deleter_(deleter) {}

    ~RAIIWrapper() {
        if (resource_) {
            deleter_(resource_);
        }
    }

    // Delete copy operations
    RAIIWrapper(const RAIIWrapper&) = delete;
    RAIIWrapper& operator=(const RAIIWrapper&) = delete;

    // Allow move operations
    RAIIWrapper(RAIIWrapper&& other) noexcept
        : resource_(std::exchange(other.resource_, nullptr)),
          deleter_(std::move(other.deleter_)) {}
};

// Usage
RAIIWrapper file(fopen("data.txt", "r"), fclose);
RAIIWrapper ssl_ctx(SSL_CTX_new(TLS_method()), SSL_CTX_free);
```

## Build Problems

### Compilation Errors

#### Missing Headers
```bash
# Error: fatal error: openssl/ssl.h: No such file or directory

# Solution - Install development packages
sudo apt-get install libssl-dev  # Ubuntu/Debian
sudo yum install openssl-devel  # RHEL/CentOS
brew install openssl  # macOS

# Update include paths
export CPLUS_INCLUDE_PATH=/usr/local/opt/openssl/include:$CPLUS_INCLUDE_PATH
```

#### Undefined References
```bash
# Error: undefined reference to `SSL_library_init'

# Solution - Link libraries correctly
g++ network_server.cpp -lssl -lcrypto -lpthread

# Or in CMakeLists.txt
find_package(OpenSSL REQUIRED)
target_link_libraries(network_server OpenSSL::SSL OpenSSL::Crypto)
```

#### Template Instantiation Issues
```cpp
// Problem: undefined reference to template function

// Solution 1: Define in header
template<typename T>
class Container {
    T* data_;
public:
    T& get() { return *data_; }  // Define inline
};

// Solution 2: Explicit instantiation
// In .cpp file
template class Container<int>;
template class Container<std::string>;
```

### Linking Errors

#### Symbol Conflicts
```bash
# Multiple definition errors

# Check for duplicate symbols
nm -C *.o | grep "T " | sort | uniq -d

# Use unnamed namespaces for internal linkage
namespace {
    void internal_function() { }
}
```

#### Library Order
```bash
# Correct library order (dependencies last)
g++ main.o -lmylib -lssl -lcrypto -lpthread

# Wrong order (will fail)
g++ main.o -lpthread -lcrypto -lssl -lmylib
```

### CMake Issues

#### Finding Packages
```cmake
# Set hints for finding packages
set(CMAKE_PREFIX_PATH "/usr/local/opt/openssl")
find_package(OpenSSL REQUIRED)

# Or use pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENSSL REQUIRED openssl)
```

#### Cross-Platform Builds
```cmake
# Platform-specific settings
if(WIN32)
    set(PLATFORM_LIBS ws2_32)
elseif(APPLE)
    set(PLATFORM_LIBS "-framework CoreFoundation")
else()
    set(PLATFORM_LIBS pthread dl)
endif()

target_link_libraries(network_server ${PLATFORM_LIBS})
```

## Platform-Specific Issues

### Linux Issues

#### File Descriptor Limits
```bash
# Check current limits
ulimit -n

# Increase limits temporarily
ulimit -n 65535

# Increase limits permanently
# /etc/security/limits.conf
* soft nofile 65535
* hard nofile 65535
```

#### Systemd Service Issues
```bash
# Check service status
systemctl status network_service

# View full logs
journalctl -u network_service --no-pager

# Common fixes
systemctl daemon-reload  # After changing service file
systemctl reset-failed network_service  # Clear failed state
```

### macOS Issues

#### Code Signing
```bash
# Sign binary to avoid firewall prompts
codesign -s "Developer ID" network_server

# Allow unsigned binary
spctl --add network_server
```

#### Homebrew Library Paths
```bash
# Fix library paths
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH

# Or use install_name_tool
install_name_tool -change @rpath/libssl.dylib /usr/local/lib/libssl.dylib network_server
```

### Windows Issues

#### DLL Not Found
```powershell
# Check dependencies
dumpbin /DEPENDENTS network_server.exe

# Add to PATH
$env:PATH += ";C:\Program Files\OpenSSL\bin"

# Or copy DLLs to executable directory
copy "C:\Program Files\OpenSSL\bin\*.dll" .
```

#### Windows Firewall
```powershell
# Add firewall exception
netsh advfirewall firewall add rule name="Network Server" dir=in action=allow program="C:\network\network_server.exe"

# Check firewall rules
netsh advfirewall firewall show rule name="Network Server"
```

#### Visual Studio Runtime
```powershell
# Install Visual C++ Redistributable
# Download from Microsoft website

# Or link statically
# In CMake:
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
```

## Error Reference

### Error Codes

```cpp
enum class ErrorCode {
    SUCCESS = 0,

    // Connection errors (1000-1999)
    CONNECTION_REFUSED = 1001,
    CONNECTION_TIMEOUT = 1002,
    CONNECTION_CLOSED = 1003,
    CONNECTION_RESET = 1004,

    // Protocol errors (2000-2999)
    INVALID_MESSAGE = 2001,
    UNSUPPORTED_VERSION = 2002,
    AUTHENTICATION_FAILED = 2003,

    // Resource errors (3000-3999)
    OUT_OF_MEMORY = 3001,
    TOO_MANY_CONNECTIONS = 3002,
    RESOURCE_EXHAUSTED = 3003,

    // System errors (4000-4999)
    FILE_NOT_FOUND = 4001,
    PERMISSION_DENIED = 4002,
    OPERATION_NOT_SUPPORTED = 4003
};

const char* error_to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::CONNECTION_REFUSED:
            return "Connection refused by remote host";
        case ErrorCode::CONNECTION_TIMEOUT:
            return "Connection attempt timed out";
        // ... more cases
    }
}
```

### Error Recovery Strategies

```cpp
class ErrorRecovery {
    template<typename Func>
    auto retry_with_backoff(Func func, int max_retries = 3) {
        int retry = 0;
        std::chrono::milliseconds delay(100);

        while (retry < max_retries) {
            try {
                return func();
            } catch (const std::exception& e) {
                if (++retry >= max_retries) {
                    throw;
                }

                LOG_WARN("Attempt {} failed: {}. Retrying in {}ms",
                    retry, e.what(), delay.count());

                std::this_thread::sleep_for(delay);
                delay *= 2;  // Exponential backoff
            }
        }
    }
};
```

## Support Resources

### Logging Best Practices

```cpp
// Structured logging for better debugging
class StructuredLogger {
    void log_error(const std::string& message,
                   const std::map<std::string, std::any>& context) {
        json log_entry;
        log_entry["timestamp"] = std::chrono::system_clock::now();
        log_entry["level"] = "ERROR";
        log_entry["message"] = message;

        for (const auto& [key, value] : context) {
            log_entry["context"][key] = value;
        }

        std::cerr << log_entry.dump() << std::endl;
    }
};

// Usage
logger.log_error("Connection failed", {
    {"remote_ip", "192.168.1.100"},
    {"port", 8080},
    {"error_code", ECONNREFUSED},
    {"retry_count", 3}
});
```

### Performance Metrics Collection

```cpp
class MetricsCollector {
    void record_latency(const std::string& operation,
                       std::chrono::microseconds duration) {
        histogram_[operation].record(duration.count());

        // Alert on high latency
        if (duration > std::chrono::seconds(1)) {
            alert_manager_.send({
                .severity = AlertSeverity::WARNING,
                .message = fmt::format("{} took {}ms",
                    operation, duration.count() / 1000)
            });
        }
    }
};
```

### Getting Help

1. **Check Logs First**
   - Application logs: `/var/log/network_system/`
   - System logs: `journalctl`, `dmesg`, Event Viewer
   - Enable debug logging for detailed information

2. **Gather Information**
   - Operating system and version
   - Compiler version
   - Library versions
   - Configuration settings
   - Recent changes
   - Error messages and stack traces

3. **Create Minimal Reproduction**
```cpp
// Minimal test case
#include <iostream>
#include "network_system.h"

int main() {
    try {
        NetworkServer server;
        server.configure("config.json");
        server.start();

        // Reproduce issue here

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

4. **Community Resources**
   - GitHub Issues: Report bugs and feature requests
   - Stack Overflow: Search for similar issues
   - Discord/Slack: Real-time community support
   - Documentation: Check API reference and guides

5. **Professional Support**
   - Enterprise support contracts
   - Consulting services
   - Training workshops
   - Priority bug fixes

## Conclusion

This troubleshooting guide covers the most common issues and their solutions. For issues not covered here:

1. Enable debug logging to gather more information
2. Check the error reference for specific error codes
3. Use the debugging techniques to isolate the problem
4. Create a minimal reproduction case
5. Reach out to the community or support channels

Remember to always:
- Keep your system updated
- Monitor logs and metrics
- Implement proper error handling
- Test thoroughly before deployment
- Document any workarounds or custom solutions

For operational procedures, see the [Operations Guide](OPERATIONS.md).
For API documentation, see the [API Reference](API_REFERENCE.md).