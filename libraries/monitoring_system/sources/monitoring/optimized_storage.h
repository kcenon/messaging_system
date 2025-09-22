#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/multi_process_monitoring_interface.h"
#include <memory>
#include <atomic>
#include <vector>
#include <array>
#include <cstring>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <optional>

namespace monitoring_module {

/**
 * @class compressed_metrics_storage
 * @brief 압축된 메트릭 저장소
 * 
 * 메모리 효율적인 메트릭 저장을 위한 압축 스토리지
 */
class compressed_metrics_storage {
public:
    /**
     * @struct compressed_metric
     * @brief 압축된 메트릭 데이터
     */
    struct compressed_metric {
        std::uint32_t timestamp_offset;  // 기준 시간으로부터의 오프셋 (초)
        std::uint16_t cpu_percent;       // CPU 사용률 (0.01% 단위)
        std::uint32_t memory_mb;         // 메모리 사용량 (MB)
        std::uint16_t thread_count;      // 스레드 수
        std::uint32_t jobs_completed;    // 완료된 작업 수
        std::uint16_t queue_depth;       // 큐 깊이
        std::uint16_t latency_ms;        // 평균 지연시간 (ms)
    } __attribute__((packed));
    
    static_assert(sizeof(compressed_metric) == 20, "Compressed metric size mismatch");
    
    /**
     * @brief 생성자
     * @param capacity 저장 용량
     * @param base_time 기준 시간
     */
    explicit compressed_metrics_storage(
        std::size_t capacity,
        std::chrono::steady_clock::time_point base_time);
    
    /**
     * @brief 메트릭 압축 및 저장
     * @param snapshot 원본 메트릭 스냅샷
     * @return 저장 성공 여부
     */
    bool store(const monitoring_interface::metrics_snapshot& snapshot);
    
    /**
     * @brief 압축된 메트릭 복원
     * @param index 인덱스
     * @return 복원된 메트릭 (없으면 nullopt)
     */
    std::optional<monitoring_interface::metrics_snapshot> retrieve(std::size_t index) const;
    
    /**
     * @brief 시간 범위로 메트릭 조회
     * @param start_time 시작 시간
     * @param end_time 종료 시간
     * @return 해당 범위의 메트릭들
     */
    std::vector<monitoring_interface::metrics_snapshot> retrieve_range(
        std::chrono::steady_clock::time_point start_time,
        std::chrono::steady_clock::time_point end_time) const;
    
    /**
     * @brief 현재 저장된 메트릭 수
     * @return 메트릭 개수
     */
    std::size_t size() const { return current_size_.load(); }
    
    /**
     * @brief 사용된 메모리 크기
     * @return 바이트 단위 메모리 사용량
     */
    std::size_t memory_usage() const {
        return sizeof(compressed_metric) * capacity_ + sizeof(*this);
    }
    
    /**
     * @brief 압축률 계산
     * @return 압축률 (0-1, 1이 압축 안됨)
     */
    double compression_ratio() const;
    
private:
    std::chrono::steady_clock::time_point base_time_;
    std::size_t capacity_;
    std::atomic<std::size_t> current_size_{0};
    std::atomic<std::size_t> write_index_{0};
    std::unique_ptr<compressed_metric[]> storage_;
    
