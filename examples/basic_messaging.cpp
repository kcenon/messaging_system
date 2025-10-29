// Basic Messaging System Example
// Demonstrates core messaging functionality with TraceContext

#include "messaging_system/core/messaging_container.h"
#include "messaging_system/core/message_bus.h"
#include "messaging_system/core/topic_router.h"
#include "messaging_system/integration/trace_context.h"

#ifdef HAS_YAML_CPP
#include "messaging_system/integration/config_loader.h"
#endif

#include "messaging_system/support/mock_executor.h"

#include <iostream>
#include <thread>
#include <atomic>

using namespace messaging;

int main() {
    std::cout << "=== Basic Messaging System Example ===" << std::endl;
    std::cout << std::endl;

#ifdef HAS_YAML_CPP
    // Load configuration if available
    std::cout << "Loading configuration..." << std::endl;
    auto config_result = MessagingSystemConfig::load_from_file("deploy/production/config.yaml");
    if (config_result.is_ok()) {
        auto config = config_result.value();
        std::cout << "  ✓ Configuration loaded" << std::endl;
        std::cout << "    - Network port: " << config.network.port << std::endl;
        std::cout << "    - IO workers: " << config.thread_pools.io_workers << std::endl;
        std::cout << "    - Work workers: " << config.thread_pools.work_workers << std::endl;

        auto validate = config.validate();
        if (validate.is_ok()) {
            std::cout << "  ✓ Configuration validated" << std::endl;
        }
    } else {
        std::cout << "  ⚠ Config file not found, using defaults" << std::endl;
    }
    std::cout << std::endl;
#endif

    std::cout << "Initializing messaging system..." << std::endl;

    // Create executors
    auto io_executor = std::make_shared<messaging::support::MockExecutor>(2);
    auto work_executor = std::make_shared<messaging::support::MockExecutor>(4);

    // Create router and message bus
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    auto start_result = message_bus->start();
    if (!start_result.is_ok()) {
        std::cerr << "Failed to start message bus" << std::endl;
        return 1;
    }
    std::cout << "  ✓ Message bus started" << std::endl;
    std::cout << std::endl;

    // Example 1: Simple pub/sub
    std::cout << "Example 1: Simple Pub/Sub" << std::endl;
    std::atomic<int> received_count{0};

    auto sub1 = message_bus->subscribe("user.created",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            ScopedTrace trace(msg.trace_id());
            std::cout << "  → [" << TraceContext::get_trace_id() << "] "
                      << "Subscriber 1 received: " << msg.topic() << std::endl;
            received_count++;
            return common::ok();
        });

    if (sub1.is_ok()) {
        std::cout << "  ✓ Subscribed to user.created" << std::endl;
    }

    // Publish messages
    for (int i = 0; i < 3; ++i) {
        auto msg = MessagingContainer::create("example", "subscribers", "user.created");
        if (msg.is_ok()) {
            message_bus->publish_async(msg.value());
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "  ✓ Received " << received_count.load() << " messages" << std::endl;
    std::cout << std::endl;

    // Example 2: Wildcard subscriptions
    std::cout << "Example 2: Wildcard Subscriptions" << std::endl;
    std::atomic<int> wildcard_count{0};

    auto sub2 = message_bus->subscribe("order.*",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            std::cout << "  → Wildcard subscriber received: " << msg.topic() << std::endl;
            wildcard_count++;
            return common::ok();
        });

    if (sub2.is_ok()) {
        std::cout << "  ✓ Subscribed to order.* (wildcard)" << std::endl;
    }

    // Publish to different topics
    std::vector<std::string> topics = {"order.placed", "order.shipped", "order.delivered"};
    for (const auto& topic : topics) {
        auto msg = MessagingContainer::create("example", "subscribers", topic);
        if (msg.is_ok()) {
            message_bus->publish_async(msg.value());
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "  ✓ Wildcard received " << wildcard_count.load() << " messages" << std::endl;
    std::cout << std::endl;

    // Example 3: Multi-level wildcard
    std::cout << "Example 3: Multi-level Wildcard" << std::endl;
    std::atomic<int> multilevel_count{0};

    auto sub3 = message_bus->subscribe("event.#",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            std::cout << "  → Multi-level subscriber received: " << msg.topic() << std::endl;
            multilevel_count++;
            return common::ok();
        });

    if (sub3.is_ok()) {
        std::cout << "  ✓ Subscribed to event.# (multi-level)" << std::endl;
    }

    std::vector<std::string> events = {
        "event.user",
        "event.user.login",
        "event.system.startup",
        "event.system.shutdown.complete"
    };

    for (const auto& topic : events) {
        auto msg = MessagingContainer::create("example", "subscribers", topic);
        if (msg.is_ok()) {
            message_bus->publish_async(msg.value());
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "  ✓ Multi-level received " << multilevel_count.load() << " messages" << std::endl;
    std::cout << std::endl;

    // Cleanup
    message_bus->stop();
    std::cout << "  ✓ Message bus stopped" << std::endl;
    std::cout << std::endl;
    std::cout << "Example completed successfully!" << std::endl;

    return 0;
}
