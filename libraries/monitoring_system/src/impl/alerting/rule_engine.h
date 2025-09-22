#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <variant>
#include <optional>
#include <regex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <sstream>
#include <cmath>

namespace monitoring_system::alerting {

// Rule evaluation result
enum class RuleEvaluationResult {
    TRIGGERED,
    NOT_TRIGGERED,
    ERROR
};

// Alert severity levels
enum class AlertSeverity {
    INFO,
    WARNING,
    CRITICAL,
    EMERGENCY
};

// Condition operator types
enum class ConditionOperator {
    EQUAL,
    NOT_EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    CONTAINS,
    NOT_CONTAINS,
    REGEX_MATCH
};

// Aggregation function for metrics
enum class AggregationFunction {
    NONE,
    AVG,
    SUM,
    MIN,
    MAX,
    COUNT,
    STDDEV,
    PERCENTILE
};

// Rule condition structure
struct RuleCondition {
    std::string metric_name;
    ConditionOperator op;
    std::variant<double, std::string> threshold;
    AggregationFunction aggregation = AggregationFunction::NONE;
    std::chrono::seconds window = std::chrono::seconds(60);
    double percentile_value = 0.95;  // For percentile aggregation
};

// Composite condition for complex rules
struct CompositeCondition {
    enum LogicalOperator { AND, OR, NOT };
    LogicalOperator op;
    std::vector<std::variant<RuleCondition, std::shared_ptr<CompositeCondition>>> conditions;
};

// Alert rule definition
struct AlertRule {
    std::string id;
    std::string name;
    std::string description;
    AlertSeverity severity;
    bool enabled = true;
    std::variant<RuleCondition, CompositeCondition> condition;
    std::chrono::seconds evaluation_interval = std::chrono::seconds(60);
    std::chrono::seconds cooldown_period = std::chrono::seconds(300);
    std::unordered_map<std::string, std::string> labels;
    std::unordered_map<std::string, std::string> annotations;
    std::chrono::system_clock::time_point last_triggered;
};

// Metric data point for evaluation
struct MetricDataPoint {
    std::string name;
    double value;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> labels;
};

// Alert instance generated from rule evaluation
struct Alert {
    std::string id;
    std::string rule_id;
    std::string rule_name;
    AlertSeverity severity;
    std::string message;
    std::chrono::system_clock::time_point triggered_at;
    std::chrono::system_clock::time_point resolved_at;
    std::unordered_map<std::string, std::string> labels;
    std::unordered_map<std::string, std::string> annotations;
    bool is_resolved = false;
};

// Expression evaluation context
class ExpressionContext {
public:
    void set_variable(const std::string& name, double value) {
        variables_[name] = value;
    }

