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

#include "container/internal/thread_safe_container.h"
#include "utilities/core/formatter.h"
#include <algorithm>

namespace container_module
{
    using namespace utility_module;

    thread_safe_container::thread_safe_container(
        std::initializer_list<std::pair<std::string, ValueVariant>> init)
    {
        for (const auto& [key, value] : init) {
            values_.emplace(key, variant_value(key, value));
        }
    }

    thread_safe_container::thread_safe_container(const thread_safe_container& other)
    {
        std::shared_lock other_lock(other.mutex_);
        values_ = other.values_;
        read_count_ = other.read_count_.load();
        write_count_ = other.write_count_.load();
        bulk_read_count_ = other.bulk_read_count_.load();
        bulk_write_count_ = other.bulk_write_count_.load();
    }

    thread_safe_container::thread_safe_container(thread_safe_container&& other) noexcept
    {
        std::unique_lock other_lock(other.mutex_);
        values_ = std::move(other.values_);
        read_count_ = other.read_count_.load();
        write_count_ = other.write_count_.load();
        bulk_read_count_ = other.bulk_read_count_.load();
        bulk_write_count_ = other.bulk_write_count_.load();
        
        other.read_count_ = 0;
        other.write_count_ = 0;
        other.bulk_read_count_ = 0;
        other.bulk_write_count_ = 0;
    }

