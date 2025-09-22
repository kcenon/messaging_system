#pragma once

/*
 * BSD 3-Clause License
 * 
 * Copyright (c) 2024, DongCheol Shin
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file type_traits.h
 * @brief Type traits and concepts for typed_thread_pool module
 * 
 * This file contains type traits, concepts, and compile-time utilities
 * that help ensure type safety and provide better error messages.
 */

#include <type_traits>
#include <concepts>
#include <string>

namespace typed_kcenon::thread::detail {
    
    /**
     * @brief Concept to validate job type parameters
     * 
     * A valid job type must be either an enumeration or an integral type
     * (excluding bool for clarity). This ensures job types can be used
     * for priority comparison and categorization.
     */
    template<typename T>
    concept JobType = std::is_enum_v<T> || 
                     (std::is_integral_v<T> && !std::is_same_v<T, bool>);
    
    /**
     * @brief Concept for callable job functions
     * 
     * Validates that a type can be used as a job callback function.
     */
    template<typename F>
    concept JobCallable = std::is_invocable_v<F> && 
                         (std::is_void_v<std::invoke_result_t<F>> ||
                          std::is_same_v<std::invoke_result_t<F>, bool> ||
                          std::is_convertible_v<std::invoke_result_t<F>, std::string>);
    
    /**
     * @brief Type traits for job types
     * 
     * Provides compile-time information about job type characteristics.
     */
    template<JobType T>
    struct job_type_traits {
        using type = T;
        using underlying_type = std::conditional_t<std::is_enum_v<T>, 
                                                  std::underlying_type_t<T>, 
                                                  T>;
        
        static constexpr bool is_enum = std::is_enum_v<T>;
        static constexpr bool is_integral = std::is_integral_v<T>;
        static constexpr bool has_ordering = true;
        static constexpr bool is_signed = std::is_signed_v<underlying_type>;
        
        /**
         * @brief Converts job type to its underlying representation
         */
        static constexpr underlying_type to_underlying(T value) noexcept {
            if constexpr (is_enum) {
                return static_cast<underlying_type>(value);
            } else {
                return value;
            }
        }
        
        /**
         * @brief Creates job type from underlying representation
         */
        static constexpr T from_underlying(underlying_type value) noexcept {
            if constexpr (is_enum) {
                return static_cast<T>(value);
            } else {
                return value;
            }
        }
    };
    
    /**
     * @brief SFINAE helper for optional job type validation
     */
    template<typename T, typename = void>
    struct is_valid_job_type : std::false_type {};
    
    template<typename T>
    struct is_valid_job_type<T, std::void_t<std::enable_if_t<JobType<T>>>> 
        : std::true_type {};
    
    template<typename T>
    constexpr bool is_valid_job_type_v = is_valid_job_type<T>::value;
    
    /**
     * @brief Helper to determine if a type can be used as a job priority
     */
    template<typename T>
    constexpr bool can_compare_priority() {
        if constexpr (JobType<T>) {
            using traits = job_type_traits<T>;
            return traits::has_ordering;
        }
        return false;
    }
    
    /**
     * @brief Compile-time priority comparison
     */
    template<JobType T>
    constexpr bool higher_priority(T lhs, T rhs) noexcept {
        using traits = job_type_traits<T>;
        
        // Assume lower numerical values = higher priority
        return traits::to_underlying(lhs) < traits::to_underlying(rhs);
    }
    
    /**
     * @brief Type alias for job type conversion
     */
    template<JobType T>
    using job_underlying_t = typename job_type_traits<T>::underlying_type;
    
} // namespace typed_kcenon::thread::detail