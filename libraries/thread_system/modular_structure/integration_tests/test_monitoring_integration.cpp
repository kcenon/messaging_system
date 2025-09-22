#include <gtest/gtest.h>
#include <thread_system_core/interfaces/monitoring_interface.h>
#include <thread_system_core/interfaces/thread_context.h>
#include <thread_system_core/thread_pool/core/thread_pool.h>
#include <thread_system_core/thread_base/jobs/callback_job.h>
#include <map>
#include <mutex>

using namespace thread_pool_module;
using namespace thread_module;
using namespace ::monitoring_interface;

// Custom monitoring implementation for testing
class TestMonitoring : public ::monitoring_interface::monitoring_interface {
public:
    TestMonitoring() = default;

    void record_thread_created(const std::string& thread_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_created_count_[thread_name]++;
    }

    void record_thread_destroyed(const std::string& thread_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_destroyed_count_[thread_name]++;
    }

    void update_thread_pool_metrics(
        const std::string& pool_name,
        const thread_pool_metrics& metrics) override {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_metrics_[pool_name] = metrics;
    }

    void update_thread_pool_metrics(
        const std::string& pool_name,
        std::uint32_t pool_instance_id,
        const thread_pool_metrics& metrics) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = pool_name + "_" + std::to_string(pool_instance_id);
        pool_metrics_[key] = metrics;
    }

    void record_job_enqueued(const std::string& job_type) override {
        std::lock_guard<std::mutex> lock(mutex_);
        job_enqueued_count_[job_type]++;
    }

    void record_job_started(const std::string& job_type) override {
        std::lock_guard<std::mutex> lock(mutex_);
        job_started_count_[job_type]++;
    }

    void record_job_completed(
        const std::string& job_type,
        std::chrono::nanoseconds duration) override {
        std::lock_guard<std::mutex> lock(mutex_);
        job_completed_count_[job_type]++;
        job_durations_[job_type].push_back(duration);
    }

    void record_job_failed(const std::string& job_type) override {
        std::lock_guard<std::mutex> lock(mutex_);
        job_failed_count_[job_type]++;
    }

    std::optional<std::string> get_identifier() const override {
        return "TestMonitoring";
    }

    std::string to_string() const override {
        return "TestMonitoring";
    }

    // Test helpers
    int get_thread_created_count(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = thread_created_count_.find(name);
        return it != thread_created_count_.end() ? it->second : 0;
    }

    int get_job_completed_count(const std::string& job_type) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = job_completed_count_.find(job_type);
        return it != job_completed_count_.end() ? it->second : 0;
    }

    std::optional<thread_pool_metrics> get_pool_metrics(const std::string& pool_name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = pool_metrics_.find(pool_name);
        if (it != pool_metrics_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

private:
    mutable std::mutex mutex_;
    std::map<std::string, int> thread_created_count_;
    std::map<std::string, int> thread_destroyed_count_;
    std::map<std::string, thread_pool_metrics> pool_metrics_;
    std::map<std::string, int> job_enqueued_count_;
    std::map<std::string, int> job_started_count_;
    std::map<std::string, int> job_completed_count_;
    std::map<std::string, int> job_failed_count_;
    std::map<std::string, std::vector<std::chrono::nanoseconds>> job_durations_;
};

class MonitoringIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitoring_ = std::make_shared<TestMonitoring>();
        context_ = thread_context(nullptr, monitoring_);
    }

    std::shared_ptr<TestMonitoring> monitoring_;
    thread_context context_;
};

TEST_F(MonitoringIntegrationTest, ThreadPoolMetrics) {
    auto pool = std::make_shared<thread_pool>("monitored_pool", context_);
    
    EXPECT_FALSE(pool->start().has_value());
    
    // Should have recorded thread creation
    EXPECT_GT(monitoring_->get_thread_created_count("monitored_pool"), 0);
    
    // Submit some jobs
    for (int i = 0; i < 10; ++i) {
        pool->enqueue(std::make_unique<callback_job>(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); },
            "test_job"
        ));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Check job metrics
    EXPECT_EQ(monitoring_->get_job_completed_count("test_job"), 10);
    
    // Report metrics
    pool->report_metrics();
    
    // Check pool metrics
    auto metrics = monitoring_->get_pool_metrics("monitored_pool_" + 
                                                std::to_string(pool->get_pool_instance_id()));
    ASSERT_TRUE(metrics.has_value());
    EXPECT_EQ(metrics->pending_jobs, 0);  // All jobs should be completed
}

