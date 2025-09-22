/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "trend_analyzer.h"
#include <numeric>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace monitoring_module {

trend_analyzer::trend_result 
trend_analyzer::analyze_trend(const std::vector<trend_point>& points) {
    trend_result result{};
    
    if (points.size() < 2) {
        return result;
    }
    
    // 시간을 초 단위로 변환
    std::vector<double> x_values, y_values;
    auto start_time = points.front().timestamp;
    
    for (const auto& point : points) {
        auto duration = point.timestamp - start_time;
        x_values.push_back(
            std::chrono::duration_cast<std::chrono::seconds>(duration).count());
        y_values.push_back(point.value);
    }
    
    // 평균 계산
    double x_mean = std::accumulate(x_values.begin(), x_values.end(), 0.0) / x_values.size();
    double y_mean = std::accumulate(y_values.begin(), y_values.end(), 0.0) / y_values.size();
    result.mean = y_mean;
    
    // 선형 회귀
    double numerator = 0.0, denominator = 0.0;
    for (size_t i = 0; i < x_values.size(); ++i) {
        numerator += (x_values[i] - x_mean) * (y_values[i] - y_mean);
        denominator += (x_values[i] - x_mean) * (x_values[i] - x_mean);
    }
    
    if (denominator != 0) {
        result.slope = numerator / denominator;
        result.intercept = y_mean - result.slope * x_mean;
    }
    
    // 표준편차 및 R-squared 계산
    double ss_tot = 0.0, ss_res = 0.0, variance = 0.0;
    for (size_t i = 0; i < y_values.size(); ++i) {
        double y_pred = result.slope * x_values[i] + result.intercept;
        ss_tot += (y_values[i] - y_mean) * (y_values[i] - y_mean);
        ss_res += (y_values[i] - y_pred) * (y_values[i] - y_pred);
        variance += (y_values[i] - y_mean) * (y_values[i] - y_mean);
    }
    
    result.std_deviation = std::sqrt(variance / y_values.size());
    result.r_squared = (ss_tot != 0) ? (1.0 - ss_res / ss_tot) : 0.0;
    
    // 트렌드 방향 및 강도
    double slope_per_minute = result.slope * 60;  // 분당 변화율
    if (std::abs(slope_per_minute) < 0.01 * result.mean) {
        result.trend_direction = "stable";
    } else if (slope_per_minute > 0) {
        result.trend_direction = "increasing";
    } else {
        result.trend_direction = "decreasing";
    }
    
    result.trend_strength = std::min(100.0, std::abs(result.r_squared) * 100.0);
    
    return result;
}

std::vector<trend_analyzer::trend_point> 
trend_analyzer::calculate_moving_average(
    const std::vector<trend_point>& points,
    std::size_t window_size) {
    
    std::vector<trend_point> ma_points;
    if (points.size() < window_size) {
        return ma_points;
    }
    
    for (size_t i = window_size - 1; i < points.size(); ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < window_size; ++j) {
            sum += points[i - j].value;
        }
        
        trend_point ma_point;
        ma_point.timestamp = points[i].timestamp;
        ma_point.value = sum / window_size;
        ma_points.push_back(ma_point);
    }
    
    return ma_points;
}

std::vector<trend_analyzer::trend_point> 
trend_analyzer::calculate_ema(const std::vector<trend_point>& points, double alpha) {
    std::vector<trend_point> ema_points;
    if (points.empty()) {
        return ema_points;
    }
    
    // 첫 번째 값은 그대로 사용
    ema_points.push_back(points[0]);
    
    for (size_t i = 1; i < points.size(); ++i) {
        trend_point ema_point;
        ema_point.timestamp = points[i].timestamp;
        ema_point.value = alpha * points[i].value + (1 - alpha) * ema_points.back().value;
        ema_points.push_back(ema_point);
    }
    
    return ema_points;
}

std::vector<trend_analyzer::anomaly_result> 
trend_analyzer::detect_anomalies(const std::vector<trend_point>& points, double threshold) {
    std::vector<anomaly_result> anomalies;
    
    if (points.size() < 10) {  // 최소 데이터 필요
        return anomalies;
    }
    
    // 평균 및 표준편차 계산
    double mean = 0.0;
    for (const auto& point : points) {
        mean += point.value;
    }
    mean /= points.size();
    
    double variance = 0.0;
    for (const auto& point : points) {
        variance += (point.value - mean) * (point.value - mean);
    }
    double std_dev = std::sqrt(variance / points.size());
    
    // Z-score 기반 이상 탐지
    for (const auto& point : points) {
        double z_score = (std_dev != 0) ? (point.value - mean) / std_dev : 0;
        
        if (std::abs(z_score) > threshold) {
            anomaly_result anomaly;
            anomaly.timestamp = point.timestamp;
            anomaly.value = point.value;
            anomaly.expected_value = mean;
            anomaly.deviation = z_score;
            
            if (std::abs(z_score) > 5) {
                anomaly.severity = "severe";
            } else if (std::abs(z_score) > 4) {
                anomaly.severity = "moderate";
            } else {
                anomaly.severity = "minor";
            }
            
            std::stringstream desc;
            desc << "Value " << point.value << " deviates " 
                 << std::abs(z_score) << " standard deviations from mean";
            anomaly.description = desc.str();
            
            anomalies.push_back(anomaly);
        }
    }
    
    return anomalies;
}

