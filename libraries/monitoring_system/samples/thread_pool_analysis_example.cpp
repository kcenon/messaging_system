/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <monitoring/multi_process_monitoring.h>
#include <monitoring/thread_pool_analyzer.h>
#include <thread>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <unistd.h>

using namespace monitoring_interface;
using namespace monitoring_module;

// ANSI color codes for terminal output
namespace colors {
    const char* reset = "\033[0m";
    const char* red = "\033[31m";
    const char* green = "\033[32m";
    const char* yellow = "\033[33m";
    const char* blue = "\033[34m";
    const char* magenta = "\033[35m";
    const char* cyan = "\033[36m";
    const char* bold = "\033[1m";
}

// 시뮬레이션된 워크로드 생성기
class workload_generator {
public:
    enum class workload_type {
        cpu_intensive,
        io_bound,
        balanced,
        bursty
    };
    
    static process_thread_pool_metrics generate_metrics(
        const thread_pool_identifier& pool_id,
        workload_type type,
        std::size_t worker_count,
        int time_offset) {
        
        process_thread_pool_metrics metrics;
        metrics.pool_id = pool_id;
        metrics.worker_threads = worker_count;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        
        switch (type) {
            case workload_type::cpu_intensive:
                metrics.idle_threads = gen() % 2;
                metrics.jobs_completed = 1000 + (gen() % 500);
                metrics.jobs_pending = gen() % 10;
                metrics.average_latency_ns = 1000000 + (gen() % 500000); // 1-1.5ms
                break;
                
            case workload_type::io_bound:
                metrics.idle_threads = worker_count / 2 + (gen() % (worker_count / 2));
                metrics.jobs_completed = 100 + (gen() % 50);
                metrics.jobs_pending = 50 + (gen() % 100);
                metrics.average_latency_ns = 50000000 + (gen() % 50000000); // 50-100ms
                break;
                
            case workload_type::balanced:
                metrics.idle_threads = worker_count / 4;
                metrics.jobs_completed = 500 + (gen() % 200);
                metrics.jobs_pending = 20 + (gen() % 30);
                metrics.average_latency_ns = 5000000 + (gen() % 5000000); // 5-10ms
                break;
                
            case workload_type::bursty:
                if (time_offset % 10 < 3) { // 30% burst
                    metrics.idle_threads = 0;
                    metrics.jobs_completed = 2000 + (gen() % 1000);
                    metrics.jobs_pending = 500 + (gen() % 500);
                    metrics.average_latency_ns = 2000000; // 2ms
                } else {
                    metrics.idle_threads = worker_count - 1;
                    metrics.jobs_completed = 10 + (gen() % 10);
                    metrics.jobs_pending = 0;
                    metrics.average_latency_ns = 1000000; // 1ms
                }
                break;
        }
        
        metrics.jobs_failed = metrics.jobs_completed / 100; // 1% 실패율
        metrics.total_execution_time_ns = metrics.jobs_completed * metrics.average_latency_ns;
        
        // 워커 부하 분산 생성
        metrics.worker_load_distribution.resize(worker_count);
        if (type == workload_type::cpu_intensive || type == workload_type::balanced) {
            // 균등 분산
            auto base_load = metrics.jobs_completed / worker_count;
            for (auto& load : metrics.worker_load_distribution) {
                load = base_load + (gen() % (base_load / 10 + 1));
            }
        } else {
            // 불균등 분산
            for (size_t i = 0; i < worker_count; ++i) {
                if (i < worker_count / 3) {
                    metrics.worker_load_distribution[i] = 
                        (metrics.jobs_completed * 2) / worker_count;
                } else {
                    metrics.worker_load_distribution[i] = 
                        metrics.jobs_completed / (worker_count * 2);
                }
            }
        }
        
        return metrics;
    }
};

// 분석 결과 시각화
class analysis_visualizer {
public:
    static void print_header() {
        std::cout << colors::bold << colors::cyan 
                  << "\n=== Thread Pool Performance Analysis Dashboard ===\n" 
                  << colors::reset << std::endl;
    }
    
