#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../interfaces/metric_collector_interface.h"
#include "../interfaces/metric_types_adapter.h"
#include "../interfaces/observer_interface.h"
#include "../utils/metric_types.h"

namespace monitoring_system {

/**
 * Plugin interface for metric collectors
 * All metric collector plugins must implement this interface
 */
class metric_collector_plugin {
  public:
    virtual ~metric_collector_plugin() = default;

    /**
     * Initialize the plugin with configuration
     * @param config Plugin-specific configuration
     * @return true if initialization successful
     */
    virtual bool initialize(const std::unordered_map<std::string, std::string>& config) = 0;

    /**
     * Collect metrics from the data source
     * @return Collection of metrics
     */
    virtual std::vector<metric> collect() = 0;

    /**
     * Get the name of this plugin
     * @return Plugin name
     */
    virtual std::string get_name() const = 0;

    /**
     * Get supported metric types
     * @return Vector of supported metric type names
     */
    virtual std::vector<std::string> get_metric_types() const = 0;

    /**
     * Check if the plugin is healthy
     * @return true if plugin is operational
     */
    virtual bool is_healthy() const = 0;

    /**
     * Get plugin-specific statistics
     * @return Map of statistic name to value
     */
    virtual std::unordered_map<std::string, double> get_statistics() const = 0;
};

/**
 * Plugin loading and management interface
 */
class plugin_loader {
  public:
    virtual ~plugin_loader() = default;

    /**
     * Load a plugin from a shared library
     * @param path Path to the plugin library
     * @return Loaded plugin instance or nullptr on failure
     */
    virtual std::unique_ptr<metric_collector_plugin> load_plugin(const std::string& path) = 0;

    /**
     * Unload a plugin
     * @param plugin_name Name of the plugin to unload
     * @return true if unloaded successfully
     */
    virtual bool unload_plugin(const std::string& plugin_name) = 0;
};

/**
 * Configuration for plugin metric collector
 */
struct plugin_collector_config {
    // Collection interval in milliseconds
    std::chrono::milliseconds collection_interval{1000};

    // Maximum batch size for metric collection
    size_t max_batch_size{1000};

    // Enable caching of metrics
    bool enable_caching{true};

    // Cache TTL in seconds
    std::chrono::seconds cache_ttl{60};

    // Enable real-time streaming
    bool enable_streaming{false};

    // Number of worker threads for collection
    size_t worker_threads{2};

    // Enable metric aggregation
    bool enable_aggregation{true};

    // Aggregation window in seconds
    std::chrono::seconds aggregation_window{10};
};

/**
 * Main plugin-based metric collector implementation
 * Manages multiple collector plugins and provides unified metric collection
 */
class plugin_metric_collector : public interface_metric_collector {
  public:
    explicit plugin_metric_collector(const plugin_collector_config& config = {});
    ~plugin_metric_collector() override;

    // Disable copy
    plugin_metric_collector(const plugin_metric_collector&) = delete;
    plugin_metric_collector& operator=(const plugin_metric_collector&) = delete;

    // Enable move
    plugin_metric_collector(plugin_metric_collector&&) noexcept = default;
    plugin_metric_collector& operator=(plugin_metric_collector&&) noexcept = default;

    /**
     * Register a metric collector plugin
     * @param plugin Plugin instance to register
     * @return true if registered successfully
     */
    bool register_plugin(std::unique_ptr<metric_collector_plugin> plugin);

    /**
     * Unregister a plugin by name
     * @param plugin_name Name of the plugin to unregister
     * @return true if unregistered successfully
     */
    bool unregister_plugin(const std::string& plugin_name);

    /**
     * Get list of registered plugin names
     * @return Vector of plugin names
     */
    std::vector<std::string> get_registered_plugins() const;

    /**
     * Start metric collection
     * @return true if started successfully
     */
    bool start();

    /**
     * Stop metric collection
     */
    void stop();

