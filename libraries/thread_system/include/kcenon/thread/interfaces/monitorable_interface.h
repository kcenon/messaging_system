/*
 * BSD 3-Clause License
 * Copyright (c) 2025, DongCheol Shin
 */

#pragma once

#include "monitoring_interface.h"

namespace kcenon::thread {

/**
 * @brief Interface for components exposing metrics.
 */
class monitorable_interface {
public:
    virtual ~monitorable_interface() = default;

    /** @brief Fetch current metrics snapshot. */
    virtual auto get_metrics() -> ::monitoring_interface::metrics_snapshot = 0;

    /** @brief Reset internal metrics counters. */
    virtual void reset_metrics() = 0;
};

} // namespace kcenon::thread

