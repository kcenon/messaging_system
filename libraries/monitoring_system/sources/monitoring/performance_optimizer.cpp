/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "performance_optimizer.h"
#include <algorithm>
#include <random>

namespace monitoring_module {

// Performance Optimizer 구현
performance_optimizer::performance_optimizer(const optimization_config& config)
    : config_(config) {
    
    if (config_.enable_tiered_storage) {
        storage_ = std::make_unique<tiered_storage>(1024, 4096, 8192);
    }
    
    if (config_.enable_batching) {
        batch_processor_ = std::make_unique<batch_metrics_processor>(
            config_.batch_size,
            config_.batch_interval,
            [this](const std::vector<monitoring_interface::metrics_snapshot>& batch) {
                // 배치 처리 로직
                for (const auto& snapshot : batch) {
                    if (storage_) {
                        storage_->store(snapshot);
                    }
                }
            });
        batch_processor_->start();
    }
    
    if (config_.enable_compression) {
        compression_buffer_ = std::make_unique<compressed_metrics_storage>(
            config_.compression_threshold, std::chrono::steady_clock::now());
    }
}

bool performance_optimizer::optimize_metric(const monitoring_interface::metrics_snapshot& snapshot) {
    // 적응형 샘플링 확인
    if (config_.enable_adaptive_sampling) {
        // 프로세스 ID가 없으므로 전체 시스템 기준으로 처리
        monitoring_interface::process_identifier dummy_id{0, "system", snapshot.capture_time};
        if (!should_sample(dummy_id)) {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.samples_skipped++;
            return false;
        }
    }
    
    // 배치 처리
    if (config_.enable_batching && batch_processor_) {
        batch_processor_->add(snapshot);
        return true;
    }
    
    // 압축 저장
    if (config_.enable_compression && compression_buffer_) {
        auto before_size = sizeof(snapshot);
        compression_buffer_->store(snapshot);
        auto after_size = sizeof(compressed_metrics_storage::compressed_metric);
        
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.memory_saved_bytes += (before_size - after_size);
        stats_.compression_ratio = compression_buffer_->compression_ratio();
    }
    
    // 계층형 저장
    if (config_.enable_tiered_storage && storage_) {
        storage_->store(snapshot);
    }
    
    return true;
}

void performance_optimizer::adjust_sampling_rate(
    const monitoring_interface::process_identifier& process_id,
    double current_load) {
    
    std::lock_guard<std::mutex> lock(sampling_mutex_);
    auto& state = sampling_states_[process_id];
    
    // 부하에 따른 샘플링 레이트 조정
    if (current_load > 80.0) {
        // 고부하: 샘플링 감소
        state.rate = std::max(0.1, state.rate * 0.8);
    } else if (current_load < 30.0) {
        // 저부하: 샘플링 증가
        state.rate = std::min(1.0, state.rate * 1.2);
    }
}

void performance_optimizer::adapt_to_memory_pressure(double memory_pressure) {
    if (memory_pressure > 0.8) {
        // 높은 메모리 압력: 압축 강화, 배치 크기 감소
        config_.enable_compression = true;
        config_.batch_size = std::max<std::size_t>(10, config_.batch_size / 2);
        
        // 계층형 저장소 에이징 트리거
        if (storage_) {
            storage_->perform_aging();
        }
    } else if (memory_pressure < 0.3) {
        // 낮은 메모리 압력: 성능 우선
        config_.batch_size = std::min<std::size_t>(1000, config_.batch_size * 2);
    }
}

void performance_optimizer::adapt_to_cpu_load(double cpu_load) {
    if (cpu_load > 80.0) {
        // 높은 CPU 부하: 처리 간격 증가
        config_.batch_interval = std::chrono::milliseconds(
            std::min(1000, static_cast<int>(config_.batch_interval.count() * 1.5)));
    } else if (cpu_load < 30.0) {
        // 낮은 CPU 부하: 처리 간격 감소
        config_.batch_interval = std::chrono::milliseconds(
            std::max(10, static_cast<int>(config_.batch_interval.count() * 0.8)));
    }
}

performance_optimizer::optimization_stats performance_optimizer::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto stats = stats_;
    
    if (batch_processor_) {
        stats.batches_processed = batch_processor_->get_stats().batches_processed;
    }
    
    return stats;
}

void performance_optimizer::set_batch_callback(batch_metrics_processor::batch_callback callback) {
    if (batch_processor_) {
        batch_processor_->stop();
        batch_processor_ = std::make_unique<batch_metrics_processor>(
            config_.batch_size, config_.batch_interval, callback);
        batch_processor_->start();
    }
}

bool performance_optimizer::should_sample(const monitoring_interface::process_identifier& process_id) {
    std::lock_guard<std::mutex> lock(sampling_mutex_);
    auto& state = sampling_states_[process_id];
    
    // 확률적 샘플링
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    if (dis(gen) < state.rate) {
        state.last_sample = std::chrono::steady_clock::now();
        return true;
    }
    
    state.skip_count++;
    return false;
}

