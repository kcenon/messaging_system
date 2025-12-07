/**
 * @file example_task_event_bridge.cpp
 * @brief Example demonstrating task event bridge integration
 *
 * This example shows how to:
 * 1. Set up task event bridge
 * 2. Subscribe to task lifecycle events
 * 3. Monitor worker and queue status
 */

#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/integration/task_event_bridge.h>
#include <kcenon/messaging/integration/task_events.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

namespace {

// Helper function to format timestamp
std::string format_time(const std::chrono::system_clock::time_point& tp) {
	auto time = std::chrono::system_clock::to_time_t(tp);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				  tp.time_since_epoch()) %
			  1000;
	std::stringstream ss;
	ss << std::put_time(std::localtime(&time), "%H:%M:%S") << '.' << std::setfill('0')
	   << std::setw(3) << ms.count();
	return ss.str();
}

// Event handlers
void on_task_queued(const task_queued_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] QUEUED: "
			  << "task=" << evt.task_name << " "
			  << "id=" << evt.task_id << " "
			  << "queue=" << evt.queue << std::endl;
}

void on_task_started(const task_started_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] STARTED: "
			  << "task=" << evt.task_name << " "
			  << "id=" << evt.task_id << " "
			  << "worker=" << evt.worker_id << std::endl;
}

void on_task_progress(const task_progress_event& evt) {
	int percent = static_cast<int>(evt.progress * 100);
	std::cout << "[" << format_time(evt.timestamp) << "] PROGRESS: "
			  << "task=" << evt.task_name << " "
			  << percent << "% " << evt.message << std::endl;
}

void on_task_succeeded(const task_succeeded_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] SUCCESS: "
			  << "task=" << evt.task_name << " "
			  << "id=" << evt.task_id << " "
			  << "duration=" << evt.duration.count() << "ms" << std::endl;
}

void on_task_failed(const task_failed_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] FAILED: "
			  << "task=" << evt.task_name << " "
			  << "id=" << evt.task_id << " "
			  << "error=\"" << evt.error_message << "\" "
			  << "attempt=" << evt.attempt << std::endl;
}

void on_task_retrying(const task_retrying_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] RETRYING: "
			  << "task=" << evt.task_name << " "
			  << "attempt=" << evt.attempt << "/" << evt.max_retries << " "
			  << "delay=" << evt.retry_delay.count() << "ms" << std::endl;
}

void on_worker_online(const worker_online_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] WORKER ONLINE: "
			  << "id=" << evt.worker_id << " "
			  << "concurrency=" << evt.concurrency << " "
			  << "queues=[";
	for (size_t i = 0; i < evt.queues.size(); ++i) {
		if (i > 0)
			std::cout << ",";
		std::cout << evt.queues[i];
	}
	std::cout << "]" << std::endl;
}

void on_worker_heartbeat(const worker_heartbeat_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] HEARTBEAT: "
			  << "worker=" << evt.worker_id << " "
			  << "active=" << evt.active_tasks << " "
			  << "completed=" << evt.completed_tasks << " "
			  << "failed=" << evt.failed_tasks << std::endl;
}

void on_queue_high_watermark(const queue_high_watermark_event& evt) {
	std::cout << "[" << format_time(evt.timestamp) << "] HIGH WATERMARK: "
			  << "queue=" << evt.queue << " "
			  << "size=" << evt.current_size << "/" << evt.threshold << std::endl;
}

}  // namespace

int main() {
	std::cout << "=== Task Event Bridge Example ===" << std::endl;
	std::cout << std::endl;

	// Get the global event bus
	auto& event_bus = get_event_bus();
	event_bus.start();

	// Subscribe to task events
	std::cout << "Subscribing to task events..." << std::endl;
	auto sub_queued = event_bus.subscribe<task_queued_event>(on_task_queued);
	auto sub_started = event_bus.subscribe<task_started_event>(on_task_started);
	auto sub_progress = event_bus.subscribe<task_progress_event>(on_task_progress);
	auto sub_succeeded = event_bus.subscribe<task_succeeded_event>(on_task_succeeded);
	auto sub_failed = event_bus.subscribe<task_failed_event>(on_task_failed);
	auto sub_retrying = event_bus.subscribe<task_retrying_event>(on_task_retrying);
	auto sub_worker_online = event_bus.subscribe<worker_online_event>(on_worker_online);
	auto sub_heartbeat = event_bus.subscribe<worker_heartbeat_event>(on_worker_heartbeat);
	auto sub_watermark = event_bus.subscribe<queue_high_watermark_event>(on_queue_high_watermark);

	std::cout << std::endl;

	// Create and start the task event bridge
	task_event_bridge_config config;
	config.enable_progress_events = true;
	config.enable_heartbeat_events = true;

	task_event_bridge bridge(config);
	auto start_result = bridge.start();
	if (start_result.is_err()) {
		std::cerr << "Failed to start bridge: " << start_result.error().message << std::endl;
		return 1;
	}

	std::cout << "Task event bridge started." << std::endl;
	std::cout << std::endl;

	// Simulate task workflow
	std::cout << "--- Simulating Task Workflow ---" << std::endl;
	std::cout << std::endl;

	// Worker comes online
	bridge.on_worker_online("worker-1", {"default", "high-priority"}, 4);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Task 1: Successful task
	std::string task1_id = "task-001";
	bridge.on_task_queued(task1_id, "email.send", "default");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	bridge.on_task_started(task1_id, "email.send", "default", "worker-1");

	// Simulate progress
	for (int i = 1; i <= 4; ++i) {
		double progress = i * 0.25;
		bridge.on_task_progress(task1_id, "email.send", progress,
								"Processing batch " + std::to_string(i));
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	bridge.on_task_succeeded(task1_id, "email.send", "default", "worker-1",
							 std::chrono::milliseconds(200));
	std::cout << std::endl;

	// Task 2: Failed task with retry
	std::string task2_id = "task-002";
	bridge.on_task_queued(task2_id, "image.process", "default");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	bridge.on_task_started(task2_id, "image.process", "default", "worker-1");
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	bridge.on_task_failed(task2_id, "image.process", "default", "worker-1",
						  "Connection timeout", "", 1);

	bridge.on_task_retrying(task2_id, "image.process", "default", 1, 3,
							std::chrono::milliseconds(1000));
	std::cout << std::endl;

	// Worker heartbeat
	bridge.on_worker_heartbeat("worker-1", 1, 1, 1);
	std::cout << std::endl;

	// Queue high watermark
	bridge.on_queue_high_watermark("default", 950, 1000);
	std::cout << std::endl;

	// Cleanup
	std::cout << "--- Cleanup ---" << std::endl;
	bridge.stop();

	event_bus.unsubscribe(sub_queued);
	event_bus.unsubscribe(sub_started);
	event_bus.unsubscribe(sub_progress);
	event_bus.unsubscribe(sub_succeeded);
	event_bus.unsubscribe(sub_failed);
	event_bus.unsubscribe(sub_retrying);
	event_bus.unsubscribe(sub_worker_online);
	event_bus.unsubscribe(sub_heartbeat);
	event_bus.unsubscribe(sub_watermark);

	event_bus.stop();

	std::cout << "Example completed." << std::endl;

	return 0;
}
