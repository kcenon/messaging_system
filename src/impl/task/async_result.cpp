// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/async_result.h>

#include <thread>

namespace kcenon::messaging::task {

// Simple IJob wrapper for lambda functions
class callback_monitor_job : public common::interfaces::IJob {
public:
	explicit callback_monitor_job(std::function<void()> func)
		: func_(std::move(func)) {
	}

	common::VoidResult execute() override {
		func_();
		return common::ok();
	}

	std::string get_name() const override { return "async_result_callback_monitor"; }
	int get_priority() const override { return 0; }

private:
	std::function<void()> func_;
};

async_result::async_result(
	std::string task_id,
	std::shared_ptr<result_backend_interface> backend,
	std::shared_ptr<common::interfaces::IExecutor> executor)
	: task_id_(std::move(task_id))
	, backend_(std::move(backend))
	, executor_(std::move(executor)) {
}

async_result::async_result()
	: task_id_()
	, backend_(nullptr)
	, executor_(nullptr) {
}

async_result::async_result(const async_result& other)
	: task_id_(other.task_id_)
	, backend_(other.backend_)
	, executor_(other.executor_)
	, callback_invoked_(other.callback_invoked_.load())
	, callback_monitor_started_(false) {
	std::lock_guard<std::mutex> lock(other.mutex_);
	success_callback_ = other.success_callback_;
	failure_callback_ = other.failure_callback_;
	child_task_ids_ = other.child_task_ids_;
}

async_result::async_result(async_result&& other) noexcept
	: task_id_(std::move(other.task_id_))
	, backend_(std::move(other.backend_))
	, executor_(std::move(other.executor_))
	, callback_invoked_(other.callback_invoked_.load())
	, callback_monitor_started_(other.callback_monitor_started_.load()) {
	std::lock_guard<std::mutex> lock(other.mutex_);
	success_callback_ = std::move(other.success_callback_);
	failure_callback_ = std::move(other.failure_callback_);
	child_task_ids_ = std::move(other.child_task_ids_);
}

async_result& async_result::operator=(const async_result& other) {
	if (this != &other) {
		std::lock_guard<std::mutex> lock1(mutex_);
		std::lock_guard<std::mutex> lock2(other.mutex_);
		task_id_ = other.task_id_;
		backend_ = other.backend_;
		executor_ = other.executor_;
		success_callback_ = other.success_callback_;
		failure_callback_ = other.failure_callback_;
		callback_invoked_ = other.callback_invoked_.load();
		callback_monitor_started_ = false;
		child_task_ids_ = other.child_task_ids_;
	}
	return *this;
}

async_result& async_result::operator=(async_result&& other) noexcept {
	if (this != &other) {
		std::lock_guard<std::mutex> lock1(mutex_);
		std::lock_guard<std::mutex> lock2(other.mutex_);
		task_id_ = std::move(other.task_id_);
		backend_ = std::move(other.backend_);
		executor_ = std::move(other.executor_);
		success_callback_ = std::move(other.success_callback_);
		failure_callback_ = std::move(other.failure_callback_);
		callback_invoked_ = other.callback_invoked_.load();
		callback_monitor_started_ = other.callback_monitor_started_.load();
		child_task_ids_ = std::move(other.child_task_ids_);
	}
	return *this;
}

task_state async_result::state() const {
	if (!is_valid()) {
		return task_state::pending;
	}

	auto result = backend_->get_state(task_id_);
	if (result.is_ok()) {
		return result.value();
	}
	return task_state::pending;
}

bool async_result::is_ready() const {
	auto current_state = state();
	return current_state == task_state::succeeded
		|| current_state == task_state::failed
		|| current_state == task_state::cancelled
		|| current_state == task_state::expired;
}

bool async_result::is_successful() const {
	return state() == task_state::succeeded;
}

bool async_result::is_failed() const {
	return state() == task_state::failed;
}

bool async_result::is_cancelled() const {
	return state() == task_state::cancelled;
}

double async_result::progress() const {
	if (!is_valid()) {
		return 0.0;
	}

	auto result = backend_->get_progress(task_id_);
	if (result.is_ok()) {
		return result.value().progress;
	}
	return 0.0;
}

std::string async_result::progress_message() const {
	if (!is_valid()) {
		return "";
	}

	auto result = backend_->get_progress(task_id_);
	if (result.is_ok()) {
		return result.value().message;
	}
	return "";
}

common::Result<container_module::value_container> async_result::get(
	std::chrono::milliseconds timeout) {
	if (!is_valid()) {
		return common::Result<container_module::value_container>(
			common::error_info{-1, "Invalid async_result handle"});
	}

	return backend_->wait_for_result(task_id_, timeout);
}

bool async_result::wait(std::chrono::milliseconds timeout) {
	if (!is_valid()) {
		return false;
	}

	auto start = std::chrono::steady_clock::now();
	auto remaining = timeout;
	const auto poll_interval = std::chrono::milliseconds(100);

	while (!is_ready()) {
		if (timeout != std::chrono::milliseconds::max()) {
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - start);
			if (elapsed >= timeout) {
				return false;
			}
			remaining = timeout - elapsed;
		}

		std::this_thread::sleep_for(
			std::min(poll_interval, remaining));
	}

