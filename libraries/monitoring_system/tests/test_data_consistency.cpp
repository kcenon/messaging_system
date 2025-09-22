#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <monitoring/reliability/data_consistency.h>

using namespace monitoring_system;

class DataConsistencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        call_count = 0;
        success_count = 0;
        rollback_count = 0;
    }
    
    void TearDown() override {
        // Clean up any resources if needed
    }
    
    std::atomic<int> call_count{0};
    std::atomic<int> success_count{0};
    std::atomic<int> rollback_count{0};
    
    // Helper function for testing operations
    result_void test_operation() {
        ++call_count;
        ++success_count;
        return result_void{};
    }
    
    // Helper function that fails
    result_void failing_operation() {
        ++call_count;
        return result_void{monitoring_error_code::operation_failed, "Simulated failure"};
    }
    
    // Helper function for rollback
    result_void rollback_operation() {
        ++rollback_count;
        return result_void{};
    }
    
    // Helper validation function
    validation_result test_validation() {
        return validation_result::valid;
    }
    
    validation_result failing_validation() {
        return validation_result::invalid;
    }
    
    result_void test_repair() {
        return result_void{};
    }
};

// Transaction Operation Tests
TEST_F(DataConsistencyTest, TransactionOperationBasic) {
    auto op = std::make_unique<transaction_operation>(
        "test_op",
        [this]() { return test_operation(); },
        [this]() { return rollback_operation(); }
    );
    
    EXPECT_EQ(op->name(), "test_op");
    EXPECT_FALSE(op->is_executed());
    
    auto result = op->execute();
    EXPECT_TRUE(result);
    EXPECT_TRUE(op->is_executed());
    EXPECT_EQ(call_count.load(), 1);
    EXPECT_EQ(success_count.load(), 1);
    
    auto rollback_result = op->rollback();
    EXPECT_TRUE(rollback_result);
    EXPECT_EQ(rollback_count.load(), 1);
}

// Transaction Tests
TEST_F(DataConsistencyTest, TransactionCommitSuccess) {
    transaction_config config;
    transaction tx("test_tx", config);
    
    // Add operations
    auto op1 = std::make_unique<transaction_operation>(
        "op1", [this]() { return test_operation(); });
    auto op2 = std::make_unique<transaction_operation>(
        "op2", [this]() { return test_operation(); });
    
    auto add_result1 = tx.add_operation(std::move(op1));
    auto add_result2 = tx.add_operation(std::move(op2));
    EXPECT_TRUE(add_result1);
    EXPECT_TRUE(add_result2);
    
    EXPECT_EQ(tx.operation_count(), 2);
    EXPECT_EQ(tx.state(), transaction_state::active);
    
    // Commit transaction
    auto commit_result = tx.commit();
    EXPECT_TRUE(commit_result);
    EXPECT_EQ(tx.state(), transaction_state::committed);
    EXPECT_EQ(call_count.load(), 2);
    EXPECT_EQ(success_count.load(), 2);
}

TEST_F(DataConsistencyTest, TransactionRollbackOnFailure) {
    transaction_config config;
    transaction tx("test_tx", config);
    
    // Add operations - second one will fail
    auto op1 = std::make_unique<transaction_operation>(
        "op1", 
        [this]() { return test_operation(); },
        [this]() { return rollback_operation(); });
    auto op2 = std::make_unique<transaction_operation>(
        "op2", [this]() { return failing_operation(); });
    
    tx.add_operation(std::move(op1));
    tx.add_operation(std::move(op2));
    
    // Commit should fail and rollback
    auto commit_result = tx.commit();
    EXPECT_FALSE(commit_result);
    EXPECT_EQ(tx.state(), transaction_state::aborted);
    EXPECT_EQ(call_count.load(), 2); // Both operations attempted
    EXPECT_EQ(success_count.load(), 1); // Only first succeeded
    EXPECT_EQ(rollback_count.load(), 1); // First operation rolled back
}

TEST_F(DataConsistencyTest, TransactionManualAbort) {
    transaction_config config;
    transaction tx("test_tx", config);
    
    auto op = std::make_unique<transaction_operation>(
        "op", 
        [this]() { return test_operation(); },
        [this]() { return rollback_operation(); });
    
    tx.add_operation(std::move(op));
    
    // Manually abort
    auto abort_result = tx.abort();
    EXPECT_TRUE(abort_result);
    EXPECT_EQ(tx.state(), transaction_state::aborted);
    
    // Should not be able to add more operations
    auto op2 = std::make_unique<transaction_operation>("op2", [this]() { return test_operation(); });
    auto add_result = tx.add_operation(std::move(op2));
    EXPECT_FALSE(add_result);
}

