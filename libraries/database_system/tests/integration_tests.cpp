/**
 * BSD 3-Clause License
 * Copyright (c) 2025, Database System Project
 *
 * Integration Tests for Phase 4 Components
 * Tests the interaction between different Phase 4 modules
 */

#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <thread>
#include <future>
#include <vector>

#include "database/database_manager.h"
#include "database/database_types.h"
#include "database/orm/entity.h"
#include "database/monitoring/performance_monitor.h"
#include "database/security/secure_connection.h"
#include "database/async/async_operations.h"

using namespace database;
using namespace database::orm;
using namespace database::monitoring;
using namespace database::security;
using namespace database::async;

// Test entity for integration tests
class IntegrationTestUser : public entity_base
{
    ENTITY_TABLE("integration_users")

    ENTITY_FIELD(int64_t, id, primary_key() | auto_increment())
    ENTITY_FIELD(std::string, username, not_null() | unique())
    ENTITY_FIELD(std::string, email, not_null())
    ENTITY_FIELD(std::string, role, not_null())
    ENTITY_FIELD(bool, is_active, not_null())
    ENTITY_FIELD(std::chrono::system_clock::time_point, created_at, default_now())

    ENTITY_METADATA()

public:
    IntegrationTestUser() {
        is_active = true;
        created_at = std::chrono::system_clock::now();
    }

    bool is_valid() const {
        return !username.get().empty() && !email.get().empty();
    }
};

void IntegrationTestUser::initialize_metadata() {
    metadata_.add_field(id_field());
    metadata_.add_field(username_field());
    metadata_.add_field(email_field());
    metadata_.add_field(role_field());
    metadata_.add_field(is_active_field());
    metadata_.add_field(created_at_field());
}

// Base integration test fixture
class IntegrationTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize all Phase 4 components
        setup_performance_monitoring();
        setup_security_framework();
        setup_async_operations();
        setup_orm_framework();
    }

    void TearDown() override {
        // Cleanup all components
        cleanup_components();
    }

private:
    void setup_performance_monitoring() {
        auto& monitor = performance_monitor::instance();
        monitoring_config config;
        config.enable_query_tracking = true;
        config.enable_connection_tracking = true;
        config.enable_data_access_logging = true;
        config.slow_query_threshold = std::chrono::milliseconds(100);
        monitor.configure(config);
    }

    void setup_security_framework() {
        auto& rbac = rbac_manager::instance();
        auto& logger = audit_logger::instance();

        // Create test roles
        rbac_role admin_role("admin");
        admin_role.add_permission("user.create");
        admin_role.add_permission("user.read");
        admin_role.add_permission("user.update");
        admin_role.add_permission("user.delete");
        rbac.create_role(admin_role);

        rbac_role user_role("user");
        user_role.add_permission("user.read");
        rbac.create_role(user_role);

        // Configure audit logging
        audit_config audit_cfg;
        audit_cfg.enable_database_operations = true;
        audit_cfg.enable_authentication_events = true;
        audit_cfg.enable_authorization_events = true;
        logger.configure(audit_cfg);
    }

    void setup_async_operations() {
        auto& executor = async_executor::instance();
        async_config config;
        config.thread_pool_size = 4;
        config.max_concurrent_operations = 20;
        config.operation_timeout = std::chrono::seconds(30);
        executor.configure(config);
    }

    void setup_orm_framework() {
        entity_manager& mgr = entity_manager::instance();
        mgr.register_entity<IntegrationTestUser>();
    }

    void cleanup_components() {
        // Reset all components to clean state
        auto& db = database_manager::handle();
        db.disconnect();
    }
};

// Test 1: ORM + Security Integration
class ORMSecurityIntegrationTest : public IntegrationTestBase {
protected:
    void SetUp() override {
        IntegrationTestBase::SetUp();

        // Create test user in RBAC system
        auto& rbac = rbac_manager::instance();
        rbac_user test_user("integration.user", "integration@test.com");
        rbac.create_user(test_user);
        rbac.assign_role_to_user("integration.user", "user");
    }
};

