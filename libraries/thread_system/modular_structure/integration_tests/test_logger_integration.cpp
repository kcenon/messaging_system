#include <gtest/gtest.h>
#include <thread_system_core/interfaces/logger_interface.h>
#include <thread_system_core/interfaces/thread_context.h>
#include <thread_system_core/thread_pool/core/thread_pool.h>
#include <thread_system_core/thread_base/jobs/callback_job.h>
#include <sstream>
#include <mutex>
#include <vector>

using namespace thread_pool_module;
using namespace thread_module;

// Custom logger implementation for testing
class TestLogger : public logger_interface {
public:
    TestLogger() = default;

    void log(log_level level, const std::string& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.push_back({level, message});
    }

    void log_with_caller(
        log_level level,
        const std::string& message,
        const std::string& caller_function,
        const std::string& caller_file,
        int caller_line) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::stringstream ss;
        ss << "[" << caller_file << ":" << caller_line << " " << caller_function << "] " << message;
        logs_.push_back({level, ss.str()});
    }

    std::optional<std::string> get_identifier() const override {
        return "TestLogger";
    }

    std::string to_string() const override {
        return "TestLogger";
    }

    // Test helpers
    size_t log_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return logs_.size();
    }

    bool has_log_with_level(log_level level) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::any_of(logs_.begin(), logs_.end(),
            [level](const auto& log) { return log.first == level; });
    }

    bool has_log_containing(const std::string& text) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::any_of(logs_.begin(), logs_.end(),
            [&text](const auto& log) { 
                return log.second.find(text) != std::string::npos; 
            });
    }

private:
    mutable std::mutex mutex_;
    std::vector<std::pair<log_level, std::string>> logs_;
};

class LoggerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<TestLogger>();
        context_ = thread_context(logger_);
    }

    std::shared_ptr<TestLogger> logger_;
    thread_context context_;
};

TEST_F(LoggerIntegrationTest, ThreadPoolWithLogger) {
    auto pool = std::make_shared<thread_pool>("logged_pool", context_);
    
    EXPECT_FALSE(pool->start().has_value());
    
    // The pool should log when starting
    EXPECT_GT(logger_->log_count(), 0);
    EXPECT_TRUE(logger_->has_log_containing("logged_pool"));
    
    pool->stop();
}

TEST_F(LoggerIntegrationTest, JobExecutionLogging) {
    auto pool = std::make_shared<thread_pool>("test_pool", context_);
    pool->start();
    
    auto initial_count = logger_->log_count();
    
    auto job = std::make_unique<callback_job>(
        []() { 
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        },
        "test_job"
    );
    
    pool->enqueue(std::move(job));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should have logged job execution
    EXPECT_GT(logger_->log_count(), initial_count);
    EXPECT_TRUE(logger_->has_log_containing("test_job"));
}

TEST_F(LoggerIntegrationTest, ErrorLogging) {
    auto pool = std::make_shared<thread_pool>("error_pool", context_);
    pool->start();
    
    auto job = std::make_unique<callback_job>(
        []() { 
            throw std::runtime_error("Test error");
        },
        "error_job"
    );
    
    pool->enqueue(std::move(job));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should have logged the error
    EXPECT_TRUE(logger_->has_log_with_level(log_level::error));
    EXPECT_TRUE(logger_->has_log_containing("error"));
}

TEST_F(LoggerIntegrationTest, MultiplePoolsSharedLogger) {
    auto pool1 = std::make_shared<thread_pool>("pool1", context_);
    auto pool2 = std::make_shared<thread_pool>("pool2", context_);
    
    pool1->start();
    pool2->start();
    
    // Both pools should log
    EXPECT_TRUE(logger_->has_log_containing("pool1"));
    EXPECT_TRUE(logger_->has_log_containing("pool2"));
    
    // Submit jobs to both pools
    for (int i = 0; i < 10; ++i) {
        pool1->enqueue(std::make_unique<callback_job>(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); },
            "pool1_job_" + std::to_string(i)
        ));
        
        pool2->enqueue(std::make_unique<callback_job>(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); },
            "pool2_job_" + std::to_string(i)
        ));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Should have logs from both pools
    EXPECT_TRUE(logger_->has_log_containing("pool1_job"));
    EXPECT_TRUE(logger_->has_log_containing("pool2_job"));
}

TEST_F(LoggerIntegrationTest, ContextWithoutLogger) {
    thread_context empty_context;
    auto pool = std::make_shared<thread_pool>("no_logger_pool", empty_context);
    
    // Should work without logger
    EXPECT_FALSE(pool->start().has_value());
    
    auto job = std::make_unique<callback_job>(
        []() { /* do nothing */ },
        "silent_job"
    );
    
    EXPECT_FALSE(pool->enqueue(std::move(job)).has_value());
    pool->stop();
    
    // No logs should have been created
    EXPECT_EQ(logger_->log_count(), 0);
}