    compressed_metric compress(const monitoring_interface::metrics_snapshot& snapshot) const;
    monitoring_interface::metrics_snapshot decompress(const compressed_metric& compressed) const;
};

/**
 * @class lock_free_metrics_queue
 * @brief 락프리 메트릭 큐
 * 
 * 고성능 메트릭 수집을 위한 락프리 큐 구현
 */
template<typename T>
class lock_free_metrics_queue {
public:
    explicit lock_free_metrics_queue(std::size_t capacity)
        : capacity_(capacity)
        , mask_(capacity - 1)
        , buffer_(std::make_unique<slot[]>(capacity))
        , head_(0)
        , tail_(0) {
        
        // capacity는 2의 제곱수여야 함
        if ((capacity & (capacity - 1)) != 0) {
            throw std::invalid_argument("Capacity must be power of 2");
        }
        
        // 슬롯 초기화
        for (std::size_t i = 0; i < capacity; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }
    
    bool enqueue(T item) {
        cell* cell;
        std::size_t pos = head_.load(std::memory_order_relaxed);
        
        for (;;) {
            cell = &buffer_[pos & mask_];
            std::size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            
            if (dif == 0) {
                if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (dif < 0) {
                return false;  // 큐 가득 참
            } else {
                pos = head_.load(std::memory_order_relaxed);
            }
        }
        
        cell->data = std::move(item);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }
    
    bool dequeue(T& item) {
        cell* cell;
        std::size_t pos = tail_.load(std::memory_order_relaxed);
        
        for (;;) {
            cell = &buffer_[pos & mask_];
            std::size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            
            if (dif == 0) {
                if (tail_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (dif < 0) {
                return false;  // 큐 비어있음
            } else {
                pos = tail_.load(std::memory_order_relaxed);
            }
        }
        
        item = std::move(cell->data);
        cell->sequence.store(pos + mask_ + 1, std::memory_order_release);
        return true;
    }
    
    std::size_t size() const {
        std::size_t head = head_.load(std::memory_order_acquire);
        std::size_t tail = tail_.load(std::memory_order_acquire);
        return head - tail;
    }
    
    bool empty() const {
        return size() == 0;
    }
    
private:
    struct cell {
        std::atomic<std::size_t> sequence;
        T data;
    };
    
    using slot = cell;
    using cacheline_pad_t = char[64];
    
    cacheline_pad_t pad0_;
    const std::size_t capacity_;
    const std::size_t mask_;
    std::unique_ptr<slot[]> buffer_;
    
    cacheline_pad_t pad1_;
    std::atomic<std::size_t> head_;
    
    cacheline_pad_t pad2_;
    std::atomic<std::size_t> tail_;
    
    cacheline_pad_t pad3_;
};

/**
 * @class tiered_storage
 * @brief 계층형 저장소
 * 
 * 핫/웜/콜드 데이터 계층을 관리하는 저장소
 */
class tiered_storage {
public:
    /**
     * @brief 생성자
     * @param hot_capacity 핫 계층 용량
     * @param warm_capacity 웜 계층 용량
     * @param cold_capacity 콜드 계층 용량
     */
    tiered_storage(std::size_t hot_capacity,
                   std::size_t warm_capacity,
                   std::size_t cold_capacity);
    
    /**
     * @brief 메트릭 저장
     * @param snapshot 메트릭 스냅샷
     */
    void store(const monitoring_interface::metrics_snapshot& snapshot);
    
    /**
     * @brief 시간 기반 조회
     * @param time_point 조회 시점
     * @return 메트릭 스냅샷
     */
    std::optional<monitoring_interface::metrics_snapshot> 
    retrieve(std::chrono::steady_clock::time_point time_point) const;
    
    /**
     * @brief 데이터 에이징 수행
     * 
     * 오래된 데이터를 하위 계층으로 이동
     */
    void perform_aging();
    
    /**
     * @brief 메모리 사용량 조회
     * @return 계층별 메모리 사용량
     */
    struct memory_stats {
        std::size_t hot_tier_bytes;
        std::size_t warm_tier_bytes;
        std::size_t cold_tier_bytes;
        std::size_t total_bytes;
    };
    memory_stats get_memory_stats() const;
    
private:
    // 핫 계층: 최근 데이터, 빠른 접근
    lock_free_metrics_queue<monitoring_interface::metrics_snapshot> hot_tier_;
    
    // 웜 계층: 중간 데이터, 압축 저장
    std::unique_ptr<compressed_metrics_storage> warm_tier_;
    
    // 콜드 계층: 오래된 데이터, 고압축
    std::unique_ptr<compressed_metrics_storage> cold_tier_;
    
    mutable std::shared_mutex tier_mutex_;
    std::chrono::steady_clock::time_point last_aging_;
};

/**
 * @class batch_metrics_processor
 * @brief 배치 메트릭 프로세서
 * 
 * 메트릭을 배치로 처리하여 효율성 향상
 */
class batch_metrics_processor {
public:
    using batch_callback = std::function<void(
        const std::vector<monitoring_interface::metrics_snapshot>&)>;
    
    /**
     * @brief 생성자
     * @param batch_size 배치 크기
     * @param flush_interval 플러시 간격
     * @param callback 배치 처리 콜백
     */
    batch_metrics_processor(
        std::size_t batch_size,
        std::chrono::milliseconds flush_interval,
        batch_callback callback);
    
    ~batch_metrics_processor();
    
    /**
     * @brief 메트릭 추가
     * @param snapshot 메트릭 스냅샷
     */
    void add(const monitoring_interface::metrics_snapshot& snapshot);
    
    /**
     * @brief 강제 플러시
     */
    void flush();
    
    /**
     * @brief 처리 시작
     */
    void start();
    
    /**
     * @brief 처리 중지
     */
    void stop();
    
    /**
     * @brief 통계 조회
     */
    struct stats {
        std::atomic<std::size_t> batches_processed{0};
        std::atomic<std::size_t> metrics_processed{0};
        std::atomic<std::size_t> flush_count{0};
    };
    const stats& get_stats() const { return stats_; }
    
private:
    const std::size_t batch_size_;
    const std::chrono::milliseconds flush_interval_;
    batch_callback callback_;
    
    std::vector<monitoring_interface::metrics_snapshot> batch_;
    std::mutex batch_mutex_;
    std::condition_variable batch_cv_;
    
    std::atomic<bool> running_{false};
    std::thread processor_thread_;
    stats stats_;
    
    void process_loop();
    void process_batch();
};

} // namespace monitoring_module