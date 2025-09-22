/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file di_container_test.cpp
 * @brief Unit tests for DI container implementations
 */

#include <gtest/gtest.h>
#include "../../sources/logger/di/lightweight_di_container.h"
#include "../../sources/logger/di/di_container_factory.h"
#include "../../sources/logger/writers/base_writer.h"
#include <memory>
#include <thread>
#include <vector>
#include <chrono>

using namespace logger_module;

// Mock writer for testing
class mock_writer : public base_writer {
private:
    std::string name_;
    static int instance_count_;
    
public:
    explicit mock_writer(const std::string& name = "mock")
        : name_(name) {
        ++instance_count_;
    }
    
    ~mock_writer() override {
        --instance_count_;
    }
    
    result_void write(thread_module::log_level level,
                      const std::string& message,
                      const std::string& file,
                      int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) override {
        return {};
    }
    
    result_void flush() override {
        return {};
    }
    
    bool is_healthy() const override {
        return true;
    }
    
    std::string get_name() const override {
        return name_;
    }
    
    static int get_instance_count() {
        return instance_count_;
    }
    
    static void reset_instance_count() {
        instance_count_ = 0;
    }
};

int mock_writer::instance_count_ = 0;

// Test fixture for DI container tests
class di_container_test : public ::testing::Test {
protected:
    void SetUp() override {
        mock_writer::reset_instance_count();
    }
    
    void TearDown() override {
        mock_writer::reset_instance_count();
    }
};

// Lightweight Container Tests
TEST_F(di_container_test, lightweight_container_factory_registration) {
    lightweight_di_container<base_writer> container;
    
    // Register a factory
    auto result = container.register_factory("test_writer", []() {
        return std::make_shared<mock_writer>("test");
    });
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(container.is_registered("test_writer"));
    EXPECT_EQ(container.size(), 1);
}

TEST_F(di_container_test, lightweight_container_resolve_factory) {
    lightweight_di_container<base_writer> container;
    
    // Register a factory
    container.register_factory("test_writer", []() {
        return std::make_shared<mock_writer>("factory_created");
    });
    
    // Resolve should create a new instance
    auto result1 = container.resolve("test_writer");
    ASSERT_TRUE(result1);
    
    auto writer1 = std::dynamic_pointer_cast<mock_writer>(result1.value());
    ASSERT_NE(writer1, nullptr);
    EXPECT_EQ(writer1->get_name(), "factory_created");
    
    // Resolve again should create another instance
    auto result2 = container.resolve("test_writer");
    ASSERT_TRUE(result2);
    
    auto writer2 = std::dynamic_pointer_cast<mock_writer>(result2.value());
    ASSERT_NE(writer2, nullptr);
    EXPECT_NE(writer1, writer2);  // Different instances
}

TEST_F(di_container_test, lightweight_container_singleton_registration) {
    lightweight_di_container<base_writer> container;
    
    auto singleton = std::make_shared<mock_writer>("singleton");
    
    // Register a singleton
    auto result = container.register_singleton("singleton_writer", singleton);
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(container.is_registered("singleton_writer"));
}

TEST_F(di_container_test, lightweight_container_resolve_singleton) {
    lightweight_di_container<base_writer> container;
    
    auto singleton = std::make_shared<mock_writer>("singleton");
    container.register_singleton("singleton_writer", singleton);
    
    // Resolve should return the same instance
    auto result1 = container.resolve("singleton_writer");
    ASSERT_TRUE(result1);
    
    auto result2 = container.resolve("singleton_writer");
    ASSERT_TRUE(result2);
    
    EXPECT_EQ(result1.value(), result2.value());  // Same instance
    EXPECT_EQ(result1.value(), singleton);  // Same as registered
}

TEST_F(di_container_test, lightweight_container_resolve_not_found) {
    lightweight_di_container<base_writer> container;
    
    auto result = container.resolve("non_existent");
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error_code(), error_code::component_not_found);
}

TEST_F(di_container_test, lightweight_container_invalid_registration) {
    lightweight_di_container<base_writer> container;
    
    // Empty name
    auto result1 = container.register_factory("", []() {
        return std::make_shared<mock_writer>();
    });
    ASSERT_FALSE(result1);
    EXPECT_EQ(result1.error_code(), error_code::invalid_argument);
    
    // Null factory
    std::function<std::shared_ptr<base_writer>()> null_factory;
    auto result2 = container.register_factory("test", null_factory);
    ASSERT_FALSE(result2);
    EXPECT_EQ(result2.error_code(), error_code::invalid_argument);
    
    // Null singleton
    std::shared_ptr<base_writer> null_singleton;
    auto result3 = container.register_singleton("test", null_singleton);
    ASSERT_FALSE(result3);
    EXPECT_EQ(result3.error_code(), error_code::invalid_argument);
}

