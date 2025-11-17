#pragma once

#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/backends/integration_backend.h>
#include <memory>

namespace kcenon::messaging::testing {

/**
 * @class MessagingFixture
 * @brief Base test fixture for messaging integration tests
 */
class MessagingFixture : public ::testing::Test {
protected:
    std::shared_ptr<backend_interface> backend_;
    std::shared_ptr<message_bus> bus_;

    void SetUp() override {
        // Create standalone backend with 4 worker threads
        backend_ = std::make_shared<standalone_backend>(4);
        ASSERT_TRUE(backend_->initialize().is_ok());

        // Create message bus
        message_bus_config config;
        config.queue_capacity = 1000;
        config.worker_threads = 2;
        config.enable_priority_queue = true;

        bus_ = std::make_shared<message_bus>(backend_, config);
        ASSERT_TRUE(bus_->start().is_ok());
    }

    void TearDown() override {
        if (bus_) {
            bus_->stop();
        }
        if (backend_) {
            backend_->shutdown();
        }
    }
};

/**
 * @class IntegrationBackendFixture
 * @brief Test fixture for integration backend tests
 */
class IntegrationBackendFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup will be done by individual tests
    }

    void TearDown() override {
        // Cleanup will be done by individual tests
    }
};

} // namespace kcenon::messaging::testing
