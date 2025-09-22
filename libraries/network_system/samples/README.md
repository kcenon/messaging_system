# Network System Samples

This directory contains example programs demonstrating the Network System's networking capabilities including TCP/UDP communications and HTTP client functionality.

## Available Samples

### 1. Basic Usage (`basic_usage.cpp`)
Demonstrates fundamental network operations:
- Network manager setup and configuration
- TCP server and client operations
- UDP communication
- HTTP client requests (GET/POST)
- Binary data transmission
- Network utilities and diagnostics
- Error handling and connection management

**Usage:**
```bash
./basic_usage
```

### 2. TCP Server/Client Demo (`tcp_server_client.cpp`)
Advanced TCP communication patterns:
- Multi-client TCP server implementation
- Concurrent client connections
- Text and binary data exchange
- Message and connection handlers
- Performance benchmarking
- Thread-safe server operations
- Client load testing

**Usage:**
```bash
./tcp_server_client
```

### 3. HTTP Client Demo (`http_client_demo.cpp`)
Comprehensive HTTP client functionality:
- GET and POST requests with various data types
- Custom headers and authentication
- Form data and JSON payload handling
- File upload and download operations
- Error handling for different HTTP status codes
- Concurrent request processing
- Performance benchmarking

**Usage:**
```bash
./http_client_demo
```

### 4. Run All Samples (`run_all_samples.cpp`)
Utility to run all samples or a specific sample:

**Usage:**
```bash
# Run all samples
./run_all_samples

# Run specific sample
./run_all_samples basic_usage
./run_all_samples tcp_server_client
./run_all_samples http_client_demo

# List available samples
./run_all_samples --list

# Show help
./run_all_samples --help
```

## Building the Samples

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16 or later
- Network System library
- Platform-specific networking libraries (automatically handled)
- Optional: libcurl for enhanced HTTP functionality

### Build Instructions

1. **From the main project directory:**
```bash
mkdir build && cd build
cmake .. -DBUILD_NETWORK_SAMPLES=ON
make
```

2. **Run samples:**
```bash
cd bin
./basic_usage
./tcp_server_client
./http_client_demo
./run_all_samples
```

### Alternative Build (samples only)
```bash
cd samples
mkdir build && cd build
cmake ..
make
```

## Network Configuration

### TCP Server Configuration
The samples use the following default configuration:
- **Address**: `127.0.0.1` (localhost)
- **Port**: `8080` for TCP, `8081` for UDP
- **Buffer Size**: `8192` bytes
- **Timeout**: `5000` ms (5 seconds)

### HTTP Client Configuration
- **User-Agent**: `NetworkSystem/1.0`
- **Timeout**: `30000` ms (30 seconds)
- **Maximum Redirects**: `5`
- **Default Headers**: Accept, Accept-Language, Accept-Encoding

### Customizing Configuration
You can modify network settings in the samples:
```cpp
// TCP configuration
const std::string server_address = "192.168.1.100";
const int server_port = 9090;

// HTTP configuration
http_client->set_timeout(10000);  // 10 seconds
http_client->set_user_agent("MyApp/2.0");
```

## Sample Output Examples

### Basic Usage Output
```
=== Network System - Basic Usage Example ===

1. Network Manager Setup:
Network manager created
Network configuration set

2. TCP Server Operations:
Starting TCP server on 127.0.0.1:8080
✓ TCP server started successfully
Server status: Running

3. TCP Client Operations:
Connecting to TCP server...
✓ TCP client connected successfully
Connection status: Connected

4. Data Transmission:
Sending message: "Hello from TCP client!"
✓ Message sent successfully
✓ Response received: "Echo: Hello from TCP client!"
...
```

### TCP Server/Client Demo Output
```
=== Network System - TCP Server/Client Demo ===

=== TCP Server Demo ===
Starting server on 127.0.0.1:8080
✓ TCP Server started successfully

=== TCP Client 1 Demo ===
[Server] Client connected: client_001
[Client 1] Testing text communication...
[Server] Received from client_001: Hello Server!
[Client 1] Sent: Hello Server!
[Client 1] Received: Echo: Hello Server! (server time: 1640995200)

=== TCP Client 2 Demo ===
[Client 2] Testing binary communication...
[Server] Received binary data from client_002 (4 bytes)
[Client 2] ✓ Binary echo verified
...
```

### HTTP Client Demo Output
```
=== Network System - HTTP Client Demo ===

1. Basic GET Requests:
Testing simple GET request...
✓ GET request successful
Response size: 1247 bytes
Status code: 200
Content-Type: application/json

2. POST Requests:
Testing JSON POST request...
✓ JSON POST successful
Status code: 200

3. Headers and Authentication:
Testing basic authentication...
✓ Basic authentication successful
Status code: 200
...
```

