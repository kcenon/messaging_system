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
 * @file pool_traits.h
 * @brief Type traits and metaprogramming utilities for thread pool
 * 
 * This file contains type traits, concepts, and compile-time utilities
 * that help ensure type safety and provide better error messages.
 */

#include <type_traits>
#include <concepts>
#include <functional>
#include <chrono>

namespace kcenon::thread::detail {
    
    /**
     * @brief Concept to validate callable types for thread pool jobs
     */
    template<typename F>
    concept Callable = std::is_invocable_v<F>;
    
    /**
     * @brief Concept for callable types that return void
     */
    template<typename F>
    concept VoidCallable = Callable<F> && std::is_void_v<std::invoke_result_t<F>>;
    
    /**
     * @brief Concept for callable types that return a result
     */
    template<typename F>
    concept ReturningCallable = Callable<F> && !std::is_void_v<std::invoke_result_t<F>>;
    
    /**
     * @brief Concept for callables with specific argument types
     */
    template<typename F, typename... Args>
    concept CallableWith = std::is_invocable_v<F, Args...>;
    
    /**
     * @brief Type trait to detect if a type is a duration
     */
    template<typename T>
    struct is_duration : std::false_type {};
    
    template<typename Rep, typename Period>
    struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};
    
    template<typename T>
    constexpr bool is_duration_v = is_duration<T>::value;
    
    /**
     * @brief Concept for duration types
     */
    template<typename T>
    concept Duration = is_duration_v<T>;
    
    /**
     * @brief Type trait to extract return type from callable
     */
    template<typename F>
    struct callable_return_type {
        using type = std::invoke_result_t<F>;
    };
    
    template<typename F>
    using callable_return_type_t = typename callable_return_type<F>::type;
    
    /**
     * @brief Type trait to check if a callable is noexcept
     */
    template<typename F>
    struct is_nothrow_callable : std::bool_constant<std::is_nothrow_invocable_v<F>> {};
    
    template<typename F>
    constexpr bool is_nothrow_callable_v = is_nothrow_callable<F>::value;
    
    /**
     * @brief SFINAE helper to detect if a type has a specific member
     */
    template<typename T, typename = void>
    struct has_get_method : std::false_type {};
    
    template<typename T>
    struct has_get_method<T, std::void_t<decltype(std::declval<T>().get())>> 
        : std::true_type {};
    
    template<typename T>
    constexpr bool has_get_method_v = has_get_method<T>::value;
    
    /**
     * @brief SFINAE helper to detect future-like types
     */
    template<typename T>
    struct is_future_like : std::bool_constant<
        has_get_method_v<T> && 
        std::is_same_v<decltype(std::declval<T>().wait()), void>
    > {};
    
    template<typename T>
    constexpr bool is_future_like_v = is_future_like<T>::value;
    
    /**
     * @brief Concept for future-like types
     */
    template<typename T>
    concept FutureLike = is_future_like_v<T>;
    
    /**
     * @brief Type trait for function signature analysis
     */
    template<typename F>
    struct function_traits;
    
    // Specialization for function pointers
    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
        static constexpr bool is_noexcept = false;
    };
    
    // Specialization for noexcept function pointers
    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...) noexcept> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
        static constexpr bool is_noexcept = true;
    };
    
    // Specialization for member function pointers
    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)> {
        using return_type = R;
        using class_type = C;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
        static constexpr bool is_noexcept = false;
    };
    
    // Specialization for const member function pointers
    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        using return_type = R;
        using class_type = C;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
        static constexpr bool is_noexcept = false;
    };
    
    // Specialization for function objects and lambdas
    template<typename F>
    struct function_traits : function_traits<decltype(&F::operator())> {};
    
    /**
     * @brief Helper to get function traits
     */
    template<typename F>
    using function_return_t = typename function_traits<F>::return_type;
    
    template<typename F>
    using function_args_t = typename function_traits<F>::argument_types;
    
    template<typename F>
    constexpr size_t function_arity_v = function_traits<F>::arity;
    
    /**
     * @brief Compile-time validation for thread pool configuration
     */
    template<size_t ThreadCount>
    struct validate_thread_count {
        static_assert(ThreadCount > 0, "Thread count must be positive");
        static_assert(ThreadCount <= 1024, "Thread count is unreasonably high");
        static constexpr bool value = true;
    };
    
    /**
     * @brief Template helper for perfect forwarding with type constraints
     */
    template<typename T>
    constexpr auto forward_if_callable(T&& t) -> std::enable_if_t<Callable<T>, T&&> {
        return std::forward<T>(t);
    }
    
    /**
     * @brief Type eraser for heterogeneous callable storage
     */
    class callable_eraser {
    public:
        template<typename F>
        callable_eraser(F&& f) 
            requires Callable<F>
            : vtable_(&vtable_for<std::decay_t<F>>)
            , storage_(std::forward<F>(f)) {}
        
        void operator()() {
            vtable_->invoke(storage_);
        }
        
    private:
        struct vtable_t {
            void (*invoke)(void*);
            void (*destroy)(void*);
        };
        
        template<typename F>
        static constexpr vtable_t vtable_for = {
            [](void* ptr) { (*static_cast<F*>(ptr))(); },
            [](void* ptr) { static_cast<F*>(ptr)->~F(); }
        };
        
        const vtable_t* vtable_;
        alignas(std::max_align_t) char storage_[64];
    };
    
    /**
     * @brief Concept for valid thread pool job types
     */
    template<typename Job>
    concept PoolJob = Callable<Job> && (
        VoidCallable<Job> || 
        std::is_convertible_v<callable_return_type_t<Job>, bool>
    );
    
    /**
     * @brief Compile-time string for template error messages
     */
    template<size_t N>
    struct compile_string {
        constexpr compile_string(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        char value[N];
    };
    
    /**
     * @brief Template for generating descriptive error messages
     */
    template<typename T>
    constexpr auto get_type_name() {
        // This would ideally use std::source_location or compiler intrinsics
        // For now, return a generic message
        return "unknown_type";
    }
    
} // namespace kcenon::thread::detail