TEST_F(DataConsistencyTest, TransactionTimeout) {
    transaction_config config;
    config.timeout = std::chrono::milliseconds(50);
    transaction tx("test_tx", config);
    
    auto op = std::make_unique<transaction_operation>("op", [this]() { return test_operation(); });
    tx.add_operation(std::move(op));
    
    // Wait for timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto commit_result = tx.commit();
    EXPECT_FALSE(commit_result);
    EXPECT_EQ(tx.state(), transaction_state::aborted);
}

// State Validator Tests
TEST_F(DataConsistencyTest, StateValidatorBasicValidation) {
    validation_config config;
    config.validation_interval = std::chrono::milliseconds(100);
    state_validator validator("test_validator", config);
    
    // Add validation rule
    auto add_result = validator.add_validation_rule(
        "test_rule",
        [this]() { return test_validation(); },
        [this]() { return test_repair(); }
    );
    EXPECT_TRUE(add_result);
    
    // Manual validation
    auto validation_result = validator.validate();
    EXPECT_TRUE(validation_result);
    
    auto results = validation_result.value();
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results["test_rule"], validation_result::valid);
    
    auto health = validator.is_healthy();
    EXPECT_TRUE(health);
    EXPECT_TRUE(health.value());
}

TEST_F(DataConsistencyTest, StateValidatorFailureAndRepair) {
    validation_config config;
    config.enable_auto_repair = true;
    state_validator validator("test_validator", config);
    
    // Add validation rule that fails initially
    std::atomic<bool> should_fail{true};
    validator.add_validation_rule(
        "failing_rule",
        [&should_fail]() { 
            return should_fail.load() ? validation_result::invalid : validation_result::valid; 
        },
        [&should_fail]() { 
            should_fail = false; // Repair fixes the issue
            return result_void{}; 
        }
    );
    
    // First validation should fail and trigger repair
    auto validation_result = validator.validate();
    EXPECT_TRUE(validation_result);
    
    auto results = validation_result.value();
    EXPECT_EQ(results["failing_rule"], validation_result::invalid);
    EXPECT_EQ(results["failing_rule_after_repair"], validation_result::valid);
    
    auto metrics = validator.get_metrics();
    EXPECT_EQ(metrics.validation_runs.load(), 1);
    EXPECT_EQ(metrics.repair_operations.load(), 1);
}

TEST_F(DataConsistencyTest, StateValidatorContinuousValidation) {
    validation_config config;
    config.validation_interval = std::chrono::milliseconds(50);
    state_validator validator("test_validator", config);
    
    std::atomic<int> validation_calls{0};
    validator.add_validation_rule(
        "continuous_rule",
        [&validation_calls]() { 
            validation_calls++; 
            return validation_result::valid; 
        }
    );
    
    // Start continuous validation
    auto start_result = validator.start();
    EXPECT_TRUE(start_result);
    
    // Wait for several validation cycles
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Stop validation
    auto stop_result = validator.stop();
    EXPECT_TRUE(stop_result);
    
    // Should have run multiple validations
    EXPECT_GT(validation_calls.load(), 2);
}

// Transaction Manager Tests
TEST_F(DataConsistencyTest, TransactionManagerBasicOperations) {
    transaction_config config;
    transaction_manager manager("test_manager", config);
    
    // Begin transaction
    auto begin_result = manager.begin_transaction("tx1");
    EXPECT_TRUE(begin_result);
    
    auto tx = begin_result.value();
    EXPECT_EQ(tx->id(), "tx1");
    EXPECT_EQ(tx->state(), transaction_state::active);
    EXPECT_EQ(manager.active_transaction_count(), 1);
    
    // Add operation and commit
    auto op = std::make_unique<transaction_operation>("op", [this]() { return test_operation(); });
    tx->add_operation(std::move(op));
    
    auto commit_result = manager.commit_transaction("tx1");
    EXPECT_TRUE(commit_result);
    EXPECT_EQ(manager.active_transaction_count(), 0);
    EXPECT_EQ(manager.completed_transaction_count(), 1);
    
    auto metrics = manager.get_metrics();
    EXPECT_EQ(metrics.total_transactions.load(), 1);
    EXPECT_EQ(metrics.committed_transactions.load(), 1);
    EXPECT_EQ(metrics.aborted_transactions.load(), 0);
}

