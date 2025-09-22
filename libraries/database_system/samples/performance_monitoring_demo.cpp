/**
 * BSD 3-Clause License
 * Copyright (c) 2025, Database System Project
 *
 * Performance Monitoring Demonstration
 * Shows real-time metrics collection, analysis, and alerting capabilities
 */

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <random>
#include "database/database_manager.h"
#include "database/monitoring/performance_monitor.h"

using namespace database;
using namespace database::monitoring;

void demonstrate_basic_metrics() {
    std::cout << "=== Basic Performance Metrics Demonstration ===\n";

    // Initialize performance monitor
    performance_monitor& monitor = performance_monitor::instance();

    // Configure monitoring settings
    monitoring_config config;
    config.enable_query_tracking = true;
    config.enable_connection_tracking = true;
    config.slow_query_threshold = std::chrono::milliseconds(100);
    config.alert_threshold_cpu = 80.0;
    config.alert_threshold_memory = 85.0;

    monitor.configure(config);
    std::cout << "Performance monitoring configured with:\n";
    std::cout << "  - Query tracking: enabled\n";
    std::cout << "  - Connection tracking: enabled\n";
    std::cout << "  - Slow query threshold: 100ms\n";
    std::cout << "  - CPU alert threshold: 80%\n";
    std::cout << "  - Memory alert threshold: 85%\n";

    // Get current system metrics
    const auto& system_metrics = monitor.get_system_metrics();
    std::cout << "\nCurrent System Metrics:\n";
    std::cout << "  CPU Usage: " << system_metrics.cpu_usage_percent << "%\n";
    std::cout << "  Memory Usage: " << system_metrics.memory_usage_percent << "%\n";
    std::cout << "  Disk I/O: " << system_metrics.disk_io_bytes_per_sec << " bytes/sec\n";
    std::cout << "  Network I/O: " << system_metrics.network_io_bytes_per_sec << " bytes/sec\n";
}

void demonstrate_query_metrics() {
    std::cout << "\n=== Query Performance Tracking ===\n";

    performance_monitor& monitor = performance_monitor::instance();

    // Simulate various database queries with different performance characteristics
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> query_time_dist(10, 200);
    std::uniform_int_distribution<> success_rate(1, 100);

    std::cout << "Simulating database queries...\n";

    for (int i = 0; i < 20; ++i) {
        std::string query_type = (i % 4 == 0) ? "SELECT" :
                                (i % 4 == 1) ? "INSERT" :
                                (i % 4 == 2) ? "UPDATE" : "DELETE";

        auto start_time = std::chrono::high_resolution_clock::now();

        // Simulate query execution time
        auto execution_time = std::chrono::milliseconds(query_time_dist(gen));
        std::this_thread::sleep_for(execution_time);

        auto end_time = std::chrono::high_resolution_clock::now();
        bool success = success_rate(gen) > 5; // 95% success rate

        // Record query metrics
        query_metrics metrics;
        metrics.query_type = query_type;
        metrics.execution_time = execution_time;
        metrics.success = success;
        metrics.rows_affected = success ? (i * 3 + 1) : 0;
        metrics.timestamp = start_time;

        monitor.record_query_execution(metrics);

        std::cout << "  Query " << (i+1) << " (" << query_type << "): "
                  << execution_time.count() << "ms, "
                  << (success ? "SUCCESS" : "FAILED") << "\n";
    }

    // Display aggregated query statistics
    std::cout << "\nQuery Performance Summary:\n";
    const auto& query_stats = monitor.get_query_statistics();

    std::cout << "  Total Queries: " << query_stats.total_queries << "\n";
    std::cout << "  Successful Queries: " << query_stats.successful_queries << "\n";
    std::cout << "  Failed Queries: " << query_stats.failed_queries << "\n";
    std::cout << "  Success Rate: " <<
        (100.0 * query_stats.successful_queries / query_stats.total_queries) << "%\n";
    std::cout << "  Average Execution Time: " <<
        query_stats.average_execution_time.count() << "ms\n";
    std::cout << "  Slow Queries Detected: " << query_stats.slow_queries_count << "\n";
}

