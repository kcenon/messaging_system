#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/multi_process_monitoring_interface.h"
#include <vector>
#include <deque>
#include <chrono>
#include <optional>
#include <functional>

namespace monitoring_module {

/**
 * @class trend_analyzer
 * @brief 메트릭 트렌드 분석 및 예측 도구
 * 
 * 시계열 데이터를 분석하여 트렌드, 이상 징후, 예측 제공
 */
class trend_analyzer {
public:
    using time_point = std::chrono::steady_clock::time_point;
    using duration = std::chrono::steady_clock::duration;
    
    /**
     * @struct trend_point
     * @brief 시계열 데이터 포인트
     */
    struct trend_point {
        time_point timestamp;
        double value;
    };
    
    /**
     * @struct trend_result
     * @brief 트렌드 분석 결과
     */
    struct trend_result {
        double slope;                    // 기울기 (변화율)
        double intercept;               // y 절편
        double r_squared;               // 결정계수 (0-1)
        double mean;                    // 평균값
        double std_deviation;           // 표준편차
        double trend_strength;          // 트렌드 강도 (0-100)
        std::string trend_direction;    // "increasing", "decreasing", "stable"
    };
    
    /**
     * @struct anomaly_result
     * @brief 이상 징후 탐지 결과
     */
    struct anomaly_result {
        time_point timestamp;
        double value;
        double expected_value;
        double deviation;               // 표준편차 단위
        std::string severity;           // "minor", "moderate", "severe"
        std::string description;
    };
    
    /**
     * @struct prediction_result
     * @brief 예측 결과
     */
    struct prediction_result {
        time_point timestamp;
        double predicted_value;
        double confidence_lower;        // 신뢰구간 하한
        double confidence_upper;        // 신뢰구간 상한
        double confidence_level;        // 신뢰수준 (예: 0.95)
    };
    
    /**
     * @brief 선형 회귀를 사용한 트렌드 분석
     * @param points 시계열 데이터
     * @return 트렌드 분석 결과
     */
    static trend_result analyze_trend(const std::vector<trend_point>& points);
    
    /**
     * @brief 이동 평균 계산
     * @param points 시계열 데이터
     * @param window_size 윈도우 크기
     * @return 이동 평균 값들
     */
    static std::vector<trend_point> calculate_moving_average(
        const std::vector<trend_point>& points,
        std::size_t window_size);
    
    /**
     * @brief 지수 이동 평균 계산
     * @param points 시계열 데이터
     * @param alpha 평활 계수 (0-1)
     * @return EMA 값들
     */
    static std::vector<trend_point> calculate_ema(
        const std::vector<trend_point>& points,
        double alpha = 0.3);
    
    /**
     * @brief Z-score 기반 이상 징후 탐지
     * @param points 시계열 데이터
     * @param threshold Z-score 임계값 (기본 3.0)
     * @return 이상 징후 목록
     */
    static std::vector<anomaly_result> detect_anomalies(
        const std::vector<trend_point>& points,
        double threshold = 3.0);
    
    /**
     * @brief 선형 예측
     * @param points 과거 데이터
     * @param future_duration 예측할 미래 시간
     * @param num_predictions 예측 포인트 수
     * @return 예측 결과들
     */
    static std::vector<prediction_result> predict_linear(
        const std::vector<trend_point>& points,
        duration future_duration,
        std::size_t num_predictions = 10);
    
    /**
     * @brief 계절성 탐지
     * @param points 시계열 데이터
     * @param period_hint 예상 주기 (옵션)
     * @return 탐지된 주기 (없으면 0)
     */
    static std::size_t detect_seasonality(
        const std::vector<trend_point>& points,
        std::size_t period_hint = 0);
    
    /**
     * @brief 변화점 탐지
     * @param points 시계열 데이터
     * @param sensitivity 민감도 (0-1, 높을수록 민감)
     * @return 변화점 인덱스들
     */
    static std::vector<std::size_t> detect_change_points(
        const std::vector<trend_point>& points,
        double sensitivity = 0.5);
    
    /**
     * @brief 메트릭 상관관계 분석
     * @param series1 첫 번째 시계열
     * @param series2 두 번째 시계열
     * @return 상관계수 (-1 ~ 1)
     */
    static double calculate_correlation(
        const std::vector<trend_point>& series1,
        const std::vector<trend_point>& series2);
    
    /**
     * @brief 용량 계획을 위한 성장률 예측
     * @param points 과거 사용량 데이터
     * @param capacity_limit 용량 한계
     * @return 한계 도달 예상 시점 (없으면 nullopt)
     */
    static std::optional<time_point> predict_capacity_exhaustion(
        const std::vector<trend_point>& points,
        double capacity_limit);
    
    /**
     * @brief 메트릭 건강도 점수 트렌드
     * @param points 건강도 점수 시계열
     * @return 개선/악화 트렌드 (-100 ~ 100)
     */
    static double calculate_health_trend(const std::vector<trend_point>& points);
};

/**
 * @class alert_manager
 * @brief 알림 관리 시스템
 * 
 * 임계값 기반 알림 및 트렌드 기반 조기 경고
 */
class alert_manager {
public:
    /**
     * @enum alert_severity
     * @brief 알림 심각도
     */
    enum class alert_severity {
        info,
        warning,
        critical,
        emergency
    };
    
    /**
     * @struct alert_condition
     * @brief 알림 조건
     */
    struct alert_condition {
        std::string name;
        std::function<bool(double)> condition;
        alert_severity severity;
        std::string message_template;
        std::chrono::seconds cooldown{60};  // 재알림 방지 시간
    };
    
    /**
     * @struct alert_event
     * @brief 발생한 알림 이벤트
     */
    struct alert_event {
        std::string condition_name;
        alert_severity severity;
        trend_analyzer::time_point timestamp;
        double value;
        std::string message;
    };
    
    /**
     * @brief 알림 조건 추가
     * @param condition 알림 조건
     */
    void add_condition(const alert_condition& condition);
    
    /**
     * @brief 메트릭 평가 및 알림 생성
     * @param metric_name 메트릭 이름
     * @param value 현재 값
     * @return 발생한 알림들
     */
    std::vector<alert_event> evaluate(const std::string& metric_name, double value);
    
    /**
     * @brief 최근 알림 조회
     * @param count 조회할 개수
     * @return 최근 알림 이벤트들
     */
    std::vector<alert_event> get_recent_alerts(std::size_t count = 10) const;
    
    /**
     * @brief 심각도별 알림 통계
     * @return 심각도별 알림 개수
     */
    std::unordered_map<alert_severity, std::size_t> get_alert_statistics() const;
    
    /**
     * @brief 알림 히스토리 초기화
     */
    void clear_history();
    
private:
    std::vector<alert_condition> conditions_;
    std::deque<alert_event> alert_history_;
    std::unordered_map<std::string, trend_analyzer::time_point> last_alert_time_;
    static constexpr std::size_t max_history_size_ = 1000;
};

} // namespace monitoring_module