    std::optional<double> get_variable(const std::string& name) const {
        auto it = variables_.find(name);
        if (it != variables_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void clear() {
        variables_.clear();
    }

private:
    std::unordered_map<std::string, double> variables_;
};

// Expression evaluator for complex conditions
class ExpressionEvaluator {
public:
    ExpressionEvaluator() = default;

    // Parse and compile expression
    bool compile(const std::string& expression);

    // Evaluate compiled expression with context
    std::optional<double> evaluate(const ExpressionContext& context) const;

    // Get compilation error message
    const std::string& get_error() const { return error_message_; }

private:
    struct ExpressionNode;
    using ExpressionNodePtr = std::shared_ptr<ExpressionNode>;

    ExpressionNodePtr root_;
    std::string error_message_;

    ExpressionNodePtr parse_expression(const std::string& expr);
    double evaluate_node(const ExpressionNodePtr& node, const ExpressionContext& context) const;
};

// Rule engine for alert evaluation
class RuleEngine {
public:
    RuleEngine();
    ~RuleEngine();

    // Rule management
    void add_rule(const AlertRule& rule);
    void update_rule(const std::string& rule_id, const AlertRule& rule);
    void remove_rule(const std::string& rule_id);
    void enable_rule(const std::string& rule_id);
    void disable_rule(const std::string& rule_id);

    // Dynamic rule loading
    bool load_rules_from_file(const std::string& filepath);
    bool load_rules_from_json(const std::string& json_content);
    bool save_rules_to_file(const std::string& filepath) const;

    // Rule evaluation
    RuleEvaluationResult evaluate_rule(const std::string& rule_id,
                                      const std::vector<MetricDataPoint>& metrics);
    std::vector<Alert> evaluate_all_rules(const std::vector<MetricDataPoint>& metrics);

    // Threshold-based evaluation
    bool evaluate_threshold(double value, ConditionOperator op, double threshold) const;
    bool evaluate_string_condition(const std::string& value, ConditionOperator op,
                                  const std::string& pattern) const;

    // Complex condition evaluation
    bool evaluate_composite_condition(const CompositeCondition& condition,
                                     const std::vector<MetricDataPoint>& metrics) const;

    // Metric aggregation
    std::optional<double> aggregate_metrics(const std::vector<MetricDataPoint>& metrics,
                                           AggregationFunction func,
                                           const std::chrono::seconds& window,
                                           double percentile = 0.95) const;

    // Rule query
    std::optional<AlertRule> get_rule(const std::string& rule_id) const;
    std::vector<AlertRule> get_all_rules() const;
    std::vector<AlertRule> get_enabled_rules() const;
    std::vector<AlertRule> get_rules_by_severity(AlertSeverity severity) const;

    // Alert management
    std::vector<Alert> get_active_alerts() const;
    void resolve_alert(const std::string& alert_id);
    void clear_resolved_alerts();

    // Statistics
    size_t get_rule_count() const { return rules_.size(); }
    size_t get_active_alert_count() const { return active_alerts_.size(); }
    size_t get_evaluation_count() const { return evaluation_count_.load(); }

    // Background evaluation
    void start_background_evaluation();
    void stop_background_evaluation();
    void set_metric_provider(std::function<std::vector<MetricDataPoint>()> provider);

private:
    mutable std::mutex rules_mutex_;
    mutable std::mutex alerts_mutex_;
    std::unordered_map<std::string, AlertRule> rules_;
    std::unordered_map<std::string, Alert> active_alerts_;
    std::atomic<size_t> evaluation_count_{0};

    // Background evaluation
    std::atomic<bool> running_{false};
    std::thread evaluation_thread_;
    std::condition_variable cv_;
    std::function<std::vector<MetricDataPoint>()> metric_provider_;

    // Expression evaluator
    ExpressionEvaluator expression_evaluator_;

    // Helper methods
    bool should_trigger_alert(const AlertRule& rule) const;
    Alert create_alert(const AlertRule& rule, const std::string& message) const;
    std::string format_alert_message(const AlertRule& rule,
                                    const std::vector<MetricDataPoint>& metrics) const;
    void background_evaluation_loop();

    // Serialization helpers
    std::string serialize_rule_to_json(const AlertRule& rule) const;
    std::optional<AlertRule> deserialize_rule_from_json(const std::string& json) const;
};

// Rule builder for fluent API
class RuleBuilder {
public:
    RuleBuilder(const std::string& id) : rule_{} {
        rule_.id = id;
    }

    RuleBuilder& with_name(const std::string& name) {
        rule_.name = name;
        return *this;
    }

    RuleBuilder& with_description(const std::string& description) {
        rule_.description = description;
        return *this;
    }

    RuleBuilder& with_severity(AlertSeverity severity) {
        rule_.severity = severity;
        return *this;
    }

    RuleBuilder& with_condition(const RuleCondition& condition) {
        rule_.condition = condition;
        return *this;
    }

    RuleBuilder& with_composite_condition(const CompositeCondition& condition) {
        rule_.condition = condition;
        return *this;
    }

    RuleBuilder& with_evaluation_interval(std::chrono::seconds interval) {
        rule_.evaluation_interval = interval;
        return *this;
    }

    RuleBuilder& with_cooldown_period(std::chrono::seconds period) {
        rule_.cooldown_period = period;
        return *this;
    }

    RuleBuilder& add_label(const std::string& key, const std::string& value) {
        rule_.labels[key] = value;
        return *this;
    }

    RuleBuilder& add_annotation(const std::string& key, const std::string& value) {
        rule_.annotations[key] = value;
        return *this;
    }

    AlertRule build() const {
        return rule_;
    }

private:
    AlertRule rule_;
};

} // namespace monitoring_system::alerting