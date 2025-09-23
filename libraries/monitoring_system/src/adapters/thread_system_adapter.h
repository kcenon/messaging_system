#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <atomic>

namespace kcenon::monitoring::adapters {

    /**
     * @brief Adapter for integrating with thread_system library
     *
     * This is a stub implementation for the thread system adapter.
     * It provides the basic interface required by the monitoring system
     * to integrate with the thread_system library when available.
     */
    class thread_system_adapter {
    public:
        thread_system_adapter() = default;
        virtual ~thread_system_adapter() = default;

        // Thread monitoring capabilities
        virtual size_t get_thread_count() const { return 0; }
        virtual size_t get_active_threads() const { return 0; }
        virtual size_t get_idle_threads() const { return 0; }

        // Task monitoring
        virtual size_t get_pending_tasks() const { return 0; }
        virtual size_t get_completed_tasks() const { return 0; }
        virtual size_t get_failed_tasks() const { return 0; }

        // Performance metrics
        virtual double get_average_task_duration_ms() const { return 0.0; }
        virtual double get_thread_utilization() const { return 0.0; }

        // Control
        virtual bool is_enabled() const { return false; }
        virtual void enable() {}
        virtual void disable() {}

    private:
        std::atomic<bool> enabled_{false};
    };

} // namespace kcenon::monitoring::adapters