#include <gtest/gtest.h>
#include <kcenon/messaging/di/messaging_bootstrapper.h>
#include <kcenon/common/di/unified_bootstrapper.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace kcenon::messaging::di;
using namespace kcenon::common::di;

class MessagingBootstrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state before each test
        if (messaging_bootstrapper::is_integrated()) {
            messaging_bootstrapper::remove();
        }
        if (unified_bootstrapper::is_initialized()) {
            unified_bootstrapper::shutdown();
        }
    }

    void TearDown() override {
        // Clean up after each test
        if (messaging_bootstrapper::is_integrated()) {
            messaging_bootstrapper::remove();
        }
        if (unified_bootstrapper::is_initialized()) {
            unified_bootstrapper::shutdown();
        }
    }
};

// =============================================================================
// Integration Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, IntegrateRequiresInitializedBootstrapper) {
    // Act - Try to integrate without initializing bootstrapper
    auto result = messaging_bootstrapper::integrate();

    // Assert
    EXPECT_TRUE(result.is_err());
    EXPECT_FALSE(messaging_bootstrapper::is_integrated());
}

TEST_F(MessagingBootstrapperTest, BasicIntegration) {
    // Arrange - Initialize bootstrapper first
    auto init_result = unified_bootstrapper::initialize({
        .enable_logging = false,
        .enable_monitoring = false,
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 2,
            .queue_capacity = 100,
            .enable_event_bridge = false
        },
        .auto_start = true
    });

    // Assert
    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(messaging_bootstrapper::is_integrated());

    auto bus = messaging_bootstrapper::get_message_bus();
    ASSERT_NE(bus, nullptr);
    EXPECT_TRUE(bus->is_running());
}

TEST_F(MessagingBootstrapperTest, IntegrationWithoutAutoStart) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 2,
            .queue_capacity = 100,
            .enable_event_bridge = false
        },
        .auto_start = false
    });

    // Assert
    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(messaging_bootstrapper::is_integrated());

    auto bus = messaging_bootstrapper::get_message_bus();
    ASSERT_NE(bus, nullptr);
    EXPECT_FALSE(bus->is_running());
}

TEST_F(MessagingBootstrapperTest, DoubleIntegrationFails) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    auto first_result = messaging_bootstrapper::integrate({
        .config = {.enable_event_bridge = false},
        .auto_start = false
    });
    ASSERT_TRUE(first_result.is_ok());

    // Act - Try to integrate again
    auto second_result = messaging_bootstrapper::integrate({
        .config = {.enable_event_bridge = false},
        .auto_start = false
    });

    // Assert
    EXPECT_TRUE(second_result.is_err());
}

TEST_F(MessagingBootstrapperTest, RemoveIntegration) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    auto integrate_result = messaging_bootstrapper::integrate({
        .config = {.enable_event_bridge = false},
        .auto_start = true
    });
    ASSERT_TRUE(integrate_result.is_ok());
    EXPECT_TRUE(messaging_bootstrapper::is_integrated());

    // Act
    auto remove_result = messaging_bootstrapper::remove();

    // Assert
    EXPECT_TRUE(remove_result.is_ok());
    EXPECT_FALSE(messaging_bootstrapper::is_integrated());
    EXPECT_EQ(messaging_bootstrapper::get_message_bus(), nullptr);
}

TEST_F(MessagingBootstrapperTest, RemoveWithoutIntegrationFails) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::remove();

    // Assert
    EXPECT_TRUE(result.is_err());
}

// =============================================================================
// Builder Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, BuilderBasicConfiguration) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::builder()
        .with_worker_threads(4)
        .with_queue_capacity(500)
        .with_event_bridge(false)
        .with_auto_start(true)
        .integrate();

    // Assert
    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(messaging_bootstrapper::is_integrated());

    auto opts = messaging_bootstrapper::get_options();
    EXPECT_EQ(opts.config.worker_threads, 4);
    EXPECT_EQ(opts.config.queue_capacity, 500);
    EXPECT_FALSE(opts.config.enable_event_bridge);
    EXPECT_TRUE(opts.auto_start);
}

TEST_F(MessagingBootstrapperTest, BuilderWithConfig) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    messaging_config config;
    config.worker_threads = 8;
    config.queue_capacity = 1000;
    config.enable_event_bridge = false;

    // Act
    auto result = messaging_bootstrapper::builder()
        .with_config(config)
        .with_auto_start(false)
        .integrate();

    // Assert
    EXPECT_TRUE(result.is_ok());

    auto opts = messaging_bootstrapper::get_options();
    EXPECT_EQ(opts.config.worker_threads, 8);
    EXPECT_EQ(opts.config.queue_capacity, 1000);
}

