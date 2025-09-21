#pragma once

namespace kcenon::messaging::config {
    struct messaging_config {
        // Placeholder configuration for Phase 1
        bool enable_logging = true;
        bool enable_monitoring = true;
        size_t thread_pool_size = 4;
    };
}