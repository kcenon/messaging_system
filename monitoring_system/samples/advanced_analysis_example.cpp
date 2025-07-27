/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <monitoring/multi_process_monitoring.h>
#include <monitoring/analysis_dashboard.h>
#include <thread>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <csignal>
#include <atomic>
#include <unistd.h>

using namespace monitoring_interface;
using namespace monitoring_module;

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nShutting down gracefully...\n";
        running = false;
    }
}

// 워크로드 시뮬레이터
class workload_simulator {
public:
    workload_simulator(std::shared_ptr<multi_process_monitoring> monitor)
        : monitor_(monitor) {
        
        // 프로세스 등록
        web_process_ = {
            .pid = static_cast<std::uint32_t>(getpid()),
            .process_name = "web_server",
            .start_time = std::chrono::steady_clock::now()
        };
        
        db_process_ = {
            .pid = static_cast<std::uint32_t>(getpid() + 1),
            .process_name = "database",
            .start_time = std::chrono::steady_clock::now()
        };
        
        monitor_->register_process(web_process_);
        monitor_->register_process(db_process_);
        
        // Thread pools 등록
        web_api_pool_ = {
            .process_id = web_process_,
            .pool_name = "api_handlers",
            .pool_instance_id = 1
        };
        
        web_static_pool_ = {
            .process_id = web_process_,
            .pool_name = "static_file_servers",
            .pool_instance_id = 1
        };
        
        db_query_pool_ = {
            .process_id = db_process_,
            .pool_name = "query_executors",
            .pool_instance_id = 1
        };
        
        db_maintenance_pool_ = {
            .process_id = db_process_,
            .pool_name = "maintenance_workers",
            .pool_instance_id = 1
        };
        
        monitor_->register_thread_pool(web_api_pool_);
        monitor_->register_thread_pool(web_static_pool_);
        monitor_->register_thread_pool(db_query_pool_);
        monitor_->register_thread_pool(db_maintenance_pool_);
    }
    
