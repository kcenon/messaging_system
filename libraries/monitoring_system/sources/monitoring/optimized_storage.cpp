/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "optimized_storage.h"
#include <algorithm>
#include <shared_mutex>

namespace monitoring_module {

// Compressed Metrics Storage 구현
compressed_metrics_storage::compressed_metrics_storage(
    std::size_t capacity,
    std::chrono::steady_clock::time_point base_time)
    : base_time_(base_time)
    , capacity_(capacity)
    , storage_(std::make_unique<compressed_metric[]>(capacity)) {
}

bool compressed_metrics_storage::store(const monitoring_interface::metrics_snapshot& snapshot) {
    std::size_t index = write_index_.fetch_add(1, std::memory_order_relaxed) % capacity_;
    
    storage_[index] = compress(snapshot);
    
    std::size_t expected_size = current_size_.load(std::memory_order_relaxed);
    while (expected_size < capacity_) {
        if (current_size_.compare_exchange_weak(expected_size, expected_size + 1)) {
            break;
        }
    }
    
    return true;
}

std::optional<monitoring_interface::metrics_snapshot> 
compressed_metrics_storage::retrieve(std::size_t index) const {
    if (index >= current_size_.load(std::memory_order_acquire)) {
        return std::nullopt;
    }
    
    return decompress(storage_[index % capacity_]);
}

std::vector<monitoring_interface::metrics_snapshot> 
compressed_metrics_storage::retrieve_range(
    std::chrono::steady_clock::time_point start_time,
    std::chrono::steady_clock::time_point end_time) const {
    
    std::vector<monitoring_interface::metrics_snapshot> results;
    std::size_t size = current_size_.load(std::memory_order_acquire);
    
    for (std::size_t i = 0; i < size; ++i) {
        auto snapshot = retrieve(i);
        if (snapshot && snapshot->capture_time >= start_time && 
            snapshot->capture_time <= end_time) {
            results.push_back(*snapshot);
        }
    }
    
    return results;
}

double compressed_metrics_storage::compression_ratio() const {
    // 원본 크기 대비 압축 크기 비율
    constexpr std::size_t original_size = sizeof(monitoring_interface::metrics_snapshot);
    constexpr std::size_t compressed_size = sizeof(compressed_metric);
    return static_cast<double>(compressed_size) / original_size;
}

compressed_metrics_storage::compressed_metric 
compressed_metrics_storage::compress(const monitoring_interface::metrics_snapshot& snapshot) const {
    compressed_metric compressed{};
    
    // 시간 압축 (초 단위 오프셋)
    auto duration = snapshot.capture_time - base_time_;
    compressed.timestamp_offset = static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    
    // CPU 압축 (0.01% 단위)
    compressed.cpu_percent = static_cast<std::uint16_t>(
        snapshot.system.cpu_usage_percent * 100);
    
    // 메모리 압축 (MB 단위)
    compressed.memory_mb = static_cast<std::uint32_t>(
        snapshot.system.memory_usage_bytes / (1024 * 1024));
    
    // 스레드 수
    compressed.thread_count = static_cast<std::uint16_t>(
        snapshot.system.active_threads);
    
    // Thread pool 메트릭
    compressed.jobs_completed = static_cast<std::uint32_t>(
        snapshot.thread_pool.jobs_completed);
    compressed.queue_depth = static_cast<std::uint16_t>(
        snapshot.thread_pool.jobs_pending);
    compressed.latency_ms = static_cast<std::uint16_t>(
        snapshot.thread_pool.average_latency_ns / 1000000);
    
    return compressed;
}

monitoring_interface::metrics_snapshot 
compressed_metrics_storage::decompress(const compressed_metric& compressed) const {
    monitoring_interface::metrics_snapshot snapshot{};
    
    // 시간 복원
    snapshot.capture_time = base_time_ + std::chrono::seconds(compressed.timestamp_offset);
    
    // 시스템 메트릭 복원
    snapshot.system.cpu_usage_percent = compressed.cpu_percent / 100;
    snapshot.system.memory_usage_bytes = 
        static_cast<std::uint64_t>(compressed.memory_mb) * 1024 * 1024;
    snapshot.system.active_threads = compressed.thread_count;
    
    // Thread pool 메트릭 복원
    snapshot.thread_pool.jobs_completed = compressed.jobs_completed;
    snapshot.thread_pool.jobs_pending = compressed.queue_depth;
    snapshot.thread_pool.average_latency_ns = 
        static_cast<std::uint64_t>(compressed.latency_ms) * 1000000;
    
    return snapshot;
}

// Tiered Storage 구현
tiered_storage::tiered_storage(std::size_t hot_capacity,
                             std::size_t warm_capacity,
                             std::size_t cold_capacity)
    : hot_tier_(hot_capacity)
    , warm_tier_(std::make_unique<compressed_metrics_storage>(
        warm_capacity, std::chrono::steady_clock::now()))
    , cold_tier_(std::make_unique<compressed_metrics_storage>(
        cold_capacity, std::chrono::steady_clock::now()))
    , last_aging_(std::chrono::steady_clock::now()) {
}

void tiered_storage::store(const monitoring_interface::metrics_snapshot& snapshot) {
    // 핫 계층에 저장
    if (!hot_tier_.enqueue(snapshot)) {
        // 핫 계층이 가득 찬 경우 에이징 수행
        perform_aging();
        hot_tier_.enqueue(snapshot);  // 재시도
    }
}

std::optional<monitoring_interface::metrics_snapshot> 
tiered_storage::retrieve(std::chrono::steady_clock::time_point time_point) const {
    std::shared_lock<std::shared_mutex> lock(tier_mutex_);
    
    // 핫 계층에서 검색 (가장 최근 데이터)
    // 간단한 구현을 위해 핫 계층은 건너뜀
    
    // 웜 계층에서 검색
    auto warm_results = warm_tier_->retrieve_range(time_point, time_point);
    if (!warm_results.empty()) {
        return warm_results.front();
    }
    
    // 콜드 계층에서 검색
    auto cold_results = cold_tier_->retrieve_range(time_point, time_point);
    if (!cold_results.empty()) {
        return cold_results.front();
    }
    
    return std::nullopt;
}

void tiered_storage::perform_aging() {
    std::unique_lock<std::shared_mutex> lock(tier_mutex_);
    
    // 핫 -> 웜 이동
    monitoring_interface::metrics_snapshot snapshot;
    std::vector<monitoring_interface::metrics_snapshot> to_warm;
    
    // 핫 계층의 절반을 웜으로 이동
    std::size_t to_move = hot_tier_.size() / 2;
    for (std::size_t i = 0; i < to_move; ++i) {
        if (hot_tier_.dequeue(snapshot)) {
            to_warm.push_back(snapshot);
        }
    }
    
    // 웜 계층에 저장
    for (const auto& s : to_warm) {
        warm_tier_->store(s);
    }
    
    // 웜 -> 콜드 이동 (1시간 이상 된 데이터)
    // TODO: 실제 구현에서는 웜 계층의 오래된 데이터를 콜드로 이동
    
    last_aging_ = std::chrono::steady_clock::now();
}

tiered_storage::memory_stats tiered_storage::get_memory_stats() const {
    memory_stats stats{};
    
    // 핫 계층 (원본 크기)
    stats.hot_tier_bytes = hot_tier_.size() * sizeof(monitoring_interface::metrics_snapshot);
    
    // 웜 계층 (압축)
    stats.warm_tier_bytes = warm_tier_->memory_usage();
    
    // 콜드 계층 (고압축)
    stats.cold_tier_bytes = cold_tier_->memory_usage();
    
    stats.total_bytes = stats.hot_tier_bytes + stats.warm_tier_bytes + stats.cold_tier_bytes;
    
    return stats;
}

// Batch Metrics Processor 구현
batch_metrics_processor::batch_metrics_processor(
    std::size_t batch_size,
    std::chrono::milliseconds flush_interval,
    batch_callback callback)
    : batch_size_(batch_size)
    , flush_interval_(flush_interval)
    , callback_(callback) {
    batch_.reserve(batch_size);
}

batch_metrics_processor::~batch_metrics_processor() {
    stop();
}

void batch_metrics_processor::add(const monitoring_interface::metrics_snapshot& snapshot) {
    std::unique_lock lock(batch_mutex_);
    
    batch_.push_back(snapshot);
    
    if (batch_.size() >= batch_size_) {
        process_batch();
    }
}

void batch_metrics_processor::flush() {
    std::unique_lock lock(batch_mutex_);
    if (!batch_.empty()) {
        process_batch();
    }
    stats_.flush_count.fetch_add(1, std::memory_order_relaxed);
}

void batch_metrics_processor::start() {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
        return;  // 이미 실행 중
    }
    
