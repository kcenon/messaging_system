#pragma once

#include <memory>
#include <string>
#include "../../sources/thread_base/jobs/callback_job.h"
#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
#include "../../sources/typed_thread_pool/scheduling/typed_thread_worker.h"

// Make sure we have the proper namespace aliases
using result_void = thread_module::result_void;

// Helper macro to create callback jobs with proper return type
#define MAKE_JOB(lambda_body) \
    std::make_unique<thread_module::callback_job>([&]() -> result_void { \
        lambda_body \
        return result_void{}; \
    })

// Helper function to create and setup a thread pool
inline auto setup_thread_pool(size_t worker_count) -> std::shared_ptr<thread_pool_module::thread_pool> {
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add workers
    for (size_t i = 0; i < worker_count; ++i) {
        auto worker = std::make_unique<thread_pool_module::thread_worker>();
        pool->enqueue(std::move(worker));
    }
    
    // Start the pool
    pool->start();
    
    return pool;
}

// Helper function for typed thread pool
template<typename PriorityType>
inline auto setup_typed_thread_pool(size_t worker_count) -> std::shared_ptr<typed_thread_pool_module::typed_thread_pool_t<PriorityType>> {
    auto pool = std::make_shared<typed_thread_pool_module::typed_thread_pool_t<PriorityType>>();
    
    // Add workers
    for (size_t i = 0; i < worker_count; ++i) {
        auto worker = std::make_unique<typed_thread_pool_module::typed_thread_worker_t<PriorityType>>();
        pool->enqueue(std::move(worker));
    }
    
    // Start the pool
    pool->start();
    
    return pool;
}