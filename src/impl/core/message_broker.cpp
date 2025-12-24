// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/core/message_broker.h>

#include <kcenon/common/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>
#include <kcenon/messaging/error/error_codes.h>

#include <deque>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace kcenon::messaging {

// =============================================================================
// Internal Route Data
// =============================================================================

struct internal_route {
	route_info info;
	message_handler handler;
	uint64_t subscription_id = 0;
};

// =============================================================================
// Message Broker Implementation (PIMPL)
// =============================================================================

class message_broker_impl {
public:
	explicit message_broker_impl(broker_config config)
		: config_(std::move(config))
		, router_(std::make_unique<topic_router>())
		, running_(false) {
		statistics_.last_reset = std::chrono::steady_clock::now();
	}

	~message_broker_impl() {
		if (running_.load()) {
			stop();
		}
	}

	// =========================================================================
	// Lifecycle
	// =========================================================================

	common::VoidResult start() {
		if (running_.load()) {
			common::logging::log_warning("Message broker start called but already running");
			return common::make_error<std::monostate>(
				error::already_running,
				"Message broker is already running"
			);
		}

		common::logging::log_info("Starting message broker");
		running_.store(true);
		common::logging::log_info("Message broker started successfully");
		return common::ok();
	}

	common::VoidResult stop() {
		if (!running_.load()) {
			common::logging::log_debug("Message broker stop called but not running");
			return common::make_error<std::monostate>(
				error::not_running,
				"Message broker is not running"
			);
		}

		common::logging::log_info("Stopping message broker");
		running_.store(false);
		common::logging::log_info("Message broker stopped successfully");
		return common::ok();
	}

	bool is_running() const {
		return running_.load();
	}

	// =========================================================================
	// Route Management
	// =========================================================================

	common::VoidResult add_route(const std::string& route_id,
								 const std::string& topic_pattern,
								 message_handler handler,
								 int priority) {
		if (route_id.empty()) {
			common::logging::log_error("Add route failed: route_id is empty");
			return common::error_info(
				common::error::codes::common_errors::invalid_argument,
				"Route ID cannot be empty"
			);
		}

		if (topic_pattern.empty()) {
			common::logging::log_error("Add route failed: topic_pattern is empty");
			return common::error_info(
				common::error::codes::common_errors::invalid_argument,
				"Topic pattern cannot be empty"
			);
		}

		if (!handler) {
			common::logging::log_error("Add route failed: handler is null");
			return common::error_info(
				common::error::codes::common_errors::invalid_argument,
				"Handler cannot be null"
			);
		}

		if (priority < 0 || priority > 10) {
			common::logging::log_error("Add route failed: invalid priority " +
				std::to_string(priority));
			return common::error_info(
				common::error::codes::common_errors::invalid_argument,
				"Priority must be between 0 and 10"
			);
		}

		std::unique_lock lock(mutex_);

		// Check for duplicate route
		if (routes_.find(route_id) != routes_.end()) {
			common::logging::log_error("Add route failed: duplicate route_id " + route_id);
			return common::error_info(
				error::duplicate_subscription,
				"Route already exists: " + route_id
			);
		}

		// Check max routes limit
		if (routes_.size() >= config_.max_routes) {
			common::logging::log_error("Add route failed: max routes limit reached");
			return common::error_info(
				error::queue_full,
				"Maximum number of routes reached"
			);
		}

		// Create internal route
		internal_route route;
		route.info.route_id = route_id;
		route.info.topic_pattern = topic_pattern;
		route.info.priority = priority;
		route.info.active = true;
		route.info.messages_processed = 0;
		route.handler = std::move(handler);

		// Subscribe to topic router
		auto sub_result = router_->subscribe(
			topic_pattern,
			[this, route_id](const message& msg) {
				return handle_route_message(route_id, msg);
			},
			nullptr,
			priority
		);

		if (!sub_result.is_ok()) {
			common::logging::log_error("Add route failed: subscription error - " +
				sub_result.error().message);
			return common::make_error<std::monostate>(
				sub_result.error().code,
				sub_result.error().message,
				"messaging_system"
			);
		}

		route.subscription_id = sub_result.unwrap();
		routes_[route_id] = std::move(route);

		if (config_.enable_statistics) {
			update_active_routes_count();
		}

		common::logging::log_debug("Route added, id: " + route_id +
			", pattern: " + topic_pattern + ", priority: " + std::to_string(priority));

		return common::ok();
	}

