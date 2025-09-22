#pragma once

#include <kcenon/thread/interfaces/shared_interfaces.h>
#include <kcenon/thread/core/thread_pool.h>
#include <memory>

namespace kcenon::thread::adapters {

/**
 * @brief Adapter to make thread_pool compatible with IExecutor interface
 */
class thread_pool_executor : public shared::IExecutor, public shared::IService {
public:
    /**
     * @brief Constructor with thread pool configuration
     * @param num_threads Number of threads in the pool
     */
    explicit thread_pool_executor(std::size_t num_threads = std::thread::hardware_concurrency())
        : num_threads_(num_threads) {
    }

    /**
     * @brief Constructor with existing thread pool
     * @param pool Existing thread pool
     */
    explicit thread_pool_executor(std::shared_ptr<thread_pool> pool)
        : thread_pool_(std::move(pool)), is_external_(true) {
        if (thread_pool_) {
            num_threads_ = thread_pool_->get_thread_count();
        }
    }

    // IExecutor interface
    std::future<void> execute(std::function<void()> task) override {
        if (!thread_pool_) {
            throw std::runtime_error("Thread pool not initialized");
        }

        return thread_pool_->submit(std::move(task));
    }

    std::size_t capacity() const override {
        return num_threads_;
    }

    std::size_t active_tasks() const override {
        if (thread_pool_) {
            return thread_pool_->get_active_thread_count();
        }
        return 0;
    }

    // IService interface
    bool initialize() override {
        if (!is_external_ && !thread_pool_) {
            thread_pool_ = std::make_shared<thread_pool>(num_threads_);
            is_running_ = true;
            return true;
        }
        return thread_pool_ != nullptr;
    }

    void shutdown() override {
        if (!is_external_ && thread_pool_) {
            thread_pool_->shutdown();
            thread_pool_.reset();
        }
        is_running_ = false;
    }

    bool is_running() const override {
        return is_running_ && thread_pool_ != nullptr;
    }

    std::string name() const override {
        return "ThreadPoolExecutor";
    }

    /**
     * @brief Get the underlying thread pool
     * @return Thread pool instance
     */
    std::shared_ptr<thread_pool> get_thread_pool() const {
        return thread_pool_;
    }

private:
    std::shared_ptr<thread_pool> thread_pool_;
    std::size_t num_threads_;
    bool is_external_{false};
    bool is_running_{false};
};

} // namespace kcenon::thread::adapters