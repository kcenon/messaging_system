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

#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <utility>

namespace kcenon::logger {

/**
 * @brief Small String Optimization (SSO) implementation
 * 
 * This class implements a string with Small String Optimization.
 * Strings smaller than SSO_CAPACITY are stored inline, avoiding heap allocation.
 * Larger strings use heap allocation like std::string.
 * 
 * @tparam SSO_SIZE Size threshold for SSO (default 256 bytes)
 */
template<size_t SSO_SIZE = 256>
class small_string {
public:
    static constexpr size_t SSO_CAPACITY = SSO_SIZE - 1; // Reserve 1 byte for null terminator
    
    /**
     * @brief Default constructor
     */
    small_string() noexcept : size_(0), is_small_(true) {
        data_.small[0] = '\0';
    }
    
    /**
     * @brief Construct from C-string
     */
    small_string(const char* str) : small_string() {
        if (str) {
            assign(str, std::strlen(str));
        }
    }
    
    /**
     * @brief Construct from std::string
     */
    small_string(const std::string& str) : small_string() {
        assign(str.data(), str.size());
    }
    
    /**
     * @brief Construct from string_view
     */
    small_string(std::string_view str) : small_string() {
        assign(str.data(), str.size());
    }
    
    /**
     * @brief Copy constructor
     */
    small_string(const small_string& other) : small_string() {
        if (other.is_small_) {
            std::memcpy(data_.small, other.data_.small, other.size_ + 1);
            size_ = other.size_;
            is_small_ = true;
        } else {
            assign(other.data(), other.size());
        }
    }
    
    /**
     * @brief Move constructor
     */
    small_string(small_string&& other) noexcept 
        : size_(other.size_), is_small_(other.is_small_) {
        if (is_small_) {
            std::memcpy(data_.small, other.data_.small, size_ + 1);
        } else {
            data_.heap.ptr = other.data_.heap.ptr;
            data_.heap.capacity = other.data_.heap.capacity;
            other.data_.heap.ptr = nullptr;
            other.size_ = 0;
            other.is_small_ = true;
        }
    }
    
    /**
     * @brief Destructor
     */
    ~small_string() {
        if (!is_small_ && data_.heap.ptr) {
            delete[] data_.heap.ptr;
        }
    }
    
