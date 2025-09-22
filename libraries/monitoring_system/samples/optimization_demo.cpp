/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <monitoring/multi_process_monitoring.h>
#include <monitoring/performance_optimizer.h>
#include <monitoring/analysis_dashboard.h>
#include <thread>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <csignal>
#include <atomic>

using namespace monitoring_interface;
using namespace monitoring_module;

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nShutting down...\n";
        running = false;
    }
}

// 부하 시뮬레이터
class load_simulator {
public:
    enum class load_pattern {
        steady,     // 일정한 부하
        spiky,      // 스파이크 패턴
        gradual,    // 점진적 증가/감소
        random      // 랜덤 패턴
    };
    
    load_simulator(load_pattern pattern = load_pattern::steady)
        : pattern_(pattern), cycle_(0) {}
    
    double get_cpu_load() {
        switch (pattern_) {
            case load_pattern::steady:
                return 50.0 + (std::sin(cycle_ * 0.05) * 10);
                
            case load_pattern::spiky:
                return (cycle_ % 100 < 10) ? 90.0 : 30.0;
                
            case load_pattern::gradual:
                return 20.0 + (cycle_ % 200) * 0.3;
                
            case load_pattern::random:
            default:
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_real_distribution<> dis(20.0, 80.0);
                return dis(gen);
        }
    }
    
    std::uint64_t get_memory_usage() {
        double base = 500 * 1024 * 1024;  // 500MB base
        double factor = 1.0 + (get_cpu_load() / 100.0);
        return static_cast<std::uint64_t>(base * factor);
    }
    
    void advance() { cycle_++; }
    
private:
    load_pattern pattern_;
    int cycle_;
};

// 성능 비교기
class performance_comparator {
public:
    void add_unoptimized_sample(std::chrono::nanoseconds processing_time,
                               std::size_t memory_usage) {
        unoptimized_times_.push_back(processing_time);
        unoptimized_memory_.push_back(memory_usage);
    }
    
    void add_optimized_sample(std::chrono::nanoseconds processing_time,
                            std::size_t memory_usage,
                            const performance_optimizer::optimization_stats& stats) {
        optimized_times_.push_back(processing_time);
        optimized_memory_.push_back(memory_usage);
        optimization_stats_ = stats;
    }
    
    void print_comparison() {
        if (unoptimized_times_.empty() || optimized_times_.empty()) {
            return;
        }
        
        // 평균 계산
        auto avg_unopt_time = std::accumulate(unoptimized_times_.begin(), 
                                            unoptimized_times_.end(),
                                            std::chrono::nanoseconds(0)) / unoptimized_times_.size();
        auto avg_opt_time = std::accumulate(optimized_times_.begin(),
                                          optimized_times_.end(),
                                          std::chrono::nanoseconds(0)) / optimized_times_.size();
        
        auto avg_unopt_mem = std::accumulate(unoptimized_memory_.begin(),
                                           unoptimized_memory_.end(), 0ULL) / unoptimized_memory_.size();
        auto avg_opt_mem = std::accumulate(optimized_memory_.begin(),
                                         optimized_memory_.end(), 0ULL) / optimized_memory_.size();
        
        // 개선율 계산
        double time_improvement = 100.0 * (1.0 - static_cast<double>(avg_opt_time.count()) / 
                                                 avg_unopt_time.count());
        double memory_improvement = 100.0 * (1.0 - static_cast<double>(avg_opt_mem) / avg_unopt_mem);
        
        std::cout << "\n╔═══════════════════════════════════════════════════════╗\n";
        std::cout << "║           Performance Optimization Results            ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════╝\n\n";
        
        std::cout << "📊 Processing Time:\n";
        std::cout << "   Unoptimized: " << avg_unopt_time.count() / 1000 << " μs\n";
        std::cout << "   Optimized:   " << avg_opt_time.count() / 1000 << " μs\n";
        std::cout << "   Improvement: " << std::fixed << std::setprecision(1) 
                  << time_improvement << "%\n\n";
        
        std::cout << "💾 Memory Usage:\n";
        std::cout << "   Unoptimized: " << avg_unopt_mem / 1024 << " KB\n";
        std::cout << "   Optimized:   " << avg_opt_mem / 1024 << " KB\n";
        std::cout << "   Improvement: " << memory_improvement << "%\n\n";
        
        std::cout << "🔧 Optimization Statistics:\n";
        std::cout << "   Compression Ratio: " << optimization_stats_.compression_ratio << "\n";
        std::cout << "   Batches Processed: " << optimization_stats_.batches_processed << "\n";
        std::cout << "   Samples Skipped:   " << optimization_stats_.samples_skipped << "\n";
        std::cout << "   Memory Saved:      " << optimization_stats_.memory_saved_bytes / 1024 << " KB\n";
    }
    
private:
    std::vector<std::chrono::nanoseconds> unoptimized_times_;
    std::vector<std::chrono::nanoseconds> optimized_times_;
    std::vector<std::size_t> unoptimized_memory_;
    std::vector<std::size_t> optimized_memory_;
    performance_optimizer::optimization_stats optimization_stats_;
};

