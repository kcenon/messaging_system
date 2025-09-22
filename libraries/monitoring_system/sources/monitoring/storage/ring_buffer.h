#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>

namespace monitoring_module {

/**
 * @brief Thread-safe circular buffer for storing historical data
 * @tparam T Type of elements to store
 */
template<typename T>
class ring_buffer {
public:
    /**
     * @brief Constructor
     * @param capacity Maximum number of elements to store
     */
    explicit ring_buffer(std::size_t capacity)
        : buffer_(capacity)
        , capacity_(capacity)
        , head_(0)
        , tail_(0)
        , size_(0) {
    }
    
    /**
     * @brief Push an element to the buffer
     * @param value Element to add
     * @return true if successful, false if buffer is full
     */
    bool push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (size_ >= capacity_) {
            // Overwrite oldest element
            tail_ = (tail_ + 1) % capacity_;
        } else {
            size_++;
        }
        
        buffer_[head_] = value;
        head_ = (head_ + 1) % capacity_;
        
        return true;
    }
    
    /**
     * @brief Get recent elements from the buffer
     * @param count Number of elements to retrieve
     * @return Vector of elements, newest first
     */
    std::vector<T> get_recent(std::size_t count) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        count = std::min(count, size_);
        std::vector<T> result;
        result.reserve(count);
        
        // Start from the most recent element
        std::size_t index = (head_ + capacity_ - 1) % capacity_;
        
        for (std::size_t i = 0; i < count; ++i) {
            result.push_back(buffer_[index]);
            index = (index + capacity_ - 1) % capacity_;
        }
        
        return result;
    }
    
    /**
     * @brief Get all elements in the buffer
     * @return Vector of all elements, oldest first
     */
    std::vector<T> get_all() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<T> result;
        result.reserve(size_);
        
        if (size_ == 0) {
            return result;
        }
        
        std::size_t index = tail_;
        for (std::size_t i = 0; i < size_; ++i) {
            result.push_back(buffer_[index]);
            index = (index + 1) % capacity_;
        }
        
        return result;
    }
    
    /**
     * @brief Clear all elements from the buffer
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }
    
    /**
     * @brief Get the number of elements in the buffer
     * @return Current size
     */
    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }
    
    /**
     * @brief Get the capacity of the buffer
     * @return Maximum capacity
     */
    std::size_t capacity() const {
        return capacity_;
    }
    
    /**
     * @brief Check if the buffer is empty
     * @return true if empty
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_ == 0;
    }
    
    /**
     * @brief Check if the buffer is full
     * @return true if full
     */
    bool full() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_ >= capacity_;
    }
    
private:
    std::vector<T> buffer_;
    const std::size_t capacity_;
    std::size_t head_;  // Next write position
    std::size_t tail_;  // First read position
    std::size_t size_;  // Current number of elements
    mutable std::mutex mutex_;
};

} // namespace monitoring_module