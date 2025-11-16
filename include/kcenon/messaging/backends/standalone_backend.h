#pragma once

#include "backend_interface.h"
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace kcenon::messaging {

/**
 * @class standalone_backend
 * @brief Self-contained backend with internal thread pool
 *
 * Uses std::thread directly without external dependencies.
 * Provides a simple thread pool executor for async operations.
 */
class standalone_backend : public backend_interface {
public:
	/**
	 * @brief Constructor
	 * @param num_threads Number of worker threads (default: hardware concurrency)
	 */
	explicit standalone_backend(size_t num_threads = std::thread::hardware_concurrency());

	/**
	 * @brief Destructor - ensures proper shutdown
	 */
	~standalone_backend() override;

	// backend_interface implementation
	common::VoidResult initialize() override;
	common::VoidResult shutdown() override;
	std::shared_ptr<common::interfaces::IExecutor> get_executor() override;
	bool is_ready() const override;

private:
	class internal_thread_pool;
	class executor_adapter;

	size_t num_threads_;
	std::shared_ptr<internal_thread_pool> thread_pool_;
	std::shared_ptr<executor_adapter> executor_;
	std::atomic<bool> initialized_{false};
};

} // namespace kcenon::messaging
