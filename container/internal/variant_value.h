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

#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <type_traits>

namespace container_module
{
    // Forward declaration
    class thread_safe_container;

    /**
     * @brief Type-safe variant for all possible value types in the container system
     * 
     * This replaces the runtime polymorphism with compile-time type safety using std::variant.
     * The order of types in the variant corresponds to the value_types enum for compatibility.
     * 
     * Note: On some platforms (like macOS), int64_t and long long are the same type,
     * so we use conditional types to avoid duplication.
     */
    #ifdef __APPLE__
        // On macOS, int64_t is typedef'd to long long, so we skip the explicit long long types
        using ValueVariant = std::variant<
            std::monostate,                       // null_value (index 0)
            bool,                                 // bool_value (index 1)
            std::vector<uint8_t>,                 // bytes_value (index 2)
            int16_t,                              // short_value (index 3)
            uint16_t,                             // ushort_value (index 4)
            int32_t,                              // int_value (index 5)
            uint32_t,                             // uint_value (index 6)
            int64_t,                              // long_value (index 7) - same as long long on macOS
            uint64_t,                             // ulong_value (index 8) - same as unsigned long long on macOS
            float,                                // float_value (index 9)
            double,                               // double_value (index 10)
            std::string,                          // string_value (index 11)
            std::shared_ptr<thread_safe_container> // container_value (index 12)
        >;
    #else
        using ValueVariant = std::variant<
            std::monostate,                       // null_value (index 0)
            bool,                                 // bool_value (index 1)
            std::vector<uint8_t>,                 // bytes_value (index 2)
            int16_t,                              // short_value (index 3)
            uint16_t,                             // ushort_value (index 4)
            int32_t,                              // int_value (index 5)
            uint32_t,                             // uint_value (index 6)
            int64_t,                              // long_value (index 7)
            uint64_t,                             // ulong_value (index 8)
            long long,                            // llong_value (index 9)
            unsigned long long,                   // ullong_value (index 10)
            float,                                // float_value (index 11)
            double,                               // double_value (index 12)
            std::string,                          // string_value (index 13)
            std::shared_ptr<thread_safe_container> // container_value (index 14)
        >;
    #endif

    /**
     * @brief Type-safe value wrapper with thread safety
     * 
     * This class provides a modern, type-safe alternative to the polymorphic value class.
     * It uses std::variant for type safety and includes thread synchronization primitives.
     */
    class variant_value
    {
    public:
        /**
         * @brief Default constructor - creates a null value
         */
        variant_value() : name_(""), data_(std::monostate{}) {}

        /**
         * @brief Construct with name and null value
         */
        explicit variant_value(std::string_view name) 
            : name_(name), data_(std::monostate{}) {}

        /**
         * @brief Construct with name and value
         */
        template<typename T>
        variant_value(std::string_view name, T&& value)
            : name_(name), data_(std::forward<T>(value)) {}

        /**
         * @brief Copy constructor
         */
        variant_value(const variant_value& other);

        /**
         * @brief Move constructor
         */
        variant_value(variant_value&& other) noexcept;

        /**
         * @brief Copy assignment
         */
        variant_value& operator=(const variant_value& other);

        /**
         * @brief Move assignment
         */
        variant_value& operator=(variant_value&& other) noexcept;

        /**
         * @brief Get the name of this value
         */
        std::string_view name() const {
            std::shared_lock lock(mutex_);
            return name_;
        }

        /**
         * @brief Set the name of this value
         */
        void set_name(std::string_view name) {
            std::unique_lock lock(mutex_);
            name_ = name;
        }

        /**
         * @brief Get the current type index (corresponds to value_types enum)
         */
        size_t type_index() const {
            std::shared_lock lock(mutex_);
            return data_.index();
        }

        /**
         * @brief Check if this is a null value
         */
        bool is_null() const {
            std::shared_lock lock(mutex_);
            return std::holds_alternative<std::monostate>(data_);
        }