void demonstrate_connection_pool_metrics() {
    std::cout << "\n=== Connection Pool Performance Monitoring ===\n";

    performance_monitor& monitor = performance_monitor::instance();

    // Simulate connection pool activity
    std::cout << "Simulating connection pool operations...\n";

    for (int i = 0; i < 15; ++i) {
        // Simulate getting connection from pool
        auto start_time = std::chrono::high_resolution_clock::now();

        // Simulate connection acquisition time
        std::this_thread::sleep_for(std::chrono::milliseconds(5 + (i % 3) * 10));

        auto end_time = std::chrono::high_resolution_clock::now();

        connection_metrics metrics;
        metrics.total_connections.store(20);
        metrics.active_connections.store(8 + (i % 5));
        metrics.idle_connections.store(metrics.total_connections.load() - metrics.active_connections.load());
        metrics.connections_created.store(25 + i);
        metrics.connections_destroyed.store(5 + (i / 5));
        metrics.connection_errors.store(i / 10);
        metrics.average_wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        metrics.peak_connections.store(std::max(static_cast<size_t>(20), metrics.active_connections.load()));

        monitor.record_connection_metrics(metrics);

        std::cout << "  Pool State " << (i+1) << ": "
                  << metrics.active_connections.load() << "/"
                  << metrics.total_connections.load() << " active, "
                  << "wait time: " << metrics.average_wait_time.count() << "ms\n";
    }

    // Display connection pool statistics
    std::cout << "\nConnection Pool Performance Summary:\n";
    const auto& pool_stats = monitor.get_connection_pool_statistics();

    std::cout << "  Pool Utilization: " << pool_stats.utilization_percentage << "%\n";
    std::cout << "  Average Wait Time: " << pool_stats.average_wait_time.count() << "ms\n";
    std::cout << "  Peak Connections: " << pool_stats.peak_connections << "\n";
    std::cout << "  Connection Errors: " << pool_stats.connection_errors << "\n";
    std::cout << "  Pool Efficiency Score: " << pool_stats.efficiency_score << "/100\n";
}

void demonstrate_real_time_monitoring() {
    std::cout << "\n=== Real-Time Performance Monitoring ===\n";

    performance_monitor& monitor = performance_monitor::instance();

    std::cout << "Starting real-time monitoring (5 seconds)...\n";

    auto start_time = std::chrono::steady_clock::now();
    int update_count = 0;

    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        update_count++;

        // Get real-time metrics
        const auto& system_metrics = monitor.get_system_metrics();
        const auto& alerts = monitor.get_active_alerts();

        std::cout << "\n[" << update_count << "s] Real-time Status:\n";
        std::cout << "  CPU: " << system_metrics.cpu_usage_percent << "%, "
                  << "Memory: " << system_metrics.memory_usage_percent << "%, "
                  << "Active Alerts: " << alerts.size() << "\n";

        // Display any active alerts
        for (const auto& alert : alerts) {
            std::cout << "  ⚠️  ALERT: " << alert.message
                      << " (Severity: " << static_cast<int>(alert.severity) << ")\n";
        }

        if (alerts.empty()) {
            std::cout << "  ✅ All systems normal\n";
        }
    }
}

void demonstrate_performance_analysis() {
    std::cout << "\n=== Performance Analysis and Reporting ===\n";

    performance_monitor& monitor = performance_monitor::instance();

    // Generate performance report
    std::cout << "Generating comprehensive performance report...\n";

    const auto& report = monitor.generate_performance_report();

    std::cout << "\n📊 Performance Report Summary:\n";
    std::cout << "  Report Period: " << report.report_period_minutes << " minutes\n";
    std::cout << "  Total Operations: " << report.total_operations << "\n";
    std::cout << "  Average Response Time: " << report.average_response_time.count() << "ms\n";
    std::cout << "  Peak Throughput: " << report.peak_throughput_ops_per_sec << " ops/sec\n";
    std::cout << "  Error Rate: " << report.error_rate_percentage << "%\n";

    std::cout << "\n📈 Top Performance Insights:\n";
    for (const auto& insight : report.performance_insights) {
        std::cout << "  • " << insight << "\n";
    }

    std::cout << "\n🔧 Optimization Recommendations:\n";
    for (const auto& recommendation : report.recommendations) {
        std::cout << "  → " << recommendation << "\n";
    }
}

