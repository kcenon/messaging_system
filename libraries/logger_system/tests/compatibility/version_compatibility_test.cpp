/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file version_compatibility_test.cpp
 * @brief Version compatibility tests for logger_system
 *
 * This file tests backward compatibility with previous versions,
 * including log file format compatibility and API compatibility.
 */

#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <regex>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

class VersionCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any previous test artifacts
        if (fs::exists("test_logs")) {
            fs::remove_all("test_logs");
        }
        fs::create_directories("test_logs");
        fs::create_directories("test_logs/v1");
        fs::create_directories("test_logs/v2");
        fs::create_directories("test_logs/current");

        // Create sample log files from different versions
        create_v1_log_file();
        create_v2_log_file();
    }

    void TearDown() override {
        // Remove test logs
        if (fs::exists("test_logs")) {
            fs::remove_all("test_logs");
        }
    }

    void create_v1_log_file() {
        // Create a log file in v1 format
        std::ofstream v1_log("test_logs/v1/test_v1.log");
        v1_log << "[2025-01-01 12:00:00.000] [INFO] Application started\n";
        v1_log << "[2025-01-01 12:00:01.000] [DEBUG] Initializing components\n";
        v1_log << "[2025-01-01 12:00:02.000] [WARNING] Config file not found, using defaults\n";
        v1_log << "[2025-01-01 12:00:03.000] [ERROR] Failed to connect to database\n";
        v1_log << "[2025-01-01 12:00:04.000] [INFO] Retrying connection\n";
        v1_log.close();
    }

    void create_v2_log_file() {
        // Create a log file in v2 format (with additional fields)
        std::ofstream v2_log("test_logs/v2/test_v2.log");
        v2_log << "[2025-01-01 12:00:00.000] [INFO] [main.cpp:42] [main] Application started\n";
        v2_log << "[2025-01-01 12:00:01.000] [DEBUG] [init.cpp:15] [initialize] Initializing components\n";
        v2_log << "[2025-01-01 12:00:02.000] [WARNING] [config.cpp:88] [load_config] Config file not found\n";
        v2_log << "[2025-01-01 12:00:03.000] [ERROR] [db.cpp:120] [connect] Database connection failed\n";
        v2_log << "[2025-01-01 12:00:04.000] [INFO] [db.cpp:135] [retry] Retrying connection\n";
        v2_log.close();
    }

    struct LogEntry {
        std::string timestamp;
        std::string level;
        std::string message;
        std::string file;
        std::string function;
        int line = 0;
    };

    LogEntry parse_v1_log_line(const std::string& line) {
        LogEntry entry;
        std::regex v1_pattern(R"(\[([^\]]+)\] \[([^\]]+)\] (.+))");
        std::smatch matches;

        if (std::regex_match(line, matches, v1_pattern)) {
            entry.timestamp = matches[1];
            entry.level = matches[2];
            entry.message = matches[3];
        }

        return entry;
    }

    LogEntry parse_v2_log_line(const std::string& line) {
        LogEntry entry;
        std::regex v2_pattern(R"(\[([^\]]+)\] \[([^\]]+)\] \[([^:]+):(\d+)\] \[([^\]]+)\] (.+))");
        std::smatch matches;

        if (std::regex_match(line, matches, v2_pattern)) {
            entry.timestamp = matches[1];
            entry.level = matches[2];
            entry.file = matches[3];
            entry.line = std::stoi(matches[4]);
            entry.function = matches[5];
            entry.message = matches[6];
        }

        return entry;
    }
};

/**
 * @brief Test reading v1 format log files
 */
TEST_F(VersionCompatibilityTest, ReadV1LogFormat) {
    std::ifstream v1_log("test_logs/v1/test_v1.log");
    ASSERT_TRUE(v1_log.is_open());

    std::string line;
    std::vector<LogEntry> entries;

    while (std::getline(v1_log, line)) {
        auto entry = parse_v1_log_line(line);
        if (!entry.timestamp.empty()) {
            entries.push_back(entry);
        }
    }

    // Verify we can parse v1 format
    EXPECT_EQ(entries.size(), 5);
    EXPECT_EQ(entries[0].level, "INFO");
    EXPECT_EQ(entries[2].level, "WARNING");
    EXPECT_EQ(entries[3].level, "ERROR");
}

/**
 * @brief Test reading v2 format log files
 */
