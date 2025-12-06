#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>

namespace kcenon::messaging {

// ============================================================================
// Simple job wrapper for std::function
// ============================================================================

class function_job : public common::interfaces::IJob {
public:
	explicit function_job(std::function<void()> func)
		: func_(std::move(func)) {}

	common::VoidResult execute() override {
		try {
			if (func_) {
				func_();
			}
			return common::ok();
		} catch (const std::exception& e) {
			return common::make_error<std::monostate>(
				error::base,
				e.what(),
				"messaging_system"
			);
		}
	}

	std::string get_name() const override {
		return "function_job";
	}

private:
	std::function<void()> func_;
};

// ============================================================================
// Internal thread pool (not IExecutor compliant)
// ============================================================================

class standalone_backend::internal_thread_pool {
public:
	explicit internal_thread_pool(size_t num_threads) {
		workers_.reserve(num_threads);
		for (size_t i = 0; i < num_threads; ++i) {
			workers_.emplace_back(&internal_thread_pool::worker_thread, this);
		}
	}

	~internal_thread_pool() {
		stop();
	}

	void submit(std::function<void()> task) {
		if (stop_flag_.load()) {
			return;
		}

		{
			std::lock_guard<std::mutex> lock(queue_mutex_);
			tasks_.push(std::move(task));
		}
		condition_.notify_one();
	}

	void stop() {
		{
			std::lock_guard<std::mutex> lock(queue_mutex_);
			if (stop_flag_.exchange(true)) {
				return; // Already stopped
			}
		}
		// notify_all must be called after releasing the lock to avoid
		// blocking workers that are waiting to acquire the lock
		condition_.notify_all();

		for (auto& worker : workers_) {
			if (worker.joinable()) {
				worker.join();
			}
		}
	}

	bool is_running() const {
		return !stop_flag_.load();
	}

	size_t worker_count() const {
		return workers_.size();
	}

private:
	void worker_thread() {
		while (true) {
			std::function<void()> task;

			{
				std::unique_lock<std::mutex> lock(queue_mutex_);
				condition_.wait(lock, [this] {
					return stop_flag_.load() || !tasks_.empty();
				});

				if (stop_flag_.load() && tasks_.empty()) {
					return;
				}

				if (!tasks_.empty()) {
					task = std::move(tasks_.front());
					tasks_.pop();
				}
			}

			if (task) {
				try {
					task();
				} catch (...) {
					// Swallow exceptions to prevent thread termination
				}
			}
		}
	}

	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;
	std::mutex queue_mutex_;
	std::condition_variable condition_;
	std::atomic<bool> stop_flag_{false};
};

// ============================================================================
// IExecutor adapter for internal thread pool
// ============================================================================

class standalone_backend::executor_adapter : public common::interfaces::IExecutor {
public:
	explicit executor_adapter(std::shared_ptr<internal_thread_pool> pool)
		: pool_(pool) {}

	common::Result<std::future<void>> execute(std::unique_ptr<common::interfaces::IJob>&& job) override {
		if (!pool_ || !pool_->is_running()) {
			return common::make_error<std::future<void>>(
				error::base,
				"Executor not running",
				"messaging_system"
			);
		}

		auto promise = std::make_shared<std::promise<void>>();
		auto future = promise->get_future();

		// Convert unique_ptr to shared_ptr to allow lambda copy
		auto shared_job = std::shared_ptr<common::interfaces::IJob>(std::move(job));

		pool_->submit([shared_job, promise]() {
			auto result = shared_job->execute();
			if (result.is_ok()) {
				promise->set_value();
			} else {
				try {
					throw std::runtime_error(std::string(result.error().message));
				} catch (...) {
					promise->set_exception(std::current_exception());
				}
			}
		});

		return common::ok(std::move(future));
	}

	common::Result<std::future<void>> execute_delayed(
		std::unique_ptr<common::interfaces::IJob>&& job,
		std::chrono::milliseconds delay) override {

		// Simple delayed execution - not optimal but works for standalone mode
		auto promise = std::make_shared<std::promise<void>>();
		auto future = promise->get_future();

		// Convert unique_ptr to shared_ptr to allow lambda copy
		auto shared_job = std::shared_ptr<common::interfaces::IJob>(std::move(job));

		pool_->submit([shared_job, promise, delay]() {
			std::this_thread::sleep_for(delay);
			auto result = shared_job->execute();
			if (result.is_ok()) {
				promise->set_value();
			} else {
				try {
					throw std::runtime_error(std::string(result.error().message));
				} catch (...) {
					promise->set_exception(std::current_exception());
				}
			}
		});

		return common::ok(std::move(future));
	}

	size_t worker_count() const override {
		return pool_ ? pool_->worker_count() : 0;
	}

	size_t pending_tasks() const override {
		// Not tracked in simple implementation
		return 0;
	}

	bool is_running() const override {
		return pool_ && pool_->is_running();
	}

	void shutdown(bool wait_for_completion = true) override {
		if (pool_) {
			pool_->stop();
		}
	}

private:
	std::shared_ptr<internal_thread_pool> pool_;
};

// ============================================================================
// standalone_backend implementation
// ============================================================================

standalone_backend::standalone_backend(size_t num_threads)
	: num_threads_(num_threads > 0 ? num_threads : 1)
	, thread_pool_(nullptr)
	, executor_(nullptr)
	, initialized_(false) {
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
		std::to_string(num_threads_) + " threads");

	try {
		thread_pool_ = std::shared_ptr<internal_thread_pool>(
			new internal_thread_pool(num_threads_)
		);
		executor_ = std::shared_ptr<executor_adapter>(
			new executor_adapter(thread_pool_)
		);
		common::logging::log_info("Standalone backend initialized successfully");
		return common::ok();
	} catch (const std::exception& e) {
		initialized_.store(false);
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

	if (thread_pool_) {
		thread_pool_->stop();
		thread_pool_.reset();
	}
	executor_.reset();

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
