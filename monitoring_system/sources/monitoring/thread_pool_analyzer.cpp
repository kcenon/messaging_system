/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "thread_pool_analyzer.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

namespace monitoring_module {

thread_pool_analyzer::pool_performance_summary 
thread_pool_analyzer::analyze_pool(const pool_metrics& metrics) {
    pool_performance_summary summary;
    summary.pool_id = metrics.pool_id;
    summary.total_throughput = metrics.jobs_completed;
    
    // 워커당 처리량
    if (metrics.worker_threads > 0) {
        summary.throughput_per_worker = 
            static_cast<double>(metrics.jobs_completed) / metrics.worker_threads;
    }
    
    // 워커 효율성 (활성 워커 비율)
    if (metrics.worker_threads > 0) {
        double active_ratio = static_cast<double>(metrics.worker_threads - metrics.idle_threads) 
                            / metrics.worker_threads;
        summary.worker_efficiency = active_ratio * 100.0;
    }
    
    // 큐 포화도
    if (metrics.jobs_completed > 0) {
        summary.queue_saturation = 
            static_cast<double>(metrics.jobs_pending) / (metrics.jobs_completed + metrics.jobs_pending) * 100.0;
    }
    
    // 평균 워커 부하 및 부하 분산 점수
    if (!metrics.worker_load_distribution.empty()) {
        auto total_load = std::accumulate(metrics.worker_load_distribution.begin(),
                                        metrics.worker_load_distribution.end(), 0ULL);
        summary.average_worker_load = 
            static_cast<double>(total_load) / metrics.worker_load_distribution.size();
        summary.load_balance_score = 
            calculate_load_balance_score(metrics.worker_load_distribution);
    }
    
    return summary;
}

thread_pool_analyzer::pool_comparison_result 
thread_pool_analyzer::compare_pools(const pool_metrics& metrics1,
                                   const pool_metrics& metrics2) {
    pool_comparison_result result;
    result.pool1 = metrics1.pool_id;
    result.pool2 = metrics2.pool_id;
    
    auto summary1 = analyze_pool(metrics1);
    auto summary2 = analyze_pool(metrics2);
    
    // 처리량 비율
    if (summary2.total_throughput > 0) {
        result.throughput_ratio = 
            static_cast<double>(summary1.total_throughput) / summary2.total_throughput;
    }
    
    // 효율성 차이
    result.efficiency_difference = summary1.worker_efficiency - summary2.worker_efficiency;
    
    // 부하 분산 차이
    result.load_balance_difference = summary1.load_balance_score - summary2.load_balance_score;
    
    // 성능 우위 판단
    int pool1_wins = 0;
    int pool2_wins = 0;
    
    if (result.throughput_ratio > 1.1) pool1_wins++;
    else if (result.throughput_ratio < 0.9) pool2_wins++;
    
    if (result.efficiency_difference > 10) pool1_wins++;
    else if (result.efficiency_difference < -10) pool2_wins++;
    
    if (result.load_balance_difference > 10) pool1_wins++;
    else if (result.load_balance_difference < -10) pool2_wins++;
    
    if (pool1_wins > pool2_wins) {
        result.performance_winner = metrics1.pool_id.pool_name;
    } else if (pool2_wins > pool1_wins) {
        result.performance_winner = metrics2.pool_id.pool_name;
    } else {
        result.performance_winner = "Comparable";
    }
    
    // 권고사항 생성
    std::stringstream rec;
    if (result.throughput_ratio < 0.8) {
        rec << metrics1.pool_id.pool_name << " needs performance tuning. ";
    }
    if (std::abs(result.efficiency_difference) > 20) {
        auto inefficient = result.efficiency_difference < 0 ? metrics1.pool_id.pool_name 
                                                            : metrics2.pool_id.pool_name;
        rec << inefficient << " has low worker efficiency. ";
    }
    if (std::abs(result.load_balance_difference) > 20) {
        auto imbalanced = result.load_balance_difference < 0 ? metrics1.pool_id.pool_name 
                                                              : metrics2.pool_id.pool_name;
        rec << imbalanced << " has poor load distribution. ";
    }
    
    result.recommendation = rec.str();
    if (result.recommendation.empty()) {
        result.recommendation = "Both pools are performing well.";
    }
    
    return result;
}

std::unordered_map<thread_pool_analyzer::process_identifier, 
                  std::pair<thread_pool_analyzer::pool_identifier, 
                           thread_pool_analyzer::pool_identifier>>
thread_pool_analyzer::find_best_worst_pools_per_process(
    const std::unordered_map<process_identifier, 
                           std::vector<pool_metrics>>& pools_by_process) {
    
    std::unordered_map<process_identifier, 
                      std::pair<pool_identifier, pool_identifier>> result;
    
    for (const auto& [proc_id, pools] : pools_by_process) {
        if (pools.empty()) continue;
        
        pool_identifier best_pool = pools[0].pool_id;
        pool_identifier worst_pool = pools[0].pool_id;
        double best_score = 0;
        double worst_score = 100;
        
        for (const auto& pool : pools) {
            double score = calculate_health_score(pool);
            if (score > best_score) {
                best_score = score;
                best_pool = pool.pool_id;
            }
            if (score < worst_score) {
                worst_score = score;
                worst_pool = pool.pool_id;
            }
        }
        
        result[proc_id] = {best_pool, worst_pool};
    }
    
    return result;
}

std::optional<std::string> 
thread_pool_analyzer::detect_bottleneck(const pool_metrics& metrics) {
    auto summary = analyze_pool(metrics);
    
    // 큐 포화 검사
    if (summary.queue_saturation > 80) {
        return "Queue saturation detected: Too many pending jobs";
    }
    
    // 워커 효율성 검사
    if (summary.worker_efficiency < 50 && metrics.jobs_pending > 0) {
        return "Low worker utilization despite pending jobs";
    }
    
    // 부하 불균형 검사
    if (summary.load_balance_score < 50) {
        return "Severe load imbalance among workers";
    }
    
    // 레이턴시 검사
    if (metrics.average_latency_ns > 1000000000) { // 1초 이상
        return "High job latency detected";
    }
    
    return std::nullopt;
}

std::vector<std::string> 
thread_pool_analyzer::suggest_optimizations(const pool_metrics& metrics) {
    std::vector<std::string> suggestions;
    auto summary = analyze_pool(metrics);
    
    // 워커 수 조정 제안
    if (summary.queue_saturation > 70 && summary.worker_efficiency > 90) {
        suggestions.push_back("Increase worker count to handle queue backlog");
    } else if (summary.worker_efficiency < 30 && metrics.jobs_pending < metrics.worker_threads) {
        suggestions.push_back("Reduce worker count to improve efficiency");
    }
    
    // 부하 분산 개선
    if (summary.load_balance_score < 60) {
        suggestions.push_back("Implement better work stealing or load balancing");
    }
    
    // 배치 처리 제안
    if (metrics.jobs_completed > 10000 && metrics.average_latency_ns < 1000000) {
        suggestions.push_back("Consider batch processing for small jobs");
    }
    
    // 메모리 풀 사용 제안
    if (metrics.memory_pool_usage_bytes == 0 && metrics.jobs_completed > 1000) {
        suggestions.push_back("Implement memory pooling to reduce allocation overhead");
    }
    
    return suggestions;
}

std::string thread_pool_analyzer::classify_pool_type(const pool_metrics& metrics) {
    auto summary = analyze_pool(metrics);
    
    // Idle pool
    if (metrics.jobs_completed < 10 && summary.worker_efficiency < 10) {
        return "Idle";
    }
    
    // CPU-bound: 높은 효율성, 낮은 큐 포화도
    if (summary.worker_efficiency > 80 && summary.queue_saturation < 20) {
        return "CPU-bound";
    }
    
    // IO-bound: 낮은 효율성, 높은 대기 시간
    if (summary.worker_efficiency < 50 && metrics.average_latency_ns > 10000000) {
        return "IO-bound";
    }
    
    // Balanced
    return "Balanced";
}

double thread_pool_analyzer::calculate_load_balance_score(
    const std::vector<std::uint64_t>& worker_loads) {
    
    if (worker_loads.empty()) return 0.0;
    if (worker_loads.size() == 1) return 100.0;
    
    // 평균 계산
    double mean = std::accumulate(worker_loads.begin(), worker_loads.end(), 0.0) 
                / worker_loads.size();
    
    if (mean == 0) return 100.0; // 모두 0이면 완벽한 균형
    
    // 표준편차 계산
    double variance = 0.0;
    for (auto load : worker_loads) {
        double diff = load - mean;
        variance += diff * diff;
    }
    variance /= worker_loads.size();
    double std_dev = std::sqrt(variance);
    
    // 변동계수 (Coefficient of Variation)
    double cv = std_dev / mean;
    
    // 점수 계산 (CV가 작을수록 점수가 높음)
    double score = 100.0 * std::exp(-2.0 * cv);
    return std::max(0.0, std::min(100.0, score));
}

double thread_pool_analyzer::calculate_health_score(const pool_metrics& metrics) {
    auto summary = analyze_pool(metrics);
    
    double health_score = 0.0;
    double weight_sum = 0.0;
    
    // 처리량 점수 (가중치 30%)
    if (metrics.jobs_completed > 0) {
        double throughput_score = std::min(100.0, 
            static_cast<double>(metrics.jobs_completed) / 100.0);
        health_score += throughput_score * 0.3;
        weight_sum += 0.3;
    }
    
    // 효율성 점수 (가중치 25%)
    health_score += summary.worker_efficiency * 0.25;
    weight_sum += 0.25;
    
    // 부하 분산 점수 (가중치 20%)
    health_score += summary.load_balance_score * 0.2;
    weight_sum += 0.2;
    
    // 큐 건강도 (가중치 15%)
    double queue_health = 100.0 - summary.queue_saturation;
    health_score += queue_health * 0.15;
    weight_sum += 0.15;
    
    // 레이턴시 점수 (가중치 10%)
    if (metrics.average_latency_ns > 0) {
        // 10ms를 기준으로 점수 계산
        double latency_score = 100.0 * std::exp(-metrics.average_latency_ns / 10000000.0);
        health_score += latency_score * 0.1;
        weight_sum += 0.1;
    }
    
    // 정규화
    if (weight_sum > 0) {
        health_score = health_score / weight_sum * 100.0;
    }
    
    return std::max(0.0, std::min(100.0, health_score));
}

} // namespace monitoring_module