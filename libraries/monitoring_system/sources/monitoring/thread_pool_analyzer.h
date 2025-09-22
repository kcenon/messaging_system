#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/multi_process_monitoring_interface.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

namespace monitoring_module {

/**
 * @class thread_pool_analyzer
 * @brief Thread pool 분석 및 비교 도구
 * 
 * 여러 thread pool의 성능을 분석하고 비교하는 기능 제공
 */
class thread_pool_analyzer {
public:
    using pool_identifier = monitoring_interface::thread_pool_identifier;
    using pool_metrics = monitoring_interface::process_thread_pool_metrics;
    using process_identifier = monitoring_interface::process_identifier;
    
    /**
     * @struct pool_performance_summary
     * @brief Thread pool 성능 요약
     */
    struct pool_performance_summary {
        pool_identifier pool_id;
        double throughput_per_worker{0.0};    // 워커당 처리량
        double worker_efficiency{0.0};         // 워커 효율성 (0-100)
        double queue_saturation{0.0};          // 큐 포화도 (0-100)
        double average_worker_load{0.0};       // 평균 워커 부하
        double load_balance_score{0.0};        // 부하 분산 점수 (0-100)
        std::uint64_t total_throughput{0};     // 전체 처리량
    };
    
    /**
     * @struct pool_comparison_result
     * @brief Thread pool 비교 결과
     */
    struct pool_comparison_result {
        pool_identifier pool1;
        pool_identifier pool2;
        double throughput_ratio{0.0};          // pool1/pool2 처리량 비율
        double efficiency_difference{0.0};      // 효율성 차이 (pool1 - pool2)
        double load_balance_difference{0.0};    // 부하 분산 차이
        std::string performance_winner;         // 성능 우위 pool
        std::string recommendation;             // 개선 권고사항
    };
    
    /**
     * @brief Thread pool 성능 분석
     * @param metrics Thread pool 메트릭
     * @return 성능 요약
     */
    static pool_performance_summary analyze_pool(const pool_metrics& metrics);
    
    /**
     * @brief 동일 프로세스 내 thread pool 비교
     * @param metrics1 첫 번째 pool 메트릭
     * @param metrics2 두 번째 pool 메트릭
     * @return 비교 결과
     */
    static pool_comparison_result compare_pools(const pool_metrics& metrics1,
                                               const pool_metrics& metrics2);
    
    /**
     * @brief 프로세스별 thread pool 그룹 분석
     * @param pools_by_process 프로세스별 thread pool 메트릭
     * @return 프로세스별 최적/최악 pool 정보
     */
    static std::unordered_map<process_identifier, 
                            std::pair<pool_identifier, pool_identifier>>
    find_best_worst_pools_per_process(
        const std::unordered_map<process_identifier, 
                               std::vector<pool_metrics>>& pools_by_process);
    
    /**
     * @brief Thread pool 병목 현상 감지
     * @param metrics Thread pool 메트릭
     * @return 병목 현상 설명 (없으면 empty)
     */
    static std::optional<std::string> detect_bottleneck(const pool_metrics& metrics);
    
    /**
     * @brief Thread pool 최적화 제안
     * @param metrics Thread pool 메트릭
     * @return 최적화 제안 목록
     */
    static std::vector<std::string> suggest_optimizations(const pool_metrics& metrics);
    
    /**
     * @brief Thread pool 유형 분류
     * @param metrics Thread pool 메트릭
     * @return Pool 유형 (CPU-bound, IO-bound, Balanced, Idle)
     */
    static std::string classify_pool_type(const pool_metrics& metrics);
    
    /**
     * @brief 워커 부하 분산 점수 계산
     * @param worker_loads 워커별 작업 수
     * @return 부하 분산 점수 (0-100, 100이 완벽한 분산)
     */
    static double calculate_load_balance_score(const std::vector<std::uint64_t>& worker_loads);
    
    /**
     * @brief Thread pool 건강도 점수
     * @param metrics Thread pool 메트릭
     * @return 건강도 점수 (0-100)
     */
    static double calculate_health_score(const pool_metrics& metrics);
};

} // namespace monitoring_module