TEST_F(MessagingBootstrapperTest, BuilderCustomShutdownHookName) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::builder()
        .with_worker_threads(2)
        .with_event_bridge(false)
        .with_shutdown_hook_name("custom_messaging_hook")
        .with_auto_start(false)
        .integrate();

    // Assert
    EXPECT_TRUE(result.is_ok());

    auto opts = messaging_bootstrapper::get_options();
    EXPECT_EQ(opts.shutdown_hook_name, "custom_messaging_hook");
}

// =============================================================================
// Shutdown Hook Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, ShutdownStopsMessageBus) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    auto integrate_result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 2,
            .enable_event_bridge = false
        },
        .auto_start = true
    });
    ASSERT_TRUE(integrate_result.is_ok());

    auto bus = messaging_bootstrapper::get_message_bus();
    ASSERT_NE(bus, nullptr);
    EXPECT_TRUE(bus->is_running());

    // Act - Shutdown the bootstrapper (which triggers hooks)
    auto shutdown_result = unified_bootstrapper::shutdown();

    // Assert
    EXPECT_TRUE(shutdown_result.is_ok());
    EXPECT_FALSE(messaging_bootstrapper::is_integrated());
}

// =============================================================================
// Accessor Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, GetMessageBusBeforeIntegration) {
    // Act
    auto bus = messaging_bootstrapper::get_message_bus();

    // Assert
    EXPECT_EQ(bus, nullptr);
}

TEST_F(MessagingBootstrapperTest, GetEventBridgeBeforeIntegration) {
    // Act
    auto bridge = messaging_bootstrapper::get_event_bridge();

    // Assert
    EXPECT_EQ(bridge, nullptr);
}

TEST_F(MessagingBootstrapperTest, GetOptionsBeforeIntegration) {
    // Act
    auto opts = messaging_bootstrapper::get_options();

    // Assert - Should return default values
    EXPECT_EQ(opts.config.worker_threads, 4); // default value
    EXPECT_TRUE(opts.config.enable_event_bridge); // default value
}

// =============================================================================
// Event Bridge Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, EventBridgeIntegration) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 2,
            .enable_event_bridge = true
        },
        .auto_start = true
    });

    // Assert
    EXPECT_TRUE(result.is_ok());

    auto bridge = messaging_bootstrapper::get_event_bridge();
    EXPECT_NE(bridge, nullptr);
}

TEST_F(MessagingBootstrapperTest, EventBridgeDisabled) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 2,
            .enable_event_bridge = false
        },
        .auto_start = false
    });

    // Assert
    EXPECT_TRUE(result.is_ok());

    auto bridge = messaging_bootstrapper::get_event_bridge();
    EXPECT_EQ(bridge, nullptr);
}

// =============================================================================
// Service Resolution Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, ResolveFromContainer) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    auto integrate_result = messaging_bootstrapper::integrate({
        .config = {.enable_event_bridge = false},
        .auto_start = false
    });
    ASSERT_TRUE(integrate_result.is_ok());

    // Act - Resolve directly from container
    auto& container = unified_bootstrapper::services();
    auto bus_result = container.resolve<IMessageBus>();

    // Assert
    EXPECT_TRUE(bus_result.is_ok());
    EXPECT_NE(bus_result.value(), nullptr);
}

// =============================================================================
// Re-integration Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, ReintegrationAfterRemove) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // First integration
    auto first_result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 2,
            .enable_event_bridge = false
        },
        .auto_start = false
    });
    ASSERT_TRUE(first_result.is_ok());

    // Remove
    auto remove_result = messaging_bootstrapper::remove();
    ASSERT_TRUE(remove_result.is_ok());

    // Act - Re-integrate with different config
    auto second_result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 4,
            .enable_event_bridge = false
        },
        .auto_start = false
    });

    // Assert
    EXPECT_TRUE(second_result.is_ok());
    EXPECT_TRUE(messaging_bootstrapper::is_integrated());

    auto opts = messaging_bootstrapper::get_options();
    EXPECT_EQ(opts.config.worker_threads, 4);
}

// =============================================================================
// Worker Count Tests
// =============================================================================

TEST_F(MessagingBootstrapperTest, ConfiguredWorkerCount) {
    // Arrange
    auto init_result = unified_bootstrapper::initialize({
        .register_signal_handlers = false
    });
    ASSERT_TRUE(init_result.is_ok());

    // Act
    auto result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 6,
            .enable_event_bridge = false
        },
        .auto_start = true
    });

    // Assert
    EXPECT_TRUE(result.is_ok());

    auto bus = messaging_bootstrapper::get_message_bus();
    ASSERT_NE(bus, nullptr);
    EXPECT_EQ(bus->worker_count(), 6);
}
