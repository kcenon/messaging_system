/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#pragma once

#include "container/internal/variant_value.h"
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>

namespace container_module
{
    /**
     * @brief Thread-safe container with lock optimization
     * 
     * This container provides thread-safe access to variant values with optimized
     * locking strategies for different access patterns.
     */
    class thread_safe_container : public std::enable_shared_from_this<thread_safe_container>
    {
    public:
        using value_map = std::unordered_map<std::string, variant_value>;
        using const_iterator = value_map::const_iterator;
        using iterator = value_map::iterator;

        /**
         * @brief Default constructor
         */
        thread_safe_container() = default;

        /**
         * @brief Construct with initial values
         */
        thread_safe_container(std::initializer_list<std::pair<std::string, ValueVariant>> init);

        /**
         * @brief Copy constructor (thread-safe)
         */
        thread_safe_container(const thread_safe_container& other);

        /**
         * @brief Move constructor
         */
        thread_safe_container(thread_safe_container&& other) noexcept;

        /**
         * @brief Copy assignment (thread-safe)
         */
        thread_safe_container& operator=(const thread_safe_container& other);

        /**
         * @brief Move assignment
         */
        thread_safe_container& operator=(thread_safe_container&& other) noexcept;

        /**
         * @brief Get value by key (thread-safe read)
         * @param key The key to look up
         * @return Optional containing the variant_value if found
         */
        std::optional<variant_value> get(std::string_view key) const;

        /**
         * @brief Get typed value by key
         * @tparam T The expected type
         * @param key The key to look up
         * @return Optional containing the value if found and type matches
         */
        template<typename T>
        std::optional<T> get_typed(std::string_view key) const {
            std::shared_lock lock(mutex_);
            auto it = values_.find(std::string(key));
            if (it != values_.end()) {
                return it->second.get<T>();
            }
            return std::nullopt;
        }

        /**
         * @brief Set value for key (thread-safe write)
         * @param key The key to set
         * @param value The value to store
         */
        void set(std::string_view key, ValueVariant value);

        /**
         * @brief Set typed value for key
         * @tparam T The value type
         * @param key The key to set
         * @param value The value to store
         */
        template<typename T>
        void set_typed(std::string_view key, T&& value) {
            static_assert(is_variant_type_v<std::decay_t<T>>, 
                         "Type must be a valid variant type");
            set(key, ValueVariant(std::forward<T>(value)));
        }

        /**
         * @brief Remove value by key
         * @param key The key to remove
         * @return true if removed, false if not found
         */
        bool remove(std::string_view key);

        /**
         * @brief Clear all values
         */
        void clear();

        /**
         * @brief Get the number of values
         */
        size_t size() const;

        /**
         * @brief Check if container is empty
         */
        bool empty() const;

        /**
         * @brief Check if key exists
         */
        bool contains(std::string_view key) const;

        /**
         * @brief Get all keys
         */
        std::vector<std::string> keys() const;

        /**
         * @brief Apply a function to all values (read-only)
         * @tparam Func Function type
         * @param func Function to apply to each key-value pair
         */
        template<typename Func>
        void for_each(Func&& func) const {
            std::shared_lock lock(mutex_);
            for (const auto& [key, value] : values_) {
                func(key, value);
            }
        }

        /**
         * @brief Apply a function to all values (mutable)
         * @tparam Func Function type
         * @param func Function to apply to each key-value pair
         */
        template<typename Func>
        void for_each_mut(Func&& func) {
            std::unique_lock lock(mutex_);
            for (auto& [key, value] : values_) {
                func(key, value);
            }
        }

        /**
         * @brief Bulk update operation with minimal lock contention
         * @tparam Func Function type that takes the entire map
         * @param updater Function to perform bulk updates
         */
        template<typename Func>
        void bulk_update(Func&& updater) {
            std::unique_lock lock(mutex_);
            updater(values_);
            bulk_write_count_.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Bulk read operation
         * @tparam Func Function type that takes the entire map
         * @param reader Function to perform bulk reads
         * @return Result of the reader function
         */
        template<typename Func>
        auto bulk_read(Func&& reader) const {
            std::shared_lock lock(mutex_);
            bulk_read_count_.fetch_add(1, std::memory_order_relaxed);
            return reader(values_);
        }

        /**
         * @brief Atomic compare and swap
         * @param key The key to update
         * @param expected The expected current value
         * @param desired The desired new value
         * @return true if swap succeeded, false otherwise
         */
        bool compare_exchange(std::string_view key, 
                            const ValueVariant& expected,
                            const ValueVariant& desired);

        /**
         * @brief Get statistics
         */
        struct Statistics {
            size_t read_count;
            size_t write_count;
            size_t bulk_read_count;
            size_t bulk_write_count;
            size_t size;
        };

        Statistics get_statistics() const;

        /**
         * @brief Serialize container to JSON
         */
        std::string to_json() const;

        /**
         * @brief Serialize container to binary
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize from binary
         */
        static std::shared_ptr<thread_safe_container> deserialize(
            const std::vector<uint8_t>& data);

        /**
         * @brief Array-style access (creates if not exists)
         */
        variant_value& operator[](const std::string& key);

    private:
        mutable std::shared_mutex mutex_;
        value_map values_;
        
        // Statistics
        mutable std::atomic<size_t> read_count_{0};
        mutable std::atomic<size_t> write_count_{0};
        mutable std::atomic<size_t> bulk_read_count_{0};
        mutable std::atomic<size_t> bulk_write_count_{0};
    };

    /**
     * @brief Lock-free reader for frequently accessed data
     * 
     * Uses RCU (Read-Copy-Update) pattern for lock-free reads
     */
    class lockfree_reader {
    public:
        using snapshot_ptr = std::shared_ptr<const thread_safe_container::value_map>;

        explicit lockfree_reader(std::shared_ptr<thread_safe_container> container)
            : container_(container) {
            update_snapshot();
        }

        /**
         * @brief Get value without locking (from snapshot)
         */
        template<typename T>
        std::optional<T> get(std::string_view key) const {
            std::shared_lock lock(snapshot_mutex_);
            if (!snapshot_) return std::nullopt;
            
            auto it = snapshot_->find(std::string(key));
            if (it != snapshot_->end()) {
                return it->second.get<T>();
            }
            return std::nullopt;
        }

        /**
         * @brief Update snapshot from container
         */
        void update_snapshot();

    private:
        std::shared_ptr<thread_safe_container> container_;
        snapshot_ptr snapshot_;
        mutable std::shared_mutex snapshot_mutex_;
    };

} // namespace container_module