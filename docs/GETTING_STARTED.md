# Getting Started

Welcome to the Messaging System! This guide will help you quickly set up and start using the high-performance C++20 distributed messaging framework.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Basic Usage](#basic-usage)
5. [First Application](#first-application)
6. [Next Steps](#next-steps)

## Prerequisites

### System Requirements

- **Operating System**: Linux, macOS 10.15+, Windows 10+
- **Compiler**: C++20 compatible compiler
  - GCC 10.0+
  - Clang 12.0+
  - MSVC 2019+ (Visual Studio 16.10+)
- **Build System**: CMake 3.16 or later
- **Memory**: Minimum 4GB RAM (8GB+ recommended for development)
- **Disk Space**: 2GB for full build with dependencies

### Development Dependencies

- **Git**: For cloning the repository and submodules
- **vcpkg**: Package manager (included as submodule)
- **PostgreSQL**: 12+ (optional, for database module)
- **Python**: 3.8+ (optional, for Python bindings)

### Runtime Dependencies

The messaging system automatically manages most dependencies through vcpkg:

- **fmt**: String formatting library
- **asio**: Asynchronous I/O library
- **Google Test**: Testing framework (development only)
- **Google Benchmark**: Performance testing (development only)

## Installation

### 1. Clone the Repository

```bash
# Clone the main repository
git clone <repository-url> messaging_system
cd messaging_system

# Initialize submodules (includes thread_system and vcpkg)
git submodule update --init --recursive
```

### 2. Platform-Specific Setup

#### Linux (Ubuntu/Debian)

```bash
# Install basic development tools
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    curl \
    zip \
    unzip \
    tar

# Install PostgreSQL (optional)
sudo apt install -y postgresql postgresql-dev libpq-dev

# Install Python development headers (optional)
sudo apt install -y python3-dev python3-pip
```

#### macOS

```bash
# Install Xcode command line tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake pkg-config

# Install PostgreSQL (optional)
brew install postgresql

# Install Python (optional)
brew install python@3.9
```

#### Windows

1. **Install Visual Studio 2019 or later** with C++ development tools
2. **Install Git for Windows**
3. **Install CMake** (3.16+) from [cmake.org](https://cmake.org/download/)
4. **Install PostgreSQL** (optional) from [postgresql.org](https://www.postgresql.org/download/windows/)

### 3. Build the System

The project includes automated build scripts for all platforms:

#### Quick Build (Recommended)

```bash
# Build with default configuration
./scripts/build.sh

# On Windows (use PowerShell or Git Bash)
./scripts/build.bat
```

#### Custom Build Options

```bash
# Build with specific options
./scripts/build.sh --release --tests --python-bindings

# Available options:
# --debug          : Debug build (default)
# --release        : Release build with optimizations
# --tests          : Include unit tests
# --benchmarks     : Include performance benchmarks
# --python-bindings: Build Python bindings
# --no-database    : Skip database module
# --clean          : Clean before building
```

### 4. Verify Installation

```bash
# Check if build was successful
ls build/bin/

# Run basic tests
cd build/bin
./container_test
./network_test

# Run sample application
./production_ready_example
```

## Quick Start

### 1. Basic Message Bus Usage

Create a simple messaging application:

```cpp
#include <iostream>
#include <kcenon/messaging/core/message_bus.h>

int main() {
    // Initialize message bus
    kcenon::messaging::core::message_bus bus;
    bus.initialize();

    // Subscribe to messages
    bus.subscribe("user.login", [](const auto& message) {
        auto payload = message.get_payload();
        std::cout << "User logged in: "
                  << payload.get<std::string>("username") << std::endl;
        return kcenon::messaging::core::message_status::processed;
    });

    // Publish a message
    kcenon::messaging::core::message_payload payload;
    payload.set("username", "john_doe");
    payload.set("timestamp", std::time(nullptr));

    bus.publish("user.login", payload);

    // Process messages for 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
```

### 2. Container-Based Data Handling

Work with type-safe containers:

```cpp
#include <container/container.h>
using namespace container_module;

int main() {
    // Create a container
    auto container = std::make_shared<value_container>();

    // Set header information
    container->set_source("client_01", "session_123");
    container->set_target("server", "main_handler");
    container->set_message_type("user_data");

    // Add typed values
    auto values = std::vector<std::shared_ptr<value>>{
        value_factory::create("user_id", int64_value, "12345"),
        value_factory::create("username", string_value, "alice"),
        value_factory::create("active", bool_value, "true"),
        value_factory::create("score", double_value, "95.5")
    };
    container->set_values(values);

    // Serialize for storage or transmission
    std::string serialized = container->serialize();
    std::cout << "Serialized size: " << serialized.size() << " bytes" << std::endl;

    // Deserialize from data
    auto restored = std::make_shared<value_container>(serialized);
    std::cout << "Message type: " << restored->message_type() << std::endl;

    return 0;
}
```

### 3. Network Client/Server

Set up basic networking:

```cpp
#include <network/messaging_server.h>
#include <network/messaging_client.h>
using namespace network_module;

int main() {
    // Create and start server
    auto server = std::make_shared<messaging_server>("echo_server");

    server->set_message_handler([](auto session, const std::string& message) {
        std::cout << "Server received: " << message << std::endl;
        session->send_message("Echo: " + message);
    });

    if (!server->start_server(8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    // Create and start client
    auto client = std::make_shared<messaging_client>("test_client");

    client->set_message_handler([](const std::string& message) {
        std::cout << "Client received: " << message << std::endl;
    });

    if (!client->start_client("127.0.0.1", 8080)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    // Send messages
    client->send_message("Hello, Server!");
    client->send_message("How are you?");

    // Wait for responses
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Cleanup
    client->stop_client();
    server->stop_server();

    return 0;
}
```

## Basic Usage

### Project Structure

Create a basic CMake project that uses the messaging system:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(my_messaging_app)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the messaging system
find_package(messaging_system REQUIRED)

# Create your executable
add_executable(my_app main.cpp)

# Link against messaging system libraries
target_link_libraries(my_app
    messaging_system::container
    messaging_system::network
    messaging_system::database
    messaging_system::thread_system
)
```

### Environment Configuration

Set up environment variables for database connections:

```bash
# Database configuration
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=messaging_app
export DB_USER=app_user
export DB_PASSWORD=secure_password

# Network configuration
export MESSAGING_PORT=8080
export MAX_CONNECTIONS=1000

# Performance tuning
export THREAD_POOL_SIZE=8
export MESSAGE_BUFFER_SIZE=1048576
```

### Basic Configuration File

Create a configuration file for your application:

```json
{
  "messaging": {
    "bus_id": "main_bus",
    "max_message_size": 1048576,
    "processing_timeout": 30000
  },
  "network": {
    "server_port": 8080,
    "max_connections": 1000,
    "session_timeout": 300000
  },
  "database": {
    "enabled": true,
    "host": "${DB_HOST}",
    "port": 5432,
    "database": "${DB_NAME}",
    "connection_pool_size": 10
  },
  "logging": {
    "level": "info",
    "file": "messaging_app.log",
    "max_file_size": "100MB"
  }
}
```

## First Application

Let's build a complete chat server application:

### 1. Chat Server (chat_server.cpp)

```cpp
#include <iostream>
#include <unordered_map>
#include <network/messaging_server.h>
#include <container/container.h>

class ChatServer {
private:
    std::shared_ptr<network_module::messaging_server> server_;
    std::unordered_map<std::string, std::shared_ptr<network_module::messaging_session>> clients_;
    std::mutex clients_mutex_;

public:
    ChatServer() : server_(std::make_shared<network_module::messaging_server>("chat_server")) {
        setup_handlers();
    }

    bool start(unsigned short port = 8080) {
        return server_->start_server(port);
    }

    void stop() {
        server_->stop_server();
    }

private:
    void setup_handlers() {
        server_->set_connect_handler([this](auto session) {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_[session->session_id()] = session;

            std::cout << "Client connected: " << session->session_id()
                      << " (Total: " << clients_.size() << ")" << std::endl;

            broadcast_message("System", session->session_id() + " joined the chat");
        });

        server_->set_disconnect_handler([this](auto session, const std::string& reason) {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.erase(session->session_id());

            std::cout << "Client disconnected: " << session->session_id()
                      << " (" << reason << ")" << std::endl;

            broadcast_message("System", session->session_id() + " left the chat");
        });

        server_->set_message_handler([this](auto session, const std::string& message) {
            try {
                // Parse container message
                auto container = std::make_shared<container_module::value_container>(message);

                if (container->message_type() == "chat_message") {
                    auto username_val = container->get_value("username");
                    auto text_val = container->get_value("text");

                    if (username_val && text_val) {
                        std::string username = username_val->to_string();
                        std::string text = text_val->to_string();

                        std::cout << "[" << username << "]: " << text << std::endl;
                        broadcast_message(username, text);
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing message: " << e.what() << std::endl;
            }
        });
    }

    void broadcast_message(const std::string& username, const std::string& text) {
        // Create broadcast container
        auto broadcast = std::make_shared<container_module::value_container>();
        broadcast->set_source("server", "chat");
        broadcast->set_target("all", "clients");
        broadcast->set_message_type("chat_broadcast");

        auto values = std::vector<std::shared_ptr<container_module::value>>{
            container_module::value_factory::create("username",
                container_module::string_value, username),
            container_module::value_factory::create("text",
                container_module::string_value, text),
            container_module::value_factory::create("timestamp",
                container_module::int64_value, std::to_string(std::time(nullptr)))
        };
        broadcast->set_values(values);

        std::string serialized = broadcast->serialize();

        // Send to all connected clients
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto& [session_id, session] : clients_) {
            if (session && session->is_connected()) {
                session->send_message(serialized);
            }
        }
    }
};

int main() {
    ChatServer server;

    std::cout << "Starting chat server..." << std::endl;

    if (server.start(8080)) {
        std::cout << "Chat server running on port 8080" << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();

        server.stop();
    } else {
        std::cerr << "Failed to start chat server" << std::endl;
        return 1;
    }

    return 0;
}
```

### 2. Build and Run

```bash
# Add to CMakeLists.txt
add_executable(chat_server chat_server.cpp)
target_link_libraries(chat_server
    messaging_system::container
    messaging_system::network
)

# Build
cmake --build build --target chat_server

# Run
./build/bin/chat_server
```

### 3. Test with Sample Client

```bash
# Use the provided sample client
./build/bin/chat_client

# Or test with telnet
telnet localhost 8080
```

## Next Steps

### 1. Explore Sample Applications

Check out the comprehensive examples in `application_layer/samples/`:

```bash
# IoT monitoring system
./build/bin/iot_monitoring

# Distributed worker system
./build/bin/distributed_worker

# Microservices orchestrator
./build/bin/microservices_orchestrator
```

### 2. Advanced Features

- **Database Integration**: See [Database System](../libraries/database_system/README.md)
- **Performance Optimization**: Read [Performance Guide](./performance.md)
- **Production Deployment**: Follow [Deployment Guide](./deployment-guide.md)
- **Python Integration**: Explore Python bindings

### 3. Development

- **API Reference**: Complete documentation in [API Reference](./api-reference.md)
- **Architecture**: System design in [Architecture](./architecture.md)
- **Contributing**: Guidelines in [Developer Guide](./developer-guide.md)

### 4. Community

- **GitHub Repository**: Source code, issues, and discussions
- **Documentation**: This comprehensive guide
- **Examples**: Production-ready sample applications
- **Support**: GitHub Issues and Discussions

---

**Ready to build something amazing?** The messaging system provides enterprise-grade performance and reliability for your distributed applications. Start with these examples and explore the comprehensive API to build scalable, high-performance systems.