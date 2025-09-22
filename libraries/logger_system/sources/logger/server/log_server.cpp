/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "log_server.h"

// Platform-specific includes
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #define close closesocket
    #define shutdown(fd, how) shutdown(fd, SD_BOTH)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <cstring>
#include <iostream>
#include <sstream>
#include <regex>

namespace logger_module {

log_server::log_server(uint16_t port, bool use_tcp, size_t max_connections)
    : port_(port)
    , use_tcp_(use_tcp)
    , max_connections_(max_connections)
    , server_socket_(-1) {

#ifdef _WIN32
    // Initialize Winsock
    static bool winsock_initialized = false;
    if (!winsock_initialized) {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            std::cerr << "Failed to initialize Winsock" << std::endl;
        }
        winsock_initialized = true;
    }
#endif
}

log_server::~log_server() {
    stop();
}

bool log_server::start() {
    if (running_) {
        return true;
    }
    
    // Create socket
    server_socket_ = socket(AF_INET, use_tcp_ ? SOCK_STREAM : SOCK_DGRAM, 0);
#ifdef _WIN32
    if (server_socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket: " << WSAGetLastError() << std::endl;
        return false;
    }
#else
    if (server_socket_ < 0) {
        std::cerr << "Failed to create server socket: " << strerror(errno) << std::endl;
        return false;
    }
#endif
    
    // Allow socket reuse
    int reuse = 1;
#ifdef _WIN32
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR,
                  (const char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set socket options: " << WSAGetLastError() << std::endl;
        closesocket(server_socket_);
        return false;
    }
#else
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
        close(server_socket_);
        return false;
    }
#endif
    
    // Bind to port
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
        std::cerr << "Failed to bind to port " << port_ << ": " << WSAGetLastError() << std::endl;
        closesocket(server_socket_);
#else
        std::cerr << "Failed to bind to port " << port_ << ": " << strerror(errno) << std::endl;
        close(server_socket_);
#endif
        return false;
    }
    
    // Listen for TCP
    if (use_tcp_) {
        if (listen(server_socket_, static_cast<int>(max_connections_)) < 0) {
#ifdef _WIN32
            std::cerr << "Failed to listen: " << WSAGetLastError() << std::endl;
            closesocket(server_socket_);
#else
            std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
            close(server_socket_);
#endif
            return false;
        }
    }
    
    running_ = true;
    stats_.server_start_time = std::chrono::system_clock::now();
    
    // Start appropriate thread
    if (use_tcp_) {
        accept_thread_ = std::thread(&log_server::tcp_accept_thread, this);
    } else {
        accept_thread_ = std::thread(&log_server::udp_receive_thread, this);
    }
    
    std::cout << "Log server started on port " << port_ 
              << " (" << (use_tcp_ ? "TCP" : "UDP") << ")" << std::endl;
    
    return true;
}

void log_server::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Close server socket to interrupt accept()
#ifdef _WIN32
    if (server_socket_ != INVALID_SOCKET) {
        shutdown(server_socket_, SD_BOTH);
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET;
    }
#else
    if (server_socket_ >= 0) {
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
#endif
    
    // Wait for accept thread
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    
    // Wait for all client threads
    {
        std::lock_guard<std::mutex> lock(threads_mutex_);
        for (auto& thread : client_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        client_threads_.clear();
    }
    
    std::cout << "Log server stopped" << std::endl;
}

void log_server::add_handler(log_handler handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_.push_back(handler);
}

log_server::server_stats log_server::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void log_server::tcp_accept_thread() {
    while (running_) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
#ifdef _WIN32
        if (client_fd == INVALID_SOCKET) {
            if (running_) {
                std::cerr << "Accept error: " << WSAGetLastError() << std::endl;
            }
            continue;
        }
#else
        if (client_fd < 0) {
            if (running_) {
                std::cerr << "Accept error: " << strerror(errno) << std::endl;
            }
            continue;
        }
#endif
        
        // Get client address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::string client_address = std::string(client_ip) + ":" + 
                                    std::to_string(ntohs(client_addr.sin_port));
        
        std::cout << "New connection from " << client_address << std::endl;
        
        // Start client thread
        {
            std::lock_guard<std::mutex> lock(threads_mutex_);
            
            // Clean up finished threads
            client_threads_.erase(
                std::remove_if(client_threads_.begin(), client_threads_.end(),
                              [](std::thread& t) { return !t.joinable(); }),
                client_threads_.end()
            );
            
            // Add new client thread
            client_threads_.emplace_back(&log_server::tcp_client_thread, this, 
                                       client_fd, client_address);
            
            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.active_connections = client_threads_.size();
        }
    }
}

