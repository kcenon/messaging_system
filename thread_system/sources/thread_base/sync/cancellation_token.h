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
 * @file cancellation_token.h
 * @brief Implementation of a cancellation token for cooperative cancellation
 */

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace thread_module {

/**
 * @class cancellation_token
 * @brief Provides a mechanism for cooperative cancellation of operations
 *
 * Cancellation tokens allow long-running operations to be gracefully canceled.
 * They are particularly useful for worker threads that need to be notified
 * when their work should be aborted.
 */
class cancellation_token {
private:
    // Private implementation to allow copying/moving the token
    struct token_state {
        std::atomic<bool> is_cancelled{false};
        std::vector<std::function<void()>> callbacks;
        std::mutex callback_mutex;
    };
    
    std::shared_ptr<token_state> state_;
    
    // Private constructor that takes a state
    explicit cancellation_token(std::shared_ptr<token_state> state)
        : state_(std::move(state)) {}
    
public:
    // Default constructor creates a new token
    cancellation_token() : state_(std::make_shared<token_state>()) {}
    
    // Allow copy/move operations
    cancellation_token(const cancellation_token&) = default;
    cancellation_token& operator=(const cancellation_token&) = default;
    cancellation_token(cancellation_token&&) = default;
    cancellation_token& operator=(cancellation_token&&) = default;
    
    /**
     * @brief Creates a new cancellation token
     * @return A new cancellation token
     */
    static cancellation_token create() {
        return cancellation_token();
    }
    
    /**
     * @brief Creates a linked token that is canceled when any of the parent tokens are canceled
     * @param tokens The parent tokens
     * @return A new token linked to the parents
     * 
     * @note Uses weak_ptr to avoid circular references and memory leaks
     */
    static cancellation_token create_linked(std::initializer_list<cancellation_token> tokens) {
        auto new_token = create();
        auto new_state_weak = std::weak_ptr<token_state>(new_token.state_);
        
        for (const auto& token : tokens) {
            auto token_copy = token;
            token_copy.register_callback([new_state_weak]() {
                if (auto state = new_state_weak.lock()) {
                    // Directly set the cancelled flag and invoke callbacks
                    std::vector<std::function<void()>> callbacks_to_invoke;
                    
                    {
                        std::lock_guard<std::mutex> lock(state->callback_mutex);
                        bool was_cancelled = state->is_cancelled.exchange(true);
                        if (!was_cancelled) {
                            callbacks_to_invoke = std::move(state->callbacks);
                            state->callbacks.clear();
                        }
                    }
                    
                    for (const auto& callback : callbacks_to_invoke) {
                        callback();
                    }
                }
            });
        }
        
        return new_token;
    }
    
    /**
     * @brief Cancels the operation
     *
     * Sets the token to the canceled state and invokes all registered callbacks.
     * 
     * @note This method is thread-safe and guarantees callbacks are invoked exactly once.
     */
    void cancel() {
        std::vector<std::function<void()>> callbacks_to_invoke;
        
        {
            std::lock_guard<std::mutex> lock(state_->callback_mutex);
            bool was_cancelled = state_->is_cancelled.exchange(true);
            if (!was_cancelled) {
                // Move callbacks to local vector to avoid holding lock during invocation
                callbacks_to_invoke = std::move(state_->callbacks);
                state_->callbacks.clear();
            }
        }
        
        // Invoke callbacks outside the lock to prevent deadlock
        for (const auto& callback : callbacks_to_invoke) {
            callback();
        }
    }
    
    /**
     * @brief Checks if the token has been canceled
     * @return true if the token has been canceled, false otherwise
     */
    [[nodiscard]] bool is_cancelled() const {
        return state_->is_cancelled.load();
    }
    
    /**
     * @brief Throws an exception if the token has been canceled
     * @throws std::runtime_error if the token has been canceled
     */
    void throw_if_cancelled() const {
        if (is_cancelled()) {
            throw std::runtime_error("Operation cancelled");
        }
    }
    
    /**
     * @brief Registers a callback to be invoked when the token is canceled
     * @param callback The function to call when the token is canceled
     *
     * If the token is already canceled, the callback is invoked immediately.
     * 
     * @note This method is thread-safe and guarantees the callback is called exactly once.
     */
    void register_callback(std::function<void()> callback) {
        std::unique_lock<std::mutex> lock(state_->callback_mutex);
        
        // Check cancellation state while holding the lock
        if (state_->is_cancelled.load()) {
            lock.unlock();
            callback();
            return;
        }
        
        // Add callback while still holding the lock
        state_->callbacks.push_back(std::move(callback));
    }
};

} // namespace thread_module