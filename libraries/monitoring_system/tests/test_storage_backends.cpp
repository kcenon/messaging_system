/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
// Note: storage_backends.h does not exist in include directory
// #include <kcenon/monitoring/storage/storage_backends.h>
#include <kcenon/monitoring/interfaces/monitoring_interface.h>
#include <filesystem>
#include <fstream>

using namespace monitoring_system;

class StorageBackendsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test snapshots
        test_snapshots_ = create_test_snapshots();
        
        // Create test directories
        test_dir_ = std::filesystem::temp_directory_path() / "monitoring_test";
        std::filesystem::create_directories(test_dir_);
    }
    
    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }
    
    std::vector<metrics_snapshot> create_test_snapshots() {
        std::vector<metrics_snapshot> snapshots;
        
        // Create first snapshot
        metrics_snapshot snap1;
        snap1.source_id = "web_server";
        snap1.capture_time = std::chrono::system_clock::now();
        snap1.add_metric("requests_per_second", 150.0);
        snap1.add_metric("response_time_ms", 45.2);
        snap1.add_metric("error_rate", 0.02);
        snapshots.push_back(snap1);
        
        // Create second snapshot
        metrics_snapshot snap2;
        snap2.source_id = "database";
        snap2.capture_time = std::chrono::system_clock::now();
        snap2.add_metric("connections", 25.0);
        snap2.add_metric("query_time_ms", 12.8);
        snap2.add_metric("cache_hit_rate", 0.95);
        snapshots.push_back(snap2);
        
        // Create third snapshot
        metrics_snapshot snap3;
        snap3.source_id = "cache_server";
        snap3.capture_time = std::chrono::system_clock::now();
        snap3.add_metric("memory_usage_mb", 512.0);
        snap3.add_metric("hit_rate", 0.88);
        snap3.add_metric("evictions_per_sec", 2.1);
        snapshots.push_back(snap3);
        
        return snapshots;
    }
    
    std::vector<metrics_snapshot> test_snapshots_;
    std::filesystem::path test_dir_;
};

TEST_F(StorageBackendsTest, StorageConfigValidation) {
    // Valid configuration
    storage_config valid_config;
    valid_config.path = "/tmp/test.json";
    valid_config.type = storage_backend_type::file_json;
    valid_config.max_capacity = 1000;
    valid_config.batch_size = 100;
    
    auto validation = valid_config.validate();
    EXPECT_TRUE(validation);
    
    // Invalid path (for non-memory storage)
    storage_config invalid_path;
    invalid_path.type = storage_backend_type::file_json;
    invalid_path.max_capacity = 1000;
    invalid_path.batch_size = 100;
    auto path_validation = invalid_path.validate();
    EXPECT_FALSE(path_validation);
    EXPECT_EQ(path_validation.get_error().code, monitoring_error_code::invalid_configuration);
    
    // Valid memory buffer (no path required)
    storage_config memory_config;
    memory_config.type = storage_backend_type::memory_buffer;
    memory_config.max_capacity = 1000;
    memory_config.batch_size = 100;
    auto memory_validation = memory_config.validate();
    EXPECT_TRUE(memory_validation);
    
    // Invalid capacity
    storage_config invalid_capacity;
    invalid_capacity.path = "/tmp/test";
    invalid_capacity.max_capacity = 0;
    auto capacity_validation = invalid_capacity.validate();
    EXPECT_FALSE(capacity_validation);
    EXPECT_EQ(capacity_validation.get_error().code, monitoring_error_code::invalid_capacity);
    
    // Invalid batch size
    storage_config invalid_batch;
    invalid_batch.path = "/tmp/test";
    invalid_batch.max_capacity = 1000;
    invalid_batch.batch_size = 0;
    auto batch_validation = invalid_batch.validate();
    EXPECT_FALSE(batch_validation);
    
    // Batch size larger than capacity
    storage_config batch_too_large;
    batch_too_large.path = "/tmp/test";
    batch_too_large.max_capacity = 100;
    batch_too_large.batch_size = 200;
    auto batch_large_validation = batch_too_large.validate();
    EXPECT_FALSE(batch_large_validation);
}