    static void print_pool_summary(const thread_pool_analyzer::pool_performance_summary& summary) {
        std::cout << colors::bold << "Pool: " << colors::reset 
                  << summary.pool_id.pool_name 
                  << " (Process: " << summary.pool_id.process_id.process_name 
                  << ", Instance: " << summary.pool_id.pool_instance_id << ")\n";
        
        // 효율성 색상 코딩
        const char* efficiency_color = colors::green;
        if (summary.worker_efficiency < 50) efficiency_color = colors::red;
        else if (summary.worker_efficiency < 70) efficiency_color = colors::yellow;
        
        std::cout << "  Worker Efficiency: " << efficiency_color 
                  << std::fixed << std::setprecision(1) 
                  << summary.worker_efficiency << "%" << colors::reset << "\n";
        
        // 부하 분산 색상 코딩
        const char* balance_color = colors::green;
        if (summary.load_balance_score < 50) balance_color = colors::red;
        else if (summary.load_balance_score < 70) balance_color = colors::yellow;
        
        std::cout << "  Load Balance: " << balance_color 
                  << summary.load_balance_score << "%" << colors::reset << "\n";
        
        std::cout << "  Throughput/Worker: " << std::setprecision(2) 
                  << summary.throughput_per_worker << " jobs\n";
        std::cout << "  Queue Saturation: " << summary.queue_saturation << "%\n";
    }
    
    static void print_comparison(const thread_pool_analyzer::pool_comparison_result& result) {
        std::cout << colors::bold << colors::magenta 
                  << "\nComparison: " << colors::reset
                  << result.pool1.pool_name << " vs " << result.pool2.pool_name << "\n";
        
        std::cout << "  Throughput Ratio: " << std::setprecision(2) 
                  << result.throughput_ratio << "x\n";
        std::cout << "  Efficiency Diff: " << std::showpos 
                  << result.efficiency_difference << "%" << std::noshowpos << "\n";
        std::cout << "  Winner: " << colors::green << result.performance_winner 
                  << colors::reset << "\n";
        std::cout << "  " << colors::yellow << result.recommendation 
                  << colors::reset << "\n";
    }
    
    static void print_bottleneck(const std::string& pool_name, 
                               const std::optional<std::string>& bottleneck) {
        if (bottleneck) {
            std::cout << colors::red << "⚠️  Bottleneck in " << pool_name 
                      << ": " << *bottleneck << colors::reset << "\n";
        }
    }
    
    static void print_pool_type(const std::string& pool_name, const std::string& type) {
        const char* type_color = colors::blue;
        if (type == "CPU-bound") type_color = colors::red;
        else if (type == "IO-bound") type_color = colors::cyan;
        else if (type == "Idle") type_color = colors::yellow;
        
        std::cout << "  Pool Type: " << type_color << type << colors::reset << "\n";
    }
    
    static void print_health_score(const std::string& pool_name, double score) {
        const char* health_color = colors::green;
        if (score < 50) health_color = colors::red;
        else if (score < 70) health_color = colors::yellow;
        
        std::cout << "  Health Score: " << health_color 
                  << std::fixed << std::setprecision(1) 
                  << score << "/100" << colors::reset << "\n";
    }
    
    static void print_suggestions(const std::string& pool_name,
                                const std::vector<std::string>& suggestions) {
        if (!suggestions.empty()) {
            std::cout << colors::bold << "💡 Optimization suggestions for " 
                      << pool_name << ":" << colors::reset << "\n";
            for (const auto& suggestion : suggestions) {
                std::cout << "   • " << suggestion << "\n";
            }
        }
    }
};

