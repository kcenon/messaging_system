#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <chrono>
#include <string>
#include <limits>
#include <kcenon/logger/core/error_codes.h>

// Conditional include based on build mode
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    #include <kcenon/thread/interfaces/logger_interface.h>
#else
    #include <kcenon/logger/interfaces/logger_interface.h>
    #include <kcenon/logger/interfaces/logger_types.h>
#endif

namespace kcenon::logger {

/**
 * @struct logger_config
 * @brief Configuration structure for logger with validation
 * 
 * This structure holds all configuration parameters for the logger
 * and provides validation to ensure configuration correctness.
 */
struct logger_config {
    // Basic settings
    bool async = true;
    std::size_t buffer_size = 8192;
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    kcenon::thread::log_level min_level = kcenon::thread::log_level::info;
#else
    logger_system::log_level min_level = logger_system::log_level::info;
#endif
    
    // Performance settings
    std::size_t batch_size = 100;
    std::chrono::milliseconds flush_interval{1000};
    bool use_lock_free = false;
    std::size_t max_writers = 10;
    bool enable_batch_writing = false;
    
    // Feature flags
    bool enable_metrics = false;
    bool enable_crash_handler = false;
    bool enable_structured_logging = false;
    bool enable_color_output = true;
    bool enable_timestamp = true;
    bool enable_source_location = false;
    
    // Queue settings
    std::size_t max_queue_size = 10000;
    enum class overflow_policy {
        drop_oldest,    // Drop oldest messages when queue is full
        drop_newest,    // Drop new messages when queue is full (default)
        block,          // Block until space is available
        grow            // Dynamically grow the queue (use with caution)
    };
    overflow_policy queue_overflow_policy = overflow_policy::drop_newest;
    
    // File output settings
    std::size_t max_file_size = 100 * 1024 * 1024;  // 100MB default
    std::size_t max_file_count = 5;                  // For rotating files
    std::string log_directory = "./logs";
    std::string log_file_prefix = "app";
    
    // Network settings
    std::string remote_host = "";
    uint16_t remote_port = 0;
    std::chrono::milliseconds network_timeout{5000};
    std::size_t network_retry_count = 3;
    
    // Performance tuning
    std::size_t writer_thread_count = 1;
    bool enable_compression = false;
    
    /**
     * @brief Validate the configuration
     * @return result_void indicating success or specific validation error
     */
    result_void validate() const {
        // Validate buffer size
        if (buffer_size == 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Buffer size must be greater than 0");
        }
        
        if (buffer_size > std::numeric_limits<std::size_t>::max() / 2) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Buffer size is too large");
        }
        
        // Validate batch size
        if (batch_size == 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Batch size must be greater than 0");
        }
        
        if (batch_size > buffer_size) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Batch size cannot exceed buffer size");
        }
        
        // Validate flush interval
        if (flush_interval.count() < 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Flush interval must be non-negative");
        }
        
        if (flush_interval.count() > 3600000) {  // 1 hour max
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Flush interval too large (max 1 hour)");
        }
        
        // Validate queue settings
        if (max_queue_size == 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Max queue size must be greater than 0");
        }
        
        if (max_queue_size < batch_size) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Max queue size must be at least as large as batch size");
        }
        
        // Validate file settings
        if (max_file_size < 1024) {  // Minimum 1KB
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Max file size too small (minimum 1KB)");
        }
        
        if (max_file_count == 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Max file count must be greater than 0");
        }
        
        if (max_file_count > 1000) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Max file count too large (max 1000)");
        }
        
        // Validate network settings if configured
        if (!remote_host.empty()) {
            if (remote_port == 0) {
                return make_logger_error(logger_error_code::invalid_configuration,
                                "Remote port must be specified when remote host is set");
            }
            
            if (network_timeout.count() <= 0) {
                return make_logger_error(logger_error_code::invalid_configuration,
                                "Network timeout must be positive");
            }
            
            if (network_retry_count > 100) {
                return make_logger_error(logger_error_code::invalid_configuration,
                                "Network retry count too large (max 100)");
            }
        }
        
        // Validate writer settings
        if (max_writers == 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Must allow at least one writer");
        }
        
        if (max_writers > 100) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Max writers too large (max 100)");
        }
        
        // Validate thread count
        if (writer_thread_count == 0) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Writer thread count must be at least 1");
        }
        
        if (writer_thread_count > 32) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Writer thread count too large (max 32)");
        }
        
        // Validate feature combinations
        if (use_lock_free && queue_overflow_policy == overflow_policy::grow) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Lock-free queue cannot use grow overflow policy");
        }
        
        if (!async && batch_size > 1) {
            return make_logger_error(logger_error_code::invalid_configuration,
                            "Batch processing requires async mode");
        }
        
        return result_void{};
    }
    
    /**
     * @brief Create a default configuration
     * @return Default logger configuration
     */
    static logger_config default_config() {
        return logger_config{};
    }
    
    /**
     * @brief Create a high-performance configuration
     * @return Configuration optimized for performance
     */
    static logger_config high_performance() {
        logger_config config;
        config.async = true;
        config.buffer_size = 65536;
        config.batch_size = 500;
        config.flush_interval = std::chrono::milliseconds(5000);
        config.use_lock_free = true;
        config.max_queue_size = 100000;
        config.writer_thread_count = 2;
        config.enable_compression = true;
        config.enable_batch_writing = true;
        return config;
    }
    
    /**
     * @brief Create a low-latency configuration
     * @return Configuration optimized for low latency
     */
    static logger_config low_latency() {
        logger_config config;
        config.async = true;
        config.buffer_size = 4096;
        config.batch_size = 10;
        config.flush_interval = std::chrono::milliseconds(10);
        config.use_lock_free = true;
        config.max_queue_size = 10000;
        config.queue_overflow_policy = overflow_policy::drop_oldest;
        return config;
    }
    
    /**
     * @brief Create a debug configuration
     * @return Configuration optimized for debugging
     */
    static logger_config debug_config() {
        logger_config config;
        config.async = false;  // Synchronous for immediate output
        config.min_level = kcenon::thread::log_level::trace;
        config.enable_metrics = true;
        config.enable_crash_handler = true;
        config.enable_color_output = true;
        config.batch_size = 1;
        config.flush_interval = std::chrono::milliseconds(0);
        return config;
    }
    
    /**
     * @brief Create a production configuration
     * @return Configuration optimized for production use
     */
    static logger_config production() {
        logger_config config;
        config.async = true;
        config.buffer_size = 16384;
        config.min_level = kcenon::thread::log_level::warning;
        config.enable_metrics = true;
        config.enable_crash_handler = true;
        config.enable_color_output = false;
        config.max_file_size = 500 * 1024 * 1024;  // 500MB
        config.max_file_count = 10;
        config.enable_compression = true;
        config.enable_batch_writing = true;
        config.batch_size = 200;
        return config;
    }
};

} // namespace kcenon::logger