void demonstrate_metrics_export() {
    std::cout << "\n=== Metrics Export for External Monitoring ===\n";

    performance_monitor& monitor = performance_monitor::instance();

    std::cout << "Exporting metrics in various formats...\n";

    // Export Prometheus metrics
    std::cout << "\n--- Prometheus Metrics Format ---\n";
    const auto prometheus_metrics = monitor.export_prometheus_metrics();
    std::cout << prometheus_metrics.substr(0, 300) << "...\n";

    // Export JSON metrics
    std::cout << "\n--- JSON Metrics Format ---\n";
    const auto json_metrics = monitor.export_json_metrics();
    std::cout << json_metrics.substr(0, 200) << "...\n";

    // Export CSV metrics for analysis
    std::cout << "\n--- CSV Export for Analysis ---\n";
    const auto csv_data = monitor.export_csv_metrics();
    std::cout << "CSV data exported: " << csv_data.size() << " bytes\n";
    std::cout << "First few lines:\n";
    auto first_newline = csv_data.find('\n', csv_data.find('\n') + 1);
    std::cout << csv_data.substr(0, first_newline) << "\n";

    std::cout << "\nMetrics can be integrated with:\n";
    std::cout << "  • Prometheus + Grafana for visualization\n";
    std::cout << "  • ELK Stack for log analysis\n";
    std::cout << "  • Custom monitoring dashboards\n";
    std::cout << "  • Third-party APM solutions\n";
}

void demonstrate_alerting_system() {
    std::cout << "\n=== Alerting and Notification System ===\n";

    performance_monitor& monitor = performance_monitor::instance();

    // Configure alert rules
    alert_rule cpu_rule;
    cpu_rule.metric_name = "cpu_usage_percent";
    cpu_rule.threshold_value = 75.0;
    cpu_rule.comparison = alert_comparison::greater_than;
    cpu_rule.severity = alert_severity::warning;
    cpu_rule.message = "High CPU usage detected";

    alert_rule memory_rule;
    memory_rule.metric_name = "memory_usage_percent";
    memory_rule.threshold_value = 90.0;
    memory_rule.comparison = alert_comparison::greater_than;
    memory_rule.severity = alert_severity::critical;
    memory_rule.message = "Critical memory usage level";

    monitor.add_alert_rule(cpu_rule);
    monitor.add_alert_rule(memory_rule);

    std::cout << "Configured alert rules:\n";
    std::cout << "  • CPU usage > 75% (Warning)\n";
    std::cout << "  • Memory usage > 90% (Critical)\n";

    // Simulate threshold breaches
    std::cout << "\nSimulating alert conditions...\n";

    // The monitoring system would automatically trigger alerts
    // based on real system metrics or simulated conditions
    std::cout << "Alert system is active and monitoring thresholds.\n";
    std::cout << "In production, alerts would be sent via:\n";
    std::cout << "  • Email notifications\n";
    std::cout << "  • Slack/Teams integration\n";
    std::cout << "  • SMS alerts for critical issues\n";
    std::cout << "  • Webhook notifications to external systems\n";
}

int main() {
    std::cout << "=== Performance Monitoring Framework Demonstration ===\n";
    std::cout << "This sample demonstrates comprehensive performance monitoring\n";
    std::cout << "capabilities for database operations and system resources.\n";

    try {
        demonstrate_basic_metrics();
        demonstrate_query_metrics();
        demonstrate_connection_pool_metrics();
        demonstrate_real_time_monitoring();
        demonstrate_performance_analysis();
        demonstrate_metrics_export();
        demonstrate_alerting_system();

        std::cout << "\n=== Performance Monitoring Features Summary ===\n";
        std::cout << "✓ Real-time system and database metrics collection\n";
        std::cout << "✓ Query performance tracking and analysis\n";
        std::cout << "✓ Connection pool monitoring and optimization\n";
        std::cout << "✓ Slow query detection and alerting\n";
        std::cout << "✓ Performance trend analysis and reporting\n";
        std::cout << "✓ Multi-format metrics export (Prometheus, JSON, CSV)\n";
        std::cout << "✓ Configurable alerting with multiple severity levels\n";
        std::cout << "✓ Integration with external monitoring systems\n";

        std::cout << "\nFor production deployment:\n";
        std::cout << "  performance_monitor::instance().configure(your_config);\n";
        std::cout << "  performance_monitor::instance().start_monitoring();\n";
        std::cout << "  // Metrics are automatically collected during database operations\n";

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}