int main() {
    // 모니터링 시스템 초기화
    auto monitor = std::make_shared<multi_process_monitoring>();
    monitor->start();
    
    // 프로세스 등록
    process_identifier web_process{
        .pid = static_cast<std::uint32_t>(getpid()),
        .process_name = "web_server",
        .start_time = std::chrono::steady_clock::now()
    };
    
    process_identifier worker_process{
        .pid = static_cast<std::uint32_t>(getpid() + 1),
        .process_name = "worker_service",
        .start_time = std::chrono::steady_clock::now()
    };
    
    monitor->register_process(web_process);
    monitor->register_process(worker_process);
    
    // Thread pool 등록
    std::vector<thread_pool_identifier> pools;
    
    // Web server pools
    pools.push_back({.process_id = web_process, .pool_name = "http_handlers", .pool_instance_id = 1});
    pools.push_back({.process_id = web_process, .pool_name = "websocket_handlers", .pool_instance_id = 1});
    
    // Worker service pools
    pools.push_back({.process_id = worker_process, .pool_name = "cpu_workers", .pool_instance_id = 1});
    pools.push_back({.process_id = worker_process, .pool_name = "io_workers", .pool_instance_id = 1});
    pools.push_back({.process_id = worker_process, .pool_name = "batch_processors", .pool_instance_id = 1});
    
    for (const auto& pool : pools) {
        monitor->register_thread_pool(pool);
    }
    
    analysis_visualizer::print_header();
    
    // 시뮬레이션 실행
    for (int cycle = 0; cycle < 20; ++cycle) {
        std::cout << colors::bold << "\n--- Analysis Cycle " << cycle + 1 
                  << " ---" << colors::reset << "\n";
        
        // 메트릭 생성 및 업데이트
        auto http_metrics = workload_generator::generate_metrics(
            pools[0], workload_generator::workload_type::balanced, 4, cycle);
        auto ws_metrics = workload_generator::generate_metrics(
            pools[1], workload_generator::workload_type::io_bound, 2, cycle);
        auto cpu_metrics = workload_generator::generate_metrics(
            pools[2], workload_generator::workload_type::cpu_intensive, 8, cycle);
        auto io_metrics = workload_generator::generate_metrics(
            pools[3], workload_generator::workload_type::io_bound, 6, cycle);
        auto batch_metrics = workload_generator::generate_metrics(
            pools[4], workload_generator::workload_type::bursty, 4, cycle);
        
        monitor->update_thread_pool_metrics(pools[0], http_metrics);
        monitor->update_thread_pool_metrics(pools[1], ws_metrics);
        monitor->update_thread_pool_metrics(pools[2], cpu_metrics);
        monitor->update_thread_pool_metrics(pools[3], io_metrics);
        monitor->update_thread_pool_metrics(pools[4], batch_metrics);
        
        // 분석 수행
        std::vector<process_thread_pool_metrics> all_metrics = {
            http_metrics, ws_metrics, cpu_metrics, io_metrics, batch_metrics
        };
        
        // 각 pool 분석
        for (const auto& metrics : all_metrics) {
            auto summary = thread_pool_analyzer::analyze_pool(metrics);
            analysis_visualizer::print_pool_summary(summary);
            
            auto pool_type = thread_pool_analyzer::classify_pool_type(metrics);
            analysis_visualizer::print_pool_type(metrics.pool_id.pool_name, pool_type);
            
            auto health = thread_pool_analyzer::calculate_health_score(metrics);
            analysis_visualizer::print_health_score(metrics.pool_id.pool_name, health);
            
            auto bottleneck = thread_pool_analyzer::detect_bottleneck(metrics);
            analysis_visualizer::print_bottleneck(metrics.pool_id.pool_name, bottleneck);
            
            std::cout << "\n";
        }
        
        // Pool 비교
        if (cycle % 5 == 0) {
            std::cout << colors::bold << "\nDetailed Comparisons:\n" << colors::reset;
            
            // 같은 프로세스 내 pool 비교
            auto http_vs_ws = thread_pool_analyzer::compare_pools(http_metrics, ws_metrics);
            analysis_visualizer::print_comparison(http_vs_ws);
            
            auto cpu_vs_io = thread_pool_analyzer::compare_pools(cpu_metrics, io_metrics);
            analysis_visualizer::print_comparison(cpu_vs_io);
            
            // 최적화 제안
            auto cpu_suggestions = thread_pool_analyzer::suggest_optimizations(cpu_metrics);
            analysis_visualizer::print_suggestions(cpu_metrics.pool_id.pool_name, cpu_suggestions);
            
            auto batch_suggestions = thread_pool_analyzer::suggest_optimizations(batch_metrics);
            analysis_visualizer::print_suggestions(batch_metrics.pool_id.pool_name, batch_suggestions);
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    monitor->stop();
    std::cout << colors::bold << colors::green 
              << "\nAnalysis Complete!" << colors::reset << std::endl;
    
    return 0;
}