    /**
     * @brief Copy assignment
     */
    small_string& operator=(const small_string& other) {
        if (this != &other) {
            assign(other.data(), other.size());
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     */
    small_string& operator=(small_string&& other) noexcept {
        if (this != &other) {
            // Clean up current heap allocation if any
            if (!is_small_ && data_.heap.ptr) {
                delete[] data_.heap.ptr;
            }
            
            size_ = other.size_;
            is_small_ = other.is_small_;
            
            if (is_small_) {
                std::memcpy(data_.small, other.data_.small, size_ + 1);
            } else {
                data_.heap.ptr = other.data_.heap.ptr;
                data_.heap.capacity = other.data_.heap.capacity;
                other.data_.heap.ptr = nullptr;
                other.size_ = 0;
                other.is_small_ = true;
            }
        }
        return *this;
    }
    
    /**
     * @brief Assign from string data
     */
    void assign(const char* str, size_t len) {
        if (len <= SSO_CAPACITY) {
            // Use small string optimization
            if (!is_small_ && data_.heap.ptr) {
                delete[] data_.heap.ptr;
            }
            std::memcpy(data_.small, str, len);
            data_.small[len] = '\0';
            size_ = len;
            is_small_ = true;
        } else {
            // Use heap allocation
            if (is_small_ || data_.heap.capacity < len + 1) {
                // Need to allocate new buffer
                if (!is_small_ && data_.heap.ptr) {
                    delete[] data_.heap.ptr;
                }
                
                size_t new_capacity = calculate_capacity(len);
                data_.heap.ptr = new char[new_capacity];
                data_.heap.capacity = new_capacity;
                is_small_ = false;
            }
            
            std::memcpy(data_.heap.ptr, str, len);
            data_.heap.ptr[len] = '\0';
            size_ = len;
        }
    }
    
    /**
     * @brief Get C-string pointer
     */
    const char* c_str() const noexcept {
        return data();
    }
    
    /**
     * @brief Get data pointer
     */
    const char* data() const noexcept {
        return is_small_ ? data_.small : data_.heap.ptr;
    }
    
    /**
     * @brief Get size
     */
    size_t size() const noexcept {
        return size_;
    }
    
    /**
     * @brief Get length (same as size)
     */
    size_t length() const noexcept {
        return size_;
    }
    
    /**
     * @brief Check if empty
     */
    bool empty() const noexcept {
        return size_ == 0;
    }
    
    /**
     * @brief Check if using small string optimization
     */
    bool is_small() const noexcept {
        return is_small_;
    }
    
    /**
     * @brief Get capacity
     */
    size_t capacity() const noexcept {
        return is_small_ ? SSO_CAPACITY : data_.heap.capacity - 1;
    }
    
    /**
     * @brief Clear the string
     */
    void clear() noexcept {
        if (!is_small_ && data_.heap.ptr) {
            data_.heap.ptr[0] = '\0';
        } else {
            data_.small[0] = '\0';
        }
        size_ = 0;
    }
    
    /**
     * @brief Reserve capacity
     */
    void reserve(size_t new_capacity) {
        if (new_capacity <= capacity()) {
            return;
        }
        
        if (new_capacity <= SSO_CAPACITY) {
            // Already using SSO, no need to reserve
            return;
        }
        
        // Need heap allocation
        size_t actual_capacity = calculate_capacity(new_capacity);
        char* new_ptr = new char[actual_capacity];
        
        // Copy existing data
        std::memcpy(new_ptr, data(), size_ + 1);
        
        // Clean up old allocation
        if (!is_small_ && data_.heap.ptr) {
            delete[] data_.heap.ptr;
        }
        
        data_.heap.ptr = new_ptr;
        data_.heap.capacity = actual_capacity;
        is_small_ = false;
    }
    
    /**
     * @brief Append a string
     */
    void append(const char* str, size_t len) {
        size_t new_size = size_ + len;
        
        if (new_size <= SSO_CAPACITY && is_small_) {
            // Can still use SSO
            std::memcpy(data_.small + size_, str, len);
            data_.small[new_size] = '\0';
            size_ = new_size;
        } else {
            // Need heap allocation
            if (is_small_ || data_.heap.capacity < new_size + 1) {
                // Need to reallocate
                size_t new_capacity = calculate_capacity(new_size);
                char* new_ptr = new char[new_capacity];
                
                // Copy existing data
                std::memcpy(new_ptr, data(), size_);
                // Append new data
                std::memcpy(new_ptr + size_, str, len);
                new_ptr[new_size] = '\0';
                
                // Clean up old allocation
                if (!is_small_ && data_.heap.ptr) {
                    delete[] data_.heap.ptr;
                }
                
                data_.heap.ptr = new_ptr;
                data_.heap.capacity = new_capacity;
                is_small_ = false;
            } else {
                // Existing heap buffer is large enough
                std::memcpy(data_.heap.ptr + size_, str, len);
                data_.heap.ptr[new_size] = '\0';
            }
            size_ = new_size;
        }
    }
    
    /**
     * @brief Append a string
     */
    void append(const std::string& str) {
        append(str.data(), str.size());
    }
    
    /**
     * @brief Append operator
     */
    small_string& operator+=(const char* str) {
        append(str, std::strlen(str));
        return *this;
    }
    
    /**
     * @brief Append operator
     */
    small_string& operator+=(const std::string& str) {
        append(str);
        return *this;
    }
    
    /**
     * @brief Convert to std::string
     */
    std::string to_string() const {
        return std::string(data(), size_);
    }
    
    /**
     * @brief Implicit conversion to string_view
     */
    operator std::string_view() const noexcept {
        return std::string_view(data(), size_);
    }
    
    /**
     * @brief Equality comparison
     */
    bool operator==(const small_string& other) const noexcept {
        return size_ == other.size_ && 
               std::memcmp(data(), other.data(), size_) == 0;
    }
    
    /**
     * @brief Inequality comparison
     */
    bool operator!=(const small_string& other) const noexcept {
        return !(*this == other);
    }
    
    /**
     * @brief Comparison with std::string
     */
    bool operator==(const std::string& str) const noexcept {
        return size_ == str.size() && 
               std::memcmp(data(), str.data(), size_) == 0;
    }
    
    /**
     * @brief Get memory usage statistics
     */
    struct memory_stats {
        size_t string_size;
        size_t capacity;
        bool is_small;
        size_t heap_bytes_used;
        size_t total_bytes;
    };
    
    memory_stats get_memory_stats() const noexcept {
        memory_stats stats;
        stats.string_size = size_;
        stats.capacity = capacity();
        stats.is_small = is_small_;
        stats.heap_bytes_used = is_small_ ? 0 : data_.heap.capacity;
        stats.total_bytes = sizeof(*this) + stats.heap_bytes_used;
        return stats;
    }
    
private:
    /**
     * @brief Calculate capacity for heap allocation
     */
    static size_t calculate_capacity(size_t required) {
        // Round up to next power of 2 for better allocation patterns
        size_t capacity = required + 1; // +1 for null terminator
        capacity = capacity * 3 / 2; // 1.5x growth factor
        
        // Align to 16 bytes for better memory alignment
        return (capacity + 15) & ~15;
    }
    
    // Data storage
    union data_union {
        // Small string storage (stack)
        char small[SSO_SIZE];
        
        // Heap storage
        struct {
            char* ptr;
            size_t capacity;
        } heap;
        
        data_union() : small{} {}
    } data_;
    
    size_t size_;
    bool is_small_;
};

// Type aliases for common sizes
using small_string_64 = small_string<64>;
using small_string_128 = small_string<128>;
using small_string_256 = small_string<256>;
using small_string_512 = small_string<512>;

} // namespace kcenon::logger