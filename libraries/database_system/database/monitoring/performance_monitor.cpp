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

#include "performance_monitor.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <functional>

namespace database::monitoring
{
	// performance_alert implementation
	performance_alert::performance_alert(alert_type type, const std::string& message,
	                                     std::chrono::steady_clock::time_point timestamp)
		: type_(type), message_(message), timestamp_(timestamp)
	{
	}

	// query_timer implementation
	query_timer::query_timer(const std::string& query, database_types db_type)
		: start_time_(std::chrono::steady_clock::now())
	{
		metrics_.query_hash = std::to_string(std::hash<std::string>{}(query));
		metrics_.start_time = start_time_;
		metrics_.db_type = db_type;
		metrics_.success = true; // Assume success unless error is set
	}

	query_timer::~query_timer()
	{
		metrics_.end_time = std::chrono::steady_clock::now();
		metrics_.execution_time = std::chrono::duration_cast<std::chrono::microseconds>(
			metrics_.end_time - metrics_.start_time);

		performance_monitor::instance().record_query_metrics(metrics_);
	}

	void query_timer::set_error(const std::string& error)
	{
		metrics_.success = false;
		metrics_.error_message = error;
	}

	// performance_monitor implementation
	performance_monitor& performance_monitor::instance()
	{
		static performance_monitor instance;
		return instance;
	}

	performance_monitor::performance_monitor()
		: cleanup_thread_(&performance_monitor::cleanup_thread, this)
	{
	}

	performance_monitor::~performance_monitor()
	{
		cleanup_running_ = false;
		cleanup_cv_.notify_all();
		if (cleanup_thread_.joinable()) {
			cleanup_thread_.join();
		}
	}

	void performance_monitor::set_alert_thresholds(double error_rate_threshold,
	                                               std::chrono::microseconds latency_threshold)
	{
		error_rate_threshold_ = error_rate_threshold;
		latency_threshold_ = latency_threshold;
	}

	void performance_monitor::record_query_metrics(const query_metrics& metrics)
	{
		if (!monitoring_enabled_) return;

		std::lock_guard<std::mutex> lock(metrics_mutex_);
		query_history_.push_back(metrics);

		// Update query patterns
		query_patterns_[metrics.query_hash]++;

		auto& avg_time = query_avg_times_[metrics.query_hash];
		auto count = query_patterns_[metrics.query_hash];
		avg_time = std::chrono::microseconds(
			(avg_time.count() * (count - 1) + metrics.execution_time.count()) / count);

		// Check for slow queries
		if (metrics.execution_time > latency_threshold_) {
			emit_alert(performance_alert::alert_type::slow_query,
			          "Slow query detected: " + std::to_string(metrics.execution_time.count()) + "μs");
		}

		check_thresholds();
	}

	void performance_monitor::record_connection_metrics(database_types db_type, const connection_metrics& metrics)
	{
		if (!monitoring_enabled_) return;

		std::lock_guard<std::mutex> lock(metrics_mutex_);
		auto& stored_metrics = connection_metrics_[db_type];
		stored_metrics.total_connections.store(metrics.total_connections.load());
		stored_metrics.active_connections.store(metrics.active_connections.load());
		stored_metrics.idle_connections.store(metrics.idle_connections.load());
		stored_metrics.failed_connections.store(metrics.failed_connections.load());
		stored_metrics.avg_acquisition_time.store(metrics.avg_acquisition_time.load());
		stored_metrics.max_acquisition_time.store(metrics.max_acquisition_time.load());
		stored_metrics.last_update = metrics.last_update;

		// Check for connection pool exhaustion
		auto total = metrics.total_connections.load();
		auto active = metrics.active_connections.load();
		if (total > 0 && (double(active) / total) > 0.9) {
			emit_alert(performance_alert::alert_type::connection_pool_exhaustion,
			          "Connection pool utilization high: " + std::to_string(active) + "/" + std::to_string(total));
		}
	}

