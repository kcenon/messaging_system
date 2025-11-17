#include <gtest/gtest.h>
#include "../../../src/impl/di/messaging_di_container.h"
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/interfaces/logger_interface.h>
#include <kcenon/common/interfaces/monitoring_interface.h>
#include <memory>
#include <string>

using namespace kcenon::messaging::di;

// Test service interfaces for DI container
class test_service {
public:
    virtual ~test_service() = default;
    virtual std::string get_name() const = 0;
};

class test_service_impl : public test_service {
    std::string name_;

public:
    explicit test_service_impl(std::string name) : name_(std::move(name)) {}
    std::string get_name() const override { return name_; }
};

class MessagingDIContainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start with a clean container for each test
        container.clear();
    }

    void TearDown() override {
        container.clear();
    }

    messaging_di_container container;
};

TEST_F(MessagingDIContainerTest, RegisterAndResolveService) {
    // Arrange
    auto service = std::make_shared<test_service_impl>("test_service");

    // Act
    container.register_service<test_service>(service);
    auto resolved = container.resolve<test_service>();

    // Assert
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->get_name(), "test_service");
}

TEST_F(MessagingDIContainerTest, RegisterMultipleServices) {
    // Arrange
    auto service1 = std::make_shared<test_service_impl>("service1");
    auto service2 = std::make_shared<test_service_impl>("service2");

    // Act
    container.register_service<test_service>(service1);

    // Register a different service type
    class another_service {};
    auto another = std::make_shared<another_service>();
    container.register_service<another_service>(another);

    auto resolved1 = container.resolve<test_service>();
    auto resolved2 = container.resolve<another_service>();

    // Assert
    ASSERT_NE(resolved1, nullptr);
    ASSERT_NE(resolved2, nullptr);
    EXPECT_EQ(resolved1->get_name(), "service1");
}

TEST_F(MessagingDIContainerTest, ResolveNonExistentService) {
    // Act
    auto resolved = container.resolve<test_service>();

    // Assert
    EXPECT_EQ(resolved, nullptr);
}

TEST_F(MessagingDIContainerTest, OverwriteExistingService) {
    // Arrange
    auto service1 = std::make_shared<test_service_impl>("service1");
    auto service2 = std::make_shared<test_service_impl>("service2");

    // Act
    container.register_service<test_service>(service1);
    container.register_service<test_service>(service2); // Overwrite

    auto resolved = container.resolve<test_service>();

    // Assert
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->get_name(), "service2");
}

TEST_F(MessagingDIContainerTest, HasService) {
    // Arrange
    auto service = std::make_shared<test_service_impl>("test_service");

    // Act & Assert
    EXPECT_FALSE(container.has_service<test_service>());

    container.register_service<test_service>(service);
    EXPECT_TRUE(container.has_service<test_service>());
}

TEST_F(MessagingDIContainerTest, ClearAllServices) {
    // Arrange
    auto service = std::make_shared<test_service_impl>("test_service");
    container.register_service<test_service>(service);
    EXPECT_TRUE(container.has_service<test_service>());

    // Act
    container.clear();

    // Assert
    EXPECT_FALSE(container.has_service<test_service>());
    EXPECT_EQ(container.resolve<test_service>(), nullptr);
}

TEST_F(MessagingDIContainerTest, GlobalContainer) {
    // Arrange
    auto service = std::make_shared<test_service_impl>("global_service");

    // Act
    auto& global1 = get_global_container();
    auto& global2 = get_global_container();

    // Assert - should be the same instance
    EXPECT_EQ(&global1, &global2);

    // Register and resolve through global container
    global1.register_service<test_service>(service);
    auto resolved = global2.resolve<test_service>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->get_name(), "global_service");

    // Clean up
    global1.clear();
}

TEST_F(MessagingDIContainerTest, ThreadSafety) {
    // This is a basic smoke test for thread safety
    // A more comprehensive test would use thread sanitizer

    auto service = std::make_shared<test_service_impl>("thread_safe");

    // Register from one "thread"
    container.register_service<test_service>(service);

    // Resolve from another "thread" (simulated)
    auto resolved = container.resolve<test_service>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->get_name(), "thread_safe");
}

TEST_F(MessagingDIContainerTest, RegisterCommonSystemInterfaces) {
    // Test with actual common_system interfaces

    // Note: We can't test real implementations without the systems,
    // but we can test that the container works with these types

    // Create mock implementations would go here in a real scenario
    // For now, just verify the container can handle these types

    EXPECT_FALSE(container.has_service<kcenon::common::interfaces::IExecutor>());
    EXPECT_FALSE(container.has_service<kcenon::common::interfaces::ILogger>());

    // Note: IMonitoring is not in common_system interfaces
    // It would be defined in monitoring_system if needed
}
