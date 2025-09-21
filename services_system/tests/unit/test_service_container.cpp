#include <gtest/gtest.h>
#include <kcenon/messaging/integrations/service_container.h>
#include <memory>
#include <string>

using namespace kcenon::messaging::integrations;

// Mock service classes for testing
class MockService {
public:
    MockService(int value = 42) : value_(value) {}
    int get_value() const { return value_; }
    void set_value(int value) { value_ = value; }

private:
    int value_;
};

class MockDependentService {
public:
    MockDependentService(std::shared_ptr<MockService> dependency)
        : dependency_(std::move(dependency)) {}

    int get_dependency_value() const {
        return dependency_ ? dependency_->get_value() : -1;
    }

private:
    std::shared_ptr<MockService> dependency_;
};

class ServiceContainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_unique<service_container>();
    }

    std::unique_ptr<service_container> container_;
};

TEST_F(ServiceContainerTest, RegisterAndResolveTransientService) {
    // Register a transient service (non-singleton)
    container_->register_service<MockService>(
        "mock_service",
        []() { return std::make_shared<MockService>(100); },
        false  // transient
    );

    EXPECT_TRUE(container_->is_registered("mock_service"));
    EXPECT_TRUE(container_->is_registered<MockService>());

    // Resolve service multiple times
    auto service1 = container_->resolve<MockService>("mock_service");
    auto service2 = container_->resolve<MockService>("mock_service");

    ASSERT_NE(service1, nullptr);
    ASSERT_NE(service2, nullptr);

    // Should be different instances for transient services
    EXPECT_NE(service1.get(), service2.get());
    EXPECT_EQ(service1->get_value(), 100);
    EXPECT_EQ(service2->get_value(), 100);
}

TEST_F(ServiceContainerTest, RegisterAndResolveSingletonService) {
    // Register a singleton service
    container_->register_service<MockService>(
        "singleton_service",
        []() { return std::make_shared<MockService>(200); },
        true  // singleton
    );

    // Resolve service multiple times
    auto service1 = container_->resolve<MockService>("singleton_service");
    auto service2 = container_->resolve<MockService>("singleton_service");

    ASSERT_NE(service1, nullptr);
    ASSERT_NE(service2, nullptr);

    // Should be the same instance for singleton services
    EXPECT_EQ(service1.get(), service2.get());
    EXPECT_EQ(service1->get_value(), 200);

    // Modify value through one reference
    service1->set_value(300);
    EXPECT_EQ(service2->get_value(), 300);
}

TEST_F(ServiceContainerTest, RegisterSingletonInstance) {
    // Create an instance and register it directly
    auto instance = std::make_shared<MockService>(500);
    container_->register_singleton<MockService>("instance_service", instance);

    EXPECT_TRUE(container_->is_registered("instance_service"));

    auto resolved = container_->resolve<MockService>("instance_service");
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved.get(), instance.get());
    EXPECT_EQ(resolved->get_value(), 500);
}

TEST_F(ServiceContainerTest, ResolveByType) {
    // Register service with type resolution
    container_->register_service<MockService>(
        "type_service",
        []() { return std::make_shared<MockService>(600); }
    );

    // Resolve by type (without name)
    auto service = container_->resolve<MockService>();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->get_value(), 600);
}

TEST_F(ServiceContainerTest, DependencyInjection) {
    // Register dependency first
    container_->register_service<MockService>(
        "dependency",
        []() { return std::make_shared<MockService>(700); }
    );

    // Register dependent service that uses the dependency
    container_->register_service<MockDependentService>(
        "dependent_service",
        [this]() {
            auto dependency = container_->resolve<MockService>("dependency");
            return std::make_shared<MockDependentService>(dependency);
        }
    );

    auto dependent = container_->resolve<MockDependentService>("dependent_service");
    ASSERT_NE(dependent, nullptr);
    EXPECT_EQ(dependent->get_dependency_value(), 700);
}

TEST_F(ServiceContainerTest, NonExistentService) {
    // Try to resolve a service that doesn't exist
    auto service = container_->resolve<MockService>("nonexistent");
    EXPECT_EQ(service, nullptr);

    auto service_by_type = container_->resolve<MockService>();
    EXPECT_EQ(service_by_type, nullptr);

    EXPECT_FALSE(container_->is_registered("nonexistent"));
    EXPECT_FALSE(container_->is_registered<MockService>());
}

TEST_F(ServiceContainerTest, ServiceRegistrationList) {
    // Register multiple services
    container_->register_service<MockService>(
        "service1",
        []() { return std::make_shared<MockService>(1); }
    );

    container_->register_service<MockService>(
        "service2",
        []() { return std::make_shared<MockService>(2); }
    );

    container_->register_service<MockDependentService>(
        "dependent",
        []() { return std::make_shared<MockDependentService>(nullptr); }
    );

    auto registered_services = container_->get_registered_services();
    EXPECT_EQ(registered_services.size(), 3);

    // Check that all services are in the list
    std::sort(registered_services.begin(), registered_services.end());
    EXPECT_TRUE(std::find(registered_services.begin(), registered_services.end(), "service1") != registered_services.end());
    EXPECT_TRUE(std::find(registered_services.begin(), registered_services.end(), "service2") != registered_services.end());
    EXPECT_TRUE(std::find(registered_services.begin(), registered_services.end(), "dependent") != registered_services.end());
}

TEST_F(ServiceContainerTest, ClearContainer) {
    // Register some services
    container_->register_service<MockService>(
        "service1",
        []() { return std::make_shared<MockService>(1); }
    );

    container_->register_service<MockService>(
        "service2",
        []() { return std::make_shared<MockService>(2); }
    );

    EXPECT_EQ(container_->get_registered_services().size(), 2);
    EXPECT_TRUE(container_->is_registered("service1"));

    // Clear all registrations
    container_->clear();

    EXPECT_EQ(container_->get_registered_services().size(), 0);
    EXPECT_FALSE(container_->is_registered("service1"));
    EXPECT_FALSE(container_->is_registered("service2"));

    auto service = container_->resolve<MockService>("service1");
    EXPECT_EQ(service, nullptr);
}

TEST_F(ServiceContainerTest, ThreadSafety) {
    constexpr int num_threads = 8;
    constexpr int registrations_per_thread = 10;

    // Register services concurrently
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, registrations_per_thread]() {
            for (int i = 0; i < registrations_per_thread; ++i) {
                std::string service_name = "service_" + std::to_string(t) + "_" + std::to_string(i);
                int expected_value = t * 100 + i;

                container_->register_service<MockService>(
                    service_name,
                    [expected_value]() { return std::make_shared<MockService>(expected_value); }
                );
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all services were registered
    auto registered_services = container_->get_registered_services();
    EXPECT_EQ(registered_services.size(), num_threads * registrations_per_thread);

    // Resolve services concurrently
    std::atomic<int> successful_resolutions{0};
    threads.clear();

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, registrations_per_thread, &successful_resolutions]() {
            for (int i = 0; i < registrations_per_thread; ++i) {
                std::string service_name = "service_" + std::to_string(t) + "_" + std::to_string(i);
                int expected_value = t * 100 + i;

                auto service = container_->resolve<MockService>(service_name);
                if (service && service->get_value() == expected_value) {
                    successful_resolutions++;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successful_resolutions.load(), num_threads * registrations_per_thread);
}