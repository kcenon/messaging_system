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

#include "container/internal/variant_value.h"
#include "container/internal/thread_safe_container.h"
#include "utilities/core/formatter.h"
#include <sstream>
#include <iomanip>

namespace container_module
{
    using namespace utility_module;

    variant_value::variant_value(const variant_value& other)
    {
        std::shared_lock other_lock(other.mutex_);
        name_ = other.name_;
        data_ = other.data_;
    }

    variant_value::variant_value(variant_value&& other) noexcept
    {
        std::unique_lock other_lock(other.mutex_);
        name_ = std::move(other.name_);
        data_ = std::move(other.data_);
        read_count_ = other.read_count_.load();
        write_count_ = other.write_count_.load();
        other.read_count_ = 0;
        other.write_count_ = 0;
    }

    variant_value& variant_value::operator=(const variant_value& other)
    {
        if (this != &other) {
            std::unique_lock lock(mutex_);
            std::shared_lock other_lock(other.mutex_);
            name_ = other.name_;
            data_ = other.data_;
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    variant_value& variant_value::operator=(variant_value&& other) noexcept
    {
        if (this != &other) {
            std::unique_lock lock(mutex_);
            std::unique_lock other_lock(other.mutex_);
            name_ = std::move(other.name_);
            data_ = std::move(other.data_);
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    std::string variant_value::to_string() const
    {
        return visit([](auto&& value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "null";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return value ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                std::ostringstream oss;
                oss << std::hex << std::setfill('0');
                for (auto byte : value) {
                    oss << std::setw(2) << static_cast<int>(byte);
                }
                return oss.str();
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(value);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                return value;
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
                return value ? value->to_json() : "null";
            }
            return "";
        });
    }

    std::string variant_value::to_json() const
    {
        auto var_name = name();  // Capture name before visit
        auto var_type_idx = type_index();  // Capture type_index before visit
        
        return visit([var_name, var_type_idx](auto&& value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            
            std::string result;
            formatter::format_to(std::back_inserter(result), 
                               "{{\"name\":\"{}\",\"type\":{},\"value\":", 
                               var_name, var_type_idx);
            
            if constexpr (std::is_same_v<T, std::monostate>) {
                result += "null";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                result += value ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                result += "\"";
                std::ostringstream oss;
                oss << std::hex << std::setfill('0');
                for (auto byte : value) {
                    oss << std::setw(2) << static_cast<int>(byte);
                }
                result += oss.str();
                result += "\"";
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                result += std::to_string(value);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                // Escape string for JSON
                result += "\"";
                for (char c : value) {
                    switch (c) {
                        case '"': result += "\\\""; break;
                        case '\\': result += "\\\\"; break;
                        case '\b': result += "\\b"; break;
                        case '\f': result += "\\f"; break;
                        case '\n': result += "\\n"; break;
                        case '\r': result += "\\r"; break;
                        case '\t': result += "\\t"; break;
                        default:
                            if (c >= 0x20 && c <= 0x7E) {
                                result += c;
                            } else {
                                formatter::format_to(std::back_inserter(result),
                                                   "\\u{:04x}", 
                                                   static_cast<unsigned>(c));
                            }
                    }
                }
                result += "\"";
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
                result += value ? value->to_json() : "null";
            }
            
            result += "}";
            return result;
        });
    }

    std::vector<uint8_t> variant_value::serialize() const
    {
        std::vector<uint8_t> result;
        
        // Header: name length (4 bytes) + name + type index (1 byte)
        uint32_t name_len = static_cast<uint32_t>(name_.size());
        result.insert(result.end(), 
                     reinterpret_cast<const uint8_t*>(&name_len),
                     reinterpret_cast<const uint8_t*>(&name_len) + sizeof(name_len));
        result.insert(result.end(), name_.begin(), name_.end());
        
        uint8_t type_idx = static_cast<uint8_t>(type_index());
        result.push_back(type_idx);
        
        // Serialize the variant data
        visit([&result](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            
            if constexpr (std::is_same_v<T, std::monostate>) {
                // Nothing to serialize
            }
            else if constexpr (std::is_same_v<T, bool>) {
                result.push_back(value ? 1 : 0);
            }
            else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                uint32_t size = static_cast<uint32_t>(value.size());
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&size),
                             reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                result.insert(result.end(), value.begin(), value.end());
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&value),
                             reinterpret_cast<const uint8_t*>(&value) + sizeof(T));
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                uint32_t size = static_cast<uint32_t>(value.size());
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&size),
                             reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                result.insert(result.end(), value.begin(), value.end());
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
                if (value) {
                    auto container_data = value->serialize();
                    uint32_t size = static_cast<uint32_t>(container_data.size());
                    result.insert(result.end(),
                                 reinterpret_cast<const uint8_t*>(&size),
                                 reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                    result.insert(result.end(), container_data.begin(), container_data.end());
                } else {
                    uint32_t size = 0;
                    result.insert(result.end(),
                                 reinterpret_cast<const uint8_t*>(&size),
                                 reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                }
            }
        });
        
        return result;
    }

