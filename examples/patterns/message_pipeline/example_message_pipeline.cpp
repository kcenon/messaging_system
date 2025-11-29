/**
 * @file example_message_pipeline.cpp
 * @brief Example demonstrating Message Pipeline pattern
 *
 * This example shows how to use the pipes-and-filters pattern for
 * sequential message processing with validation, transformation, and enrichment.
 */

#include <kcenon/messaging/patterns/message_pipeline.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace kcenon;
using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;

int main() {
    std::cout << "=== Message Pipeline Pattern Example ===" << std::endl;

    // 1. Create backend and message bus
    std::cout << "\n1. Setting up message bus..." << std::endl;
    auto backend = std::make_shared<standalone_backend>(2);
    message_bus_config config;
    config.worker_threads = 2;
    config.queue_capacity = 100;
    auto bus = std::make_shared<message_bus>(backend, config);

    auto start_result = bus->start();
    if (!start_result.is_ok()) {
        std::cerr << "Failed to start message bus" << std::endl;
        return 1;
    }
    std::cout << "Message bus started successfully" << std::endl;

    // 2. Build pipeline using builder pattern
    std::cout << "\n2. Building message pipeline..." << std::endl;

    pipeline_builder builder(bus);

    auto pipeline_result = builder
        .from("input.raw_data")
        .to("output.processed_data")

        // Stage 1: Validation
        .add_stage("validate", [](const message& msg) -> common::Result<message> {
            std::cout << "  [Stage: Validate] Processing message: "
                      << msg.metadata().id << std::endl;

            // Validate message has required fields
            if (msg.metadata().topic.empty()) {
                return common::make_error<message>(-1, "Invalid message: empty topic");
            }

            std::cout << "  [Stage: Validate] Message validated successfully" << std::endl;
            return common::ok(message(msg));
        })

        // Stage 2: Filter (keep only high priority messages)
        .add_filter("high_priority_filter", [](const message& msg) {
            bool keep = msg.metadata().priority >= message_priority::normal;
            if (keep) {
                std::cout << "  [Stage: Filter] Message passed filter" << std::endl;
            } else {
                std::cout << "  [Stage: Filter] Message filtered out" << std::endl;
            }
            return keep;
        })

        // Stage 3: Transform (boost priority)
        .add_transformer("boost_priority", [](const message& msg) {
            std::cout << "  [Stage: Transform] Boosting message priority" << std::endl;
            message transformed(msg);
            transformed.metadata().priority = message_priority::high;
            return transformed;
        })

        // Stage 4: Enrichment (add metadata)
        .add_stage("enrich", [](const message& msg) -> common::Result<message> {
            std::cout << "  [Stage: Enrich] Adding metadata" << std::endl;
            message enriched(msg);
            enriched.metadata().source = "pipeline-processor";
            enriched.metadata().timestamp = std::chrono::system_clock::now();
            return common::ok(std::move(enriched));
        })

        // Stage 5: Logging (optional stage - won't fail pipeline)
        .add_stage("log", [](const message& msg) -> common::Result<message> {
            std::cout << "  [Stage: Log] Message: " << msg.metadata().id
                      << " | Source: " << msg.metadata().source
                      << " | Priority: " << static_cast<int>(msg.metadata().priority)
                      << std::endl;
            return common::ok(message(msg));
        }, true)  // Mark as optional

        .build();

    if (!pipeline_result.is_ok()) {
        std::cerr << "Failed to build pipeline: "
                  << pipeline_result.error().message << std::endl;
        return 1;
    }

    auto pipeline = std::move(pipeline_result.value());
    std::cout << "Pipeline built with " << pipeline->stage_count() << " stages:" << std::endl;

    auto stage_names = pipeline->get_stage_names();
    for (size_t i = 0; i < stage_names.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << stage_names[i] << std::endl;
    }

    // 3. Manual message processing
    std::cout << "\n3. Processing messages manually..." << std::endl;

    // Process message 1 (valid, normal priority)
    std::cout << "\nProcessing Message 1:" << std::endl;
    message msg1("input.raw_data");
    msg1.metadata().id = "msg-001";
    msg1.metadata().priority = message_priority::normal;

    auto result1 = pipeline->process(std::move(msg1));
    if (result1.is_ok()) {
        auto processed = result1.unwrap();
        std::cout << "Message 1 processed successfully" << std::endl;
        std::cout << "  Final priority: "
                  << static_cast<int>(processed.metadata().priority) << std::endl;
    }

    // Process message 2 (valid, high priority)
    std::cout << "\nProcessing Message 2:" << std::endl;
    message msg2("input.raw_data");
    msg2.metadata().id = "msg-002";
    msg2.metadata().priority = message_priority::high;

    auto result2 = pipeline->process(std::move(msg2));
    if (result2.is_ok()) {
        std::cout << "Message 2 processed successfully" << std::endl;
    }

    // Process message 3 (low priority - should be filtered)
    std::cout << "\nProcessing Message 3 (low priority):" << std::endl;
    message msg3("input.raw_data");
    msg3.metadata().id = "msg-003";
    msg3.metadata().priority = message_priority::low;

    auto result3 = pipeline->process(std::move(msg3));
    if (result3.is_err()) {
        std::cout << "Message 3 was filtered out (expected)" << std::endl;
    }

    // 4. Automatic processing
    std::cout << "\n4. Starting automatic pipeline processing..." << std::endl;

    // Subscribe to output topic to see results
    std::atomic<int> output_count{0};
    auto output_sub = bus->subscribe("output.processed_data",
        [&output_count](const message& msg) {
            output_count++;
            std::cout << "  [Output Subscriber] Received processed message: "
                      << msg.metadata().id << std::endl;
            return common::ok();
        }
    );

    auto pipeline_start = pipeline->start();
    if (!pipeline_start.is_ok()) {
        std::cerr << "Failed to start pipeline" << std::endl;
        return 1;
    }
    std::cout << "Pipeline started - processing messages automatically" << std::endl;

    // Publish messages to input topic
    for (int i = 1; i <= 5; ++i) {
        message msg("input.raw_data");
        msg.metadata().id = "auto-msg-" + std::to_string(i);
        msg.metadata().priority = (i % 2 == 0) ? message_priority::high : message_priority::normal;

        bus->publish(std::move(msg));
        std::cout << "Published message to input topic: auto-msg-" << i << std::endl;
    }

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\nReceived " << output_count.load()
              << " messages on output topic" << std::endl;

    // 5. Display pipeline statistics
    std::cout << "\n5. Pipeline statistics:" << std::endl;
    auto stats = pipeline->get_statistics();
    std::cout << "  Messages processed: " << stats.messages_processed << std::endl;
    std::cout << "  Messages succeeded: " << stats.messages_succeeded << std::endl;
    std::cout << "  Messages failed: " << stats.messages_failed << std::endl;
    std::cout << "  Stage failures: " << stats.stage_failures << std::endl;

    // 6. Demonstrate common pipeline stages
    std::cout << "\n6. Demonstrating common pipeline stages..." << std::endl;

    message_pipeline custom_pipeline(bus, "input.test", "output.test");

    // Add validation stage
    auto validator = [](const message& msg) {
        return !msg.metadata().id.empty();
    };
    custom_pipeline.add_stage("validate",
        pipeline_stages::create_validation_stage(validator));

    // Add enrichment stage
    auto enricher = [](message& msg) {
        msg.metadata().source = "custom-pipeline";
    };
    custom_pipeline.add_stage("enrich",
        pipeline_stages::create_enrichment_stage(enricher));

    // Add retry stage for flaky operations
    auto flaky_op = [](const message& msg) -> common::Result<message> {
        // This operation might fail occasionally
        static int attempt = 0;
        if (++attempt % 3 == 0) {
            return common::ok(message(msg));
        }
        return common::make_error<message>(-1, "Temporary failure");
    };

    custom_pipeline.add_stage("process_with_retry",
        pipeline_stages::create_retry_stage(flaky_op, 3));

    std::cout << "Custom pipeline created with common stages" << std::endl;

    // 7. Cleanup
    std::cout << "\n7. Cleaning up..." << std::endl;
    pipeline->stop();
    bus->stop();

    std::cout << "\n=== Example completed successfully ===" << std::endl;

    return 0;
}
