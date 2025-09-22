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
 * @file template_helpers.h
 * @brief Template metaprogramming utilities for typed_thread_pool module
 * 
 * This file contains advanced template metaprogramming utilities, SFINAE helpers,
 * and compile-time utilities that support the typed thread pool implementation.
 */

#include "type_traits.h"
#include <type_traits>
#include <utility>
#include <tuple>
#include <functional>

namespace typed_thread_pool_module::detail {
    
    /**
     * @brief SFINAE helper to detect if a type has a specific member function
     */
    template<typename T, typename = void>
    struct has_priority_method : std::false_type {};
    
    template<typename T>
    struct has_priority_method<T, std::void_t<decltype(std::declval<T>().priority())>>
        : std::true_type {};
    
    template<typename T>
    constexpr bool has_priority_method_v = has_priority_method<T>::value;
    
    /**
     * @brief SFINAE helper to detect if a type has a do_work method
     */
    template<typename T, typename = void>
    struct has_do_work_method : std::false_type {};
    
    template<typename T>
    struct has_do_work_method<T, std::void_t<decltype(std::declval<T>().do_work())>>
        : std::true_type {};
    
    template<typename T>
    constexpr bool has_do_work_method_v = has_do_work_method<T>::value;
    
    /**
     * @brief Template to extract job type from job classes
     */
    template<typename Job>
    struct job_type_extractor {
        using type = void; // Default for non-job types
    };
    
    // Specialization for typed jobs
    template<typename T>
    struct job_type_extractor<T> {
        using type = std::conditional_t<
            has_priority_method_v<T>,
            decltype(std::declval<T>().priority()),
            void
        >;
    };
    
    template<typename Job>
    using job_type_t = typename job_type_extractor<Job>::type;
    
    /**
     * @brief Compile-time job validation
     */
    template<typename Job, typename ExpectedJobType>
    struct is_compatible_job : std::false_type {};
    
    template<typename Job, typename ExpectedJobType>
    struct is_compatible_job<Job, ExpectedJobType> {
        static constexpr bool value = 
            has_priority_method_v<Job> &&
            has_do_work_method_v<Job> &&
            std::is_same_v<job_type_t<Job>, ExpectedJobType>;
    };
    
    template<typename Job, typename ExpectedJobType>
    constexpr bool is_compatible_job_v = is_compatible_job<Job, ExpectedJobType>::value;
    
    /**
     * @brief Template helper for function signature detection
     */
    template<typename F>
    struct function_traits;
    
    // Specialization for function pointers
    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
    };
    
    // Specialization for member function pointers
    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)> {
        using return_type = R;
        using class_type = C;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
    };
    
    // Specialization for const member function pointers
    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        using return_type = R;
        using class_type = C;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
    };
    
    // Specialization for function objects and lambdas
    template<typename F>
    struct function_traits : function_traits<decltype(&F::operator())> {};
    
    /**
     * @brief Constexpr string utilities for compile-time job naming
     */
    template<size_t N>
    struct constexpr_string {
        char data[N];
        
        constexpr constexpr_string(const char (&str)[N]) {
            for (size_t i = 0; i < N; ++i) {
                data[i] = str[i];
            }
        }
        
        constexpr operator const char*() const { return data; }
        constexpr size_t size() const { return N - 1; } // Exclude null terminator
    };
    
    /**
     * @brief Template for automatic job naming based on function signature
     */
    template<typename F>
    constexpr auto generate_job_name() {
        // This would ideally use compile-time type name extraction
        // For now, return a generic name
        return "generated_job";
    }
    
    /**
     * @brief Priority comparison helper with custom comparison functions
     */
    template<JobType T, typename Comparator = std::less<>>
    struct priority_comparator {
        Comparator comp;
        
        constexpr priority_comparator(Comparator c = {}) : comp(c) {}
        
        constexpr bool operator()(T lhs, T rhs) const {
            using traits = job_type_traits<T>;
            return comp(traits::to_underlying(lhs), traits::to_underlying(rhs));
        }
    };
    
    /**
     * @brief Template for conditional compilation based on feature flags
     */
    template<bool Condition>
    struct conditional_feature {
        template<typename T>
        using type = std::conditional_t<Condition, T, void>;
    };
    
    /**
     * @brief Variadic template helper for job type lists
     */
    template<JobType... Types>
    struct job_type_list {
        static constexpr size_t size = sizeof...(Types);
        
        template<size_t Index>
        using at = std::tuple_element_t<Index, std::tuple<std::integral_constant<Types, Types>...>>;
        
        static constexpr std::array<std::common_type_t<Types...>, size> values = {Types...};
    };
    
    /**
     * @brief Type erasure helper for heterogeneous job storage
     */
    class job_eraser {
    public:
        template<typename Job>
        job_eraser(Job&& job) 
            : vtable_(&vtable_for<std::decay_t<Job>>)
            , storage_(std::forward<Job>(job)) {}
        
        void execute() {
            vtable_->execute(storage_);
        }
        
        void destroy() {
            vtable_->destroy(storage_);
        }
        
    private:
        struct vtable_t {
            void (*execute)(void*);
            void (*destroy)(void*);
        };
        
        template<typename Job>
        static constexpr vtable_t vtable_for = {
            [](void* ptr) { static_cast<Job*>(ptr)->do_work(); },
            [](void* ptr) { static_cast<Job*>(ptr)->~Job(); }
        };
        
        const vtable_t* vtable_;
        alignas(std::max_align_t) char storage_[64]; // Adjust size as needed
    };
    
    /**
     * @brief Perfect forwarding helper for job construction
     */
    template<typename JobType, typename... Args>
    constexpr auto make_job_args(Args&&... args) {
        return std::forward_as_tuple(std::forward<Args>(args)...);
    }
    
    /**
     * @brief Compile-time job validation with detailed error messages
     */
    template<typename Job, JobType ExpectedType>
    struct job_validator {
        static_assert(has_priority_method_v<Job>, 
                     "Job type must have a priority() method");
        static_assert(has_do_work_method_v<Job>, 
                     "Job type must have a do_work() method");
        static_assert(std::is_same_v<job_type_t<Job>, ExpectedType>, 
                     "Job priority type must match expected type");
        
        static constexpr bool value = true;
    };
    
} // namespace typed_thread_pool_module::detail