void performance_optimizer::update_stats(std::size_t memory_saved, std::size_t cpu_saved) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.memory_saved_bytes += memory_saved;
    stats_.cpu_cycles_saved += cpu_saved;
}

// Auto Scaler 구현
auto_scaler::auto_scaler(const scaling_policy& policy)
    : policy_(policy)
    , last_scale_time_(std::chrono::steady_clock::now() - policy.cooldown) {
}

auto_scaler::scaling_decision auto_scaler::decide(
    const monitoring_interface::metrics_snapshot& current_metrics) {
    
    scaling_decision decision;
    
    // 쿨다운 확인
    if (is_in_cooldown()) {
        decision.recommended_action = scaling_decision::action::none;
        decision.reason = "In cooldown period";
        return decision;
    }
    
    // 메트릭 평활화
    double cpu_load = current_metrics.system.cpu_usage_percent;
    double memory_load = (current_metrics.system.memory_usage_bytes / (4.0 * 1024 * 1024 * 1024)) * 100;
    update_smoothed_metrics(cpu_load, memory_load);
    
    // 스케일링 결정
    if (state_.smoothed_cpu_load > policy_.cpu_threshold_up || 
        state_.smoothed_memory_load > policy_.memory_threshold_up) {
        
        decision.recommended_action = scaling_decision::action::scale_up;
        decision.confidence = std::max(
            (state_.smoothed_cpu_load - policy_.cpu_threshold_up) / 20.0,
            (state_.smoothed_memory_load - policy_.memory_threshold_up) / 20.0);
        decision.confidence = std::min(1.0, std::max(0.0, decision.confidence));
        decision.recommended_resources = static_cast<std::size_t>(
            state_.current_resources * policy_.scale_factor);
        decision.reason = "High resource utilization";
        
    } else if (state_.smoothed_cpu_load < policy_.cpu_threshold_down && 
               state_.smoothed_memory_load < policy_.memory_threshold_down) {
        
        decision.recommended_action = scaling_decision::action::scale_down;
        decision.confidence = std::max(
            (policy_.cpu_threshold_down - state_.smoothed_cpu_load) / 20.0,
            (policy_.memory_threshold_down - state_.smoothed_memory_load) / 20.0);
        decision.confidence = std::min(1.0, std::max(0.0, decision.confidence));
        decision.recommended_resources = std::max<std::size_t>(1,
            static_cast<std::size_t>(state_.current_resources / policy_.scale_factor));
        decision.reason = "Low resource utilization";
    }
    
    // 결정 기록
    if (decision.recommended_action != scaling_decision::action::none) {
        record_decision(decision);
        last_scale_time_ = std::chrono::steady_clock::now();
    }
    
    return decision;
}

auto_scaler::scaling_decision auto_scaler::decide_predictive(
    double predicted_load, std::chrono::seconds time_horizon) {
    
    scaling_decision decision;
    
    // 예측 기반 스케일링
    if (predicted_load > policy_.cpu_threshold_up) {
        decision.recommended_action = scaling_decision::action::scale_up;
        decision.confidence = (predicted_load - policy_.cpu_threshold_up) / 20.0;
        decision.reason = "Predicted high load in " + 
                         std::to_string(time_horizon.count()) + " seconds";
    } else if (predicted_load < policy_.cpu_threshold_down) {
        decision.recommended_action = scaling_decision::action::scale_down;
        decision.confidence = (policy_.cpu_threshold_down - predicted_load) / 20.0;
        decision.reason = "Predicted low load in " + 
                         std::to_string(time_horizon.count()) + " seconds";
    }
    
    decision.confidence = std::min(1.0, std::max(0.0, decision.confidence));
    
    return decision;
}

std::vector<std::pair<std::chrono::steady_clock::time_point, auto_scaler::scaling_decision>>
auto_scaler::get_history(std::size_t count) const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    std::vector<std::pair<std::chrono::steady_clock::time_point, scaling_decision>> result;
    auto start = history_.size() > count ? history_.end() - count : history_.begin();
    std::copy(start, history_.end(), std::back_inserter(result));
    
    return result;
}

bool auto_scaler::is_in_cooldown() const {
    auto now = std::chrono::steady_clock::now();
    return (now - last_scale_time_) < policy_.cooldown;
}

void auto_scaler::update_smoothed_metrics(double cpu_load, double memory_load) {
    const double alpha = 0.3;  // 평활화 계수
    state_.smoothed_cpu_load = alpha * cpu_load + (1 - alpha) * state_.smoothed_cpu_load;
    state_.smoothed_memory_load = alpha * memory_load + (1 - alpha) * state_.smoothed_memory_load;
}

