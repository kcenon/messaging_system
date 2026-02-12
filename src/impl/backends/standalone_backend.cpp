#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>
#include <kcenon/thread/adapters/common_executor_adapter.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/thread_worker.h>

#include <thread>

namespace kcenon::messaging {

standalone_backend::standalone_backend(size_t num_threads)
	: num_threads_(num_threads > 0 ? num_threads : std::thread::hardware_concurrency())
	, thread_pool_(nullptr)
	, initialized_(false) {
	if (num_threads_ == 0) {
		num_threads_ = 1;
	}
}

standalone_backend::~standalone_backend() {
	if (initialized_.load()) {
		shutdown();
	}
}

common::VoidResult standalone_backend::initialize() {
	if (initialized_.exchange(true)) {
		common::logging::log_warning("Standalone backend already initialized");
		return common::make_error<std::monostate>(
			error::base,
			"Backend already initialized",
			"messaging_system"
		);
	}

	common::logging::log_info("Initializing standalone backend with " +
		std::to_string(num_threads_) + " threads (using thread_system)");

	try {
		thread_pool_ = std::make_shared<kcenon::thread::thread_pool>("messaging_standalone");

		for (size_t i = 0; i < num_threads_; ++i) {
			auto worker = std::make_unique<kcenon::thread::thread_worker>();
			worker->set_job_queue(thread_pool_->get_job_queue());
			auto result = thread_pool_->enqueue(std::move(worker));
			if (result.is_err()) {
				initialized_.store(false);
				thread_pool_.reset();
				return common::make_error<std::monostate>(
					error::base,
					"Failed to add worker to thread pool",
					"messaging_system"
				);
			}
		}

		auto start_result = thread_pool_->start();
		if (start_result.is_err()) {
			initialized_.store(false);
			thread_pool_.reset();
			common::logging::log_error("Failed to start thread pool");
			return common::make_error<std::monostate>(
				error::base,
				"Failed to start thread pool",
				"messaging_system"
			);
		}

		executor_ = std::make_shared<kcenon::thread::adapters::thread_pool_executor_adapter>(thread_pool_);

		common::logging::log_info("Standalone backend initialized successfully");
		return common::ok();
	} catch (const std::exception& e) {
		initialized_.store(false);
		thread_pool_.reset();
		common::logging::log_error("Failed to initialize standalone backend: " +
			std::string(e.what()));
		return common::make_error<std::monostate>(
			error::base,
			std::string("Failed to initialize backend: ") + e.what(),
			"messaging_system"
		);
	}
}

common::VoidResult standalone_backend::shutdown() {
	if (!initialized_.exchange(false)) {
		common::logging::log_debug("Standalone backend shutdown called but not initialized");
		return common::make_error<std::monostate>(
			error::base,
			"Backend not initialized",
			"messaging_system"
		);
	}

	common::logging::log_info("Shutting down standalone backend");

	executor_.reset();

	if (thread_pool_) {
		thread_pool_->stop();
		thread_pool_.reset();
	}

	common::logging::log_info("Standalone backend shutdown complete");
	return common::ok();
}

std::shared_ptr<common::interfaces::IExecutor> standalone_backend::get_executor() {
	return executor_;
}

bool standalone_backend::is_ready() const {
	return initialized_.load() && thread_pool_ && thread_pool_->is_running();
}

} // namespace kcenon::messaging