std::vector<trend_analyzer::prediction_result> 
trend_analyzer::predict_linear(
    const std::vector<trend_point>& points,
    duration future_duration,
    std::size_t num_predictions) {
    
    std::vector<prediction_result> predictions;
    
    if (points.size() < 3) {
        return predictions;
    }
    
    // 트렌드 분석
    auto trend = analyze_trend(points);
    
    // 예측 신뢰구간 계산을 위한 표준 오차
    double se = trend.std_deviation * std::sqrt(1.0 - trend.r_squared);
    
    // 시작 시간과 간격 계산
    auto last_time = points.back().timestamp;
    auto time_step = future_duration / num_predictions;
    auto start_time = points.front().timestamp;
    
    for (size_t i = 1; i <= num_predictions; ++i) {
        prediction_result pred;
        pred.timestamp = last_time + (time_step * i);
        
        // 시간을 초 단위로 변환
        auto duration_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            pred.timestamp - start_time).count();
        
        // 예측값 계산
        pred.predicted_value = trend.slope * duration_seconds + trend.intercept;
        
        // 95% 신뢰구간 (약 2 표준오차)
        double margin = 1.96 * se * std::sqrt(1.0 + 1.0/points.size() + 
            std::pow(duration_seconds - duration_seconds/2, 2) / 
            (points.size() * trend.std_deviation * trend.std_deviation));
        
        pred.confidence_lower = pred.predicted_value - margin;
        pred.confidence_upper = pred.predicted_value + margin;
        pred.confidence_level = 0.95;
        
        predictions.push_back(pred);
    }
    
    return predictions;
}

std::size_t trend_analyzer::detect_seasonality(
    const std::vector<trend_point>& points,
    std::size_t period_hint) {
    
    if (points.size() < 20) {  // 최소 데이터 필요
        return 0;
    }
    
    // 자기상관 함수 계산
    std::vector<double> values;
    for (const auto& point : points) {
        values.push_back(point.value);
    }
    
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    
    // 후보 주기 범위
    size_t min_period = period_hint > 0 ? period_hint / 2 : 2;
    size_t max_period = period_hint > 0 ? period_hint * 2 : values.size() / 3;
    
    double max_correlation = 0.0;
    size_t best_period = 0;
    
    for (size_t lag = min_period; lag <= max_period && lag < values.size(); ++lag) {
        double correlation = 0.0;
        double variance1 = 0.0, variance2 = 0.0;
        
        for (size_t i = 0; i < values.size() - lag; ++i) {
            double diff1 = values[i] - mean;
            double diff2 = values[i + lag] - mean;
            correlation += diff1 * diff2;
            variance1 += diff1 * diff1;
            variance2 += diff2 * diff2;
        }
        
        if (variance1 > 0 && variance2 > 0) {
            correlation /= std::sqrt(variance1 * variance2);
            
            if (correlation > max_correlation && correlation > 0.5) {
                max_correlation = correlation;
                best_period = lag;
            }
        }
    }
    
    return best_period;
}

std::vector<std::size_t> trend_analyzer::detect_change_points(
    const std::vector<trend_point>& points,
    double sensitivity) {
    
    std::vector<std::size_t> change_points;
    
    if (points.size() < 10) {
        return change_points;
    }
    
    // CUSUM (Cumulative Sum) 알고리즘
    std::vector<double> values;
    for (const auto& point : points) {
        values.push_back(point.value);
    }
    
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double std_dev = 0.0;
    for (double val : values) {
        std_dev += (val - mean) * (val - mean);
    }
    std_dev = std::sqrt(std_dev / values.size());
    
    double threshold = (3.0 - 2.0 * sensitivity) * std_dev;  // 민감도에 따른 임계값
    
    double cusum_pos = 0.0, cusum_neg = 0.0;
    for (size_t i = 1; i < values.size(); ++i) {
        double diff = values[i] - values[i-1];
        
        cusum_pos = std::max(0.0, cusum_pos + diff - threshold/2);
        cusum_neg = std::max(0.0, cusum_neg - diff - threshold/2);
        
        if (cusum_pos > threshold || cusum_neg > threshold) {
            change_points.push_back(i);
            cusum_pos = 0.0;
            cusum_neg = 0.0;
        }
    }
    
    return change_points;
}

