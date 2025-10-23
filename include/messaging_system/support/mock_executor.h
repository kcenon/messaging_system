#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>

namespace messaging::support {

/**
 * @brief Lightweight executor used for demos and tests when thread_system
 *        integration is unavailable or unstable.
 *
 * This executor spins up a small worker pool and exposes the
 * common::interfaces::IExecutor contract used throughout messaging_system.
 * It mirrors the behaviour relied upon in integration tests while keeping
 * the implementation self-contained.
 */
class MockExecutor : public common::interfaces::IExecutor {
public:
    explicit MockExecutor(size_t num_workers = 4)
        : num_workers_(num_workers ? num_workers : 1),
          running_(true),
          pending_count_(0) {
        workers_.reserve(num_workers_);
        for (size_t i = 0; i < num_workers_; ++i) {
            workers_.emplace_back([this] { work_loop(); });
        }
    }

    ~MockExecutor() override {
        shutdown(true);
    }

    std::future<void> submit(std::function<void()> task) override {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        if (!running_.load()) {
            promise->set_exception(std::make_exception_ptr(
                std::runtime_error("Executor is shutting down")));
            return future;
        }

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            tasks_.emplace([task = std::move(task), promise]() mutable {
                try {
                    task();
                    promise->set_value();
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            });
            ++pending_count_;
        }

        queue_cv_.notify_one();
        return future;
    }

    std::future<void> submit_delayed(
        std::function<void()> task,
        std::chrono::milliseconds delay) override {
        return std::async(std::launch::async, [this, task = std::move(task), delay]() mutable {
            std::this_thread::sleep_for(delay);
            auto future = submit(std::move(task));
            future.get();
        });
    }

    common::Result<std::future<void>> execute(
        std::unique_ptr<common::interfaces::IJob>&& job) override {
        if (!job) {
            return common::Result<std::future<void>>(
                common::error_info{1, "Job is null", "MockExecutor", ""});
        }

        auto shared_job = std::shared_ptr<common::interfaces::IJob>(std::move(job));
        auto task = [shared_job]() {
            (void)shared_job->execute();
        };

        return common::Result<std::future<void>>::ok(submit(std::move(task)));
    }

    common::Result<std::future<void>> execute_delayed(
        std::unique_ptr<common::interfaces::IJob>&& job,
        std::chrono::milliseconds delay) override {
        if (!job) {
            return common::Result<std::future<void>>(
                common::error_info{1, "Job is null", "MockExecutor", ""});
        }

        auto shared_job = std::shared_ptr<common::interfaces::IJob>(std::move(job));
        auto task = [shared_job]() {
            (void)shared_job->execute();
        };

        return common::Result<std::future<void>>::ok(
            submit_delayed(std::move(task), delay));
    }

    size_t worker_count() const override {
        return num_workers_;
    }

    bool is_running() const override {
        return running_;
    }

    size_t pending_tasks() const override {
        return pending_count_;
    }

    void shutdown(bool wait_for_completion = true) override {
        bool expected = true;
        if (!running_.compare_exchange_strong(expected, false)) {
            return;
        }

        if (wait_for_completion) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return tasks_.empty() && pending_count_.load() == 0;
            });
        }

        queue_cv_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers_.clear();
    }

private:
    void work_loop() {
        for (;;) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [this] {
                    return !tasks_.empty() || !running_;
                });

                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                } else if (!running_) {
                    break;
                }
            }

            if (task) {
                task();
                --pending_count_;
                if (tasks_.empty() && pending_count_.load() == 0) {
                    queue_cv_.notify_all();
                }
            }
        }
    }

    size_t num_workers_;
    std::atomic<bool> running_;
    std::atomic<size_t> pending_count_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
};

} // namespace messaging::support