	common::VoidResult remove_route(const std::string& route_id) {
		std::unique_lock lock(mutex_);

		auto it = routes_.find(route_id);
		if (it == routes_.end()) {
			common::logging::log_warning("Remove route failed: route not found " + route_id);
			return common::make_error<std::monostate>(
				error::route_not_found,
				"Route not found: " + route_id,
				"messaging_system"
			);
		}

		// Unsubscribe from topic router
		auto unsub_result = router_->unsubscribe(it->second.subscription_id);
		if (!unsub_result.is_ok()) {
			common::logging::log_warning("Failed to unsubscribe route " + route_id +
				": " + unsub_result.error().message);
		}

		routes_.erase(it);

		if (config_.enable_statistics) {
			update_active_routes_count();
		}

		common::logging::log_debug("Route removed, id: " + route_id);
		return common::ok();
	}

	common::VoidResult enable_route(const std::string& route_id) {
		std::unique_lock lock(mutex_);

		auto it = routes_.find(route_id);
		if (it == routes_.end()) {
			common::logging::log_warning("Enable route failed: route not found " + route_id);
			return common::make_error<std::monostate>(
				error::route_not_found,
				"Route not found: " + route_id,
				"messaging_system"
			);
		}

		if (it->second.info.active) {
			common::logging::log_debug("Route already active: " + route_id);
			return common::ok();
		}

		// Re-subscribe to topic router
		auto sub_result = router_->subscribe(
			it->second.info.topic_pattern,
			[this, route_id](const message& msg) {
				return handle_route_message(route_id, msg);
			},
			nullptr,
			it->second.info.priority
		);

		if (!sub_result.is_ok()) {
			common::logging::log_error("Enable route failed: subscription error - " +
				sub_result.error().message);
			return common::make_error<std::monostate>(
				sub_result.error().code,
				sub_result.error().message,
				"messaging_system"
			);
		}

		it->second.subscription_id = sub_result.unwrap();
		it->second.info.active = true;

		if (config_.enable_statistics) {
			update_active_routes_count();
		}

		common::logging::log_debug("Route enabled, id: " + route_id);
		return common::ok();
	}

	common::VoidResult disable_route(const std::string& route_id) {
		std::unique_lock lock(mutex_);

		auto it = routes_.find(route_id);
		if (it == routes_.end()) {
			common::logging::log_warning("Disable route failed: route not found " + route_id);
			return common::make_error<std::monostate>(
				error::route_not_found,
				"Route not found: " + route_id,
				"messaging_system"
			);
		}

		if (!it->second.info.active) {
			common::logging::log_debug("Route already disabled: " + route_id);
			return common::ok();
		}

		// Unsubscribe from topic router
		auto unsub_result = router_->unsubscribe(it->second.subscription_id);
		if (!unsub_result.is_ok()) {
			common::logging::log_warning("Failed to unsubscribe route " + route_id +
				": " + unsub_result.error().message);
		}

		it->second.info.active = false;
		it->second.subscription_id = 0;

		if (config_.enable_statistics) {
			update_active_routes_count();
		}

		common::logging::log_debug("Route disabled, id: " + route_id);
		return common::ok();
	}

	bool has_route(const std::string& route_id) const {
		std::shared_lock lock(mutex_);
		return routes_.find(route_id) != routes_.end();
	}

	common::Result<route_info> get_route(const std::string& route_id) const {
		std::shared_lock lock(mutex_);

		auto it = routes_.find(route_id);
		if (it == routes_.end()) {
			return common::make_error<route_info>(
				error::route_not_found,
				"Route not found: " + route_id,
				"messaging_system"
			);
		}

		return common::ok(it->second.info);
	}

