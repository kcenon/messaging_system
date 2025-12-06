#include <kcenon/messaging/patterns/message_pipeline.h>

#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>
#include <algorithm>
#include <thread>

namespace kcenon::messaging::patterns {

using namespace kcenon::common;
using namespace kcenon::common::error;

// ============================================================================
// message_pipeline implementation
// ============================================================================

message_pipeline::message_pipeline(
	std::shared_ptr<message_bus> bus,
	std::string input_topic,
	std::string output_topic
)
	: bus_(std::move(bus)),
	  input_topic_(std::move(input_topic)),
	  output_topic_(std::move(output_topic)) {
}

message_pipeline::~message_pipeline() {
	stop();
}

message_pipeline& message_pipeline::add_stage(
	std::string name,
	message_processor processor,
	bool optional
) {
	std::lock_guard lock(stages_mutex_);
	stages_.emplace_back(std::move(name), std::move(processor), optional);
	return *this;
}

VoidResult message_pipeline::remove_stage(const std::string& name) {
	std::lock_guard lock(stages_mutex_);

	auto it = std::find_if(stages_.begin(), stages_.end(),
						   [&name](const pipeline_stage& stage) {
							   return stage.name == name;
						   });

	if (it == stages_.end()) {
		return VoidResult(
			error_info{messaging::error::invalid_topic_pattern,
					   "Stage not found: " + name});
	}

	stages_.erase(it);
	return ok();
}

VoidResult message_pipeline::start() {
	if (!bus_) {
		return VoidResult(
			error_info{messaging::error::broker_unavailable,
					   "Message bus is not available"});
	}

	if (!bus_->is_running()) {
		return VoidResult(
			error_info{messaging::error::broker_unavailable,
					   "Message bus is not running"});
	}

	if (running_.load()) {
		return ok();  // Already running
	}

	// Subscribe to input topic
	auto sub_result = bus_->subscribe(
		input_topic_,
		[this](const message& msg) -> VoidResult {
			handle_message(msg);
			return ok();
		}
	);

	if (sub_result.is_err()) {
		return VoidResult(sub_result.error());
	}

	subscription_id_ = sub_result.unwrap();
	running_.store(true);

	return ok();
}

VoidResult message_pipeline::stop() {
	if (!running_.load()) {
		return ok();  // Already stopped
	}

	running_.store(false);

	if (subscription_id_ != 0 && bus_) {
		auto result = bus_->unsubscribe(subscription_id_);
		subscription_id_ = 0;
		return result;
	}

	return ok();
}

Result<message> message_pipeline::process(message msg) const {
	std::lock_guard lock(stages_mutex_);

	// Track processing attempt
	stats_.messages_processed.fetch_add(1, std::memory_order_relaxed);

	message current = std::move(msg);

	// Process through each stage
	for (const auto& stage : stages_) {
		auto result = stage.processor(current);

		if (result.is_err()) {
			if (stage.optional) {
				// Skip optional stage failures
				continue;
			} else {
				// Required stage failed, abort pipeline
				stats_.messages_failed.fetch_add(1, std::memory_order_relaxed);
				return result;
			}
		}

		current = result.unwrap();
	}

	// Successfully processed through all stages
	stats_.messages_succeeded.fetch_add(1, std::memory_order_relaxed);
	return current;
}

size_t message_pipeline::stage_count() const {
	std::lock_guard lock(stages_mutex_);
	return stages_.size();
}

std::vector<std::string> message_pipeline::get_stage_names() const {
	std::lock_guard lock(stages_mutex_);
	std::vector<std::string> names;
	names.reserve(stages_.size());

	for (const auto& stage : stages_) {
		names.push_back(stage.name);
	}

	return names;
}

message_pipeline::statistics_snapshot message_pipeline::get_statistics() const {
	return {
		stats_.messages_processed.load(std::memory_order_relaxed),
		stats_.messages_succeeded.load(std::memory_order_relaxed),
		stats_.messages_failed.load(std::memory_order_relaxed),
		stats_.stage_failures.load(std::memory_order_relaxed)
	};
}

void message_pipeline::reset_statistics() {
	stats_.messages_processed.store(0, std::memory_order_relaxed);
	stats_.messages_succeeded.store(0, std::memory_order_relaxed);
	stats_.messages_failed.store(0, std::memory_order_relaxed);
	stats_.stage_failures.store(0, std::memory_order_relaxed);
}

void message_pipeline::handle_message(const message& msg) {
	// Note: process() already tracks messages_processed, messages_succeeded, messages_failed
	auto result = process(msg);

	if (result.is_ok()) {
		// Publish to output topic
		auto pub_result = bus_->publish(output_topic_, result.unwrap());
		if (!pub_result.is_ok()) {
			// Process succeeded but publish failed
			// Adjust stats: decrement succeeded, increment failed
			stats_.messages_succeeded.fetch_sub(1, std::memory_order_relaxed);
			stats_.messages_failed.fetch_add(1, std::memory_order_relaxed);
		}
	}
}

// ============================================================================
// pipeline_builder implementation
// ============================================================================

pipeline_builder::pipeline_builder(std::shared_ptr<message_bus> bus)
	: bus_(std::move(bus)) {
}

pipeline_builder& pipeline_builder::from(std::string topic) {
	input_topic_ = std::move(topic);
	return *this;
}

pipeline_builder& pipeline_builder::to(std::string topic) {
	output_topic_ = std::move(topic);
	return *this;
}

pipeline_builder& pipeline_builder::add_stage(
	std::string name,
	message_processor processor,
	bool optional
) {
	stages_.emplace_back(std::move(name), std::move(processor), optional);
	return *this;
}

pipeline_builder& pipeline_builder::add_filter(
	std::string name,
	std::function<bool(const message&)> filter
) {
	return add_stage(
		std::move(name),
		[filter = std::move(filter)](const message& msg) -> Result<message> {
			if (filter(msg)) {
				return msg;
			} else {
				return Result<message>(
					error_info{messaging::error::message_rejected,
							   "Message filtered out"});
			}
		}
	);
}

pipeline_builder& pipeline_builder::add_transformer(
	std::string name,
	std::function<message(const message&)> transformer
) {
	return add_stage(
		std::move(name),
		[transformer = std::move(transformer)](const message& msg) -> Result<message> {
			return transformer(msg);
		}
	);
}

Result<std::unique_ptr<message_pipeline>> pipeline_builder::build() {
	if (!bus_) {
		return Result<std::unique_ptr<message_pipeline>>(
			error_info{messaging::error::broker_unavailable,
					   "Message bus is not available"});
	}

	if (input_topic_.empty()) {
		return Result<std::unique_ptr<message_pipeline>>(
			error_info{messaging::error::invalid_topic_pattern,
					   "Input topic not set"});
	}

	if (output_topic_.empty()) {
		return Result<std::unique_ptr<message_pipeline>>(
			error_info{messaging::error::invalid_topic_pattern,
					   "Output topic not set"});
	}

	auto pipeline = std::make_unique<message_pipeline>(bus_, input_topic_, output_topic_);

	for (auto& stage : stages_) {
		pipeline->add_stage(
			std::move(stage.name),
			std::move(stage.processor),
			stage.optional
		);
	}

	return pipeline;
}

// ============================================================================
// pipeline_stages implementations
// ============================================================================

namespace pipeline_stages {

message_processor create_logging_stage(const std::string& stage_name) {
	return [stage_name](const message& msg) -> Result<message> {
		// Log message passing through the pipeline stage using common_system logging
		logging::log_debug("Pipeline stage '" + stage_name + "' processing message: "
			+ msg.get_topic());
		return msg;
	};
}

message_processor create_validation_stage(
	std::function<bool(const message&)> validator
) {
	return [validator = std::move(validator)](const message& msg) -> Result<message> {
		if (validator(msg)) {
			return msg;
		} else {
			return Result<message>(
				error_info{messaging::error::invalid_message,
						   "Message validation failed"});
		}
	};
}

message_processor create_enrichment_stage(
	std::function<void(message&)> enricher
) {
	return [enricher = std::move(enricher)](const message& msg) -> Result<message> {
		message enriched = msg;
		enricher(enriched);
		return enriched;
	};
}

message_processor create_retry_stage(
	message_processor processor,
	size_t max_retries,
	std::chrono::milliseconds retry_delay
) {
	return [processor = std::move(processor), max_retries, retry_delay]
		   (const message& msg) -> Result<message> {
		Result<message> result = Result<message>(
			error_info{messaging::error::publication_failed,
					   "No attempts made"});

		for (size_t attempt = 0; attempt <= max_retries; ++attempt) {
			result = processor(msg);

			if (result.is_ok()) {
				return result;
			}

			// Wait before retry (except on last attempt)
			if (attempt < max_retries) {
				std::this_thread::sleep_for(retry_delay);
			}
		}

		return result;  // Return last error
	};
}

}  // namespace pipeline_stages

}  // namespace kcenon::messaging::patterns