TEST_F(DataConsistencyTest, TransactionManagerAbort) {
    transaction_config config;
    transaction_manager manager("test_manager", config);
    
    auto begin_result = manager.begin_transaction("tx1");
    EXPECT_TRUE(begin_result);
    
    auto abort_result = manager.abort_transaction("tx1");
    EXPECT_TRUE(abort_result);
    
    auto metrics = manager.get_metrics();
    EXPECT_EQ(metrics.total_transactions.load(), 1);
    EXPECT_EQ(metrics.committed_transactions.load(), 0);
    EXPECT_EQ(metrics.aborted_transactions.load(), 1);
    EXPECT_NEAR(metrics.get_abort_rate(), 1.0, 0.01);
}

TEST_F(DataConsistencyTest, TransactionManagerDuplicateTransaction) {
    transaction_config config;
    transaction_manager manager("test_manager", config);
    
    auto begin_result1 = manager.begin_transaction("tx1");
    EXPECT_TRUE(begin_result1);
    
    // Should fail with duplicate ID
    auto begin_result2 = manager.begin_transaction("tx1");
    EXPECT_FALSE(begin_result2);
    EXPECT_EQ(begin_result2.get_error().code, monitoring_error_code::already_exists);
}

TEST_F(DataConsistencyTest, TransactionManagerDeadlockDetection) {
    transaction_config config;
    config.timeout = std::chrono::milliseconds(100);
    transaction_manager manager("test_manager", config);
    
    // Create long-running transaction
    auto begin_result = manager.begin_transaction("long_tx");
    EXPECT_TRUE(begin_result);
    
    // Wait longer than timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    auto deadlocks = manager.detect_deadlocks();
    EXPECT_TRUE(deadlocks);
    EXPECT_EQ(deadlocks.value().size(), 1);
    EXPECT_EQ(deadlocks.value()[0], "long_tx");
    
    auto metrics = manager.get_metrics();
    EXPECT_EQ(metrics.deadlocks_detected.load(), 1);
}

TEST_F(DataConsistencyTest, TransactionManagerCleanup) {
    transaction_config config;
    transaction_manager manager("test_manager", config);
    
    // Create and commit transaction
    auto begin_result = manager.begin_transaction("tx1");
    EXPECT_TRUE(begin_result);
    
    auto op = std::make_unique<transaction_operation>("op", [this]() { return test_operation(); });
    begin_result.value()->add_operation(std::move(op));
    
    manager.commit_transaction("tx1");
    EXPECT_EQ(manager.completed_transaction_count(), 1);
    
    // Cleanup should remove old transactions
    manager.cleanup_completed_transactions(std::chrono::milliseconds(0));
    EXPECT_EQ(manager.completed_transaction_count(), 0);
}

// Data Consistency Manager Tests
TEST_F(DataConsistencyTest, DataConsistencyManagerTransactionManagers) {
    data_consistency_manager consistency_manager("test_consistency");
    
    transaction_config tx_config;
    auto add_result = consistency_manager.add_transaction_manager("tx_manager", tx_config);
    EXPECT_TRUE(add_result);
    
    auto manager = consistency_manager.get_transaction_manager("tx_manager");
    EXPECT_NE(manager, nullptr);
    EXPECT_EQ(manager->get_name(), "tx_manager");
    
    // Should fail to add duplicate
    auto duplicate_result = consistency_manager.add_transaction_manager("tx_manager", tx_config);
    EXPECT_FALSE(duplicate_result);
    EXPECT_EQ(duplicate_result.get_error().code, monitoring_error_code::already_exists);
}

TEST_F(DataConsistencyTest, DataConsistencyManagerStateValidators) {
    data_consistency_manager consistency_manager("test_consistency");
    
    validation_config val_config;
    auto add_result = consistency_manager.add_state_validator("validator", val_config);
    EXPECT_TRUE(add_result);
    
    auto validator = consistency_manager.get_state_validator("validator");
    EXPECT_NE(validator, nullptr);
    EXPECT_EQ(validator->get_name(), "validator");
}

TEST_F(DataConsistencyTest, DataConsistencyManagerGlobalOperations) {
    data_consistency_manager consistency_manager("test_consistency");
    
    // Add validators
    validation_config config;
    config.validation_interval = std::chrono::milliseconds(100);
    
    consistency_manager.add_state_validator("validator1", config);
    consistency_manager.add_state_validator("validator2", config);
    
    // Start all validators
    auto start_result = consistency_manager.start_all_validators();
    EXPECT_TRUE(start_result);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Stop all validators
    auto stop_result = consistency_manager.stop_all_validators();
    EXPECT_TRUE(stop_result);
}

