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

#pragma once

#include "../database_types.h"
#include <chrono>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <condition_variable>
#include <functional>
#include <string>

namespace database::monitoring
{
	/**
	 * @struct query_metrics
	 * @brief Metrics for individual query execution.
	 */
	struct query_metrics
	{
		std::string query_hash;
		std::chrono::steady_clock::time_point start_time;
		std::chrono::steady_clock::time_point end_time;
		std::chrono::microseconds execution_time{0};
		size_t rows_affected = 0;
		bool success = false;
		std::string error_message;
		database_types db_type = database_types::none;
	};

	/**
	 * @struct connection_metrics
	 * @brief Metrics for database connection usage.
	 */
	struct connection_metrics
	{
		std::atomic<size_t> total_connections{0};
		std::atomic<size_t> active_connections{0};
		std::atomic<size_t> idle_connections{0};
		std::atomic<size_t> failed_connections{0};
		std::atomic<std::chrono::microseconds> avg_acquisition_time{std::chrono::microseconds{0}};
		std::atomic<std::chrono::microseconds> max_acquisition_time{std::chrono::microseconds{0}};
		std::chrono::steady_clock::time_point last_update;

		// Default constructor
		connection_metrics() = default;

		// Copy constructor
		connection_metrics(const connection_metrics& other) :
			total_connections(other.total_connections.load()),
			active_connections(other.active_connections.load()),
			idle_connections(other.idle_connections.load()),
			failed_connections(other.failed_connections.load()),
			avg_acquisition_time(other.avg_acquisition_time.load()),
			max_acquisition_time(other.max_acquisition_time.load()),
			last_update(other.last_update) {}

		// Move constructor
		connection_metrics(connection_metrics&& other) noexcept :
			total_connections(other.total_connections.load()),
			active_connections(other.active_connections.load()),
			idle_connections(other.idle_connections.load()),
			failed_connections(other.failed_connections.load()),
			avg_acquisition_time(other.avg_acquisition_time.load()),
			max_acquisition_time(other.max_acquisition_time.load()),
			last_update(std::move(other.last_update)) {}

		// Copy assignment operator
		connection_metrics& operator=(const connection_metrics& other) {
			if (this != &other) {
				total_connections.store(other.total_connections.load());
				active_connections.store(other.active_connections.load());
				idle_connections.store(other.idle_connections.load());
				failed_connections.store(other.failed_connections.load());
				avg_acquisition_time.store(other.avg_acquisition_time.load());
				max_acquisition_time.store(other.max_acquisition_time.load());
				last_update = other.last_update;
			}
			return *this;
		}

		// Move assignment operator
		connection_metrics& operator=(connection_metrics&& other) noexcept {
			if (this != &other) {
				total_connections.store(other.total_connections.load());
				active_connections.store(other.active_connections.load());
				idle_connections.store(other.idle_connections.load());
				failed_connections.store(other.failed_connections.load());
				avg_acquisition_time.store(other.avg_acquisition_time.load());
				max_acquisition_time.store(other.max_acquisition_time.load());
				last_update = std::move(other.last_update);
			}
			return *this;
		}
	};

	/**
	 * @struct performance_summary
	 * @brief Aggregated performance metrics.
	 */
	struct performance_summary
	{
		// Query metrics
		size_t total_queries = 0;
		size_t successful_queries = 0;
		size_t failed_queries = 0;
		std::chrono::microseconds avg_query_time{0};
		std::chrono::microseconds min_query_time{0};
		std::chrono::microseconds max_query_time{0};
		double queries_per_second = 0.0;

		// Connection metrics
		size_t total_connections = 0;
		size_t active_connections = 0;
		double connection_utilization = 0.0;
		std::chrono::microseconds avg_connection_time{0};

		// Error rates
		double error_rate = 0.0;
		std::unordered_map<std::string, size_t> error_counts;

		// Timestamps
		std::chrono::steady_clock::time_point measurement_start;
		std::chrono::steady_clock::time_point measurement_end;
	};

	/**
	 * @class performance_alert
	 * @brief Alert system for performance thresholds.
	 */
	class performance_alert
	{
	public:
		enum class alert_type {
			high_latency,
			high_error_rate,
			connection_pool_exhaustion,
			slow_query,
			memory_usage,
			cpu_usage
		};

		performance_alert(alert_type type, const std::string& message,
		                 std::chrono::steady_clock::time_point timestamp);

		alert_type type() const { return type_; }
		const std::string& message() const { return message_; }
		std::chrono::steady_clock::time_point timestamp() const { return timestamp_; }

	private:
		alert_type type_;
		std::string message_;
		std::chrono::steady_clock::time_point timestamp_;
	};

	/**
	 * @class query_timer
	 * @brief RAII timer for measuring query execution time.
	 */
	class query_timer
	{
	public:
		query_timer(const std::string& query, database_types db_type);
		~query_timer();