        /**
         * @brief Type-safe getter with optional return
         * @tparam T The type to retrieve
         * @return std::optional containing the value if type matches, empty otherwise
         */
        template<typename T>
        std::optional<T> get() const {
            std::shared_lock lock(mutex_);
            if (auto* ptr = std::get_if<T>(&data_)) {
                return *ptr;
            }
            return std::nullopt;
        }

        /**
         * @brief Type-safe setter
         * @tparam T The type to set
         * @param value The value to store
         */
        template<typename T>
        void set(T&& value) {
            std::unique_lock lock(mutex_);
            data_ = std::forward<T>(value);
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Apply a visitor to the contained value
         * @tparam Visitor Callable object that can handle all variant types
         * @param vis The visitor to apply
         * @return The result of the visitor
         */
        template<typename Visitor>
        auto visit(Visitor&& vis) const {
            std::shared_lock lock(mutex_);
            read_count_.fetch_add(1, std::memory_order_relaxed);
            return std::visit(std::forward<Visitor>(vis), data_);
        }

        /**
         * @brief Apply a visitor to the contained value (mutable version)
         */
        template<typename Visitor>
        auto visit_mut(Visitor&& vis) {
            std::unique_lock lock(mutex_);
            write_count_.fetch_add(1, std::memory_order_relaxed);
            return std::visit(std::forward<Visitor>(vis), data_);
        }

        /**
         * @brief Convert to string representation
         */
        std::string to_string() const;

        /**
         * @brief Convert to JSON representation
         */
        std::string to_json() const;

        /**
         * @brief Serialize to binary format
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize from binary format
         */
        static std::optional<variant_value> deserialize(const std::vector<uint8_t>& data);

        /**
         * @brief Get read count (for statistics/debugging)
         */
        size_t read_count() const {
            return read_count_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Get write count (for statistics/debugging)
         */
        size_t write_count() const {
            return write_count_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Comparison operators
         */
        bool operator==(const variant_value& other) const;
        bool operator!=(const variant_value& other) const;
        bool operator<(const variant_value& other) const;

    private:
        std::string name_;
        ValueVariant data_;
        mutable std::shared_mutex mutex_;
        mutable std::atomic<size_t> read_count_{0};
        mutable std::atomic<size_t> write_count_{0};
    };

    /**
     * @brief Type traits for variant value types
     */
    template<typename T>
    struct is_variant_type : std::false_type {};

    template<>
    struct is_variant_type<std::monostate> : std::true_type {};
    
    template<>
    struct is_variant_type<bool> : std::true_type {};
    
    template<>
    struct is_variant_type<std::vector<uint8_t>> : std::true_type {};
    
    template<>
    struct is_variant_type<int16_t> : std::true_type {};
    
    template<>
    struct is_variant_type<uint16_t> : std::true_type {};
    
    template<>
    struct is_variant_type<int32_t> : std::true_type {};
    
    template<>
    struct is_variant_type<uint32_t> : std::true_type {};
    
    template<>
    struct is_variant_type<int64_t> : std::true_type {};

    template<>
    struct is_variant_type<uint64_t> : std::true_type {};

    // Only define these if they're different from int64_t/uint64_t
    // On Windows and some other platforms, long long == int64_t
    #if !defined(_WIN32) && !defined(_WIN64) && !defined(__APPLE__)
    #if !std::is_same_v<long long, int64_t>
    template<>
    struct is_variant_type<long long> : std::true_type {};
    #endif

    #if !std::is_same_v<unsigned long long, uint64_t>
    template<>
    struct is_variant_type<unsigned long long> : std::true_type {};
    #endif
    #endif
    
    template<>
    struct is_variant_type<float> : std::true_type {};
    
    template<>
    struct is_variant_type<double> : std::true_type {};
    
    template<>
    struct is_variant_type<std::string> : std::true_type {};
    
    template<>
    struct is_variant_type<std::shared_ptr<thread_safe_container>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_variant_type_v = is_variant_type<T>::value;

} // namespace container_module