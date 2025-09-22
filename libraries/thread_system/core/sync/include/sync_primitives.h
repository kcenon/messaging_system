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

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <memory>

namespace kcenon::thread::sync {
    /**
     * @brief RAII-based scoped lock guard with timeout support
     * 
     * This class provides automatic lock management with optional timeout
     * capabilities, ensuring locks are always released even in exceptional situations.
     */
    template<typename Mutex>
    class scoped_lock_guard {
    public:
        /**
         * @brief Construct and immediately acquire the lock
         * @param mutex The mutex to lock
         */
        explicit scoped_lock_guard(Mutex& mutex) 
            : mutex_(mutex), locked_(true) {
            mutex_.lock();
        }

        /**
         * @brief Construct and try to acquire the lock with timeout
         * @param mutex The mutex to lock
         * @param timeout Maximum time to wait for the lock
         */
        template<typename Rep, typename Period>
        scoped_lock_guard(Mutex& mutex, const std::chrono::duration<Rep, Period>& timeout)
            : mutex_(mutex), locked_(false) {
            if constexpr (std::is_same_v<Mutex, std::timed_mutex> || 
                         std::is_same_v<Mutex, std::recursive_timed_mutex>) {
                locked_ = mutex_.try_lock_for(timeout);
            } else {
                // Fallback for mutexes without timeout support
                locked_ = mutex_.try_lock();
            }
        }

        /**
         * @brief Destructor - automatically releases the lock if held
         */
        ~scoped_lock_guard() {
            if (locked_) {
                mutex_.unlock();
            }
        }

        // Non-copyable and non-movable
        scoped_lock_guard(const scoped_lock_guard&) = delete;
        scoped_lock_guard& operator=(const scoped_lock_guard&) = delete;
        scoped_lock_guard(scoped_lock_guard&&) = delete;
        scoped_lock_guard& operator=(scoped_lock_guard&&) = delete;

        /**
         * @brief Check if the lock was successfully acquired
         * @return true if lock is held, false otherwise
         */
        [[nodiscard]] bool owns_lock() const noexcept {
            return locked_;
        }

        /**
         * @brief Explicitly release the lock before destruction
         */
        void unlock() {
            if (locked_) {
                mutex_.unlock();
                locked_ = false;
            }
        }

    private:
        Mutex& mutex_;
        bool locked_;
    };

    /**
     * @brief Enhanced condition variable wrapper with timeout and predicate support
     */
    class condition_variable_wrapper {
    public:
        condition_variable_wrapper() = default;
        ~condition_variable_wrapper() = default;

        // Non-copyable but movable
        condition_variable_wrapper(const condition_variable_wrapper&) = delete;
        condition_variable_wrapper& operator=(const condition_variable_wrapper&) = delete;
        condition_variable_wrapper(condition_variable_wrapper&&) = default;
        condition_variable_wrapper& operator=(condition_variable_wrapper&&) = default;

        /**
         * @brief Wait indefinitely for notification
         * @param lock Unique lock that must be held by calling thread
         */
        template<typename Lock>
        void wait(Lock& lock) {
            cv_.wait(lock);
        }

        /**
         * @brief Wait with predicate until condition is met
         * @param lock Unique lock that must be held by calling thread
         * @param predicate Function that returns true when waiting should stop
         */
        template<typename Lock, typename Predicate>
        void wait(Lock& lock, Predicate predicate) {
            cv_.wait(lock, predicate);
        }

        /**
         * @brief Wait with timeout
         * @param lock Unique lock that must be held by calling thread
         * @param timeout Maximum time to wait
         * @return true if notified before timeout, false if timeout occurred
         */
        template<typename Lock, typename Rep, typename Period>
        bool wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& timeout) {
            return cv_.wait_for(lock, timeout) == std::cv_status::no_timeout;
        }