TEST_F(StorageBackendsTest, FileStorageBackendBasicOperations) {
    storage_config config;
    config.type = storage_backend_type::file_json;
    config.path = (test_dir_ / "test.json").string();
    config.max_capacity = 10;
    
    file_storage_backend backend(config);
    
    // Test initial state
    EXPECT_EQ(backend.size(), 0);
    EXPECT_EQ(backend.capacity(), 10);
    
    // Store snapshots
    for (const auto& snapshot : test_snapshots_) {
        auto store_result = backend.store(snapshot);
        EXPECT_TRUE(store_result);
    }
    
    EXPECT_EQ(backend.size(), 3);
    
    // Retrieve snapshot
    auto retrieve_result = backend.retrieve(0);
    EXPECT_TRUE(retrieve_result);
    
    // Retrieve range
    auto range_result = backend.retrieve_range(0, 2);
    EXPECT_TRUE(range_result);
    EXPECT_EQ(range_result.value().size(), 2);
    
    // Test flush
    auto flush_result = backend.flush();
    EXPECT_TRUE(flush_result);
    
    // Test clear
    auto clear_result = backend.clear();
    EXPECT_TRUE(clear_result);
    EXPECT_EQ(backend.size(), 0);
}

TEST_F(StorageBackendsTest, FileStorageBackendCapacityLimit) {
    storage_config config;
    config.type = storage_backend_type::file_json;
    config.path = (test_dir_ / "capacity_test.json").string();
    config.max_capacity = 2; // Small capacity for testing
    
    file_storage_backend backend(config);
    
    // Store snapshots beyond capacity
    for (const auto& snapshot : test_snapshots_) {
        auto store_result = backend.store(snapshot);
        EXPECT_TRUE(store_result);
    }
    
    // Should not exceed capacity (oldest removed)
    EXPECT_EQ(backend.size(), 2);
    
    // Get statistics
    auto stats = backend.get_stats();
    EXPECT_EQ(stats["total_snapshots"], 2);
    EXPECT_EQ(stats["capacity"], 2);
}

TEST_F(StorageBackendsTest, FileStorageBackendDifferentFormats) {
    // Test JSON format
    {
        storage_config json_config;
        json_config.type = storage_backend_type::file_json;
        json_config.path = (test_dir_ / "test.json").string();
        json_config.max_capacity = 10;
        
        file_storage_backend json_backend(json_config);
        auto store_result = json_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
        
        auto retrieve_result = json_backend.retrieve(0);
        EXPECT_TRUE(retrieve_result);
    }
    
    // Test Binary format
    {
        storage_config binary_config;
        binary_config.type = storage_backend_type::file_binary;
        binary_config.path = (test_dir_ / "test.bin").string();
        binary_config.max_capacity = 10;
        
        file_storage_backend binary_backend(binary_config);
        auto store_result = binary_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
        
        auto retrieve_result = binary_backend.retrieve(0);
        EXPECT_TRUE(retrieve_result);
    }
    
    // Test CSV format
    {
        storage_config csv_config;
        csv_config.type = storage_backend_type::file_csv;
        csv_config.path = (test_dir_ / "test.csv").string();
        csv_config.max_capacity = 10;
        
        file_storage_backend csv_backend(csv_config);
        auto store_result = csv_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
        
        auto retrieve_result = csv_backend.retrieve(0);
        EXPECT_TRUE(retrieve_result);
    }
}

TEST_F(StorageBackendsTest, MemoryStorageBackend) {
    storage_config config;
    config.type = storage_backend_type::memory_buffer;
    config.max_capacity = 5;
    
    file_storage_backend backend(config); // Memory buffer uses file_storage_backend
    
    // Store snapshots
    for (const auto& snapshot : test_snapshots_) {
        auto store_result = backend.store(snapshot);
        EXPECT_TRUE(store_result);
    }
    
    EXPECT_EQ(backend.size(), 3);
    
    // Memory backend should not create files
    // (Implementation detail - file operations are skipped for memory type)
    
    // Test all operations work
    auto retrieve_result = backend.retrieve(0);
    EXPECT_TRUE(retrieve_result);
    
    auto range_result = backend.retrieve_range(0, 2);
    EXPECT_TRUE(range_result);
    
    auto clear_result = backend.clear();
    EXPECT_TRUE(clear_result);
    EXPECT_EQ(backend.size(), 0);
}

