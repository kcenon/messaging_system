#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/backends/integration_backend.h>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;

/**
 * @brief Integration tests for Backend implementations
 */
class BackendIntegrationTest : public IntegrationBackendFixture {};

TEST_F(BackendIntegrationTest, StandaloneBackendLifecycle) {
    auto backend = std::make_shared<standalone_backend>(4);

    // Initialize
    ASSERT_TRUE(backend->initialize().is_ok());
    EXPECT_TRUE(backend->is_ready());

    // Get executor
    auto executor = backend->get_executor();
    ASSERT_NE(executor, nullptr);

    // Shutdown
    ASSERT_TRUE(backend->shutdown().is_ok());
}

TEST_F(BackendIntegrationTest, StandaloneBackendTaskExecution) {
    auto backend = std::make_shared<standalone_backend>(2);
    ASSERT_TRUE(backend->initialize().is_ok());

    auto executor = backend->get_executor();
    ASSERT_NE(executor, nullptr);

    // Backend is ready
    EXPECT_TRUE(backend->is_ready());

    ASSERT_TRUE(backend->shutdown().is_ok());
}

TEST_F(BackendIntegrationTest, StandaloneBackendMultipleInitShutdown) {
    // Test multiple init/shutdown cycles
    for (int i = 0; i < 3; ++i) {
        auto backend = std::make_shared<standalone_backend>(4);
        ASSERT_TRUE(backend->initialize().is_ok());

        auto executor = backend->get_executor();
        ASSERT_NE(executor, nullptr);
        EXPECT_TRUE(backend->is_ready());

        ASSERT_TRUE(backend->shutdown().is_ok());
    }
}

TEST_F(BackendIntegrationTest, MessageBusWithStandaloneBackend) {
    auto backend = std::make_shared<standalone_backend>(4);
    ASSERT_TRUE(backend->initialize().is_ok());

    message_bus_config config;
    config.worker_threads = 2;

    auto bus = std::make_shared<message_bus>(backend, config);
    ASSERT_TRUE(bus->start().is_ok());

    // Test pub/sub
    MessageCounter counter;
    auto sub_result = bus->subscribe("test.topic", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    ASSERT_TRUE(bus->publish(create_test_message("test.topic")).is_ok());

    ASSERT_TRUE(wait_for_condition([&]() { return counter.count() >= 1; }));
    EXPECT_EQ(counter.count(), 1);

    ASSERT_TRUE(bus->stop().is_ok());
    ASSERT_TRUE(backend->shutdown().is_ok());
}

TEST_F(BackendIntegrationTest, ConcurrentMessageBusesWithSameBackend) {
    auto backend = std::make_shared<standalone_backend>(8);
    ASSERT_TRUE(backend->initialize().is_ok());

    // Create two message buses sharing the same backend
    auto bus1 = std::make_shared<message_bus>(backend);
    auto bus2 = std::make_shared<message_bus>(backend);

    ASSERT_TRUE(bus1->start().is_ok());
    ASSERT_TRUE(bus2->start().is_ok());

    // Test both buses independently
    MessageCounter counter1, counter2;

    auto sub1 = bus1->subscribe("bus1.topic", create_counting_callback(counter1));
    auto sub2 = bus2->subscribe("bus2.topic", create_counting_callback(counter2));

    ASSERT_TRUE(sub1.is_ok() && sub2.is_ok());

    ASSERT_TRUE(bus1->publish(create_test_message("bus1.topic")).is_ok());
    ASSERT_TRUE(bus2->publish(create_test_message("bus2.topic")).is_ok());

    ASSERT_TRUE(wait_for_condition([&]() {
        return counter1.count() >= 1 && counter2.count() >= 1;
    }));

    EXPECT_EQ(counter1.count(), 1);
    EXPECT_EQ(counter2.count(), 1);

    ASSERT_TRUE(bus1->stop().is_ok());
    ASSERT_TRUE(bus2->stop().is_ok());
    ASSERT_TRUE(backend->shutdown().is_ok());
}

TEST_F(BackendIntegrationTest, BackendResourceCleanup) {
    // Create and destroy backend multiple times
    for (int i = 0; i < 5; ++i) {
        auto backend = std::make_shared<standalone_backend>(2);
        ASSERT_TRUE(backend->initialize().is_ok());

        auto bus = std::make_shared<message_bus>(backend);
        ASSERT_TRUE(bus->start().is_ok());

        MessageCounter counter;
        auto sub_result = bus->subscribe("test", create_counting_callback(counter));
        ASSERT_TRUE(sub_result.is_ok());

        ASSERT_TRUE(bus->publish(create_test_message("test")).is_ok());
        ASSERT_TRUE(wait_for_condition([&]() { return counter.count() >= 1; }));

        ASSERT_TRUE(bus->stop().is_ok());
        ASSERT_TRUE(backend->shutdown().is_ok());
    }
}
