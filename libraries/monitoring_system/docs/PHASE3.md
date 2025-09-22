# Phase 3: Real-time Alerting System and Web Dashboard

## Overview

Phase 3 transforms the monitoring_system into a complete observability platform by adding real-time alerting capabilities and a web-based dashboard for visualization. This phase implements a sophisticated rule-based alert engine, multi-channel notification system, and responsive web interface with WebSocket streaming.

## Architecture

### Component Overview

```
┌────────────────────────────────────────────┐
│            Web Dashboard                   │
│  - Real-time visualization                 │
│  - Interactive controls                    │
│  - Mobile responsive UI                    │
└────────────────────────────────────────────┘
                    │
         HTTP/WebSocket Protocol
                    │
┌────────────────────────────────────────────┐
│         Dashboard Server                   │
│  - HTTP request handling                   │
│  - WebSocket management                    │
│  - Authentication & CORS                   │
└────────────────────────────────────────────┘
                    │
┌────────────────────────────────────────────┐
│           Metric API                       │
│  - RESTful endpoints                       │
│  - Query processing                        │
│  - Data transformation                     │
└────────────────────────────────────────────┘
                    │
┌────────────────────────────────────────────┐
│         Alerting System                    │
│  ┌──────────────┐  ┌──────────────┐      │
│  │Rule Engine   │  │Notification  │      │
│  │              │  │Manager       │      │
│  └──────────────┘  └──────────────┘      │
│  ┌──────────────────────────────┐        │
│  │Alert Deduplication System     │        │
│  └──────────────────────────────┘        │
└────────────────────────────────────────────┘
```

## Components

### 1. Rule Engine (`alerting/rule_engine.h`)

The rule engine evaluates metrics against defined rules and triggers alerts when conditions are met.

#### Features
- Dynamic rule management (CRUD operations)
- Complex condition evaluation (AND/OR/NOT)
- Metric aggregation functions
- Background evaluation thread
- Expression evaluator for complex thresholds

#### Rule Structure
```cpp
struct AlertRule {
    std::string id;
    std::string name;
    AlertSeverity severity;
    std::variant<RuleCondition, CompositeCondition> condition;
    std::chrono::seconds evaluation_interval;
    std::chrono::seconds cooldown_period;
    std::unordered_map<std::string, std::string> labels;
};
```

#### Example Usage
```cpp
// Create a CPU usage alert rule
auto rule = RuleBuilder("cpu_high")
    .with_name("High CPU Usage")
    .with_severity(AlertSeverity::CRITICAL)
    .with_condition({
        .metric_name = "cpu_usage",
        .op = ConditionOperator::GREATER_THAN,
        .threshold = 90.0,
        .aggregation = AggregationFunction::AVG,
        .window = std::chrono::minutes(5)
    })
    .with_cooldown_period(std::chrono::minutes(10))
    .build();

engine.add_rule(rule);
engine.start_background_evaluation();
```

### 2. Notification Manager (`alerting/notification_manager.h`)

Manages alert notifications across multiple channels with retry logic and templating.

#### Supported Channels
- **Email**: SMTP-based email notifications
- **Slack**: Webhook integration
- **SMS**: Via Twilio, Nexmo, etc.
- **Webhook**: Generic HTTP callbacks
- **PagerDuty**: Incident management
- **OpsGenie**: Alert management

#### Features
- Template-based message rendering
- Priority queue processing
- Retry mechanism with exponential backoff
- Rate limiting per channel
- Notification history tracking

#### Example Configuration
```cpp
// Configure Slack notifications
auto slack_config = std::make_shared<SlackConfig>();
slack_config->webhook_url = "https://hooks.slack.com/services/...";
slack_config->channel = "#alerts";
slack_config->username = "Monitoring Bot";
notifier.add_channel_config("slack_primary", slack_config);

// Configure email notifications
auto email_config = std::make_shared<EmailConfig>();
email_config->smtp_server = "smtp.company.com";
email_config->smtp_port = 587;
email_config->use_tls = true;
email_config->from_address = "monitoring@company.com";
email_config->to_addresses = {"oncall@company.com", "ops@company.com"};
notifier.add_channel_config("email_oncall", email_config);
```

### 3. Alert Deduplication (`alerting/alert_deduplication.h`)

Intelligent system to reduce alert fatigue through deduplication and grouping.