void log_server::tcp_client_thread(int client_fd, const std::string& client_addr) {
    char buffer[4096];
    std::string incomplete_data;

    while (running_) {
#ifdef _WIN32
        int received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            if (received < 0) {
                std::cerr << "Receive error from " << client_addr
                         << ": " << WSAGetLastError() << std::endl;
            }
            break;
        }
#else
        ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            if (received < 0) {
                std::cerr << "Receive error from " << client_addr
                         << ": " << strerror(errno) << std::endl;
            }
            break;
        }
#endif
        
        buffer[received] = '\0';
        incomplete_data += buffer;
        
        // Process complete lines
        size_t pos;
        while ((pos = incomplete_data.find('\n')) != std::string::npos) {
            std::string line = incomplete_data.substr(0, pos);
            incomplete_data.erase(0, pos + 1);
            
            if (!line.empty()) {
                network_log_entry entry;
                entry.source_address = client_addr;
                entry.source_port = 0;  // Already included in address
                entry.received_time = std::chrono::system_clock::now();
                entry.raw_data = line;
                
                if (parse_log_entry(line, entry)) {
                    process_log(entry);
                }
            }
        }
    }

#ifdef _WIN32
    closesocket(client_fd);
#else
    close(client_fd);
#endif
    std::cout << "Connection closed: " << client_addr << std::endl;
    
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.active_connections--;
    }
}

void log_server::udp_receive_thread() {
    char buffer[65536];  // Max UDP packet size
    
    while (running_) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
#ifdef _WIN32
        int received = recvfrom(server_socket_, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr*)&client_addr, &client_len);
        if (received == SOCKET_ERROR) {
            if (running_) {
                std::cerr << "UDP receive error: " << WSAGetLastError() << std::endl;
            }
            continue;
        }
#else
        ssize_t received = recvfrom(server_socket_, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr*)&client_addr, &client_len);
        if (received < 0) {
            if (running_) {
                std::cerr << "UDP receive error: " << strerror(errno) << std::endl;
            }
            continue;
        }
#endif
        
        buffer[received] = '\0';
        
        // Get client address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        network_log_entry entry;
        entry.source_address = client_ip;
        entry.source_port = ntohs(client_addr.sin_port);
        entry.received_time = std::chrono::system_clock::now();
        entry.raw_data = buffer;
        
        if (parse_log_entry(buffer, entry)) {
            process_log(entry);
        }
    }
}

bool log_server::parse_log_entry(const std::string& data, network_log_entry& entry) {
    // Simple JSON parser for log entries
    static const std::regex field_regex("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");
    static const std::regex number_regex("\"([^\"]+)\"\\s*:\\s*(\\d+)");
    
    // Extract string fields
    auto fields_begin = std::sregex_iterator(data.begin(), data.end(), field_regex);
    auto fields_end = std::sregex_iterator();
    
    for (auto it = fields_begin; it != fields_end; ++it) {
        entry.parsed_fields[(*it)[1]] = (*it)[2];
    }
    
    // Extract number fields
    auto numbers_begin = std::sregex_iterator(data.begin(), data.end(), number_regex);
    auto numbers_end = std::sregex_iterator();
    
    for (auto it = numbers_begin; it != numbers_end; ++it) {
        entry.parsed_fields[(*it)[1]] = (*it)[2];
    }
    
    if (entry.parsed_fields.empty()) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.parse_errors++;
        return false;
    }
    
    return true;
}

void log_server::process_log(const network_log_entry& entry) {
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.total_logs_received++;
        stats_.total_bytes_received += entry.raw_data.size();
        stats_.logs_per_source[entry.source_address]++;
    }
    
    // Call handlers
    std::vector<log_handler> handlers_copy;
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        handlers_copy = handlers_;
    }
    
    for (const auto& handler : handlers_copy) {
        try {
            handler(entry);
        } catch (const std::exception& e) {
            std::cerr << "Handler error: " << e.what() << std::endl;
        }
    }
}

} // namespace logger_module