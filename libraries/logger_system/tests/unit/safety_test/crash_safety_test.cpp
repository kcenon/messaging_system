/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "../../sources/interfaces/logger_crash_safety.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace logger_module;

class CrashSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_log_path_ = "./test_emergency.log";
        cleanup_test_files();
    }
    
    void TearDown() override {
        cleanup_test_files();
    }
    
    void cleanup_test_files() {
        if (std::filesystem::exists(test_log_path_)) {
            std::filesystem::remove(test_log_path_);
        }
        if (std::filesystem::exists(test_log_path_ + ".recovery")) {
            std::filesystem::remove(test_log_path_ + ".recovery");
        }
        if (std::filesystem::exists(test_log_path_ + ".recovered")) {
            std::filesystem::remove(test_log_path_ + ".recovered");
        }
    }
    
    std::string test_log_path_;
};

TEST_F(CrashSafetyTest, SingletonInstance) {
    auto& instance1 = logger_crash_safety::instance();
    auto& instance2 = logger_crash_safety::instance();
    
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(CrashSafetyTest, Initialization) {
    auto& crash_safety = logger_crash_safety::instance();
    
    crash_safety.initialize(logger_crash_safety_level::standard, 
                           test_log_path_, 
                           1000);
    
    EXPECT_FALSE(crash_safety.is_handling_crash());
}

TEST_F(CrashSafetyTest, LoggerRegistration) {
    auto& crash_safety = logger_crash_safety::instance();
    
    bool flush_called = false;
    bool backup_called = false;
    
    crash_safety.register_logger("test_logger",
        [&flush_called]() { flush_called = true; },
        [&backup_called](const std::string&) { backup_called = true; }
    );
    
    crash_safety.force_flush_all();
    EXPECT_TRUE(flush_called);
    
    crash_safety.force_backup_all();
    EXPECT_TRUE(backup_called);
    
    crash_safety.unregister_logger("test_logger");
    
    flush_called = false;
    crash_safety.force_flush_all();
    EXPECT_FALSE(flush_called);
}

TEST_F(CrashSafetyTest, EmergencyLogging) {
    auto& crash_safety = logger_crash_safety::instance();
    
    crash_safety.set_emergency_log_path(test_log_path_);
    crash_safety.emergency_log("ERROR", "Test emergency message");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(std::filesystem::exists(test_log_path_));
    
    std::ifstream log_file(test_log_path_);
    std::string line;
    bool found = false;
    while (std::getline(log_file, line)) {
        if (line.find("Test emergency message") != std::string::npos) {
            found = true;
            break;
        }
    }
    
    EXPECT_TRUE(found);
}

TEST_F(CrashSafetyTest, MaxEmergencyEntries) {
    auto& crash_safety = logger_crash_safety::instance();
    
    crash_safety.set_max_emergency_entries(5);
    
    for (int i = 0; i < 10; ++i) {
        crash_safety.emergency_log("INFO", "Message " + std::to_string(i));
    }
    
    auto stats = crash_safety.get_stats();
    EXPECT_GE(stats.total_emergency_logs, 10);
}

TEST_F(CrashSafetyTest, AutoBackup) {
    auto& crash_safety = logger_crash_safety::instance();
    
    int backup_count = 0;
    crash_safety.register_logger("auto_backup_test",
        []() {},
        [&backup_count](const std::string&) { backup_count++; }
    );
    
    crash_safety.set_auto_backup(true, 100);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    crash_safety.set_auto_backup(false);
    
    EXPECT_GE(backup_count, 0);
    
    crash_safety.unregister_logger("auto_backup_test");
}

TEST_F(CrashSafetyTest, RecoveryDetection) {
    auto& crash_safety = logger_crash_safety::instance();
    
    std::ofstream recovery_marker(test_log_path_ + ".recovery");
    recovery_marker << "Recovery marker\n";
    recovery_marker.close();
    
    bool recovered = crash_safety.check_and_recover();
    EXPECT_TRUE(recovered);
    
    EXPECT_FALSE(std::filesystem::exists(test_log_path_ + ".recovery"));
}

TEST_F(CrashSafetyTest, Statistics) {
    auto& crash_safety = logger_crash_safety::instance();
    
    crash_safety.register_logger("stats_test",
        []() {},
        [](const std::string&) {}
    );
    
    crash_safety.force_flush_all();
    crash_safety.force_backup_all();
    
    auto stats = crash_safety.get_stats();
    EXPECT_GE(stats.successful_flushes, 1);
    EXPECT_GE(stats.backup_count, 1);
    
    crash_safety.unregister_logger("stats_test");
}

TEST_F(CrashSafetyTest, ScopedProtection) {
    auto& crash_safety = logger_crash_safety::instance();
    
    bool flush_called = false;
    
    {
        scoped_logger_crash_protection protection(
            "scoped_test",
            [&flush_called]() { flush_called = true; }
        );
        
        crash_safety.force_flush_all();
        EXPECT_TRUE(flush_called);
    }
    
    flush_called = false;
    crash_safety.force_flush_all();
    EXPECT_FALSE(flush_called);
}

class LogFileRecoveryTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_file_ = "./test_log.txt";
        recovery_file_ = "./recovered_log.txt";
        backup_file_ = "./backup_log.txt";
        checksum_file_ = "./backup_log.txt.sha256";
        
        cleanup_files();
    }
    
    void TearDown() override {
        cleanup_files();
    }
    
    void cleanup_files() {
        for (const auto& file : {test_file_, recovery_file_, backup_file_, checksum_file_}) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }
    
    std::string test_file_;
    std::string recovery_file_;
    std::string backup_file_;
    std::string checksum_file_;
};