    thread_safe_container& thread_safe_container::operator=(const thread_safe_container& other)
    {
        if (this != &other) {
            std::unique_lock lock(mutex_);
            std::shared_lock other_lock(other.mutex_);
            values_ = other.values_;
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    thread_safe_container& thread_safe_container::operator=(thread_safe_container&& other) noexcept
    {
        if (this != &other) {
            std::unique_lock lock(mutex_);
            std::unique_lock other_lock(other.mutex_);
            values_ = std::move(other.values_);
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    std::optional<variant_value> thread_safe_container::get(std::string_view key) const
    {
        std::shared_lock lock(mutex_);
        read_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = values_.find(std::string(key));
        if (it != values_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void thread_safe_container::set(std::string_view key, ValueVariant value)
    {
        std::unique_lock lock(mutex_);
        write_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = values_.find(std::string(key));
        if (it != values_.end()) {
            it->second.set(std::move(value));
        } else {
            values_.emplace(std::string(key), variant_value(key, std::move(value)));
        }
    }

    bool thread_safe_container::remove(std::string_view key)
    {
        std::unique_lock lock(mutex_);
        write_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = values_.find(std::string(key));
        if (it != values_.end()) {
            values_.erase(it);
            return true;
        }
        return false;
    }

    void thread_safe_container::clear()
    {
        std::unique_lock lock(mutex_);
        write_count_.fetch_add(1, std::memory_order_relaxed);
        values_.clear();
    }

    size_t thread_safe_container::size() const
    {
        std::shared_lock lock(mutex_);
        return values_.size();
    }

    bool thread_safe_container::empty() const
    {
        std::shared_lock lock(mutex_);
        return values_.empty();
    }

    bool thread_safe_container::contains(std::string_view key) const
    {
        std::shared_lock lock(mutex_);
        read_count_.fetch_add(1, std::memory_order_relaxed);
        return values_.find(std::string(key)) != values_.end();
    }

    std::vector<std::string> thread_safe_container::keys() const
    {
        std::shared_lock lock(mutex_);
        read_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::vector<std::string> result;
        result.reserve(values_.size());
        
        for (const auto& [key, value] : values_) {
            result.push_back(key);
        }
        
        return result;
    }

    bool thread_safe_container::compare_exchange(std::string_view key,
                                                const ValueVariant& expected,
                                                const ValueVariant& desired)
    {
        std::unique_lock lock(mutex_);
        write_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = values_.find(std::string(key));
        if (it != values_.end()) {
            // Compare variant values directly
            return it->second.visit([&, this](auto&& current) -> bool {
                using T = std::decay_t<decltype(current)>;
                
                // Check if expected holds same type
                if (auto* expected_val = std::get_if<T>(&expected)) {
                    if (current == *expected_val) {
                        it->second.set(desired);
                        return true;
                    }
                }
                return false;
            });
        }
        return false;
    }

    thread_safe_container::Statistics thread_safe_container::get_statistics() const
    {
        std::shared_lock lock(mutex_);
        return {
            read_count_.load(std::memory_order_relaxed),
            write_count_.load(std::memory_order_relaxed),
            bulk_read_count_.load(std::memory_order_relaxed),
            bulk_write_count_.load(std::memory_order_relaxed),
            values_.size()
        };
    }

    std::string thread_safe_container::to_json() const
    {
        std::shared_lock lock(mutex_);
        read_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::string result = "{";
        bool first = true;
        
        for (const auto& [key, value] : values_) {
            if (!first) {
                result += ",";
            }
            first = false;
            
            // Escape key for JSON
            result += "\"";
            for (char c : key) {
                switch (c) {
                    case '"': result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default: result += c;
                }
            }
            result += "\":";
            result += value.to_json();
        }
        
        result += "}";
        return result;
    }

    std::vector<uint8_t> thread_safe_container::serialize() const
    {
        std::shared_lock lock(mutex_);
        read_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::vector<uint8_t> result;
        
        // Header: number of entries (4 bytes)
        uint32_t count = static_cast<uint32_t>(values_.size());
        result.insert(result.end(),
                     reinterpret_cast<const uint8_t*>(&count),
                     reinterpret_cast<const uint8_t*>(&count) + sizeof(count));
        
        // Serialize each key-value pair
        for (const auto& [key, value] : values_) {
            // Key length and key
            uint32_t key_len = static_cast<uint32_t>(key.size());
            result.insert(result.end(),
                         reinterpret_cast<const uint8_t*>(&key_len),
                         reinterpret_cast<const uint8_t*>(&key_len) + sizeof(key_len));
            result.insert(result.end(), key.begin(), key.end());
            
            // Value serialization
            auto value_data = value.serialize();
            uint32_t value_len = static_cast<uint32_t>(value_data.size());
            result.insert(result.end(),
                         reinterpret_cast<const uint8_t*>(&value_len),
                         reinterpret_cast<const uint8_t*>(&value_len) + sizeof(value_len));
            result.insert(result.end(), value_data.begin(), value_data.end());
        }
        
        return result;
    }

    std::shared_ptr<thread_safe_container> thread_safe_container::deserialize(
        const std::vector<uint8_t>& data)
    {
        if (data.size() < sizeof(uint32_t)) {
            return nullptr;
        }
        
        auto container = std::make_shared<thread_safe_container>();
        size_t offset = 0;
        
        // Read number of entries
        uint32_t count;
        std::memcpy(&count, data.data() + offset, sizeof(count));
        offset += sizeof(count);
        
        // Read each key-value pair
        for (uint32_t i = 0; i < count; ++i) {
            if (offset + sizeof(uint32_t) > data.size()) {
                return nullptr;
            }
            
            // Read key
            uint32_t key_len;
            std::memcpy(&key_len, data.data() + offset, sizeof(key_len));
            offset += sizeof(key_len);
            
            if (offset + key_len + sizeof(uint32_t) > data.size()) {
                return nullptr;
            }
            
            std::string key(data.begin() + offset, data.begin() + offset + key_len);
            offset += key_len;
            
            // Read value length
            uint32_t value_len;
            std::memcpy(&value_len, data.data() + offset, sizeof(value_len));
            offset += sizeof(value_len);
            
            if (offset + value_len > data.size()) {
                return nullptr;
            }
            
            // Deserialize value
            std::vector<uint8_t> value_data(data.begin() + offset,
                                           data.begin() + offset + value_len);
            offset += value_len;
            
            auto value_opt = variant_value::deserialize(value_data);
            if (value_opt) {
                container->values_[key] = std::move(*value_opt);
            }
        }
        
        return container;
    }

    variant_value& thread_safe_container::operator[](const std::string& key)
    {
        std::unique_lock lock(mutex_);
        write_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = values_.find(key);
        if (it == values_.end()) {
            auto [new_it, inserted] = values_.emplace(key, variant_value(key));
            return new_it->second;
        }
        return it->second;
    }

    // lockfree_reader implementation
    void lockfree_reader::update_snapshot()
    {
        auto new_snapshot = std::make_shared<thread_safe_container::value_map>(
            container_->bulk_read([](const auto& map) { return map; })
        );
        
        std::unique_lock lock(snapshot_mutex_);
        snapshot_ = new_snapshot;
    }

} // namespace container_module