#### Deduplication Strategies
- **Exact Match**: Identical alert content
- **Fuzzy Match**: Similarity-based using Levenshtein distance
- **Time-based**: Within configurable time windows
- **Fingerprint**: Content-based hashing

#### Grouping Strategies
- **BY_RULE**: Group by rule ID
- **BY_SEVERITY**: Group by severity level
- **BY_LABELS**: Group by label combinations
- **BY_TIME_WINDOW**: Group within time windows
- **BY_CUSTOM**: Custom grouping function

#### Silence Management
```cpp
SilenceConfig silence;
silence.name = "Maintenance Window";
silence.start_time = std::chrono::system_clock::now();
silence.end_time = std::chrono::system_clock::now() + std::chrono::hours(2);
silence.matchers = {{"service", "database"}, {"environment", "staging"}};
silence_manager.add_silence(silence);
```

### 4. Dashboard Server (`web/dashboard_server.h`)

HTTP/WebSocket server for the web dashboard with comprehensive request handling.

#### Features
- HTTP request routing
- WebSocket bidirectional communication
- Session-based authentication
- Rate limiting
- CORS support
- Static file serving
- Multi-threaded worker pool

#### Server Configuration
```cpp
DashboardServer server(8080);

// Configure CORS
CorsConfig cors;
cors.allowed_origins = {"http://localhost:3000", "https://app.company.com"};
cors.allowed_methods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
cors.allowed_headers = {"Content-Type", "Authorization"};
server.set_cors_config(cors);

// Configure authentication
AuthConfig auth;
auth.type = AuthConfig::BEARER;
auth.validate_token = [](const std::string& token) {
    return jwt_validator.validate(token);
};
server.set_auth_config(auth);

// Configure rate limiting
RateLimitConfig rate_limit;
rate_limit.requests_per_minute = 100;
rate_limit.burst_size = 200;
server.set_rate_limit_config(rate_limit);

server.start();
```

### 5. Metric API (`web/metric_api.h`)

RESTful API for metric queries and dashboard operations.

#### Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/v1/metrics` | GET | List all metrics |
| `/api/v1/metrics/{name}` | GET | Get specific metric |
| `/api/v1/query` | POST | Execute metric query |
| `/api/v1/alerts` | GET | List active alerts |
| `/api/v1/alerts/{id}/ack` | POST | Acknowledge alert |
| `/api/v1/dashboards` | GET/POST | Dashboard CRUD |
| `/api/v1/health` | GET | Health check |
| `/ws/metrics` | WS | Real-time metric stream |

#### Query Builder
```cpp
auto query = MetricQueryBuilder()
    .metric("cpu_usage")
    .from(std::chrono::system_clock::now() - std::chrono::hours(24))
    .to(std::chrono::system_clock::now())
    .interval(std::chrono::minutes(5))
    .label("host", "server-01")
    .aggregate("avg")
    .build();

auto results = api.query_metrics(query);
```

### 6. Web Dashboard (`web/static/dashboard.html`)

Responsive web interface for real-time monitoring.

#### Features
- **Real-time Charts**: Chart.js-based visualizations
- **WebSocket Updates**: Live metric streaming
- **Responsive Design**: Mobile-first approach
- **Interactive Filters**: Time range, metric type, refresh interval
- **Alert Panel**: Active alerts with severity indicators
- **System Overview**: Key metrics at a glance

#### Technologies
- HTML5/CSS3 with responsive grid layout
- JavaScript ES6+ for interactivity
- Chart.js for data visualization
- WebSocket API for real-time updates

## Configuration Examples

### Complete Setup Example

