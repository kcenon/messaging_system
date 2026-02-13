#include <kcenon/messaging/patterns/event_streaming.h>

#include <kcenon/messaging/error/messaging_error_category.h>
#include <algorithm>

namespace kcenon::messaging::patterns {

using namespace kcenon::common;
using namespace kcenon::common::error;

// ============================================================================
// event_stream implementation
// ============================================================================

event_stream::event_stream(
	std::shared_ptr<message_bus> bus,
	std::string stream_topic,
	event_stream_config config
)
	: bus_(std::move(bus)),
	  stream_topic_(std::move(stream_topic)),
	  config_(std::move(config)) {
}

event_stream::~event_stream() {
	std::lock_guard lock(subscription_mutex_);
	for (uint64_t sub_id : subscription_ids_) {
		if (bus_) {
			bus_->unsubscribe(sub_id);
		}
	}
}

VoidResult event_stream::publish_event(message event) {
	if (!bus_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!bus_->is_running()) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	// Set topic and type
	event.metadata().topic = stream_topic_;
	event.metadata().type = message_type::event;

	// Buffer event if replay is enabled
	if (config_.enable_replay) {
		buffer_event(event);
	}

	// Publish event
	return bus_->publish(event);
}

Result<uint64_t> event_stream::subscribe(
	subscription_callback callback,
	bool replay_past_events
) {
	return subscribe(std::move(callback), nullptr, replay_past_events);
}

Result<uint64_t> event_stream::subscribe(
	subscription_callback callback,
	message_filter filter,
	bool replay_past_events
) {
	if (!bus_) {
		return Result<uint64_t>::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!bus_->is_running()) {
		return Result<uint64_t>::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!callback) {
		return Result<uint64_t>::err(make_typed_error_code(messaging_error_category::subscription_failed));
	}

	// Replay past events if requested
	if (replay_past_events && config_.enable_replay) {
		replay_buffered_events(callback, filter);
	}

	// Subscribe to future events
	auto sub_result = bus_->subscribe(stream_topic_, callback, filter);
	if (sub_result.is_ok()) {
		std::lock_guard lock(subscription_mutex_);
		subscription_ids_.push_back(sub_result.unwrap());
	}

	return sub_result;
}

VoidResult event_stream::unsubscribe(uint64_t subscription_id) {
	if (!bus_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	auto result = bus_->unsubscribe(subscription_id);
	if (result.is_ok()) {
		std::lock_guard lock(subscription_mutex_);
		auto it = std::find(subscription_ids_.begin(), subscription_ids_.end(),
							subscription_id);
		if (it != subscription_ids_.end()) {
			subscription_ids_.erase(it);
		}
	}

	return result;
}

VoidResult event_stream::replay(
	subscription_callback callback,
	message_filter filter
) {
	if (!callback) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::subscription_failed));
	}

	replay_buffered_events(callback, filter);
	return ok();
}

std::vector<message> event_stream::get_events(message_filter filter) const {
	std::lock_guard lock(buffer_mutex_);
	std::vector<message> events;

	for (const auto& event : event_buffer_) {
		if (!filter || filter(event)) {
			events.push_back(event);
		}
	}

	return events;
}

size_t event_stream::event_count() const {
	std::lock_guard lock(buffer_mutex_);
	return event_buffer_.size();
}

void event_stream::clear_buffer() {
	std::lock_guard lock(buffer_mutex_);
	event_buffer_.clear();
}

void event_stream::buffer_event(const message& event) {
	std::lock_guard lock(buffer_mutex_);

	// Add to buffer
	event_buffer_.push_back(event);

	// Maintain max buffer size
	while (event_buffer_.size() > config_.max_buffer_size) {
		event_buffer_.pop_front();
	}
}

void event_stream::replay_buffered_events(
	const subscription_callback& callback,
	const message_filter& filter
) {
	std::lock_guard lock(buffer_mutex_);

	for (const auto& event : event_buffer_) {
		if (!filter || filter(event)) {
			callback(event);
		}
	}
}

// ============================================================================
// event_batch_processor implementation
// ============================================================================

event_batch_processor::event_batch_processor(
	std::shared_ptr<message_bus> bus,
	std::string topic_pattern,
	batch_callback callback,
	size_t batch_size,
	std::chrono::milliseconds batch_timeout
)
	: bus_(std::move(bus)),
	  topic_pattern_(std::move(topic_pattern)),
	  batch_callback_(std::move(callback)),
	  batch_size_(batch_size),
	  batch_timeout_(batch_timeout),
	  batch_start_time_(std::chrono::steady_clock::now()) {
}

event_batch_processor::~event_batch_processor() {
	stop();
}

VoidResult event_batch_processor::start() {
	if (!bus_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!bus_->is_running()) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!batch_callback_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::subscription_failed));
	}

	if (running_.load()) {
		return ok();  // Already running
	}

	// Subscribe to events
	auto sub_result = bus_->subscribe(
		topic_pattern_,
		[this](const message& msg) -> VoidResult {
			handle_event(msg);
			return ok();
		}
	);

	if (sub_result.is_err()) {
		return VoidResult(sub_result.error());
	}

	subscription_id_ = sub_result.unwrap();
	running_.store(true);
	batch_start_time_ = std::chrono::steady_clock::now();

	// Start background processor
	processor_future_ = std::async(
		std::launch::async,
		[this]() { processor_loop(); }
	);

	return ok();
}

VoidResult event_batch_processor::stop() {
	if (!running_.load()) {
		return ok();  // Already stopped
	}

	running_.store(false);

	// Wait for processor to finish
	if (processor_future_.valid()) {
		processor_future_.wait();
	}

	// Flush remaining events
	flush();

	// Unsubscribe
	if (subscription_id_ != 0 && bus_) {
		auto result = bus_->unsubscribe(subscription_id_);
		subscription_id_ = 0;
		return result;
	}

	return ok();
}

VoidResult event_batch_processor::flush() {
	std::lock_guard lock(batch_mutex_);

	if (current_batch_.empty()) {
		return ok();
	}

	process_batch();
	return ok();
}

void event_batch_processor::handle_event(const message& event) {
	std::lock_guard lock(batch_mutex_);

	current_batch_.push_back(event);

	// Process batch if full
	if (current_batch_.size() >= batch_size_) {
		process_batch();
	}
}

void event_batch_processor::process_batch() {
	if (current_batch_.empty()) {
		return;
	}

	// Process batch
	batch_callback_(current_batch_);

	// Clear batch
	current_batch_.clear();
	batch_start_time_ = std::chrono::steady_clock::now();
}

void event_batch_processor::processor_loop() {
	while (running_.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		std::lock_guard lock(batch_mutex_);

		// Check if batch timeout has elapsed
		auto elapsed = std::chrono::steady_clock::now() - batch_start_time_;
		if (!current_batch_.empty() &&
			elapsed >= std::chrono::duration_cast<std::chrono::steady_clock::duration>(batch_timeout_)) {
			process_batch();
		}
	}
}

}  // namespace kcenon::messaging::patterns
