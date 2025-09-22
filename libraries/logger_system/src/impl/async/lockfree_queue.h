#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file lockfree_queue.h
 * @brief High-performance lock-free queue implementation
 *
 * This file provides a lock-free queue optimized for single-producer,
 * single-consumer scenarios commonly found in logging systems.
 *
 * Features:
 * - Lock-free implementation using atomic operations
 * - Single-producer, single-consumer optimized
 * - Memory ordering optimization
 * - ABA problem prevention
 * - Cache-friendly design with padding
 */

#include <atomic>
#include <memory>
#include <array>
#include <type_traits>

namespace kcenon::logger::async {

/**
 * @brief Lock-free single-producer single-consumer queue
 * @tparam T Type of elements to store
 * @tparam Size Queue capacity (must be power of 2)
 */
template<typename T, size_t Size>
class lockfree_spsc_queue {
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    static_assert(Size > 1, "Size must be greater than 1");

public:
    /**
     * @brief Constructor
     */
    lockfree_spsc_queue()
        : head_{0}
        , tail_{0} {
        // Initialize array elements
        for (size_t i = 0; i < Size; ++i) {
            sequence_[i].store(i, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Destructor
     */
    ~lockfree_spsc_queue() {
        T item;
        while (dequeue(item)) {
            // Clean up remaining items
        }
    }

    /**
     * @brief Enqueue an item (producer side)
     * @param item Item to enqueue
     * @return true if successful, false if queue is full
     */
    bool enqueue(const T& item) {
        return enqueue_impl(item);
    }

    /**
     * @brief Enqueue an item using move semantics (producer side)
     * @param item Item to enqueue
     * @return true if successful, false if queue is full
     */
    bool enqueue(T&& item) {
        return enqueue_impl(std::move(item));
    }

    /**
     * @brief Dequeue an item (consumer side)
     * @param item Reference to store the dequeued item
     * @return true if successful, false if queue is empty
     */
    bool dequeue(T& item) {
        const size_t pos = tail_.load(std::memory_order_relaxed);
        auto& cell = cells_[pos & mask_];

        const size_t seq = cell.sequence.load(std::memory_order_acquire);
        const size_t expected_seq = pos + 1;

        if (seq != expected_seq) {
            return false; // Queue is empty
        }

        item = std::move(cell.data);
        cell.sequence.store(pos + mask_ + 1, std::memory_order_release);
        tail_.store(pos + 1, std::memory_order_relaxed);

        return true;
    }

    /**
     * @brief Check if queue is empty
     * @return true if empty
     */
    bool empty() const {
        const size_t head_pos = head_.load(std::memory_order_acquire);
        const size_t tail_pos = tail_.load(std::memory_order_acquire);
        return head_pos == tail_pos;
    }

    /**
     * @brief Check if queue is full
     * @return true if full
     */
    bool full() const {
        const size_t head_pos = head_.load(std::memory_order_acquire);
        const size_t tail_pos = tail_.load(std::memory_order_acquire);
        return ((head_pos + 1) & mask_) == (tail_pos & mask_);
    }

    /**
     * @brief Get approximate queue size
     * @return Approximate number of elements in queue
     */
    size_t size() const {
        const size_t head_pos = head_.load(std::memory_order_acquire);
        const size_t tail_pos = tail_.load(std::memory_order_acquire);
        return head_pos - tail_pos;
    }

    /**
     * @brief Get queue capacity
     * @return Maximum number of elements
     */
    constexpr size_t capacity() const {
        return Size;
    }

private:
    static constexpr size_t mask_ = Size - 1;

    /**
     * @brief Cache line size for padding
     */
    static constexpr size_t cache_line_size = 64;

    /**
     * @brief Cell structure with sequence number for ABA prevention
     */
    struct alignas(cache_line_size) cell {
        std::atomic<size_t> sequence;
        T data;

        cell() : sequence{0} {}
    };

    /**
     * @brief Implementation for enqueue operations
     * @tparam U Universal reference type
     * @param item Item to enqueue
     * @return true if successful, false if queue is full
     */
    template<typename U>
    bool enqueue_impl(U&& item) {
        const size_t pos = head_.load(std::memory_order_relaxed);
        auto& cell = cells_[pos & mask_];

        const size_t seq = cell.sequence.load(std::memory_order_acquire);
        const size_t expected_seq = pos;

        if (seq != expected_seq) {
            return false; // Queue is full
        }

        cell.data = std::forward<U>(item);
        cell.sequence.store(pos + 1, std::memory_order_release);
        head_.store(pos + 1, std::memory_order_relaxed);

        return true;
    }

    // Align to cache line boundaries to prevent false sharing
    alignas(cache_line_size) std::atomic<size_t> head_;
    alignas(cache_line_size) std::atomic<size_t> tail_;
    alignas(cache_line_size) std::array<cell, Size> cells_;
    alignas(cache_line_size) std::array<std::atomic<size_t>, Size> sequence_;
};

/**
 * @brief Factory function to create a lock-free queue
 * @tparam T Element type
 * @tparam Size Queue size (must be power of 2)
 * @return Unique pointer to the queue
 */
template<typename T, size_t Size = 1024>
std::unique_ptr<lockfree_spsc_queue<T, Size>> make_lockfree_queue() {
    return std::make_unique<lockfree_spsc_queue<T, Size>>();
}

/**
 * @brief Multi-producer multi-consumer lock-free queue (for future use)
 *
 * Note: This is a placeholder for MPMC implementation if needed.
 * Currently, SPSC is sufficient for most logging scenarios.
 */
template<typename T, size_t Size>
class lockfree_mpmc_queue {
    // TODO: Implement if multi-producer scenarios are needed
    static_assert(sizeof(T) == 0, "MPMC queue not yet implemented");
};

} // namespace kcenon::logger::async