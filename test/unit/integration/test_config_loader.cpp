// Unit tests for ConfigLoader YAML parsing

#include "messaging_system/integration/config_loader.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>

#ifdef HAS_YAML_CPP

using namespace messaging;

// Test fixture: Create temporary test config file
class TestConfigFixture {
public:
    TestConfigFixture() : test_dir_("test_config_temp") {
        std::filesystem::create_directory(test_dir_);
    }

    ~TestConfigFixture() {
        std::filesystem::remove_all(test_dir_);
    }

    std::string create_test_config(const std::string& content) {
        std::string path = test_dir_ + "/test_config.yaml";
        std::ofstream file(path);
        file << content;
        file.close();
        return path;
    }

private:
    std::string test_dir_;
};

void test_load_valid_config() {
    std::cout << "Test: Load valid configuration..." << std::endl;

    TestConfigFixture fixture;

    std::string config_content = R"(
messaging_system:
  version: "2.0.0"
  network:
    port: 8080
    max_connections: 1000
    timeout_ms: 5000
    retry_attempts: 3
  thread_pools:
    io:
      workers: 4
      queue_size: 1000
    work:
      workers: 8
      queue_size: 2000
      lockfree: true
  database:
    type: "postgresql"
    connection_string: "postgresql://localhost:5432/msgdb"
    pool:
      min_connections: 5
      max_connections: 20
      idle_timeout_s: 60
  logging:
    level: "info"
    async: true
    writers:
      - "console"
      - "file"
  monitoring:
    enabled: true
    interval_ms: 1000
)";

    auto config_path = fixture.create_test_config(config_content);
    auto result = MessagingSystemConfig::load_from_file(config_path);

    assert(result.is_ok() && "Should load valid config");

    auto config = result.value();
    assert(config.version == "2.0.0" && "Version should match");
    assert(config.network.port == 8080 && "Port should match");
    assert(config.network.max_connections == 1000 && "Max connections should match");
    assert(config.network.timeout == std::chrono::milliseconds(5000) && "Timeout should match");
    assert(config.network.retry_attempts == 3 && "Retry attempts should match");

    assert(config.thread_pools.io_workers == 4 && "IO workers should match");
    assert(config.thread_pools.work_workers == 8 && "Work workers should match");
    assert(config.thread_pools.use_lockfree == true && "Lockfree should match");

    assert(config.database.type == "postgresql" && "Database type should match");
    assert(config.database.pool_config.min_connections == 5 && "Min connections should match");
    assert(config.database.pool_config.max_connections == 20 && "Max connections should match");

    assert(config.logging.level == "info" && "Log level should match");
    assert(config.logging.async == true && "Async logging should match");
    assert(config.logging.writers.size() == 2 && "Should have 2 writers");

    assert(config.monitoring.enabled == true && "Monitoring should be enabled");
    assert(config.monitoring.interval == std::chrono::milliseconds(1000) && "Monitoring interval should match");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_load_minimal_config() {
    std::cout << "Test: Load minimal configuration..." << std::endl;

    TestConfigFixture fixture;

    std::string config_content = R"(
messaging_system:
  version: "2.0.0"
  network:
    port: 9000
  thread_pools:
    io:
      workers: 2
    work:
      workers: 4
)";

    auto config_path = fixture.create_test_config(config_content);
    auto result = MessagingSystemConfig::load_from_file(config_path);

    assert(result.is_ok() && "Should load minimal config");

    auto config = result.value();
    assert(config.version == "2.0.0" && "Version should match");
    assert(config.network.port == 9000 && "Port should match");
    assert(config.thread_pools.io_workers == 2 && "IO workers should match");
    assert(config.thread_pools.work_workers == 4 && "Work workers should match");

    // Check defaults are maintained
    assert(config.network.max_connections == 10000 && "Should have default max connections");
    assert(config.logging.level == "info" && "Should have default log level");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_load_missing_root_node() {
    std::cout << "Test: Load config with missing root node..." << std::endl;

    TestConfigFixture fixture;

    std::string config_content = R"(
some_other_config:
  value: "test"
)";

    auto config_path = fixture.create_test_config(config_content);
    auto result = MessagingSystemConfig::load_from_file(config_path);

    assert(result.is_err() && "Should fail with missing root node");
    assert(result.error().code == messaging::error::INVALID_MESSAGE && "Error code should be INVALID_MESSAGE");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_load_nonexistent_file() {
    std::cout << "Test: Load nonexistent config file..." << std::endl;

    auto result = MessagingSystemConfig::load_from_file("/nonexistent/path/config.yaml");

    assert(result.is_err() && "Should fail with nonexistent file");
    assert(result.error().code == messaging::error::SERIALIZATION_ERROR && "Error code should be SERIALIZATION_ERROR");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_load_malformed_yaml() {
    std::cout << "Test: Load malformed YAML..." << std::endl;

    TestConfigFixture fixture;

    std::string config_content = R"(
messaging_system:
  network:
    port: [invalid
    - unclosed array
)";

    auto config_path = fixture.create_test_config(config_content);
    auto result = MessagingSystemConfig::load_from_file(config_path);

    assert(result.is_err() && "Should fail with malformed YAML");
    assert(result.error().code == messaging::error::SERIALIZATION_ERROR && "Error code should be SERIALIZATION_ERROR");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_validate_valid_config() {
    std::cout << "Test: Validate valid configuration..." << std::endl;

    TestConfigFixture fixture;

    std::string config_content = R"(
messaging_system:
  version: "2.0.0"
  network:
    port: 8080
  thread_pools:
    io:
      workers: 2
    work:
      workers: 4
)";

    auto config_path = fixture.create_test_config(config_content);
    auto load_result = MessagingSystemConfig::load_from_file(config_path);
    assert(load_result.is_ok() && "Should load config");

    auto config = load_result.value();
    auto validate_result = config.validate();

    assert(validate_result.is_ok() && "Should validate successfully");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_validate_invalid_port() {
    std::cout << "Test: Validate config with invalid port..." << std::endl;

    MessagingSystemConfig config;
    config.network.port = 0; // Invalid port
    config.thread_pools.io_workers = 2;
    config.thread_pools.work_workers = 4;

    auto result = config.validate();

    assert(result.is_err() && "Should fail validation with port 0");
    assert(result.error().code == messaging::error::INVALID_MESSAGE && "Error code should be INVALID_MESSAGE");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_validate_invalid_thread_pools() {
    std::cout << "Test: Validate config with invalid thread pools..." << std::endl;

    MessagingSystemConfig config;
    config.network.port = 8080;
    config.thread_pools.io_workers = 0; // Invalid
    config.thread_pools.work_workers = 4;

    auto result = config.validate();

    assert(result.is_err() && "Should fail validation with 0 workers");
    assert(result.error().code == messaging::error::INVALID_MESSAGE && "Error code should be INVALID_MESSAGE");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_validate_database_config() {
    std::cout << "Test: Validate database configuration..." << std::endl;

    MessagingSystemConfig config;
    config.network.port = 8080;
    config.thread_pools.io_workers = 2;
    config.thread_pools.work_workers = 4;
    config.database.type = "postgresql";
    config.database.connection_string = ""; // Invalid: type set but no connection string

    auto result = config.validate();

    assert(result.is_err() && "Should fail validation with missing connection string");
    assert(result.error().code == messaging::error::INVALID_MESSAGE && "Error code should be INVALID_MESSAGE");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_partial_config_with_defaults() {
    std::cout << "Test: Partial config uses defaults..." << std::endl;

    TestConfigFixture fixture;

    std::string config_content = R"(
messaging_system:
  network:
    port: 7000
  thread_pools:
    io:
      workers: 1
    work:
      workers: 2
)";

    auto config_path = fixture.create_test_config(config_content);
    auto result = MessagingSystemConfig::load_from_file(config_path);

    assert(result.is_ok() && "Should load partial config");

    auto config = result.value();

    // Verify specified values
    assert(config.network.port == 7000 && "Port should be specified value");
    assert(config.thread_pools.io_workers == 1 && "IO workers should be specified");
    assert(config.thread_pools.work_workers == 2 && "Work workers should be specified");

    // Verify defaults
    assert(config.version == "2.0.0" && "Should have default version");
    assert(config.network.max_connections == 10000 && "Should have default max connections");
    assert(config.network.timeout == std::chrono::milliseconds(30000) && "Should have default timeout");
    assert(config.network.retry_attempts == 3 && "Should have default retry attempts");

    std::cout << "  ✓ Passed" << std::endl;
}

int main() {
    std::cout << "=== ConfigLoader Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        test_load_valid_config();
        test_load_minimal_config();
        test_load_missing_root_node();
        test_load_nonexistent_file();
        test_load_malformed_yaml();
        test_validate_valid_config();
        test_validate_invalid_port();
        test_validate_invalid_thread_pools();
        test_validate_database_config();
        test_partial_config_with_defaults();

        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

#else

int main() {
    std::cerr << "ConfigLoader tests require HAS_YAML_CPP" << std::endl;
    std::cout << "Skipping tests (not an error)" << std::endl;
    return 0;
}

#endif
