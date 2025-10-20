// Integration tests for end-to-end messaging flow

#include "messaging_system/core/message_bus.h"
#include "messaging_system/core/topic_router.h"
#include "messaging_system/core/messaging_container.h"
#include "messaging_system/integration/trace_context.h"

#ifdef HAS_THREAD_SYSTEM
#include <kcenon/thread/core/thread_pool.h>
#endif

#ifdef HAS_YAML_CPP
#include "messaging_system/integration/config_loader.h"
#endif

#include <iostream>
#include <cassert>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

using namespace messaging;

#ifdef HAS_THREAD_SYSTEM

void test_complete_pubsub_flow() {
    std::cout << "Integration Test: Complete pub/sub flow with trace context..." << std::endl;

    auto io_executor = std::make_shared<thread::thread_pool>(2);
    auto work_executor = std::make_shared<thread::thread_pool>(4);
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    message_bus->start();

    std::atomic<int> received_count{0};
    std::vector<std::string> received_trace_ids;
    std::mutex trace_ids_mutex;

    // Subscribe with trace context handling
    auto sub_result = message_bus->subscribe("order.placed",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            ScopedTrace trace(msg.trace_id());

            received_count++;

            std::lock_guard<std::mutex> lock(trace_ids_mutex);
            received_trace_ids.push_back(TraceContext::get_trace_id());

            return common::VoidResult::ok();
        });

    assert(sub_result.is_ok() && "Should subscribe successfully");

    // Publish messages with trace IDs
    std::vector<std::string> sent_trace_ids;
    for (int i = 0; i < 3; ++i) {
        auto msg = MessagingContainer::create("order_service", "fulfillment", "order.placed");
        assert(msg.is_ok() && "Should create message");

        auto message = msg.value();
        sent_trace_ids.push_back(message.trace_id());

        message_bus->publish_async(message);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    assert(received_count == 3 && "Should receive all 3 messages");
    assert(received_trace_ids.size() == 3 && "Should capture 3 trace IDs");

    // Verify trace IDs were propagated
    for (size_t i = 0; i < sent_trace_ids.size(); ++i) {
        assert(received_trace_ids[i] == sent_trace_ids[i] && "Trace IDs should match");
    }

    message_bus->stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_complex_routing_scenario() {
    std::cout << "Integration Test: Complex routing with multiple patterns..." << std::endl;

    auto io_executor = std::make_shared<thread::thread_pool>(2);
    auto work_executor = std::make_shared<thread::thread_pool>(4);
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    message_bus->start();

    std::atomic<int> event_all_count{0};
    std::atomic<int> event_user_count{0};
    std::atomic<int> event_user_login_count{0};

    // Multi-level wildcard: receives ALL events
    message_bus->subscribe("event.#",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            event_all_count++;
            return common::VoidResult::ok();
        });

    // Single-level wildcard: receives event.user.*, event.order.*, etc.
    message_bus->subscribe("event.user.*",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            event_user_count++;
            return common::VoidResult::ok();
        });

    // Exact match: only event.user.login
    message_bus->subscribe("event.user.login",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            event_user_login_count++;
            return common::VoidResult::ok();
        });

    // Test messages
    auto msg1 = MessagingContainer::create("src", "tgt", "event.user.login").value();
    message_bus->publish_sync(msg1); // All 3 should match

    auto msg2 = MessagingContainer::create("src", "tgt", "event.user.logout").value();
    message_bus->publish_sync(msg2); // event_all and event_user should match

    auto msg3 = MessagingContainer::create("src", "tgt", "event.order.placed").value();
    message_bus->publish_sync(msg3); // Only event_all should match

    auto msg4 = MessagingContainer::create("src", "tgt", "event.system.startup.complete").value();
    message_bus->publish_sync(msg4); // Only event_all should match

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    assert(event_all_count == 4 && "Multi-level wildcard should match all 4");
    assert(event_user_count == 2 && "Single-level wildcard should match 2");
    assert(event_user_login_count == 1 && "Exact match should match 1");

    message_bus->stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_multi_subscriber_coordination() {
    std::cout << "Integration Test: Multi-subscriber coordination..." << std::endl;

    auto io_executor = std::make_shared<thread::thread_pool>(4);
    auto work_executor = std::make_shared<thread::thread_pool>(8);
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    message_bus->start();

    // Simulate microservices architecture
    std::atomic<int> inventory_service_count{0};
    std::atomic<int> email_service_count{0};
    std::atomic<int> analytics_service_count{0};

    // Inventory service listens to order events
    message_bus->subscribe("order.*",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            ScopedTrace trace(msg.trace_id());
            inventory_service_count++;
            // Simulate inventory update work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return common::VoidResult::ok();
        });

    // Email service listens to specific events
    message_bus->subscribe("order.placed",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            ScopedTrace trace(msg.trace_id());
            email_service_count++;
            // Simulate sending email
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            return common::VoidResult::ok();
        });

    // Analytics service listens to everything
    message_bus->subscribe("#",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            ScopedTrace trace(msg.trace_id());
            analytics_service_count++;
            return common::VoidResult::ok();
        });

    // Simulate order flow
    auto order_placed = MessagingContainer::create("api", "services", "order.placed").value();
    message_bus->publish_async(order_placed);

    auto order_confirmed = MessagingContainer::create("payment", "services", "order.confirmed").value();
    message_bus->publish_async(order_confirmed);

    auto order_shipped = MessagingContainer::create("warehouse", "services", "order.shipped").value();
    message_bus->publish_async(order_shipped);

    auto user_login = MessagingContainer::create("auth", "services", "user.login").value();
    message_bus->publish_async(user_login);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    assert(inventory_service_count == 3 && "Inventory should process 3 order events");
    assert(email_service_count == 1 && "Email should only send for order.placed");
    assert(analytics_service_count == 4 && "Analytics should capture all events");

    message_bus->stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_high_throughput_scenario() {
    std::cout << "Integration Test: High throughput scenario..." << std::endl;

    auto io_executor = std::make_shared<thread::thread_pool>(4);
    auto work_executor = std::make_shared<thread::thread_pool>(8);
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    message_bus->start();

    std::atomic<int> received_count{0};

    message_bus->subscribe("high.throughput.#",
        [&](const MessagingContainer& msg) -> common::Result<void> {
            received_count++;
            return common::VoidResult::ok();
        });

    const int num_messages = 1000;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Publish messages from multiple threads
    std::vector<std::thread> publishers;
    const int num_publishers = 4;
    const int messages_per_publisher = num_messages / num_publishers;

    for (int p = 0; p < num_publishers; ++p) {
        publishers.emplace_back([&, p]() {
            for (int i = 0; i < messages_per_publisher; ++i) {
                auto msg = MessagingContainer::create(
                    "publisher_" + std::to_string(p),
                    "subscriber",
                    "high.throughput.test." + std::to_string(i)
                ).value();
                message_bus->publish_async(msg);
            }
        });
    }

    for (auto& pub : publishers) {
        pub.join();
    }

    // Wait for all messages to be processed
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    assert(received_count == num_messages && "Should receive all messages");

    std::cout << "  Processed " << num_messages << " messages in " << duration.count() << "ms" << std::endl;
    std::cout << "  Throughput: " << (num_messages * 1000.0 / duration.count()) << " msg/s" << std::endl;
    std::cout << "  ✓ Passed" << std::endl;

    message_bus->stop();
}

