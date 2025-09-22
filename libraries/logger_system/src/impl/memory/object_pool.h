/**
 * @file object_pool.h
 * @brief Object pool implementation for high-performance memory management
 *
 * This file provides a thread-safe object pool implementation for reducing
 * memory allocation overhead in high-frequency logging scenarios.
 */

#pragma once

#include <kcenon/logger/core/error_codes.h>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>

namespace kcenon::logger::memory {

/**
 * @brief Thread-safe object pool for high-performance memory management
 * @tparam T The type of objects to pool
 */
template<typename T>
class object_pool {
public:
    /**
     * @brief Configuration for object pool
     */
    struct config {
        size_t initial_size{100};        ///< Initial pool size
        size_t max_size{10000};          ///< Maximum pool size
        bool allow_growth{true};         ///< Allow pool to grow beyond initial size

        config() = default;
    };

    /**
     * @brief Construct object pool with configuration
     * @param cfg Pool configuration
     */
    explicit object_pool(const config& cfg = config{})
        : config_(cfg), pool_size_(0) {
        initialize_pool();
    }

    /**
     * @brief Destructor
     */
    ~object_pool() = default;

    /**
     * @brief Get an object from the pool
     * @return Unique pointer to object, or nullptr if pool is empty and growth is disabled
     */
    std::unique_ptr<T> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!available_objects_.empty()) {
            auto obj = std::move(available_objects_.front());
            available_objects_.pop();
            return obj;
        }

        // If pool is empty and growth is allowed, create new object
        if (config_.allow_growth && pool_size_.load() < config_.max_size) {
            pool_size_.fetch_add(1);
            return std::make_unique<T>();
        }

        // Create temporary object if pool limits reached
        return std::make_unique<T>();
    }

    /**
     * @brief Return an object to the pool
     * @param obj Object to return
     */
    void release(std::unique_ptr<T> obj) {
        if (!obj) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        // Only return to pool if not exceeding max size
        if (available_objects_.size() < config_.max_size) {
            available_objects_.push(std::move(obj));
        }
        // If exceeding max size, object will be destroyed automatically
    }

    /**
     * @brief Get current pool statistics
     */
    struct statistics {
        size_t total_size;
        size_t available_count;
        size_t in_use_count;
    };

    /**
     * @brief Get pool statistics
     * @return Current pool statistics
     */
    statistics get_statistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return {
            .total_size = pool_size_.load(),
            .available_count = available_objects_.size(),
            .in_use_count = pool_size_.load() - available_objects_.size()
        };
    }

    /**
     * @brief Clear all objects from pool
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!available_objects_.empty()) {
            available_objects_.pop();
        }
        pool_size_.store(0);
    }

private:
    /**
     * @brief Initialize pool with initial objects
     */
    void initialize_pool() {
        std::lock_guard<std::mutex> lock(mutex_);

        for (size_t i = 0; i < config_.initial_size; ++i) {
            available_objects_.push(std::make_unique<T>());
        }

        pool_size_.store(config_.initial_size);
    }

    config config_;                                    ///< Pool configuration
    mutable std::mutex mutex_;                         ///< Thread safety mutex
    std::queue<std::unique_ptr<T>> available_objects_; ///< Available objects
    std::atomic<size_t> pool_size_;                    ///< Current pool size
};

} // namespace kcenon::logger::memory