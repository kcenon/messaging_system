/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "high_performance_async_writer.h"
#include <algorithm>

namespace kcenon::logger::async {

high_performance_async_writer::high_performance_async_writer(
    std::unique_ptr<base_writer> wrapped_writer,
    const config& cfg)
    : config_(cfg)
    , wrapped_writer_(std::move(wrapped_writer)) {

    if (!wrapped_writer_) {
        throw std::invalid_argument("Wrapped writer cannot be null");
    }

    // Initialize memory pool if enabled
    if (config_.enable_memory_pooling) {
        memory_pool_ = std::make_unique<memory::object_pool<memory::log_entry_pool::pooled_log_entry>>(
            config_.pool_config);
    }

    // Initialize batch processor if enabled
    if (config_.enable_batch_processing) {
        batch_processor_ = make_batch_processor(
            std::unique_ptr<base_writer>(wrapped_writer_.release()),
            config_.batch_config);
    }
}

high_performance_async_writer::~high_performance_async_writer() {
    stop(true);
}

bool high_performance_async_writer::start() {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
        return false; // Already running
    }

    if (batch_processor_) {
        return batch_processor_->start();
    }

    return true;
}

void high_performance_async_writer::stop(bool flush_remaining) {
    if (!running_.exchange(false)) {
        return; // Already stopped
    }

    if (batch_processor_) {
        batch_processor_->stop(flush_remaining);
    } else if (flush_remaining && wrapped_writer_) {
        wrapped_writer_->flush();
    }
}

result_void high_performance_async_writer::write(
    logger_system::log_level level,
    const std::string& message,
    const std::string& file,
    int line,
    const std::string& function,
    const std::chrono::system_clock::time_point& timestamp) {

    const auto start_time = std::chrono::steady_clock::now();

    if (!running_.load(std::memory_order_relaxed)) {
        return write_direct(level, message, file, line, function, timestamp);
    }

    stats_.total_writes.fetch_add(1, std::memory_order_relaxed);

    // Use batch processor if available
    if (batch_processor_) {
        batch_processor::batch_entry entry(level, message, file, line, function, timestamp);

        if (batch_processor_->add_entry(std::move(entry))) {
            const auto end_time = std::chrono::steady_clock::now();
            const auto latency = end_time - start_time;
            update_stats(true, latency);
            return result_void{};
        } else {
            stats_.dropped_writes.fetch_add(1, std::memory_order_relaxed);
            // Fall back to direct write
            return write_direct(level, message, file, line, function, timestamp);
        }
    }

    // Direct async write (fallback mode)
    return write_direct(level, message, file, line, function, timestamp);
}

result_void high_performance_async_writer::flush() {
    if (batch_processor_) {
        batch_processor_->flush();
    } else if (wrapped_writer_) {
        return wrapped_writer_->flush();
    }
    return result_void{};
}

bool high_performance_async_writer::is_healthy() const {
    if (!running_.load(std::memory_order_relaxed)) {
        return false;
    }

    if (batch_processor_) {
        return batch_processor_->is_healthy();
    }

    return wrapped_writer_ && wrapped_writer_->is_healthy();
}

std::string high_performance_async_writer::get_name() const {
    const std::string base_name = wrapped_writer_ ? wrapped_writer_->get_name() : "unknown";
    return "high_perf_async_" + base_name;
}

void high_performance_async_writer::set_use_color(bool use_color) {
    if (wrapped_writer_) {
        wrapped_writer_->set_use_color(use_color);
    }
}

double high_performance_async_writer::get_queue_utilization() const {
    if (batch_processor_) {
        const auto queue_size = batch_processor_->get_queue_size();
        const auto max_size = config_.queue_size;
        return static_cast<double>(queue_size) / max_size;
    }
    return 0.0;
}

const batch_processor::processing_stats* high_performance_async_writer::get_batch_stats() const {
    if (batch_processor_) {
        return &batch_processor_->get_stats();
    }
    return nullptr;
}

result_void high_performance_async_writer::write_direct(
    logger_system::log_level level,
    const std::string& message,
    const std::string& file,
    int line,
    const std::string& function,
    const std::chrono::system_clock::time_point& timestamp) {

    if (!wrapped_writer_) {
        return make_logger_error(logger_error_code::writer_not_available, "No wrapped writer available");
    }

    const auto start_time = std::chrono::steady_clock::now();
    auto result = wrapped_writer_->write(level, message, file, line, function, timestamp);
    const auto end_time = std::chrono::steady_clock::now();

    const auto latency = end_time - start_time;
    update_stats(static_cast<bool>(result), latency);

    return result;
}

void high_performance_async_writer::update_stats(bool success, std::chrono::nanoseconds latency) {
    if (success) {
        stats_.successful_writes.fetch_add(1, std::memory_order_relaxed);
    }

    // Update average latency using exponential moving average
    const double latency_us = std::chrono::duration_cast<std::chrono::microseconds>(latency).count();
    const double alpha = 0.1; // Smoothing factor
    const double current_avg = stats_.average_latency_us.load(std::memory_order_relaxed);
    const double new_avg = alpha * latency_us + (1.0 - alpha) * current_avg;
    stats_.average_latency_us.store(new_avg, std::memory_order_relaxed);

    // Update throughput
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - stats_.start_time).count();
    if (elapsed > 0) {
        const auto total_writes = stats_.total_writes.load(std::memory_order_relaxed);
        const double throughput = static_cast<double>(total_writes) / elapsed;
        stats_.throughput_per_second.store(throughput, std::memory_order_relaxed);
    }
}

batch_processor::batch_entry high_performance_async_writer::to_batch_entry(
    const queued_log_entry& entry) const {
    return batch_processor::batch_entry(
        entry.level,
        entry.message,
        entry.file,
        entry.line,
        entry.function,
        entry.timestamp);
}

std::unique_ptr<high_performance_async_writer> make_high_performance_async_writer(
    std::unique_ptr<base_writer> writer,
    const high_performance_async_writer::config& cfg) {
    return std::make_unique<high_performance_async_writer>(std::move(writer), cfg);
}

} // namespace kcenon::logger::async