	return true;
}

void async_result::then(
	std::function<void(const container_module::value_container&)> on_success,
	std::function<void(const std::string&)> on_failure) {
	if (!is_valid()) {
		if (on_failure) {
			on_failure("Invalid async_result handle");
		}
		return;
	}

	{
		std::lock_guard<std::mutex> lock(mutex_);
		success_callback_ = std::move(on_success);
		failure_callback_ = std::move(on_failure);
	}

	// Check if already complete
	if (is_ready()) {
		invoke_callbacks();
	} else {
		start_callback_monitor();
	}
}

common::VoidResult async_result::revoke() {
	if (!is_valid()) {
		return common::VoidResult(
			common::error_info{-1, "Invalid async_result handle"});
	}

	return backend_->store_state(task_id_, task_state::cancelled);
}

std::vector<async_result> async_result::children() const {
	std::lock_guard<std::mutex> lock(mutex_);
	std::vector<async_result> results;
	results.reserve(child_task_ids_.size());
	for (const auto& child_id : child_task_ids_) {
		results.emplace_back(child_id, backend_);
	}
	return results;
}

void async_result::add_child(const std::string& child_task_id) {
	std::lock_guard<std::mutex> lock(mutex_);
	child_task_ids_.push_back(child_task_id);
}

std::string async_result::error_message() const {
	if (!is_valid()) {
		return "";
	}

	auto result = backend_->get_error(task_id_);
	if (result.is_ok()) {
		return result.value().message;
	}
	return "";
}

std::string async_result::error_traceback() const {
	if (!is_valid()) {
		return "";
	}

	auto result = backend_->get_error(task_id_);
	if (result.is_ok()) {
		return result.value().traceback;
	}
	return "";
}

void async_result::invoke_callbacks() {
	if (callback_invoked_.exchange(true)) {
		return;  // Already invoked
	}

	std::function<void(const container_module::value_container&)> success_cb;
	std::function<void(const std::string&)> failure_cb;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		success_cb = success_callback_;
		failure_cb = failure_callback_;
	}

	if (is_successful()) {
		if (success_cb) {
			auto result = backend_->get_result(task_id_);
			if (result.is_ok()) {
				success_cb(result.value());
			} else if (failure_cb) {
				failure_cb(result.error().message);
			}
		}
	} else {
		if (failure_cb) {
			auto error_result = backend_->get_error(task_id_);
			if (error_result.is_ok()) {
				failure_cb(error_result.value().message);
			} else {
				failure_cb("Task failed with unknown error");
			}
		}
	}
}

void async_result::start_callback_monitor() {
	if (callback_monitor_started_.exchange(true)) {
		return;  // Already started
	}

	// Capture necessary data for the monitoring task
	std::string task_id_copy = task_id_;
	auto backend_copy = backend_;
	auto* self = this;

	auto monitor_func = [task_id_copy, backend_copy, self]() {
		const auto poll_interval = std::chrono::milliseconds(100);
		const auto max_wait = std::chrono::hours(24);  // Safety limit
		auto start = std::chrono::steady_clock::now();

		while (true) {
			auto elapsed = std::chrono::steady_clock::now() - start;
			if (elapsed >= max_wait) {
				break;
			}

			auto state_result = backend_copy->get_state(task_id_copy);
			if (state_result.is_ok()) {
				auto current_state = state_result.value();
				if (current_state == task_state::succeeded
					|| current_state == task_state::failed
					|| current_state == task_state::cancelled
					|| current_state == task_state::expired) {
					self->invoke_callbacks();
					break;
				}
			}

			std::this_thread::sleep_for(poll_interval);
		}
	};

	if (executor_ && executor_->is_running()) {
		// Use executor (preferred)
		auto job = std::make_unique<callback_monitor_job>(std::move(monitor_func));
		auto result = executor_->execute(std::move(job));
		if (!result.is_ok()) {
			// If executor fails, fall back to std::thread
			std::thread(monitor_func).detach();
		}
	} else {
		// Fallback to std::thread for backward compatibility
		std::thread(std::move(monitor_func)).detach();
	}
}

}  // namespace kcenon::messaging::task