void test_subscribe_unsubscribe_lifecycle() {
    std::cout << "Integration Test: Subscribe/unsubscribe lifecycle..." << std::endl;

    auto io_executor = std::make_shared<thread::thread_pool>(2);
    auto work_executor = std::make_shared<thread::thread_pool>(4);
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    message_bus->start();

    std::atomic<int> sub1_count{0};
    std::atomic<int> sub2_count{0};
    std::atomic<int> sub3_count{0};

    // Subscribe 3 subscribers
    auto sub1 = message_bus->subscribe("lifecycle.test",
        [&](const MessagingContainer&) -> common::Result<void> {
            sub1_count++;
            return common::VoidResult::ok();
        }).value();

    auto sub2 = message_bus->subscribe("lifecycle.test",
        [&](const MessagingContainer&) -> common::Result<void> {
            sub2_count++;
            return common::VoidResult::ok();
        }).value();

    auto sub3 = message_bus->subscribe("lifecycle.test",
        [&](const MessagingContainer&) -> common::Result<void> {
            sub3_count++;
            return common::VoidResult::ok();
        }).value();

    // Round 1: All receive
    auto msg1 = MessagingContainer::create("src", "tgt", "lifecycle.test").value();
    message_bus->publish_sync(msg1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(sub1_count == 1 && sub2_count == 1 && sub3_count == 1);

    // Unsubscribe sub2
    message_bus->unsubscribe(sub2);

    // Round 2: Only sub1 and sub3 receive
    auto msg2 = MessagingContainer::create("src", "tgt", "lifecycle.test").value();
    message_bus->publish_sync(msg2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(sub1_count == 2 && sub2_count == 1 && sub3_count == 2);

    // Unsubscribe sub1 and sub3
    message_bus->unsubscribe(sub1);
    message_bus->unsubscribe(sub3);

    // Round 3: No one receives
    auto msg3 = MessagingContainer::create("src", "tgt", "lifecycle.test").value();
    message_bus->publish_sync(msg3);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(sub1_count == 2 && sub2_count == 1 && sub3_count == 2);

    message_bus->stop();
    std::cout << "  ✓ Passed" << std::endl;
}

#ifdef HAS_YAML_CPP
void test_config_driven_initialization() {
    std::cout << "Integration Test: Config-driven initialization..." << std::endl;

    // Create test config
    std::string config_content = R"(
messaging_system:
  version: "2.0.0"
  network:
    port: 9000
  thread_pools:
    io:
      workers: 2
    work:
      workers: 4
)";

    std::ofstream config_file("test_integration_config.yaml");
    config_file << config_content;
    config_file.close();

    auto config_result = MessagingSystemConfig::load_from_file("test_integration_config.yaml");
    assert(config_result.is_ok() && "Should load config");

    auto config = config_result.value();
    auto validate_result = config.validate();
    assert(validate_result.is_ok() && "Config should be valid");

    // Use config to create system
    auto io_executor = std::make_shared<thread::thread_pool>(config.thread_pools.io_workers);
    auto work_executor = std::make_shared<thread::thread_pool>(config.thread_pools.work_workers);
    auto router = std::make_shared<TopicRouter>(work_executor);
    auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);

    auto start_result = message_bus->start();
    assert(start_result.is_ok() && "Should start with config-driven initialization");

    // Verify it works
    std::atomic<int> count{0};
    message_bus->subscribe("config.test",
        [&](const MessagingContainer&) -> common::Result<void> {
            count++;
            return common::VoidResult::ok();
        });

    auto msg = MessagingContainer::create("src", "tgt", "config.test").value();
    message_bus->publish_sync(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(count == 1 && "Should receive message");

    message_bus->stop();
    std::filesystem::remove("test_integration_config.yaml");

    std::cout << "  ✓ Passed" << std::endl;
}
#endif

int main() {
    std::cout << "=== End-to-End Integration Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        test_complete_pubsub_flow();
        test_complex_routing_scenario();
        test_multi_subscriber_coordination();
        test_high_throughput_scenario();
        test_subscribe_unsubscribe_lifecycle();

#ifdef HAS_YAML_CPP
        test_config_driven_initialization();
#endif

        std::cout << std::endl;
        std::cout << "All integration tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Integration test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

#else

int main() {
    std::cerr << "Integration tests require HAS_THREAD_SYSTEM" << std::endl;
    return 1;
}

#endif