    void simulate_tick(int cycle) {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        // 시간대별 부하 패턴 시뮬레이션
        double load_factor = 1.0;
        if (cycle % 100 < 20) {
            load_factor = 0.5;  // 저부하
        } else if (cycle % 100 < 40) {
            load_factor = 1.5;  // 중부하
        } else if (cycle % 100 < 60) {
            load_factor = 2.5;  // 고부하
        } else if (cycle % 100 < 70) {
            load_factor = 3.0;  // 피크
        } else {
            load_factor = 1.0;  // 정상
        }
        
        // 이상 상황 시뮬레이션
        bool memory_leak = (cycle > 200 && cycle < 250);
        bool cpu_spike = (cycle % 150 == 0);
        bool queue_buildup = (cycle > 300 && cycle < 320);
        
        // Web server 메트릭
        system_metrics web_sys;
        web_sys.cpu_usage_percent = static_cast<std::uint64_t>(
            30 * load_factor + (cpu_spike ? 60 : 0) + (gen() % 10));
        web_sys.memory_usage_bytes = static_cast<std::uint64_t>(
            500 * 1024 * 1024 + (memory_leak ? cycle * 5 * 1024 * 1024 : 0));
        web_sys.active_threads = 20;
        monitor_->update_process_system_metrics(web_process_, web_sys);
        
        // Database 메트릭
        system_metrics db_sys;
        db_sys.cpu_usage_percent = static_cast<std::uint64_t>(
            40 * load_factor + (gen() % 15));
        db_sys.memory_usage_bytes = static_cast<std::uint64_t>(
            2 * 1024 * 1024 * 1024 + (gen() % (500 * 1024 * 1024)));
        db_sys.active_threads = 50;
        monitor_->update_process_system_metrics(db_process_, db_sys);
        
        // API handler pool
        process_thread_pool_metrics api_metrics;
        api_metrics.pool_id = web_api_pool_;
        api_metrics.worker_threads = 8;
        api_metrics.idle_threads = std::max(0, 8 - static_cast<int>(4 * load_factor));
        api_metrics.jobs_completed = static_cast<std::uint64_t>(100 * load_factor + gen() % 50);
        api_metrics.jobs_pending = queue_buildup ? 500 + cycle : static_cast<std::uint64_t>(10 * load_factor);
        api_metrics.jobs_failed = static_cast<std::uint64_t>(api_metrics.jobs_completed * 0.01);
        api_metrics.average_latency_ns = static_cast<std::uint64_t>(
            (50 + 20 * load_factor) * 1000000);  // ms to ns
        
        // 워커 부하 분산
        api_metrics.worker_load_distribution.resize(8);
        for (int i = 0; i < 8; ++i) {
            if (i < 2) {
                // 불균형 부하 시뮬레이션
                api_metrics.worker_load_distribution[i] = 
                    api_metrics.jobs_completed * 0.3 / 2;
            } else {
                api_metrics.worker_load_distribution[i] = 
                    api_metrics.jobs_completed * 0.7 / 6;
            }
        }
        monitor_->update_thread_pool_metrics(web_api_pool_, api_metrics);
        
        // Static file server pool
        process_thread_pool_metrics static_metrics;
        static_metrics.pool_id = web_static_pool_;
        static_metrics.worker_threads = 4;
        static_metrics.idle_threads = 2;
        static_metrics.jobs_completed = static_cast<std::uint64_t>(50 * load_factor);
        static_metrics.jobs_pending = 5;
        static_metrics.average_latency_ns = 10000000;  // 10ms
        monitor_->update_thread_pool_metrics(web_static_pool_, static_metrics);
        
        // Query executor pool
        process_thread_pool_metrics query_metrics;
        query_metrics.pool_id = db_query_pool_;
        query_metrics.worker_threads = 16;
        query_metrics.idle_threads = static_cast<std::uint64_t>(16 / load_factor);
        query_metrics.jobs_completed = static_cast<std::uint64_t>(200 * load_factor);
        query_metrics.jobs_pending = static_cast<std::uint64_t>(20 * load_factor);
        query_metrics.average_latency_ns = static_cast<std::uint64_t>(
            (100 + 50 * load_factor) * 1000000);
        monitor_->update_thread_pool_metrics(db_query_pool_, query_metrics);
        
        // Maintenance pool
        process_thread_pool_metrics maint_metrics;
        maint_metrics.pool_id = db_maintenance_pool_;
        maint_metrics.worker_threads = 2;
        maint_metrics.idle_threads = 1;
        maint_metrics.jobs_completed = 5;
        maint_metrics.jobs_pending = 0;
        maint_metrics.average_latency_ns = 5000000000;  // 5s
        monitor_->update_thread_pool_metrics(db_maintenance_pool_, maint_metrics);
    }
    
private:
    std::shared_ptr<multi_process_monitoring> monitor_;
    process_identifier web_process_;
    process_identifier db_process_;
    thread_pool_identifier web_api_pool_;
    thread_pool_identifier web_static_pool_;
    thread_pool_identifier db_query_pool_;
    thread_pool_identifier db_maintenance_pool_;
};

// 대시보드 렌더러
class dashboard_renderer {
public:
    static void render_header() {
        std::cout << "\033[2J\033[H";  // Clear screen
        std::cout << "\033[1;36m";     // Cyan bold
        std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║             Advanced Monitoring Analysis Dashboard                ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
        std::cout << "\033[0m";
    }
    
