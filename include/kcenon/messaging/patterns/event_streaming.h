#pragma once

#include "../core/message_bus.h"
#include <kcenon/common/patterns/result.h>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace kcenon::messaging::patterns {

/**
 * @struct event_stream_config
 * @brief Configuration for event streams
 */
struct event_stream_config {
	size_t max_buffer_size = 1000;          // Maximum events to buffer
	bool enable_replay = true;              // Allow replay of past events
	bool enable_persistence = false;        // Persist events to storage
	std::chrono::milliseconds batch_timeout{100};  // Max time to wait for batch
	size_t batch_size = 10;                 // Number of events per batch
};

/**
 * @class event_stream
 * @brief Event streaming and sourcing pattern
 *
 * Provides event sourcing capabilities with event replay, filtering,
 * and batch processing.
 */
class event_stream {
	std::shared_ptr<message_bus> bus_;
	std::string stream_topic_;
	event_stream_config config_;

	// Event buffer for replay
	std::deque<message> event_buffer_;
	mutable std::mutex buffer_mutex_;

	// Active subscriptions
	std::vector<uint64_t> subscription_ids_;
	mutable std::mutex subscription_mutex_;

public:
	/**
	 * @brief Construct an event stream
	 * @param bus Message bus to use
	 * @param stream_topic Topic for this event stream
	 * @param config Stream configuration
	 */
	event_stream(
		std::shared_ptr<message_bus> bus,
		std::string stream_topic,
		event_stream_config config = {}
	);

	/**
	 * @brief Destructor - cleanup subscriptions
	 */
	~event_stream();

	// Non-copyable
	event_stream(const event_stream&) = delete;
	event_stream& operator=(const event_stream&) = delete;

	// Movable
	event_stream(event_stream&&) noexcept = default;
	event_stream& operator=(event_stream&&) noexcept = default;

	/**
	 * @brief Publish an event to the stream
	 * @param event Event message to publish
	 * @return Result indicating success or error
	 */
	common::VoidResult publish_event(message event);

	/**
	 * @brief Subscribe to the event stream
	 * @param callback Callback for each event
	 * @param replay_past_events If true, replay buffered events to new subscriber
	 * @return Subscription ID or error
	 */
	common::Result<uint64_t> subscribe(
		subscription_callback callback,
		bool replay_past_events = false
	);

	/**
	 * @brief Subscribe with event filter
	 * @param callback Callback for matching events
	 * @param filter Filter function
	 * @param replay_past_events If true, replay buffered events to new subscriber
	 * @return Subscription ID or error
	 */
	common::Result<uint64_t> subscribe(
		subscription_callback callback,
		message_filter filter,
		bool replay_past_events = false
	);

	/**
	 * @brief Unsubscribe from the event stream
	 * @param subscription_id Subscription ID to remove
	 * @return Result indicating success or error
	 */
	common::VoidResult unsubscribe(uint64_t subscription_id);

	/**
	 * @brief Replay all buffered events to a callback
	 * @param callback Callback to receive replayed events
	 * @param filter Optional filter for replay
	 * @return Result indicating success or error
	 */
	common::VoidResult replay(
		subscription_callback callback,
		message_filter filter = nullptr
	);

	/**
	 * @brief Get buffered events
	 * @param filter Optional filter
	 * @return Vector of matching events
	 */
	std::vector<message> get_events(message_filter filter = nullptr) const;

	/**
	 * @brief Get number of buffered events
	 * @return Event count
	 */
	size_t event_count() const;

	/**
	 * @brief Clear event buffer
	 */
	void clear_buffer();

	/**
	 * @brief Get stream topic
	 * @return Stream topic string
	 */
	const std::string& get_stream_topic() const { return stream_topic_; }

private:
	/**
	 * @brief Add event to buffer
	 * @param event Event to buffer
	 */
	void buffer_event(const message& event);

	/**
	 * @brief Replay buffered events
	 * @param callback Callback to receive events
	 * @param filter Optional filter
	 */
	void replay_buffered_events(
		const subscription_callback& callback,
		const message_filter& filter
	);
};

/**
 * @class event_batch_processor
 * @brief Processes events in batches
 *
 * Collects events and processes them in batches for efficiency.
 */
class event_batch_processor {
public:
	/**
	 * @brief Batch processing callback type
	 */
	using batch_callback = std::function<common::VoidResult(const std::vector<message>&)>;

private:
	std::shared_ptr<message_bus> bus_;
	std::string topic_pattern_;
	batch_callback batch_callback_;
	size_t batch_size_;
	std::chrono::milliseconds batch_timeout_;

	std::vector<message> current_batch_;
	mutable std::mutex batch_mutex_;
	std::chrono::steady_clock::time_point batch_start_time_;

	uint64_t subscription_id_{0};
	std::future<void> processor_future_;
	std::atomic<bool> running_{false};

public:
	/**
	 * @brief Construct a batch processor
	 * @param bus Message bus to use
	 * @param topic_pattern Topic pattern to subscribe to
	 * @param callback Callback for processing batches
	 * @param batch_size Number of events per batch
	 * @param batch_timeout Maximum time to wait for batch
	 */
	event_batch_processor(
		std::shared_ptr<message_bus> bus,
		std::string topic_pattern,
		batch_callback callback,
		size_t batch_size = 10,
		std::chrono::milliseconds batch_timeout = std::chrono::milliseconds{100}
	);

	/**
	 * @brief Destructor - stop processing
	 */
	~event_batch_processor();

	// Non-copyable
	event_batch_processor(const event_batch_processor&) = delete;
	event_batch_processor& operator=(const event_batch_processor&) = delete;

	// Movable
	event_batch_processor(event_batch_processor&&) noexcept = default;
	event_batch_processor& operator=(event_batch_processor&&) noexcept = default;

	/**
	 * @brief Start batch processing
	 * @return Result indicating success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop batch processing
	 * @return Result indicating success or error
	 */
	common::VoidResult stop();

	/**
	 * @brief Check if processor is running
	 * @return true if running
	 */
	bool is_running() const { return running_.load(); }

	/**
	 * @brief Flush current batch immediately
	 * @return Result indicating success or error
	 */
	common::VoidResult flush();

private:
	/**
	 * @brief Handle incoming event
	 * @param event Event message
	 */
	void handle_event(const message& event);

	/**
	 * @brief Process current batch
	 */
	void process_batch();

	/**
	 * @brief Background processor loop
	 */
	void processor_loop();
};

}  // namespace kcenon::messaging::patterns
