/**
 * @file mock_writer.hpp
 * @brief Mock writer implementation for testing
 * @date 2025-09-09
 *
 * BSD 3-Clause License
 * Copyright (c) 2025, kcenon
 * All rights reserved.
 */

#pragma once

#include "../../sources/logger/writers/base_writer.h"
#include "../../sources/logger/interfaces/log_entry.h"
#include "../../sources/logger/error_codes.h"
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

namespace logger_system::testing {

using namespace logger_module;

/**
 * @brief Mock writer for unit testing
 * 
 * Provides controllable behavior for testing various scenarios
 * including success, failure, delays, and tracking write operations.
 */
class mock_writer : public base_writer {
public:
    struct write_record {
        thread_module::log_level level;
        std::string message;
        std::optional<source_location> location;
        std::chrono::system_clock::time_point log_timestamp;
        std::chrono::steady_clock::time_point write_timestamp;
        
        write_record(log_entry&& entry, std::chrono::steady_clock::time_point write_ts)
            : level(entry.level)
            , message(entry.message.to_string())
            , location(std::move(entry.location))
            , log_timestamp(entry.timestamp)
            , write_timestamp(write_ts) {}
    };

private:
    mutable std::mutex mutex_;
    std::vector<write_record> written_entries_;
    std::atomic<size_t> write_count_{0};
    std::atomic<size_t> flush_count_{0};
    std::atomic<bool> should_fail_{false};
    std::atomic<bool> is_open_{true};
    std::chrono::milliseconds write_delay_{0};
    logger_error_code failure_error_{logger_error_code::file_write_failed};

public:
    mock_writer() = default;
    ~mock_writer() override = default;

    // base_writer interface implementation
    result_void write(thread_module::log_level level,
                      const std::string& message,
                      const std::string& file,
                      int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) override {
        if (should_fail_.load()) {
            return make_logger_error(failure_error_);
        }

        if (!is_open_.load()) {
            return make_logger_error(logger_error_code::writer_not_healthy);
        }

        if (write_delay_.count() > 0) {
            std::this_thread::sleep_for(write_delay_);
        }

        // Create log_entry from parameters
        log_entry entry(level, message, file, line, function, timestamp);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            written_entries_.emplace_back(std::move(entry), std::chrono::steady_clock::now());
        }
        
        write_count_.fetch_add(1);
        return {};
    }
    
    // Legacy write method for compatibility
    result_void write(const log_entry& entry) {
        if (should_fail_.load()) {
            return make_logger_error(failure_error_);
        }

        if (!is_open_.load()) {
            return make_logger_error(logger_error_code::writer_not_healthy);
        }

        if (write_delay_.count() > 0) {
            std::this_thread::sleep_for(write_delay_);
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            // Create a copy of entry data since we can't move from const reference
            log_entry copy(entry.level, entry.message.to_string(), entry.timestamp);
            if (entry.location) {
                copy.location = entry.location;
            }
            written_entries_.emplace_back(std::move(copy), std::chrono::steady_clock::now());
        }
        
        write_count_.fetch_add(1);
        return {};
    }

    result_void flush() override {
        if (should_fail_.load()) {
            return make_logger_error(failure_error_);
        }

        flush_count_.fetch_add(1);
        return {};
    }

    result_void open() {
        is_open_.store(true);
        return {};
    }

    result_void close() {
        is_open_.store(false);
        return {};
    }

    bool is_thread_safe() const {
        return true;
    }
    
    std::string get_name() const override {
        return "mock_writer";
    }

    // Mock control methods
    void set_should_fail(bool fail, logger_error_code error = logger_error_code::file_write_failed) {
        should_fail_.store(fail);
        failure_error_ = error;
    }

    void set_write_delay(std::chrono::milliseconds delay) {
        write_delay_ = delay;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        written_entries_.clear();
        write_count_.store(0);
        flush_count_.store(0);
        should_fail_.store(false);
        is_open_.store(true);
        write_delay_ = std::chrono::milliseconds(0);
    }

    // Inspection methods
    size_t get_write_count() const {
        return write_count_.load();
    }

    size_t get_flush_count() const {
        return flush_count_.load();
    }

    std::vector<write_record> get_written_entries() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return written_entries_;
    }

    bool has_entry_with_message(const std::string& message) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::any_of(written_entries_.begin(), written_entries_.end(),
            [&message](const write_record& record) {
                return record.message == message;
            });
    }

    write_record get_last_entry() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (written_entries_.empty()) {
            throw std::runtime_error("No entries written");
        }
        return written_entries_.back();
    }
};

/**
 * @brief Factory for creating mock writers
 */
class mock_writer_factory {
private:
    std::vector<std::shared_ptr<mock_writer>> created_writers_;

public:
    std::shared_ptr<mock_writer> create_writer() {
        auto writer = std::make_shared<mock_writer>();
        created_writers_.push_back(writer);
        return writer;
    }

    std::vector<std::shared_ptr<mock_writer>> get_all_writers() const {
        return created_writers_;
    }

    void reset_all() {
        for (auto& writer : created_writers_) {
            writer->reset();
        }
    }

    size_t get_total_write_count() const {
        size_t total = 0;
        for (const auto& writer : created_writers_) {
            total += writer->get_write_count();
        }
        return total;
    }
};

} // namespace logger_system::testing