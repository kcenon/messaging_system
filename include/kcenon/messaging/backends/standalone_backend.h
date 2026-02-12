#pragma once

#include "backend_interface.h"
#include <atomic>
#include <cstddef>
#include <memory>

namespace kcenon::thread {
class thread_pool;
} // namespace kcenon::thread

namespace kcenon::messaging {

/**
 * @class standalone_backend
 * @brief Backend that uses thread_system for async operations
 *
 * Provides a self-contained backend using thread_system's thread_pool.
 * This backend owns and manages its own thread pool instance.
 *
 * @note This backend uses thread_system as a runtime dependency.
 *       For environments without thread_system, use integration_backend
 *       with an externally provided executor.
 */
class standalone_backend : public backend_interface {
public:
	/**
	 * @brief Constructor
	 * @param num_threads Number of worker threads (default: hardware concurrency)
	 */
	explicit standalone_backend(size_t num_threads = 0);

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
	size_t num_threads_;
	std::shared_ptr<kcenon::thread::thread_pool> thread_pool_;
	std::shared_ptr<common::interfaces::IExecutor> executor_;
	std::atomic<bool> initialized_{false};
};

} // namespace kcenon::messaging