TEST_F(ORMSecurityIntegrationTest, SecureEntityOperations) {
    auto& rbac = rbac_manager::instance();
    auto& logger = audit_logger::instance();

    // Test entity creation with permission check
    std::string user_id = "integration.user";

    // Check permission before operation
    bool can_read = rbac.check_permission(user_id, "user.read");
    bool can_create = rbac.check_permission(user_id, "user.create");

    EXPECT_TRUE(can_read);
    EXPECT_FALSE(can_create); // user role doesn't have create permission

    // Create entity and log the operation
    IntegrationTestUser entity;
    entity.username = "test_user";
    entity.email = "test@example.com";
    entity.role = "user";

    // Log the attempted operation
    audit_event event;
    event.event_type = audit_event_type::data_access;
    event.user_id = user_id;
    event.event_description = "Entity creation attempt";
    event.success = can_create;
    event.timestamp = std::chrono::system_clock::now();
    event.resource_accessed = "integration_users";

    logger.log_event(event);

    // Verify audit log contains the event
    auto events = logger.get_events_by_user(user_id);
    EXPECT_GT(events.size(), 0);

    // Verify entity metadata includes security information
    const auto& metadata = entity.get_metadata();
    EXPECT_EQ(metadata.table_name(), "integration_users");
    EXPECT_TRUE(entity.is_valid());
}

// Test 2: Performance Monitoring + Async Operations Integration
class PerformanceAsyncIntegrationTest : public IntegrationTestBase {
};

TEST_F(PerformanceAsyncIntegrationTest, MonitoredAsyncOperations) {
    auto& monitor = performance_monitor::instance();
    auto& executor = async_executor::instance();

    // Submit multiple async operations while monitoring performance
    std::vector<std::future<query_result>> futures;

    for (int i = 0; i < 10; ++i) {
        auto future = executor.execute_async([i, &monitor]() -> query_result {
            auto start_time = std::chrono::high_resolution_clock::now();

            // Simulate database operation
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + (i * 10)));

            auto end_time = std::chrono::high_resolution_clock::now();
            auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            // Record query metrics
            query_metrics metrics;
            metrics.query_type = "SELECT";
            metrics.execution_time = execution_time;
            metrics.success = true;
            metrics.rows_affected = i + 1;
            metrics.timestamp = start_time;

            monitor.record_query_execution(metrics);

            query_result result;
            result.success = true;
            result.execution_time = execution_time;
            result.rows_affected = i + 1;

            return result;
        });

        futures.push_back(std::move(future));
    }

    // Wait for all operations to complete
    for (auto& future : futures) {
        auto result = future.get();
        EXPECT_TRUE(result.success);
        EXPECT_GT(result.execution_time.count(), 0);
    }

    // Verify performance metrics were recorded
    const auto& stats = monitor.get_query_statistics();
    EXPECT_GE(stats.total_queries, 10);
    EXPECT_GT(stats.average_execution_time.count(), 0);
}

// Test 3: Security + Monitoring Integration
class SecurityMonitoringIntegrationTest : public IntegrationTestBase {
};

TEST_F(SecurityMonitoringIntegrationTest, MonitoredSecurityEvents) {
    auto& rbac = rbac_manager::instance();
    auto& logger = audit_logger::instance();
    auto& monitor = performance_monitor::instance();

    // Create users with different permission levels
    std::vector<std::tuple<std::string, std::string, std::string>> test_users = {
        {"admin.user", "admin@test.com", "admin"},
        {"normal.user", "normal@test.com", "user"},
        {"unauthorized.user", "unauthorized@test.com", "user"}
    };

    for (const auto& [username, email, role] : test_users) {
        rbac_user user(username, email);
        rbac.create_user(user);
        rbac.assign_role_to_user(username, role);
    }

    // Simulate various security events and monitor performance impact
    std::vector<std::string> operations = {
        "user.read", "user.create", "user.update", "user.delete"
    };

    for (const auto& [username, email, role] : test_users) {
        for (const auto& operation : operations) {
            auto start_time = std::chrono::high_resolution_clock::now();

            // Check permission (this is the operation being monitored)
            bool has_permission = rbac.check_permission(username, operation);

            auto end_time = std::chrono::high_resolution_clock::now();
            auto check_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

            // Log the authorization event
            audit_event event;
            event.event_type = audit_event_type::authorization;
            event.user_id = username;
            event.event_description = "Permission check: " + operation;
            event.success = has_permission;
            event.timestamp = start_time;
            event.resource_accessed = operation;

            logger.log_event(event);

            // Record performance metrics for security operations
            query_metrics metrics;
            metrics.query_type = "SECURITY_CHECK";
            metrics.execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(check_time);
            metrics.success = true;
            metrics.rows_affected = 1;
            metrics.timestamp = start_time;

            monitor.record_query_execution(metrics);
        }
    }

    // Verify audit events were logged
    auto admin_events = logger.get_events_by_user("admin.user");
    auto normal_events = logger.get_events_by_user("normal.user");
    auto unauthorized_events = logger.get_events_by_user("unauthorized.user");

    EXPECT_EQ(admin_events.size(), 4); // admin should have 4 events (all operations)
    EXPECT_EQ(normal_events.size(), 4); // normal user should have 4 events
    EXPECT_EQ(unauthorized_events.size(), 4); // unauthorized user should have 4 events

    // Verify performance metrics include security operations
    const auto& stats = monitor.get_query_statistics();
    EXPECT_GT(stats.total_queries, 12); // At least 12 security checks recorded
}

