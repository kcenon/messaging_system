#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace logger_module {

/**
 * @class log_server
 * @brief Server that receives logs from network writers
 */
class log_server {
public:
    /**
     * @brief Log entry received from network
     */
    struct network_log_entry {
        std::string source_address;
        uint16_t source_port;
        std::chrono::system_clock::time_point received_time;
        std::string raw_data;
        std::unordered_map<std::string, std::string> parsed_fields;
    };
    
    /**
     * @brief Callback for processing received logs
     */
    using log_handler = std::function<void(const network_log_entry&)>;
    
    /**
     * @brief Constructor
     * @param port Port to listen on
     * @param use_tcp Use TCP (true) or UDP (false)
     * @param max_connections Maximum concurrent TCP connections
     */
    log_server(uint16_t port, bool use_tcp = true, size_t max_connections = 100);
    
    /**
     * @brief Destructor
     */
    ~log_server();
    
    /**
     * @brief Start the server
     * @return true if started successfully
     */
    bool start();
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     */
    bool is_running() const { return running_.load(); }
    
    /**
     * @brief Add log handler
     * @param handler Function to process received logs
     */
    void add_handler(log_handler handler);
    
    /**
     * @brief Get server statistics
     */
    struct server_stats {
        uint64_t total_logs_received;
        uint64_t total_bytes_received;
        uint64_t parse_errors;
        uint64_t active_connections;
        std::chrono::system_clock::time_point server_start_time;
        std::unordered_map<std::string, uint64_t> logs_per_source;
    };
    
    server_stats get_stats() const;
    
private:
    // TCP handling
    void tcp_accept_thread();
    void tcp_client_thread(int client_fd, const std::string& client_addr);
    
    // UDP handling
    void udp_receive_thread();
    
    // Parse received log data
    bool parse_log_entry(const std::string& data, network_log_entry& entry);
    
    // Process log entry
    void process_log(const network_log_entry& entry);
    
private:
    uint16_t port_;
    bool use_tcp_;
    size_t max_connections_;
    
    // Socket
    int server_socket_;
    std::atomic<bool> running_{false};
    
    // Threads
    std::thread accept_thread_;
    std::vector<std::thread> client_threads_;
    std::mutex threads_mutex_;
    
    // Handlers
    std::vector<log_handler> handlers_;
    mutable std::mutex handlers_mutex_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    server_stats stats_{};
};

} // namespace logger_module