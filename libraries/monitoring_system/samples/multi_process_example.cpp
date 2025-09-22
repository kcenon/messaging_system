/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <monitoring/multi_process_monitoring.h>
#include <thread>
#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <unistd.h>

using namespace monitoring_interface;
using namespace monitoring_module;

// 시뮬레이션된 워커 스레드
class simulated_worker {
public:
    simulated_worker(std::size_t id, 
                    thread_pool_identifier pool_id,
                    std::shared_ptr<multi_process_monitoring> monitor)
        : worker_id_(id)
        , pool_id_(pool_id)
        , monitor_(monitor)
        , running_(true)
        , jobs_processed_(0)
        , total_processing_time_ns_(0) {
        worker_thread_ = std::thread(&simulated_worker::work_loop, this);
    }
    
    ~simulated_worker() {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }
    
    std::size_t get_jobs_processed() const { return jobs_processed_; }
    std::uint64_t get_total_processing_time() const { return total_processing_time_ns_; }
    
private:
    void work_loop() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> work_time(1, 10);
        std::uniform_int_distribution<> idle_time(5, 20);
        
        while (running_) {
            // 작업 시뮬레이션
            auto start = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(work_time(gen)));
            auto end = std::chrono::steady_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            jobs_processed_++;
            total_processing_time_ns_ += duration;
            
            // 워커 메트릭 업데이트
            worker_metrics metrics;
            metrics.jobs_processed = jobs_processed_;
            metrics.total_processing_time_ns = total_processing_time_ns_;
            metrics.idle_time_ns = 0; // 간단화를 위해 생략
            
            monitor_->update_process_worker_metrics(pool_id_.process_id, worker_id_, metrics);
            
            // 유휴 시간
            std::this_thread::sleep_for(std::chrono::milliseconds(idle_time(gen)));
        }
    }
    
    std::size_t worker_id_;
    thread_pool_identifier pool_id_;
    std::shared_ptr<multi_process_monitoring> monitor_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    std::atomic<std::size_t> jobs_processed_;
    std::atomic<std::uint64_t> total_processing_time_ns_;
};

// 시뮬레이션된 스레드 풀
class simulated_thread_pool {
public:
    simulated_thread_pool(thread_pool_identifier pool_id,
                         std::size_t worker_count,
                         std::shared_ptr<multi_process_monitoring> monitor)
        : pool_id_(pool_id)
        , monitor_(monitor)
        , jobs_completed_(0)
        , jobs_pending_(0) {
        
        // 워커 생성
        for (std::size_t i = 0; i < worker_count; ++i) {
            workers_.push_back(std::make_unique<simulated_worker>(i, pool_id, monitor));
        }
    }
    
    void update_metrics() {
        process_thread_pool_metrics metrics;
        metrics.pool_id = pool_id_;
        metrics.worker_threads = workers_.size();
        metrics.idle_threads = 0; // 간단화
        metrics.jobs_completed = jobs_completed_++;
        metrics.jobs_pending = jobs_pending_;
        metrics.jobs_failed = 0;
        metrics.total_execution_time_ns = 0;
        metrics.average_latency_ns = 5000000; // 5ms
        
        // 워커별 부하 분산
        for (const auto& worker : workers_) {
            metrics.worker_load_distribution.push_back(worker->get_jobs_processed());
        }
        
        monitor_->update_thread_pool_metrics(pool_id_, metrics);
    }
    
private:
    thread_pool_identifier pool_id_;
    std::shared_ptr<multi_process_monitoring> monitor_;
    std::vector<std::unique_ptr<simulated_worker>> workers_;
    std::atomic<std::uint64_t> jobs_completed_;
    std::atomic<std::uint64_t> jobs_pending_;
};

// 프로세스 시뮬레이터
class process_simulator {
public:
    process_simulator(process_identifier process_id,
                     std::shared_ptr<multi_process_monitoring> monitor)
        : process_id_(process_id)
        , monitor_(monitor) {
        monitor_->register_process(process_id_);
    }
    
    ~process_simulator() {
        thread_pools_.clear();
        monitor_->unregister_process(process_id_);
    }
    
    void add_thread_pool(const std::string& pool_name, std::size_t instance_id, std::size_t worker_count) {
        thread_pool_identifier pool_id{
            .process_id = process_id_,
            .pool_name = pool_name,
            .pool_instance_id = static_cast<std::uint32_t>(instance_id)
        };
        
        monitor_->register_thread_pool(pool_id);
        thread_pools_.push_back(std::make_unique<simulated_thread_pool>(pool_id, worker_count, monitor_));
    }
    
    void update_system_metrics() {
        system_metrics metrics;
        metrics.cpu_usage_percent = 20 + (rand() % 30);
        metrics.memory_usage_bytes = 100 * 1024 * 1024 + (rand() % (50 * 1024 * 1024));
        metrics.active_threads = 0;
        
        for (const auto& pool : thread_pools_) {
            metrics.active_threads += 4; // 간단화
        }
        
        monitor_->update_process_system_metrics(process_id_, metrics);
    }
    
    void update_pool_metrics() {
        for (auto& pool : thread_pools_) {
            pool->update_metrics();
        }
    }
    
private:
    process_identifier process_id_;
    std::shared_ptr<multi_process_monitoring> monitor_;
    std::vector<std::unique_ptr<simulated_thread_pool>> thread_pools_;
};