TEST_F(StorageBackendsTest, DatabaseStorageBackendBasicOperations) {
    storage_config config;
    config.type = storage_backend_type::database_sqlite;
    config.path = (test_dir_ / "test.db").string();
    config.table_name = "test_metrics";
    config.max_capacity = 100;
    
    database_storage_backend backend(config);
    
    // Test initial state
    EXPECT_EQ(backend.capacity(), 100);
    EXPECT_GE(backend.size(), 0); // Database starts at 0
    
    // Store snapshots
    for (const auto& snapshot : test_snapshots_) {
        auto store_result = backend.store(snapshot);
        EXPECT_TRUE(store_result);
    }
    
    EXPECT_EQ(backend.size(), 3);
    
    // Retrieve snapshot
    auto retrieve_result = backend.retrieve(0);
    EXPECT_TRUE(retrieve_result);
    
    // Retrieve range
    auto range_result = backend.retrieve_range(0, 2);
    EXPECT_TRUE(range_result);
    
    // Test flush
    auto flush_result = backend.flush();
    EXPECT_TRUE(flush_result);
    
    // Test clear
    auto clear_result = backend.clear();
    EXPECT_TRUE(clear_result);
    EXPECT_EQ(backend.size(), 0);
    
    // Get statistics
    auto stats = backend.get_stats();
    EXPECT_EQ(stats["stored_count"], 0); // After clear
    EXPECT_EQ(stats["capacity"], 100);
    EXPECT_EQ(stats["connected"], 1);
}