    std::optional<variant_value> variant_value::deserialize(const std::vector<uint8_t>& data)
    {
        if (data.size() < sizeof(uint32_t) + 1) { // Minimum: name length + type
            return std::nullopt;
        }
        
        size_t offset = 0;
        
        // Read name length
        uint32_t name_len;
        std::memcpy(&name_len, data.data() + offset, sizeof(name_len));
        offset += sizeof(name_len);
        
        if (offset + name_len + 1 > data.size()) {
            return std::nullopt;
        }
        
        // Read name
        std::string name(data.begin() + offset, data.begin() + offset + name_len);
        offset += name_len;
        
        // Read type index
        uint8_t type_idx = data[offset++];
        
        // Create variant_value based on type
        variant_value result(name);
        
        // Deserialize based on type index
        switch (type_idx) {
            case 0: // std::monostate
                result.set(std::monostate{});
                break;
                
            case 1: // bool
                if (offset >= data.size()) return std::nullopt;
                result.set(data[offset] != 0);
                break;
                
            case 2: // std::vector<uint8_t>
            {
                if (offset + sizeof(uint32_t) > data.size()) return std::nullopt;
                uint32_t size;
                std::memcpy(&size, data.data() + offset, sizeof(size));
                offset += sizeof(size);
                if (offset + size > data.size()) return std::nullopt;
                std::vector<uint8_t> bytes(data.begin() + offset, 
                                          data.begin() + offset + size);
                result.set(std::move(bytes));
                break;
            }
            
            // Add cases for other numeric types...
            case 11: // float
            {
                if (offset + sizeof(float) > data.size()) return std::nullopt;
                float value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                result.set(value);
                break;
            }
            
            case 12: // double
            {
                if (offset + sizeof(double) > data.size()) return std::nullopt;
                double value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                result.set(value);
                break;
            }
            
            case 13: // std::string
            {
                if (offset + sizeof(uint32_t) > data.size()) return std::nullopt;
                uint32_t size;
                std::memcpy(&size, data.data() + offset, sizeof(size));
                offset += sizeof(size);
                if (offset + size > data.size()) return std::nullopt;
                std::string str(data.begin() + offset, data.begin() + offset + size);
                result.set(std::move(str));
                break;
            }
            
            // Container type would need special handling
            case 14: // std::shared_ptr<thread_safe_container>
                // TODO: Implement container deserialization
                break;
                
            default:
                return std::nullopt;
        }
        
        return result;
    }

    bool variant_value::operator==(const variant_value& other) const
    {
        std::shared_lock lock(mutex_);
        std::shared_lock other_lock(other.mutex_);
        return name_ == other.name_ && data_ == other.data_;
    }

    bool variant_value::operator!=(const variant_value& other) const
    {
        return !(*this == other);
    }

    bool variant_value::operator<(const variant_value& other) const
    {
        std::shared_lock lock(mutex_);
        std::shared_lock other_lock(other.mutex_);
        if (name_ != other.name_) {
            return name_ < other.name_;
        }
        return data_ < other.data_;
    }

} // namespace container_module