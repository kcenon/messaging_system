/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "network_writer.h"

// Platform-specific includes
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace logger_module {

network_writer::network_writer(const std::string& host,
                               uint16_t port,
                               protocol_type protocol,
                               size_t buffer_size,
                               std::chrono::seconds reconnect_interval)
    : host_(host)
    , port_(port)
    , protocol_(protocol)
    , buffer_size_(buffer_size)
    , reconnect_interval_(reconnect_interval)
    , socket_fd_(-1) {

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return;
    }
#endif

    running_ = true;
    
    // Start worker thread
    worker_thread_ = std::thread(&network_writer::worker_thread, this);
    
    // Start reconnect thread for TCP
    if (protocol_ == protocol_type::tcp) {
        reconnect_thread_ = std::thread(&network_writer::reconnect_thread, this);
    }
    
    // Initial connection attempt
    connect();
}

network_writer::~network_writer() {
    running_ = false;
    buffer_cv_.notify_all();

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }

    disconnect();

#ifdef _WIN32
    // Cleanup Winsock
    WSACleanup();
#endif
}

bool network_writer::write(thread_module::log_level level,
                          const std::string& message,
                          const std::string& file,
                          int line,
                          const std::string& function,
                          const std::chrono::system_clock::time_point& timestamp) {
    
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    // Check buffer size
    if (buffer_.size() >= buffer_size_) {
        // Drop oldest message
        buffer_.pop();
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.send_failures++;
    }
    
    buffer_.push({level, message, file, line, function, timestamp});
    buffer_cv_.notify_one();
    
    return true;
}

void network_writer::flush() {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    while (!buffer_.empty()) {
        buffer_cv_.wait(lock, [this] { return buffer_.empty() || !running_; });
    }
}

network_writer::connection_stats network_writer::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

bool network_writer::connect() {
    if (connected_) {
        return true;
    }
    
    // Create socket
    int sock_type = (protocol_ == protocol_type::tcp) ? SOCK_STREAM : SOCK_DGRAM;
    socket_fd_ = socket(AF_INET, sock_type, 0);
#ifdef _WIN32
    if (socket_fd_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }
#else
    if (socket_fd_ < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
#endif
    
    // Resolve hostname
    struct hostent* server = gethostbyname(host_.c_str());
    if (!server) {
        std::cerr << "Failed to resolve host: " << host_ << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }
    
    // Setup server address
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    // Connect (TCP only)
    if (protocol_ == protocol_type::tcp) {
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
            std::cerr << "Failed to connect to " << host_ << ":" << port_
                     << " - Error: " << WSAGetLastError() << std::endl;
#else
            std::cerr << "Failed to connect to " << host_ << ":" << port_
                     << " - " << strerror(errno) << std::endl;
#endif
            close(socket_fd_);
            socket_fd_ = -1;
            
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.connection_failures++;
            stats_.last_error = std::chrono::system_clock::now();
            return false;
        }
    } else {
        // For UDP, just save the server address
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
            std::cerr << "Failed to set UDP destination: Error " << WSAGetLastError() << std::endl;
#else
            std::cerr << "Failed to set UDP destination: " << strerror(errno) << std::endl;
#endif
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }
    }
    
    connected_ = true;
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.last_connected = std::chrono::system_clock::now();
    
    std::cout << "Connected to " << host_ << ":" << port_ 
              << " via " << (protocol_ == protocol_type::tcp ? "TCP" : "UDP") << std::endl;
    
    return true;
}

void network_writer::disconnect() {
#ifdef _WIN32
    if (socket_fd_ != INVALID_SOCKET) {
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
    }
#else
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
#endif
    connected_ = false;
}

bool network_writer::send_data(const std::string& data) {
#ifdef _WIN32
    if (!connected_ || socket_fd_ == INVALID_SOCKET) {
        return false;
    }
#else
    if (!connected_ || socket_fd_ < 0) {
        return false;
    }
#endif
    
#ifdef _WIN32
    int sent = ::send(socket_fd_, data.c_str(), static_cast<int>(data.length()), 0);
    if (sent == SOCKET_ERROR) {
        if (protocol_ == protocol_type::tcp) {
            // TCP connection lost
            std::cerr << "Send failed: Error " << WSAGetLastError() << std::endl;
            disconnect();
#else
    ssize_t sent = ::send(socket_fd_, data.c_str(), data.length(), 0);
    if (sent < 0) {
        if (protocol_ == protocol_type::tcp) {
            // TCP connection lost
            std::cerr << "Send failed: " << strerror(errno) << std::endl;
            disconnect();
#endif
            
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.send_failures++;
            stats_.last_error = std::chrono::system_clock::now();
        }
        return false;
    }
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.messages_sent++;
    stats_.bytes_sent += sent;
    
    return true;
}

void network_writer::worker_thread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(buffer_mutex_);
        
        // Wait for logs or shutdown
        buffer_cv_.wait(lock, [this] {
            return !buffer_.empty() || !running_;
        });
        
        // Process buffered logs
        while (!buffer_.empty() && running_) {
            auto log = std::move(buffer_.front());
            buffer_.pop();
            lock.unlock();
            
            // Format and send
            std::string formatted = format_for_network(log);
            send_data(formatted);
            
            lock.lock();
        }
    }
}

void network_writer::reconnect_thread() {
    while (running_) {
        std::this_thread::sleep_for(reconnect_interval_);
        
        if (!connected_ && running_) {
            std::cout << "Attempting to reconnect to " << host_ << ":" << port_ << std::endl;
            connect();
        }
    }
}

std::string network_writer::format_for_network(const buffered_log& log) {
    // Format as JSON for network transmission
    std::ostringstream oss;
    oss << "{";
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(log.timestamp);
    oss << "\"@timestamp\":\"";
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << "\",";
    
    // Level
    oss << "\"level\":\"" << level_to_string(log.level) << "\",";
    
    // Message
    oss << "\"message\":\"" << escape_json(log.message) << "\"";
    
    // Optional fields
    if (!log.file.empty()) {
        oss << ",\"file\":\"" << escape_json(log.file) << "\"";
        oss << ",\"line\":" << log.line;
    }
    
    if (!log.function.empty()) {
        oss << ",\"function\":\"" << escape_json(log.function) << "\"";
    }
    
    // Add hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        oss << ",\"host\":\"" << hostname << "\"";
    }
    
    oss << "}\n";
    return oss.str();
}

std::string network_writer::escape_json(const std::string& str) const {
    std::string escaped;
    for (char c : str) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }
    return escaped;
}

} // namespace logger_module