    static void render_health_section(const analysis_dashboard::system_health_report& health) {
        std::cout << "\n\033[1;33m▶ System Health\033[0m\n";
        
        // 건강도 바
        std::cout << "  Overall: ";
        render_health_bar(health.overall_health_score);
        std::cout << " " << std::fixed << std::setprecision(1) 
                  << health.overall_health_score << "% ";
        
        // 상태 색상
        if (health.health_status == "Excellent") {
            std::cout << "\033[32m";  // Green
        } else if (health.health_status == "Good") {
            std::cout << "\033[36m";  // Cyan
        } else if (health.health_status == "Fair") {
            std::cout << "\033[33m";  // Yellow
        } else {
            std::cout << "\033[31m";  // Red
        }
        std::cout << "[" << health.health_status << "]\033[0m\n";
        
        // 문제점
        if (!health.issues.empty()) {
            std::cout << "  \033[31mIssues:\033[0m\n";
            for (const auto& issue : health.issues) {
                std::cout << "    • " << issue << "\n";
            }
        }
    }
    
    static void render_forecast_section(const analysis_dashboard::performance_forecast& forecast) {
        std::cout << "\n\033[1;33m▶ Performance Forecast (5 min)\033[0m\n";
        
        for (const auto& [metric, prediction] : forecast.cpu_predictions) {
            std::cout << "  " << metric << ": ";
            std::cout << std::fixed << std::setprecision(1) 
                      << prediction.predicted_value << "% ";
            std::cout << "(±" << (prediction.confidence_upper - prediction.predicted_value) 
                      << "%)\n";
        }
        
        if (forecast.capacity_warning_time) {
            std::cout << "  \033[31m⚠ Capacity Warning: Resource exhaustion predicted!\033[0m\n";
        }
    }
    
    static void render_correlation_section(const analysis_dashboard::correlation_matrix& matrix) {
        if (!matrix.strong_correlations.empty()) {
            std::cout << "\n\033[1;33m▶ Strong Correlations\033[0m\n";
            for (const auto& [metric1, metric2] : matrix.strong_correlations) {
                std::cout << "  " << metric1 << " ↔ " << metric2 << "\n";
            }
        }
    }
    
    static void render_alerts_section(alert_manager& alert_mgr) {
        auto recent = alert_mgr.get_recent_alerts(3);
        if (!recent.empty()) {
            std::cout << "\n\033[1;33m▶ Recent Alerts\033[0m\n";
            for (const auto& alert : recent) {
                const char* color = "\033[0m";
                switch (alert.severity) {
                    case alert_manager::alert_severity::critical:
                    case alert_manager::alert_severity::emergency:
                        color = "\033[31m";  // Red
                        break;
                    case alert_manager::alert_severity::warning:
                        color = "\033[33m";  // Yellow
                        break;
                    default:
                        color = "\033[36m";  // Cyan
                }
                
                auto time_since = std::chrono::steady_clock::now() - alert.timestamp;
                auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since).count();
                
                std::cout << "  " << color << "• " << alert.message 
                          << " (" << seconds << "s ago)\033[0m\n";
            }
        }
    }
    
    static void render_optimization_section(const std::vector<std::pair<int, std::string>>& suggestions) {
        if (!suggestions.empty()) {
            std::cout << "\n\033[1;33m▶ Top Optimizations\033[0m\n";
            int count = 0;
            for (const auto& [priority, suggestion] : suggestions) {
                if (count++ >= 3) break;
                std::cout << "  " << count << ". " << suggestion << "\n";
            }
        }
    }
    
private:
    static void render_health_bar(double score) {
        int filled = static_cast<int>(score / 5);  // 20 segments
        std::cout << "[";
        
        const char* color;
        if (score >= 80) color = "\033[32m";      // Green
        else if (score >= 60) color = "\033[33m"; // Yellow
        else color = "\033[31m";                   // Red
        
        std::cout << color;
        for (int i = 0; i < 20; ++i) {
            std::cout << (i < filled ? "█" : "░");
        }
        std::cout << "\033[0m]";
    }
};