// Test 4: Full System Integration
class FullSystemIntegrationTest : public IntegrationTestBase {
};

TEST_F(FullSystemIntegrationTest, CompleteWorkflow) {
    auto& rbac = rbac_manager::instance();
    auto& logger = audit_logger::instance();
    auto& monitor = performance_monitor::instance();
    auto& executor = async_executor::instance();
    auto& entity_mgr = entity_manager::instance();

    // Create a complete user management workflow
    std::string admin_user = "system.admin";
    rbac_user admin(admin_user, "admin@system.com");
    rbac.create_user(admin);
    rbac.assign_role_to_user(admin_user, "admin");

    // Test async entity operations with full security and monitoring
    auto workflow_future = executor.execute_async([&]() -> bool {
        auto start_time = std::chrono::high_resolution_clock::now();

        // Step 1: Check permissions
        bool can_create = rbac.check_permission(admin_user, "user.create");
        if (!can_create) {
            audit_event denied_event;
            denied_event.event_type = audit_event_type::authorization;
            denied_event.user_id = admin_user;
            denied_event.event_description = "Access denied for user creation";
            denied_event.success = false;
            denied_event.timestamp = std::chrono::system_clock::now();
            logger.log_event(denied_event);
            return false;
        }

        // Step 2: Create entities
        std::vector<IntegrationTestUser> users;
        for (int i = 0; i < 5; ++i) {
            IntegrationTestUser user;
            user.username = "workflow_user_" + std::to_string(i);
            user.email = "workflow" + std::to_string(i) + "@test.com";
            user.role = (i % 2 == 0) ? "admin" : "user";
            users.push_back(std::move(user));
        }

        // Step 3: Log successful operations
        for (const auto& user : users) {
            audit_event create_event;
            create_event.event_type = audit_event_type::data_access;
            create_event.user_id = admin_user;
            create_event.event_description = "Created user: " + user.username.get();
            create_event.success = true;
            create_event.timestamp = std::chrono::system_clock::now();
            create_event.resource_accessed = "integration_users";
            logger.log_event(create_event);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto workflow_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Step 4: Record performance metrics
        query_metrics metrics;
        metrics.query_type = "WORKFLOW";
        metrics.execution_time = workflow_time;
        metrics.success = true;
        metrics.rows_affected = users.size();
        metrics.timestamp = start_time;
        monitor.record_query_execution(metrics);

        return true;
    });

    // Wait for workflow completion
    bool workflow_success = workflow_future.get();
    EXPECT_TRUE(workflow_success);

    // Verify all systems recorded the workflow
    auto admin_events = logger.get_events_by_user(admin_user);
    EXPECT_GT(admin_events.size(), 5); // At least 5 events (1 creation + 5 user operations)

    const auto& perf_stats = monitor.get_query_statistics();
    EXPECT_GT(perf_stats.total_queries, 0);

    // Verify entity manager has the registered entity
    const auto& user_metadata = entity_mgr.get_metadata<IntegrationTestUser>();
    EXPECT_EQ(user_metadata.table_name(), "integration_users");
}

// Test 5: Error Handling and Recovery Integration
class ErrorHandlingIntegrationTest : public IntegrationTestBase {
};

TEST_F(ErrorHandlingIntegrationTest, FailureRecoveryWorkflow) {
    auto& rbac = rbac_manager::instance();
    auto& logger = audit_logger::instance();
    auto& monitor = performance_monitor::instance();
    auto& executor = async_executor::instance();

    // Create user without sufficient permissions
    rbac_user limited_user("limited.user", "limited@test.com");
    rbac.create_user(limited_user);
    rbac.assign_role_to_user("limited.user", "user"); // Only has read permission

    // Test error handling in async operations
    auto error_test_future = executor.execute_async([&]() -> bool {
        std::string user_id = "limited.user";

        // Attempt unauthorized operation
        bool can_delete = rbac.check_permission(user_id, "user.delete");

        // Log the authorization failure
        audit_event auth_event;
        auth_event.event_type = audit_event_type::authorization;
        auth_event.user_id = user_id;
        auth_event.event_description = "Attempted unauthorized delete operation";
        auth_event.success = can_delete;
        auth_event.timestamp = std::chrono::system_clock::now();
        logger.log_event(auth_event);

        // Simulate failed query due to insufficient permissions
        auto start_time = std::chrono::high_resolution_clock::now();

        // This would be the actual failed operation
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto end_time = std::chrono::high_resolution_clock::now();
        auto failed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Record failed operation in performance metrics
        query_metrics failed_metrics;
        failed_metrics.query_type = "DELETE";
        failed_metrics.execution_time = failed_time;
        failed_metrics.success = false; // Failed operation
        failed_metrics.rows_affected = 0;
        failed_metrics.timestamp = start_time;
        monitor.record_query_execution(failed_metrics);

        return !can_delete; // Return true if operation properly failed
    });

    bool error_handling_success = error_test_future.get();
    EXPECT_TRUE(error_handling_success);

    // Verify error was properly logged and monitored
    auto failed_events = logger.get_failed_events();
    EXPECT_GT(failed_events.size(), 0);

    const auto& stats = monitor.get_query_statistics();
    EXPECT_GT(stats.failed_queries, 0);
    EXPECT_LT(stats.successful_queries / static_cast<double>(stats.total_queries), 1.0);
}

// Test 6: Concurrent Operations Integration
class ConcurrentOperationsIntegrationTest : public IntegrationTestBase {
};

TEST_F(ConcurrentOperationsIntegrationTest, ConcurrentSecureOperations) {
    auto& rbac = rbac_manager::instance();
    auto& logger = audit_logger::instance();
    auto& monitor = performance_monitor::instance();
    auto& executor = async_executor::instance();

    // Create multiple concurrent users
    std::vector<std::string> user_ids;
    for (int i = 0; i < 10; ++i) {
        std::string user_id = "concurrent_user_" + std::to_string(i);
        rbac_user user(user_id, user_id + "@test.com");
        rbac.create_user(user);
        rbac.assign_role_to_user(user_id, (i % 2 == 0) ? "admin" : "user");
        user_ids.push_back(user_id);
    }

    // Launch concurrent operations
    std::vector<std::future<bool>> operation_futures;

    for (const auto& user_id : user_ids) {
        auto future = executor.execute_async([&, user_id]() -> bool {
            // Perform multiple operations concurrently
            std::vector<std::string> operations = {"user.read", "user.create", "user.update"};
            bool all_successful = true;

            for (const auto& operation : operations) {
                auto start_time = std::chrono::high_resolution_clock::now();

                bool has_permission = rbac.check_permission(user_id, operation);

                auto end_time = std::chrono::high_resolution_clock::now();
                auto op_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                // Log concurrent operation
                audit_event event;
                event.event_type = audit_event_type::authorization;
                event.user_id = user_id;
                event.event_description = "Concurrent operation: " + operation;
                event.success = has_permission;
                event.timestamp = start_time;
                logger.log_event(event);

                // Monitor concurrent performance
                query_metrics metrics;
                metrics.query_type = "CONCURRENT_" + operation;
                metrics.execution_time = op_time;
                metrics.success = has_permission;
                metrics.rows_affected = 1;
                metrics.timestamp = start_time;
                monitor.record_query_execution(metrics);

                if (!has_permission && operation == "user.read") {
                    all_successful = false; // Even basic read should work
                }
            }

            return all_successful;
        });

        operation_futures.push_back(std::move(future));
    }

    // Wait for all concurrent operations
    int successful_operations = 0;
    for (auto& future : operation_futures) {
        if (future.get()) {
            successful_operations++;
        }
    }

    // Verify concurrent operations worked correctly
    EXPECT_GT(successful_operations, 0);

    // Verify all systems handled concurrency properly
    const auto& stats = monitor.get_query_statistics();
    EXPECT_GE(stats.total_queries, user_ids.size() * 3); // Each user performed 3 operations

    // Verify audit log captured all concurrent events
    size_t total_events = 0;
    for (const auto& user_id : user_ids) {
        auto events = logger.get_events_by_user(user_id);
        total_events += events.size();
    }
    EXPECT_GE(total_events, user_ids.size() * 3);
}

// Main function for running integration tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}