        /**
         * @brief Wait with timeout and predicate
         * @param lock Unique lock that must be held by calling thread
         * @param timeout Maximum time to wait
         * @param predicate Function that returns true when waiting should stop
         * @return true if predicate became true before timeout
         */
        template<typename Lock, typename Rep, typename Period, typename Predicate>
        bool wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& timeout, Predicate predicate) {
            return cv_.wait_for(lock, timeout, predicate);
        }

        /**
         * @brief Notify one waiting thread
         */
        void notify_one() noexcept {
            cv_.notify_one();
        }

        /**
         * @brief Notify all waiting threads
         */
        void notify_all() noexcept {
            cv_.notify_all();
        }

    private:
        std::condition_variable cv_;
    };

    /**
     * @brief Enhanced atomic flag with additional operations
     */
    class atomic_flag_wrapper {
    public:
        atomic_flag_wrapper() noexcept : flag_(ATOMIC_FLAG_INIT) {}
        ~atomic_flag_wrapper() = default;

        // Non-copyable but movable
        atomic_flag_wrapper(const atomic_flag_wrapper&) = delete;
        atomic_flag_wrapper& operator=(const atomic_flag_wrapper&) = delete;
        atomic_flag_wrapper(atomic_flag_wrapper&&) = default;
        atomic_flag_wrapper& operator=(atomic_flag_wrapper&&) = default;

        /**
         * @brief Test and set the flag atomically
         * @param order Memory ordering constraint
         * @return Previous value of the flag
         */
        bool test_and_set(std::memory_order order = std::memory_order_acq_rel) noexcept {
            return flag_.test_and_set(order);
        }

        /**
         * @brief Clear the flag atomically
         * @param order Memory ordering constraint
         */
        void clear(std::memory_order order = std::memory_order_release) noexcept {
            flag_.clear(order);
        }

        /**
         * @brief Test the flag without modifying it
         * @param order Memory ordering constraint
         * @return Current value of the flag
         */
        bool test(std::memory_order order = std::memory_order_acquire) const noexcept {
            return flag_.test(order);
        }

        /**
         * @brief Wait until the flag becomes false
         * @param order Memory ordering constraint
         */
        void wait(bool expected, std::memory_order order = std::memory_order_acquire) const noexcept {
            flag_.wait(expected, order);
        }

        /**
         * @brief Notify one thread waiting on this flag
         */
        void notify_one() noexcept {
            flag_.notify_one();
        }

        /**
         * @brief Notify all threads waiting on this flag
         */
        void notify_all() noexcept {
            flag_.notify_all();
        }

    private:
        std::atomic_flag flag_;
    };

    /**
     * @brief Shared mutex wrapper with reader-writer lock semantics
     */
    class shared_mutex_wrapper {
    public:
        shared_mutex_wrapper() = default;
        ~shared_mutex_wrapper() = default;

        // Non-copyable and non-movable
        shared_mutex_wrapper(const shared_mutex_wrapper&) = delete;
        shared_mutex_wrapper& operator=(const shared_mutex_wrapper&) = delete;
        shared_mutex_wrapper(shared_mutex_wrapper&&) = delete;
        shared_mutex_wrapper& operator=(shared_mutex_wrapper&&) = delete;

        /**
         * @brief Acquire exclusive (writer) lock
         */
        void lock() {
            mutex_.lock();
        }

        /**
         * @brief Try to acquire exclusive (writer) lock
         * @return true if lock was acquired, false otherwise
         */
        bool try_lock() {
            return mutex_.try_lock();
        }

        /**
         * @brief Release exclusive (writer) lock
         */
        void unlock() {
            mutex_.unlock();
        }

        /**
         * @brief Acquire shared (reader) lock
         */
        void lock_shared() {
            mutex_.lock_shared();
        }

        /**
         * @brief Try to acquire shared (reader) lock
         * @return true if lock was acquired, false otherwise
         */
        bool try_lock_shared() {
            return mutex_.try_lock_shared();
        }

        /**
         * @brief Release shared (reader) lock
         */
        void unlock_shared() {
            mutex_.unlock_shared();
        }

    private:
        std::shared_mutex mutex_;
    };

    // Convenience type aliases
    using unique_lock = std::unique_lock<std::mutex>;
    using shared_lock = std::shared_lock<shared_mutex_wrapper>;
    using scoped_mutex_lock = scoped_lock_guard<std::mutex>;
    using scoped_shared_mutex_lock = scoped_lock_guard<shared_mutex_wrapper>;
}