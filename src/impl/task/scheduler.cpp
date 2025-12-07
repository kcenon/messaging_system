// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include "kcenon/messaging/task/scheduler.h"

#include <kcenon/messaging/error/error_codes.h>

#include <algorithm>

namespace kcenon::messaging::task {

// ============================================================================
// Constructor / Destructor
// ============================================================================

task_scheduler::task_scheduler(std::shared_ptr<task_client> client)
	: client_(std::move(client)) {}

task_scheduler::~task_scheduler() {
	if (running_.load(std::memory_order_acquire)) {
		stop();
	}
}

task_scheduler::task_scheduler(task_scheduler&& other) noexcept
	: client_(std::move(other.client_)),
	  schedules_(std::move(other.schedules_)),
	  on_executed_(std::move(other.on_executed_)),
	  on_failed_(std::move(other.on_failed_)) {
	// Note: thread and atomic state cannot be moved
	// The moved-from object should not be running
}

task_scheduler& task_scheduler::operator=(task_scheduler&& other) noexcept {
	if (this != &other) {
		// Stop current scheduler if running
		if (running_.load(std::memory_order_acquire)) {
			stop();
		}

		std::lock_guard<std::mutex> lock(mutex_);
		client_ = std::move(other.client_);
		schedules_ = std::move(other.schedules_);
		on_executed_ = std::move(other.on_executed_);
		on_failed_ = std::move(other.on_failed_);
	}
	return *this;
}

// ============================================================================
// Schedule registration
// ============================================================================

common::VoidResult task_scheduler::add_periodic(
	const std::string& name,
	task task_template,
	std::chrono::seconds interval) {
	if (name.empty()) {
		return common::VoidResult(
			common::error_info{error::task_invalid_argument, "Schedule name cannot be empty"});
	}

	if (interval.count() <= 0) {
		return common::VoidResult(common::error_info{
			error::task_invalid_argument, "Interval must be positive"});
	}

	std::lock_guard<std::mutex> lock(mutex_);

	if (schedules_.find(name) != schedules_.end()) {
		return common::VoidResult(common::error_info{
			error::schedule_already_exists, "Schedule already exists: " + name});
	}

	schedule_entry entry;
	entry.name = name;
	entry.task_template = std::move(task_template);
	entry.schedule = interval;
	entry.enabled = true;
	entry.next_run = calculate_next_run(entry);

	schedules_.emplace(name, std::move(entry));

	// Wake up the scheduler to recalculate next run time
	wake_up();

	return common::ok();
}

common::VoidResult task_scheduler::add_cron(
	const std::string& name,
	task task_template,
	const std::string& cron_expression) {
	if (name.empty()) {
		return common::VoidResult(
			common::error_info{error::task_invalid_argument, "Schedule name cannot be empty"});
	}

	// Validate cron expression
	if (!cron_parser::is_valid(cron_expression)) {
		return common::VoidResult(common::error_info{
			error::task_invalid_argument, "Invalid cron expression: " + cron_expression});
	}

	std::lock_guard<std::mutex> lock(mutex_);

	if (schedules_.find(name) != schedules_.end()) {
		return common::VoidResult(common::error_info{
			error::schedule_already_exists, "Schedule already exists: " + name});
	}

	schedule_entry entry;
	entry.name = name;
	entry.task_template = std::move(task_template);
	entry.schedule = cron_expression;
	entry.enabled = true;
	entry.next_run = calculate_next_run(entry);

	schedules_.emplace(name, std::move(entry));

	// Wake up the scheduler to recalculate next run time
	wake_up();

	return common::ok();
}

// ============================================================================
// Schedule management
// ============================================================================

common::VoidResult task_scheduler::remove(const std::string& name) {
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::VoidResult(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	schedules_.erase(it);
	wake_up();

	return common::ok();
}

common::VoidResult task_scheduler::enable(const std::string& name) {
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::VoidResult(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	if (!it->second.enabled) {
		it->second.enabled = true;
		it->second.next_run = calculate_next_run(it->second);
		wake_up();
	}

	return common::ok();
}

common::VoidResult task_scheduler::disable(const std::string& name) {
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::VoidResult(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	it->second.enabled = false;

	return common::ok();
}

common::VoidResult task_scheduler::trigger_now(const std::string& name) {
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::VoidResult(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	execute_schedule(it->second);

	return common::ok();
}

common::VoidResult task_scheduler::update_interval(
	const std::string& name,
	std::chrono::seconds interval) {
	if (interval.count() <= 0) {
		return common::VoidResult(common::error_info{
			error::task_invalid_argument, "Interval must be positive"});
	}

	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::VoidResult(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	if (!it->second.is_periodic()) {
		return common::VoidResult(common::error_info{
			error::task_invalid_argument, "Schedule is not periodic: " + name});
	}

	it->second.schedule = interval;
	it->second.next_run = calculate_next_run(it->second);
	wake_up();

	return common::ok();
}

common::VoidResult task_scheduler::update_cron(
	const std::string& name,
	const std::string& cron_expression) {
	if (!cron_parser::is_valid(cron_expression)) {
		return common::VoidResult(common::error_info{
			error::task_invalid_argument, "Invalid cron expression: " + cron_expression});
	}

	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::VoidResult(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	if (!it->second.is_cron()) {
		return common::VoidResult(common::error_info{
			error::task_invalid_argument, "Schedule is not cron-based: " + name});
	}

	it->second.schedule = cron_expression;
	it->second.next_run = calculate_next_run(it->second);
	wake_up();

	return common::ok();
}

// ============================================================================
// Lifecycle management
// ============================================================================

common::VoidResult task_scheduler::start() {
	bool expected = false;
	if (!running_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
		// Already running
		return common::ok();
	}

	stop_requested_.store(false, std::memory_order_release);

	// Calculate next run times for all enabled schedules
	{
		std::lock_guard<std::mutex> lock(mutex_);
		for (auto& [name, entry] : schedules_) {
			if (entry.enabled && !entry.next_run.has_value()) {
				entry.next_run = calculate_next_run(entry);
			}
		}
	}

	// Start the scheduler thread
	scheduler_thread_ = std::thread([this]() { scheduler_loop(); });

	return common::ok();
}

common::VoidResult task_scheduler::stop() {
	bool expected = true;
	if (!running_.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
		// Not running
		return common::ok();
	}

	stop_requested_.store(true, std::memory_order_release);
	cv_.notify_all();

	if (scheduler_thread_.joinable()) {
		scheduler_thread_.join();
	}

	return common::ok();
}

bool task_scheduler::is_running() const {
	return running_.load(std::memory_order_acquire);
}

// ============================================================================
// Query
// ============================================================================

std::vector<schedule_entry> task_scheduler::list_schedules() const {
	std::lock_guard<std::mutex> lock(mutex_);

	std::vector<schedule_entry> result;
	result.reserve(schedules_.size());

	for (const auto& [name, entry] : schedules_) {
		result.push_back(entry);
	}

	return result;
}

common::Result<schedule_entry> task_scheduler::get_schedule(const std::string& name) const {
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = schedules_.find(name);
	if (it == schedules_.end()) {
		return common::Result<schedule_entry>(
			common::error_info{error::task_not_found, "Schedule not found: " + name});
	}

	return common::Result<schedule_entry>(it->second);
}

size_t task_scheduler::schedule_count() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return schedules_.size();
}

bool task_scheduler::has_schedule(const std::string& name) const {
	std::lock_guard<std::mutex> lock(mutex_);
	return schedules_.find(name) != schedules_.end();
}

// ============================================================================
// Event callbacks
// ============================================================================

void task_scheduler::on_task_executed(schedule_callback callback) {
	std::lock_guard<std::mutex> lock(mutex_);
	on_executed_ = std::move(callback);
}

void task_scheduler::on_task_failed(schedule_callback callback) {
	std::lock_guard<std::mutex> lock(mutex_);
	on_failed_ = std::move(callback);
}

// ============================================================================
// Private methods
// ============================================================================

void task_scheduler::scheduler_loop() {
	while (!stop_requested_.load(std::memory_order_acquire)) {
		std::unique_lock<std::mutex> lock(mutex_);

		// Find the next schedule to run
		auto next_it = find_next_schedule();

		if (next_it == schedules_.end()) {
			// No schedules to run, wait indefinitely until woken up
			cv_.wait(lock, [this]() {
				return stop_requested_.load(std::memory_order_acquire);
			});
			continue;
		}

		auto& entry = next_it->second;
		auto now = std::chrono::system_clock::now();

		if (!entry.next_run.has_value()) {
			entry.next_run = calculate_next_run(entry);
			continue;
		}

		auto next_run = entry.next_run.value();

		if (next_run <= now) {
			// Time to execute
			execute_schedule(entry);
			entry.next_run = calculate_next_run(entry);
		} else {
			// Wait until next run time or until woken up
			auto wait_duration = next_run - now;
			cv_.wait_for(lock, wait_duration, [this]() {
				return stop_requested_.load(std::memory_order_acquire);
			});
		}
	}
}

std::chrono::system_clock::time_point task_scheduler::calculate_next_run(
	const schedule_entry& entry) {
	auto now = std::chrono::system_clock::now();

	if (entry.is_periodic()) {
		// For periodic schedules, add interval to last run or use now
		if (entry.last_run.has_value()) {
			return entry.last_run.value() + entry.interval();
		}
		return now;
	}

	// For cron schedules
	auto cron_result = cron_parser::parse(entry.cron_expression());
	if (!cron_result.is_ok()) {
		// Fallback to one hour from now on parse error
		return now + std::chrono::hours(1);
	}

	auto next_result = cron_parser::next_run_time(cron_result.unwrap(), now);
	if (!next_result.is_ok()) {
		// Fallback to one hour from now on calculation error
		return now + std::chrono::hours(1);
	}

	return next_result.unwrap();
}

void task_scheduler::execute_schedule(schedule_entry& entry) {
	if (!client_) {
		if (on_failed_) {
			on_failed_(entry);
		}
		entry.failure_count++;
		return;
	}

	// Create a copy of the task template with a new ID
	task task_copy = entry.task_template;

	try {
		// Submit the task
		client_->send(std::move(task_copy));

		// Update entry statistics
		entry.last_run = std::chrono::system_clock::now();
		entry.run_count++;

		// Notify success callback
		if (on_executed_) {
			on_executed_(entry);
		}
	} catch (...) {
		entry.failure_count++;
		if (on_failed_) {
			on_failed_(entry);
		}
	}
}

std::unordered_map<std::string, schedule_entry>::iterator task_scheduler::find_next_schedule() {
	auto earliest = schedules_.end();
	std::optional<std::chrono::system_clock::time_point> earliest_time;

	for (auto it = schedules_.begin(); it != schedules_.end(); ++it) {
		const auto& entry = it->second;

		// Skip disabled schedules
		if (!entry.enabled) {
			continue;
		}

		// Skip schedules without next run time
		if (!entry.next_run.has_value()) {
			continue;
		}

		auto next_run = entry.next_run.value();

		if (!earliest_time.has_value() || next_run < earliest_time.value()) {
			earliest_time = next_run;
			earliest = it;
		}
	}

	return earliest;
}

void task_scheduler::wake_up() {
	cv_.notify_all();
}

}  // namespace kcenon::messaging::task
