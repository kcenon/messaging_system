/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "analysis_dashboard.h"
#include <iomanip>
#include <algorithm>

namespace monitoring_module {

analysis_dashboard::analysis_dashboard(
    std::shared_ptr<multi_process_monitoring> monitor,
    const dashboard_config& config)
    : monitor_(monitor)
    , config_(config) {
    
    if (config_.enable_alerts) {
        setup_default_alerts();
    }
}

void analysis_dashboard::setup_default_alerts() {
    // CPU 사용률 알림
    alert_manager_.add_condition({
        .name = "high_cpu_usage",
        .condition = [](double value) { return value > 80.0; },
        .severity = alert_manager::alert_severity::warning,
        .message_template = "High CPU usage: {value}%",
        .cooldown = std::chrono::seconds(300)
    });
    
    alert_manager_.add_condition({
        .name = "critical_cpu_usage",
        .condition = [](double value) { return value > 95.0; },
        .severity = alert_manager::alert_severity::critical,
        .message_template = "Critical CPU usage: {value}%",
        .cooldown = std::chrono::seconds(60)
    });
    
    // 메모리 사용률 알림
    alert_manager_.add_condition({
        .name = "high_memory_usage",
        .condition = [](double value) { return value > 80.0; },
        .severity = alert_manager::alert_severity::warning,
        .message_template = "High memory usage: {value}%",
        .cooldown = std::chrono::seconds(300)
    });
    
    // 큐 포화 알림
    alert_manager_.add_condition({
        .name = "queue_saturation",
        .condition = [](double value) { return value > 90.0; },
        .severity = alert_manager::alert_severity::warning,
        .message_template = "Queue saturation: {value}%",
        .cooldown = std::chrono::seconds(120)
    });
    
    // 응답 시간 알림
    alert_manager_.add_condition({
        .name = "high_latency",
        .condition = [](double value) { return value > 1000.0; },  // 1초
        .severity = alert_manager::alert_severity::warning,
        .message_template = "High latency: {value}ms",
        .cooldown = std::chrono::seconds(180)
    });
}

analysis_dashboard::system_health_report 
analysis_dashboard::generate_health_report() {
    system_health_report report;
    report.report_time = std::chrono::steady_clock::now();
    
    update_metric_cache();
    
    auto snapshot = monitor_->get_multi_process_snapshot();
    double total_score = 0.0;
    int process_count = 0;
    
    // 프로세스별 건강도 계산
    for (const auto& [proc_id, sys_metrics] : snapshot.process_system_metrics) {
        double proc_score = calculate_process_health_score(proc_id);
        total_score += proc_score;
        process_count++;
        
        if (proc_score < 50) {
            report.issues.push_back(
                "Process " + proc_id.process_name + " has poor health score: " + 
                std::to_string(static_cast<int>(proc_score)));
        }
        
        // CPU 확인
        if (sys_metrics.cpu_usage_percent > 80) {
            report.warnings.push_back(
                "Process " + proc_id.process_name + " has high CPU usage: " +
                std::to_string(sys_metrics.cpu_usage_percent) + "%");
        }
        
        // 메모리 확인
        if (sys_metrics.memory_usage_bytes > 1024 * 1024 * 1024) { // 1GB
            report.warnings.push_back(
                "Process " + proc_id.process_name + " uses significant memory: " +
                std::to_string(sys_metrics.memory_usage_bytes / (1024 * 1024)) + " MB");
        }
    }
    
    // Thread pool별 건강도
    for (const auto& [pool_id, pool_metrics] : snapshot.thread_pool_metrics_map) {
        // double pool_score = calculate_pool_health_score(pool_id);
        
        auto bottleneck = thread_pool_analyzer::detect_bottleneck(pool_metrics);
        if (bottleneck) {
            report.issues.push_back(
                "Pool " + pool_id.pool_name + ": " + *bottleneck);
        }
        
        auto suggestions = thread_pool_analyzer::suggest_optimizations(pool_metrics);
        for (const auto& suggestion : suggestions) {
            report.recommendations.push_back(
                "Pool " + pool_id.pool_name + ": " + suggestion);
        }
    }
    
    // 전체 건강도 점수 계산
    report.overall_health_score = process_count > 0 ? total_score / process_count : 0.0;
    
    // 건강 상태 판정
    if (report.overall_health_score >= 90) {
        report.health_status = "Excellent";
    } else if (report.overall_health_score >= 75) {
        report.health_status = "Good";
    } else if (report.overall_health_score >= 60) {
        report.health_status = "Fair";
    } else if (report.overall_health_score >= 40) {
        report.health_status = "Poor";
    } else {
        report.health_status = "Critical";
    }
    
    return report;
}

analysis_dashboard::performance_forecast 
analysis_dashboard::generate_forecast(std::chrono::seconds duration) {
    performance_forecast forecast;
    
    if (!config_.enable_predictions) {
        return forecast;
    }
    
    update_metric_cache();
    
    // CPU 예측
    for (const auto& [metric_name, history] : metric_cache_) {
        if (metric_name.find("cpu") != std::string::npos && history.size() >= 10) {
            auto predictions = trend_analyzer::predict_linear(
                history, duration, 5);
            if (!predictions.empty()) {
                forecast.cpu_predictions[metric_name] = predictions.back();
            }
        }
    }
    
    // 메모리 예측
    for (const auto& [metric_name, history] : metric_cache_) {
        if (metric_name.find("memory") != std::string::npos && history.size() >= 10) {
            auto predictions = trend_analyzer::predict_linear(
                history, duration, 5);
            if (!predictions.empty()) {
                forecast.memory_predictions[metric_name] = predictions.back();
                
                // 용량 경고 확인 (예: 4GB 한계)
                auto exhaustion = trend_analyzer::predict_capacity_exhaustion(
                    history, 4.0 * 1024 * 1024 * 1024);
                if (exhaustion) {
                    forecast.capacity_warning_time = exhaustion;
                    forecast.risk_factors.push_back(
                        "Memory exhaustion predicted for " + metric_name);
                }
            }
        }
    }
    
    // 처리량 예측
    for (const auto& [metric_name, history] : metric_cache_) {
        if (metric_name.find("throughput") != std::string::npos && history.size() >= 10) {
            auto predictions = trend_analyzer::predict_linear(
                history, duration, 5);
            if (!predictions.empty()) {
                forecast.throughput_predictions[metric_name] = predictions.back();
            }
        }
    }
    
    return forecast;
}

analysis_dashboard::correlation_matrix 
analysis_dashboard::analyze_correlations() {
    correlation_matrix matrix;
    
    if (!config_.enable_correlations) {
        return matrix;
    }
    
    update_metric_cache();
    
    // 메트릭 이름 수집
    for (const auto& [name, _] : metric_cache_) {
        matrix.metric_names.push_back(name);
    }
    
    // 상관관계 계산
    matrix.correlations.resize(matrix.metric_names.size(),
                              std::vector<double>(matrix.metric_names.size(), 0.0));
    
    for (size_t i = 0; i < matrix.metric_names.size(); ++i) {
        for (size_t j = 0; j < matrix.metric_names.size(); ++j) {
            if (i == j) {
                matrix.correlations[i][j] = 1.0;
            } else {
                const auto& series1 = metric_cache_[matrix.metric_names[i]];
                const auto& series2 = metric_cache_[matrix.metric_names[j]];
                
                if (series1.size() == series2.size() && series1.size() >= 10) {
                    double correlation = trend_analyzer::calculate_correlation(
                        series1, series2);
                    matrix.correlations[i][j] = correlation;
                    
                    // 강한 상관관계 기록
                    if (std::abs(correlation) > 0.7 && i < j) {
                        matrix.strong_correlations.push_back(
                            {matrix.metric_names[i], matrix.metric_names[j]});
                    }
                }
            }
        }
    }
    
    return matrix;
}

std::vector<trend_analyzer::anomaly_result> 
analysis_dashboard::detect_real_time_anomalies() {
    std::vector<trend_analyzer::anomaly_result> all_anomalies;
    
    update_metric_cache();
    
    for (const auto& [metric_name, history] : metric_cache_) {
        auto anomalies = trend_analyzer::detect_anomalies(
            history, config_.anomaly_threshold);
        all_anomalies.insert(all_anomalies.end(), 
                           anomalies.begin(), anomalies.end());
    }
    
    return all_anomalies;
}

std::unordered_map<monitoring_interface::thread_pool_identifier, std::string> 
analysis_dashboard::analyze_bottlenecks() {
    std::unordered_map<monitoring_interface::thread_pool_identifier, std::string> bottlenecks;
    
    auto snapshot = monitor_->get_multi_process_snapshot();
    
    for (const auto& [pool_id, pool_metrics] : snapshot.thread_pool_metrics_map) {
        auto bottleneck = thread_pool_analyzer::detect_bottleneck(pool_metrics);
        if (bottleneck) {
            bottlenecks[pool_id] = *bottleneck;
        }
    }
    
    return bottlenecks;
}

std::vector<std::pair<int, std::string>> 
analysis_dashboard::generate_optimization_suggestions() {
    std::vector<std::pair<int, std::string>> prioritized_suggestions;
    
    auto snapshot = monitor_->get_multi_process_snapshot();
    
    // Thread pool 최적화
    for (const auto& [pool_id, pool_metrics] : snapshot.thread_pool_metrics_map) {
        auto suggestions = thread_pool_analyzer::suggest_optimizations(pool_metrics);
        auto pool_health = thread_pool_analyzer::calculate_health_score(pool_metrics);
        
        for (const auto& suggestion : suggestions) {
            int priority = 100 - static_cast<int>(pool_health);  // 건강도가 낮을수록 우선순위 높음
            prioritized_suggestions.push_back(
                {priority, pool_id.pool_name + ": " + suggestion});
        }
    }
    
    // 시스템 레벨 최적화
    if (snapshot.global_system.cpu_usage_percent > 80) {
        prioritized_suggestions.push_back(
            {90, "System: Consider scaling out - high global CPU usage"});
    }
    
    if (snapshot.global_system.memory_usage_bytes > static_cast<std::uint64_t>(3) * 1024 * 1024 * 1024) {
        prioritized_suggestions.push_back(
            {85, "System: Memory usage is high - consider memory optimization"});
    }
    
    // 우선순위별 정렬
    std::sort(prioritized_suggestions.begin(), prioritized_suggestions.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    return prioritized_suggestions;
}

std::unordered_map<std::string, std::optional<std::chrono::steady_clock::time_point>>
analysis_dashboard::analyze_capacity_planning() {
    std::unordered_map<std::string, std::optional<std::chrono::steady_clock::time_point>> capacity_map;
    
    update_metric_cache();
    
    // 메모리 용량 분석
    for (const auto& [metric_name, history] : metric_cache_) {
        if (metric_name.find("memory") != std::string::npos) {
            auto exhaustion = trend_analyzer::predict_capacity_exhaustion(
                history, 4.0 * 1024 * 1024 * 1024);  // 4GB 한계
            capacity_map[metric_name] = exhaustion;
        }
    }
    
    // 큐 용량 분석
    for (const auto& [metric_name, history] : metric_cache_) {
        if (metric_name.find("queue") != std::string::npos) {
            auto exhaustion = trend_analyzer::predict_capacity_exhaustion(
                history, 10000);  // 큐 크기 한계
            capacity_map[metric_name] = exhaustion;
        }
    }
    
    return capacity_map;
}

std::string analysis_dashboard::render_text_dashboard() {
    std::stringstream ss;
    
    ss << "\n===== System Analysis Dashboard =====\n";
    ss << "Timestamp: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";
    
    // 건강도 리포트
    auto health_report = generate_health_report();
    ss << "=== System Health ===\n";
    ss << "Overall Score: " << std::fixed << std::setprecision(1) 
       << health_report.overall_health_score << "/100\n";
    ss << "Status: " << health_report.health_status << "\n";
    
    if (!health_report.issues.empty()) {
        ss << "\nIssues:\n";
        for (const auto& issue : health_report.issues) {
            ss << "  - " << issue << "\n";
        }
    }
    
    if (!health_report.warnings.empty()) {
        ss << "\nWarnings:\n";
        for (const auto& warning : health_report.warnings) {
            ss << "  - " << warning << "\n";
        }
    }
    
    // 병목 현상
    auto bottlenecks = analyze_bottlenecks();
    if (!bottlenecks.empty()) {
        ss << "\n=== Bottlenecks Detected ===\n";
        for (const auto& [pool_id, bottleneck] : bottlenecks) {
            ss << pool_id.pool_name << ": " << bottleneck << "\n";
        }
    }
    
    // 이상 징후
    auto anomalies = detect_real_time_anomalies();
    if (!anomalies.empty()) {
        ss << "\n=== Anomalies Detected ===\n";
        for (const auto& anomaly : anomalies) {
            ss << anomaly.severity << ": " << anomaly.description << "\n";
        }
    }
    
    // 최적화 제안
    auto suggestions = generate_optimization_suggestions();
    if (!suggestions.empty()) {
        ss << "\n=== Optimization Suggestions ===\n";
        int count = 0;
        for (const auto& [priority, suggestion] : suggestions) {
            if (count++ >= 5) break;  // 상위 5개만
            ss << "[P" << priority << "] " << suggestion << "\n";
        }
    }
    
    // 예측
    if (config_.enable_predictions) {
        auto forecast = generate_forecast(std::chrono::seconds(300));
        if (forecast.capacity_warning_time) {
            ss << "\n=== Capacity Warning ===\n";
            ss << "Resource exhaustion predicted!\n";
        }
    }
    
    // 최근 알림
    auto recent_alerts = alert_manager_.get_recent_alerts(5);
    if (!recent_alerts.empty()) {
        ss << "\n=== Recent Alerts ===\n";
        for (const auto& alert : recent_alerts) {
            ss << "[" << severity_to_string(alert.severity) << "] " 
               << alert.message << "\n";
        }
    }
    
    ss << "\n=====================================\n";
    
    return ss.str();
}

std::string analysis_dashboard::export_json() {
    std::stringstream json;
    json << "{\n";
    
    // 건강도
    auto health = generate_health_report();
    json << "  \"health\": {\n";
    json << "    \"score\": " << health.overall_health_score << ",\n";
    json << "    \"status\": \"" << health.health_status << "\",\n";
    json << "    \"issues\": " << health.issues.size() << ",\n";
    json << "    \"warnings\": " << health.warnings.size() << "\n";
    json << "  },\n";
    
    // 메트릭 스냅샷
    auto snapshot = monitor_->get_multi_process_snapshot();
    json << "  \"metrics\": {\n";
    json << "    \"processes\": " << snapshot.process_system_metrics.size() << ",\n";
    json << "    \"thread_pools\": " << snapshot.thread_pool_metrics_map.size() << ",\n";
    json << "    \"global_cpu\": " << snapshot.global_system.cpu_usage_percent << ",\n";
    json << "    \"global_memory_mb\": " 
         << (snapshot.global_system.memory_usage_bytes / (1024.0 * 1024.0)) << "\n";
    json << "  },\n";
    
    // 알림
    auto alert_stats = alert_manager_.get_alert_statistics();
    json << "  \"alerts\": {\n";
    json << "    \"info\": " << alert_stats[alert_manager::alert_severity::info] << ",\n";
    json << "    \"warning\": " << alert_stats[alert_manager::alert_severity::warning] << ",\n";
    json << "    \"critical\": " << alert_stats[alert_manager::alert_severity::critical] << ",\n";
    json << "    \"emergency\": " << alert_stats[alert_manager::alert_severity::emergency] << "\n";
    json << "  }\n";
    
    json << "}";
    
    return json.str();
}

std::vector<trend_analyzer::trend_point> 
analysis_dashboard::collect_metric_history(
    const std::string& metric_name,
    std::chrono::seconds duration) {
    
    // 실제 구현에서는 모니터링 시스템에서 히스토리를 가져옴
    // 여기서는 시뮬레이션
    std::vector<trend_analyzer::trend_point> history;
    
    auto now = std::chrono::steady_clock::now();
    auto start = now - duration;
    auto step = duration / 60;  // 60개 데이터 포인트
    
    for (int i = 0; i < 60; ++i) {
        trend_analyzer::trend_point point;
        point.timestamp = start + (step * i);
        
        // 시뮬레이션된 값
        if (metric_name.find("cpu") != std::string::npos) {
            point.value = 50.0 + 30.0 * std::sin(i * 0.1) + (rand() % 10);
        } else if (metric_name.find("memory") != std::string::npos) {
            point.value = 1024 * 1024 * 1024 + i * 10 * 1024 * 1024;  // 증가 추세
        } else {
            point.value = 100.0 + (rand() % 50);
        }
        
        history.push_back(point);
    }
    
    return history;
}

void analysis_dashboard::update_metric_cache() {
    auto now = std::chrono::steady_clock::now();
    if (now - last_update_ < std::chrono::seconds(5)) {
        return;  // 5초 내 업데이트 건너뛰기
    }
    
    auto snapshot = monitor_->get_multi_process_snapshot();
    
    // 시스템 메트릭 캐싱
    for (const auto& [proc_id, sys_metrics] : snapshot.process_system_metrics) {
        std::string cpu_key = proc_id.process_name + "_cpu";
        std::string mem_key = proc_id.process_name + "_memory";
        
        trend_analyzer::trend_point cpu_point{now, 
            static_cast<double>(sys_metrics.cpu_usage_percent)};
        trend_analyzer::trend_point mem_point{now, 
            static_cast<double>(sys_metrics.memory_usage_bytes)};
        
        metric_cache_[cpu_key].push_back(cpu_point);
        metric_cache_[mem_key].push_back(mem_point);
        
        // 크기 제한
        if (metric_cache_[cpu_key].size() > 1000) {
            metric_cache_[cpu_key].erase(metric_cache_[cpu_key].begin());
        }
        if (metric_cache_[mem_key].size() > 1000) {
            metric_cache_[mem_key].erase(metric_cache_[mem_key].begin());
        }
    }
    
    // Thread pool 메트릭 캐싱
    for (const auto& [pool_id, pool_metrics] : snapshot.thread_pool_metrics_map) {
        std::string throughput_key = pool_id.pool_name + "_throughput";
        std::string queue_key = pool_id.pool_name + "_queue";
        
        trend_analyzer::trend_point throughput_point{now, 
            static_cast<double>(pool_metrics.jobs_completed)};
        trend_analyzer::trend_point queue_point{now, 
            static_cast<double>(pool_metrics.jobs_pending)};
        
        metric_cache_[throughput_key].push_back(throughput_point);
        metric_cache_[queue_key].push_back(queue_point);
    }
    
    last_update_ = now;
}

double analysis_dashboard::calculate_process_health_score(
    const monitoring_interface::process_identifier& proc_id) {
    
    auto snapshot = monitor_->get_process_snapshot(proc_id);
    
    double score = 100.0;
    
    // CPU 페널티
    if (snapshot.system.cpu_usage_percent > 80) {
        score -= (snapshot.system.cpu_usage_percent - 80) * 0.5;
    }
    
    // 메모리 페널티
    double memory_gb = snapshot.system.memory_usage_bytes / (1024.0 * 1024.0 * 1024.0);
    if (memory_gb > 2.0) {
        score -= (memory_gb - 2.0) * 10.0;
    }
    
    // Thread pool 건강도
    if (snapshot.thread_pool.jobs_completed > 0) {
        double failure_rate = static_cast<double>(snapshot.thread_pool.jobs_failed) / 
                             snapshot.thread_pool.jobs_completed;
        score -= failure_rate * 100.0;
    }
    
    return std::max(0.0, std::min(100.0, score));
}

double analysis_dashboard::calculate_pool_health_score(
    const monitoring_interface::thread_pool_identifier& pool_id) {
    
    auto pool_metrics = monitor_->get_thread_pool_metrics(pool_id);
    return thread_pool_analyzer::calculate_health_score(pool_metrics);
}

std::string analysis_dashboard::format_duration(std::chrono::seconds duration) {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    
    std::stringstream ss;
    if (hours.count() > 0) {
        ss << hours.count() << "h ";
    }
    if (minutes.count() > 0) {
        ss << minutes.count() << "m ";
    }
    ss << duration.count() << "s";
    
    return ss.str();
}

std::string analysis_dashboard::severity_to_string(alert_manager::alert_severity severity) {
    switch (severity) {
        case alert_manager::alert_severity::info: return "INFO";
        case alert_manager::alert_severity::warning: return "WARN";
        case alert_manager::alert_severity::critical: return "CRIT";
        case alert_manager::alert_severity::emergency: return "EMRG";
        default: return "UNKN";
    }
}

} // namespace monitoring_module