	std::vector<route_info> get_routes() const {
		std::shared_lock lock(mutex_);

		std::vector<route_info> result;
		result.reserve(routes_.size());

		for (const auto& [id, route] : routes_) {
			result.push_back(route.info);
		}

		return result;
	}

	size_t route_count() const {
		std::shared_lock lock(mutex_);
		return routes_.size();
	}

	void clear_routes() {
		std::unique_lock lock(mutex_);

		for (const auto& [id, route] : routes_) {
			if (route.info.active && route.subscription_id != 0) {
				router_->unsubscribe(route.subscription_id);
			}
		}

		routes_.clear();
		router_->clear();

		if (config_.enable_statistics) {
			statistics_.active_routes = 0;
		}

		common::logging::log_debug("All routes cleared");
	}

	// =========================================================================
	// Message Routing
	// =========================================================================

	common::VoidResult route(const message& msg) {
		if (!running_.load()) {
			common::logging::log_debug("Route rejected: broker not running");
			return common::make_error<std::monostate>(
				error::broker_not_started,
				"Message broker is not running"
			);
		}

		if (config_.enable_trace_logging) {
			common::logging::log_trace("Routing message, topic: " + msg.metadata().topic +
				", id: " + msg.metadata().id);
		}

		if (config_.enable_statistics) {
			messages_routed_.fetch_add(1);
		}

		auto result = router_->route(msg);

		if (result.is_ok()) {
			if (config_.enable_statistics) {
				messages_delivered_.fetch_add(1);
			}
		} else {
			// Check if it was a "no subscribers" error vs actual failure
			if (result.error().code == common::error::codes::common_errors::not_found) {
				if (config_.enable_statistics) {
					messages_unrouted_.fetch_add(1);
				}
				if (config_.enable_trace_logging) {
					common::logging::log_trace("No routes for topic: " + msg.metadata().topic);
				}
			} else {
				if (config_.enable_statistics) {
					messages_failed_.fetch_add(1);
				}
				common::logging::log_warning("Route failed for topic: " + msg.metadata().topic +
					", error: " + result.error().message);
			}
		}

		return result;
	}

	// =========================================================================
	// Statistics
	// =========================================================================

	broker_statistics get_statistics() const {
		broker_statistics stats;
		stats.messages_routed = messages_routed_.load();
		stats.messages_delivered = messages_delivered_.load();
		stats.messages_failed = messages_failed_.load();
		stats.messages_unrouted = messages_unrouted_.load();
		stats.active_routes = statistics_.active_routes;
		stats.last_reset = statistics_.last_reset;
		return stats;
	}

	void reset_statistics() {
		messages_routed_.store(0);
		messages_delivered_.store(0);
		messages_failed_.store(0);
		messages_unrouted_.store(0);
		statistics_.last_reset = std::chrono::steady_clock::now();

		// Reset per-route statistics
		std::unique_lock lock(mutex_);
		for (auto& [id, route] : routes_) {
			route.info.messages_processed = 0;
		}

		common::logging::log_debug("Statistics reset");
	}

	// =========================================================================
	// Dead Letter Queue Operations
	// =========================================================================

	common::VoidResult configure_dlq(dlq_config config) {
		std::unique_lock lock(dlq_mutex_);
		dlq_config_ = std::move(config);
		common::logging::log_info("DLQ configured, max_size: " +
			std::to_string(dlq_config_->max_size) +
			", retention: " + std::to_string(dlq_config_->retention_period.count()) + "s");
		return common::ok();
	}