```cpp
#include "monitoring/alerting/rule_engine.h"
#include "monitoring/alerting/notification_manager.h"
#include "monitoring/alerting/alert_deduplication.h"
#include "monitoring/web/dashboard_server.h"
#include "monitoring/web/metric_api.h"

using namespace monitoring_system;

int main() {
    // 1. Initialize alert deduplication
    alerting::AlertDeduplicationSystem dedup_system;
    dedup_system.get_deduplication().set_deduplication_strategy(
        alerting::DeduplicationStrategy::FUZZY_MATCH);
    dedup_system.get_deduplication().set_similarity_threshold(0.85);

    // 2. Set up rule engine
    alerting::RuleEngine rule_engine;

    // CPU usage rule
    auto cpu_rule = alerting::RuleBuilder("cpu_critical")
        .with_name("Critical CPU Usage")
        .with_severity(alerting::AlertSeverity::CRITICAL)
        .with_condition({
            .metric_name = "system.cpu.usage",
            .op = alerting::ConditionOperator::GREATER_THAN,
            .threshold = 95.0,
            .aggregation = alerting::AggregationFunction::AVG,
            .window = std::chrono::minutes(5)
        })
        .with_cooldown_period(std::chrono::minutes(15))
        .add_label("team", "infrastructure")
        .add_annotation("runbook", "https://wiki/runbooks/high-cpu")
        .build();

    // Memory usage rule
    auto memory_rule = alerting::RuleBuilder("memory_high")
        .with_name("High Memory Usage")
        .with_severity(alerting::AlertSeverity::WARNING)
        .with_condition({
            .metric_name = "system.memory.usage",
            .op = alerting::ConditionOperator::GREATER_THAN,
            .threshold = 85.0
        })
        .with_evaluation_interval(std::chrono::seconds(30))
        .build();

    // Disk space rule with composite condition
    alerting::CompositeCondition disk_condition;
    disk_condition.op = alerting::CompositeCondition::OR;
    disk_condition.conditions.push_back(alerting::RuleCondition{
        .metric_name = "system.disk.usage",
        .op = alerting::ConditionOperator::GREATER_THAN,
        .threshold = 90.0
    });
    disk_condition.conditions.push_back(alerting::RuleCondition{
        .metric_name = "system.disk.available",
        .op = alerting::ConditionOperator::LESS_THAN,
        .threshold = 1000000000.0  // 1GB
    });

    auto disk_rule = alerting::RuleBuilder("disk_space")
        .with_name("Low Disk Space")
        .with_severity(alerting::AlertSeverity::WARNING)
        .with_composite_condition(disk_condition)
        .build();

    rule_engine.add_rule(cpu_rule);
    rule_engine.add_rule(memory_rule);
    rule_engine.add_rule(disk_rule);

    // 3. Configure notifications
    alerting::NotificationManager notifier;

    // Slack for all alerts
    auto slack_config = std::make_shared<alerting::SlackConfig>();
    slack_config->webhook_url = std::getenv("SLACK_WEBHOOK_URL");
    slack_config->channel = "#monitoring-alerts";
    slack_config->use_attachments = true;
    notifier.add_channel_config("slack_all", slack_config);

    // Email for critical alerts
    auto email_config = std::make_shared<alerting::EmailConfig>();
    email_config->smtp_server = "smtp.gmail.com";
    email_config->smtp_port = 587;
    email_config->use_tls = true;
    email_config->username = std::getenv("SMTP_USERNAME");
    email_config->password = std::getenv("SMTP_PASSWORD");
    email_config->from_address = "monitoring@company.com";
    email_config->to_addresses = {"oncall@company.com"};
    notifier.add_channel_config("email_critical", email_config);

    // PagerDuty for emergencies
    auto pagerduty_config = std::make_shared<alerting::WebhookConfig>();
    pagerduty_config->url = "https://events.pagerduty.com/v2/enqueue";
    pagerduty_config->headers["Authorization"] = std::getenv("PAGERDUTY_KEY");
    notifier.add_channel_config("pagerduty", pagerduty_config);

    // 4. Set up web dashboard
    web::DashboardServer server(8080);

    // Configure server
    web::CorsConfig cors;
    cors.allowed_origins = {"*"};  // Allow all origins for development
    cors.enabled = true;
    server.set_cors_config(cors);

    web::RateLimitConfig rate_limit;
    rate_limit.requests_per_minute = 600;
    rate_limit.burst_size = 100;
    server.set_rate_limit_config(rate_limit);

    // 5. Set up metric API
    web::MetricAPI api;
    api.set_rule_engine(std::make_shared<alerting::RuleEngine>(rule_engine));
    api.register_routes(server);

    // 6. Start all services
    notifier.start();
    rule_engine.start_background_evaluation();
    server.start();

    std::cout << "Monitoring system started:\n";
    std::cout << "  - Dashboard: http://localhost:8080\n";
    std::cout << "  - API: http://localhost:8080/api/v1\n";
    std::cout << "  - WebSocket: ws://localhost:8080/ws/metrics\n";
    std::cout << "\nPress Enter to stop...\n";
    std::cin.get();

    // 7. Cleanup
    server.stop();
    rule_engine.stop_background_evaluation();
    notifier.stop();

    return 0;
}
```

## Performance Considerations