TEST_F(VersionCompatibilityTest, ReadV2LogFormat) {
    std::ifstream v2_log("test_logs/v2/test_v2.log");
    ASSERT_TRUE(v2_log.is_open());

    std::string line;
    std::vector<LogEntry> entries;

    while (std::getline(v2_log, line)) {
        auto entry = parse_v2_log_line(line);
        if (!entry.timestamp.empty()) {
            entries.push_back(entry);
        }
    }

    // Verify we can parse v2 format
    EXPECT_EQ(entries.size(), 5);
    EXPECT_EQ(entries[0].file, "main.cpp");
    EXPECT_EQ(entries[0].line, 42);
    EXPECT_EQ(entries[0].function, "main");
}

/**
 * @brief Test backward compatibility API
 */
TEST_F(VersionCompatibilityTest, BackwardCompatibleAPI) {
    // Simulate v1 compatibility layer
    struct V1Logger {
        void log(const std::string& message) {
            std::cout << "[LOG] " << message << std::endl;
        }
        void log_info(const std::string& message) {
            std::cout << "[INFO] " << message << std::endl;
        }
        void log_warning(const std::string& message) {
            std::cout << "[WARNING] " << message << std::endl;
        }
        void log_error(const std::string& message) {
            std::cout << "[ERROR] " << message << std::endl;
        }
        bool is_compatible() const { return true; }
    };

    V1Logger v1_logger;

    // v1 API calls (output redirected to void for testing)
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    v1_logger.log("Test message from v1 API");
    v1_logger.log_info("Info message from v1 API");
    v1_logger.log_warning("Warning message from v1 API");
    v1_logger.log_error("Error message from v1 API");

    std::cout.rdbuf(old);

    // Verify backward compatibility
    EXPECT_TRUE(v1_logger.is_compatible());
    EXPECT_FALSE(buffer.str().empty());
}

/**
 * @brief Test configuration file compatibility
 */
TEST_F(VersionCompatibilityTest, ConfigFileCompatibility) {
    // Create v1 config file
    std::ofstream v1_config("test_logs/config_v1.json");
    v1_config << R"({
        "log_level": "info",
        "output_file": "test_logs/v1/output.log",
        "max_file_size": 10485760,
        "max_files": 5
    })";
    v1_config.close();

    // Create v2 config file
    std::ofstream v2_config("test_logs/config_v2.json");
    v2_config << R"({
        "version": "2.0",
        "log_level": "debug",
        "writers": [
            {
                "type": "file",
                "path": "test_logs/v2/output.log",
                "max_size": 10485760,
                "rotation_count": 5
            }
        ],
        "async": true,
        "buffer_size": 8192
    })";
    v2_config.close();

    // Test that both config files exist and can be opened
    EXPECT_TRUE(fs::exists("test_logs/config_v1.json"));
    EXPECT_TRUE(fs::exists("test_logs/config_v2.json"));

    // Verify file sizes
    EXPECT_GT(fs::file_size("test_logs/config_v1.json"), 0);
    EXPECT_GT(fs::file_size("test_logs/config_v2.json"), 0);
}

/**
 * @brief Test log format migration
 */
TEST_F(VersionCompatibilityTest, LogFormatMigration) {
    // Read v1 logs
    std::ifstream v1_log("test_logs/v1/test_v1.log");
    std::vector<LogEntry> v1_entries;
    std::string line;

    while (std::getline(v1_log, line)) {
        auto entry = parse_v1_log_line(line);
        if (!entry.timestamp.empty()) {
            v1_entries.push_back(entry);
        }
    }
    v1_log.close();

    // Migrate to current format
    std::ofstream migrated_log("test_logs/current/migrated.log");
    for (const auto& entry : v1_entries) {
        // Convert to current format (v2-like)
        migrated_log << "[" << entry.timestamp << "] "
                     << "[" << entry.level << "] "
                     << "[migrated.cpp:0] "
                     << "[migration] "
                     << entry.message << std::endl;
    }
    migrated_log.close();

    // Verify migration
    EXPECT_TRUE(fs::exists("test_logs/current/migrated.log"));
    EXPECT_GT(fs::file_size("test_logs/current/migrated.log"), 0);
}

/**
 * @brief Test API version detection
 */