int main() {
    std::signal(SIGINT, signal_handler);
    
    std::cout << "Starting Advanced Monitoring Analysis Example\n";
    std::cout << "Press Ctrl+C to stop\n\n";
    
    // 모니터링 시스템 초기화
    auto monitor = std::make_shared<multi_process_monitoring>(
        1000,   // history_size
        100,    // collection_interval_ms
        10,     // max_processes
        10      // max_pools_per_process
    );
    monitor->start();
    
    // 분석 대시보드 설정
    analysis_dashboard::dashboard_config config;
    config.trend_window_size = 60;
    config.prediction_horizon = 300;
    config.anomaly_threshold = 2.5;
    config.enable_alerts = true;
    config.enable_predictions = true;
    config.enable_correlations = true;
    
    auto dashboard = std::make_unique<analysis_dashboard>(monitor, config);
    
    // 추가 알림 설정
    dashboard->get_alert_manager().add_condition({
        .name = "memory_growth",
        .condition = [](double value) { 
            static double last_value = 0;
            bool growing = value > last_value * 1.1;
            last_value = value;
            return growing && value > 1024 * 1024 * 1024;  // 1GB 이상이면서 증가중
        },
        .severity = alert_manager::alert_severity::warning,
        .message_template = "Memory usage growing rapidly",
        .cooldown = std::chrono::seconds(60)
    });
    
    // 워크로드 시뮬레이터
    workload_simulator simulator(monitor);
    
    int cycle = 0;
    auto last_render = std::chrono::steady_clock::now();
    
    while (running) {
        // 워크로드 시뮬레이션
        simulator.simulate_tick(cycle++);
        
        // 대시보드 렌더링 (1초마다)
        auto now = std::chrono::steady_clock::now();
        if (now - last_render >= std::chrono::seconds(1)) {
            dashboard_renderer::render_header();
            
            // 건강도 분석
            auto health = dashboard->generate_health_report();
            dashboard_renderer::render_health_section(health);
            
            // 예측
            auto forecast = dashboard->generate_forecast(std::chrono::seconds(300));
            dashboard_renderer::render_forecast_section(forecast);
            
            // 상관관계
            auto correlations = dashboard->analyze_correlations();
            dashboard_renderer::render_correlation_section(correlations);
            
            // 알림
            dashboard_renderer::render_alerts_section(dashboard->get_alert_manager());
            
            // 최적화 제안
            auto optimizations = dashboard->generate_optimization_suggestions();
            dashboard_renderer::render_optimization_section(optimizations);
            
            // 병목 현상
            auto bottlenecks = dashboard->analyze_bottlenecks();
            if (!bottlenecks.empty()) {
                std::cout << "\n\033[1;33m▶ Bottlenecks\033[0m\n";
                for (const auto& [pool_id, bottleneck] : bottlenecks) {
                    std::cout << "  " << pool_id.pool_name << ": " << bottleneck << "\n";
                }
            }
            
            // 이상 징후
            auto anomalies = dashboard->detect_real_time_anomalies();
            if (!anomalies.empty() && anomalies.size() < 5) {
                std::cout << "\n\033[1;33m▶ Anomalies Detected\033[0m\n";
                for (const auto& anomaly : anomalies) {
                    std::cout << "  \033[31m• " << anomaly.description << "\033[0m\n";
                }
            }
            
            std::cout << "\n\033[90mCycle: " << cycle 
                      << " | Press Ctrl+C to exit\033[0m" << std::flush;
            
            last_render = now;
        }
        
        // 메트릭 평가 (알림 생성)
        auto snapshot = monitor->get_multi_process_snapshot();
        for (const auto& [proc_id, sys_metrics] : snapshot.process_system_metrics) {
            dashboard->get_alert_manager().evaluate(
                proc_id.process_name + "_cpu", 
                static_cast<double>(sys_metrics.cpu_usage_percent));
            dashboard->get_alert_manager().evaluate(
                proc_id.process_name + "_memory", 
                static_cast<double>(sys_metrics.memory_usage_bytes));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    monitor->stop();
    
    // 최종 리포트
    std::cout << "\n\n=== Final Analysis Report ===\n";
    std::cout << dashboard->render_text_dashboard();
    
    // JSON 내보내기
    std::cout << "\n=== JSON Export ===\n";
    std::cout << dashboard->export_json() << "\n";
    
    return 0;
}