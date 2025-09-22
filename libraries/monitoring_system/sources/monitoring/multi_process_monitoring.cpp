/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "multi_process_monitoring.h"
#include "storage/ring_buffer.h"
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace monitoring_module {

using namespace monitoring_interface;

/**
 * @class multi_process_monitoring::multi_process_impl
 * @brief 다중 프로세스 모니터링 구현 세부사항
 */
class multi_process_monitoring::multi_process_impl {
public:
    multi_process_impl(std::size_t history_size,
                      std::uint32_t collection_interval_ms,
                      std::size_t max_processes,
                      std::size_t max_pools_per_process)
        : history_size_(history_size)
        , collection_interval_ms_(collection_interval_ms)
        , max_processes_(max_processes)
        , max_pools_per_process_(max_pools_per_process)
        , is_active_(false)
        , global_history_(history_size) {
    }

    ~multi_process_impl() {
        stop();
    }

    // 프로세스 관리
    void register_process(const process_identifier& process_id) {
        std::unique_lock lock(data_mutex_);
        
        if (process_data_.find(process_id) != process_data_.end()) {
            return; // 이미 등록됨
        }
        
        if (process_data_.size() >= max_processes_) {
            throw std::runtime_error("Maximum number of processes reached");
        }
        
        process_data_[process_id] = std::make_unique<process_monitoring_data>(history_size_);
    }

    void unregister_process(const process_identifier& process_id) {
        std::unique_lock lock(data_mutex_);
        
        // Thread pool 먼저 제거
        auto it = std::remove_if(registered_pools_.begin(), registered_pools_.end(),
            [&process_id](const thread_pool_identifier& pool_id) {
                return pool_id.process_id == process_id;
            });
        registered_pools_.erase(it, registered_pools_.end());
        
        // 프로세스 데이터 제거
        process_data_.erase(process_id);
    }

    void register_thread_pool(const thread_pool_identifier& pool_id) {
        std::unique_lock lock(data_mutex_);
        
        // 프로세스가 등록되어 있는지 확인
        auto proc_it = process_data_.find(pool_id.process_id);
        if (proc_it == process_data_.end()) {
            throw std::runtime_error("Process not registered");
        }
        
        // Thread pool 등록
        if (proc_it->second->pool_metrics.size() >= max_pools_per_process_) {
            throw std::runtime_error("Maximum number of thread pools per process reached");
        }
        
        proc_it->second->pool_metrics[pool_id] = process_thread_pool_metrics{};
        registered_pools_.push_back(pool_id);
    }

    void unregister_thread_pool(const thread_pool_identifier& pool_id) {
        std::unique_lock lock(data_mutex_);
        
        auto proc_it = process_data_.find(pool_id.process_id);
        if (proc_it != process_data_.end()) {
            proc_it->second->pool_metrics.erase(pool_id);
        }
        
        auto it = std::remove(registered_pools_.begin(), registered_pools_.end(), pool_id);
        registered_pools_.erase(it, registered_pools_.end());
    }

    // 메트릭 업데이트
    void update_process_system_metrics(const process_identifier& process_id,
                                     const system_metrics& metrics) {
        std::shared_lock lock(data_mutex_);
        
        auto it = process_data_.find(process_id);
        if (it != process_data_.end()) {
            std::unique_lock proc_lock(it->second->mutex);
            it->second->system_metrics = metrics;
            it->second->last_update = std::chrono::steady_clock::now();
        }
    }

    void update_thread_pool_metrics(const thread_pool_identifier& pool_id,
                                   const process_thread_pool_metrics& metrics) {
        std::shared_lock lock(data_mutex_);
        
        auto proc_it = process_data_.find(pool_id.process_id);
        if (proc_it != process_data_.end()) {
            std::unique_lock proc_lock(proc_it->second->mutex);
            proc_it->second->pool_metrics[pool_id] = metrics;
        }
    }

    void update_process_worker_metrics(const process_identifier& process_id,
                                     std::size_t worker_id,
                                     const worker_metrics& metrics) {
        std::shared_lock lock(data_mutex_);
        
        auto it = process_data_.find(process_id);
        if (it != process_data_.end()) {
            std::unique_lock proc_lock(it->second->mutex);
            it->second->worker_metrics[worker_id] = metrics;
        }
    }

    // 스냅샷 조회
    multi_process_metrics_snapshot get_multi_process_snapshot() const {
        std::shared_lock lock(data_mutex_);
        
        multi_process_metrics_snapshot snapshot;
        snapshot.capture_time = std::chrono::steady_clock::now();
        
        // 전체 시스템 메트릭 집계
        system_metrics global_metrics{};
        
        for (const auto& [proc_id, proc_data] : process_data_) {
            std::shared_lock proc_lock(proc_data->mutex);
            
            // 프로세스별 시스템 메트릭
            snapshot.process_system_metrics[proc_id] = proc_data->system_metrics;
            
            // 전체 시스템 메트릭 누적
            global_metrics.cpu_usage_percent = std::min(static_cast<std::uint64_t>(100),
                global_metrics.cpu_usage_percent + proc_data->system_metrics.cpu_usage_percent);
            global_metrics.memory_usage_bytes += proc_data->system_metrics.memory_usage_bytes;
            global_metrics.active_threads += proc_data->system_metrics.active_threads;
            
            // Thread pool 메트릭
            for (const auto& [pool_id, pool_metrics] : proc_data->pool_metrics) {
                snapshot.thread_pool_metrics_map[pool_id] = pool_metrics;
            }
            
            // Worker 메트릭
            snapshot.process_worker_metrics[proc_id] = proc_data->worker_metrics;
        }
        
        snapshot.global_system = global_metrics;
        return snapshot;
    }

    metrics_snapshot get_process_snapshot(const process_identifier& process_id) const {
        std::shared_lock lock(data_mutex_);
        
        auto it = process_data_.find(process_id);
        if (it == process_data_.end()) {
            return metrics_snapshot{};
        }
        
        std::shared_lock proc_lock(it->second->mutex);
        
        metrics_snapshot snapshot;
        snapshot.capture_time = std::chrono::steady_clock::now();
        snapshot.system = it->second->system_metrics;
        
        // Thread pool 메트릭 집계
        thread_pool_metrics aggregated_pool{};
        for (const auto& [pool_id, pool_metrics] : it->second->pool_metrics) {
            aggregated_pool.worker_threads += pool_metrics.worker_threads;
            aggregated_pool.idle_threads += pool_metrics.idle_threads;
            aggregated_pool.jobs_completed += pool_metrics.jobs_completed;
            aggregated_pool.jobs_pending += pool_metrics.jobs_pending;
            aggregated_pool.jobs_failed += pool_metrics.jobs_failed;
            aggregated_pool.total_execution_time_ns += pool_metrics.total_execution_time_ns;
            
            if (pool_metrics.jobs_completed > 0) {
                aggregated_pool.average_latency_ns = 
                    (aggregated_pool.average_latency_ns + pool_metrics.average_latency_ns) / 2;
            }
        }
        snapshot.thread_pool = aggregated_pool;
        
        // Worker 메트릭 집계
        worker_metrics aggregated_worker{};
        for (const auto& [worker_id, worker] : it->second->worker_metrics) {
            aggregated_worker.jobs_processed += worker.jobs_processed;
            aggregated_worker.total_processing_time_ns += worker.total_processing_time_ns;
            aggregated_worker.idle_time_ns += worker.idle_time_ns;
        }
        snapshot.worker = aggregated_worker;
        
        return snapshot;
    }

    // 비교 분석
    std::unordered_map<std::string, double> compare_process_performance(
        const std::vector<process_identifier>& process_ids) const {
        
        std::shared_lock lock(data_mutex_);
        std::unordered_map<std::string, double> results;
        
        for (const auto& proc_id : process_ids) {
            auto it = process_data_.find(proc_id);
            if (it == process_data_.end()) continue;
            
            std::shared_lock proc_lock(it->second->mutex);
            
            // CPU 효율성 점수
            double cpu_efficiency = calculate_cpu_efficiency(it->second->system_metrics);
            results[proc_id.process_name + "_cpu_efficiency"] = cpu_efficiency;
            
            // 메모리 효율성 점수  
            double memory_efficiency = calculate_memory_efficiency(it->second->system_metrics);
            results[proc_id.process_name + "_memory_efficiency"] = memory_efficiency;
            
            // 처리량 점수
            double throughput_score = calculate_throughput_score(it->second.get());
            results[proc_id.process_name + "_throughput"] = throughput_score;
        }
        
        return results;
    }

    // 활성화 상태
    bool is_active() const {
        return is_active_.load();
    }

    void start() {
        bool expected = false;
        if (!is_active_.compare_exchange_strong(expected, true)) {
            return; // 이미 실행 중
        }
        
        collection_thread_ = std::thread([this]() {
            collection_loop();
        });
    }

    void stop() {
        is_active_.store(false);
        if (collection_thread_.joinable()) {
            collection_thread_.join();
        }
    }

    // 기본 프로세스/pool 설정
    void set_default_process(const process_identifier& process_id) {
        std::unique_lock lock(data_mutex_);
        default_process_ = process_id;
    }

    void set_default_thread_pool(const thread_pool_identifier& pool_id) {
        std::unique_lock lock(data_mutex_);
        default_pool_ = pool_id;
    }

    // 하위 호환성을 위한 메서드
    void update_system_metrics(const system_metrics& metrics) {
        if (default_process_.pid != 0) {
            update_process_system_metrics(default_process_, metrics);
        }
    }

    void update_thread_pool_metrics(const thread_pool_metrics& metrics) {
        if (default_pool_.process_id.pid != 0) {
            process_thread_pool_metrics proc_metrics;
            static_cast<thread_pool_metrics&>(proc_metrics) = metrics;
            proc_metrics.pool_id = default_pool_;
            update_thread_pool_metrics(default_pool_, proc_metrics);
        }
    }

    void update_worker_metrics(std::size_t worker_id, const worker_metrics& metrics) {
        if (default_process_.pid != 0) {
            update_process_worker_metrics(default_process_, worker_id, metrics);
        }
    }

    metrics_snapshot get_current_snapshot() const {
        if (default_process_.pid != 0) {
            return get_process_snapshot(default_process_);
        }
        
        // 전체 시스템 스냅샷 반환
        auto multi_snapshot = get_multi_process_snapshot();
        metrics_snapshot snapshot;
        snapshot.capture_time = multi_snapshot.capture_time;
        snapshot.system = multi_snapshot.global_system;
        
        // 모든 thread pool 집계
        thread_pool_metrics aggregated_pool{};
        for (const auto& [pool_id, pool_metrics] : multi_snapshot.thread_pool_metrics_map) {
            aggregated_pool.worker_threads += pool_metrics.worker_threads;
            aggregated_pool.idle_threads += pool_metrics.idle_threads;
            aggregated_pool.jobs_completed += pool_metrics.jobs_completed;
            aggregated_pool.jobs_pending += pool_metrics.jobs_pending;
            aggregated_pool.jobs_failed += pool_metrics.jobs_failed;
        }
        snapshot.thread_pool = aggregated_pool;
        
        return snapshot;
    }

    // 조회 메서드
    std::vector<process_identifier> get_registered_processes() const {
        std::shared_lock lock(data_mutex_);
        std::vector<process_identifier> processes;
        processes.reserve(process_data_.size());
        
        for (const auto& [proc_id, _] : process_data_) {
            processes.push_back(proc_id);
        }
        
        return processes;
    }

    std::vector<thread_pool_identifier> get_process_thread_pools(const process_identifier& process_id) const {
        std::shared_lock lock(data_mutex_);
        std::vector<thread_pool_identifier> pools;
        
        auto it = process_data_.find(process_id);
        if (it != process_data_.end()) {
            std::shared_lock proc_lock(it->second->mutex);
            pools.reserve(it->second->pool_metrics.size());
            
            for (const auto& [pool_id, _] : it->second->pool_metrics) {
                pools.push_back(pool_id);
            }
        }
        
        return pools;
    }

private:
    // 프로세스별 모니터링 데이터
    struct process_monitoring_data {
        mutable std::shared_mutex mutex;
        system_metrics system_metrics;
        std::unordered_map<thread_pool_identifier, process_thread_pool_metrics> pool_metrics;
        std::unordered_map<std::size_t, worker_metrics> worker_metrics;
        std::chrono::steady_clock::time_point last_update;
        ring_buffer<metrics_snapshot> history;
        bool monitoring_enabled{true};
        
        explicit process_monitoring_data(std::size_t history_size)
            : history(history_size) {}
    };

    // 효율성 계산 함수들
    double calculate_cpu_efficiency(const system_metrics& metrics) const {
        if (metrics.active_threads == 0) return 0.0;
        return (100.0 - metrics.cpu_usage_percent) / metrics.active_threads;
    }

    double calculate_memory_efficiency(const system_metrics& metrics) const {
        if (metrics.memory_usage_bytes == 0) return 100.0;
        const uint64_t mb = 1024 * 1024;
        return 100.0 / (1.0 + std::log10(metrics.memory_usage_bytes / mb));
    }

    double calculate_throughput_score(const process_monitoring_data* proc_data) const {
        double total_jobs = 0;
        double total_latency = 0;
        
        for (const auto& [pool_id, pool_metrics] : proc_data->pool_metrics) {
            total_jobs += pool_metrics.jobs_completed;
            if (pool_metrics.jobs_completed > 0) {
                total_latency += pool_metrics.average_latency_ns;
            }
        }
        
        if (total_jobs == 0) return 0.0;
        if (total_latency == 0) return total_jobs;
        
        return total_jobs / (total_latency / 1000000.0); // jobs per millisecond
    }

    // 수집 루프
    void collection_loop() {
        while (is_active_.load()) {
            auto start_time = std::chrono::steady_clock::now();
            
            // 스냅샷 수집
            collect_snapshots();
            
            // 다음 수집까지 대기
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            auto sleep_time = std::chrono::milliseconds(collection_interval_ms_) - elapsed;
            
            if (sleep_time.count() > 0) {
                std::this_thread::sleep_for(sleep_time);
            }
        }
    }

    void collect_snapshots() {
        std::shared_lock lock(data_mutex_);
        
        for (auto& [proc_id, proc_data] : process_data_) {
            if (!proc_data->monitoring_enabled) continue;
            
            std::shared_lock proc_lock(proc_data->mutex);
            
            // 프로세스별 스냅샷 생성
            metrics_snapshot snapshot;
            snapshot.capture_time = std::chrono::steady_clock::now();
            snapshot.system = proc_data->system_metrics;
            
            // Thread pool 메트릭 집계
            thread_pool_metrics aggregated_pool{};
            for (const auto& [pool_id, pool_metrics] : proc_data->pool_metrics) {
                aggregated_pool.worker_threads += pool_metrics.worker_threads;
                aggregated_pool.idle_threads += pool_metrics.idle_threads;
                aggregated_pool.jobs_completed += pool_metrics.jobs_completed;
                aggregated_pool.jobs_pending += pool_metrics.jobs_pending;
                aggregated_pool.jobs_failed += pool_metrics.jobs_failed;
            }
            snapshot.thread_pool = aggregated_pool;
            
            // Worker 메트릭 집계
            worker_metrics aggregated_worker{};
            for (const auto& [worker_id, worker] : proc_data->worker_metrics) {
                aggregated_worker.jobs_processed += worker.jobs_processed;
                aggregated_worker.total_processing_time_ns += worker.total_processing_time_ns;
            }
            snapshot.worker = aggregated_worker;
            
            // 히스토리에 추가
            proc_lock.unlock();
            std::unique_lock write_lock(proc_data->mutex);
            proc_data->history.push(snapshot);
            
            // 전체 히스토리에도 추가
            global_history_.push(snapshot);
        }
    }

    // 멤버 변수
    std::size_t history_size_;
    std::uint32_t collection_interval_ms_;
    std::size_t max_processes_;
    std::size_t max_pools_per_process_;
    
    std::atomic<bool> is_active_;
    mutable std::shared_mutex data_mutex_;
    
    std::unordered_map<process_identifier, std::unique_ptr<process_monitoring_data>> process_data_;
    std::vector<thread_pool_identifier> registered_pools_;
    
    process_identifier default_process_;
    thread_pool_identifier default_pool_;
    
    ring_buffer<metrics_snapshot> global_history_;
    std::thread collection_thread_;
};

// multi_process_monitoring 구현
multi_process_monitoring::multi_process_monitoring(std::size_t history_size,
                                                 std::uint32_t collection_interval_ms,
                                                 std::size_t max_processes,
                                                 std::size_t max_pools_per_process)
    : pimpl_(std::make_unique<multi_process_impl>(history_size, 
                                                  collection_interval_ms,
                                                  max_processes,
                                                  max_pools_per_process)) {
}

multi_process_monitoring::~multi_process_monitoring() = default;

// 기존 인터페이스 구현
void multi_process_monitoring::update_system_metrics(const system_metrics& metrics) {
    pimpl_->update_system_metrics(metrics);
}

void multi_process_monitoring::update_thread_pool_metrics(const thread_pool_metrics& metrics) {
    pimpl_->update_thread_pool_metrics(metrics);
}

void multi_process_monitoring::update_worker_metrics(std::size_t worker_id, const worker_metrics& metrics) {
    pimpl_->update_worker_metrics(worker_id, metrics);
}

metrics_snapshot multi_process_monitoring::get_current_snapshot() const {
    return pimpl_->get_current_snapshot();
}

std::vector<metrics_snapshot> multi_process_monitoring::get_recent_snapshots(std::size_t /* count */) const {
    // 간단한 구현 - 현재 스냅샷만 반환
    std::vector<metrics_snapshot> snapshots;
    snapshots.push_back(get_current_snapshot());
    return snapshots;
}

bool multi_process_monitoring::is_active() const {
    return pimpl_->is_active();
}

void multi_process_monitoring::start() {
    pimpl_->start();
}

void multi_process_monitoring::stop() {
    pimpl_->stop();
}

double multi_process_monitoring::get_average_cpu_usage(std::chrono::steady_clock::duration /* duration */) const {
    auto snapshot = get_current_snapshot();
    return snapshot.system.cpu_usage_percent;
}

std::uint64_t multi_process_monitoring::get_peak_memory_usage(std::chrono::steady_clock::duration /* duration */) const {
    auto snapshot = get_current_snapshot();
    return snapshot.system.memory_usage_bytes;
}

double multi_process_monitoring::get_average_job_latency(std::chrono::steady_clock::duration /* duration */) const {
    auto snapshot = get_current_snapshot();
    if (snapshot.thread_pool.jobs_completed > 0) {
        return static_cast<double>(snapshot.thread_pool.average_latency_ns) / 1000000.0; // to ms
    }
    return 0.0;
}

std::unordered_map<std::string, double> multi_process_monitoring::get_statistics() const {
    auto snapshot = get_current_snapshot();
    std::unordered_map<std::string, double> stats;
    
    stats["cpu_usage_percent"] = snapshot.system.cpu_usage_percent;
    stats["memory_usage_mb"] = snapshot.system.memory_usage_bytes / (1024.0 * 1024.0);
    stats["active_threads"] = snapshot.system.active_threads;
    stats["jobs_completed"] = snapshot.thread_pool.jobs_completed;
    stats["jobs_pending"] = snapshot.thread_pool.jobs_pending;
    stats["average_latency_ms"] = snapshot.thread_pool.average_latency_ns / 1000000.0;
    
    return stats;
}

// 새로운 인터페이스 구현
void multi_process_monitoring::register_process(const process_identifier& process_id) {
    pimpl_->register_process(process_id);
}

void multi_process_monitoring::unregister_process(const process_identifier& process_id) {
    pimpl_->unregister_process(process_id);
}

void multi_process_monitoring::register_thread_pool(const thread_pool_identifier& pool_id) {
    pimpl_->register_thread_pool(pool_id);
}

void multi_process_monitoring::unregister_thread_pool(const thread_pool_identifier& pool_id) {
    pimpl_->unregister_thread_pool(pool_id);
}

void multi_process_monitoring::update_process_system_metrics(const process_identifier& process_id,
                                                           const system_metrics& metrics) {
    pimpl_->update_process_system_metrics(process_id, metrics);
}

void multi_process_monitoring::update_thread_pool_metrics(const thread_pool_identifier& pool_id,
                                                        const process_thread_pool_metrics& metrics) {
    pimpl_->update_thread_pool_metrics(pool_id, metrics);
}

void multi_process_monitoring::update_process_worker_metrics(const process_identifier& process_id,
                                                           std::size_t worker_id,
                                                           const worker_metrics& metrics) {
    pimpl_->update_process_worker_metrics(process_id, worker_id, metrics);
}

multi_process_metrics_snapshot multi_process_monitoring::get_multi_process_snapshot() const {
    return pimpl_->get_multi_process_snapshot();
}

metrics_snapshot multi_process_monitoring::get_process_snapshot(const process_identifier& process_id) const {
    return pimpl_->get_process_snapshot(process_id);
}

process_thread_pool_metrics multi_process_monitoring::get_thread_pool_metrics(const thread_pool_identifier& pool_id) const {
    auto snapshot = pimpl_->get_multi_process_snapshot();
    auto it = snapshot.thread_pool_metrics_map.find(pool_id);
    
    if (it != snapshot.thread_pool_metrics_map.end()) {
        return it->second;
    }
    
    return process_thread_pool_metrics{};
}

std::vector<process_identifier> multi_process_monitoring::get_registered_processes() const {
    return pimpl_->get_registered_processes();
}

std::vector<thread_pool_identifier> multi_process_monitoring::get_process_thread_pools(
    const process_identifier& process_id) const {
    return pimpl_->get_process_thread_pools(process_id);
}

std::unordered_map<std::string, double> multi_process_monitoring::compare_process_performance(
    const std::vector<process_identifier>& process_ids) const {
    return pimpl_->compare_process_performance(process_ids);
}

void multi_process_monitoring::set_process_monitoring_enabled(const process_identifier& /* process_id */, bool /* enabled */) {
    // TODO: Implement
}

void multi_process_monitoring::set_thread_pool_monitoring_enabled(const thread_pool_identifier& /* pool_id */, bool /* enabled */) {
    // TODO: Implement
}

void multi_process_monitoring::set_process_alert_thresholds(const process_identifier& /* process_id */,
                                                          double /* cpu_threshold */,
                                                          std::uint64_t /* memory_threshold */,
                                                          std::uint64_t /* latency_threshold_ns */) {
    // TODO: Implement
}

void multi_process_monitoring::set_thread_pool_alert_thresholds(const thread_pool_identifier& /* pool_id */,
                                                              std::uint64_t /* queue_size_threshold */,
                                                              std::uint64_t /* latency_threshold_ns */,
                                                              double /* worker_utilization_threshold */) {
    // TODO: Implement
}

void multi_process_monitoring::set_default_process(const process_identifier& process_id) {
    pimpl_->set_default_process(process_id);
}

void multi_process_monitoring::set_default_thread_pool(const thread_pool_identifier& pool_id) {
    pimpl_->set_default_thread_pool(pool_id);
}

} // namespace monitoring_module