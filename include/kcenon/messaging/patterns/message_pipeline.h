#pragma once

#include "../core/message_bus.h"
#include <kcenon/common/patterns/result.h>
#include <kcenon/common/concepts/concepts.h>

#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

namespace kcenon::messaging::patterns {

// =============================================================================
// C++20 Concepts for Message Pipeline
// =============================================================================

/**
 * @concept MessageProcessorCallable
 * @brief A callable type that processes messages.
 *
 * Types satisfying this concept can be invoked with a const message&
 * and return a Result<message>. This provides compile-time type safety
 * for message processing stages.
 *
 * @tparam F The callable type to validate
 *
 * Example usage:
 * @code
 * template<MessageProcessorCallable Processor>
 * void add_stage(const std::string& name, Processor&& proc) {
 *     // Processor is guaranteed to be callable with (const message&)
 *     // and return common::Result<message>
 * }
 * @endcode
 */
template<typename F>
concept MessageProcessorCallable = std::invocable<F, const message&> &&
	std::same_as<std::invoke_result_t<F, const message&>,
				 common::Result<message>>;

/**
 * @concept MessageFilterCallable
 * @brief A callable type that filters messages.
 *
 * Types satisfying this concept can be invoked with a const message&
 * and return a boolean indicating whether the message should be kept.
 *
 * @tparam F The callable type to validate
 */
template<typename F>
concept MessageFilterCallable = std::invocable<F, const message&> &&
	std::convertible_to<std::invoke_result_t<F, const message&>, bool>;

/**
 * @concept MessageTransformerCallable
 * @brief A callable type that transforms messages.
 *
 * Types satisfying this concept can be invoked with a const message&
 * and return a transformed message.
 *
 * @tparam F The callable type to validate
 */
template<typename F>
concept MessageTransformerCallable = std::invocable<F, const message&> &&
	std::same_as<std::invoke_result_t<F, const message&>, message>;

/**
 * @concept MessageEnricherCallable
 * @brief A callable type that enriches messages in-place.
 *
 * Types satisfying this concept can be invoked with a message&
 * to add data to the message in-place.
 *
 * @tparam F The callable type to validate
 */
template<typename F>
concept MessageEnricherCallable = std::invocable<F, message&>;

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
	 * @brief Add a processing stage using C++20 concept constraint
	 *
	 * This overload accepts any callable that satisfies the MessageProcessorCallable
	 * concept, providing better compile-time error messages.
	 *
	 * @tparam Processor A type satisfying MessageProcessorCallable concept
	 * @param name Stage name
	 * @param processor Any callable matching the message processor signature
	 * @param optional If true, stage failures won't stop pipeline
	 * @return Reference to this builder for chaining
	 */
	template<MessageProcessorCallable Processor>
	pipeline_builder& add_stage(
		std::string name,
		Processor&& processor,
		bool optional = false
	) {
		return add_stage(std::move(name),
						 message_processor(std::forward<Processor>(processor)),
						 optional);
	}

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
	 * @brief Add a filter stage using C++20 concept constraint
	 *
	 * This overload accepts any callable that satisfies the MessageFilterCallable
	 * concept, providing better compile-time error messages.
	 *
	 * @tparam Filter A type satisfying MessageFilterCallable concept
	 * @param name Stage name
	 * @param filter Any callable returning bool for message filtering
	 * @return Reference to this builder for chaining
	 */
	template<MessageFilterCallable Filter>
	pipeline_builder& add_filter(std::string name, Filter&& filter) {
		return add_filter(std::move(name),
						  std::function<bool(const message&)>(std::forward<Filter>(filter)));
	}

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
	 * @brief Add a transformation stage using C++20 concept constraint
	 *
	 * This overload accepts any callable that satisfies the MessageTransformerCallable
	 * concept, providing better compile-time error messages.
	 *
	 * @tparam Transformer A type satisfying MessageTransformerCallable concept
	 * @param name Stage name
	 * @param transformer Any callable returning a transformed message
	 * @return Reference to this builder for chaining
	 */
	template<MessageTransformerCallable Transformer>
	pipeline_builder& add_transformer(std::string name, Transformer&& transformer) {
		return add_transformer(std::move(name),
							   std::function<message(const message&)>(std::forward<Transformer>(transformer)));
	}

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
 * @brief Create a validation stage using C++20 concept constraint
 *
 * This overload accepts any callable that satisfies the MessageFilterCallable
 * concept, providing better compile-time error messages.
 *
 * @tparam Validator A type satisfying MessageFilterCallable concept
 * @param validator Any callable returning bool for message validation
 * @return Message processor that validates messages
 */
template<MessageFilterCallable Validator>
message_processor create_validation_stage(Validator&& validator) {
	return create_validation_stage(
		std::function<bool(const message&)>(std::forward<Validator>(validator)));
}

/**
 * @brief Create an enrichment stage
 * @param enricher Function that adds data to message
 * @return Message processor that enriches messages
 */
message_processor create_enrichment_stage(
	std::function<void(message&)> enricher
);

/**
 * @brief Create an enrichment stage using C++20 concept constraint
 *
 * This overload accepts any callable that satisfies the MessageEnricherCallable
 * concept, providing better compile-time error messages.
 *
 * @tparam Enricher A type satisfying MessageEnricherCallable concept
 * @param enricher Any callable that modifies messages in-place
 * @return Message processor that enriches messages
 */
template<MessageEnricherCallable Enricher>
message_processor create_enrichment_stage(Enricher&& enricher) {
	return create_enrichment_stage(
		std::function<void(message&)>(std::forward<Enricher>(enricher)));
}

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
