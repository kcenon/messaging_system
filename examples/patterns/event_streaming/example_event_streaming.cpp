/**
 * @file example_event_streaming.cpp
 * @brief Example demonstrating Event Streaming pattern
 *
 * This example shows how to use event sourcing with replay capabilities,
 * event filtering, and batch processing.
 * Updated to use messaging_container_builder and serialization features.
 */

#include <kcenon/messaging/patterns/event_streaming.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/integration/messaging_container_builder.h>
#include <kcenon/messaging/serialization/message_serializer.h>
#include <format>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace kcenon;
using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;
using namespace kcenon::messaging::integration;
using namespace kcenon::messaging::serialization;

int main() {
    std::cout << "=== Event Streaming Pattern Example ===" << std::endl;

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

    // 2. Create event stream
    std::cout << "\n2. Creating event stream..." << std::endl;
    event_stream_config stream_config;
    stream_config.max_buffer_size = 100;
    stream_config.enable_replay = true;

    event_stream stream(bus, "events.orders", stream_config);
    std::cout << "Event stream created for topic: events.orders" << std::endl;

    // 3. Publish some events using messaging_container_builder
    std::cout << "\n3. Publishing events to stream..." << std::endl;
    message_serializer serializer;

    for (int i = 1; i <= 10; ++i) {
        // Use container builder for type-safe event construction
        auto container = messaging_container_builder()
            .source("order-service", "stream-publisher")
            .target("event-store", "*")
            .message_type("order_event")
            .add_value("order_id", std::format("order-{}", i))
            .add_value("sequence", i)
            .add_value("amount", 100.0 * i)
            .add_value("timestamp", std::chrono::system_clock::now())
            .optimize_for_speed()
            .build();

        if (!container.is_ok()) {
            std::cerr << "  Failed to build container for event " << i << std::endl;
            continue;
        }

        auto msg_result = message_builder()
            .topic("events.orders")
            .type(message_type::event)
            .source("order-service")
            .priority((i % 3 == 0) ? message_priority::high : message_priority::normal)
            .payload(container.value())
            .build();

        if (!msg_result.is_ok()) {
            std::cerr << "  Failed to build message for event " << i << std::endl;
            continue;
        }

        auto result = stream.publish_event(std::move(msg_result.value()));
        if (result.is_ok()) {
            std::cout << std::format("  Published event: order-{}\n", i);
        }
    }

    // Demonstrate serialization
    std::cout << "\n  [Serialization Demo]" << std::endl;
    auto demo_container = messaging_container_builder()
        .source("demo", "test")
        .message_type("sample")
        .add_value("key", "value")
        .add_value("count", 42)
        .build();

    if (demo_container.is_ok()) {
        auto json = serializer.to_json(demo_container.value());
        if (json.is_ok()) {
            std::cout << "  JSON output: " << json.value() << std::endl;
        }
    }

    // Allow events to be buffered
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 4. Get event snapshot
    std::cout << "\n4. Event stream snapshot:" << std::endl;
    std::cout << "  Total events in buffer: " << stream.event_count() << std::endl;

    auto all_events = stream.get_events();
    std::cout << "  Retrieved " << all_events.size() << " events" << std::endl;

    // 5. Subscribe with replay
    std::cout << "\n5. Subscribing with replay..." << std::endl;
    std::atomic<int> replayed_count{0};

    auto replay_sub = stream.subscribe(
        [&replayed_count](const message& event) {
            replayed_count++;
            std::cout << "  [Replay Subscriber] Received: " << event.metadata().id << std::endl;
            return common::ok();
        },
        true  // Enable replay
    );

    if (!replay_sub.is_ok()) {
        std::cerr << "Failed to subscribe with replay" << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "  Replayed " << replayed_count.load() << " events" << std::endl;

    // 6. Subscribe with filter (high priority only)
    std::cout << "\n6. Subscribing with filter (high priority only)..." << std::endl;
    std::atomic<int> high_priority_count{0};

    auto filter_sub = stream.subscribe(
        [&high_priority_count](const message& event) {
            high_priority_count++;
            std::cout << "  [Filtered Subscriber] High priority event: "
                      << event.metadata().id << std::endl;
            return common::ok();
        },
        [](const message& event) {
            return event.metadata().priority == message_priority::high;
        },
        true  // Replay with filter
    );

    if (!filter_sub.is_ok()) {
        std::cerr << "Failed to subscribe with filter" << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "  Received " << high_priority_count.load()
              << " high-priority events" << std::endl;

    // 7. Publish more events (only new subscribers should receive these)
    std::cout << "\n7. Publishing additional events..." << std::endl;
    for (int i = 11; i <= 13; ++i) {
        message event("events.orders", message_type::event);
        event.metadata().source = "order-service";
        event.metadata().id = "order-" + std::to_string(i);
        event.metadata().priority = message_priority::high;

        stream.publish_event(std::move(event));
        std::cout << "  Published event: order-" << i << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 8. Batch processing example
    std::cout << "\n8. Setting up batch processor..." << std::endl;
    std::atomic<int> batches_processed{0};
    std::atomic<int> total_in_batches{0};

    event_batch_processor processor(
        bus,
        "events.batch.*",
        [&batches_processed, &total_in_batches](const std::vector<message>& batch) {
            batches_processed++;
            total_in_batches += static_cast<int>(batch.size());
            std::cout << "  [Batch Processor] Processing batch of "
                      << batch.size() << " events" << std::endl;
            return common::ok();
        },
        5,  // batch size
        std::chrono::milliseconds{500}  // batch timeout
    );

    auto batch_start = processor.start();
    if (!batch_start.is_ok()) {
        std::cerr << "Failed to start batch processor" << std::endl;
        return 1;
    }

    // Publish events for batch processing
    std::cout << "  Publishing events for batch processing..." << std::endl;
    for (int i = 1; i <= 12; ++i) {
        message event("events.batch.test", message_type::event);
        event.metadata().id = "batch-event-" + std::to_string(i);
        bus->publish(std::move(event));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "  Batches processed: " << batches_processed.load() << std::endl;
    std::cout << "  Total events in batches: " << total_in_batches.load() << std::endl;

    processor.stop();

    // 9. Display final statistics
    std::cout << "\n9. Final statistics:" << std::endl;
    std::cout << "  Event stream buffer size: " << stream.event_count() << std::endl;

    auto stats = bus->get_statistics();
    std::cout << "  Total messages published: " << stats.messages_published << std::endl;
    std::cout << "  Total messages processed: " << stats.messages_processed << std::endl;

    // 10. Cleanup
    std::cout << "\n10. Cleaning up..." << std::endl;
    stream.clear_buffer();
    bus->stop();

    std::cout << "\n=== Example completed successfully ===" << std::endl;

    return 0;
}
