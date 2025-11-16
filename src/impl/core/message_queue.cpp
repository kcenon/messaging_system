#include "kcenon/messaging/core/message_queue.h"
#include "kcenon/messaging/error/error_codes.h"

#include <algorithm>

namespace kcenon::messaging {

message_queue::message_queue(queue_config config)
	: config_(config) {
	if (config_.enable_priority) {
		queue_ = std::priority_queue<message, std::vector<message>, priority_comparator>();
	} else {
		queue_ = std::queue<message>();
	}
}

message_queue::~message_queue() {
	stop();
}

common::VoidResult message_queue::enqueue(message msg) {
	std::unique_lock lock(mutex_);

	if (stopped_.load()) {
		return common::VoidResult(
			common::error_info{error::queue_stopped,
							   "Queue has been stopped"});
	}

	size_t current_size = get_queue_size();
	if (current_size >= config_.max_size) {
		if (config_.drop_on_full) {
			// Drop oldest message (only for regular queue)
			if (std::holds_alternative<std::queue<message>>(queue_)) {
				auto& q = std::get<std::queue<message>>(queue_);
				if (!q.empty()) {
					q.pop();
				}
			} else {
				// For priority queue, cannot drop specific message
				return common::VoidResult(
					common::error_info{error::queue_full,
									   "Priority queue is full"});
			}
		} else {
			return common::VoidResult(
				common::error_info{error::queue_full,
								   "Queue is full"});
		}
	}

	push_to_queue(std::move(msg));
	cv_.notify_one();

	return common::ok();
}

common::Result<message> message_queue::dequeue(std::chrono::milliseconds timeout) {
	std::unique_lock lock(mutex_);

	auto wait_until = std::chrono::steady_clock::now() + timeout;

	while (get_queue_size() == 0 && !stopped_.load()) {
		if (timeout == std::chrono::milliseconds::max()) {
			cv_.wait(lock);
		} else {
			if (cv_.wait_until(lock, wait_until) == std::cv_status::timeout) {
				return common::Result<message>(
					common::error_info{error::queue_empty,
									   "Queue is empty (timeout)"});
			}
		}
	}

	if (stopped_.load()) {
		return common::Result<message>(
			common::error_info{error::queue_stopped,
							   "Queue has been stopped"});
	}

	auto msg_opt = pop_from_queue();
	if (!msg_opt.has_value()) {
		return common::Result<message>(
			common::error_info{error::dequeue_failed,
							   "Failed to dequeue message"});
	}

	return common::ok(std::move(msg_opt.value()));
}

common::Result<message> message_queue::try_dequeue() {
	std::unique_lock lock(mutex_);

	if (get_queue_size() == 0) {
		return common::Result<message>(
			common::error_info{error::queue_empty,
							   "Queue is empty"});
	}

	if (stopped_.load()) {
		return common::Result<message>(
			common::error_info{error::queue_stopped,
							   "Queue has been stopped"});
	}

	auto msg_opt = pop_from_queue();
	if (!msg_opt.has_value()) {
		return common::Result<message>(
			common::error_info{error::dequeue_failed,
							   "Failed to dequeue message"});
	}

	return common::ok(std::move(msg_opt.value()));
}

size_t message_queue::size() const {
	std::lock_guard lock(mutex_);
	return get_queue_size();
}

bool message_queue::empty() const {
	std::lock_guard lock(mutex_);
	return get_queue_size() == 0;
}

void message_queue::clear() {
	std::lock_guard lock(mutex_);

	if (std::holds_alternative<std::queue<message>>(queue_)) {
		std::get<std::queue<message>>(queue_) = std::queue<message>();
	} else {
		std::get<std::priority_queue<message, std::vector<message>, priority_comparator>>(queue_)
			= std::priority_queue<message, std::vector<message>, priority_comparator>();
	}
}

void message_queue::stop() {
	{
		std::lock_guard lock(mutex_);
		stopped_.store(true);
	}
	cv_.notify_all();
}

// Private methods

size_t message_queue::get_queue_size() const {
	if (std::holds_alternative<std::queue<message>>(queue_)) {
		return std::get<std::queue<message>>(queue_).size();
	} else {
		return std::get<std::priority_queue<message, std::vector<message>, priority_comparator>>(queue_).size();
	}
}

void message_queue::push_to_queue(message msg) {
	if (std::holds_alternative<std::queue<message>>(queue_)) {
		std::get<std::queue<message>>(queue_).push(std::move(msg));
	} else {
		std::get<std::priority_queue<message, std::vector<message>, priority_comparator>>(queue_).push(std::move(msg));
	}
}

std::optional<message> message_queue::pop_from_queue() {
	if (std::holds_alternative<std::queue<message>>(queue_)) {
		auto& q = std::get<std::queue<message>>(queue_);
		if (q.empty()) {
			return std::nullopt;
		}
		message msg = std::move(q.front());
		q.pop();
		return msg;
	} else {
		auto& q = std::get<std::priority_queue<message, std::vector<message>, priority_comparator>>(queue_);
		if (q.empty()) {
			return std::nullopt;
		}
		message msg = std::move(const_cast<message&>(q.top()));
		q.pop();
		return msg;
	}
}

}  // namespace kcenon::messaging