// 결과 출력 헬퍼
void print_multi_process_snapshot(const multi_process_metrics_snapshot& snapshot) {
    std::cout << "\n=== Multi-Process Monitoring Snapshot ===\n";
    std::cout << std::fixed << std::setprecision(2);
    
    // 전체 시스템 메트릭
    std::cout << "\nGlobal System Metrics:\n";
    std::cout << "  CPU Usage: " << snapshot.global_system.cpu_usage_percent << "%\n";
    std::cout << "  Memory: " << (snapshot.global_system.memory_usage_bytes / (1024.0 * 1024.0)) << " MB\n";
    std::cout << "  Active Threads: " << snapshot.global_system.active_threads << "\n";
    
    // 프로세스별 메트릭
    std::cout << "\nProcess Metrics:\n";
    for (const auto& [proc_id, sys_metrics] : snapshot.process_system_metrics) {
        std::cout << "  Process: " << proc_id.process_name << " (PID: " << proc_id.pid << ")\n";
        std::cout << "    CPU: " << sys_metrics.cpu_usage_percent << "%\n";
        std::cout << "    Memory: " << (sys_metrics.memory_usage_bytes / (1024.0 * 1024.0)) << " MB\n";
        std::cout << "    Threads: " << sys_metrics.active_threads << "\n";
    }
    
    // Thread Pool별 메트릭
    std::cout << "\nThread Pool Metrics:\n";
    for (const auto& [pool_id, pool_metrics] : snapshot.thread_pool_metrics_map) {
        std::cout << "  Pool: " << pool_id.pool_name << " (Instance: " << pool_id.pool_instance_id << ")\n";
        std::cout << "    Process: " << pool_id.process_id.process_name << "\n";
        std::cout << "    Workers: " << pool_metrics.worker_threads << " (Idle: " << pool_metrics.idle_threads << ")\n";
        std::cout << "    Jobs: " << pool_metrics.jobs_completed << " completed, " 
                  << pool_metrics.jobs_pending << " pending\n";
        std::cout << "    Avg Latency: " << (pool_metrics.average_latency_ns / 1000000.0) << " ms\n";
        
        if (!pool_metrics.worker_load_distribution.empty()) {
            std::cout << "    Worker Load: ";
            for (size_t i = 0; i < pool_metrics.worker_load_distribution.size(); ++i) {
                std::cout << "[" << i << "]:" << pool_metrics.worker_load_distribution[i] << " ";
            }
            std::cout << "\n";
        }
    }
}

int main() {
    // 다중 프로세스 모니터링 생성
    auto monitor = std::make_shared<multi_process_monitoring>(
        1000,  // history_size
        100,   // collection_interval_ms
        10,    // max_processes
        5      // max_pools_per_process
    );
    
    monitor->start();
    
    // 프로세스 시뮬레이터 생성
    std::vector<std::unique_ptr<process_simulator>> processes;
    
    // 프로세스 1: 웹 서버
    process_identifier web_process{
        .pid = static_cast<std::uint32_t>(getpid()),
        .process_name = "web_server",
        .start_time = std::chrono::steady_clock::now()
    };
    auto web_sim = std::make_unique<process_simulator>(web_process, monitor);
    web_sim->add_thread_pool("http_workers", 1, 4);
    web_sim->add_thread_pool("websocket_workers", 1, 2);
    processes.push_back(std::move(web_sim));
    
    // 프로세스 2: 백그라운드 작업자
    process_identifier worker_process{
        .pid = static_cast<std::uint32_t>(getpid() + 1),
        .process_name = "background_worker",
        .start_time = std::chrono::steady_clock::now()
    };
    auto worker_sim = std::make_unique<process_simulator>(worker_process, monitor);
    worker_sim->add_thread_pool("job_workers", 1, 8);
    worker_sim->add_thread_pool("job_workers", 2, 8); // 두 번째 인스턴스
    processes.push_back(std::move(worker_sim));
    
    // 프로세스 3: 데이터베이스 서비스
    process_identifier db_process{
        .pid = static_cast<std::uint32_t>(getpid() + 2),
        .process_name = "database_service",
        .start_time = std::chrono::steady_clock::now()
    };
    auto db_sim = std::make_unique<process_simulator>(db_process, monitor);
    db_sim->add_thread_pool("query_workers", 1, 6);
    processes.push_back(std::move(db_sim));
    
    std::cout << "Multi-Process Monitoring Example Started\n";
    std::cout << "Monitoring " << processes.size() << " processes with multiple thread pools\n";
    std::cout << "Press Ctrl+C to exit\n\n";
    
    // 모니터링 루프
    for (int i = 0; i < 30; ++i) {
        // 메트릭 업데이트
        for (auto& proc : processes) {
            proc->update_system_metrics();
            proc->update_pool_metrics();
        }
        
        // 1초마다 스냅샷 출력
        if (i % 5 == 0) {
            auto snapshot = monitor->get_multi_process_snapshot();
            print_multi_process_snapshot(snapshot);
            
            // 프로세스 성능 비교
            std::vector<process_identifier> proc_ids = {web_process, worker_process, db_process};
            auto comparison = monitor->compare_process_performance(proc_ids);
            
            std::cout << "\nPerformance Comparison:\n";
            for (const auto& [metric, value] : comparison) {
                std::cout << "  " << metric << ": " << value << "\n";
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    monitor->stop();
    
    return 0;
}