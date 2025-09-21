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

#include <memory>
#include <atomic>

#ifdef USE_STD_JTHREAD
#include <thread>
#include <stop_token>
#else
#include <thread>
#endif

namespace thread_module::detail {
    /**
     * @brief Thread implementation abstraction that handles differences between std::jthread and std::thread
     * 
     * This class encapsulates the conditional compilation complexity for thread management,
     * providing a unified interface regardless of C++20 jthread availability.
     */
    class thread_impl {
    public:
        #ifdef USE_STD_JTHREAD
            using thread_type = std::jthread;
            using stop_token_type = std::stop_token;
            using stop_source_type = std::stop_source;
        #else
            using thread_type = std::thread;
            using stop_token_type = std::atomic<bool>;
            using stop_source_type = std::atomic<bool>;
        #endif

        thread_impl() = default;
        ~thread_impl() = default;

        thread_impl(const thread_impl&) = delete;
        thread_impl& operator=(const thread_impl&) = delete;
        thread_impl(thread_impl&&) = default;
        thread_impl& operator=(thread_impl&&) = default;

        /**
         * @brief Create and start a thread with the given function
         */
        template<typename F>
        void start_thread(F&& func) {
            #ifdef USE_STD_JTHREAD
                stop_source_ = std::make_unique<stop_source_type>();
                thread_ = std::make_unique<thread_type>([func = std::forward<F>(func), this](std::stop_token st) {
                    func(st);
                });
            #else
                stop_requested_ = std::make_unique<stop_token_type>(false);
                thread_ = std::make_unique<thread_type>([func = std::forward<F>(func), this]() {
                    func(*stop_requested_);
                });
            #endif
        }

        /**
         * @brief Request thread to stop
         */
        void request_stop() {
            #ifdef USE_STD_JTHREAD
                if (stop_source_) {
                    stop_source_->request_stop();
                }
            #else
                if (stop_requested_) {
                    stop_requested_->store(true);
                }
            #endif
        }

        /**
         * @brief Check if stop has been requested
         */
        [[nodiscard]] bool stop_requested() const {
            #ifdef USE_STD_JTHREAD
                return stop_source_ && stop_source_->stop_requested();
            #else
                return stop_requested_ && stop_requested_->load();
            #endif
        }

        /**
         * @brief Join the thread
         */
        void join() {
            if (thread_ && thread_->joinable()) {
                thread_->join();
            }
        }

        /**
         * @brief Detach the thread
         */
        void detach() {
            if (thread_ && thread_->joinable()) {
                thread_->detach();
            }
        }

        /**
         * @brief Check if thread is joinable
         */
        [[nodiscard]] bool joinable() const {
            return thread_ && thread_->joinable();
        }

    private:
        std::unique_ptr<thread_type> thread_;
        
        #ifdef USE_STD_JTHREAD
            std::unique_ptr<stop_source_type> stop_source_;
        #else
            std::unique_ptr<stop_token_type> stop_requested_;
        #endif
    };
}