// 스케일링 시각화
void visualize_scaling_decision(const auto_scaler::scaling_decision& decision) {
    std::cout << "\n🎯 Auto-Scaling Decision:\n";
    
    const char* action_str = "NONE";
    const char* action_color = "\033[90m";  // Gray
    
    switch (decision.recommended_action) {
        case auto_scaler::scaling_decision::action::scale_up:
            action_str = "SCALE UP ↑";
            action_color = "\033[32m";  // Green
            break;
        case auto_scaler::scaling_decision::action::scale_down:
            action_str = "SCALE DOWN ↓";
            action_color = "\033[33m";  // Yellow
            break;
        default:
            break;
    }
    
    std::cout << "   Action: " << action_color << action_str << "\033[0m\n";
    std::cout << "   Confidence: ";
    
    // 신뢰도 바
    int confidence_bars = static_cast<int>(decision.confidence * 10);
    std::cout << "[";
    for (int i = 0; i < 10; ++i) {
        if (i < confidence_bars) {
            std::cout << "█";
        } else {
            std::cout << "░";
        }
    }
    std::cout << "] " << std::fixed << std::setprecision(1) 
              << (decision.confidence * 100) << "%\n";
    
    std::cout << "   Reason: " << decision.reason << "\n";
    if (decision.recommended_resources > 0) {
        std::cout << "   Recommended Resources: " << decision.recommended_resources << "\n";
    }
}