TEST_F(LogFileRecoveryTest, CorruptionDetection) {
    std::ofstream file(test_file_, std::ios::binary);
    file << "Line 1\n";
    file << "Line 2\n";
    file << "Incomplete line without newline";
    file.close();
    
    EXPECT_TRUE(log_file_recovery::is_corrupted(test_file_));
    
    std::ofstream good_file(test_file_);
    good_file << "Line 1\n";
    good_file << "Line 2\n";
    good_file.close();
    
    EXPECT_FALSE(log_file_recovery::is_corrupted(test_file_));
}

TEST_F(LogFileRecoveryTest, FileRecovery) {
    std::ofstream file(test_file_);
    file << "Good line 1\n";
    file << "Good line 2\n";
    file << "Corrupted line";
    file.close();
    
    bool recovered = log_file_recovery::recover_file(test_file_, recovery_file_);
    EXPECT_TRUE(recovered);
    EXPECT_TRUE(std::filesystem::exists(recovery_file_));
    
    std::ifstream recovered_file(recovery_file_);
    std::string line;
    int line_count = 0;
    while (std::getline(recovered_file, line)) {
        line_count++;
    }
    EXPECT_EQ(line_count, 3);
}

TEST_F(LogFileRecoveryTest, BackupWithChecksum) {
    std::ofstream file(test_file_);
    file << "Test content\n";
    file << "More content\n";
    file.close();
    
    bool created = log_file_recovery::create_backup_with_checksum(test_file_, backup_file_);
    EXPECT_TRUE(created);
    EXPECT_TRUE(std::filesystem::exists(backup_file_));
    EXPECT_TRUE(std::filesystem::exists(checksum_file_));
    
    bool verified = log_file_recovery::verify_integrity(backup_file_, checksum_file_);
    EXPECT_TRUE(verified);
    
    std::ofstream corrupt(backup_file_, std::ios::app);
    corrupt << "Corrupted data\n";
    corrupt.close();
    
    verified = log_file_recovery::verify_integrity(backup_file_, checksum_file_);
    EXPECT_FALSE(verified);
}

TEST_F(LogFileRecoveryTest, NonExistentFile) {
    EXPECT_FALSE(log_file_recovery::is_corrupted("non_existent_file.txt"));
    EXPECT_FALSE(log_file_recovery::recover_file("non_existent_file.txt", recovery_file_));
}

class AsyncLoggerCrashSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

TEST_F(AsyncLoggerCrashSafetyTest, ConfigureAsyncSafety) {
    async_logger_crash_safety::configure_async_safety("async_logger", 500, true);
}

TEST_F(AsyncLoggerCrashSafetyTest, OverflowHandler) {
    bool overflow_called = false;
    size_t dropped_count = 0;
    
    async_logger_crash_safety::set_overflow_handler("async_logger",
        [&overflow_called, &dropped_count](size_t dropped) {
            overflow_called = true;
            dropped_count = dropped;
        }
    );
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}