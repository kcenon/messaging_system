// Suppress MSVC C4996 for gmtime in thread_system's execution_event.h
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>
#include <kcenon/thread/core/job.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/thread_worker.h>
#include <kcenon/thread/core/job_builder.h>

#include <future>
#include <thread>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace kcenon::messaging {

namespace {

// Wraps a common::interfaces::IJob as a thread::job for thread_pool enqueue
class executor_job_wrapper : public kcenon::thread::job {
public:
	executor_job_wrapper(std::unique_ptr<common::interfaces::IJob> wrapped,
						 std::shared_ptr<std::promise<void>> promise)
		: wrapped_(std::move(wrapped)), promise_(std::move(promise)) {}

protected:
	auto do_work() -> common::VoidResult override {
		try {
			auto result = wrapped_->execute();
			if (result.is_err()) {
				promise_->set_exception(
					std::make_exception_ptr(
						std::runtime_error(result.error().message)));
				return result;
			}
			promise_->set_value();
			return common::ok();
		} catch (...) {
			promise_->set_exception(std::current_exception());
			return common::make_error<std::monostate>(
				error::base,
				"Exception during job execution",
				"messaging_system"
			);
		}
	}

private:
	std::unique_ptr<common::interfaces::IJob> wrapped_;
	std::shared_ptr<std::promise<void>> promise_;
};

// Minimal IExecutor adapter for thread_pool (avoids broken common_executor_adapter.h
// which has ::common namespace mismatch with kcenon::common)
class thread_pool_executor : public common::interfaces::IExecutor {
public:
	explicit thread_pool_executor(std::shared_ptr<kcenon::thread::thread_pool> pool)
		: pool_(std::move(pool)) {}

	common::Result<std::future<void>> execute(
		std::unique_ptr<common::interfaces::IJob>&& job) override {
		auto promise = std::make_shared<std::promise<void>>();
		auto future = promise->get_future();

		auto wrapper = std::make_unique<executor_job_wrapper>(
			std::move(job), promise);

		auto result = pool_->enqueue(std::move(wrapper));
		if (result.is_err()) {
			return common::Result<std::future<void>>(result.error());
		}
		return common::Result<std::future<void>>::ok(std::move(future));
	}

	common::Result<std::future<void>> execute_delayed(
		std::unique_ptr<common::interfaces::IJob>&& job,
		std::chrono::milliseconds /*delay*/) override {
		// Simplified: immediate execution (delay support requires additional thread_pool API)
		return execute(std::move(job));
	}

	size_t worker_count() const override {
		return pool_ ? pool_->get_active_worker_count() : 0;
	}

	bool is_running() const override {
		return pool_ && pool_->is_running();
	}

	size_t pending_tasks() const override {
		return pool_ ? pool_->get_pending_task_count() : 0;
	}

	void shutdown(bool wait_for_completion) override {
		if (pool_) {
			[[maybe_unused]] auto _ = pool_->stop(!wait_for_completion);
		}
	}

private:
	std::shared_ptr<kcenon::thread::thread_pool> pool_;
};

} // anonymous namespace

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

		executor_ = std::make_shared<thread_pool_executor>(thread_pool_);

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