double trend_analyzer::calculate_correlation(
    const std::vector<trend_point>& series1,
    const std::vector<trend_point>& series2) {
    
    if (series1.size() != series2.size() || series1.empty()) {
        return 0.0;
    }
    
    double mean1 = 0.0, mean2 = 0.0;
    for (size_t i = 0; i < series1.size(); ++i) {
        mean1 += series1[i].value;
        mean2 += series2[i].value;
    }
    mean1 /= series1.size();
    mean2 /= series2.size();
    
    double covariance = 0.0, variance1 = 0.0, variance2 = 0.0;
    for (size_t i = 0; i < series1.size(); ++i) {
        double diff1 = series1[i].value - mean1;
        double diff2 = series2[i].value - mean2;
        covariance += diff1 * diff2;
        variance1 += diff1 * diff1;
        variance2 += diff2 * diff2;
    }
    
    if (variance1 == 0 || variance2 == 0) {
        return 0.0;
    }
    
    return covariance / std::sqrt(variance1 * variance2);
}

std::optional<trend_analyzer::time_point> 
trend_analyzer::predict_capacity_exhaustion(
    const std::vector<trend_point>& points,
    double capacity_limit) {
    
    if (points.size() < 5) {
        return std::nullopt;
    }
    
    auto trend = analyze_trend(points);
    
    // 감소 추세이거나 안정적이면 고갈 없음
    if (trend.slope <= 0) {
        return std::nullopt;
    }
    
    // 현재 값이 이미 한계를 초과
    if (points.back().value >= capacity_limit) {
        return points.back().timestamp;
    }
    
    // 선형 예측으로 한계 도달 시점 계산
    auto start_time = points.front().timestamp;
    double seconds_to_limit = (capacity_limit - trend.intercept) / trend.slope;
    
    if (seconds_to_limit < 0) {
        return std::nullopt;
    }
    
    return start_time + std::chrono::seconds(static_cast<long>(seconds_to_limit));
}

double trend_analyzer::calculate_health_trend(const std::vector<trend_point>& points) {
    if (points.size() < 3) {
        return 0.0;
    }
    
    auto trend = analyze_trend(points);
    
    // 건강도는 0-100 범위라고 가정
    // 증가하면 개선, 감소하면 악화
    double trend_percentage = (trend.slope * 3600) / 100.0 * 100.0;  // 시간당 변화율
    
    // R-squared로 신뢰도 반영
    trend_percentage *= trend.r_squared;
    
    return std::max(-100.0, std::min(100.0, trend_percentage));
}

// Alert Manager 구현
void alert_manager::add_condition(const alert_condition& condition) {
    conditions_.push_back(condition);
}

std::vector<alert_manager::alert_event> 
alert_manager::evaluate(const std::string& metric_name, double value) {
    std::vector<alert_event> events;
    auto now = std::chrono::steady_clock::now();
    
    for (const auto& condition : conditions_) {
        // 조건 확인
        if (!condition.condition(value)) {
            continue;
        }
        
        // 쿨다운 확인
        auto it = last_alert_time_.find(condition.name);
        if (it != last_alert_time_.end()) {
            auto elapsed = now - it->second;
            if (elapsed < condition.cooldown) {
                continue;
            }
        }
        
        // 알림 생성
        alert_event event;
        event.condition_name = condition.name;
        event.severity = condition.severity;
        event.timestamp = now;
        event.value = value;
        
        // 메시지 생성 (간단한 템플릿 치환)
        event.message = condition.message_template;
        size_t pos = event.message.find("{value}");
        if (pos != std::string::npos) {
            event.message.replace(pos, 7, std::to_string(value));
        }
        pos = event.message.find("{metric}");
        if (pos != std::string::npos) {
            event.message.replace(pos, 8, metric_name);
        }
        
        events.push_back(event);
        alert_history_.push_back(event);
        last_alert_time_[condition.name] = now;
        
        // 히스토리 크기 제한
        if (alert_history_.size() > max_history_size_) {
            alert_history_.pop_front();
        }
    }
    
    return events;
}

std::vector<alert_manager::alert_event> 
alert_manager::get_recent_alerts(std::size_t count) const {
    std::vector<alert_event> recent;
    
    auto start = alert_history_.size() > count ? 
                 alert_history_.end() - count : alert_history_.begin();
    
    std::copy(start, alert_history_.end(), std::back_inserter(recent));
    return recent;
}

std::unordered_map<alert_manager::alert_severity, std::size_t> 
alert_manager::get_alert_statistics() const {
    std::unordered_map<alert_severity, std::size_t> stats;
    
    for (const auto& event : alert_history_) {
        stats[event.severity]++;
    }
    
    return stats;
}

void alert_manager::clear_history() {
    alert_history_.clear();
    last_alert_time_.clear();
}

} // namespace monitoring_module