### Alert Engine Optimization
- Use batch evaluation for multiple rules
- Enable metric caching for frequently accessed data
- Configure appropriate evaluation intervals
- Use composite conditions judiciously

### Notification Optimization
- Configure appropriate retry delays
- Use batch notifications where possible
- Implement rate limiting per channel
- Cache template rendering results

### Dashboard Optimization
- Enable HTTP compression
- Use WebSocket for real-time updates
- Implement client-side caching
- Paginate large result sets

## Troubleshooting

### Common Issues

#### High Alert Volume
- Review deduplication settings
- Adjust rule thresholds
- Increase cooldown periods
- Configure silence windows

#### Notification Failures
- Check channel configurations
- Verify network connectivity
- Review authentication credentials
- Check rate limits

#### Dashboard Performance
- Reduce query time ranges
- Increase aggregation intervals
- Enable response caching
- Optimize WebSocket subscriptions

## Migration Guide

### From Existing Monitoring Systems

#### From Prometheus Alertmanager
```cpp
// Convert Prometheus rule
// alert: HighCPU
// expr: avg(cpu_usage) > 80
// for: 5m

auto rule = RuleBuilder("HighCPU")
    .with_condition({
        .metric_name = "cpu_usage",
        .op = ConditionOperator::GREATER_THAN,
        .threshold = 80.0,
        .aggregation = AggregationFunction::AVG,
        .window = std::chrono::minutes(5)
    })
    .build();
```

#### From Grafana Alerts
```cpp
// Convert Grafana alert
// Condition: avg() OF query(A, 5m, now) IS ABOVE 80

auto rule = RuleBuilder("grafana_alert")
    .with_condition({
        .metric_name = "query_A",
        .op = ConditionOperator::GREATER_THAN,
        .threshold = 80.0,
        .aggregation = AggregationFunction::AVG,
        .window = std::chrono::minutes(5)
    })
    .build();
```

## Best Practices

### Alert Design
1. **Start with high thresholds** and gradually tune down
2. **Use aggregation functions** to reduce noise
3. **Set appropriate cooldown periods** to prevent alert storms
4. **Include runbook links** in annotations
5. **Use severity levels** consistently

### Notification Strategy
1. **Route by severity**: Critical → PagerDuty, Warning → Slack
2. **Implement escalation**: Email → Slack → SMS → Phone
3. **Configure quiet hours** for non-critical alerts
4. **Test notification channels** regularly
5. **Document oncall procedures**

### Dashboard Design
1. **Group related metrics** in panels
2. **Use consistent time ranges** across charts
3. **Provide drill-down capabilities**
4. **Include alert status** indicators
5. **Optimize for mobile** viewing

## API Reference

### Rule Engine API
```cpp
class RuleEngine {
    void add_rule(const AlertRule& rule);
    void update_rule(const std::string& rule_id, const AlertRule& rule);
    void remove_rule(const std::string& rule_id);
    void enable_rule(const std::string& rule_id);
    void disable_rule(const std::string& rule_id);

    RuleEvaluationResult evaluate_rule(const std::string& rule_id,
                                      const std::vector<MetricDataPoint>& metrics);
    std::vector<Alert> evaluate_all_rules(const std::vector<MetricDataPoint>& metrics);

    void start_background_evaluation();
    void stop_background_evaluation();
};
```

### Notification Manager API
```cpp
class NotificationManager {
    void add_channel_config(const std::string& id, std::shared_ptr<ChannelConfig> config);
    void add_template(const NotificationTemplate& tmpl);

    std::future<NotificationResult> send_notification(const NotificationRequest& request);
    std::vector<std::future<NotificationResult>> notify_alert(const Alert& alert);

    void start();
    void stop();
};
```

### Dashboard Server API
```cpp
class DashboardServer {
    bool start();
    void stop();

    void add_route(const std::string& path, HttpMethod method, HttpHandler handler);
    void add_websocket_endpoint(const std::string& path, WebSocketHandler handler);

    void set_auth_config(const AuthConfig& config);
    void set_rate_limit_config(const RateLimitConfig& config);
    void set_cors_config(const CorsConfig& config);
};
```

## Conclusion

Phase 3 successfully transforms the monitoring_system into a complete observability platform with:
- **Sophisticated alerting** with rule-based evaluation
- **Multi-channel notifications** with intelligent routing
- **Real-time web dashboard** with responsive design
- **Comprehensive API** for integration

The implementation provides enterprise-grade features while maintaining high performance and reliability.