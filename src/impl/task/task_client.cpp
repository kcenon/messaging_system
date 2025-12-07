// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/task_client.h>

#include <atomic>
#include <random>
#include <sstream>
#include <iomanip>
#include <thread>

namespace kcenon::messaging::task {

task_client::task_client(
	std::shared_ptr<task_queue> queue,
	std::shared_ptr<result_backend_interface> results)
	: queue_(std::move(queue))
	, results_(std::move(results)) {
}

async_result task_client::send(task t) {
	return submit_task(t);
}

async_result task_client::send(
	const std::string& task_name,
	const container_module::value_container& payload) {
	auto build_result = task_builder(task_name)
		.payload(payload)
		.build();

	if (!build_result.is_ok()) {
		return async_result();  // Invalid result
	}

	task t = std::move(build_result).value();
	return submit_task(t);
}

async_result task_client::send(
	const std::string& task_name,
	std::shared_ptr<container_module::value_container> payload) {
	auto build_result = task_builder(task_name)
		.payload(std::move(payload))
		.build();

	if (!build_result.is_ok()) {
		return async_result();
	}

	task t = std::move(build_result).value();
	return submit_task(t);
}

async_result task_client::send_later(task t, std::chrono::milliseconds delay) {
	auto eta = std::chrono::system_clock::now() + delay;
	return send_at(std::move(t), eta);
}

async_result task_client::send_at(task t, std::chrono::system_clock::time_point eta) {
	t.config().eta = eta;
	return submit_task(t);
}

std::vector<async_result> task_client::send_batch(std::vector<task> tasks) {
	std::vector<async_result> results;
	results.reserve(tasks.size());

	for (auto& t : tasks) {
		results.push_back(submit_task(t));
	}

	return results;
}

async_result task_client::chain(std::vector<task> tasks) {
	if (tasks.empty()) {
		return async_result();
	}

	if (tasks.size() == 1) {
		return submit_task(tasks[0]);
	}

	// Generate a unique chain ID
	std::string chain_id = "chain-" + generate_workflow_id();

	// Create a virtual task to track the chain
	auto chain_task_result = task_builder("__chain__")
		.tag(chain_id)
		.build();

	if (!chain_task_result.is_ok()) {
		return async_result();
	}

	task chain_task = std::move(chain_task_result).value();
	std::string chain_task_id = chain_task.task_id();

	// Store initial state for chain task
	results_->store_state(chain_task_id, task_state::pending);
	results_->store_progress(chain_task_id, 0.0, "Starting chain");

	// Submit first task
	async_result first_result = submit_task(tasks[0]);

	// Create chain result
	async_result chain_result(chain_task_id, results_);

	// Start a background thread to manage the chain
	auto queue_copy = queue_;
	auto results_copy = results_;
	std::vector<task> remaining_tasks(tasks.begin() + 1, tasks.end());

	std::thread([chain_task_id, first_result, remaining_tasks, queue_copy, results_copy]() mutable {
		async_result current = first_result;
		size_t total = remaining_tasks.size() + 1;
		size_t completed = 0;

		while (true) {
			// Wait for current task to complete
			auto result = current.get(std::chrono::hours(24));

			if (!result.is_ok()) {
				// Chain failed
				results_copy->store_state(chain_task_id, task_state::failed);
				results_copy->store_error(chain_task_id, "Chain failed: " + result.error().message);
				return;
			}

			completed++;
			double progress = static_cast<double>(completed) / total;
			results_copy->store_progress(chain_task_id, progress,
				"Completed " + std::to_string(completed) + "/" + std::to_string(total));

			if (remaining_tasks.empty()) {
				// Chain completed
				results_copy->store_state(chain_task_id, task_state::succeeded);
				results_copy->store_result(chain_task_id, result.value());
				return;
			}

			// Get next task and pass result as input
			task next_task = std::move(remaining_tasks[0]);
			remaining_tasks.erase(remaining_tasks.begin());

			// Set the previous result as input for the next task
			auto payload = std::make_shared<container_module::value_container>(result.value());
			next_task.set_task_payload(payload);

			// Submit next task
			auto enqueue_result = queue_copy->enqueue(std::move(next_task));
			if (!enqueue_result.is_ok()) {
				results_copy->store_state(chain_task_id, task_state::failed);
				results_copy->store_error(chain_task_id, "Failed to enqueue next task in chain");
				return;
			}

			current = async_result(enqueue_result.value(), results_copy);
			results_copy->store_state(current.task_id(), task_state::pending);
		}
	}).detach();

	return chain_result;
}

async_result task_client::chord(std::vector<task> tasks, task callback) {
	if (tasks.empty()) {
		// No parallel tasks, just run the callback
		return submit_task(callback);
	}

	// Generate a unique chord ID
	std::string chord_id = "chord-" + generate_workflow_id();

	// Create a virtual task to track the chord
	auto chord_task_result = task_builder("__chord__")
		.tag(chord_id)
		.build();

	if (!chord_task_result.is_ok()) {
		return async_result();
	}

	task chord_task = std::move(chord_task_result).value();
	std::string chord_task_id = chord_task.task_id();

	// Store initial state
	results_->store_state(chord_task_id, task_state::pending);
	results_->store_progress(chord_task_id, 0.0, "Starting chord");

	// Submit all parallel tasks
	std::vector<async_result> parallel_results;
	for (auto& t : tasks) {
		parallel_results.push_back(submit_task(t));
	}

	// Create chord result
	async_result chord_result(chord_task_id, results_);

	// Start a background thread to manage the chord
	auto queue_copy = queue_;
	auto results_copy = results_;
	task callback_copy = std::move(callback);

	std::thread([chord_task_id, parallel_results, callback_copy, queue_copy, results_copy]() mutable {
		size_t total = parallel_results.size();
		std::vector<container_module::value_container> collected_results;
		collected_results.reserve(total);

		// Wait for all parallel tasks
		for (size_t i = 0; i < total; ++i) {
			auto& pr = parallel_results[i];
			auto result = pr.get(std::chrono::hours(24));

			if (!result.is_ok()) {
				// One task failed, chord fails
				results_copy->store_state(chord_task_id, task_state::failed);
				results_copy->store_error(chord_task_id,
					"Chord task " + std::to_string(i) + " failed: " + result.error().message);
				return;
			}

			collected_results.push_back(result.value());
			double progress = static_cast<double>(i + 1) / (total + 1);  // +1 for callback
			results_copy->store_progress(chord_task_id, progress,
				"Completed " + std::to_string(i + 1) + "/" + std::to_string(total) + " parallel tasks");
		}

		// All parallel tasks completed, prepare callback payload
		container_module::value_container callback_payload;
		// Store collected results - each result is stored by index
		for (size_t i = 0; i < collected_results.size(); ++i) {
			// Simplified: store as string representation
			// In production, would need proper serialization
		}

		// Create payload with results count (simplified approach)
		auto payload = std::make_shared<container_module::value_container>();
		callback_copy.set_task_payload(payload);

		// Submit callback task
		auto enqueue_result = queue_copy->enqueue(std::move(callback_copy));
		if (!enqueue_result.is_ok()) {
			results_copy->store_state(chord_task_id, task_state::failed);
			results_copy->store_error(chord_task_id, "Failed to enqueue chord callback");
			return;
		}

		// Wait for callback to complete
		async_result callback_result(enqueue_result.value(), results_copy);
		results_copy->store_state(callback_result.task_id(), task_state::pending);

		auto final_result = callback_result.get(std::chrono::hours(24));

		if (!final_result.is_ok()) {
			results_copy->store_state(chord_task_id, task_state::failed);
			results_copy->store_error(chord_task_id, "Chord callback failed: " + final_result.error().message);
			return;
		}

		results_copy->store_state(chord_task_id, task_state::succeeded);
		results_copy->store_result(chord_task_id, final_result.value());
		results_copy->store_progress(chord_task_id, 1.0, "Chord completed");
	}).detach();

	return chord_result;
}

async_result task_client::get_result(const std::string& task_id) {
	return async_result(task_id, results_);
}

common::VoidResult task_client::cancel(const std::string& task_id) {
	if (!queue_) {
		return common::VoidResult(common::error_info{-1, "Queue not available"});
	}

	auto cancel_result = queue_->cancel(task_id);
	if (cancel_result.is_ok() && results_) {
		results_->store_state(task_id, task_state::cancelled);
	}
	return cancel_result;
}

common::VoidResult task_client::cancel_by_tag(const std::string& tag) {
	if (!queue_) {
		return common::VoidResult(common::error_info{-1, "Queue not available"});
	}
	return queue_->cancel_by_tag(tag);
}

size_t task_client::pending_count(const std::string& queue_name) const {
	if (!queue_) {
		return 0;
	}
	return queue_->queue_size(queue_name);
}

bool task_client::is_connected() const {
	return queue_ != nullptr && results_ != nullptr && queue_->is_running();
}

async_result task_client::submit_task(task& t) {
	if (!queue_ || !results_) {
		return async_result();
	}

	std::string task_id = t.task_id();

	// Initialize task state in backend
	results_->store_state(task_id, task_state::pending);

	// Enqueue the task
	auto enqueue_result = queue_->enqueue(std::move(t));
	if (!enqueue_result.is_ok()) {
		results_->store_state(task_id, task_state::failed);
		results_->store_error(task_id, "Failed to enqueue: " + enqueue_result.error().message);
		return async_result();
	}

	return async_result(task_id, results_);
}

std::string task_client::generate_workflow_id() {
	static std::atomic<uint64_t> counter{0};
	auto now = std::chrono::system_clock::now();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()).count();

	std::ostringstream oss;
	oss << std::hex << std::setfill('0')
		<< std::setw(12) << millis
		<< std::setw(8) << counter++;
	return oss.str();
}

}  // namespace kcenon::messaging::task
