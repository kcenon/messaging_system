#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/multi_process_monitoring_interface.h"
#include "optimized_storage.h"
#include <memory>
#include <functional>
#include <chrono>
#include <thread>
#include <deque>

namespace monitoring_module {

/**
 * @class performance_optimizer
 * @brief 성능 최적화 관리자
 * 
 * 모니터링 시스템의 성능을 자동으로 최적화
 */
class performance_optimizer {
public:
    /**
     * @struct optimization_config
     * @brief 최적화 설정
     */
    struct optimization_config {
        bool enable_compression{true};          // 데이터 압축 활성화
        bool enable_batching{true};             // 배치 처리 활성화
        bool enable_tiered_storage{true};       // 계층형 저장소 활성화
        bool enable_adaptive_sampling{true};    // 적응형 샘플링 활성화
        std::size_t batch_size{100};            // 배치 크기
        std::size_t compression_threshold{1000}; // 압축 임계값
        std::chrono::milliseconds batch_interval{100}; // 배치 간격
    };
    
    /**
     * @struct optimization_stats
     * @brief 최적화 통계
     */
    struct optimization_stats {
        std::size_t memory_saved_bytes{0};      // 절약된 메모리
        std::size_t cpu_cycles_saved{0};        // 절약된 CPU 사이클
        double compression_ratio{1.0};          // 압축률
        std::size_t batches_processed{0};       // 처리된 배치 수
        std::size_t samples_skipped{0};         // 건너뛴 샘플 수
        std::chrono::nanoseconds time_saved_ns{0}; // 절약된 시간
    };
    
    /**
     * @brief 생성자
     * @param config 최적화 설정
     */
    explicit performance_optimizer(const optimization_config& config);
    
    /**
     * @brief 메트릭 최적화 처리
     * @param snapshot 원본 메트릭
     * @return 최적화 여부
     */
    bool optimize_metric(const monitoring_interface::metrics_snapshot& snapshot);
    
    /**
     * @brief 적응형 샘플링 레이트 조정
     * @param process_id 프로세스 ID
     * @param current_load 현재 부하
     */
    void adjust_sampling_rate(const monitoring_interface::process_identifier& process_id,
                            double current_load);
    
    /**
     * @brief 메모리 압력에 따른 자동 조정
     * @param memory_pressure 메모리 압력 (0-1)
     */
    void adapt_to_memory_pressure(double memory_pressure);
    
    /**
     * @brief CPU 부하에 따른 자동 조정
     * @param cpu_load CPU 부하 (0-100)
     */
    void adapt_to_cpu_load(double cpu_load);
    
    /**
     * @brief 최적화 통계 조회
     * @return 최적화 통계
     */
    optimization_stats get_stats() const;
    
    /**
     * @brief 최적화된 저장소 접근
     * @return 계층형 저장소
     */
    tiered_storage& get_storage() { return *storage_; }
    
    /**
     * @brief 배치 프로세서 콜백 설정
     * @param callback 배치 처리 콜백
     */
    void set_batch_callback(batch_metrics_processor::batch_callback callback);
    
private:
    optimization_config config_;
    mutable std::mutex stats_mutex_;
    optimization_stats stats_;
    
    // 최적화 컴포넌트
    std::unique_ptr<tiered_storage> storage_;
    std::unique_ptr<batch_metrics_processor> batch_processor_;
    std::unique_ptr<compressed_metrics_storage> compression_buffer_;
    
    // 적응형 샘플링
    struct sampling_state {
        double rate{1.0};  // 샘플링 비율 (0-1)
        std::chrono::steady_clock::time_point last_sample;
        std::size_t skip_count{0};
    };
    std::unordered_map<monitoring_interface::process_identifier, sampling_state> sampling_states_;
    std::mutex sampling_mutex_;
    
    // 내부 헬퍼
    bool should_sample(const monitoring_interface::process_identifier& process_id);
    void update_stats(std::size_t memory_saved, std::size_t cpu_saved);
};

/**
 * @class auto_scaler
 * @brief 자동 스케일러
 * 
 * 부하에 따라 모니터링 리소스를 자동으로 조정
 */
class auto_scaler {
public:
    /**
     * @struct scaling_policy
     * @brief 스케일링 정책
     */
    struct scaling_policy {
        double cpu_threshold_up{80.0};      // 스케일 업 CPU 임계값
        double cpu_threshold_down{30.0};    // 스케일 다운 CPU 임계값
        double memory_threshold_up{80.0};   // 스케일 업 메모리 임계값
        double memory_threshold_down{30.0}; // 스케일 다운 메모리 임계값
        std::chrono::seconds cooldown{60};  // 쿨다운 기간
        double scale_factor{1.5};           // 스케일 팩터
    };
    
