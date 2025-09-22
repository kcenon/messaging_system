#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>

#include "implementations/thread_pool/include/thread_pool.h"
#include "core/jobs/include/callback_job.h"

using namespace kcenon::thread;
using namespace kcenon::thread;

// Performance test to measure interface overhead
class performance_tester {
public:
    struct test_results {
        double jobs_per_second;
        double avg_latency_ns;
        std::chrono::milliseconds total_time;
    };

    static auto run_thread_pool_test(int num_jobs, int num_workers) -> test_results {
        auto pool = std::make_shared<thread_pool>("perf_test");
        
        // Add workers
        std::vector<std::unique_ptr<thread_worker>> workers;
        for (int i = 0; i < num_workers; ++i) {
            workers.push_back(std::make_unique<thread_worker>());
        }
        pool->enqueue_batch(std::move(workers));
        pool->start();

        std::atomic<int> completed_jobs{0};
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Submit jobs using old interface
        for (int i = 0; i < num_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([&completed_jobs]() -> result_void {
                completed_jobs.fetch_add(1);
                return {};
            }));
        }

        // Wait for completion
        while (completed_jobs.load() < num_jobs) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        pool->stop();

        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        double jobs_per_second = static_cast<double>(num_jobs) / total_time.count() * 1000.0;
        double avg_latency_ns = static_cast<double>(total_time.count()) * 1000000.0 / num_jobs;

        return {jobs_per_second, avg_latency_ns, total_time};
    }

    static auto run_interface_test(int num_jobs, int num_workers) -> test_results {
        auto pool = std::make_shared<thread_pool>("interface_test");
        
        // Add workers
        std::vector<std::unique_ptr<thread_worker>> workers;
        for (int i = 0; i < num_workers; ++i) {
            workers.push_back(std::make_unique<thread_worker>());
        }
        pool->enqueue_batch(std::move(workers));
        pool->start();

        std::atomic<int> completed_jobs{0};
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Use new interface methods
        for (int i = 0; i < num_jobs; ++i) {
            pool->submit_task([&completed_jobs]() {
                completed_jobs.fetch_add(1);
            });
        }

        // Wait for completion
        while (completed_jobs.load() < num_jobs) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        pool->stop();

        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        double jobs_per_second = static_cast<double>(num_jobs) / total_time.count() * 1000.0;
        double avg_latency_ns = static_cast<double>(total_time.count()) * 1000000.0 / num_jobs;

        return {jobs_per_second, avg_latency_ns, total_time};
    }
};

int main() {
    const int num_jobs = 100000;
    const int num_workers = 4;
    const int num_runs = 3;

    std::cout << "Performance Regression Test\n";
    std::cout << "Jobs: " << num_jobs << ", Workers: " << num_workers << "\n";
    std::cout << "Runs per test: " << num_runs << "\n\n";

    // Warm up
    std::cout << "Warming up...\n";
    performance_tester::run_thread_pool_test(1000, 2);

    double old_interface_total = 0.0;
    double new_interface_total = 0.0;

    for (int run = 0; run < num_runs; ++run) {
        std::cout << "Run " << (run + 1) << ":\n";

        // Test old interface
        auto old_result = performance_tester::run_thread_pool_test(num_jobs, num_workers);
        std::cout << "  Old Interface: " << old_result.jobs_per_second 
                  << " jobs/sec, " << old_result.avg_latency_ns << " ns/job, "
                  << old_result.total_time.count() << " ms\n";
        
        old_interface_total += old_result.jobs_per_second;

        // Test new interface  
        auto new_result = performance_tester::run_interface_test(num_jobs, num_workers);
        std::cout << "  New Interface: " << new_result.jobs_per_second 
                  << " jobs/sec, " << new_result.avg_latency_ns << " ns/job, "
                  << new_result.total_time.count() << " ms\n";
        
        new_interface_total += new_result.jobs_per_second;

        double impact_percent = ((old_result.jobs_per_second - new_result.jobs_per_second) 
                                / old_result.jobs_per_second) * 100.0;
        std::cout << "  Performance Impact: " << impact_percent << "%\n\n";
    }

    // Calculate averages
    double avg_old = old_interface_total / num_runs;
    double avg_new = new_interface_total / num_runs;
    double avg_impact = ((avg_old - avg_new) / avg_old) * 100.0;

    std::cout << "Average Results:\n";
    std::cout << "  Old Interface: " << avg_old << " jobs/sec\n";
    std::cout << "  New Interface: " << avg_new << " jobs/sec\n";
    std::cout << "  Average Impact: " << avg_impact << "%\n";

    if (std::abs(avg_impact) <= 5.0) {
        std::cout << "✅ PASS: Performance impact " << avg_impact 
                  << "% is within 5% threshold\n";
        return 0;
    } else {
        std::cout << "❌ FAIL: Performance impact " << avg_impact 
                  << "% exceeds 5% threshold\n";
        return 1;
    }
}