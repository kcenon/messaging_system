/*
 * BSD 3-Clause License
 * Copyright (c) 2025, DongCheol Shin
 */

#pragma once

#include <memory>

#include <kcenon/thread/core/error_handling.h>
#include <kcenon/thread/core/job.h>

namespace kcenon::thread {

/**
 * @brief Scheduler interface for queuing and retrieving jobs.
 */
class scheduler_interface {
public:
    virtual ~scheduler_interface() = default;

    /**
     * @brief Enqueue a job for processing.
     */
    virtual auto schedule(std::unique_ptr<job>&& work) -> result_void = 0;

    /**
     * @brief Dequeue the next available job.
     */
    virtual auto get_next_job() -> result<std::unique_ptr<job>> = 0;
};

} // namespace kcenon::thread

