#pragma once

#include "../core/message_bus.h"
#include <kcenon/common/patterns/result.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace kcenon::messaging::patterns {

/**
 * @brief Message processor function type
 * Takes a message and returns a transformed message or error
 */
using message_processor =
	std::function<common::Result<message>(const message&)>;

/**
 * @class message_pipeline
 * @brief Sequential message processing pipeline
 *
 * Implements the pipes-and-filters pattern for message processing.
 * Messages flow through a series of processing stages, where each
 * stage can transform, filter, or enrich the message.
 */
class message_pipeline {
public:
	/**
	 * @struct pipeline_stage
	 * @brief Represents a single processing stage in the pipeline
	 */
	struct pipeline_stage {
		std::string name;
		message_processor processor;
		bool optional;  // If true, stage failures won't stop pipeline

		pipeline_stage(
			std::string stage_name,
			message_processor proc,
			bool is_optional = false
		)
			: name(std::move(stage_name)),
			  processor(std::move(proc)),
			  optional(is_optional) {
		}
	};

private:
	std::shared_ptr<message_bus> bus_;
	std::string input_topic_;
	std::string output_topic_;
	std::vector<pipeline_stage> stages_;
	mutable std::mutex stages_mutex_;

	uint64_t subscription_id_{0};
	std::atomic<bool> running_{false};

	// Statistics
	struct statistics {
		std::atomic<uint64_t> messages_processed{0};
		std::atomic<uint64_t> messages_succeeded{0};
		std::atomic<uint64_t> messages_failed{0};
		std::atomic<uint64_t> stage_failures{0};
	};
	mutable statistics stats_;

public:
	/**
	 * @brief Construct a message pipeline
	 * @param bus Message bus to use
	 * @param input_topic Topic to consume messages from
	 * @param output_topic Topic to publish processed messages to
	 */
	message_pipeline(
		std::shared_ptr<message_bus> bus,
		std::string input_topic,
		std::string output_topic
	);

	/**
	 * @brief Destructor - stop pipeline
	 */
	~message_pipeline();

	// Non-copyable, non-movable (due to mutex)
	message_pipeline(const message_pipeline&) = delete;
	message_pipeline& operator=(const message_pipeline&) = delete;
	message_pipeline(message_pipeline&&) = delete;
	message_pipeline& operator=(message_pipeline&&) = delete;

	/**
	 * @brief Add a processing stage to the pipeline
	 * @param name Stage name for logging/debugging
	 * @param processor Processor function
	 * @param optional If true, stage failures won't stop pipeline
	 * @return Reference to this pipeline for chaining
	 */
	message_pipeline& add_stage(
		std::string name,
		message_processor processor,
		bool optional = false
	);

	/**
	 * @brief Remove a stage by name
	 * @param name Stage name to remove
	 * @return Result indicating success or error
	 */
	common::VoidResult remove_stage(const std::string& name);

	/**
	 * @brief Start the pipeline
	 * @return Result indicating success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the pipeline
	 * @return Result indicating success or error
	 */
	common::VoidResult stop();

	/**
	 * @brief Check if pipeline is running
	 * @return true if running
	 */
	bool is_running() const { return running_.load(); }

	/**
	 * @brief Process a single message through the pipeline
	 * @param msg Message to process
	 * @return Processed message or error
	 */
	common::Result<message> process(message msg) const;

	/**
	 * @brief Get number of stages in pipeline
	 * @return Stage count
	 */
	size_t stage_count() const;

	/**
	 * @brief Get stage names
	 * @return Vector of stage names
	 */
	std::vector<std::string> get_stage_names() const;

	/**
	 * @brief Get pipeline statistics
	 */
	struct statistics_snapshot {
		uint64_t messages_processed;
		uint64_t messages_succeeded;
		uint64_t messages_failed;
		uint64_t stage_failures;
	};

	statistics_snapshot get_statistics() const;
	void reset_statistics();

private:
	/**
	 * @brief Handle incoming message
	 * @param msg Message to process
	 */
	void handle_message(const message& msg);
};

/**
 * @class pipeline_builder
 * @brief Builder pattern for constructing message pipelines
 */
class pipeline_builder {
	std::shared_ptr<message_bus> bus_;
	std::string input_topic_;
	std::string output_topic_;
	std::vector<message_pipeline::pipeline_stage> stages_;

public:
	/**
	 * @brief Construct a pipeline builder
	 * @param bus Message bus to use
	 */
	explicit pipeline_builder(std::shared_ptr<message_bus> bus);

	/**
	 * @brief Set input topic
	 * @param topic Input topic
	 * @return Reference to this builder for chaining
	 */
	pipeline_builder& from(std::string topic);

	/**
	 * @brief Set output topic
	 * @param topic Output topic
	 * @return Reference to this builder for chaining
	 */
	pipeline_builder& to(std::string topic);

	/**
	 * @brief Add a processing stage
	 * @param name Stage name
	 * @param processor Processor function
	 * @param optional If true, stage failures won't stop pipeline
	 * @return Reference to this builder for chaining
	 */
	pipeline_builder& add_stage(
		std::string name,
		message_processor processor,
		bool optional = false
	);

	/**
	 * @brief Add a filter stage
	 * @param name Stage name
	 * @param filter Filter function (returns true to keep message)
	 * @return Reference to this builder for chaining
	 */
	pipeline_builder& add_filter(
		std::string name,
		std::function<bool(const message&)> filter
	);

	/**
	 * @brief Add a transformation stage
	 * @param name Stage name
	 * @param transformer Transformation function
	 * @return Reference to this builder for chaining
	 */
	pipeline_builder& add_transformer(
		std::string name,
		std::function<message(const message&)> transformer
	);

	/**
	 * @brief Build the pipeline
	 * @return Constructed pipeline pointer or error
	 */
	common::Result<std::unique_ptr<message_pipeline>> build();
};

/**
 * @namespace pipeline_stages
 * @brief Common pipeline stage implementations
 */
namespace pipeline_stages {

/**
 * @brief Create a logging stage
 * @param stage_name Name for the logging stage
 * @return Message processor that logs messages
 */
message_processor create_logging_stage(const std::string& stage_name);

/**
 * @brief Create a validation stage
 * @param validator Validation function
 * @return Message processor that validates messages
 */
message_processor create_validation_stage(
	std::function<bool(const message&)> validator
);

/**
 * @brief Create an enrichment stage
 * @param enricher Function that adds data to message
 * @return Message processor that enriches messages
 */
message_processor create_enrichment_stage(
	std::function<void(message&)> enricher
);

/**
 * @brief Create a retry stage wrapper
 * @param processor Processor to wrap with retry logic
 * @param max_retries Maximum retry attempts
 * @param retry_delay Delay between retries
 * @return Message processor with retry logic
 */
message_processor create_retry_stage(
	message_processor processor,
	size_t max_retries = 3,
	std::chrono::milliseconds retry_delay = std::chrono::milliseconds{100}
);

}  // namespace pipeline_stages

}  // namespace kcenon::messaging::patterns