TEST_F(di_container_test, lightweight_container_clear) {
    lightweight_di_container<base_writer> container;
    
    // Register multiple items
    container.register_factory("factory1", []() {
        return std::make_shared<mock_writer>("f1");
    });
    container.register_singleton("singleton1", 
        std::make_shared<mock_writer>("s1"));
    
    EXPECT_EQ(container.size(), 2);
    
    // Clear all
    auto result = container.clear();
    ASSERT_TRUE(result);
    
    EXPECT_EQ(container.size(), 0);
    EXPECT_FALSE(container.is_registered("factory1"));
    EXPECT_FALSE(container.is_registered("singleton1"));
}

TEST_F(di_container_test, lightweight_container_thread_safety) {
    lightweight_di_container<base_writer> container;
    
    const int num_threads = 10;
    const int operations_per_thread = 100;
    std::vector<std::thread> threads;
    
    // Register a factory
    container.register_factory("concurrent", []() {
        return std::make_shared<mock_writer>("concurrent");
    });
    
    // Spawn threads that resolve concurrently
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&container, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                auto result = container.resolve("concurrent");
                EXPECT_TRUE(result);
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Container should still be consistent
    EXPECT_TRUE(container.is_registered("concurrent"));
}

// DI Factory Tests
TEST_F(di_container_test, factory_create_lightweight) {
    auto container = di_container_factory::create_container<base_writer>(
        di_container_factory::container_type::lightweight
    );
    
    ASSERT_NE(container, nullptr);
    
    // Should be able to use it
    container->register_factory("test", []() {
        return std::make_shared<mock_writer>();
    });
    
    auto result = container->resolve("test");
    ASSERT_TRUE(result);
}

TEST_F(di_container_test, factory_create_automatic) {
    auto container = di_container_factory::create_container<base_writer>(
        di_container_factory::container_type::automatic
    );
    
    ASSERT_NE(container, nullptr);
    
    // Should work regardless of thread_system availability
    container->register_factory("test", []() {
        return std::make_shared<mock_writer>();
    });
    
    auto result = container->resolve("test");
    ASSERT_TRUE(result);
}

TEST_F(di_container_test, factory_create_best_available) {
    auto container = di_container_factory::create_best_available<base_writer>();
    
    ASSERT_NE(container, nullptr);
    
    // Should work with best available implementation
    container->register_factory("test", []() {
        return std::make_shared<mock_writer>();
    });
    
    auto result = container->resolve("test");
    ASSERT_TRUE(result);
}

TEST_F(di_container_test, factory_type_check) {
    // Check if we can determine available type
    auto type = di_container_factory::get_available_type();
    
    // Should be either lightweight or thread_system
    EXPECT_TRUE(type == di_container_factory::container_type::lightweight ||
                type == di_container_factory::container_type::thread_system);
    
    // Get name should work
    const char* name = di_container_factory::get_container_type_name(type);
    EXPECT_NE(name, nullptr);
    EXPECT_NE(std::string(name), "unknown");
}

// Template helper methods tests
TEST_F(di_container_test, lightweight_container_register_type) {
    lightweight_di_container<base_writer> container;
    
    // Register type with default constructor
    auto result = container.register_type<mock_writer>("typed_writer");
    ASSERT_TRUE(result);
    
    // Resolve should work
    auto resolved = container.resolve("typed_writer");
    ASSERT_TRUE(resolved);
    
    auto writer = std::dynamic_pointer_cast<mock_writer>(resolved.value());
    ASSERT_NE(writer, nullptr);
}

TEST_F(di_container_test, lightweight_container_register_type_with_args) {
    lightweight_di_container<base_writer> container;
    
    // Register type with constructor arguments
    auto result = container.register_type_with_args<mock_writer>(
        "typed_writer_args", "custom_name"
    );
    ASSERT_TRUE(result);
    
    // Resolve should work
    auto resolved = container.resolve("typed_writer_args");
    ASSERT_TRUE(resolved);
    
    auto writer = std::dynamic_pointer_cast<mock_writer>(resolved.value());
    ASSERT_NE(writer, nullptr);
    EXPECT_EQ(writer->get_name(), "custom_name");
}

// Factory returning null test
TEST_F(di_container_test, lightweight_container_factory_returns_null) {
    lightweight_di_container<base_writer> container;
    
    // Register a factory that returns null
    container.register_factory("null_factory", []() {
        return std::shared_ptr<base_writer>(nullptr);
    });
    
    // Resolve should fail
    auto result = container.resolve("null_factory");
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error_code(), error_code::creation_failed);
}

// Factory throwing exception test
TEST_F(di_container_test, lightweight_container_factory_throws) {
    lightweight_di_container<base_writer> container;
    
    // Register a factory that throws
    container.register_factory("throwing_factory", []() -> std::shared_ptr<base_writer> {
        throw std::runtime_error("Factory error");
    });
    
    // Resolve should fail gracefully
    auto result = container.resolve("throwing_factory");
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error_code(), error_code::creation_failed);
}