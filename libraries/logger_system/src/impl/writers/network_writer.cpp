/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/writers/network_writer.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    typedef SSIZE_T ssize_t;  // Define ssize_t for Windows
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#include <cstring>
#include <cerrno>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace kcenon::logger {

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
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
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

result_void network_writer::write(logger_system::log_level level,
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
        // Note: We still accept the new message after dropping the oldest
    }
    
    buffer_.push({level, message, file, line, function, timestamp});
    buffer_cv_.notify_one();
    
    return {}; // Success
}

result_void network_writer::flush() {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    auto start = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds(5); // 5 second timeout
    
    while (!buffer_.empty()) {
        if (buffer_cv_.wait_for(lock, timeout, [this] { return buffer_.empty() || !running_; })) {
            if (!buffer_.empty() && !running_) {
                return make_logger_error(logger_error_code::flush_timeout,
                                        "Network writer stopped before flush completed");
            }
        } else {
            return make_logger_error(logger_error_code::flush_timeout,
                                    "Network flush timeout");
        }
        
        if (std::chrono::steady_clock::now() - start > timeout) {
            return make_logger_error(logger_error_code::flush_timeout,
                                    "Network flush exceeded timeout");
        }
    }
    return {}; // Success
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
    if (socket_fd_ < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
    
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
            std::cerr << "Failed to connect to " << host_ << ":" << port_ 
                     << " - " << strerror(errno) << std::endl;
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
            std::cerr << "Failed to set UDP destination: " << strerror(errno) << std::endl;
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
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
}

bool network_writer::send_data(const std::string& data) {
    if (!connected_ || socket_fd_ < 0) {
        return false;
    }
    
#ifdef _WIN32
    int sent = ::send(socket_fd_, data.c_str(), static_cast<int>(data.length()), 0);
#else
    ssize_t sent = ::send(socket_fd_, data.c_str(), data.length(), 0);
#endif
    if (sent < 0) {
        if (protocol_ == protocol_type::tcp) {
            // TCP connection lost
            std::cerr << "Send failed: " << strerror(errno) << std::endl;
            disconnect();
            
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

} // namespace kcenon::logger