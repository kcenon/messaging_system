#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../monitoring/monitoring_interface.h"
#include "process_identifier.h"
#include <unordered_map>
#include <vector>

namespace monitoring_interface {

/**
 * @struct process_thread_pool_metrics
 * @brief 프로세스별 thread pool 메트릭
 * 
 * 기존 thread_pool_metrics를 확장하여 프로세스별 추가 정보 포함
 */
struct process_thread_pool_metrics : public thread_pool_metrics {
    thread_pool_identifier pool_id;          // Pool 식별자
    std::uint64_t cross_process_jobs{0};     // 프로세스간 작업수
    std::uint64_t memory_pool_usage_bytes{0}; // 메모리 풀 사용량
    std::vector<std::uint64_t> worker_load_distribution; // 워커별 부하 분산
    
    // Constructor for compatibility
    process_thread_pool_metrics() = default;
    process_thread_pool_metrics(const thread_pool_metrics& base) 
        : thread_pool_metrics(base) {}
};

/**
 * @struct multi_process_metrics_snapshot
 * @brief 다중 프로세스 메트릭 스냅샷
 * 
 * 여러 프로세스의 메트릭을 한번에 담는 스냅샷
 */
struct multi_process_metrics_snapshot {
    std::chrono::steady_clock::time_point capture_time;
    
    // 전체 시스템 메트릭
    system_metrics global_system;
    
    // 프로세스별 메트릭 맵
    std::unordered_map<process_identifier, system_metrics> process_system_metrics;
    
    // Thread pool별 메트릭 맵
    std::unordered_map<thread_pool_identifier, process_thread_pool_metrics> thread_pool_metrics_map;
    
    // 프로세스별 worker 메트릭 맵
    std::unordered_map<process_identifier, 
                      std::unordered_map<std::size_t, worker_metrics>> process_worker_metrics;
};

/**
 * @class multi_process_monitoring_interface
 * @brief 다중 프로세스 모니터링 인터페이스
 * 
 * 기존 monitoring_interface를 확장하여 프로세스별 모니터링 지원
 */
class multi_process_monitoring_interface : public monitoring_interface {
public:
    virtual ~multi_process_monitoring_interface() = default;
    
    // Bring base class method into scope to avoid hiding
    using monitoring_interface::update_thread_pool_metrics;

    /**
     * @brief 프로세스 등록
     * @param process_id 프로세스 식별자
     */
    virtual void register_process(const process_identifier& process_id) = 0;

    /**
     * @brief 프로세스 등록 해제
     * @param process_id 프로세스 식별자
     */
    virtual void unregister_process(const process_identifier& process_id) = 0;

    /**
     * @brief Thread pool 등록
     * @param pool_id Thread pool 식별자
     */
    virtual void register_thread_pool(const thread_pool_identifier& pool_id) = 0;

    /**
     * @brief Thread pool 등록 해제
     * @param pool_id Thread pool 식별자
     */
    virtual void unregister_thread_pool(const thread_pool_identifier& pool_id) = 0;

    /**
     * @brief 프로세스별 시스템 메트릭 업데이트
     * @param process_id 프로세스 식별자
     * @param metrics 시스템 메트릭
     */
    virtual void update_process_system_metrics(const process_identifier& process_id,
                                             const system_metrics& metrics) = 0;

    /**
     * @brief Thread pool별 메트릭 업데이트
     * @param pool_id Thread pool 식별자
     * @param metrics Thread pool 메트릭
     */
    virtual void update_thread_pool_metrics(const thread_pool_identifier& pool_id,
                                           const process_thread_pool_metrics& metrics) = 0;

    /**
     * @brief 프로세스별 worker 메트릭 업데이트
     * @param process_id 프로세스 식별자
     * @param worker_id Worker ID
     * @param metrics Worker 메트릭
     */
    virtual void update_process_worker_metrics(const process_identifier& process_id,
                                             std::size_t worker_id,
                                             const worker_metrics& metrics) = 0;

    /**
     * @brief 다중 프로세스 스냅샷 조회
     * @return 현재 다중 프로세스 메트릭 스냅샷
     */
    virtual multi_process_metrics_snapshot get_multi_process_snapshot() const = 0;

    /**
     * @brief 특정 프로세스 메트릭 조회
     * @param process_id 프로세스 식별자
     * @return 프로세스별 메트릭 스냅샷
     */
    virtual metrics_snapshot get_process_snapshot(const process_identifier& process_id) const = 0;

    /**
     * @brief 특정 thread pool 메트릭 조회
     * @param pool_id Thread pool 식별자
     * @return Thread pool 메트릭
     */
    virtual process_thread_pool_metrics get_thread_pool_metrics(const thread_pool_identifier& pool_id) const = 0;

    /**
     * @brief 등록된 모든 프로세스 조회
     * @return 프로세스 식별자 벡터
     */
    virtual std::vector<process_identifier> get_registered_processes() const = 0;

    /**
     * @brief 프로세스별 thread pool 목록 조회
     * @param process_id 프로세스 식별자
     * @return Thread pool 식별자 벡터
     */
    virtual std::vector<thread_pool_identifier> get_process_thread_pools(const process_identifier& process_id) const = 0;

    /**
     * @brief 프로세스간 성능 비교 분석
     * @param process_ids 비교할 프로세스 목록
     * @return 비교 분석 결과 (키: 메트릭명, 값: 프로세스별 점수)
     */
    virtual std::unordered_map<std::string, double> compare_process_performance(
        const std::vector<process_identifier>& process_ids) const = 0;

    // Additional virtual methods from monitoring_interface extensions
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual double get_average_cpu_usage(std::chrono::steady_clock::duration duration) const = 0;
    virtual std::uint64_t get_peak_memory_usage(std::chrono::steady_clock::duration duration) const = 0;
    virtual double get_average_job_latency(std::chrono::steady_clock::duration duration) const = 0;
    virtual std::unordered_map<std::string, double> get_statistics() const = 0;
};

} // namespace monitoring_interface