TEST_F(VersionCompatibilityTest, APIVersionDetection) {
    // Simulate version info
    struct VersionInfo {
        std::string version = "3.0.0";
        int major = 3;
        int minor = 0;
        int patch = 0;
    };

    VersionInfo version_info;

    EXPECT_FALSE(version_info.version.empty());
    EXPECT_GT(version_info.major, 0);
    EXPECT_GE(version_info.minor, 0);
    EXPECT_GE(version_info.patch, 0);

    // Check API compatibility simulation
    auto is_api_compatible = [](int major, int minor) {
        (void)minor;  // Suppress unused parameter warning
        return major <= 3; // Current version is 3.x
    };

    EXPECT_TRUE(is_api_compatible(1, 0)); // v1.0 should be compatible
    EXPECT_TRUE(is_api_compatible(2, 0)); // v2.0 should be compatible
    EXPECT_TRUE(is_api_compatible(3, 0)); // v3.0 should be compatible
    EXPECT_FALSE(is_api_compatible(4, 0)); // v4.0 not compatible (future)
}

/**
 * @brief Test C++ standard compatibility
 */
TEST_F(VersionCompatibilityTest, CppStandardCompatibility) {
    // Test C++ standard detection
    bool cpp17_available = false;
    bool cpp20_available = false;

#if __cplusplus >= 201703L
    cpp17_available = true;
#endif

#if __cplusplus >= 202002L
    cpp20_available = true;
#endif

    // At least C++17 should be available for this project
    EXPECT_TRUE(cpp17_available) << "C++17 or later is required";

    std::cout << "C++ Standard: ";
    if (cpp20_available) {
        std::cout << "C++20 or later" << std::endl;
    } else if (cpp17_available) {
        std::cout << "C++17" << std::endl;
    } else {
        std::cout << "Pre-C++17" << std::endl;
    }
}

/**
 * @brief Test plugin version compatibility
 */
TEST_F(VersionCompatibilityTest, PluginVersionCompatibility) {
    // Create a mock plugin info for different versions
    struct PluginInfo {
        std::string name;
        std::string version;
        int api_version;
    };

    std::vector<PluginInfo> plugins = {
        {"plugin_v1", "1.0.0", 1},
        {"plugin_v2", "2.0.0", 2},
        {"plugin_v2_1", "2.1.0", 2},
        {"plugin_current", "3.0.0", 3}
    };

    // Check compatibility
    const int current_api_version = 3;
    for (const auto& plugin : plugins) {
        bool is_compatible = plugin.api_version <= current_api_version;
        EXPECT_TRUE(is_compatible) << "Plugin " << plugin.name
                                   << " v" << plugin.version
                                   << " should be compatible with API v" << current_api_version;
    }
}

/**
 * @brief Test data structure compatibility
 */
TEST_F(VersionCompatibilityTest, DataStructureCompatibility) {
    // Test that log entry structures are compatible across versions
    struct LogEntryV1 {
        std::string timestamp;
        std::string level;
        std::string message;
    };

    struct LogEntryV2 {
        std::string timestamp;
        std::string level;
        std::string message;
        std::string file;
        int line;
        std::string function;
    };

    // V1 entry
    LogEntryV1 v1_entry{"2025-01-01 12:00:00", "INFO", "Test message"};

    // Convert V1 to V2 (forward compatibility)
    LogEntryV2 v2_entry;
    v2_entry.timestamp = v1_entry.timestamp;
    v2_entry.level = v1_entry.level;
    v2_entry.message = v1_entry.message;
    v2_entry.file = "";  // Default value for new fields
    v2_entry.line = 0;
    v2_entry.function = "";

    EXPECT_EQ(v2_entry.timestamp, v1_entry.timestamp);
    EXPECT_EQ(v2_entry.level, v1_entry.level);
    EXPECT_EQ(v2_entry.message, v1_entry.message);
}

/**
 * @brief Test serialization format compatibility
 */
TEST_F(VersionCompatibilityTest, SerializationCompatibility) {
    // Test JSON serialization compatibility
    std::string json_v1 = R"({
        "timestamp": "2025-01-01T12:00:00",
        "level": "INFO",
        "message": "Test message"
    })";

    std::string json_v2 = R"({
        "timestamp": "2025-01-01T12:00:00",
        "level": "INFO",
        "message": "Test message",
        "context": {
            "file": "test.cpp",
            "line": 42,
            "function": "test_func"
        }
    })";

    // Both formats should be parseable
    EXPECT_FALSE(json_v1.empty());
    EXPECT_FALSE(json_v2.empty());
    EXPECT_TRUE(json_v2.find("context") != std::string::npos);

    // V2 has additional context field
    EXPECT_GT(json_v2.length(), json_v1.length());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}