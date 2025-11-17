// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/core/topic_router.h>

#include <kcenon/common/error/error_codes.h>
#include <kcenon/messaging/error/error_codes.h>

#include <algorithm>
#include <sstream>

namespace kcenon::messaging {

// Helper function to split topic string by '.'
std::vector<std::string> topic_router::split_topic(
	const std::string& str) const {
	std::vector<std::string> segments;
	std::istringstream iss(str);
	std::string segment;

	while (std::getline(iss, segment, '.')) {
		segments.push_back(segment);
	}

	return segments;
}

// Pattern matching implementation
bool topic_router::match_pattern(const std::string& topic,
								 const std::string& pattern) const {
	// Exact match
	if (topic == pattern) {
		return true;
	}

	auto topic_segments = split_topic(topic);
	auto pattern_segments = split_topic(pattern);

	size_t topic_idx = 0;
	size_t pattern_idx = 0;

	while (topic_idx < topic_segments.size() &&
		   pattern_idx < pattern_segments.size()) {
		const auto& pattern_seg = pattern_segments[pattern_idx];

		if (pattern_seg == "#") {
			// '#' matches zero or more levels
			// If it's the last pattern segment, it matches everything
			// remaining
			if (pattern_idx == pattern_segments.size() - 1) {
				return true;
			}

			// Try matching the rest of the pattern from each remaining topic
			// position
			for (size_t i = topic_idx; i < topic_segments.size(); ++i) {
				std::string remaining_topic;
				for (size_t j = i; j < topic_segments.size(); ++j) {
					if (!remaining_topic.empty()) {
						remaining_topic += ".";
					}
					remaining_topic += topic_segments[j];
				}

				std::string remaining_pattern;
				for (size_t j = pattern_idx + 1; j < pattern_segments.size();
					 ++j) {
					if (!remaining_pattern.empty()) {
						remaining_pattern += ".";
					}
					remaining_pattern += pattern_segments[j];
				}

				if (match_pattern(remaining_topic, remaining_pattern)) {
					return true;
				}
			}
			return false;
		} else if (pattern_seg == "*") {
			// '*' matches exactly one level
			++topic_idx;
			++pattern_idx;
		} else {
			// Exact segment match required
			if (topic_segments[topic_idx] != pattern_seg) {
				return false;
			}
			++topic_idx;
			++pattern_idx;
		}
	}

	// Both must be fully consumed for a match
	return topic_idx == topic_segments.size() &&
		   pattern_idx == pattern_segments.size();
}

// Helper function to split topic (same as member function)
static std::vector<std::string> split_topic_segments(const std::string& str) {
	std::vector<std::string> segments;
	std::istringstream iss(str);
	std::string segment;

	while (std::getline(iss, segment, '.')) {
		segments.push_back(segment);
	}

	return segments;
}

// Helper function for pattern matching (free function)
static bool match_topic_pattern(const std::string& topic,
								const std::string& pattern) {
	// Exact match
	if (topic == pattern) {
		return true;
	}

	auto topic_segments = split_topic_segments(topic);
	auto pattern_segments = split_topic_segments(pattern);

	size_t topic_idx = 0;
	size_t pattern_idx = 0;

	while (topic_idx < topic_segments.size() &&
		   pattern_idx < pattern_segments.size()) {
		const auto& pattern_seg = pattern_segments[pattern_idx];

		if (pattern_seg == "#") {
			// '#' matches zero or more levels
			if (pattern_idx == pattern_segments.size() - 1) {
				return true;
			}

			// Try matching the rest
			for (size_t i = topic_idx; i < topic_segments.size(); ++i) {
				std::string remaining_topic;
				for (size_t j = i; j < topic_segments.size(); ++j) {
					if (!remaining_topic.empty()) {
						remaining_topic += ".";
					}
					remaining_topic += topic_segments[j];
				}

				std::string remaining_pattern;
				for (size_t j = pattern_idx + 1; j < pattern_segments.size();
					 ++j) {
					if (!remaining_pattern.empty()) {
						remaining_pattern += ".";
					}
					remaining_pattern += pattern_segments[j];
				}

				if (match_topic_pattern(remaining_topic, remaining_pattern)) {
					return true;
				}
			}
			return false;
		} else if (pattern_seg == "*") {
			// '*' matches exactly one level
			++topic_idx;
			++pattern_idx;
		} else {
			// Exact segment match required
			if (topic_segments[topic_idx] != pattern_seg) {
				return false;
			}
			++topic_idx;
			++pattern_idx;
		}
	}

	// Both must be fully consumed for a match
	return topic_idx == topic_segments.size() &&
		   pattern_idx == pattern_segments.size();
}

// Subscription::matches implementation
bool subscription::matches(const std::string& topic) const {
	return match_topic_pattern(topic, topic_pattern);
}

common::Result<uint64_t> topic_router::subscribe(
	const std::string& pattern,
	subscription_callback callback,
	message_filter filter,
	int priority) {
	if (!callback) {
		return common::error_info(
			common::error::codes::common_errors::invalid_argument,
			"Callback cannot be null");
	}

	if (pattern.empty()) {
		return common::error_info(
			common::error::codes::common_errors::invalid_argument,
			"Topic pattern cannot be empty");
	}

	if (priority < 0 || priority > 10) {
		return common::error_info(
			common::error::codes::common_errors::invalid_argument,
			"Priority must be between 0 and 10");
	}

	std::unique_lock lock(mutex_);

	uint64_t id = next_id_.fetch_add(1);

	subscription sub{id, pattern, std::move(callback), std::move(filter),
					 priority};

	subscriptions_[pattern].push_back(std::move(sub));

	// Sort by priority (higher first)
	std::sort(subscriptions_[pattern].begin(), subscriptions_[pattern].end(),
			  [](const subscription& a, const subscription& b) {
				  return a.priority > b.priority;
			  });

	return id;
}

common::VoidResult topic_router::unsubscribe(uint64_t subscription_id) {
	std::unique_lock lock(mutex_);

	for (auto& [pattern, subs] : subscriptions_) {
		auto it = std::find_if(subs.begin(), subs.end(),
							   [subscription_id](const subscription& sub) {
								   return sub.id == subscription_id;
							   });

		if (it != subs.end()) {
			subs.erase(it);

			// Clean up empty patterns
			if (subs.empty()) {
				subscriptions_.erase(pattern);
			}

			return common::ok();
		}
	}

	return common::make_error<std::monostate>(
		error::subscription_not_found,
		"Subscription not found: " + std::to_string(subscription_id),
		"messaging_system");
}

std::vector<const subscription*> topic_router::find_matching_subscriptions(
	const std::string& topic) const {
	std::vector<const subscription*> matches;

	std::shared_lock lock(mutex_);

	for (const auto& [pattern, subs] : subscriptions_) {
		if (match_pattern(topic, pattern)) {
			for (const auto& sub : subs) {
				matches.push_back(&sub);
			}
		}
	}

	// Sort by priority (higher first)
	std::sort(matches.begin(), matches.end(),
			  [](const subscription* a, const subscription* b) {
				  return a->priority > b->priority;
			  });

	return matches;
}

common::VoidResult topic_router::route(const message& msg) {
	const std::string& topic = msg.metadata().topic;

	if (topic.empty()) {
		return common::error_info(
			common::error::codes::common_errors::invalid_argument,
			"Message topic cannot be empty");
	}

	auto matching_subs = find_matching_subscriptions(topic);

	if (matching_subs.empty()) {
		return common::error_info(
			common::error::codes::common_errors::not_found,
			"No subscribers found for topic: " + topic);
	}

	// First, filter subscribers
	std::vector<const subscription*> filtered_subs;
	for (const auto* sub : matching_subs) {
		// Apply filter if present
		if (sub->filter && !sub->filter(msg)) {
			continue;
		}
		filtered_subs.push_back(sub);
	}

	// If no subscribers remain after filtering, return error
	if (filtered_subs.empty()) {
		return common::error_info(
			common::error::codes::common_errors::not_found,
			"No subscribers found for topic after filtering: " + topic);
	}

	bool any_succeeded = false;
	std::string error_messages;

	for (const auto* sub : filtered_subs) {
		auto result = sub->callback(msg);
		if (result.is_ok()) {
			any_succeeded = true;
		} else {
			if (!error_messages.empty()) {
				error_messages += "; ";
			}
			error_messages += "Subscription " + std::to_string(sub->id) +
							  " failed: " + result.error().message;
		}
	}

	if (!any_succeeded && !error_messages.empty()) {
		return common::error_info(
			common::error::codes::common_errors::internal_error,
			"All subscribers failed: " + error_messages);
	}

	return common::ok();
}

size_t topic_router::subscriber_count(const std::string& topic) const {
	auto matching_subs = find_matching_subscriptions(topic);
	return matching_subs.size();
}

std::vector<std::string> topic_router::get_topics() const {
	std::shared_lock lock(mutex_);

	std::vector<std::string> topics;
	topics.reserve(subscriptions_.size());

	for (const auto& [pattern, _] : subscriptions_) {
		topics.push_back(pattern);
	}

	return topics;
}

void topic_router::clear() {
	std::unique_lock lock(mutex_);
	subscriptions_.clear();
}

}  // namespace kcenon::messaging
