#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/multi_process_monitoring_interface.h"
#include <memory>
#include <cstdint>

namespace monitoring_module {

/**
 * @class multi_process_monitoring
 * @brief 다중 프로세스 모니터링 구현 클래스
 * 
 * 프로세스별, thread pool별 독립적인 모니터링을 지원하며
 * 기존 인터페이스와의 하위 호환성을 유지
 */
class multi_process_monitoring : public ::monitoring_interface::multi_process_monitoring_interface {
public:
    /**
     * @brief 생성자
     * @param history_size 히스토리 크기
     * @param collection_interval_ms 수집 간격 (밀리초)
     * @param max_processes 최대 프로세스 수
     * @param max_pools_per_process 프로세스당 최대 thread pool 수
     */
    explicit multi_process_monitoring(std::size_t history_size = 1000,
                                    std::uint32_t collection_interval_ms = 1000,
                                    std::size_t max_processes = 100,
                                    std::size_t max_pools_per_process = 50);
    
    ~multi_process_monitoring() override;

    // 기존 인터페이스 지원 (하위 호환성)
    void update_system_metrics(const ::monitoring_interface::system_metrics& metrics) override;
    void update_thread_pool_metrics(const ::monitoring_interface::thread_pool_metrics& metrics) override;
    
    // Bring base class overloads into scope
    using multi_process_monitoring_interface::update_thread_pool_metrics;
    void update_worker_metrics(std::size_t worker_id, const ::monitoring_interface::worker_metrics& metrics) override;
    ::monitoring_interface::metrics_snapshot get_current_snapshot() const override;
    std::vector<::monitoring_interface::metrics_snapshot> get_recent_snapshots(std::size_t count) const override;
    bool is_active() const override;
    void start() override;
    void stop() override;
    double get_average_cpu_usage(std::chrono::steady_clock::duration duration) const override;
    std::uint64_t get_peak_memory_usage(std::chrono::steady_clock::duration duration) const override;
    double get_average_job_latency(std::chrono::steady_clock::duration duration) const override;
    std::unordered_map<std::string, double> get_statistics() const override;

    // 새로운 다중 프로세스 인터페이스
    void register_process(const ::monitoring_interface::process_identifier& process_id) override;
    void unregister_process(const ::monitoring_interface::process_identifier& process_id) override;
    void register_thread_pool(const ::monitoring_interface::thread_pool_identifier& pool_id) override;
    void unregister_thread_pool(const ::monitoring_interface::thread_pool_identifier& pool_id) override;
    
    void update_process_system_metrics(const ::monitoring_interface::process_identifier& process_id,
                                     const ::monitoring_interface::system_metrics& metrics) override;
    void update_thread_pool_metrics(const ::monitoring_interface::thread_pool_identifier& pool_id,
                                   const ::monitoring_interface::process_thread_pool_metrics& metrics) override;
    void update_process_worker_metrics(const ::monitoring_interface::process_identifier& process_id,
                                     std::size_t worker_id,
                                     const ::monitoring_interface::worker_metrics& metrics) override;

    ::monitoring_interface::multi_process_metrics_snapshot get_multi_process_snapshot() const override;
    ::monitoring_interface::metrics_snapshot get_process_snapshot(const ::monitoring_interface::process_identifier& process_id) const override;
    ::monitoring_interface::process_thread_pool_metrics get_thread_pool_metrics(const ::monitoring_interface::thread_pool_identifier& pool_id) const override;
    
    std::vector<::monitoring_interface::process_identifier> get_registered_processes() const override;
    std::vector<::monitoring_interface::thread_pool_identifier> get_process_thread_pools(const ::monitoring_interface::process_identifier& process_id) const override;
    std::unordered_map<std::string, double> compare_process_performance(
        const std::vector<::monitoring_interface::process_identifier>& process_ids) const override;

    // 추가 기능
    void set_process_monitoring_enabled(const ::monitoring_interface::process_identifier& process_id, bool enabled);
    void set_thread_pool_monitoring_enabled(const ::monitoring_interface::thread_pool_identifier& pool_id, bool enabled);
    
    /**
     * @brief 프로세스별 알림 임계값 설정
     */
    void set_process_alert_thresholds(const ::monitoring_interface::process_identifier& process_id,
                                    double cpu_threshold,
                                    std::uint64_t memory_threshold,
                                    std::uint64_t latency_threshold_ns);

    /**
     * @brief Thread pool별 알림 임계값 설정
     */
    void set_thread_pool_alert_thresholds(const ::monitoring_interface::thread_pool_identifier& pool_id,
                                         std::uint64_t queue_size_threshold,
                                         std::uint64_t latency_threshold_ns,
                                         double worker_utilization_threshold);

    /**
     * @brief 현재 프로세스를 기본 프로세스로 설정
     * @param process_id 기본 프로세스 식별자
     */
    void set_default_process(const ::monitoring_interface::process_identifier& process_id);

    /**
     * @brief 기본 thread pool 설정
     * @param pool_id 기본 thread pool 식별자
     */
    void set_default_thread_pool(const ::monitoring_interface::thread_pool_identifier& pool_id);

private:
    class multi_process_impl;
    std::unique_ptr<multi_process_impl> pimpl_;
};

} // namespace monitoring_module