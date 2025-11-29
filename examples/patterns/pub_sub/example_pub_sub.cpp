/**
 * @file example_pub_sub.cpp
 * @brief Example demonstrating Pub/Sub messaging pattern
 *
 * This example shows how to use the publisher and subscriber classes
 * for simple publish-subscribe messaging with topic patterns and filters.
 * Updated to use messaging_container_builder for type-safe message construction.
 */

#include <kcenon/messaging/patterns/pub_sub.h>
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
    std::cout << "=== Pub/Sub Pattern Example ===" << std::endl;

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

    // 2. Create subscribers
    std::cout << "\n2. Creating subscribers..." << std::endl;
    subscriber sub(bus);

    std::atomic<int> user_event_count{0};
    std::atomic<int> order_event_count{0};
    std::atomic<int> high_priority_count{0};

    // Subscribe to all user events
    auto user_sub = sub.subscribe("events.user.*", [&user_event_count](const message& msg) {
        user_event_count++;
        std::cout << "  [User Subscriber] Received: " << msg.metadata().topic << std::endl;
        return common::ok();
    });

    if (!user_sub.is_ok()) {
        std::cerr << "Failed to subscribe to user events" << std::endl;
        return 1;
    }
    std::cout << "Subscribed to user events (events.user.*)" << std::endl;

    // Subscribe to all order events
    auto order_sub = sub.subscribe("events.order.*", [&order_event_count](const message& msg) {
        order_event_count++;
        std::cout << "  [Order Subscriber] Received: " << msg.metadata().topic << std::endl;
        return common::ok();
    });

    if (!order_sub.is_ok()) {
        std::cerr << "Failed to subscribe to order events" << std::endl;
        return 1;
    }
    std::cout << "Subscribed to order events (events.order.*)" << std::endl;

    // Subscribe to all events with high priority filter
    auto high_priority_sub = sub.subscribe(
        "events.#",  // Multi-level wildcard
        [&high_priority_count](const message& msg) {
            high_priority_count++;
            std::cout << "  [High Priority Subscriber] Received: " << msg.metadata().topic
                      << " (priority: high)" << std::endl;
            return common::ok();
        },
        [](const message& msg) {
            // Filter: only high priority messages
            return msg.metadata().priority == message_priority::high;
        }
    );

    if (!high_priority_sub.is_ok()) {
        std::cerr << "Failed to subscribe with filter" << std::endl;
        return 1;
    }
    std::cout << "Subscribed to high-priority events (events.#)" << std::endl;

    // 3. Create publishers
    std::cout << "\n3. Creating publishers..." << std::endl;
    publisher user_pub(bus, "events.user");
    publisher order_pub(bus, "events.order");
    std::cout << "Publishers created" << std::endl;

    // 4. Publish some events
    std::cout << "\n4. Publishing events..." << std::endl;

    // User events using messaging_container_builder
    for (int i = 1; i <= 3; ++i) {
        // Use container builder for type-safe message construction
        auto container = messaging_container_builder()
            .source("user-service", std::format("session_{}", i))
            .target("subscribers", "*")
            .message_type("user_created")
            .add_value("user_id", std::format("USR-{:05d}", i))
            .add_value("event_index", i)
            .add_value("timestamp", std::chrono::system_clock::now())
            .optimize_for_speed()
            .build();

        if (container.is_ok()) {
            message msg("events.user.created", message_type::event);
            msg.metadata().source = "user-service";
            msg.metadata().priority = (i == 2) ? message_priority::high : message_priority::normal;
            msg.payload() = *container.value();

            auto result = user_pub.publish("events.user.created", std::move(msg));
            if (result.is_ok()) {
                std::cout << std::format("  Published: events.user.created (event {})\n", i);
            }
        }
    }

    // Order events using messaging_container_builder
    for (int i = 1; i <= 2; ++i) {
        auto container = messaging_container_builder()
            .source("order-service", "main")
            .target("order-processor", "*")
            .message_type("order_placed")
            .add_value("order_id", std::format("ORD-{:08d}", i * 1000))
            .add_value("amount", 99.99 * i)
            .add_value("quantity", i * 10)
            .optimize_for_network()
            .build();

        if (container.is_ok()) {
            message msg("events.order.placed", message_type::event);
            msg.metadata().source = "order-service";
            msg.metadata().priority = (i == 1) ? message_priority::high : message_priority::normal;
            msg.payload() = *container.value();

            auto result = order_pub.publish("events.order.placed", std::move(msg));
            if (result.is_ok()) {
                std::cout << std::format("  Published: events.order.placed (event {})\n", i);
            }
        }
    }

    // Give time for messages to be processed
    std::cout << "\n5. Waiting for message processing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 6. Display statistics
    std::cout << "\n6. Statistics:" << std::endl;
    std::cout << "  User events received: " << user_event_count.load() << std::endl;
    std::cout << "  Order events received: " << order_event_count.load() << std::endl;
    std::cout << "  High-priority events received: " << high_priority_count.load() << std::endl;

    auto stats = bus->get_statistics();
    std::cout << "  Total messages published: " << stats.messages_published << std::endl;
    std::cout << "  Total messages processed: " << stats.messages_processed << std::endl;

    // 7. Cleanup
    std::cout << "\n7. Cleaning up..." << std::endl;
    sub.unsubscribe_all();
    bus->stop();

    std::cout << "\n=== Example completed successfully ===" << std::endl;

    return 0;
}
