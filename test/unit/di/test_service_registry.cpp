#include <gtest/gtest.h>
#include "../../../src/impl/di/service_registry.h"
#include <memory>
#include <string>

using namespace kcenon::messaging::di;

// Test service for registry
class test_counter_service {
    static int instance_count_;

public:
    test_counter_service() {
        ++instance_count_;
    }

    ~test_counter_service() {
        --instance_count_;
    }

    static int get_instance_count() {
        return instance_count_;
    }

    static void reset_count() {
        instance_count_ = 0;
    }

    std::string get_data() const {
        return "counter_service";
    }
};

int test_counter_service::instance_count_ = 0;

class test_data_service {
    std::string data_;

public:
    explicit test_data_service(std::string data) : data_(std::move(data)) {}

    std::string get_data() const {
        return data_;
    }
};

class ServiceRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        registry.clear();
        test_counter_service::reset_count();
    }

    void TearDown() override {
        registry.clear();
        test_counter_service::reset_count();
    }

    service_registry registry;
};

TEST_F(ServiceRegistryTest, RegisterAndResolveSingleton) {
    // Arrange
    auto service = std::make_shared<test_data_service>("singleton_data");

    // Act
    registry.register_singleton<test_data_service>(service);
    auto resolved1 = registry.resolve<test_data_service>();
    auto resolved2 = registry.resolve<test_data_service>();

    // Assert
    ASSERT_NE(resolved1, nullptr);
    ASSERT_NE(resolved2, nullptr);
    EXPECT_EQ(resolved1.get(), resolved2.get()); // Same instance
    EXPECT_EQ(resolved1->get_data(), "singleton_data");
}

TEST_F(ServiceRegistryTest, RegisterAndResolveTransient) {
    // Arrange
    int call_count = 0;
    auto factory = [&call_count]() {
        ++call_count;
        return std::make_shared<test_data_service>("transient_" + std::to_string(call_count));
    };

    // Act
    registry.register_transient<test_data_service>(factory);
    auto resolved1 = registry.resolve<test_data_service>();
    auto resolved2 = registry.resolve<test_data_service>();

    // Assert
    ASSERT_NE(resolved1, nullptr);
    ASSERT_NE(resolved2, nullptr);
    EXPECT_NE(resolved1.get(), resolved2.get()); // Different instances
    EXPECT_EQ(resolved1->get_data(), "transient_1");
    EXPECT_EQ(resolved2->get_data(), "transient_2");
    EXPECT_EQ(call_count, 2);
}

TEST_F(ServiceRegistryTest, SingletonInstancePersistence) {
    // Arrange
    auto service = std::make_shared<test_counter_service>();
    int initial_count = test_counter_service::get_instance_count();

    // Act
    registry.register_singleton<test_counter_service>(service);
    auto resolved1 = registry.resolve<test_counter_service>();
    auto resolved2 = registry.resolve<test_counter_service>();
    auto resolved3 = registry.resolve<test_counter_service>();

    // Assert
    EXPECT_EQ(test_counter_service::get_instance_count(), initial_count);
    EXPECT_EQ(resolved1.get(), service.get());
    EXPECT_EQ(resolved2.get(), service.get());
    EXPECT_EQ(resolved3.get(), service.get());
}

TEST_F(ServiceRegistryTest, TransientCreatesNewInstances) {
    // Arrange
    auto factory = []() {
        return std::make_shared<test_counter_service>();
    };

    int initial_count = test_counter_service::get_instance_count();

    // Act
    registry.register_transient<test_counter_service>(factory);
    auto resolved1 = registry.resolve<test_counter_service>();
    auto resolved2 = registry.resolve<test_counter_service>();

    // Assert
    EXPECT_EQ(test_counter_service::get_instance_count(), initial_count + 2);
    EXPECT_NE(resolved1.get(), resolved2.get());
}

TEST_F(ServiceRegistryTest, ResolveNonExistentService) {
    // Act
    auto resolved = registry.resolve<test_data_service>();

    // Assert
    EXPECT_EQ(resolved, nullptr);
}

TEST_F(ServiceRegistryTest, HasService) {
    // Arrange
    auto service = std::make_shared<test_data_service>("test");

    // Act & Assert
    EXPECT_FALSE(registry.has_service<test_data_service>());

    registry.register_singleton<test_data_service>(service);
    EXPECT_TRUE(registry.has_service<test_data_service>());
}