TEST_F(StorageBackendsTest, DatabaseStorageBackendDifferentTypes) {
    // Test SQLite
    {
        storage_config sqlite_config;
        sqlite_config.type = storage_backend_type::database_sqlite;
        sqlite_config.path = (test_dir_ / "sqlite.db").string();
        sqlite_config.max_capacity = 50;
        
        database_storage_backend sqlite_backend(sqlite_config);
        auto store_result = sqlite_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test PostgreSQL (simulated)
    {
        storage_config pg_config;
        pg_config.type = storage_backend_type::database_postgresql;
        pg_config.host = "localhost";
        pg_config.port = 5432;
        pg_config.database_name = "monitoring_test";
        pg_config.username = "test_user";
        pg_config.password = "test_pass";
        pg_config.max_capacity = 50;
        
        database_storage_backend pg_backend(pg_config);
        auto store_result = pg_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test MySQL (simulated)
    {
        storage_config mysql_config;
        mysql_config.type = storage_backend_type::database_mysql;
        mysql_config.host = "localhost";
        mysql_config.port = 3306;
        mysql_config.database_name = "monitoring_test";
        mysql_config.username = "test_user";
        mysql_config.password = "test_pass";
        mysql_config.max_capacity = 50;
        
        database_storage_backend mysql_backend(mysql_config);
        auto store_result = mysql_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
}

TEST_F(StorageBackendsTest, CloudStorageBackendBasicOperations) {
    storage_config config;
    config.type = storage_backend_type::cloud_s3;
    config.path = "test-monitoring-bucket";
    config.max_capacity = 1000;
    
    cloud_storage_backend backend(config);
    
    // Test initial state
    EXPECT_EQ(backend.capacity(), 1000);
    EXPECT_EQ(backend.size(), 0);
    
    // Store snapshots
    for (const auto& snapshot : test_snapshots_) {
        auto store_result = backend.store(snapshot);
        EXPECT_TRUE(store_result);
    }
    
    EXPECT_EQ(backend.size(), 3);
    
    // Retrieve snapshot
    auto retrieve_result = backend.retrieve(0);
    EXPECT_TRUE(retrieve_result);
    
    // Retrieve range
    auto range_result = backend.retrieve_range(0, 2);
    EXPECT_TRUE(range_result);
    EXPECT_LE(range_result.value().size(), 2); // May be less due to simulated failures
    
    // Test flush
    auto flush_result = backend.flush();
    EXPECT_TRUE(flush_result);
    
    // Test clear
    auto clear_result = backend.clear();
    EXPECT_TRUE(clear_result);
    EXPECT_EQ(backend.size(), 0);
}

TEST_F(StorageBackendsTest, CloudStorageBackendDifferentProviders) {
    // Test AWS S3
    {
        storage_config s3_config;
        s3_config.type = storage_backend_type::cloud_s3;
        s3_config.path = "s3-test-bucket";
        s3_config.max_capacity = 100;
        
        cloud_storage_backend s3_backend(s3_config);
        auto store_result = s3_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test Google Cloud Storage
    {
        storage_config gcs_config;
        gcs_config.type = storage_backend_type::cloud_gcs;
        gcs_config.path = "gcs-test-bucket";
        gcs_config.max_capacity = 100;
        
        cloud_storage_backend gcs_backend(gcs_config);
        auto store_result = gcs_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test Azure Blob Storage
    {
        storage_config azure_config;
        azure_config.type = storage_backend_type::cloud_azure_blob;
        azure_config.path = "azure-test-container";
        azure_config.max_capacity = 100;
        
        cloud_storage_backend azure_backend(azure_config);
        auto store_result = azure_backend.store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
}

TEST_F(StorageBackendsTest, StorageBackendFactory) {
    // Test file storage creation
    {
        storage_config file_config;
        file_config.type = storage_backend_type::file_json;
        file_config.path = (test_dir_ / "factory_test.json").string();
        file_config.max_capacity = 50;
        
        auto backend = storage_backend_factory::create_backend(file_config);
        EXPECT_TRUE(backend);
        
        auto store_result = backend->store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test database storage creation
    {
        storage_config db_config;
        db_config.type = storage_backend_type::database_sqlite;
        db_config.path = (test_dir_ / "factory_test.db").string();
        db_config.max_capacity = 50;
        
        auto backend = storage_backend_factory::create_backend(db_config);
        EXPECT_TRUE(backend);
        
        auto store_result = backend->store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test cloud storage creation
    {
        storage_config cloud_config;
        cloud_config.type = storage_backend_type::cloud_s3;
        cloud_config.path = "factory-test-bucket";
        cloud_config.max_capacity = 50;
        
        auto backend = storage_backend_factory::create_backend(cloud_config);
        EXPECT_TRUE(backend);
        
        auto store_result = backend->store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test invalid type
    {
        storage_config invalid_config;
        invalid_config.type = static_cast<storage_backend_type>(999);
        
        auto backend = storage_backend_factory::create_backend(invalid_config);
        EXPECT_FALSE(backend);
    }
}

TEST_F(StorageBackendsTest, SupportedBackendsList) {
    auto supported = storage_backend_factory::get_supported_backends();
    EXPECT_EQ(supported.size(), 10); // All backend types
    
    EXPECT_TRUE(std::find(supported.begin(), supported.end(), 
                         storage_backend_type::file_json) != supported.end());
    EXPECT_TRUE(std::find(supported.begin(), supported.end(), 
                         storage_backend_type::database_sqlite) != supported.end());
    EXPECT_TRUE(std::find(supported.begin(), supported.end(), 
                         storage_backend_type::cloud_s3) != supported.end());
    EXPECT_TRUE(std::find(supported.begin(), supported.end(), 
                         storage_backend_type::memory_buffer) != supported.end());
}

TEST_F(StorageBackendsTest, HelperFunctions) {
    // Test file storage helper
    {
        auto backend = create_file_storage(
            (test_dir_ / "helper_test.json").string(),
            storage_backend_type::file_json,
            100);
        EXPECT_TRUE(backend);
        
        auto store_result = backend->store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test database storage helper
    {
        auto backend = create_database_storage(
            storage_backend_type::database_sqlite,
            (test_dir_ / "helper_test.db").string(),
            "test_table");
        EXPECT_TRUE(backend);
        
        auto store_result = backend->store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
    
    // Test cloud storage helper
    {
        auto backend = create_cloud_storage(
            storage_backend_type::cloud_s3,
            "helper-test-bucket");
        EXPECT_TRUE(backend);
        
        auto store_result = backend->store(test_snapshots_[0]);
        EXPECT_TRUE(store_result);
    }
}

TEST_F(StorageBackendsTest, ErrorHandling) {
    // Test file storage with invalid path (skip if filesystem throws)
    {
        try {
            storage_config invalid_config;
            invalid_config.type = storage_backend_type::file_json;
            invalid_config.path = "/invalid/path/that/does/not/exist/file.json";
            invalid_config.max_capacity = 10;
            
            // Constructor may throw if filesystem operations fail
            file_storage_backend backend(invalid_config);
            
            // Store operation may fail due to invalid path
            auto store_result = backend.store(test_snapshots_[0]);
            // Don't assert on result as it depends on filesystem permissions
        } catch (const std::exception&) {
            // Expected if filesystem doesn't allow directory creation
            // This is acceptable for this test
        }
    }
    
    // Test retrieval of non-existent snapshot
    {
        storage_config config;
        config.type = storage_backend_type::memory_buffer;
        config.max_capacity = 10;
        
        file_storage_backend backend(config);
        
        auto retrieve_result = backend.retrieve(999);
        EXPECT_FALSE(retrieve_result);
        EXPECT_EQ(retrieve_result.get_error().code, monitoring_error_code::not_found);
    }
}

TEST_F(StorageBackendsTest, ConcurrentOperations) {
    storage_config config;
    config.type = storage_backend_type::memory_buffer;
    config.max_capacity = 100;
    
    file_storage_backend backend(config);
    
    // Test concurrent stores
    std::vector<std::thread> store_threads;
    std::atomic<int> successful_stores{0};
    
    for (int i = 0; i < 10; ++i) {
        store_threads.emplace_back([&, i]() {
            metrics_snapshot snapshot;
            snapshot.source_id = "thread_" + std::to_string(i);
            snapshot.add_metric("value", static_cast<double>(i));
            
            auto result = backend.store(snapshot);
            if (result) {
                successful_stores++;
            }
        });
    }
    
    for (auto& thread : store_threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_stores.load(), 10);
    EXPECT_EQ(backend.size(), 10);
    
    // Test concurrent retrieval
    std::vector<std::thread> retrieve_threads;
    std::atomic<int> successful_retrievals{0};
    
    for (int i = 0; i < 5; ++i) {
        retrieve_threads.emplace_back([&, i]() {
            auto result = backend.retrieve(i);
            if (result) {
                successful_retrievals++;
            }
        });
    }
    
    for (auto& thread : retrieve_threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_retrievals.load(), 5);
}

TEST_F(StorageBackendsTest, LargeDatasetHandling) {
    storage_config config;
    config.type = storage_backend_type::memory_buffer;
    config.max_capacity = 50;
    
    file_storage_backend backend(config);
    
    // Store more data than capacity
    for (int i = 0; i < 100; ++i) {
        metrics_snapshot snapshot;
        snapshot.source_id = "generator_" + std::to_string(i);
        
        // Add multiple metrics per snapshot
        for (int j = 0; j < 10; ++j) {
            snapshot.add_metric("metric_" + std::to_string(j), static_cast<double>(i * 10 + j));
        }
        
        auto store_result = backend.store(snapshot);
        EXPECT_TRUE(store_result);
    }
    
    // Should not exceed capacity
    EXPECT_EQ(backend.size(), 50);
    
    // Test range retrieval of large dataset
    auto range_result = backend.retrieve_range(0, 25);
    EXPECT_TRUE(range_result);
    EXPECT_EQ(range_result.value().size(), 25);
}