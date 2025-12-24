#include "kcenon/messaging/core/message_bus.h"

#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>
#include <kcenon/common/interfaces/executor_interface.h>

#include <thread>
#include <algorithm>

namespace kcenon::messaging {

// ============================================================================
// message_processing_job - IJob implementation for worker threads
// ============================================================================

/**
 * @class message_processing_job
 * @brief Job implementation that processes messages from the queue
 *
 * This class encapsulates the message processing logic for use with
 * the backend's IExecutor, allowing worker threads to be managed by
 * the thread pool instead of std::async.
 */
class message_processing_job : public common::interfaces::IJob {
public:
	message_processing_job(message_bus* bus, std::atomic<bool>* running)
		: bus_(bus)
		, running_(running) {
	}

	common::VoidResult execute() override {
		while (running_->load()) {
			bus_->process_single_message();
		}
		return common::ok();
	}

	std::string get_name() const override {
		return "message_processing_job";
	}

	int get_priority() const override {
		return 0;
	}

private:
	message_bus* bus_;
	std::atomic<bool>* running_;
};

// ============================================================================
// message_bus implementation
// ============================================================================

message_bus::message_bus(
	std::shared_ptr<backend_interface> backend,
	message_bus_config config)
	: config_(std::move(config))
	, backend_(std::move(backend))
	, queue_(nullptr)
	, router_(std::make_unique<topic_router>())
	, dead_letter_queue_(nullptr)
	, transport_(config_.transport) {

	// Create main queue
	queue_config qconfig;
	qconfig.max_size = config_.queue_capacity;
	qconfig.enable_priority = config_.enable_priority_queue;
	qconfig.enable_persistence = false;
	qconfig.drop_on_full = false;
	queue_ = std::make_unique<message_queue>(qconfig);

	// Create dead letter queue if enabled
	if (config_.enable_dead_letter_queue) {
		queue_config dlq_config;
		dlq_config.max_size = config_.queue_capacity / 10; // 10% of main queue
		dlq_config.enable_priority = false;
		dlq_config.enable_persistence = false;
		dlq_config.drop_on_full = true;
		dead_letter_queue_ = std::make_unique<message_queue>(dlq_config);
	}

	// Setup transport handlers if transport is configured
	if (transport_) {
		setup_transport_handlers();
	}
}

message_bus::~message_bus() {
	if (running_.load()) {
		stop();
	}
}

common::VoidResult message_bus::start() {
	if (running_.load()) {
		common::logging::log_warning("Message bus start called but already running");
		return common::make_error<std::monostate>(
			error::already_running,
			"Message bus is already running"
		);
	}

	common::logging::log_info("Starting message bus with " +
		std::to_string(config_.worker_threads) + " worker threads");

	// Initialize backend
	auto result = backend_->initialize();
	if (!result.is_ok()) {
		common::logging::log_error("Failed to initialize backend: " +
			result.error().message);
		return result;
	}

	if (!backend_->is_ready()) {
		common::logging::log_error("Backend is not ready after initialization");
		return common::make_error<std::monostate>(
			error::backend_not_ready,
			"Backend is not ready"
		);
	}

	// Connect transport if configured and mode requires it
	if (transport_ && config_.mode != transport_mode::local) {
		common::logging::log_info("Connecting transport for remote messaging");
		auto connect_result = transport_->connect();
		if (!connect_result.is_ok()) {
			common::logging::log_error("Failed to connect transport: " +
				connect_result.error().message);
			backend_->shutdown();
			return connect_result;
		}
	}

	running_.store(true);
	start_workers();

	common::logging::log_info("Message bus started successfully");
	return common::ok();
}

common::VoidResult message_bus::stop() {
	if (!running_.load()) {
		common::logging::log_debug("Message bus stop called but not running");
		return common::make_error<std::monostate>(
			error::not_running,
			"Message bus is not running"
		);
	}

	common::logging::log_info("Stopping message bus");

	running_.store(false);
	stop_workers();

	// Disconnect transport if connected
	if (transport_ && transport_->is_connected()) {
		common::logging::log_info("Disconnecting transport");
		auto disconnect_result = transport_->disconnect();
		if (!disconnect_result.is_ok()) {
			common::logging::log_warning("Failed to disconnect transport: " +
				disconnect_result.error().message);
		}
	}

	// Shutdown backend
	auto result = backend_->shutdown();
	if (!result.is_ok()) {
		common::logging::log_error("Failed to shutdown backend: " +
			result.error().message);
		return result;
	}

	common::logging::log_info("Message bus stopped successfully");
	return common::ok();
}

common::VoidResult message_bus::publish(const message& msg) {
	if (!running_.load()) {
		common::logging::log_debug("Publish rejected: message bus not running");
		return common::make_error<std::monostate>(
			error::not_running,
			"Message bus is not running"
		);
	}

	common::logging::log_trace("Publishing message to topic: " + msg.metadata().topic +
		", id: " + msg.metadata().id);

	common::VoidResult local_result = common::ok();
	common::VoidResult remote_result = common::ok();

	// Route based on transport mode
	switch (config_.mode) {
		case transport_mode::local:
			local_result = route_local(msg);
			break;

		case transport_mode::remote:
			remote_result = send_to_remote(msg);
			break;

		case transport_mode::hybrid:
			// Route to both local and remote
			local_result = route_local(msg);
			remote_result = send_to_remote(msg);
			break;
	}

	// Check results
	if (!local_result.is_ok() && !remote_result.is_ok()) {
		stats_.messages_dropped.fetch_add(1);
		return local_result;  // Return local error for diagnostics
	}

	stats_.messages_published.fetch_add(1);
	return common::ok();
}

common::VoidResult message_bus::publish(const std::string& topic, message msg) {
	msg.metadata().topic = topic;
	return publish(msg);
}

common::Result<uint64_t> message_bus::subscribe(
	const std::string& topic_pattern,
	subscription_callback callback,
	message_filter filter,
	int priority) {

	return router_->subscribe(topic_pattern, callback, filter, priority);
}

common::VoidResult message_bus::unsubscribe(uint64_t subscription_id) {
	return router_->unsubscribe(subscription_id);
}

common::Result<message> message_bus::request(
	const message& request_msg,
	std::chrono::milliseconds timeout) {

	if (!running_.load()) {
		return common::make_error<message>(
			error::not_running,
			"Message bus is not running",
			"messaging_system"
		);
	}

	// Generate correlation ID for request-reply
	std::string correlation_id = request_msg.metadata().id;
	std::string reply_topic = request_msg.metadata().topic + ".reply." + correlation_id;

	// Create promise for reply
	std::promise<common::Result<message>> reply_promise;
	auto reply_future = reply_promise.get_future();

	// Subscribe to reply topic
	auto sub_result = subscribe(
		reply_topic,
		[&reply_promise](const message& reply) {
			reply_promise.set_value(common::ok(reply));
			return common::ok();
		}
	);

	if (!sub_result.is_ok()) {
		return common::make_error<message>(
			sub_result.error().code,
			sub_result.error().message,
			"messaging_system"
		);
	}

	uint64_t sub_id = sub_result.unwrap();

	// Publish request
	auto msg_copy = request_msg;
	msg_copy.metadata().correlation_id = correlation_id;

	auto pub_result = publish(msg_copy);
	if (!pub_result.is_ok()) {
		unsubscribe(sub_id);
		return common::make_error<message>(
			pub_result.error().code,
			pub_result.error().message,
			"messaging_system"
		);
	}

	// Wait for reply
	auto status = reply_future.wait_for(timeout);

	// Unsubscribe from reply topic
	unsubscribe(sub_id);

	if (status == std::future_status::timeout) {
		return common::make_error<message>(
			error::request_timeout,
			"Request timeout",
			"messaging_system"
		);
	}

	return reply_future.get();
}

message_bus::statistics_snapshot message_bus::get_statistics() const {
	statistics_snapshot snapshot;
	snapshot.messages_published = stats_.messages_published.load();
	snapshot.messages_processed = stats_.messages_processed.load();
	snapshot.messages_failed = stats_.messages_failed.load();
	snapshot.messages_dropped = stats_.messages_dropped.load();
	snapshot.messages_sent_remote = stats_.messages_sent_remote.load();
	snapshot.messages_received_remote = stats_.messages_received_remote.load();
	return snapshot;
}

void message_bus::reset_statistics() {
	stats_.messages_published.store(0);
	stats_.messages_processed.store(0);
	stats_.messages_failed.store(0);
	stats_.messages_dropped.store(0);
	stats_.messages_sent_remote.store(0);
	stats_.messages_received_remote.store(0);
}

bool message_bus::is_transport_connected() const {
	return transport_ && transport_->is_connected();
}

void message_bus::process_messages() {
	while (running_.load()) {
		process_single_message();
	}
}

void message_bus::process_single_message() {
	// Try to dequeue message with timeout
	auto result = queue_->dequeue(std::chrono::milliseconds(100));

	if (!result.is_ok()) {
		// Queue might be empty or stopped, return and let caller decide
		return;
	}

	auto msg = result.unwrap();

	// Handle message
	auto handle_result = handle_message(msg);

	if (handle_result.is_ok()) {
		stats_.messages_processed.fetch_add(1);
	} else {
		stats_.messages_failed.fetch_add(1);

		// Move to dead letter queue if enabled
		if (dead_letter_queue_) {
			dead_letter_queue_->enqueue(std::move(msg));
		}
	}
}

common::VoidResult message_bus::handle_message(const message& msg) {
	// Check if message is expired
	if (msg.is_expired()) {
		common::logging::log_debug("Message expired, id: " + msg.metadata().id +
			", topic: " + msg.metadata().topic);
		return common::make_error<std::monostate>(
			error::message_expired,
			"Message has expired",
			"messaging_system"
		);
	}

	common::logging::log_trace("Routing message, id: " + msg.metadata().id +
		", topic: " + msg.metadata().topic);

	// Route message to subscribers
	auto result = router_->route(msg);
	if (!result.is_ok()) {
		common::logging::log_debug("Message routing failed, id: " + msg.metadata().id +
			", error: " + result.error().message);
		return result;
	}

	return common::ok();
}

void message_bus::start_workers() {
	auto executor = backend_->get_executor();

	if (executor && executor->is_running()) {
		// Use executor for worker threads (preferred)
		common::logging::log_debug("Starting workers using backend executor");

		for (size_t i = 0; i < config_.worker_threads; ++i) {
			auto job = std::make_unique<message_processing_job>(this, &running_);
			auto result = executor->execute(std::move(job));

			if (result.is_ok()) {
				workers_.push_back(std::move(result.value()));
			} else {
				common::logging::log_warning(
					"Failed to submit worker job to executor: " + result.error().message);
			}
		}

		common::logging::log_info("Started " + std::to_string(workers_.size()) +
			" workers using executor");
	} else {
		// Fallback to std::async for backward compatibility
		common::logging::log_debug("Starting workers using std::async (fallback)");

		for (size_t i = 0; i < config_.worker_threads; ++i) {
			workers_.push_back(
				std::async(std::launch::async, [this]() {
					this->process_messages();
				})
			);
		}

		common::logging::log_info("Started " + std::to_string(config_.worker_threads) +
			" workers using std::async");
	}
}

void message_bus::stop_workers() {
	// Stop the queue to wake up blocking dequeue calls
	queue_->stop();

	// Wait for all workers to finish
	for (auto& worker : workers_) {
		if (worker.valid()) {
			worker.wait();
		}
	}
	workers_.clear();
}

void message_bus::setup_transport_handlers() {
	if (!transport_) {
		return;
	}

	// Setup message handler for incoming remote messages
	transport_->set_message_handler(
		[this](const message& msg) {
			handle_remote_message(msg);
		}
	);

	// Setup error handler
	transport_->set_error_handler(
		[](const std::string& error_msg) {
			common::logging::log_error("Transport error: " + error_msg);
		}
	);

	// Setup state handler for connection state changes
	transport_->set_state_handler(
		[](adapters::transport_state state) {
			switch (state) {
				case adapters::transport_state::connected:
					common::logging::log_info("Transport connected");
					break;
				case adapters::transport_state::disconnected:
					common::logging::log_info("Transport disconnected");
					break;
				case adapters::transport_state::connecting:
					common::logging::log_debug("Transport connecting");
					break;
				case adapters::transport_state::disconnecting:
					common::logging::log_debug("Transport disconnecting");
					break;
				case adapters::transport_state::error:
					common::logging::log_warning("Transport in error state");
					break;
			}
		}
	);
}

void message_bus::handle_remote_message(const message& msg) {
	if (!running_.load()) {
		common::logging::log_debug("Remote message ignored: message bus not running");
		return;
	}

	common::logging::log_trace("Received remote message, topic: " + msg.metadata().topic +
		", id: " + msg.metadata().id);

	stats_.messages_received_remote.fetch_add(1);

	// Route to local subscribers
	auto result = queue_->enqueue(msg);
	if (!result.is_ok()) {
		common::logging::log_warning("Failed to enqueue remote message: " +
			result.error().message);
		stats_.messages_dropped.fetch_add(1);
	}
}

common::VoidResult message_bus::send_to_remote(const message& msg) {
	if (!transport_) {
		common::logging::log_debug("No transport configured for remote send");
		return common::make_error<std::monostate>(
			error::no_route_found,
			"No transport configured"
		);
	}

	if (!transport_->is_connected()) {
		common::logging::log_debug("Transport not connected for remote send");
		return common::make_error<std::monostate>(
			error::broker_unavailable,
			"Transport not connected"
		);
	}

	auto result = transport_->send(msg);
	if (result.is_ok()) {
		stats_.messages_sent_remote.fetch_add(1);
	}
	return result;
}

common::VoidResult message_bus::route_local(const message& msg) {
	auto result = queue_->enqueue(msg);
	if (!result.is_ok()) {
		common::logging::log_warning("Message dropped, queue full for topic: " +
			msg.metadata().topic);
	}
	return result;
}

} // namespace kcenon::messaging
