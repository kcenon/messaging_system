#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "base_writer.h"
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <mutex>
#include <cstdint>

namespace kcenon::logger {

/**
 * @class network_writer
 * @brief Sends logs over network (TCP/UDP)
 */
class network_writer : public base_writer {
public:
    enum class protocol_type {
        tcp,
        udp
    };
    
    /**
     * @brief Constructor
     * @param host Remote host address
     * @param port Remote port
     * @param protocol Network protocol (TCP/UDP)
     * @param buffer_size Internal buffer size
     * @param reconnect_interval Reconnection interval in seconds
     */
    network_writer(const std::string& host,
                   uint16_t port,
                   protocol_type protocol = protocol_type::tcp,
                   size_t buffer_size = 8192,
                   std::chrono::seconds reconnect_interval = std::chrono::seconds(5));
    
    /**
     * @brief Destructor
     */
    ~network_writer() override;
    
    /**
     * @brief Write log entry
     */
    result_void write(logger_system::log_level level,
                      const std::string& message,
                      const std::string& file,
                      int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) override;
    
    /**
     * @brief Flush pending logs
     */
    result_void flush() override;
    
    /**
     * @brief Get writer name
     */
    std::string get_name() const override { return "network"; }
    
    /**
     * @brief Check if connected
     */
    bool is_connected() const { return connected_.load(); }
    
    /**
     * @brief Get connection statistics
     */
    struct connection_stats {
        uint64_t messages_sent;
        uint64_t bytes_sent;
        uint64_t connection_failures;
        uint64_t send_failures;
        std::chrono::system_clock::time_point last_connected;
        std::chrono::system_clock::time_point last_error;
    };
    
    connection_stats get_stats() const;
    
private:
    // Log entry for buffering
    struct buffered_log {
        logger_system::log_level level;
        std::string message;
        std::string file;
        int line;
        std::string function;
        std::chrono::system_clock::time_point timestamp;
    };
    
    // Network operations
    bool connect();
    void disconnect();
    bool send_data(const std::string& data);
    void worker_thread();
    void reconnect_thread();
    
    // Format log for network transmission
    std::string format_for_network(const buffered_log& log);
    
private:
    std::string host_;
    uint16_t port_;
    protocol_type protocol_;
    size_t buffer_size_;
    std::chrono::seconds reconnect_interval_;
    
    // Socket handling
    int socket_fd_;
    std::atomic<bool> connected_{false};
    std::atomic<bool> running_{false};
    
    // Buffering
    std::queue<buffered_log> buffer_;
    mutable std::mutex buffer_mutex_;
    std::condition_variable buffer_cv_;
    
    // Worker threads
    std::thread worker_thread_;
    std::thread reconnect_thread_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    connection_stats stats_{};
    
    // Helper functions
    std::string escape_json(const std::string& str) const;
};

} // namespace kcenon::logger