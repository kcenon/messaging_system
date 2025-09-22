#include <thread_system_core/thread_base/jobs/job_queue.h>
#include <thread_system_core/thread_base/jobs/callback_job.h>
#include <thread_system_core/thread_base/lockfree/queues/adaptive_job_queue.h>
#include <iostream>

int main() {
    using namespace thread_module;
    
    // Test job queue directly
    auto queue = create_job_queue(adaptive_job_queue::queue_strategy::ADAPTIVE);
    
    std::cout << "Initial queue size: " << queue->size() << std::endl;
    
    // Create and enqueue a simple job
    auto job = std::make_unique<callback_job>(
        []() -> std::optional<std::string> {
            std::cout << "Job executed!" << std::endl;
            return std::nullopt;
        },
        "test_job"
    );
    
    queue->enqueue(std::move(job));
    
    std::cout << "Queue size after enqueue: " << queue->size() << std::endl;
    
    // Dequeue and execute the job
    auto dequeued = queue->dequeue();
    if (dequeued && dequeued.value()) {
        auto& job_ptr = dequeued.value();
        std::cout << "Job dequeued: " << job_ptr->get_name() << std::endl;
        job_ptr->do_work();  // Use do_work() method
    } else {
        std::cout << "No job dequeued" << std::endl;
    }
    
    std::cout << "Final queue size: " << queue->size() << std::endl;
    
    return 0;
}