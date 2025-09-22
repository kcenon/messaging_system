#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file ring_buffer.h
 * @brief Lock-free ring buffer for efficient metric storage
 * 
 * This file provides a high-performance, memory-efficient ring buffer
 * implementation specifically designed for metric storage with minimal
 * allocation overhead and cache-friendly access patterns.
 */

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <memory>
#include <atomic>
#include <vector>
#include <chrono>
#include <cstddef>
#include <type_traits>

namespace monitoring_system {

/**
 * @struct ring_buffer_config
 * @brief Configuration for ring buffer behavior
 */
struct ring_buffer_config {
    size_t capacity = 8192;                        // Default capacity (power of 2)
    bool overwrite_old = true;                     // Overwrite oldest data when full
    size_t batch_size = 64;                        // Batch size for bulk operations
    std::chrono::milliseconds gc_interval{1000};   // Garbage collection interval
    
    /**
     * @brief Validate ring buffer configuration
     */
    result_void validate() const {
        if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Capacity must be a power of 2");
        }
        
        if (batch_size == 0 || batch_size > capacity) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Invalid batch size");
        }
        
        return result_void::success();
    }
};

/**
 * @struct ring_buffer_stats
 * @brief Statistics for ring buffer performance monitoring
 */
struct ring_buffer_stats {
    std::atomic<size_t> total_writes{0};
    std::atomic<size_t> total_reads{0};
    std::atomic<size_t> overwrites{0};
    std::atomic<size_t> failed_writes{0};
    std::atomic<size_t> failed_reads{0};
    std::chrono::system_clock::time_point creation_time;
    
    ring_buffer_stats() : creation_time(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Get current utilization percentage
     */
    double get_utilization(size_t current_size, size_t capacity) const {
        return capacity > 0 ? (static_cast<double>(current_size) / capacity) * 100.0 : 0.0;
    }
    
    /**
     * @brief Get write success rate
     */
    double get_write_success_rate() const {
        auto total = total_writes.load();
        auto failed = failed_writes.load();
        return total > 0 ? (1.0 - static_cast<double>(failed) / total) * 100.0 : 100.0;
    }
};

/**
 * @class ring_buffer
 * @brief Lock-free ring buffer with atomic operations
 * @tparam T The type of elements to store
 * 
 * This implementation uses atomic operations for thread-safety and
 * provides efficient circular buffer semantics with configurable
 * overflow behavior.
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#endif

template<typename T>
class ring_buffer {
private:
    static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
    static_assert(std::is_move_assignable_v<T>, "T must be move assignable");
    
    alignas(64) std::atomic<size_t> write_index_{0};  // Cache line aligned
    alignas(64) std::atomic<size_t> read_index_{0};   // Cache line aligned
    
    std::unique_ptr<T[]> buffer_;
    ring_buffer_config config_;
    mutable ring_buffer_stats stats_;
    
    /**
     * @brief Get the mask for efficient modulo operation
     */
    size_t get_mask() const noexcept {
        return config_.capacity - 1;
    }
    
    /**
     * @brief Check if buffer is full
     */
    bool is_full_unsafe(size_t write_idx, size_t read_idx) const noexcept {
        return ((write_idx + 1) & get_mask()) == read_idx;
    }
    
    /**
     * @brief Check if buffer is empty
     */
    bool is_empty_unsafe(size_t write_idx, size_t read_idx) const noexcept {
        return write_idx == read_idx;
    }
    
public:
    /**
     * @brief Constructor with configuration
     */
    explicit ring_buffer(const ring_buffer_config& config = {})
        : buffer_(std::make_unique<T[]>(config.capacity))
        , config_(config) {
        
        // Validate configuration
        auto validation = config_.validate();
        if (!validation) {
            throw std::invalid_argument("Invalid ring buffer configuration: " + 
                                      validation.get_error().message);
        }
    }
    
    // Non-copyable but moveable
    ring_buffer(const ring_buffer&) = delete;
    ring_buffer& operator=(const ring_buffer&) = delete;
    ring_buffer(ring_buffer&&) = default;
    ring_buffer& operator=(ring_buffer&&) = default;
    
    /**
     * @brief Write a single element to the buffer
     * @param item Item to write
     * @return Result indicating success or failure
     */
    result_void write(T&& item) {
        stats_.total_writes.fetch_add(1, std::memory_order_relaxed);
        
        size_t current_write = write_index_.load(std::memory_order_acquire);
        size_t current_read = read_index_.load(std::memory_order_acquire);
        
        if (is_full_unsafe(current_write, current_read)) {
            if (config_.overwrite_old) {
                // Advance read index to overwrite oldest
                size_t new_read = (current_read + 1) & get_mask();
                read_index_.compare_exchange_weak(current_read, new_read, 
                                                std::memory_order_acq_rel);
                stats_.overwrites.fetch_add(1, std::memory_order_relaxed);
            } else {
                stats_.failed_writes.fetch_add(1, std::memory_order_relaxed);
                return result_void(monitoring_error_code::storage_full, 
                                 "Ring buffer is full");
            }
        }
        
        // Write the item
        buffer_[current_write] = std::move(item);
        
        // Update write index
        size_t new_write = (current_write + 1) & get_mask();
        write_index_.store(new_write, std::memory_order_release);
        
        return result_void::success();
    }
    