int main() {
    std::signal(SIGINT, signal_handler);
    
    std::cout << "🚀 Monitoring System Optimization Demo\n";
    std::cout << "====================================\n\n";
    
    // 모니터링 시스템 초기화
    auto monitor = std::make_shared<multi_process_monitoring>();
    monitor->start();
    
    // 프로세스 등록
    process_identifier main_process{
        .pid = 1000,
        .process_name = "optimization_demo",
        .start_time = std::chrono::steady_clock::now()
    };
    monitor->register_process(main_process);
    
    // 최적화 설정
    performance_optimizer::optimization_config opt_config;
    opt_config.enable_compression = true;
    opt_config.enable_batching = true;
    opt_config.enable_tiered_storage = true;
    opt_config.enable_adaptive_sampling = true;
    opt_config.batch_size = 50;
    opt_config.compression_threshold = 100;
    
    auto optimizer = std::make_unique<performance_optimizer>(opt_config);
    
    // 자동 스케일러
    auto_scaler::scaling_policy scale_policy;
    scale_policy.cpu_threshold_up = 70.0;
    scale_policy.cpu_threshold_down = 30.0;
    scale_policy.cooldown = std::chrono::seconds(30);
    
    auto scaler = std::make_unique<auto_scaler>(scale_policy);
    
    // 분산 집계기
    distributed_aggregator::aggregation_config agg_config;
    agg_config.enable_parallel_aggregation = true;
    agg_config.worker_threads = 4;
    
    auto aggregator = std::make_unique<distributed_aggregator>(agg_config);
    
    // 성능 비교기
    performance_comparator comparator;
    
    // 부하 시뮬레이터
    load_simulator load_sim(load_simulator::load_pattern::gradual);
    
    // 분석 대시보드
    analysis_dashboard::dashboard_config dash_config;
    auto dashboard = std::make_unique<analysis_dashboard>(monitor, dash_config);
    
    std::cout << "Running optimization demo...\n";
    std::cout << "Press Ctrl+C to see final results\n\n";
    
    int cycle = 0;
    auto last_report = std::chrono::steady_clock::now();
    
    while (running && cycle < 300) {
        // 메트릭 생성
        metrics_snapshot snapshot;
        snapshot.capture_time = std::chrono::steady_clock::now();
        snapshot.system.cpu_usage_percent = static_cast<std::uint64_t>(load_sim.get_cpu_load());
        snapshot.system.memory_usage_bytes = load_sim.get_memory_usage();
        snapshot.system.active_threads = 10;
        
        // 처리 시간 측정
        auto start_unopt = std::chrono::high_resolution_clock::now();
        monitor->update_system_metrics(snapshot.system);
        auto end_unopt = std::chrono::high_resolution_clock::now();
        
        // 최적화된 처리
        auto start_opt = std::chrono::high_resolution_clock::now();
        optimizer->optimize_metric(snapshot);
        auto end_opt = std::chrono::high_resolution_clock::now();
        
        // 샘플 추가
        comparator.add_unoptimized_sample(
            end_unopt - start_unopt,
            sizeof(snapshot));
        
        comparator.add_optimized_sample(
            end_opt - start_opt,
            optimizer->get_storage().get_memory_stats().total_bytes / 
                std::max<std::size_t>(1, optimizer->get_storage().get_memory_stats().total_bytes),
            optimizer->get_stats());
        
        // 자동 스케일링 결정
        auto scale_decision = scaler->decide(snapshot);
        
        // 메모리 압력 적응
        double memory_pressure = static_cast<double>(snapshot.system.memory_usage_bytes) / 
                               (2.0 * 1024 * 1024 * 1024);  // 2GB 기준
        optimizer->adapt_to_memory_pressure(memory_pressure);
        
        // CPU 부하 적응
        optimizer->adapt_to_cpu_load(snapshot.system.cpu_usage_percent);
        
        // 주기적 리포트 (5초마다)
        auto now = std::chrono::steady_clock::now();
        if (now - last_report >= std::chrono::seconds(5)) {
            std::cout << "\033[2J\033[H";  // Clear screen
            
            std::cout << "📊 Current Status (Cycle " << cycle << ")\n";
            std::cout << "═══════════════════════════════════\n";
            std::cout << "CPU Load: " << snapshot.system.cpu_usage_percent << "%\n";
            std::cout << "Memory: " << snapshot.system.memory_usage_bytes / (1024 * 1024) << " MB\n";
            std::cout << "Memory Pressure: " << std::fixed << std::setprecision(2) 
                      << memory_pressure * 100 << "%\n\n";
            
            // 최적화 통계
            auto opt_stats = optimizer->get_stats();
            std::cout << "⚡ Optimization Stats:\n";
            std::cout << "   Compression Ratio: " << opt_stats.compression_ratio << "\n";
            std::cout << "   Memory Saved: " << opt_stats.memory_saved_bytes / 1024 << " KB\n";
            std::cout << "   Samples Skipped: " << opt_stats.samples_skipped << "\n";
            std::cout << "   Batches: " << opt_stats.batches_processed << "\n";
            
            // 저장소 통계
            auto storage_stats = optimizer->get_storage().get_memory_stats();
            std::cout << "\n💾 Tiered Storage:\n";
            std::cout << "   Hot Tier:  " << storage_stats.hot_tier_bytes / 1024 << " KB\n";
            std::cout << "   Warm Tier: " << storage_stats.warm_tier_bytes / 1024 << " KB\n";
            std::cout << "   Cold Tier: " << storage_stats.cold_tier_bytes / 1024 << " KB\n";
            
            // 스케일링 결정 시각화
            if (scale_decision.recommended_action != auto_scaler::scaling_decision::action::none) {
                visualize_scaling_decision(scale_decision);
            }
            
            last_report = now;
        }
        
        // 다음 사이클
        load_sim.advance();
        cycle++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 최종 결과
    std::cout << "\n\n";
    comparator.print_comparison();
    
    // 시스템 건강도
    auto health = dashboard->generate_health_report();
    std::cout << "\n🏥 Final System Health: " << health.health_status 
              << " (" << health.overall_health_score << "%)\n";
    
    monitor->stop();
    
    return 0;
}