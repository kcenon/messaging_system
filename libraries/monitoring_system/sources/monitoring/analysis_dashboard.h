#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/multi_process_monitoring_interface.h"
#include "multi_process_monitoring.h"
#include "thread_pool_analyzer.h"
#include "trend_analyzer.h"
#include <memory>
#include <string>
#include <sstream>

namespace monitoring_module {

/**
 * @class analysis_dashboard
 * @brief 종합 분석 대시보드
 * 
 * 모든 고급 분석 기능을 통합하여 시각화하는 대시보드
 */
class analysis_dashboard {
public:
    /**
     * @struct dashboard_config
     * @brief 대시보드 설정
     */
    struct dashboard_config {
        std::size_t trend_window_size{60};      // 트렌드 분석 윈도우 (초)
        std::size_t prediction_horizon{300};    // 예측 범위 (초)
        double anomaly_threshold{3.0};          // 이상 탐지 Z-score 임계값
        bool enable_alerts{true};               // 알림 활성화
        bool enable_predictions{true};          // 예측 활성화
        bool enable_correlations{true};         // 상관관계 분석 활성화
    };
    
    /**
     * @struct system_health_report
     * @brief 시스템 건강도 종합 리포트
     */
    struct system_health_report {
        double overall_health_score{0.0};       // 전체 건강도 (0-100)
        std::string health_status;              // "Excellent", "Good", "Fair", "Poor", "Critical"
        std::vector<std::string> issues;        // 발견된 문제들
        std::vector<std::string> warnings;      // 경고 사항
        std::vector<std::string> recommendations; // 개선 권고사항
        std::chrono::steady_clock::time_point report_time;
    };
    
    /**
     * @struct performance_forecast
     * @brief 성능 예측 리포트
     */
    struct performance_forecast {
        std::unordered_map<std::string, trend_analyzer::prediction_result> cpu_predictions;
        std::unordered_map<std::string, trend_analyzer::prediction_result> memory_predictions;
        std::unordered_map<std::string, trend_analyzer::prediction_result> throughput_predictions;
        std::optional<std::chrono::steady_clock::time_point> capacity_warning_time;
        std::vector<std::string> risk_factors;
    };
    
    /**
     * @struct correlation_matrix
     * @brief 메트릭간 상관관계 매트릭스
     */
    struct correlation_matrix {
        std::vector<std::string> metric_names;
        std::vector<std::vector<double>> correlations;  // NxN 매트릭스
        std::vector<std::pair<std::string, std::string>> strong_correlations; // |r| > 0.7
    };
    
    /**
     * @brief 생성자
     * @param monitor 모니터링 시스템
     * @param config 대시보드 설정
     */
    explicit analysis_dashboard(
        std::shared_ptr<multi_process_monitoring> monitor,
        const dashboard_config& config);
    
    /**
     * @brief 시스템 건강도 종합 분석
     * @return 건강도 리포트
     */
    system_health_report generate_health_report();
    
    /**
     * @brief 성능 예측 분석
     * @param duration 예측 기간
     * @return 예측 리포트
     */
    performance_forecast generate_forecast(std::chrono::seconds duration);
    
    /**
     * @brief 메트릭간 상관관계 분석
     * @return 상관관계 매트릭스
     */
    correlation_matrix analyze_correlations();
    
    /**
     * @brief 실시간 이상 징후 탐지
     * @return 탐지된 이상 징후들
     */
    std::vector<trend_analyzer::anomaly_result> detect_real_time_anomalies();
    
    /**
     * @brief 병목 현상 종합 분석
     * @return 프로세스/풀별 병목 현상 맵
     */
    std::unordered_map<monitoring_interface::thread_pool_identifier, std::string> 
    analyze_bottlenecks();
    
    /**
     * @brief 최적화 제안 생성
     * @return 우선순위별 최적화 제안
     */
    std::vector<std::pair<int, std::string>> generate_optimization_suggestions();
    
    /**
     * @brief 용량 계획 분석
     * @return 리소스별 용량 예측
     */
    std::unordered_map<std::string, std::optional<std::chrono::steady_clock::time_point>>
    analyze_capacity_planning();
    
    /**
     * @brief 대시보드 텍스트 렌더링
     * @return 포맷된 대시보드 문자열
     */
    std::string render_text_dashboard();
    
    /**
     * @brief JSON 형식으로 대시보드 데이터 내보내기
     * @return JSON 문자열
     */
    std::string export_json();
    
    /**
     * @brief 알림 관리자 접근
     * @return 알림 관리자 참조
     */
    alert_manager& get_alert_manager() { return alert_manager_; }
    
    /**
     * @brief 메트릭 히스토리 수집
     * @param metric_name 메트릭 이름
     * @param duration 수집 기간
     * @return 시계열 데이터
     */
    std::vector<trend_analyzer::trend_point> collect_metric_history(
        const std::string& metric_name,
        std::chrono::seconds duration);
        
private:
    std::shared_ptr<multi_process_monitoring> monitor_;
    dashboard_config config_;
    alert_manager alert_manager_;
    
    // 캐시된 데이터
    std::unordered_map<std::string, std::vector<trend_analyzer::trend_point>> metric_cache_;
    std::chrono::steady_clock::time_point last_update_;
    
    // 내부 헬퍼 함수들
    void update_metric_cache();
    double calculate_process_health_score(const monitoring_interface::process_identifier& proc_id);
    double calculate_pool_health_score(const monitoring_interface::thread_pool_identifier& pool_id);
    void setup_default_alerts();
    std::string format_duration(std::chrono::seconds duration);
    std::string severity_to_string(alert_manager::alert_severity severity);
};

} // namespace monitoring_module