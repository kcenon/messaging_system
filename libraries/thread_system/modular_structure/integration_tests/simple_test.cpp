#include <thread_system_core/thread_pool/core/thread_pool.h>
#include <thread_system_core/thread_pool/workers/thread_worker.h>
#include <thread_system_core/thread_base/jobs/callback_job.h>
#include <iostream>
#include <atomic>

int main() {
    using namespace thread_pool_module;
    using namespace thread_module;
    
    std::cout << "Creating thread pool..." << std::endl;
    auto pool = std::make_shared<thread_pool>("test_pool");
    
    // Add workers to the pool
    const int num_workers = 4;
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        worker->set_job_queue(pool->get_job_queue());
        workers.push_back(std::move(worker));
    }
    pool->enqueue_batch(std::move(workers));
    
    std::cout << "Starting pool..." << std::endl;
    if (auto err = pool->start()) {
        std::cerr << "Failed to start pool: " << *err << std::endl;
        return 1;
    }
    
    // Give workers time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Idle worker count: " << pool->get_idle_worker_count() << std::endl;
    
    std::atomic<int> counter(0);
    
    std::cout << "Job queue size before: " << pool->get_job_queue()->size() << std::endl;
    
    std::cout << "Enqueuing jobs..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        auto job = std::make_unique<callback_job>(
            [&counter, i]() -> std::optional<std::string> {
                counter++;
                std::cout << "Job " << i << " executed by thread " << std::this_thread::get_id() << std::endl;
                return std::nullopt;
            },
            "job_" + std::to_string(i)
        );
        
        if (auto err = pool->enqueue(std::move(job))) {
            std::cerr << "Failed to enqueue job: " << *err << std::endl;
        } else {
            std::cout << "Successfully enqueued job " << i << std::endl;
        }
    }
    
    std::cout << "Job queue size after: " << pool->get_job_queue()->size() << std::endl;
    
    // Wait for completion with timeout
    auto start_time = std::chrono::steady_clock::now();
    while (counter < 10) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Add timeout check
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > std::chrono::seconds(5)) {
            std::cerr << "Timeout waiting for jobs. Completed: " << counter << "/10" << std::endl;
            break;
        }
    }
    
    std::cout << "All jobs completed. Counter = " << counter << std::endl;
    
    pool->stop();
    std::cout << "Pool stopped successfully." << std::endl;
    
    return 0;
}