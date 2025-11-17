#include <kcenon/messaging/patterns/pub_sub.h>

#include <kcenon/messaging/error/error_codes.h>

namespace kcenon::messaging::patterns {

using namespace kcenon::common;
using namespace kcenon::common::error;

// ============================================================================
// publisher implementation
// ============================================================================

publisher::publisher(std::shared_ptr<message_bus> bus, std::string default_topic)
	: bus_(std::move(bus)), default_topic_(std::move(default_topic)) {
}

VoidResult publisher::publish(message msg) {
	if (!bus_) {
		return VoidResult(error_info{messaging::error::broker_unavailable,
						  "Message bus is not available"});
	}

	if (!bus_->is_running()) {
		return VoidResult(error_info{messaging::error::broker_unavailable,
						  "Message bus is not running"});
	}

	if (default_topic_.empty()) {
		return VoidResult(error_info{messaging::error::invalid_topic_pattern,
						  "No default topic set and message has no topic"});
	}

	// Set the topic if not already set
	if (msg.metadata().topic.empty()) {
		msg.metadata().topic = default_topic_;
	}

	return bus_->publish(msg);
}

VoidResult publisher::publish(const std::string& topic, message msg) {
	if (!bus_) {
		return VoidResult(error_info{messaging::error::broker_unavailable,
						  "Message bus is not available"});
	}

	if (!bus_->is_running()) {
		return VoidResult(error_info{messaging::error::broker_unavailable,
						  "Message bus is not running"});
	}

	if (topic.empty()) {
		return VoidResult(error_info{messaging::error::invalid_topic_pattern,
						  "Topic cannot be empty"});
	}

	return bus_->publish(topic, std::move(msg));
}

// ============================================================================
// subscriber implementation
// ============================================================================

subscriber::subscriber(std::shared_ptr<message_bus> bus)
	: bus_(std::move(bus)) {
}

subscriber::~subscriber() {
	// Unsubscribe from all topics
	unsubscribe_all();
}

Result<uint64_t> subscriber::subscribe(
	const std::string& topic_pattern,
	subscription_callback callback,
	message_filter filter,
	int priority
) {
	if (!bus_) {
		return Result<uint64_t>(error_info{messaging::error::broker_unavailable,
						  "Message bus is not available"});
	}

	if (!bus_->is_running()) {
		return Result<uint64_t>(error_info{messaging::error::broker_unavailable,
						  "Message bus is not running"});
	}

	if (topic_pattern.empty()) {
		return Result<uint64_t>(error_info{messaging::error::invalid_topic_pattern,
						  "Topic pattern cannot be empty"});
	}

	if (!callback) {
		return Result<uint64_t>(error_info{messaging::error::subscription_failed,
						  "Callback cannot be null"});
	}

	auto result = bus_->subscribe(topic_pattern, callback, filter, priority);
	if (result.is_ok()) {
		std::lock_guard lock(mutex_);
		subscription_ids_.push_back(result.unwrap());
	}

	return result;
}

VoidResult subscriber::unsubscribe(uint64_t subscription_id) {
	if (!bus_) {
		return VoidResult(error_info{messaging::error::broker_unavailable,
						  "Message bus is not available"});
	}

	auto result = bus_->unsubscribe(subscription_id);
	if (result.is_ok()) {
		std::lock_guard lock(mutex_);
		auto it = std::find(subscription_ids_.begin(), subscription_ids_.end(),
							subscription_id);
		if (it != subscription_ids_.end()) {
			subscription_ids_.erase(it);
		}
	}

	return result;
}

VoidResult subscriber::unsubscribe_all() {
	if (!bus_) {
		return ok();  // Nothing to do if bus is not available
	}

	std::lock_guard lock(mutex_);
	VoidResult last_error = ok();

	for (uint64_t sub_id : subscription_ids_) {
		auto result = bus_->unsubscribe(sub_id);
		if (result.is_err()) {
			last_error = result;  // Keep track of last error
		}
	}

	subscription_ids_.clear();
	return last_error;
}

size_t subscriber::subscription_count() const {
	std::lock_guard lock(mutex_);
	return subscription_ids_.size();
}

}  // namespace kcenon::messaging::patterns