	common::VoidResult move_to_dlq(const message& msg, const std::string& reason) {
		std::unique_lock lock(dlq_mutex_);

		if (!dlq_config_) {
			return common::make_error<std::monostate>(
				error::dlq_not_configured,
				"Dead letter queue not configured"
			);
		}

		// Check if DLQ is full
		if (dlq_entries_.size() >= dlq_config_->max_size) {
			// Notify callback if registered
			if (dlq_full_callback_) {
				lock.unlock();
				dlq_full_callback_(dlq_config_->max_size);
				lock.lock();
			}

			switch (dlq_config_->on_full) {
				case dlq_policy::drop_oldest:
					if (!dlq_entries_.empty()) {
						dlq_entries_.pop_front();
						dlq_total_purged_.fetch_add(1);
						common::logging::log_debug("DLQ full, dropped oldest message");
					}
					break;
				case dlq_policy::drop_newest:
					common::logging::log_warning("DLQ full, rejecting new message: " +
						msg.metadata().id);
					return common::make_error<std::monostate>(
						error::dlq_full,
						"Dead letter queue is full"
					);
				case dlq_policy::block:
					common::logging::log_warning("DLQ full, blocking not implemented yet");
					return common::make_error<std::monostate>(
						error::dlq_full,
						"Dead letter queue is full (blocking mode)"
					);
			}
		}

		// Create DLQ entry
		dlq_entry entry;
		entry.original_message = msg;
		entry.failure_reason = reason;
		entry.failed_at = std::chrono::system_clock::now();
		entry.retry_count = 0;

		dlq_entries_.push_back(entry);
		dlq_total_received_.fetch_add(1);

		common::logging::log_debug("Message moved to DLQ, id: " + msg.metadata().id +
			", reason: " + reason);

		// Notify callback if registered
		if (dlq_message_callback_) {
			lock.unlock();
			dlq_message_callback_(entry);
		}

		return common::ok();
	}

	std::vector<dlq_entry> get_dlq_messages(std::size_t limit) const {
		std::shared_lock lock(dlq_mutex_);

		std::vector<dlq_entry> result;
		std::size_t count = (limit == 0) ? dlq_entries_.size()
			: std::min(limit, dlq_entries_.size());
		result.reserve(count);

		for (std::size_t i = 0; i < count; ++i) {
			result.push_back(dlq_entries_[i]);
		}

		return result;
	}

	std::size_t get_dlq_size() const {
		std::shared_lock lock(dlq_mutex_);
		return dlq_entries_.size();
	}

	common::VoidResult replay_dlq_message(const std::string& message_id) {
		if (!running_.load()) {
			return common::make_error<std::monostate>(
				error::broker_not_started,
				"Message broker is not running"
			);
		}

		dlq_entry entry_to_replay;
		bool found = false;

		{
			std::unique_lock lock(dlq_mutex_);

			for (auto it = dlq_entries_.begin(); it != dlq_entries_.end(); ++it) {
				if (it->original_message.metadata().id == message_id) {
					entry_to_replay = *it;
					dlq_entries_.erase(it);
					found = true;
					break;
				}
			}
		}

		if (!found) {
			return common::make_error<std::monostate>(
				error::dlq_message_not_found,
				"Message not found in DLQ: " + message_id
			);
		}

		// Try to route the message again
		auto result = route(entry_to_replay.original_message);

		if (result.is_ok()) {
			dlq_total_replayed_.fetch_add(1);
			common::logging::log_debug("DLQ message replayed successfully, id: " + message_id);
			return common::ok();
		}

		// If replay failed, put back in DLQ with updated retry count
		{
			std::unique_lock lock(dlq_mutex_);
			entry_to_replay.retry_count++;
			entry_to_replay.last_error = result.error().message;
			dlq_entries_.push_back(entry_to_replay);
		}

		return common::make_error<std::monostate>(
			error::dlq_replay_failed,
			"Failed to replay message: " + result.error().message
		);
	}