	void performance_monitor::record_slow_query(const std::string& query, std::chrono::microseconds execution_time)
	{
		if (!monitoring_enabled_) return;

		emit_alert(performance_alert::alert_type::slow_query,
		          "Slow query: " + query.substr(0, 100) + "... (" +
		          std::to_string(execution_time.count()) + "μs)");
	}

	performance_summary performance_monitor::get_performance_summary() const
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);

		performance_summary summary;
		summary.measurement_start = std::chrono::steady_clock::now() - retention_period_;
		summary.measurement_end = std::chrono::steady_clock::now();

		if (query_history_.empty()) {
			return summary;
		}

		// Filter recent queries
		auto recent_start = summary.measurement_start;
		auto recent_queries = std::count_if(query_history_.begin(), query_history_.end(),
			[recent_start](const query_metrics& m) {
				return m.start_time >= recent_start;
			});

		summary.total_queries = recent_queries;

		// Calculate metrics from recent queries
		std::chrono::microseconds total_time{0};
		std::chrono::microseconds min_time{std::chrono::microseconds::max()};
		std::chrono::microseconds max_time{0};
		size_t successful = 0;

		for (const auto& metrics : query_history_) {
			if (metrics.start_time < recent_start) continue;

			total_time += metrics.execution_time;
			min_time = std::min(min_time, metrics.execution_time);
			max_time = std::max(max_time, metrics.execution_time);

			if (metrics.success) {
				successful++;
			} else {
				summary.error_counts[metrics.error_message]++;
			}
		}

		summary.successful_queries = successful;
		summary.failed_queries = summary.total_queries - successful;

		if (summary.total_queries > 0) {
			summary.avg_query_time = total_time / summary.total_queries;
			summary.min_query_time = min_time;
			summary.max_query_time = max_time;
			summary.error_rate = double(summary.failed_queries) / summary.total_queries;
		}

		// Calculate QPS
		auto duration_seconds = std::chrono::duration_cast<std::chrono::seconds>(
			summary.measurement_end - summary.measurement_start).count();
		if (duration_seconds > 0) {
			summary.queries_per_second = double(summary.total_queries) / duration_seconds;
		}

		// Connection metrics summary
		size_t total_connections = 0;
		size_t active_connections = 0;
		for (const auto& [db_type, conn_metrics] : connection_metrics_) {
			total_connections += conn_metrics.total_connections.load();
			active_connections += conn_metrics.active_connections.load();
		}

		summary.total_connections = total_connections;
		summary.active_connections = active_connections;
		if (total_connections > 0) {
			summary.connection_utilization = double(active_connections) / total_connections;
		}

		return summary;
	}

	performance_summary performance_monitor::get_performance_summary(database_types db_type) const
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);

		performance_summary summary;
		summary.measurement_start = std::chrono::steady_clock::now() - retention_period_;
		summary.measurement_end = std::chrono::steady_clock::now();

		// Filter by database type and time
		auto recent_start = summary.measurement_start;
		auto db_queries = std::count_if(query_history_.begin(), query_history_.end(),
			[recent_start, db_type](const query_metrics& m) {
				return m.start_time >= recent_start && m.db_type == db_type;
			});

		summary.total_queries = db_queries;

		// Calculate metrics for specific database type
		std::chrono::microseconds total_time{0};
		size_t successful = 0;

		for (const auto& metrics : query_history_) {
			if (metrics.start_time < recent_start || metrics.db_type != db_type) continue;

			total_time += metrics.execution_time;
			if (metrics.success) {
				successful++;
			}
		}

		summary.successful_queries = successful;
		summary.failed_queries = summary.total_queries - successful;

		if (summary.total_queries > 0) {
			summary.avg_query_time = total_time / summary.total_queries;
			summary.error_rate = double(summary.failed_queries) / summary.total_queries;
		}

		// Connection metrics for specific database
		auto it = connection_metrics_.find(db_type);
		if (it != connection_metrics_.end()) {
			summary.total_connections = it->second.total_connections.load();
			summary.active_connections = it->second.active_connections.load();
			if (summary.total_connections > 0) {
				summary.connection_utilization = double(summary.active_connections) / summary.total_connections;
			}
		}

		return summary;
	}

	std::vector<query_metrics> performance_monitor::get_recent_queries(std::chrono::minutes window) const
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);

		auto cutoff = std::chrono::steady_clock::now() - window;
		std::vector<query_metrics> recent;

		std::copy_if(query_history_.begin(), query_history_.end(), std::back_inserter(recent),
			[cutoff](const query_metrics& m) {
				return m.start_time >= cutoff;
			});

		return recent;
	}

	std::vector<query_metrics> performance_monitor::get_slow_queries(std::chrono::microseconds threshold) const
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);

		std::vector<query_metrics> slow_queries;
		std::copy_if(query_history_.begin(), query_history_.end(), std::back_inserter(slow_queries),
			[threshold](const query_metrics& m) {
				return m.execution_time >= threshold;
			});

		return slow_queries;
	}

	void performance_monitor::update_connection_count(database_types db_type, size_t active, size_t total)
	{
		if (!monitoring_enabled_) return;

		std::lock_guard<std::mutex> lock(metrics_mutex_);
		auto& metrics = connection_metrics_[db_type];
		metrics.active_connections = active;
		metrics.total_connections = total;
		metrics.last_update = std::chrono::steady_clock::now();
	}

	connection_metrics performance_monitor::get_connection_metrics(database_types db_type) const
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);
		auto it = connection_metrics_.find(db_type);
		if (it != connection_metrics_.end()) {
			connection_metrics result;
			result.total_connections.store(it->second.total_connections.load());
			result.active_connections.store(it->second.active_connections.load());
			result.idle_connections.store(it->second.idle_connections.load());
			result.failed_connections.store(it->second.failed_connections.load());
			result.avg_acquisition_time.store(it->second.avg_acquisition_time.load());
			result.max_acquisition_time.store(it->second.max_acquisition_time.load());
			result.last_update = it->second.last_update;
			return result;
		}
		return connection_metrics{};
	}

	void performance_monitor::register_alert_handler(std::function<void(const performance_alert&)> handler)
	{
		std::lock_guard<std::mutex> lock(handlers_mutex_);
		alert_handlers_.push_back(handler);
	}

	std::vector<performance_alert> performance_monitor::get_recent_alerts(std::chrono::minutes window) const
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);

		auto cutoff = std::chrono::steady_clock::now() - window;
		std::vector<performance_alert> recent;

		std::copy_if(alerts_.begin(), alerts_.end(), std::back_inserter(recent),
			[cutoff](const performance_alert& alert) {
				return alert.timestamp() >= cutoff;
			});

		return recent;
	}

	void performance_monitor::clear_metrics()
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);
		query_history_.clear();
		connection_metrics_.clear();
		alerts_.clear();
		query_patterns_.clear();
		query_avg_times_.clear();
	}

	void performance_monitor::cleanup_old_metrics()
	{
		std::lock_guard<std::mutex> lock(metrics_mutex_);

		auto cutoff = std::chrono::steady_clock::now() - retention_period_;

		// Remove old query metrics
		query_history_.erase(
			std::remove_if(query_history_.begin(), query_history_.end(),
				[cutoff](const query_metrics& m) {
					return m.start_time < cutoff;
				}),
			query_history_.end());

		// Remove old alerts
		alerts_.erase(
			std::remove_if(alerts_.begin(), alerts_.end(),
				[cutoff](const performance_alert& alert) {
					return alert.timestamp() < cutoff;
				}),
			alerts_.end());
	}

	std::string performance_monitor::get_metrics_json() const
	{
		auto summary = get_performance_summary();

		std::ostringstream json;
		json << "{\n";
		json << "  \"total_queries\": " << summary.total_queries << ",\n";
		json << "  \"successful_queries\": " << summary.successful_queries << ",\n";
		json << "  \"failed_queries\": " << summary.failed_queries << ",\n";
		json << "  \"avg_query_time_us\": " << summary.avg_query_time.count() << ",\n";
		json << "  \"queries_per_second\": " << summary.queries_per_second << ",\n";
		json << "  \"error_rate\": " << summary.error_rate << ",\n";
		json << "  \"total_connections\": " << summary.total_connections << ",\n";
		json << "  \"active_connections\": " << summary.active_connections << ",\n";
		json << "  \"connection_utilization\": " << summary.connection_utilization << "\n";
		json << "}";

		return json.str();
	}

	void performance_monitor::cleanup_thread()
	{
		while (cleanup_running_) {
			std::unique_lock<std::mutex> lock(cleanup_mutex_);
			cleanup_cv_.wait_for(lock, std::chrono::minutes(5), [this] { return !cleanup_running_.load(); });

			if (cleanup_running_) {
				cleanup_old_metrics();
				check_thresholds();
			}
		}
	}

	void performance_monitor::check_thresholds()
	{
		auto summary = get_performance_summary();

		// Check error rate threshold
		if (summary.error_rate > error_rate_threshold_) {
			emit_alert(performance_alert::alert_type::high_error_rate,
			          "High error rate: " + std::to_string(summary.error_rate * 100) + "%");
		}

		// Check latency threshold
		if (summary.avg_query_time > latency_threshold_) {
			emit_alert(performance_alert::alert_type::high_latency,
			          "High average latency: " + std::to_string(summary.avg_query_time.count()) + "μs");
		}
	}

	void performance_monitor::emit_alert(performance_alert::alert_type type, const std::string& message)
	{
		performance_alert alert(type, message, std::chrono::steady_clock::now());

		{
			std::lock_guard<std::mutex> lock(metrics_mutex_);
			alerts_.push_back(alert);
		}

		// Notify alert handlers
		std::lock_guard<std::mutex> lock(handlers_mutex_);
		for (const auto& handler : alert_handlers_) {
			try {
				handler(alert);
			} catch (const std::exception& e) {
				std::cerr << "Alert handler exception: " << e.what() << std::endl;
			}
		}
	}

	std::string performance_monitor::calculate_query_hash(const std::string& query) const
	{
		return std::to_string(std::hash<std::string>{}(query));
	}

	// prometheus_exporter implementation
	prometheus_exporter::prometheus_exporter(const std::string& endpoint, int port)
		: endpoint_(endpoint), port_(port)
	{
	}

	bool prometheus_exporter::export_metrics(const performance_summary& summary)
	{
		// In a real implementation, this would send HTTP POST to Prometheus push gateway
		std::string metrics = format_prometheus_metrics(summary);
		std::cout << "Prometheus metrics:\n" << metrics << std::endl;
		return true;
	}

	bool prometheus_exporter::export_alerts(const std::vector<performance_alert>& alerts)
	{
		// Export alerts as metrics
		for (const auto& alert : alerts) {
			std::cout << "database_alert{type=\"" << static_cast<int>(alert.type())
			          << "\"} 1 " << std::chrono::duration_cast<std::chrono::milliseconds>(
			              alert.timestamp().time_since_epoch()).count() << std::endl;
		}
		return true;
	}

	std::string prometheus_exporter::format_prometheus_metrics(const performance_summary& summary) const
	{
		std::ostringstream metrics;

		metrics << "# HELP database_queries_total Total number of database queries\n";
		metrics << "# TYPE database_queries_total counter\n";
		metrics << "database_queries_total " << summary.total_queries << "\n";

		metrics << "# HELP database_query_duration_microseconds Average query duration in microseconds\n";
		metrics << "# TYPE database_query_duration_microseconds gauge\n";
		metrics << "database_query_duration_microseconds " << summary.avg_query_time.count() << "\n";

		metrics << "# HELP database_error_rate Query error rate\n";
		metrics << "# TYPE database_error_rate gauge\n";
		metrics << "database_error_rate " << summary.error_rate << "\n";

		metrics << "# HELP database_connections_active Active database connections\n";
		metrics << "# TYPE database_connections_active gauge\n";
		metrics << "database_connections_active " << summary.active_connections << "\n";

		return metrics.str();
	}

} // namespace database::monitoring