    processor_thread_ = std::thread(&batch_metrics_processor::process_loop, this);
}

void batch_metrics_processor::stop() {
    running_.store(false);
    batch_cv_.notify_all();
    
    if (processor_thread_.joinable()) {
        processor_thread_.join();
    }
    
    // 남은 배치 처리
    flush();
}

void batch_metrics_processor::process_loop() {
    while (running_.load()) {
        std::unique_lock lock(batch_mutex_);
        
        // 플러시 간격만큼 대기
        batch_cv_.wait_for(lock, flush_interval_, [this] {
            return !running_.load() || batch_.size() >= batch_size_;
        });
        
        if (!batch_.empty()) {
            process_batch();
        }
    }
}

void batch_metrics_processor::process_batch() {
    if (batch_.empty()) {
        return;
    }
    
    // 배치 복사 (콜백 실행 중 블로킹 방지)
    std::vector<monitoring_interface::metrics_snapshot> current_batch;
    batch_.swap(current_batch);
    
    // 통계 업데이트
    stats_.batches_processed.fetch_add(1, std::memory_order_relaxed);
    stats_.metrics_processed.fetch_add(current_batch.size(), std::memory_order_relaxed);
    
    // 콜백 실행
    if (callback_) {
        callback_(current_batch);
    }
}

} // namespace monitoring_module