		void set_rows_affected(size_t rows) { metrics_.rows_affected = rows; }
		void set_error(const std::string& error);

	private:
		query_metrics metrics_;
		std::chrono::steady_clock::time_point start_time_;
	};

	/**
	 * @class performance_monitor
	 * @brief Main performance monitoring system.
	 */
	class performance_monitor
	{
	public:
		static performance_monitor& instance();
		~performance_monitor();

		// Configuration
		void set_monitoring_enabled(bool enabled) { monitoring_enabled_ = enabled; }
		void set_metrics_retention_period(std::chrono::minutes period) { retention_period_ = period; }
		void set_alert_thresholds(double error_rate_threshold,
		                         std::chrono::microseconds latency_threshold);

		// Metrics collection
		void record_query_metrics(const query_metrics& metrics);
		void record_connection_metrics(database_types db_type, const connection_metrics& metrics);
		void record_slow_query(const std::string& query, std::chrono::microseconds execution_time);

		// Metrics retrieval
		performance_summary get_performance_summary() const;
		performance_summary get_performance_summary(database_types db_type) const;
		std::vector<query_metrics> get_recent_queries(std::chrono::minutes window) const;
		std::vector<query_metrics> get_slow_queries(std::chrono::microseconds threshold) const;

		// Connection monitoring
		void update_connection_count(database_types db_type, size_t active, size_t total);
		connection_metrics get_connection_metrics(database_types db_type) const;

		// Alert system
		void register_alert_handler(std::function<void(const performance_alert&)> handler);
		std::vector<performance_alert> get_recent_alerts(std::chrono::minutes window) const;

		// Cache management
		void clear_metrics();
		void cleanup_old_metrics();

		// Dashboard support
		std::string get_metrics_json() const;
		std::string get_dashboard_html() const;

	private:
		performance_monitor();

		// Internal methods
		void cleanup_thread();
		void check_thresholds();
		void emit_alert(performance_alert::alert_type type, const std::string& message);
		std::string calculate_query_hash(const std::string& query) const;

		// Configuration
		std::atomic<bool> monitoring_enabled_{true};
		std::chrono::minutes retention_period_{60}; // 1 hour
		double error_rate_threshold_ = 0.05; // 5%
		std::chrono::microseconds latency_threshold_{1000000}; // 1 second

		// Metrics storage
		mutable std::mutex metrics_mutex_;
		std::vector<query_metrics> query_history_;
		std::unordered_map<database_types, connection_metrics> connection_metrics_;
		std::vector<performance_alert> alerts_;

		// Alert handlers
		std::mutex handlers_mutex_;
		std::vector<std::function<void(const performance_alert&)>> alert_handlers_;

		// Background cleanup
		std::atomic<bool> cleanup_running_{true};
		std::thread cleanup_thread_;
		std::condition_variable cleanup_cv_;
		std::mutex cleanup_mutex_;

		// Query caching for pattern analysis
		std::unordered_map<std::string, size_t> query_patterns_;
		std::unordered_map<std::string, std::chrono::microseconds> query_avg_times_;
	};

	/**
	 * @class metrics_exporter
	 * @brief Export metrics to external monitoring systems.
	 */
	class metrics_exporter
	{
	public:
		virtual ~metrics_exporter() = default;

		virtual bool export_metrics(const performance_summary& summary) = 0;
		virtual bool export_alerts(const std::vector<performance_alert>& alerts) = 0;
	};

	/**
	 * @class prometheus_exporter
	 * @brief Export metrics in Prometheus format.
	 */
	class prometheus_exporter : public metrics_exporter
	{
	public:
		prometheus_exporter(const std::string& endpoint, int port);

		bool export_metrics(const performance_summary& summary) override;
		bool export_alerts(const std::vector<performance_alert>& alerts) override;

		std::string format_prometheus_metrics(const performance_summary& summary) const;

	private:
		std::string endpoint_;
		int port_;
	};

	/**
	 * @class dashboard_server
	 * @brief Simple HTTP server for performance dashboard.
	 */
	class dashboard_server
	{
	public:
		dashboard_server(int port = 8080);
		~dashboard_server();

		bool start();
		void stop();

		void set_custom_dashboard(const std::string& html_content);

	private:
		void server_thread();
		std::string handle_request(const std::string& path) const;

		int port_;
		std::atomic<bool> running_{false};
		std::thread server_thread_;
		std::string custom_dashboard_;
	};

	// Helper macros for automatic query timing
	#define MONITOR_QUERY(query, db_type) \
		database::monitoring::query_timer timer_(query, db_type)

	#define MONITOR_QUERY_RESULT(rows) \
		timer_.set_rows_affected(rows)

	#define MONITOR_QUERY_ERROR(error) \
		timer_.set_error(error)

} // namespace database::monitoring