void auto_scaler::record_decision(const scaling_decision& decision) {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    history_.emplace_back(std::chrono::steady_clock::now(), decision);
    
    // 히스토리 크기 제한
    if (history_.size() > 100) {
        history_.pop_front();
    }
    
    // 리소스 상태 업데이트
    if (decision.recommended_action == scaling_decision::action::scale_up) {
        state_.current_resources = decision.recommended_resources;
    } else if (decision.recommended_action == scaling_decision::action::scale_down) {
        state_.current_resources = decision.recommended_resources;
    }
}

// Distributed Aggregator 구현
distributed_aggregator::distributed_aggregator(const aggregation_config& config)
    : config_(config) {
    
    if (config_.enable_parallel_aggregation) {
        aggregation_thread_ = std::thread([this] {
            while (aggregating_.load()) {
                std::this_thread::sleep_for(config_.aggregation_interval);
                auto result = aggregate_global();
                if (callback_) {
                    callback_(result);
                }
            }
        });
    }
}

void distributed_aggregator::add_local_metrics(
    const std::string& node_id,
    const monitoring_interface::multi_process_metrics_snapshot& snapshot) {
    
    std::unique_lock<std::shared_mutex> lock(data_mutex_);
    
    auto& node = node_metrics_[node_id];
    node.latest_snapshot = snapshot;
    node.last_update = std::chrono::steady_clock::now();
    node.is_active = true;
}

monitoring_interface::multi_process_metrics_snapshot 
distributed_aggregator::aggregate_global() {
    monitoring_interface::multi_process_metrics_snapshot result;
    result.capture_time = std::chrono::steady_clock::now();
    
    if (config_.enable_parallel_aggregation) {
        parallel_aggregate(result);
    } else {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        
        for (const auto& [node_id, node] : node_metrics_) {
            if (!node.is_active) continue;
            
            // 시스템 메트릭 집계
            result.global_system.cpu_usage_percent += 
                node.latest_snapshot.global_system.cpu_usage_percent;
            result.global_system.memory_usage_bytes += 
                node.latest_snapshot.global_system.memory_usage_bytes;
            result.global_system.active_threads += 
                node.latest_snapshot.global_system.active_threads;
            
            // 프로세스별 메트릭 병합
            for (const auto& [proc_id, metrics] : node.latest_snapshot.process_system_metrics) {
                result.process_system_metrics[proc_id] = metrics;
            }
            
            // Thread pool 메트릭 병합
            for (const auto& [pool_id, metrics] : node.latest_snapshot.thread_pool_metrics_map) {
                result.thread_pool_metrics_map[pool_id] = metrics;
            }
        }
        
        // CPU 사용률 평균 계산
        if (!node_metrics_.empty()) {
            result.global_system.cpu_usage_percent /= node_metrics_.size();
        }
    }
    
    return result;
}

void distributed_aggregator::set_aggregation_callback(aggregation_callback callback) {
    callback_ = callback;
}

std::unordered_map<std::string, std::chrono::steady_clock::time_point>
distributed_aggregator::get_node_status() const {
    std::shared_lock<std::shared_mutex> lock(data_mutex_);
    
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> status;
    for (const auto& [node_id, node] : node_metrics_) {
        if (node.is_active) {
            status[node_id] = node.last_update;
        }
    }
    
    return status;
}

void distributed_aggregator::parallel_aggregate(
    monitoring_interface::multi_process_metrics_snapshot& result) {
    
    // 병렬 집계 구현 (간단한 버전)
    std::shared_lock<std::shared_mutex> lock(data_mutex_);
    
    std::atomic<std::uint64_t> total_cpu{0};
    std::atomic<std::uint64_t> total_memory{0};
    std::atomic<std::uint64_t> total_threads{0};
    
    // 각 노드를 병렬로 처리
    std::vector<std::thread> workers;
    for (const auto& [node_id, node] : node_metrics_) {
        if (!node.is_active) continue;
        
        workers.emplace_back([&total_cpu, &total_memory, &total_threads, &node] {
            total_cpu.fetch_add(node.latest_snapshot.global_system.cpu_usage_percent);
            total_memory.fetch_add(node.latest_snapshot.global_system.memory_usage_bytes);
            total_threads.fetch_add(node.latest_snapshot.global_system.active_threads);
        });
    }
    
    // 워커 대기
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    // 결과 설정
    result.global_system.cpu_usage_percent = total_cpu.load() / std::max<std::size_t>(1, node_metrics_.size());
    result.global_system.memory_usage_bytes = total_memory.load();
    result.global_system.active_threads = total_threads.load();
}

void distributed_aggregator::incremental_aggregate(
    const std::string& /* node_id */,
    const monitoring_interface::multi_process_metrics_snapshot& /* snapshot */,
    monitoring_interface::multi_process_metrics_snapshot& result) {
    
    // 증분 집계 구현
    // 이전 값을 빼고 새 값을 더함
    // 실제 구현에서는 더 정교한 증분 알고리즘 필요
}

} // namespace monitoring_module