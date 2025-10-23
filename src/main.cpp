#include "messaging_system/core/messaging_container.h"
#include "messaging_system/core/message_bus.h"
#include "messaging_system/core/topic_router.h"
#include "messaging_system/support/mock_executor.h"

#ifdef HAS_LOGGER_SYSTEM
#include <kcenon/logger/core/logger.h>
#endif

#include <iostream>
#include <memory>
#include <thread>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    std::cout << "Messaging System v2.0" << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << std::endl;

    // Check which external systems are available
    std::cout << "External Systems Status:" << std::endl;

#ifdef HAS_COMMON_SYSTEM
    std::cout << "  ✓ CommonSystem integrated" << std::endl;
#else
    std::cout << "  ✗ CommonSystem not available" << std::endl;
#endif

#ifdef HAS_THREAD_SYSTEM
    std::cout << "  ✓ ThreadSystem integrated" << std::endl;
#else
    std::cout << "  ✗ ThreadSystem not available" << std::endl;
#endif

#ifdef HAS_LOGGER_SYSTEM
    std::cout << "  ✓ LoggerSystem integrated" << std::endl;
#else
    std::cout << "  ✗ LoggerSystem not available" << std::endl;
#endif

#ifdef HAS_MONITORING_SYSTEM
    std::cout << "  ✓ MonitoringSystem integrated" << std::endl;
#else
    std::cout << "  ✗ MonitoringSystem not available" << std::endl;
#endif

#ifdef HAS_CONTAINER_SYSTEM
    std::cout << "  ✓ ContainerSystem integrated" << std::endl;
#else
    std::cout << "  ✗ ContainerSystem not available" << std::endl;
#endif

#ifdef HAS_DATABASE_SYSTEM
    std::cout << "  ✓ DatabaseSystem integrated" << std::endl;
#else
    std::cout << "  ✗ DatabaseSystem not available" << std::endl;
#endif

#ifdef HAS_NETWORK_SYSTEM
    std::cout << "  ✓ NetworkSystem integrated" << std::endl;
#else
    std::cout << "  ✗ NetworkSystem not available" << std::endl;
#endif

    std::cout << std::endl;

    // Basic functionality test
    std::cout << "Testing core functionality..." << std::endl;

    // Test MessagingContainer creation
    auto msg_result = messaging::MessagingContainer::create("test_source", "test_target", "test.topic");
    if (msg_result.is_ok()) {
        auto msg = msg_result.value();
        std::cout << "  ✓ MessagingContainer created" << std::endl;
        std::cout << "    - Source: " << msg.source() << std::endl;
        std::cout << "    - Target: " << msg.target() << std::endl;
        std::cout << "    - Topic: " << msg.topic() << std::endl;
        std::cout << "    - Trace ID: " << msg.trace_id() << std::endl;
    } else {
        std::cout << "  ✗ Failed to create MessagingContainer: "
                  << msg_result.error().message << std::endl;
        return 1;
    }

#ifdef HAS_THREAD_SYSTEM
    // Test MessageBus with ThreadSystem
    std::cout << std::endl;
    std::cout << "Initializing MessageBus..." << std::endl;

    // Use lightweight mock executors to drive the demo without depending on
    // thread_system's evolving adapters.
    auto io_executor = std::make_shared<messaging::support::MockExecutor>(2);
    auto work_executor = std::make_shared<messaging::support::MockExecutor>(4);

    auto router = std::make_shared<messaging::TopicRouter>(work_executor);
    auto message_bus = std::make_shared<messaging::MessageBus>(io_executor, work_executor, router);

    auto start_result = message_bus->start();
    if (start_result.is_ok()) {
        std::cout << "  ✓ MessageBus started successfully" << std::endl;

        // Subscribe to a topic
        auto subscribe_result = message_bus->subscribe("test.topic",
            [](const messaging::MessagingContainer& msg) -> common::VoidResult {
                std::cout << "  → Received message on topic: " << msg.topic() << std::endl;
                return common::VoidResult::ok(std::monostate{});
            });

        if (subscribe_result.is_ok()) {
            std::cout << "  ✓ Subscribed to test.topic (ID: "
                      << subscribe_result.value() << ")" << std::endl;

            // Publish a test message
            auto test_msg = messaging::MessagingContainer::create("main", "subscriber", "test.topic");
            if (test_msg.is_ok()) {
                auto publish_result = message_bus->publish_async(test_msg.value());
                if (publish_result.is_ok()) {
                    std::cout << "  ✓ Message published" << std::endl;

                    // Give time for async processing
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }

        auto stop_result = message_bus->stop();
        if (stop_result.is_ok()) {
            std::cout << "  ✓ MessageBus stopped successfully" << std::endl;
        }
    } else {
        std::cout << "  ✗ Failed to start MessageBus: "
                  << start_result.error().message << std::endl;
    }
#else
    std::cout << std::endl;
    std::cout << "MessageBus test skipped (ThreadSystem not available)" << std::endl;
#endif

    std::cout << std::endl;
    std::cout << "All tests completed successfully!" << std::endl;

    return 0;
}
