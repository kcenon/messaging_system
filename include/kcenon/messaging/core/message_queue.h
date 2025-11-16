#pragma once

#include "../interfaces/queue_interface.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <variant>

namespace kcenon::messaging {

/**
 * @struct queue_config
 * @brief Configuration for message queue
 */
struct queue_config {
	size_t max_size = 10000;
	bool enable_priority = false;
	bool enable_persistence = false;
	bool drop_on_full = false;  // Drop oldest on full vs reject
};

/**
 * @class message_queue
 * @brief Thread-safe message queue implementation
 */
class message_queue : public queue_interface {
	struct priority_comparator {
		bool operator()(const message& a, const message& b) const {
			return static_cast<int>(a.metadata().priority) <
				   static_cast<int>(b.metadata().priority);
		}
	};

	queue_config config_;

	// Use priority_queue if enabled, else regular queue
	std::variant<
		std::queue<message>,
		std::priority_queue<message, std::vector<message>, priority_comparator>
	> queue_;

	mutable std::mutex mutex_;
	std::condition_variable cv_;
	std::atomic<bool> stopped_{false};

public:
	explicit message_queue(queue_config config = {});
	~message_queue() override;

	// queue_interface implementation
	common::VoidResult enqueue(message msg) override;
	common::Result<message> dequeue(std::chrono::milliseconds timeout) override;
	common::Result<message> try_dequeue() override;

	size_t size() const override;
	bool empty() const override;
	void clear() override;

	// Lifecycle
	void stop();
	bool is_stopped() const { return stopped_.load(); }

private:
	size_t get_queue_size() const;
	void push_to_queue(message msg);
	std::optional<message> pop_from_queue();
};

}  // namespace kcenon::messaging