	std::size_t replay_all_dlq_messages() {
		if (!running_.load()) {
			return 0;
		}

		std::vector<dlq_entry> entries_to_replay;

		{
			std::unique_lock lock(dlq_mutex_);
			entries_to_replay.reserve(dlq_entries_.size());
			while (!dlq_entries_.empty()) {
				entries_to_replay.push_back(std::move(dlq_entries_.front()));
				dlq_entries_.pop_front();
			}
		}

		std::size_t success_count = 0;
		std::vector<dlq_entry> failed_entries;

		for (auto& entry : entries_to_replay) {
			auto result = route(entry.original_message);

			if (result.is_ok()) {
				success_count++;
				dlq_total_replayed_.fetch_add(1);
			} else {
				entry.retry_count++;
				entry.last_error = result.error().message;
				failed_entries.push_back(std::move(entry));
			}
		}

		// Put failed entries back
		if (!failed_entries.empty()) {
			std::unique_lock lock(dlq_mutex_);
			for (auto& entry : failed_entries) {
				dlq_entries_.push_back(std::move(entry));
			}
		}

		common::logging::log_info("DLQ replay complete, success: " +
			std::to_string(success_count) +
			", failed: " + std::to_string(failed_entries.size()));

		return success_count;
	}

	std::size_t purge_dlq() {
		std::unique_lock lock(dlq_mutex_);
		std::size_t purged = dlq_entries_.size();
		dlq_entries_.clear();
		dlq_total_purged_.fetch_add(purged);
		common::logging::log_info("DLQ purged, count: " + std::to_string(purged));
		return purged;
	}

	std::size_t purge_dlq_older_than(std::chrono::seconds age) {
		std::unique_lock lock(dlq_mutex_);
		auto threshold = std::chrono::system_clock::now() - age;
		std::size_t purged = 0;

		auto it = dlq_entries_.begin();
		while (it != dlq_entries_.end()) {
			if (it->failed_at < threshold) {
				it = dlq_entries_.erase(it);
				purged++;
			} else {
				++it;
			}
		}

		dlq_total_purged_.fetch_add(purged);
		common::logging::log_debug("DLQ purged old entries, count: " + std::to_string(purged));
		return purged;
	}

	dlq_statistics get_dlq_statistics() const {
		std::shared_lock lock(dlq_mutex_);

		dlq_statistics stats;
		stats.current_size = dlq_entries_.size();
		stats.total_received = dlq_total_received_.load();
		stats.total_replayed = dlq_total_replayed_.load();
		stats.total_purged = dlq_total_purged_.load();

		if (!dlq_entries_.empty()) {
			stats.oldest_entry = dlq_entries_.front().failed_at;
		}

		// Count failure reasons
		for (const auto& entry : dlq_entries_) {
			stats.failure_reasons[entry.failure_reason]++;
		}

		return stats;
	}

	void on_dlq_message(dlq_message_callback callback) {
		std::unique_lock lock(dlq_mutex_);
		dlq_message_callback_ = std::move(callback);
	}

	void on_dlq_full(dlq_full_callback callback) {
		std::unique_lock lock(dlq_mutex_);
		dlq_full_callback_ = std::move(callback);
	}

	bool is_dlq_configured() const {
		std::shared_lock lock(dlq_mutex_);
		return dlq_config_.has_value();
	}

private:
	common::VoidResult handle_route_message(const std::string& route_id,
											const message& msg) {
		message_handler handler;

		{
			std::shared_lock lock(mutex_);
			auto it = routes_.find(route_id);
			if (it == routes_.end() || !it->second.info.active) {
				return common::error_info(
					error::route_not_found,
					"Route not found or inactive: " + route_id
				);
			}
			handler = it->second.handler;
		}

		auto result = handler(msg);

		if (result.is_ok()) {
			std::unique_lock lock(mutex_);
			auto it = routes_.find(route_id);
			if (it != routes_.end()) {
				it->second.info.messages_processed++;
			}
		}

		return result;
	}

	void update_active_routes_count() {
		uint64_t active_count = 0;
		for (const auto& [id, route] : routes_) {
			if (route.info.active) {
				active_count++;
			}
		}
		statistics_.active_routes = active_count;
	}

	broker_config config_;
	std::unique_ptr<topic_router> router_;
	std::atomic<bool> running_;

	mutable std::shared_mutex mutex_;
	std::unordered_map<std::string, internal_route> routes_;

	// Statistics (atomic for thread safety)
	std::atomic<uint64_t> messages_routed_{0};
	std::atomic<uint64_t> messages_delivered_{0};
	std::atomic<uint64_t> messages_failed_{0};
	std::atomic<uint64_t> messages_unrouted_{0};
	broker_statistics statistics_;

