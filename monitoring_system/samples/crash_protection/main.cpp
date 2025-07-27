/**
 * @file main.cpp
 * @brief Monitoring system crash protection demonstration
 * 
 * This example demonstrates comprehensive crash protection mechanisms
 * for monitoring systems including metrics preservation, real-time backup,
 * alert system protection, and automatic recovery of monitoring data.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <memory>
#include <atomic>
#include <fstream>

// Include monitoring and crash protection headers
#include "interfaces/monitoring_crash_safety.h"
#include "monitoring/monitoring.h"
#include "storage/ring_buffer.h"

using namespace monitoring_module;

// Global state for demonstration
std::atomic<bool> monitoring_active{true};
std::atomic<int> metrics_collected{0};
std::atomic<int> alerts_sent{0};
std::atomic<int> critical_snapshots{0};

// Simulated monitoring data
struct system_stats {
    double cpu_usage;
    uint64_t memory_usage;
    uint32_t active_threads;
    uint32_t network_connections;
};

// Simulated crash scenarios
void simulate_monitoring_crash() {
    std::cout << "[CRASH] Simulating monitoring system crash..." << std::endl;
    std::raise(SIGINT);
}

void simulate_ring_buffer_overflow() {
    std::cout << "[CRASH] Simulating ring buffer overflow..." << std::endl;
    // This will be handled by overflow protection
}

void simulate_alert_system_failure() {
    std::cout << "[CRASH] Simulating alert system failure..." << std::endl;
    std::raise(SIGUSR1);
}

// Monitoring task functions
void metrics_collection_task(int collector_id, std::shared_ptr<monitoring> monitor) {
    std::cout << "[METRICS] Metrics collector " << collector_id << " starting" << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> cpu_dist(50.0, 15.0);
    std::uniform_int_distribution<> memory_dist(100, 800);
    std::uniform_int_distribution<> threads_dist(10, 50);
    
    for (int i = 0; i < 20; ++i) {
        system_metrics sys_metrics;
        sys_metrics.cpu_usage_percent = static_cast<uint8_t>(
            std::max(0.0, std::min(100.0, cpu_dist(gen))));
        sys_metrics.memory_usage_bytes = memory_dist(gen) * 1024 * 1024;
        sys_metrics.active_threads = threads_dist(gen);
        
        monitor->update_system_metrics(sys_metrics);
        metrics_collected.fetch_add(1);
        
        // Preserve critical metrics
        critical_metrics_snapshot snapshot;
        snapshot.timestamp = std::chrono::system_clock::now();
        snapshot.cpu_usage_percent = sys_metrics.cpu_usage_percent;
        snapshot.memory_usage_bytes = sys_metrics.memory_usage_bytes;
        snapshot.active_threads = sys_metrics.active_threads;
        snapshot.crash_context = "Collector_" + std::to_string(collector_id);
        
        monitoring_crash_safety::instance().preserve_critical_metrics(snapshot);
        critical_snapshots.fetch_add(1);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "[OK] Metrics collector " << collector_id << " completed" << std::endl;
}

void thread_pool_monitoring_task(int monitor_id) {
    std::cout << "[THREAD] Thread pool monitor " << monitor_id << " starting" << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> pending_dist(0, 50);
    std::uniform_int_distribution<> completed_dist(100, 500);
    std::uniform_int_distribution<> failed_dist(0, 10);
    std::uniform_int_distribution<> latency_dist(1000, 50000); // nanoseconds
    
    for (int i = 0; i < 15; ++i) {
        thread_pool_metrics pool_metrics;
        pool_metrics.worker_threads = 4;
        pool_metrics.jobs_pending = pending_dist(gen);
        pool_metrics.jobs_completed = completed_dist(gen);
        pool_metrics.jobs_failed = failed_dist(gen);
        pool_metrics.average_latency_ns = latency_dist(gen);
        
        // Simulate monitoring interface update
        // monitor->update_thread_pool_metrics(pool_metrics);
        metrics_collected.fetch_add(1);
        
        // Check for alert conditions
        if (pool_metrics.jobs_pending > 40) {
            std::cout << "[ALERT] HIGH QUEUE ALERT: " << pool_metrics.jobs_pending << " jobs pending" << std::endl;
            alerts_sent.fetch_add(1);
        }
        
        if (pool_metrics.average_latency_ns > 40000) {
            std::cout << "[ALERT] HIGH LATENCY ALERT: " << pool_metrics.average_latency_ns << "ns" << std::endl;
            alerts_sent.fetch_add(1);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    
    std::cout << "[OK] Thread pool monitor " << monitor_id << " completed" << std::endl;
}

void risky_monitoring_task(int task_id) {
    std::cout << "[WARN] Risky monitoring task " << task_id << " starting" << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> crash_dist(1, 10);
    
    for (int i = 0; i < 8; ++i) {
        // Collect some metrics
        critical_metrics_snapshot snapshot;
        snapshot.timestamp = std::chrono::system_clock::now();
        snapshot.cpu_usage_percent = 85 + i * 2; // High CPU
        snapshot.memory_usage_bytes = (500 + i * 50) * 1024 * 1024;
        snapshot.active_threads = 30 + i;
        snapshot.crash_context = "RiskyTask_" + std::to_string(task_id);
        
        monitoring_crash_safety::instance().preserve_critical_metrics(snapshot);
        metrics_collected.fetch_add(1);
        
        // Random chance of issues
        int outcome = crash_dist(gen);
        if (outcome <= 6) {
            // Normal execution
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        } else if (outcome == 7) {
            // Simulate buffer overflow
            simulate_ring_buffer_overflow();
        } else if (outcome == 8) {
            // Simulate alert system failure
            simulate_alert_system_failure();
            break;
        } else {
            // Simulate full monitoring crash
            simulate_monitoring_crash();
            break;
        }
    }
    
    std::cout << "[WARN] Risky monitoring task " << task_id << " finished" << std::endl;
}

// Crash callback functions
void on_monitoring_component_crash(const std::string& component_name) {
    std::cout << "\n[ALERT] MONITORING COMPONENT CRASH: " << component_name << std::endl;
    monitoring_active.store(false);
    
    // Preserve current state
    monitoring_crash_safety::instance().force_save_all_states();
}

void on_ring_buffer_overflow(size_t lost_entries) {
    std::cout << "[WARN] Ring buffer overflow: " << lost_entries << " entries lost" << std::endl;
    
    // Create emergency snapshot
    std::vector<uint8_t> emergency_data(1024, 0xFF); // Dummy data
    ring_buffer_crash_safety::create_emergency_snapshot("MainBuffer", emergency_data);
}

void on_alert_system_crash(const std::string& crash_info) {
    std::cout << "[ALERT] ALERT SYSTEM CRASH: " << crash_info << std::endl;
    
    // Send immediate crash notification
    std::cout << "[NOTIFY] Sending emergency crash notification to administrators" << std::endl;
}

// State save/restore functions
std::string save_monitoring_state() {
    std::cout << "[SAVE] Saving monitoring component state" << std::endl;
    return "metrics_collected=" + std::to_string(metrics_collected.load()) +
           ";alerts_sent=" + std::to_string(alerts_sent.load()) +
           ";critical_snapshots=" + std::to_string(critical_snapshots.load());
}

void restore_monitoring_state(const std::string& state) {
    std::cout << "[RESTORE] Restoring monitoring component state: " << state << std::endl;
    // Parse and restore state (simplified for demo)
}

std::string save_collector_state() {
    std::cout << "[SAVE] Saving metrics collector state" << std::endl;
    return "active=true;last_collection=" + 
           std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}

void restore_collector_state(const std::string& state) {
    std::cout << "[RESTORE] Restoring metrics collector state: " << state << std::endl;
}

int main() {
    std::cout << "=== Monitoring System Crash Protection Demo ===" << std::endl;
    std::cout << "This demo shows comprehensive monitoring crash protection mechanisms\n" << std::endl;
    
    // Step 1: Initialize monitoring crash protection
    std::cout << "--- Step 1: Initialize Monitoring Crash Protection ---" << std::endl;
    
    auto& monitor_safety = monitoring_crash_safety::instance();
    monitor_safety.initialize(monitoring_crash_safety_level::standard,
                            "./monitoring_backup.dat", 1500);
    
    monitor_safety.set_realtime_backup(true, 2000);
    monitor_safety.set_max_critical_snapshots(200);
    
    std::cout << "[OK] Monitoring crash protection initialized" << std::endl;
    
    // Step 2: Register monitoring components with crash protection
    std::cout << "\n--- Step 2: Register Monitoring Components ---" << std::endl;
    
    {
        scoped_monitoring_crash_protection main_monitor_protection(
            "MainMonitor", save_monitoring_state, restore_monitoring_state);
        
        scoped_monitoring_crash_protection collector_protection(
            "MetricsCollector", save_collector_state, restore_collector_state);
        
        std::cout << "[OK] Monitoring components registered for crash protection" << std::endl;
        
        // Step 3: Configure specialized crash safety features
        std::cout << "\n--- Step 3: Configure Specialized Protection ---" << std::endl;
        
        // Configure ring buffer safety
        ring_buffer_crash_safety::configure_ring_buffer_safety("MainBuffer", true, 0.85);
        ring_buffer_crash_safety::set_overflow_handler("MainBuffer", on_ring_buffer_overflow);
        
        // Configure alert system safety
        alert_system_crash_safety::configure_alert_safety("MainAlerts", true, true);
        alert_system_crash_safety::set_crash_alert_handler(on_alert_system_crash);
        
        // Configure metrics collection safety
        metrics_collection_crash_safety::configure_collector_safety("MainCollector", true, true);
        
        std::cout << "[OK] Specialized protection configured" << std::endl;
        
        // Step 4: Create monitoring system
        std::cout << "\n--- Step 4: Create Monitoring System ---" << std::endl;
        
        auto monitor = std::make_shared<monitoring>(100, 200); // 100 snapshots, 200ms interval
        monitor->start();
        
        std::cout << "[OK] Monitoring system started" << std::endl;
        
        // Step 5: Normal monitoring operations
        std::cout << "\n--- Step 5: Normal Monitoring Operations ---" << std::endl;
        
        std::vector<std::thread> monitoring_threads;
        
        // Start metrics collection threads
        for (int i = 0; i < 3; ++i) {
            monitoring_threads.emplace_back(metrics_collection_task, i, monitor);
        }
        
        // Start thread pool monitoring
        for (int i = 0; i < 2; ++i) {
            monitoring_threads.emplace_back(thread_pool_monitoring_task, i);
        }
        
        // Wait for normal operations
        for (auto& t : monitoring_threads) {
            t.join();
        }
        monitoring_threads.clear();
        
        std::cout << "[OK] Normal monitoring operations completed" << std::endl;
        
        // Step 6: Test critical metrics preservation
        std::cout << "\n--- Step 6: Critical Metrics Preservation Test ---" << std::endl;
        
        // Generate critical scenarios
        for (int i = 0; i < 10; ++i) {
            critical_metrics_snapshot critical;
            critical.timestamp = std::chrono::system_clock::now();
            critical.cpu_usage_percent = 95 + i; // Very high CPU
            critical.memory_usage_bytes = (900 + i * 10) * 1024 * 1024; // High memory
            critical.active_threads = 100 + i;
            critical.jobs_pending = 200 + i * 10;
            critical.crash_context = "CriticalScenario_" + std::to_string(i);
            
            monitor_safety.preserve_critical_metrics(critical);
        }
        
        std::cout << "[OK] Critical metrics preserved" << std::endl;
        
        // Step 7: Test alert system with crashes
        std::cout << "\n--- Step 7: Alert System Crash Test ---" << std::endl;
        
        // Preserve some pending alerts
        std::vector<std::string> pending_alerts = {
            "High CPU usage detected: 95%",
            "Memory usage critical: 950MB",
            "Thread pool queue full: 250 jobs",
            "Network latency high: 500ms"
        };
        alert_system_crash_safety::preserve_pending_alerts(pending_alerts);
        
        std::cout << "[OK] Alerts preserved for crash recovery" << std::endl;
        
        // Step 8: Risky monitoring operations
        std::cout << "\n--- Step 8: Risky Monitoring Operations ---" << std::endl;
        std::cout << "[WARN] Some operations may trigger crash protection" << std::endl;
        
        // Submit risky tasks
        for (int i = 20; i < 25; ++i) {
            monitoring_threads.emplace_back(risky_monitoring_task, i);
        }
        
        // Wait for risky operations
        for (auto& t : monitoring_threads) {
            t.join();
        }
        
        // Step 9: Force backup and save operations
        std::cout << "\n--- Step 9: Force Backup Operations ---" << std::endl;
        
        monitor_safety.force_save_all_states();
        
        // Simulate system stress
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::cout << "[OK] Backup operations completed" << std::endl;
        
        monitor->stop();
        
    } // scoped protection objects go out of scope here
    
    // Step 10: Recovery testing
    std::cout << "\n--- Step 10: Recovery Testing ---" << std::endl;
    
    if (monitor_safety.check_and_recover()) {
        std::cout << "[OK] Recovery actions were taken" << std::endl;
        
        // Restore preserved alerts
        auto restored_alerts = alert_system_crash_safety::restore_preserved_alerts();
        std::cout << "[RESTORE] Restored " << restored_alerts.size() << " alerts" << std::endl;
        for (const auto& alert : restored_alerts) {
            std::cout << "  - " << alert << std::endl;
        }
    } else {
        std::cout << "[INFO] No recovery needed" << std::endl;
    }
    
    // Step 11: Display preserved critical metrics
    std::cout << "\n--- Step 11: Preserved Critical Metrics ---" << std::endl;
    
    auto preserved_metrics = monitor_safety.get_preserved_metrics();
    std::cout << "[METRICS] Preserved " << preserved_metrics.size() << " critical metric snapshots" << std::endl;
    
    if (!preserved_metrics.empty()) {
        std::cout << "Last 5 critical snapshots:" << std::endl;
        size_t count = std::min(size_t(5), preserved_metrics.size());
        for (size_t i = preserved_metrics.size() - count; i < preserved_metrics.size(); ++i) {
            const auto& metric = preserved_metrics[i];
            std::cout << "  " << metric.crash_context 
                      << ": CPU=" << metric.cpu_usage_percent << "% "
                      << "MEM=" << metric.memory_usage_bytes / (1024*1024) << "MB "
                      << "THREADS=" << metric.active_threads << std::endl;
        }
    }
    
    // Step 12: Display crash protection statistics
    std::cout << "\n--- Step 12: Crash Protection Statistics ---" << std::endl;
    
    auto stats = monitor_safety.get_stats();
    std::cout << "Monitoring Safety Statistics:" << std::endl;
    std::cout << "  Total backups created: " << stats.total_backups_created << std::endl;
    std::cout << "  Successful saves: " << stats.successful_saves << std::endl;
    std::cout << "  Failed saves: " << stats.failed_saves << std::endl;
    std::cout << "  Successful restores: " << stats.successful_restores << std::endl;
    std::cout << "  Failed restores: " << stats.failed_restores << std::endl;
    std::cout << "  Critical snapshots preserved: " << stats.critical_snapshots_preserved << std::endl;
    
    std::cout << "\nApplication Statistics:" << std::endl;
    std::cout << "  Metrics collected: " << metrics_collected.load() << std::endl;
    std::cout << "  Alerts sent: " << alerts_sent.load() << std::endl;
    std::cout << "  Critical snapshots: " << critical_snapshots.load() << std::endl;
    std::cout << "  Monitoring active: " << (monitoring_active.load() ? "Yes" : "No") << std::endl;
    
    std::cout << "\n=== Demo Completed Successfully ===" << std::endl;
    std::cout << "Key features demonstrated:" << std::endl;
    std::cout << "[OK] Real-time metrics backup and preservation" << std::endl;
    std::cout << "[OK] Critical metrics snapshot protection" << std::endl;
    std::cout << "[OK] Ring buffer overflow protection" << std::endl;
    std::cout << "[OK] Alert system crash safety" << std::endl;
    std::cout << "[OK] Monitoring component state save/restore" << std::endl;
    std::cout << "[OK] Automatic recovery mechanisms" << std::endl;
    std::cout << "[OK] RAII-based crash protection registration" << std::endl;
    std::cout << "[OK] Metrics collection crash safety" << std::endl;
    
    return 0;
}