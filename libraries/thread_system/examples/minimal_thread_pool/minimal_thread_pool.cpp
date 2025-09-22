/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/thread_worker.h>
#include <kcenon/thread/core/callback_job.h>
#include <kcenon/thread/interfaces/thread_context.h>

using namespace kcenon::thread;
using namespace kcenon::thread;

int main() {
    std::cout << "=== Minimal Thread Pool Sample (No Logger) ===" << std::endl;
    
    // Create thread pool
    thread_context context;
    auto pool = std::make_shared<thread_pool>("MinimalPool", context);
    
    // Create workers
    const size_t worker_count = 4;
    std::vector<std::unique_ptr<thread_worker>> workers;
    
    for (size_t i = 0; i < worker_count; ++i) {
        workers.push_back(std::make_unique<thread_worker>(false, context));
    }
    
    // Add workers to pool
    auto result = pool->enqueue_batch(std::move(workers));
    if (result.has_error()) {
        std::cerr << "Error adding workers: " << result.get_error().to_string() << std::endl;
        return 1;
    }
    
    // Start the pool
    result = pool->start();
    if (result.has_error()) {
        std::cerr << "Error starting pool: " << result.get_error().to_string() << std::endl;
        return 1;
    }
    
    std::cout << "Thread pool started with " << worker_count << " workers" << std::endl;
    
    // Submit some jobs
    std::atomic<int> completed_jobs{0};
    const int total_jobs = 20;
    
    std::cout << "Submitting " << total_jobs << " jobs..." << std::endl;
    
    for (int i = 0; i < total_jobs; ++i) {
        auto job = std::make_unique<callback_job>(
            [i, &completed_jobs]() -> result_void {
                // Simulate some work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                // Print progress (thread-safe)
                int current = completed_jobs.fetch_add(1) + 1;
                std::cout << "Job " << i << " completed. Total: "
                         << current << "/" << total_jobs << std::endl;

                return result_void{};
            },
            "job_" + std::to_string(i)
        );
        
        result = pool->enqueue(std::move(job));
        if (result.has_error()) {
            std::cerr << "Error enqueuing job: " << result.get_error().to_string() << std::endl;
        }
    }
    
    // Wait for all jobs to complete
    std::cout << "Waiting for jobs to complete..." << std::endl;
    while (completed_jobs.load() < total_jobs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "All jobs completed!" << std::endl;
    
    // Stop the pool
    auto stop_result = pool->stop();
    if (stop_result.has_error()) {
        std::cerr << "Error stopping pool: " << stop_result.get_error().to_string() << std::endl;
    }
    std::cout << "Thread pool stopped." << std::endl;
    
    return 0;
}
