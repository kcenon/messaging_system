#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/thread_worker.h>
#include <kcenon/thread/core/job_builder.h>

#include <thread>

// Local IExecutor adapter bridging thread_pool with common IExecutor interface.
// The upstream common_executor_adapter.h has a compile-time bug: its execute()
// and execute_delayed() capture unique_ptr by move into lambdas passed to
// std::function (which requires copyability). This local adapter fixes the
// issue by converting unique_ptr<IJob> to shared_ptr before lambda capture.
namespace {

class thread_pool_executor_adapter final
	: public kcenon::common::interfaces::IExecutor {
public:
	explicit thread_pool_executor_adapter(
		std::shared_ptr<kcenon::thread::thread_pool> pool)
		: pool_(std::move(pool)) {}

	kcenon::common::Result<std::future<void>> execute(
		std::unique_ptr<kcenon::common::interfaces::IJob>&& job) override {
		return enqueue_job(std::move(job), std::chrono::milliseconds::zero());
	}

	kcenon::common::Result<std::future<void>> execute_delayed(
		std::unique_ptr<kcenon::common::interfaces::IJob>&& job,
		std::chrono::milliseconds delay) override {
		return enqueue_job(std::move(job), delay);
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
			pool_->stop(!wait_for_completion);
		}
	}

private:
	kcenon::common::Result<std::future<void>> enqueue_job(
		std::unique_ptr<kcenon::common::interfaces::IJob>&& job,
		std::chrono::milliseconds delay) {
		if (!pool_) {
			return kcenon::common::make_error<std::future<void>>(
				-1, "Thread pool unavailable", "messaging_system");
		}

		auto promise_ptr = std::make_shared<std::promise<void>>();
		auto future = promise_ptr->get_future();

		// Convert unique_ptr to shared_ptr for copyable lambda capture
		auto shared_job = std::shared_ptr<kcenon::common::interfaces::IJob>(
			std::move(job));

		auto thread_job = kcenon::thread::make_job()
			.name(shared_job->get_name())
			.work([shared_job, promise_ptr, delay]()
				-> kcenon::common::VoidResult {
				try {
					if (delay.count() > 0) {
						std::this_thread::sleep_for(delay);
					}
					auto result = shared_job->execute();
					if (result.is_err()) {
						promise_ptr->set_exception(
							std::make_exception_ptr(
								std::runtime_error(
									result.error().message)));
						return result;
					}
					promise_ptr->set_value();
					return kcenon::common::ok();
				} catch (const std::exception& e) {
					promise_ptr->set_exception(
						std::current_exception());
					return kcenon::common::make_error<std::monostate>(
						-1, e.what(), "messaging_system");
				}
			})
			.build();

		auto enqueue_result = pool_->enqueue(std::move(thread_job));
		if (enqueue_result.is_err()) {
			return kcenon::common::make_error<std::future<void>>(
				-1,
				"Failed to enqueue job: " +
					enqueue_result.error().message,
				"messaging_system");
		}

		return kcenon::common::Result<std::future<void>>::ok(
			std::move(future));
	}

	std::shared_ptr<kcenon::thread::thread_pool> pool_;
};

} // anonymous namespace

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

	if (thread_pool_) {
		thread_pool_->stop();
		thread_pool_.reset();
	}

	common::logging::log_info("Standalone backend shutdown complete");
	return common::ok();
}

std::shared_ptr<common::interfaces::IExecutor> standalone_backend::get_executor() {
	if (!thread_pool_) {
		return nullptr;
	}
	return std::make_shared<thread_pool_executor_adapter>(thread_pool_);
}

bool standalone_backend::is_ready() const {
	return initialized_.load() && thread_pool_ && thread_pool_->is_running();
}

} // namespace kcenon::messaging
