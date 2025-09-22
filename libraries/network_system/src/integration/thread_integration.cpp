/**
 * @file thread_integration.cpp
 * @brief Implementation of thread system integration
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include "network_system/integration/thread_integration.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>

namespace network_system::integration {

// basic_thread_pool implementation
class basic_thread_pool::impl {
public:
    impl(size_t num_threads)
        : running_(true), completed_tasks_(0) {

        if (num_threads == 0) {
            num_threads = std::thread::hardware_concurrency();
            if (num_threads == 0) num_threads = 2; // Fallback
        }

        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~impl() {
        stop(true);
    }

    std::future<void> submit(std::function<void()> task) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (!running_) {
                promise->set_exception(
                    std::make_exception_ptr(
                        std::runtime_error("Thread pool is not running")
                    )
                );
                return future;
            }

            tasks_.emplace([task, promise]() {
                try {
                    task();
                    promise->set_value();
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            });
        }

        condition_.notify_one();
        return future;
    }

    std::future<void> submit_delayed(
        std::function<void()> task,
        std::chrono::milliseconds delay
    ) {
        return submit([task, delay]() {
            std::this_thread::sleep_for(delay);
            task();
        });
    }

    size_t worker_count() const {
        return workers_.size();
    }

    bool is_running() const {
        return running_.load();
    }

    size_t pending_tasks() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

    void stop(bool wait_for_tasks) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (!wait_for_tasks) {
                while (!tasks_.empty()) {
                    tasks_.pop();
                }
            }
            running_ = false;
        }

        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        workers_.clear();
    }

    size_t get_completed_tasks() const {
        return completed_tasks_.load();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] {
                    return !running_ || !tasks_.empty();
                });

                if (!running_ && tasks_.empty()) {
                    return;
                }

                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }

            if (task) {
                task();
                completed_tasks_++;
            }
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> running_;
    std::atomic<size_t> completed_tasks_;
};

basic_thread_pool::basic_thread_pool(size_t num_threads)
    : pimpl_(std::make_unique<impl>(num_threads)) {
}

basic_thread_pool::~basic_thread_pool() = default;

std::future<void> basic_thread_pool::submit(std::function<void()> task) {
    return pimpl_->submit(task);
}

std::future<void> basic_thread_pool::submit_delayed(
    std::function<void()> task,
    std::chrono::milliseconds delay
) {
    return pimpl_->submit_delayed(task, delay);
}

size_t basic_thread_pool::worker_count() const {
    return pimpl_->worker_count();
}

bool basic_thread_pool::is_running() const {
    return pimpl_->is_running();
}

size_t basic_thread_pool::pending_tasks() const {
    return pimpl_->pending_tasks();
}

void basic_thread_pool::stop(bool wait_for_tasks) {
    pimpl_->stop(wait_for_tasks);
}

size_t basic_thread_pool::completed_tasks() const {
    return pimpl_->get_completed_tasks();
}

// thread_integration_manager implementation
class thread_integration_manager::impl {
public:
    impl() = default;

    void set_thread_pool(std::shared_ptr<thread_pool_interface> pool) {
        std::unique_lock<std::mutex> lock(mutex_);
        thread_pool_ = pool;
    }

    std::shared_ptr<thread_pool_interface> get_thread_pool() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!thread_pool_) {
            thread_pool_ = std::make_shared<basic_thread_pool>();
        }
        return thread_pool_;
    }

    std::future<void> submit_task(std::function<void()> task) {
        return get_thread_pool()->submit(task);
    }

    std::future<void> submit_delayed_task(
        std::function<void()> task,
        std::chrono::milliseconds delay
    ) {
        return get_thread_pool()->submit_delayed(task, delay);
    }

    thread_integration_manager::metrics get_metrics() const {
        std::unique_lock<std::mutex> lock(mutex_);

        metrics m;
        if (thread_pool_) {
            m.worker_threads = thread_pool_->worker_count();
            m.pending_tasks = thread_pool_->pending_tasks();
            m.is_running = thread_pool_->is_running();

            if (auto* basic = dynamic_cast<basic_thread_pool*>(thread_pool_.get())) {
                m.completed_tasks = basic->completed_tasks();
            }
        }

        return m;
    }

private:
    mutable std::mutex mutex_;
    std::shared_ptr<thread_pool_interface> thread_pool_;
};

thread_integration_manager& thread_integration_manager::instance() {
    static thread_integration_manager instance;
    return instance;
}

thread_integration_manager::thread_integration_manager()
    : pimpl_(std::make_unique<impl>()) {
}

thread_integration_manager::~thread_integration_manager() = default;

void thread_integration_manager::set_thread_pool(
    std::shared_ptr<thread_pool_interface> pool
) {
    pimpl_->set_thread_pool(pool);
}

std::shared_ptr<thread_pool_interface> thread_integration_manager::get_thread_pool() {
    return pimpl_->get_thread_pool();
}

std::future<void> thread_integration_manager::submit_task(
    std::function<void()> task
) {
    return pimpl_->submit_task(task);
}

std::future<void> thread_integration_manager::submit_delayed_task(
    std::function<void()> task,
    std::chrono::milliseconds delay
) {
    return pimpl_->submit_delayed_task(task, delay);
}

thread_integration_manager::metrics thread_integration_manager::get_metrics() const {
    return pimpl_->get_metrics();
}

} // namespace network_system::integration