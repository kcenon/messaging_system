# Troubleshooting Guide

## Table of Contents

1. [FAQ](#faq)
2. [Debug Techniques](#debug-techniques)
3. [Performance Optimization](#performance-optimization)
4. [Known Issues](#known-issues)
5. [Error Messages](#error-messages)
6. [Common Problems](#common-problems)

## FAQ

### General Questions

**Q: What are the minimum system requirements?**
A: The messaging system requires at least 2 CPU cores, 4GB RAM, and 10GB disk space. For production, we recommend 8+ cores, 16GB+ RAM, and SSD storage.

**Q: Which operating systems are supported?**
A: Full support for Ubuntu 20.04+, Debian 11+, RHEL/CentOS 8+, macOS 11+. Partial support for Windows 10/11.

**Q: Can I use this without PostgreSQL?**
A: Yes, compile with `-DUSE_DATABASE=OFF`. However, you'll lose persistence and some features.

**Q: How do I enable Python bindings?**
A: Build with `-DUSE_PYTHON_BINDING=ON` and ensure Python 3.8+ development headers are installed.

**Q: What's the maximum message size?**
A: Default is 1MB, configurable up to 100MB via `max_message_size` setting.

### Performance Questions

**Q: How many concurrent connections can it handle?**
A: Default configuration supports 10,000 connections. With tuning, it can handle 100,000+.

**Q: What's the typical message throughput?**
A: On modern hardware: 2.48M messages/sec with lock-free queues, 1.16M/sec with mutex-based queues.

**Q: How much latency does the system add?**
A: Sub-microsecond for in-memory operations, 1-5ms for network operations, 5-20ms with persistence.

**Q: Can it scale horizontally?**
A: Yes, supports clustering with automatic load balancing and failover.

### Configuration Questions

**Q: Where are configuration files located?**
A: Default location is `/etc/messaging/config.ini`. Override with `MESSAGING_CONFIG_PATH` environment variable.

**Q: How do I change the listening port?**
A: Set `listen_port` in config file or use `MESSAGING_PORT` environment variable.

**Q: Can I use environment variables for secrets?**
A: Yes, use `${VAR_NAME}` syntax in config files to reference environment variables.

## Debug Techniques

### Enable Debug Logging

#### Runtime Configuration
```ini
# config.ini
[logging]
level = debug
verbose = true
include_source_location = true
include_thread_id = true
```

#### Environment Variables
```bash
export MESSAGING_LOG_LEVEL=debug
export MESSAGING_LOG_VERBOSE=1
export MESSAGING_LOG_FILE=/tmp/messaging_debug.log
```

#### Programmatic
```cpp
logger_manager::instance().set_level(log_level::debug);
logger_manager::instance().enable_verbose(true);
```

### Using GDB for Debugging

#### Attach to Running Process
```bash
# Find process ID
ps aux | grep messaging

# Attach GDB
sudo gdb -p <pid>

# Common GDB commands
(gdb) info threads                    # List all threads
(gdb) thread apply all bt             # Backtrace all threads
(gdb) break MessageProcessor::process # Set breakpoint
(gdb) watch message_count_            # Watch variable
(gdb) print *this                     # Print object state
(gdb) call dump_state()               # Call debug function
```

#### Debug Core Dumps
```bash
# Enable core dumps
ulimit -c unlimited
echo "/tmp/core.%e.%p" | sudo tee /proc/sys/kernel/core_pattern

# Run application
./messaging_server

# If it crashes, analyze core dump
gdb ./messaging_server /tmp/core.messaging_server.12345
(gdb) bt full                         # Full backtrace
(gdb) info registers                  # Register state
(gdb) frame 0                         # Select frame
(gdb) list                           # Show source code
```

### Memory Debugging

#### Using Valgrind
```bash
# Memory leak detection
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./messaging_server 2>&1 | tee valgrind.log

# Analyze results
grep "definitely lost" valgrind.log
grep "indirectly lost" valgrind.log
```

#### Using AddressSanitizer
```bash
# Compile with ASAN
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"

# Run with ASAN options
ASAN_OPTIONS=verbosity=3:halt_on_error=0:print_stats=1 \
./messaging_server
```

#### Using HeapTrack
```bash
# Record heap usage
heaptrack ./messaging_server

# Analyze results
heaptrack_gui heaptrack.messaging_server.12345.gz
```

### Network Debugging

#### TCP Dump
```bash
# Capture all traffic on port 8080
sudo tcpdump -i any -w capture.pcap port 8080

# Real-time monitoring with filters
sudo tcpdump -i any -A -s0 'port 8080 and (tcp[tcpflags] & tcp-syn != 0)'

# Analyze with Wireshark
wireshark capture.pcap
```

#### Network Statistics
```bash
# Connection states
ss -tan | grep :8080

# Network statistics
netstat -s | grep -i tcp

# Connection tracking
sudo conntrack -L -p tcp --dport 8080
```

#### Test Connectivity
```bash
# Test TCP connection
nc -zv localhost 8080

# Test with timeout
timeout 5 nc -zv localhost 8080

# Send test message
echo "test message" | nc localhost 8080

# Performance test
iperf3 -c localhost -p 8080 -t 10
```

### Thread Debugging

#### Thread Analysis
```bash
# Thread count and states
ps -eLf | grep messaging

# Thread CPU usage
top -H -p $(pgrep messaging)

# Thread stacks
pstack $(pgrep messaging)
```

#### Detect Deadlocks
```bash
# Using GDB
gdb -p $(pgrep messaging)
(gdb) info threads
(gdb) thread apply all bt
(gdb) python
>import gdb
>for thread in gdb.inferiors()[0].threads():
>    thread.switch()
>    print(f"Thread {thread.num}: {thread.is_stopped()}")
>end
```

#### ThreadSanitizer
```bash
# Compile with TSAN
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-fsanitize=thread"

# Run with TSAN
TSAN_OPTIONS=halt_on_error=0:history_size=7 ./messaging_server
```

### Tracing and Profiling

#### System Tracing with strace
```bash
# Trace system calls
strace -f -e trace=network ./messaging_server

# Trace file operations
strace -f -e trace=file ./messaging_server

# Count system calls
strace -c ./messaging_server
```

#### Performance Profiling with perf
```bash
# Record CPU profile
sudo perf record -F 99 -g ./messaging_server

# Generate report
sudo perf report

# Real-time monitoring
sudo perf top -p $(pgrep messaging)

# Generate flame graph
sudo perf record -F 99 -g --call-graph dwarf ./messaging_server
sudo perf script | ./FlameGraph/stackcollapse-perf.pl | \
    ./FlameGraph/flamegraph.pl > flame.svg
```

#### Application Tracing
```cpp
// Add trace points in code
class Tracer {
public:
    Tracer(const std::string& name) : name_(name) {
        start_ = std::chrono::high_resolution_clock::now();
        logger_.trace("ENTER: {}", name_);
    }

    ~Tracer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start_).count();
        logger_.trace("EXIT: {} ({}µs)", name_, duration);
    }

private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
};

// Usage
void process_message(const Message& msg) {
    Tracer trace(__FUNCTION__);
    // Function implementation
}
```

## Performance Optimization

### CPU Optimization

#### Identify CPU Bottlenecks
```bash
# CPU profiling
perf stat -d ./messaging_server

# Cache misses
perf stat -e cache-misses,cache-references ./messaging_server

# Branch prediction
perf stat -e branch-misses,branches ./messaging_server
```

#### Optimization Techniques
```cpp
// 1. Use likely/unlikely hints
if (LIKELY(msg.type() == MessageType::NORMAL)) {
    process_normal(msg);
} else {
    process_special(msg);
}

// 2. Align data structures
struct alignas(64) CacheLinePadded {
    std::atomic<uint64_t> counter;
    char padding[56];  // Avoid false sharing
};

// 3. Prefetch data
__builtin_prefetch(&data[i + 1], 0, 3);

// 4. Use SIMD instructions
void process_batch(float* data, size_t count) {
    size_t simd_count = count & ~3;  // Process 4 at a time
    for (size_t i = 0; i < simd_count; i += 4) {
        __m128 vec = _mm_load_ps(&data[i]);
        vec = _mm_mul_ps(vec, _mm_set1_ps(2.0f));
        _mm_store_ps(&data[i], vec);
    }
    // Handle remaining elements
    for (size_t i = simd_count; i < count; ++i) {
        data[i] *= 2.0f;
    }
}
```

### Memory Optimization

#### Memory Usage Analysis
```bash
# Memory map
pmap -x $(pgrep messaging)

# Memory statistics
cat /proc/$(pgrep messaging)/status | grep -E "Vm|Rss"

# Heap profile
gprof ./messaging_server gmon.out
```

#### Optimization Techniques
```cpp
// 1. Use memory pools
template<typename T>
class MemoryPool {
    std::vector<std::unique_ptr<T[]>> blocks_;
    std::stack<T*> free_list_;
    size_t block_size_;

public:
    T* allocate() {
        if (free_list_.empty()) {
            allocate_block();
        }
        T* ptr = free_list_.top();
        free_list_.pop();
        return ptr;
    }

    void deallocate(T* ptr) {
        ptr->~T();
        free_list_.push(ptr);
    }
};

// 2. Reduce allocations
class MessageBuffer {
    static thread_local std::vector<char> buffer_;
public:
    static std::vector<char>& get() {
        buffer_.clear();
        return buffer_;
    }
};

// 3. Use string_view for read-only strings
void process(std::string_view data) {
    // No allocation, just reference
}
```

### I/O Optimization

#### Disk I/O Analysis
```bash
# I/O statistics
iostat -x 1

# I/O by process
iotop -p $(pgrep messaging)

# File system cache
free -h
```

#### Optimization Techniques
```cpp
// 1. Async I/O
boost::asio::async_write(socket, buffer,
    [](error_code ec, size_t bytes) {
        // Handle completion
    });

// 2. Buffered I/O
class BufferedWriter {
    std::vector<char> buffer_;
    size_t pos_ = 0;
    static constexpr size_t BUFFER_SIZE = 65536;

public:
    void write(const void* data, size_t size) {
        if (pos_ + size > BUFFER_SIZE) {
            flush();
        }
        memcpy(&buffer_[pos_], data, size);
        pos_ += size;
    }

    void flush() {
        if (pos_ > 0) {
            ::write(fd_, buffer_.data(), pos_);
            pos_ = 0;
        }
    }
};

// 3. Memory-mapped files
class MappedFile {
    void* addr_;
    size_t size_;

public:
    MappedFile(const std::string& path) {
        int fd = open(path.c_str(), O_RDONLY);
        struct stat st;
        fstat(fd, &st);
        size_ = st.st_size;
        addr_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
    }

    ~MappedFile() {
        munmap(addr_, size_);
    }
};
```

### Network Optimization

#### Network Analysis
```bash
# TCP tuning parameters
sysctl net.ipv4.tcp_wmem
sysctl net.ipv4.tcp_rmem
sysctl net.core.netdev_max_backlog

# Network statistics
netstat -s | grep -i tcp
```

#### Optimization Techniques
```cpp
// 1. TCP_NODELAY for low latency
int flag = 1;
setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// 2. Batch messages
class MessageBatcher {
    std::vector<Message> batch_;
    size_t max_batch_size_ = 100;
    std::chrono::milliseconds max_wait_{10};

public:
    void add(Message msg) {
        batch_.push_back(std::move(msg));
        if (batch_.size() >= max_batch_size_) {
            send_batch();
        }
    }

    void send_batch() {
        if (!batch_.empty()) {
            network_.send_batch(batch_);
            batch_.clear();
        }
    }
};

// 3. Connection pooling
class ConnectionPool {
    std::queue<std::unique_ptr<Connection>> idle_;
    std::mutex mutex_;

public:
    std::unique_ptr<Connection> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (idle_.empty()) {
            return create_connection();
        }
        auto conn = std::move(idle_.front());
        idle_.pop();
        return conn;
    }

    void release(std::unique_ptr<Connection> conn) {
        if (conn->is_healthy()) {
            std::lock_guard<std::mutex> lock(mutex_);
            idle_.push(std::move(conn));
        }
    }
};
```

## Known Issues

### Issue 1: High CPU Usage with Small Messages

**Symptoms**: CPU usage near 100% when processing many small messages.

**Cause**: Overhead of processing individual messages exceeds payload processing time.

**Workaround**:
```ini
[performance]
enable_batching = true
batch_size = 1000
batch_timeout_ms = 10
```

**Fix**: Implement message coalescing:
```cpp
class MessageCoalescer {
    std::vector<Message> buffer_;
    std::chrono::steady_clock::time_point last_flush_;

public:
    void add(Message msg) {
        buffer_.push_back(std::move(msg));

        auto now = std::chrono::steady_clock::now();
        if (buffer_.size() >= 1000 ||
            now - last_flush_ > std::chrono::milliseconds(10)) {
            flush();
        }
    }

    void flush() {
        if (!buffer_.empty()) {
            process_batch(buffer_);
            buffer_.clear();
            last_flush_ = std::chrono::steady_clock::now();
        }
    }
};
```

### Issue 2: Memory Leak in Long-Running Connections

**Symptoms**: Memory usage grows over time with persistent connections.

**Cause**: Connection buffers not properly cleared on partial reads.

**Workaround**: Restart service periodically or limit connection lifetime.

**Fix**:
```cpp
class Connection {
    void on_read(size_t bytes_transferred) {
        // Process data
        process_buffer(read_buffer_, bytes_transferred);

        // Clear processed data
        read_buffer_.consume(bytes_transferred);

        // Shrink buffer if too large
        if (read_buffer_.capacity() > MAX_BUFFER_SIZE &&
            read_buffer_.size() < MAX_BUFFER_SIZE / 4) {
            read_buffer_.shrink_to_fit();
        }
    }
};
```

### Issue 3: Database Connection Pool Exhaustion

**Symptoms**: "Connection pool exhausted" errors under load.

**Cause**: Connections not returned to pool due to exceptions.

**Workaround**: Increase pool size and add connection timeout.

**Fix**: Use RAII for connection management:
```cpp
class PooledConnection {
    ConnectionPool* pool_;
    std::unique_ptr<Connection> conn_;

public:
    PooledConnection(ConnectionPool* pool)
        : pool_(pool), conn_(pool->acquire()) {}

    ~PooledConnection() {
        if (conn_ && pool_) {
            pool_->release(std::move(conn_));
        }
    }

    Connection* operator->() { return conn_.get(); }
};

// Usage
{
    PooledConnection conn(&pool);
    conn->execute("SELECT * FROM messages");
}  // Connection automatically returned
```

### Issue 4: Deadlock in Message Processing

**Symptoms**: System hangs, threads waiting on locks.

**Cause**: Lock ordering violation between queue and handler locks.

**Workaround**: Increase lock timeout and add deadlock detection.

**Fix**: Use lock-free queue or consistent lock ordering:
```cpp
// Lock ordering: always acquire in this order
// 1. queue_mutex_
// 2. handler_mutex_
// 3. state_mutex_

class MessageProcessor {
    void process() {
        Message msg;

        // Acquire and release queue lock
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (queue_.empty()) return;
            msg = queue_.front();
            queue_.pop();
        }

        // Process without holding queue lock
        {
            std::lock_guard<std::mutex> lock(handler_mutex_);
            handler_->process(msg);
        }
    }
};
```

### Issue 5: Performance Degradation with Many Topics

**Symptoms**: Slow message routing with 1000+ topics.

**Cause**: Linear search through topic list for pattern matching.

**Workaround**: Limit number of topics or use exact matching.

**Fix**: Use trie for efficient topic matching:
```cpp
class TopicTrie {
    struct Node {
        std::unordered_map<std::string, std::unique_ptr<Node>> children;
        std::vector<Handler> handlers;
    };

    std::unique_ptr<Node> root_ = std::make_unique<Node>();

public:
    void subscribe(const std::string& topic, Handler handler) {
        auto parts = split(topic, '.');
        Node* node = root_.get();

        for (const auto& part : parts) {
            if (!node->children[part]) {
                node->children[part] = std::make_unique<Node>();
            }
            node = node->children[part].get();
        }

        node->handlers.push_back(handler);
    }

    std::vector<Handler> match(const std::string& topic) {
        // Efficient O(n) matching where n is topic length
    }
};
```

## Error Messages

### Connection Errors

#### "Connection refused"
```
Error: Connection refused to localhost:8080
```

**Causes**:
- Service not running
- Wrong port number
- Firewall blocking connection

**Solutions**:
```bash
# Check if service is running
systemctl status messaging-system

# Check if port is listening
netstat -tlnp | grep 8080

# Check firewall
sudo iptables -L -n | grep 8080
```

#### "Connection timeout"
```
Error: Connection timeout after 5000ms
```

**Causes**:
- Network issues
- Server overloaded
- Timeout too short

**Solutions**:
```ini
[network]
connection_timeout_ms = 30000
keepalive_enabled = true
keepalive_interval_s = 30
```

### Database Errors

#### "Database connection failed"
```
Error: Failed to connect to database: FATAL: password authentication failed
```

**Solutions**:
```bash
# Check PostgreSQL is running
systemctl status postgresql

# Test connection
psql -h localhost -U messaging_user -d messaging

# Check pg_hba.conf
sudo vi /etc/postgresql/14/main/pg_hba.conf
```

#### "Deadlock detected"
```
Error: Database deadlock detected, transaction rolled back
```

**Solutions**:
```cpp
// Retry with exponential backoff
for (int retry = 0; retry < 3; ++retry) {
    try {
        db.execute_transaction([]() {
            // Transaction code
        });
        break;
    } catch (const deadlock_exception& e) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100 * (1 << retry)));
    }
}
```

### Memory Errors

#### "Out of memory"
```
Error: Failed to allocate 1048576 bytes
```

**Solutions**:
```bash
# Check memory usage
free -h
ps aux --sort=-rss | head

# Increase limits
ulimit -v unlimited

# Adjust configuration
echo "vm.overcommit_memory = 1" >> /etc/sysctl.conf
sysctl -p
```

#### "Segmentation fault"
```
Segmentation fault (core dumped)
```

**Debugging**:
```bash
# Enable core dumps
ulimit -c unlimited

# Analyze core dump
gdb ./messaging_server core
(gdb) bt
(gdb) info registers
(gdb) x/10x $rsp
```

## Common Problems

### Problem: Service Won't Start

**Symptoms**: Service fails to start, exits immediately.

**Diagnosis**:
```bash
# Check logs
journalctl -u messaging-system -n 50

# Run manually
/usr/local/bin/messaging_server --debug

# Validate configuration
/usr/local/bin/messaging_server --validate-config
```

**Common Causes**:
1. Port already in use
2. Invalid configuration
3. Missing dependencies
4. Permission issues

**Solutions**:
```bash
# Kill process using port
sudo fuser -k 8080/tcp

# Fix permissions
sudo chown -R messaging:messaging /var/lib/messaging
sudo chmod 755 /var/lib/messaging

# Install missing dependencies
ldd /usr/local/bin/messaging_server | grep "not found"
```

### Problem: Slow Message Processing

**Symptoms**: High latency, messages queuing up.

**Diagnosis**:
```bash
# Check queue size
curl http://localhost:9090/metrics | grep queue_size

# Monitor processing rate
watch -n 1 'curl -s http://localhost:9090/metrics | grep processed'

# Profile CPU usage
perf top -p $(pgrep messaging)
```

**Solutions**:
```ini
[performance]
worker_threads = 16              # Increase workers
queue_size = 100000             # Larger queue
batch_processing = true         # Enable batching
batch_size = 100
```

### Problem: Connection Drops

**Symptoms**: Clients disconnecting frequently.

**Diagnosis**:
```bash
# Check network errors
netstat -s | grep -i error

# Monitor connections
watch -n 1 'ss -tan | grep :8080 | wc -l'

# Check logs for errors
grep -i "connection.*error" /var/log/messaging/messaging.log
```

**Solutions**:
```ini
[network]
keepalive_enabled = true
keepalive_interval_s = 30
keepalive_probes = 3
connection_timeout_ms = 60000
```

### Problem: High Memory Usage

**Symptoms**: Memory usage continuously growing.

**Diagnosis**:
```bash
# Monitor memory growth
while true; do
    ps aux | grep messaging | grep -v grep
    sleep 10
done

# Check for leaks
valgrind --leak-check=full ./messaging_server
```

**Solutions**:
```cpp
// Enable memory limits
class MemoryManager {
    static constexpr size_t MAX_MEMORY = 4UL * 1024 * 1024 * 1024;  // 4GB

    void check_memory() {
        if (get_current_usage() > MAX_MEMORY) {
            trigger_cleanup();
        }
    }
};
```

### Problem: Database Bottleneck

**Symptoms**: Database queries slow, connection pool exhausted.

**Diagnosis**:
```sql
-- Check slow queries
SELECT query, calls, mean_exec_time
FROM pg_stat_statements
ORDER BY mean_exec_time DESC
LIMIT 10;

-- Check connections
SELECT count(*) FROM pg_stat_activity;

-- Check locks
SELECT * FROM pg_locks WHERE granted = false;
```

**Solutions**:
1. Add indexes
2. Increase connection pool
3. Enable query caching
4. Use read replicas

```sql
-- Add indexes
CREATE INDEX idx_messages_timestamp ON messages(timestamp);
CREATE INDEX idx_messages_topic ON messages(topic);

-- Analyze tables
ANALYZE messages;
```