## Understanding the Results

### Performance Metrics
- **Response Time**: Time taken for individual requests
- **Throughput**: Requests per second for concurrent operations
- **Success Rate**: Percentage of successful operations
- **Connection Time**: Time to establish network connections

### TCP Communication
- **Server Capacity**: Number of concurrent client connections
- **Data Integrity**: Verification of transmitted data
- **Connection Stability**: Persistent connection management
- **Binary Protocol**: Custom protocol implementations

### HTTP Operations
- **Status Codes**: Proper handling of 2xx, 4xx, 5xx responses
- **Content Types**: JSON, form data, binary, and text handling
- **Authentication**: Basic auth and custom header support
- **Error Recovery**: Timeout and retry mechanisms

## Advanced Usage

### Custom TCP Protocol
Implement custom message protocols:
```cpp
// Custom message format
struct custom_message {
    uint32_t message_id;
    uint32_t payload_size;
    std::vector<uint8_t> payload;
};

// Serialize and send
auto serialized = serialize_message(custom_msg);
client->send_binary(serialized);
```

### HTTP Client Extensions
Add custom functionality:
```cpp
// Custom request with specific headers
std::map<std::string, std::string> headers = {
    {"Authorization", "Bearer your-token"},
    {"X-API-Version", "2.0"},
    {"Content-Type", "application/json"}
};

auto response = http_client->post(url, json_data, headers);
```

### Concurrent Server Pattern
```cpp
// Multi-threaded server handling
server->set_message_handler([](const std::string& msg, const std::string& client_id) {
    // Process in thread pool
    thread_pool.enqueue([msg, client_id]() {
        return process_client_message(msg, client_id);
    });
});
```

## Troubleshooting

### Common Issues

1. **Port Already in Use**
   ```
   ✗ Failed to start TCP server
   ```
   - Change the port number in the sample
   - Check for other applications using the port
   - Use `netstat -an | grep 8080` to verify port usage

2. **Network Connectivity**
   ```
   ✗ HTTP GET request failed
   ```
   - Check internet connectivity
   - Verify firewall settings
   - Test with simple connectivity tools (ping, curl)

3. **Permission Errors**
   ```
   ✗ Failed to bind to port
   ```
   - Use ports > 1024 for non-root users
   - Run with appropriate permissions
   - Check system network policies

4. **Compilation Errors**
   - Ensure C++20 support is enabled
   - Check that network system library is properly linked
   - Verify platform-specific network libraries are available

### Platform-Specific Considerations

#### Windows
- Requires Winsock2 library (ws2_32.dll)
- Firewall may prompt for network access
- Use Windows-specific network interface enumeration

#### Linux
- Requires pthread library
- May need to install development packages
- Check SELinux policies for network operations

#### macOS
- Built-in network libraries should work
- Check system network preferences
- Firewall configuration in System Preferences

### Performance Optimization

1. **Buffer Sizes**: Adjust based on typical message sizes
2. **Connection Pooling**: Reuse HTTP connections
3. **Concurrent Limits**: Balance thread count with system resources
4. **Timeout Values**: Set appropriate timeouts for your use case

### Testing Network Features

#### Local Testing
```bash
# Test TCP server separately
./tcp_server_client &
telnet localhost 8080

# Test HTTP client with local server
python3 -m http.server 8080 &
./http_client_demo
```

#### Network Testing
```bash
# Test with external services
curl -X GET "https://httpbin.org/get"
nc -l 8080  # Simple TCP server for testing
```

## Security Considerations

### Network Security
- Validate all incoming data
- Implement proper authentication mechanisms
- Use secure protocols (HTTPS, TLS) for sensitive data
- Sanitize user inputs to prevent injection attacks

### Error Handling
- Don't expose sensitive information in error messages
- Implement proper logging without credentials
- Use secure random generators for session tokens
- Handle timeout and retry logic safely

## Extension Points

### Adding New Samples
1. Create a new `.cpp` file in the samples directory
2. Add it to the `SAMPLE_PROGRAMS` list in `CMakeLists.txt`
3. Include it in the `run_all_samples.cpp` samples registry
4. Update this README with the new sample description

### Custom Protocols
The network system supports custom protocol implementations:
- Implement custom serialization/deserialization
- Add protocol-specific error handling
- Create custom message routing logic
- Implement protocol versioning

### Integration Examples
- WebSocket client/server implementation
- REST API client with JSON handling
- Binary protocol for IoT devices
- Proxy and load balancer implementations

## Getting Help

- Check the main project README for detailed build instructions
- Review the API documentation for network system usage
- Examine the sample source code for implementation details
- Test with simple network tools to isolate issues

## License

These samples are provided under the same BSD 3-Clause License as the Network System project.