    /**
     * Check if collector is running
     * @return true if collection is active
     */
    bool is_running() const;

    /**
     * Get current metrics from cache
     * @param plugin_name Optional plugin name filter
     * @return Vector of cached metrics
     */
    std::vector<metric> get_cached_metrics(const std::optional<std::string>& plugin_name = std::nullopt) const;

    /**
     * Get aggregated metrics
     * @param window_seconds Aggregation window in seconds
     * @return Aggregated metrics
     */
    std::vector<metric_stats> get_aggregated_metrics(size_t window_seconds = 60) const;

    /**
     * Force immediate collection from all plugins
     * @return Collected metrics
     */
    std::vector<metric> force_collect();

    // interface_metric_collector implementation
    collection_result collect_metrics() override;
    void register_observer(std::shared_ptr<interface_monitoring_observer> observer) override;
    void unregister_observer(std::shared_ptr<interface_monitoring_observer> observer) override;
    std::vector<std::string> get_metric_types() const override;
    void configure(const collection_config& config) override;

  private:
    // Plugin management
    mutable std::mutex plugins_mutex_;
    std::unordered_map<std::string, std::unique_ptr<metric_collector_plugin>> plugins_;

    // Observer management
    mutable std::mutex observers_mutex_;
    std::vector<std::weak_ptr<interface_monitoring_observer>> observers_;

    // Metric cache
    struct cached_metric {
        metric data;
        std::chrono::steady_clock::time_point timestamp;
        std::string plugin_name;
    };
    mutable std::mutex cache_mutex_;
    std::vector<cached_metric> metric_cache_;

    // Collection state
    std::atomic<bool> running_{false};
    std::vector<std::thread> worker_threads_;
    std::condition_variable work_cv_;
    mutable std::mutex work_mutex_;

    // Configuration
    plugin_collector_config config_;

    // Statistics
    std::atomic<size_t> total_metrics_collected_{0};
    std::atomic<size_t> collection_errors_{0};
    std::chrono::steady_clock::time_point start_time_;

    // Internal methods
    void collection_worker();
    void collect_from_plugin(const std::string& name, metric_collector_plugin* plugin);
    void notify_observers(const metric& m);
    void cleanup_cache();
    void aggregate_metrics();
};

/**
 * Factory for creating standard collector plugins
 */
class plugin_factory {
  public:
    /**
     * Create a system resource collector plugin
     * @param config Plugin configuration
     * @return System resource collector instance
     */
    static std::unique_ptr<metric_collector_plugin> create_system_resource_collector(
        const std::unordered_map<std::string, std::string>& config = {});

    /**
     * Create a thread system collector plugin
     * @param config Plugin configuration
     * @return Thread system collector instance
     */
    static std::unique_ptr<metric_collector_plugin> create_thread_system_collector(
        const std::unordered_map<std::string, std::string>& config = {});

    /**
     * Create a logger system collector plugin
     * @param config Plugin configuration
     * @return Logger system collector instance
     */
    static std::unique_ptr<metric_collector_plugin> create_logger_system_collector(
        const std::unordered_map<std::string, std::string>& config = {});

    /**
     * Register a custom plugin factory
     * @param type Plugin type name
     * @param factory Factory function
     */
    static void register_factory(
        const std::string& type,
        std::function<std::unique_ptr<metric_collector_plugin>(const std::unordered_map<std::string, std::string>&)>
            factory);

    /**
     * Create a plugin by type name
     * @param type Plugin type name
     * @param config Plugin configuration
     * @return Plugin instance or nullptr if type not found
     */
    static std::unique_ptr<metric_collector_plugin> create(const std::string& type,
                                                             const std::unordered_map<std::string, std::string>& config = {});

  private:
    static std::unordered_map<
        std::string, std::function<std::unique_ptr<metric_collector_plugin>(const std::unordered_map<std::string, std::string>&)>>
        factories_;
    static std::mutex factories_mutex_;
};

}  // namespace monitoring_system