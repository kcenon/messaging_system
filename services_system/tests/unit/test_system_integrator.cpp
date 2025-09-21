#include <gtest/gtest.h>
#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/config.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace kcenon::messaging::integrations;
using namespace kcenon::messaging::config;
using namespace kcenon::messaging::core;

class SystemIntegratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_builder builder;
        config_ = builder
            .set_environment("testing")
            .set_worker_threads(2)
            .set_queue_size(1000)
            .enable_compression(false)
            .build();

        integrator_ = std::make_unique<system_integrator>(config_);
    }

    void TearDown() override {
        if (integrator_ && integrator_->is_running()) {
            integrator_->shutdown();
        }
    }

    messaging_config config_;
    std::unique_ptr<system_integrator> integrator_;
};

TEST_F(SystemIntegratorTest, InitializationAndShutdown) {
    EXPECT_FALSE(integrator_->is_running());

    EXPECT_TRUE(integrator_->initialize());
    EXPECT_TRUE(integrator_->is_running());

    integrator_->shutdown();
    EXPECT_FALSE(integrator_->is_running());
}

TEST_F(SystemIntegratorTest, ConfigurationAccess) {
    const auto& retrieved_config = integrator_->get_config();

    EXPECT_EQ(retrieved_config.environment, "testing");
    EXPECT_EQ(retrieved_config.message_bus.worker_threads, 2);
    EXPECT_EQ(retrieved_config.message_bus.max_queue_size, 1000);
    EXPECT_FALSE(retrieved_config.container.enable_compression);
}

TEST_F(SystemIntegratorTest, MessageBusAccess) {
    ASSERT_TRUE(integrator_->initialize());

    auto* message_bus = integrator_->get_message_bus();
    ASSERT_NE(message_bus, nullptr);
    EXPECT_TRUE(message_bus->is_running());
}

TEST_F(SystemIntegratorTest, ServiceContainerAccess) {
    ASSERT_TRUE(integrator_->initialize());

    auto& container = integrator_->get_container();

    // Check that core services are registered
    auto registered_services = container.get_registered_services();
    EXPECT_FALSE(registered_services.empty());

    // Core services should be available
    EXPECT_TRUE(container.is_registered("message_bus"));
    EXPECT_TRUE(container.is_registered("config"));
}

TEST_F(SystemIntegratorTest, PublishSubscribeIntegration) {
    ASSERT_TRUE(integrator_->initialize());

    std::atomic<bool> message_received{false};
    std::string received_content;

    // Subscribe to a topic
    integrator_->subscribe("integration_topic", [&](const message& msg) {
        message_received = true;
        if (!msg.payload.data.empty()) {
            auto it = msg.payload.data.find("content");
            if (it != msg.payload.data.end() && std::holds_alternative<std::string>(it->second)) {
                received_content = std::get<std::string>(it->second);
            }
        }
    });

    // Publish a message
    message_payload payload;
    payload.topic = "integration_topic";
    payload.data["content"] = std::string("Integration test message");

    EXPECT_TRUE(integrator_->publish("integration_topic", payload, "test_sender"));

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(message_received.load());
    EXPECT_EQ(received_content, "Integration test message");
}

TEST_F(SystemIntegratorTest, SystemHealthCheck) {
    ASSERT_TRUE(integrator_->initialize());

    auto health = integrator_->check_system_health();

    EXPECT_TRUE(health.message_bus_healthy);
    EXPECT_GE(health.active_services, 0);
    EXPECT_GE(health.total_messages_processed, 0);

    // Publish some messages to increase the count
    message_payload payload;
    payload.topic = "health_test";
    payload.data["test"] = std::string("health");

    for (int i = 0; i < 5; ++i) {
        integrator_->publish("health_test", payload);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto updated_health = integrator_->check_system_health();
    EXPECT_TRUE(updated_health.message_bus_healthy);
}

TEST_F(SystemIntegratorTest, DefaultSystemCreation) {
    auto default_system = system_integrator::create_default();
    ASSERT_NE(default_system, nullptr);

    const auto& default_config = default_system->get_config();
    EXPECT_EQ(default_config.environment, "development");
    EXPECT_EQ(default_config.message_bus.worker_threads, 4);
    EXPECT_EQ(default_config.message_bus.max_queue_size, 10000);
    EXPECT_TRUE(default_config.container.enable_compression);

    EXPECT_TRUE(default_system->initialize());
    EXPECT_TRUE(default_system->is_running());
    default_system->shutdown();
}

TEST_F(SystemIntegratorTest, EnvironmentSpecificCreation) {
    auto production_system = system_integrator::create_for_environment("production");
    ASSERT_NE(production_system, nullptr);

    const auto& prod_config = production_system->get_config();
    EXPECT_EQ(prod_config.environment, "production");
    EXPECT_EQ(prod_config.logging.level, "warn");
    EXPECT_TRUE(prod_config.monitoring.enable);

    auto staging_system = system_integrator::create_for_environment("staging");
    ASSERT_NE(staging_system, nullptr);

    const auto& staging_config = staging_system->get_config();
    EXPECT_EQ(staging_config.environment, "staging");
}

TEST_F(SystemIntegratorTest, ServiceResolution) {
    ASSERT_TRUE(integrator_->initialize());

    // Try to resolve core services
    auto message_bus_service = integrator_->get_service<message_bus>("message_bus");
    EXPECT_NE(message_bus_service, nullptr);

    auto config_service = integrator_->get_service<messaging_config>("config");
    EXPECT_NE(config_service, nullptr);

    // Try to resolve by type
    auto config_by_type = integrator_->get_service<messaging_config>();
    EXPECT_NE(config_by_type, nullptr);
    EXPECT_EQ(config_by_type.get(), config_service.get());
}

TEST_F(SystemIntegratorTest, MultipleInstances) {
    // Create multiple system integrator instances
    auto system1 = std::make_unique<system_integrator>(config_);
    auto system2 = std::make_unique<system_integrator>(config_);

    EXPECT_TRUE(system1->initialize());
    EXPECT_TRUE(system2->initialize());

    EXPECT_TRUE(system1->is_running());
    EXPECT_TRUE(system2->is_running());

    // They should operate independently
    std::atomic<int> system1_messages{0};
    std::atomic<int> system2_messages{0};

    system1->subscribe("test_topic", [&](const message& msg) {
        system1_messages++;
    });

    system2->subscribe("test_topic", [&](const message& msg) {
        system2_messages++;
    });

    message_payload payload;
    payload.topic = "test_topic";
    payload.data["test"] = std::string("independence");

    // Publish to each system separately
    system1->publish("test_topic", payload);
    system2->publish("test_topic", payload);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Each system should only receive its own message
    EXPECT_EQ(system1_messages.load(), 1);
    EXPECT_EQ(system2_messages.load(), 1);

    system1->shutdown();
    system2->shutdown();
}

TEST_F(SystemIntegratorTest, ConcurrentOperations) {
    ASSERT_TRUE(integrator_->initialize());

    constexpr int num_threads = 4;
    constexpr int operations_per_thread = 50;
    std::atomic<int> total_received{0};

    // Subscribe to test topic
    integrator_->subscribe("concurrent_topic", [&](const message& msg) {
        total_received++;
    });

    // Launch multiple threads publishing messages
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, operations_per_thread]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                message_payload payload;
                payload.topic = "concurrent_topic";
                payload.data["thread_id"] = int64_t(t);
                payload.data["operation_id"] = int64_t(i);

                integrator_->publish("concurrent_topic", payload, "thread_" + std::to_string(t));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Wait for all messages to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(total_received.load(), num_threads * operations_per_thread);
}