TEST_F(ServiceRegistryTest, ClearAllServices) {
    // Arrange
    auto singleton = std::make_shared<test_data_service>("singleton");
    auto factory = []() { return std::make_shared<test_counter_service>(); };

    registry.register_singleton<test_data_service>(singleton);
    registry.register_transient<test_counter_service>(factory);

    EXPECT_TRUE(registry.has_service<test_data_service>());
    EXPECT_TRUE(registry.has_service<test_counter_service>());

    // Act
    registry.clear();

    // Assert
    EXPECT_FALSE(registry.has_service<test_data_service>());
    EXPECT_FALSE(registry.has_service<test_counter_service>());
}

TEST_F(ServiceRegistryTest, ServiceCount) {
    // Arrange
    auto service1 = std::make_shared<test_data_service>("service1");
    auto service2 = std::make_shared<test_counter_service>();

    // Act & Assert
    EXPECT_EQ(registry.count(), 0u);

    registry.register_singleton<test_data_service>(service1);
    EXPECT_EQ(registry.count(), 1u);

    registry.register_singleton<test_counter_service>(service2);
    EXPECT_EQ(registry.count(), 2u);

    registry.clear();
    EXPECT_EQ(registry.count(), 0u);
}

TEST_F(ServiceRegistryTest, GlobalRegistry) {
    // Arrange
    auto service = std::make_shared<test_data_service>("global");

    // Act
    auto& global1 = get_global_registry();
    auto& global2 = get_global_registry();

    // Assert - should be the same instance
    EXPECT_EQ(&global1, &global2);

    // Register and resolve through global registry
    global1.register_singleton<test_data_service>(service);
    auto resolved = global2.resolve<test_data_service>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->get_data(), "global");

    // Clean up
    global1.clear();
}

TEST_F(ServiceRegistryTest, MixedLifetimes) {
    // Arrange
    auto singleton = std::make_shared<test_data_service>("singleton");
    auto factory = []() {
        static int counter = 0;
        return std::make_shared<test_data_service>("transient_" + std::to_string(++counter));
    };

    // Act
    registry.register_singleton<test_data_service>(singleton);

    class another_service {
    public:
        std::string name{"another"};
    };
    registry.register_transient<another_service>([]() {
        return std::make_shared<another_service>();
    });

    auto singleton_resolved1 = registry.resolve<test_data_service>();
    auto singleton_resolved2 = registry.resolve<test_data_service>();
    auto transient_resolved1 = registry.resolve<another_service>();
    auto transient_resolved2 = registry.resolve<another_service>();

    // Assert
    EXPECT_EQ(singleton_resolved1.get(), singleton_resolved2.get());
    EXPECT_NE(transient_resolved1.get(), transient_resolved2.get());
}

TEST_F(ServiceRegistryTest, OverwriteSingleton) {
    // Arrange
    auto service1 = std::make_shared<test_data_service>("first");
    auto service2 = std::make_shared<test_data_service>("second");

    // Act
    registry.register_singleton<test_data_service>(service1);
    auto resolved1 = registry.resolve<test_data_service>();

    registry.register_singleton<test_data_service>(service2); // Overwrite
    auto resolved2 = registry.resolve<test_data_service>();

    // Assert
    ASSERT_NE(resolved1, nullptr);
    ASSERT_NE(resolved2, nullptr);
    EXPECT_EQ(resolved1->get_data(), "first");
    EXPECT_EQ(resolved2->get_data(), "second");
}

TEST_F(ServiceRegistryTest, OverwriteTransient) {
    // Arrange
    auto factory1 = []() { return std::make_shared<test_data_service>("factory1"); };
    auto factory2 = []() { return std::make_shared<test_data_service>("factory2"); };

    // Act
    registry.register_transient<test_data_service>(factory1);
    auto resolved1 = registry.resolve<test_data_service>();

    registry.register_transient<test_data_service>(factory2); // Overwrite
    auto resolved2 = registry.resolve<test_data_service>();

    // Assert
    ASSERT_NE(resolved1, nullptr);
    ASSERT_NE(resolved2, nullptr);
    EXPECT_EQ(resolved1->get_data(), "factory1");
    EXPECT_EQ(resolved2->get_data(), "factory2");
}
