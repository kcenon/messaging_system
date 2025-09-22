/*
 * BSD 3-Clause License
 * Copyright (c) 2025, DongCheol Shin
 */

#pragma once

#include <memory>

// Use project result type and job forward declarations
#include <kcenon/thread/core/error_handling.h>
#include <kcenon/thread/core/job.h>

namespace kcenon::thread {

/**
 * @brief Executor interface for submitting work and coordinating shutdown.
 */
class executor_interface {
public:
    virtual ~executor_interface() = default;

    /**
     * @brief Submit a unit of work for asynchronous execution.
     */
    virtual auto execute(std::unique_ptr<job>&& work) -> result_void = 0;

    /**
     * @brief Initiate a cooperative shutdown.
     */
    virtual auto shutdown() -> result_void = 0;
};

} // namespace kcenon::thread