    /**
     * @struct scaling_decision
     * @brief 스케일링 결정
     */
    struct scaling_decision {
        enum class action { none, scale_up, scale_down };
        action recommended_action{action::none};
        double confidence{0.0};             // 결정 신뢰도 (0-1)
        std::string reason;                 // 결정 이유
        std::size_t recommended_resources{0}; // 권장 리소스
    };
    
    /**
     * @brief 생성자
     * @param policy 스케일링 정책
     */
    explicit auto_scaler(const scaling_policy& policy);
    
    /**
     * @brief 스케일링 결정
     * @param current_metrics 현재 메트릭
     * @return 스케일링 결정
     */
    scaling_decision decide(const monitoring_interface::metrics_snapshot& current_metrics);
    
    /**
     * @brief 예측 기반 스케일링 결정
     * @param predicted_load 예측된 부하
     * @param time_horizon 예측 시간
     * @return 스케일링 결정
     */
    scaling_decision decide_predictive(double predicted_load,
                                     std::chrono::seconds time_horizon);
    
    /**
     * @brief 스케일링 이력 조회
     * @param count 조회할 개수
     * @return 스케일링 결정 이력
     */
    std::vector<std::pair<std::chrono::steady_clock::time_point, scaling_decision>>
    get_history(std::size_t count = 10) const;
    
private:
    scaling_policy policy_;
    std::chrono::steady_clock::time_point last_scale_time_;
    
    struct resource_state {
        std::size_t current_resources{1};
        double smoothed_cpu_load{0.0};
        double smoothed_memory_load{0.0};
    };
    resource_state state_;
    
    mutable std::mutex history_mutex_;
    std::deque<std::pair<std::chrono::steady_clock::time_point, scaling_decision>> history_;
    
    bool is_in_cooldown() const;
    void update_smoothed_metrics(double cpu_load, double memory_load);
    void record_decision(const scaling_decision& decision);
};

/**
 * @class distributed_aggregator
 * @brief 분산 집계기
 * 
 * 여러 모니터링 인스턴스의 메트릭을 효율적으로 집계
 */
class distributed_aggregator {
public:
    using aggregation_callback = std::function<void(
        const monitoring_interface::multi_process_metrics_snapshot&)>;
    
    /**
     * @struct aggregation_config
     * @brief 집계 설정
     */
    struct aggregation_config {
        std::chrono::milliseconds aggregation_interval{1000};
        bool enable_parallel_aggregation{true};
        std::size_t worker_threads{4};
        bool enable_incremental_aggregation{true};
    };
    
    /**
     * @brief 생성자
     * @param config 집계 설정
     */
    explicit distributed_aggregator(const aggregation_config& config);
    
    /**
     * @brief 로컬 메트릭 추가
     * @param node_id 노드 ID
     * @param snapshot 메트릭 스냅샷
     */
    void add_local_metrics(const std::string& node_id,
                          const monitoring_interface::multi_process_metrics_snapshot& snapshot);
    
    /**
     * @brief 글로벌 집계 수행
     * @return 집계된 메트릭
     */
    monitoring_interface::multi_process_metrics_snapshot aggregate_global();
    
    /**
     * @brief 집계 콜백 설정
     * @param callback 집계 완료 콜백
     */
    void set_aggregation_callback(aggregation_callback callback);
    
    /**
     * @brief 노드 상태 조회
     * @return 노드별 상태 맵
     */
    std::unordered_map<std::string, std::chrono::steady_clock::time_point>
    get_node_status() const;
    
private:
    aggregation_config config_;
    aggregation_callback callback_;
    
    struct node_data {
        monitoring_interface::multi_process_metrics_snapshot latest_snapshot;
        std::chrono::steady_clock::time_point last_update;
        bool is_active{true};
    };
    
    mutable std::shared_mutex data_mutex_;
    std::unordered_map<std::string, node_data> node_metrics_;
    
    std::atomic<bool> aggregating_{false};
    std::thread aggregation_thread_;
    
    void parallel_aggregate(monitoring_interface::multi_process_metrics_snapshot& result);
    void incremental_aggregate(const std::string& node_id,
                             const monitoring_interface::multi_process_metrics_snapshot& snapshot,
                             monitoring_interface::multi_process_metrics_snapshot& result);
};

} // namespace monitoring_module