	// Dead Letter Queue
	mutable std::shared_mutex dlq_mutex_;
	std::optional<dlq_config> dlq_config_;
	std::deque<dlq_entry> dlq_entries_;
	dlq_message_callback dlq_message_callback_;
	dlq_full_callback dlq_full_callback_;

	// DLQ statistics (atomic for thread safety)
	std::atomic<std::size_t> dlq_total_received_{0};
	std::atomic<std::size_t> dlq_total_replayed_{0};
	std::atomic<std::size_t> dlq_total_purged_{0};
};

// =============================================================================
// Message Broker Public Interface Implementation
// =============================================================================

message_broker::message_broker(broker_config config)
	: impl_(std::make_unique<message_broker_impl>(std::move(config))) {
}

message_broker::~message_broker() = default;

message_broker::message_broker(message_broker&&) noexcept = default;
message_broker& message_broker::operator=(message_broker&&) noexcept = default;

common::VoidResult message_broker::start() {
	return impl_->start();
}

common::VoidResult message_broker::stop() {
	return impl_->stop();
}

bool message_broker::is_running() const {
	return impl_->is_running();
}

common::VoidResult message_broker::add_route(const std::string& route_id,
											 const std::string& topic_pattern,
											 message_handler handler,
											 int priority) {
	return impl_->add_route(route_id, topic_pattern, std::move(handler), priority);
}

common::VoidResult message_broker::remove_route(const std::string& route_id) {
	return impl_->remove_route(route_id);
}

common::VoidResult message_broker::enable_route(const std::string& route_id) {
	return impl_->enable_route(route_id);
}

common::VoidResult message_broker::disable_route(const std::string& route_id) {
	return impl_->disable_route(route_id);
}

bool message_broker::has_route(const std::string& route_id) const {
	return impl_->has_route(route_id);
}

common::Result<route_info> message_broker::get_route(const std::string& route_id) const {
	return impl_->get_route(route_id);
}

std::vector<route_info> message_broker::get_routes() const {
	return impl_->get_routes();
}

size_t message_broker::route_count() const {
	return impl_->route_count();
}

void message_broker::clear_routes() {
	impl_->clear_routes();
}

common::VoidResult message_broker::route(const message& msg) {
	return impl_->route(msg);
}

broker_statistics message_broker::get_statistics() const {
	return impl_->get_statistics();
}

void message_broker::reset_statistics() {
	impl_->reset_statistics();
}

common::VoidResult message_broker::configure_dlq(dlq_config config) {
	return impl_->configure_dlq(std::move(config));
}

common::VoidResult message_broker::move_to_dlq(const message& msg,
											   const std::string& reason) {
	return impl_->move_to_dlq(msg, reason);
}

std::vector<dlq_entry> message_broker::get_dlq_messages(std::size_t limit) const {
	return impl_->get_dlq_messages(limit);
}

std::size_t message_broker::get_dlq_size() const {
	return impl_->get_dlq_size();
}

common::VoidResult message_broker::replay_dlq_message(const std::string& message_id) {
	return impl_->replay_dlq_message(message_id);
}

std::size_t message_broker::replay_all_dlq_messages() {
	return impl_->replay_all_dlq_messages();
}

std::size_t message_broker::purge_dlq() {
	return impl_->purge_dlq();
}

std::size_t message_broker::purge_dlq_older_than(std::chrono::seconds age) {
	return impl_->purge_dlq_older_than(age);
}

dlq_statistics message_broker::get_dlq_statistics() const {
	return impl_->get_dlq_statistics();
}

void message_broker::on_dlq_message(dlq_message_callback callback) {
	impl_->on_dlq_message(std::move(callback));
}

void message_broker::on_dlq_full(dlq_full_callback callback) {
	impl_->on_dlq_full(std::move(callback));
}

bool message_broker::is_dlq_configured() const {
	return impl_->is_dlq_configured();
}

}  // namespace kcenon::messaging