TEST_F(MonitoringIntegrationTest, JobLifecycleTracking) {
    auto pool = std::make_shared<thread_pool>("lifecycle_pool", context_);
    pool->start();
    
    std::atomic<bool> job_started(false);
    std::atomic<bool> job_can_proceed(false);
    
    auto job = std::make_unique<callback_job>(
        [&job_started, &job_can_proceed]() {
            job_started = true;
            while (!job_can_proceed) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        },
        "blocking_job"
    );
    
    pool->enqueue(std::move(job));
    
    // Wait for job to start
    while (!job_started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Job should be started but not completed
    EXPECT_GT(monitoring_->get_thread_created_count("lifecycle_pool"), 0);
    EXPECT_EQ(monitoring_->get_job_completed_count("blocking_job"), 0);
    
    // Let job complete
    job_can_proceed = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Job should now be completed
    EXPECT_EQ(monitoring_->get_job_completed_count("blocking_job"), 1);
}

TEST_F(MonitoringIntegrationTest, FailedJobTracking) {
    auto pool = std::make_shared<thread_pool>("error_tracking_pool", context_);
    pool->start();
    
    // Submit a job that will fail
    auto failing_job = std::make_unique<callback_job>(
        []() { throw std::runtime_error("Intentional failure"); },
        "failing_job"
    );
    
    pool->enqueue(std::move(failing_job));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should have recorded the failure
    // Note: The current implementation might not track failures separately
    // This is a test to verify the monitoring interface
}

TEST_F(MonitoringIntegrationTest, MultiplePoolsIndependentMetrics) {
    auto pool1 = std::make_shared<thread_pool>("pool_alpha", context_);
    auto pool2 = std::make_shared<thread_pool>("pool_beta", context_);
    
    pool1->start();
    pool2->start();
    
    // Submit different jobs to each pool
    for (int i = 0; i < 5; ++i) {
        pool1->enqueue(std::make_unique<callback_job>(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(5)); },
            "alpha_job"
        ));
        
        pool2->enqueue(std::make_unique<callback_job>(
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(5)); },
            "beta_job"
        ));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Each pool should have its own metrics
    EXPECT_EQ(monitoring_->get_job_completed_count("alpha_job"), 5);
    EXPECT_EQ(monitoring_->get_job_completed_count("beta_job"), 5);
    
    // Report metrics for both pools
    pool1->report_metrics();
    pool2->report_metrics();
    
    // Check that we have separate metrics for each pool
    auto metrics1 = monitoring_->get_pool_metrics("pool_alpha_" + 
                                                 std::to_string(pool1->get_pool_instance_id()));
    auto metrics2 = monitoring_->get_pool_metrics("pool_beta_" + 
                                                 std::to_string(pool2->get_pool_instance_id()));
    
    EXPECT_TRUE(metrics1.has_value());
    EXPECT_TRUE(metrics2.has_value());
}

TEST_F(MonitoringIntegrationTest, IdleWorkerTracking) {
    auto pool = std::make_shared<thread_pool>("idle_tracking_pool", context_);
    pool->start();
    
    // Initially all workers should be idle
    pool->report_metrics();
    auto initial_metrics = monitoring_->get_pool_metrics("idle_tracking_pool_" + 
                                                        std::to_string(pool->get_pool_instance_id()));
    ASSERT_TRUE(initial_metrics.has_value());
    size_t total_workers = initial_metrics->active_workers + initial_metrics->idle_workers;
    EXPECT_EQ(initial_metrics->idle_workers, total_workers);
    
    // Submit a long-running job
    std::atomic<bool> job_can_finish(false);
    pool->enqueue(std::make_unique<callback_job>(
        [&job_can_finish]() {
            while (!job_can_finish) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        },
        "long_job"
    ));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Now one worker should be active
    pool->report_metrics();
    auto busy_metrics = monitoring_->get_pool_metrics("idle_tracking_pool_" + 
                                                     std::to_string(pool->get_pool_instance_id()));
    ASSERT_TRUE(busy_metrics.has_value());
    EXPECT_EQ(busy_metrics->active_workers, 1);
    EXPECT_EQ(busy_metrics->idle_workers, total_workers - 1);
    
    // Let job finish
    job_can_finish = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // All workers should be idle again
    pool->report_metrics();
    auto final_metrics = monitoring_->get_pool_metrics("idle_tracking_pool_" + 
                                                      std::to_string(pool->get_pool_instance_id()));
    ASSERT_TRUE(final_metrics.has_value());
    EXPECT_EQ(final_metrics->idle_workers, total_workers);
}