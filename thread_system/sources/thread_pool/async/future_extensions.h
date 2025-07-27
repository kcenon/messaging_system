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
 * @file future_extensions.h
 * @brief Future and promise utilities for thread pool
 * 
 * This file provides extensions to std::future and std::promise
 * that integrate well with the thread pool system.
 */

#include "../detail/forward_declarations.h"
#include "../../thread_base/sync/error_handling.h"
#include <future>
#include <memory>
#include <chrono>
#include <functional>

namespace thread_pool_module {
    
    using namespace thread_module; // For result<T> types
    
    /**
     * @brief A future that can be scheduled on a thread pool
     * 
     * This class extends std::future with thread pool integration
     * and provides additional utilities for async operations.
     */
    template<typename T>
    class pool_future {
    public:
        using value_type = T;
        
        /**
         * @brief Construct from a std::future
         */
        explicit pool_future(std::future<T>&& future) 
            : future_(std::move(future)) {}
        
        /**
         * @brief Check if the future is ready
         */
        bool is_ready() const {
            return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }
        
        /**
         * @brief Wait for the result
         */
        T get() {
            return future_.get();
        }
        
        /**
         * @brief Wait for the result with timeout
         */
        template<typename Rep, typename Period>
        std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout) {
            return future_.wait_for(timeout);
        }
        
        /**
         * @brief Wait until the result is available
         */
        void wait() {
            future_.wait();
        }
        
        /**
         * @brief Check if the future is valid
         */
        bool valid() const {
            return future_.valid();
        }
        
        /**
         * @brief Chain another operation after this future completes
         */
        template<typename F>
        auto then(F&& continuation) -> pool_future<std::invoke_result_t<F, T>>;
        
    private:
        std::future<T> future_;
    };
    
    /**
     * @brief A promise that can schedule work on a thread pool
     */
    template<typename T>
    class pool_promise {
    public:
        using value_type = T;
        
        /**
         * @brief Default constructor
         */
        pool_promise() = default;
        
        /**
         * @brief Get the associated future
         */
        pool_future<T> get_future() {
            return pool_future<T>{promise_.get_future()};
        }
        
        /**
         * @brief Set the value
         */
        void set_value(const T& value) {
            promise_.set_value(value);
        }
        
        /**
         * @brief Set the value (move)
         */
        void set_value(T&& value) {
            promise_.set_value(std::move(value));
        }
        
        /**
         * @brief Set an exception
         */
        void set_exception(std::exception_ptr exception) {
            promise_.set_exception(exception);
        }
        
        /**
         * @brief Set the value at thread exit
         */
        void set_value_at_thread_exit(const T& value) {
            promise_.set_value_at_thread_exit(value);
        }
        
        /**
         * @brief Set an exception at thread exit
         */
        void set_exception_at_thread_exit(std::exception_ptr exception) {
            promise_.set_exception_at_thread_exit(exception);
        }
        
    private:
        std::promise<T> promise_;
    };
    
    /**
     * @brief Specialization for void type
     */
    template<>
    class pool_promise<void> {
    public:
        pool_future<void> get_future() {
            return pool_future<void>{promise_.get_future()};
        }
        
        void set_value() {
            promise_.set_value();
        }
        
        void set_exception(std::exception_ptr exception) {
            promise_.set_exception(exception);
        }
        
        void set_value_at_thread_exit() {
            promise_.set_value_at_thread_exit();
        }
        
        void set_exception_at_thread_exit(std::exception_ptr exception) {
            promise_.set_exception_at_thread_exit(exception);
        }
        
    private:
        std::promise<void> promise_;
    };
    
    /**
     * @brief Create a future that is already ready with a value
     */
    template<typename T>
    pool_future<std::decay_t<T>> make_ready_future(T&& value) {
        pool_promise<std::decay_t<T>> promise;
        auto future = promise.get_future();
        promise.set_value(std::forward<T>(value));
        return future;
    }
    
    /**
     * @brief Create a future that is already ready with void
     */
    inline pool_future<void> make_ready_future() {
        pool_promise<void> promise;
        auto future = promise.get_future();
        promise.set_value();
        return future;
    }
    
    /**
     * @brief Create a future that is already ready with an exception
     */
    template<typename T>
    pool_future<T> make_exceptional_future(std::exception_ptr exception) {
        pool_promise<T> promise;
        auto future = promise.get_future();
        promise.set_exception(exception);
        return future;
    }
    
    /**
     * @brief Wait for all futures to complete
     */
    template<typename... Futures>
    void wait_for_all(Futures&... futures) {
        (futures.wait(), ...);
    }
    
    /**
     * @brief Wait for any future to complete
     */
    template<typename... Futures>
    size_t wait_for_any(Futures&... futures) {
        // Implementation would require a more complex approach
        // This is a simplified version
        size_t index = 0;
        auto check_future = [&](auto& future) {
            if (future.is_ready()) {
                return true;
            }
            ++index;
            return false;
        };
        
        while (true) {
            index = 0;
            if ((check_future(futures) || ...)) {
                return index;
            }
            std::this_thread::yield();
        }
    }
    
} // namespace thread_pool_module