TEST_F(DataConsistencyTest, DataConsistencyManagerHealthCheck) {
    data_consistency_manager consistency_manager("test_consistency");
    
    // Add components
    transaction_config tx_config;
    consistency_manager.add_transaction_manager("tx_manager", tx_config);
    
    validation_config val_config;
    consistency_manager.add_state_validator("validator", val_config);
    
    // Should be healthy initially
    auto health = consistency_manager.is_healthy();
    EXPECT_TRUE(health);
    EXPECT_TRUE(health.value());
}

TEST_F(DataConsistencyTest, DataConsistencyManagerMetrics) {
    data_consistency_manager consistency_manager("test_consistency");
    
    // Add components
    transaction_config tx_config;
    consistency_manager.add_transaction_manager("tx_manager", tx_config);
    
    validation_config val_config;
    consistency_manager.add_state_validator("validator", val_config);
    
    // Get all metrics
    auto all_metrics = consistency_manager.get_all_metrics();
    EXPECT_EQ(all_metrics.size(), 2);
    EXPECT_TRUE(all_metrics.find("tx_manager_transactions") != all_metrics.end());
    EXPECT_TRUE(all_metrics.find("validator_validation") != all_metrics.end());
}

// Configuration Validation Tests
TEST_F(DataConsistencyTest, TransactionConfigValidation) {
    transaction_config config;
    
    // Valid config
    config.timeout = std::chrono::seconds(30);
    config.lock_timeout = std::chrono::seconds(10);
    config.max_retries = 3;
    EXPECT_TRUE(config.validate());
    
    // Invalid timeout
    config.timeout = std::chrono::milliseconds(0);
    EXPECT_FALSE(config.validate());
    
    // Invalid lock timeout
    config.timeout = std::chrono::seconds(30);
    config.lock_timeout = std::chrono::milliseconds(0);
    EXPECT_FALSE(config.validate());
    
    // Invalid max retries
    config.lock_timeout = std::chrono::seconds(10);
    config.max_retries = 0;
    EXPECT_FALSE(config.validate());
}

TEST_F(DataConsistencyTest, ValidationConfigValidation) {
    validation_config config;
    
    // Valid config
    config.validation_interval = std::chrono::seconds(60);
    config.max_validation_failures = 5;
    config.corruption_threshold = 0.1;
    EXPECT_TRUE(config.validate());
    
    // Invalid validation interval
    config.validation_interval = std::chrono::milliseconds(0);
    EXPECT_FALSE(config.validate());
    
    // Invalid max failures
    config.validation_interval = std::chrono::seconds(60);
    config.max_validation_failures = 0;
    EXPECT_FALSE(config.validate());
    
    // Invalid corruption threshold
    config.max_validation_failures = 5;
    config.corruption_threshold = -0.1;
    EXPECT_FALSE(config.validate());
    
    config.corruption_threshold = 1.1;
    EXPECT_FALSE(config.validate());
}

// Concurrency Tests
TEST_F(DataConsistencyTest, ConcurrentTransactions) {
    transaction_config config;
    transaction_manager manager("concurrent_manager", config);
    
    const int num_threads = 5;
    const int transactions_per_thread = 10;
    std::atomic<int> successful_transactions{0};
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < transactions_per_thread; ++j) {
                std::string tx_id = "tx_" + std::to_string(i) + "_" + std::to_string(j);
                
                auto begin_result = manager.begin_transaction(tx_id);
                if (begin_result) {
                    auto tx = begin_result.value();
                    
                    auto op = std::make_unique<transaction_operation>(
                        "op", [this]() { return test_operation(); });
                    tx->add_operation(std::move(op));
                    
                    auto commit_result = manager.commit_transaction(tx_id);
                    if (commit_result) {
                        successful_transactions++;
                    }
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_transactions.load(), num_threads * transactions_per_thread);
    
    auto metrics = manager.get_metrics();
    EXPECT_EQ(metrics.total_transactions.load(), num_threads * transactions_per_thread);
    EXPECT_EQ(metrics.committed_transactions.load(), num_threads * transactions_per_thread);
}

// Factory Function Tests
TEST_F(DataConsistencyTest, FactoryFunctions) {
    auto tx_manager = create_transaction_manager("factory_tx_manager");
    EXPECT_NE(tx_manager, nullptr);
    EXPECT_EQ(tx_manager->get_name(), "factory_tx_manager");
    
    auto validator = create_state_validator("factory_validator");
    EXPECT_NE(validator, nullptr);
    EXPECT_EQ(validator->get_name(), "factory_validator");
    
    auto consistency_manager = create_data_consistency_manager("factory_consistency");
    EXPECT_NE(consistency_manager, nullptr);
}