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

#include "thread_pool/core/thread_pool.h"
#include "thread_pool/workers/thread_worker.h"
#include "thread_base/jobs/callback_job.h"

using namespace thread_pool_module;
using namespace thread_module;

int main() {
    std::cout << "=== Minimal Thread Pool Sample (No Logger) ===" << std::endl;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("MinimalPool");
    
    // Create workers
    const size_t worker_count = 4;
    std::vector<std::unique_ptr<thread_worker>> workers;
    
    for (size_t i = 0; i < worker_count; ++i) {
        workers.push_back(std::make_unique<thread_worker>(false));
    }
    
    // Add workers to pool
    auto result = pool->enqueue_batch(std::move(workers));
    if (result.has_value()) {
        std::cerr << "Error adding workers: " << result.value() << std::endl;
        return 1;
    }
    
    // Start the pool
    result = pool->start();
    if (result.has_value()) {
        std::cerr << "Error starting pool: " << result.value() << std::endl;
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
            }
        );
        
        result = pool->enqueue(std::move(job));
        if (result.has_value()) {
            std::cerr << "Error enqueuing job: " << result.value() << std::endl;
        }
    }
    
    // Wait for all jobs to complete
    std::cout << "Waiting for jobs to complete..." << std::endl;
    while (completed_jobs.load() < total_jobs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "All jobs completed!" << std::endl;
    
    // Stop the pool
    pool->stop();
    std::cout << "Thread pool stopped." << std::endl;
    
    return 0;
}