    /**
     * @brief Write multiple elements in batch
     * @param items Vector of items to write
     * @return Number of items successfully written
     */
    size_t write_batch(std::vector<T>&& items) {
        if (items.empty()) {
            return 0;
        }
        
        size_t written = 0;
        for (auto& item : items) {
            auto result = write(std::move(item));
            if (result) {
                ++written;
            } else if (!config_.overwrite_old) {
                break; // Stop on first failure if not overwriting
            }
        }
        
        return written;
    }
    
    /**
     * @brief Read a single element from the buffer
     * @param item Reference to store the read item
     * @return Result indicating success or failure
     */
    result_void read(T& item) {
        stats_.total_reads.fetch_add(1, std::memory_order_relaxed);
        
        size_t current_read = read_index_.load(std::memory_order_acquire);
        size_t current_write = write_index_.load(std::memory_order_acquire);
        
        if (is_empty_unsafe(current_write, current_read)) {
            stats_.failed_reads.fetch_add(1, std::memory_order_relaxed);
            return result_void(monitoring_error_code::collection_failed,
                             "Ring buffer is empty");
        }
        
        // Read the item
        item = std::move(buffer_[current_read]);
        
        // Update read index
        size_t new_read = (current_read + 1) & get_mask();
        read_index_.store(new_read, std::memory_order_release);
        
        return result_void::success();
    }
    
    /**
     * @brief Read multiple elements in batch
     * @param items Vector to store read items
     * @param max_count Maximum number of items to read
     * @return Number of items actually read
     */
    size_t read_batch(std::vector<T>& items, size_t max_count = SIZE_MAX) {
        if (max_count == 0) {
            return 0;
        }
        
        size_t batch_size = std::min(max_count, config_.batch_size);
        items.reserve(items.size() + batch_size);
        
        size_t read_count = 0;
        T temp_item;
        
        while (read_count < batch_size) {
            auto result = read(temp_item);
            if (!result) {
                break; // No more items to read
            }
            
            items.emplace_back(std::move(temp_item));
            ++read_count;
        }
        
        return read_count;
    }
    
    /**
     * @brief Peek at the next item without removing it
     * @param item Reference to store the peeked item
     * @return Result indicating success or failure
     */
    result_void peek(T& item) const {
        size_t current_read = read_index_.load(std::memory_order_acquire);
        size_t current_write = write_index_.load(std::memory_order_acquire);
        
        if (is_empty_unsafe(current_write, current_read)) {
            return result_void(monitoring_error_code::collection_failed,
                             "Ring buffer is empty");
        }
        
        item = buffer_[current_read]; // Copy, don't move
        return result_void::success();
    }
    
    /**
     * @brief Get current number of elements in buffer
     */
    size_t size() const noexcept {
        size_t write_idx = write_index_.load(std::memory_order_acquire);
        size_t read_idx = read_index_.load(std::memory_order_acquire);
        
        if (write_idx >= read_idx) {
            return write_idx - read_idx;
        } else {
            return config_.capacity - (read_idx - write_idx);
        }
    }
    
    /**
     * @brief Check if buffer is empty
     */
    bool empty() const noexcept {
        return size() == 0;
    }
    
    /**
     * @brief Check if buffer is full
     */
    bool full() const noexcept {
        size_t write_idx = write_index_.load(std::memory_order_acquire);
        size_t read_idx = read_index_.load(std::memory_order_acquire);
        return is_full_unsafe(write_idx, read_idx);
    }
    
    /**
     * @brief Get buffer capacity
     */
    size_t capacity() const noexcept {
        return config_.capacity;
    }
    
    /**
     * @brief Clear all elements in the buffer
     */
    void clear() noexcept {
        write_index_.store(0, std::memory_order_release);
        read_index_.store(0, std::memory_order_release);
    }
    
    /**
     * @brief Get buffer configuration
     */
    const ring_buffer_config& get_config() const noexcept {
        return config_;
    }
    
    /**
     * @brief Get buffer statistics
     */
    const ring_buffer_stats& get_stats() const noexcept {
        return stats_;
    }
    
    /**
     * @brief Reset statistics
     */
    void reset_stats() noexcept {
        stats_.total_writes.store(0);
        stats_.total_reads.store(0);
        stats_.overwrites.store(0);
        stats_.failed_writes.store(0);
        stats_.failed_reads.store(0);
        stats_.creation_time = std::chrono::system_clock::now();
    }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/**
 * @brief Helper function to create a ring buffer with default configuration
 */
template<typename T>
std::unique_ptr<ring_buffer<T>> make_ring_buffer(size_t capacity = 8192) {
    ring_buffer_config config;
    config.capacity = capacity;
    return std::make_unique<ring_buffer<T>>(config);
}

/**
 * @brief Helper function to create a ring buffer with custom configuration
 */
template<typename T>
std::unique_ptr<ring_buffer<T>> make_ring_buffer(const ring_buffer_config& config) {
    return std::make_unique<ring_buffer<T>>(config);
}

} // namespace monitoring_system