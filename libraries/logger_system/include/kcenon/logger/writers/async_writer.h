#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file async_writer.h
 * @brief Asynchronous wrapper for log writers
 * 
 * This file provides an asynchronous wrapper that can be used with any
 * base_writer implementation to make it asynchronous.
 */

#include "base_writer.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace kcenon::logger {

/**
 * @class async_writer
 * @brief Asynchronous wrapper for log writers
 * 
 * This class wraps any base_writer implementation and provides
 * asynchronous writing capabilities using a background thread.
 */
class async_writer : public base_writer {
public:
    /**
     * @brief Constructor
     * @param wrapped_writer The writer to wrap with async functionality
     * @param queue_size Maximum queue size for pending messages
     */
    explicit async_writer(std::unique_ptr<base_writer> wrapped_writer, 
                         std::size_t queue_size = 10000)
        : wrapped_writer_(std::move(wrapped_writer))
        , max_queue_size_(queue_size)
        , running_(false) {
        if (!wrapped_writer_) {
            throw std::invalid_argument("Wrapped writer cannot be null");
        }
    }
    
    /**
     * @brief Destructor
     */
    ~async_writer() override {
        stop();
    }
    
    /**
     * @brief Start the async writer thread
     */
    void start() {
        if (running_.exchange(true)) {
            return; // Already running
        }
        
        worker_thread_ = std::thread([this]() {
            process_messages();
        });
    }
    
    /**
     * @brief Stop the async writer thread
     */
    void stop() {
        if (!running_.exchange(false)) {
            return; // Already stopped
        }
        
        // Signal the worker thread to stop
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_cv_.notify_one();
        }
        
        // Wait for the worker thread to finish
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        
        // Process any remaining messages
        flush_remaining();
    }
    
    /**
     * @brief Write a log message asynchronously
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param timestamp Timestamp
     * @return result_void indicating success or error
     */
    result_void write(logger_system::log_level level,
                     const std::string& message,
                     const std::string& file,
                     int line,
                     const std::string& function,
                     const std::chrono::system_clock::time_point& timestamp) override {
        if (!running_) {
            // If not running, write directly
            return wrapped_writer_->write(level, message, file, line, function, timestamp);
        }
        
        // Queue the message
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            
            // Check queue size
            if (message_queue_.size() >= max_queue_size_) {
                // Queue is full, drop the message or handle overflow
                return make_logger_error(logger_error_code::queue_full, "Async writer queue is full");
            }
            
            message_queue_.push({level, message, file, line, function, timestamp});
            queue_cv_.notify_one();
        }
        
        return result_void{};
    }
    
    /**
     * @brief Flush all pending messages
     * @return result_void indicating success or error
     */
    result_void flush() override {
        if (!running_) {
            return wrapped_writer_->flush();
        }
        
        // Wait for the queue to be empty
        std::unique_lock<std::mutex> lock(queue_mutex_);
        flush_cv_.wait(lock, [this]() {
            return message_queue_.empty();
        });
        
        // Flush the wrapped writer
        return wrapped_writer_->flush();
    }
    
    /**
     * @brief Check if the writer is healthy
     * @return true if healthy, false otherwise
     */
    bool is_healthy() const override {
        return wrapped_writer_->is_healthy() && running_;
    }
    
    /**
     * @brief Get the name of this writer
     * @return Writer name
     */
    std::string get_name() const override {
        return "async_" + wrapped_writer_->get_name();
    }
    
    /**
     * @brief Set whether to use color output
     * @param use_color Enable/disable color output
     */
    void set_use_color(bool use_color) override {
        wrapped_writer_->set_use_color(use_color);
    }
    
private:
    /**
     * @brief Message structure for queuing
     */
    struct queued_message {
        logger_system::log_level level;
        std::string message;
        std::string file;
        int line;
        std::string function;
        std::chrono::system_clock::time_point timestamp;
    };
    
    /**
     * @brief Process messages from the queue
     */
    void process_messages() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Wait for messages or stop signal
            queue_cv_.wait(lock, [this]() {
                return !message_queue_.empty() || !running_;
            });
            
            // Process all available messages
            while (!message_queue_.empty()) {
                auto msg = std::move(message_queue_.front());
                message_queue_.pop();
                
                // Unlock while writing
                lock.unlock();
                wrapped_writer_->write(msg.level, msg.message, msg.file, msg.line, msg.function, msg.timestamp);
                lock.lock();
            }
            
            // Notify flush waiters
            flush_cv_.notify_all();
        }
    }
    
    /**
     * @brief Flush any remaining messages after stopping
     */
    void flush_remaining() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!message_queue_.empty()) {
            auto msg = std::move(message_queue_.front());
            message_queue_.pop();
            wrapped_writer_->write(msg.level, msg.message, msg.file, msg.line, msg.function, msg.timestamp);
        }
        wrapped_writer_->flush();
    }
    
    std::unique_ptr<base_writer> wrapped_writer_;
    std::size_t max_queue_size_;
    
    std::queue<queued_message> message_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::condition_variable flush_cv_;
    
    std::atomic<bool> running_;
    std::thread worker_thread_;
};

} // namespace kcenon::logger