#include <kcenon/messaging/integration/messaging_health_checks.h>
#include <gtest/gtest.h>

namespace msg_integration = kcenon::messaging::integration;
namespace msg_collectors = kcenon::messaging::collectors;
namespace msg_adapters = kcenon::messaging::adapters;
namespace cmn = kcenon::common::interfaces;

// =============================================================================
// Test Helpers
// =============================================================================

namespace {

msg_collectors::message_bus_stats make_healthy_stats() {
    msg_collectors::message_bus_stats stats;
    stats.messages_published = 1000;
    stats.messages_processed = 995;
    stats.messages_failed = 5;
    stats.messages_dropped = 0;
    stats.queue_depth = 10;
    stats.queue_capacity = 1000;
    stats.queue_utilization_percent = 1.0;
    stats.throughput_per_second = 100.0;
    stats.average_latency_ms = 5.0;
    stats.max_latency_ms = 20.0;
    stats.min_latency_ms = 1.0;
    stats.topic_count = 3;
    stats.total_subscriber_count = 5;
    stats.worker_thread_count = 4;
    stats.is_running = true;
    return stats;
}

msg_collectors::message_bus_stats make_degraded_stats() {
    auto stats = make_healthy_stats();
    stats.queue_depth = 750;
    stats.queue_utilization_percent = 75.0;
    return stats;
}

msg_collectors::message_bus_stats make_unhealthy_stats() {
    auto stats = make_healthy_stats();
    stats.messages_failed = 200;
    stats.queue_depth = 950;
    stats.queue_utilization_percent = 95.0;
    stats.average_latency_ms = 600.0;
    stats.is_running = false;
    return stats;
}

/**
 * @brief Mock transport for testing transport_health_check
 */
class mock_transport : public msg_adapters::transport_interface {
public:
    explicit mock_transport(msg_adapters::transport_state state)
        : state_(state) {}

    kcenon::common::VoidResult connect() override {
        state_ = msg_adapters::transport_state::connected;
        return kcenon::common::ok(std::monostate{});
    }

    kcenon::common::VoidResult disconnect() override {
        state_ = msg_adapters::transport_state::disconnected;
        return kcenon::common::ok(std::monostate{});
    }

    bool is_connected() const override {
        return state_ == msg_adapters::transport_state::connected;
    }

    msg_adapters::transport_state get_state() const override {
        return state_;
    }

    kcenon::common::VoidResult send(
        const kcenon::messaging::message&) override {
        return kcenon::common::ok(std::monostate{});
    }

    kcenon::common::VoidResult send_binary(
        const std::vector<uint8_t>&) override {
        return kcenon::common::ok(std::monostate{});
    }

    void set_message_handler(
        std::function<void(const kcenon::messaging::message&)>) override {}

    void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)>) override {}

    void set_state_handler(
        std::function<void(msg_adapters::transport_state)>) override {}

    void set_error_handler(
        std::function<void(const std::string&)>) override {}

    msg_adapters::transport_statistics get_statistics() const override {
        return stats_;
    }

    void reset_statistics() override {
        stats_ = {};
    }

    void set_state(msg_adapters::transport_state state) { state_ = state; }

    msg_adapters::transport_statistics stats_;

private:
    msg_adapters::transport_state state_;
};

}  // anonymous namespace

// =============================================================================
// map_health_status Tests
// =============================================================================

TEST(HealthStatusMappingTest, MapsAllStatuses) {
    EXPECT_EQ(msg_integration::map_health_status(
        msg_collectors::message_bus_health_status::healthy),
        cmn::health_status::healthy);

    EXPECT_EQ(msg_integration::map_health_status(
        msg_collectors::message_bus_health_status::degraded),
        cmn::health_status::degraded);

    EXPECT_EQ(msg_integration::map_health_status(
        msg_collectors::message_bus_health_status::unhealthy),
        cmn::health_status::unhealthy);

    EXPECT_EQ(msg_integration::map_health_status(
        msg_collectors::message_bus_health_status::critical),
        cmn::health_status::unhealthy);
}

// =============================================================================
// messaging_health_check Tests
// =============================================================================

TEST(MessagingHealthCheckTest, Name) {
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    EXPECT_EQ(check->get_name(), "messaging.test_bus");
}

TEST(MessagingHealthCheckTest, Type) {
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    EXPECT_EQ(check->get_type(), cmn::health_check_type::readiness);
}

TEST(MessagingHealthCheckTest, HealthyBus) {
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::healthy);
    EXPECT_FALSE(result.message.empty());
    EXPECT_EQ(result.metadata.at("bus_name"), "test_bus");
    EXPECT_EQ(result.metadata.at("is_running"), "true");
}

TEST(MessagingHealthCheckTest, DegradedBus) {
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_degraded_stats);

    auto result = check->check();

    // Queue saturation at 75% triggers warning
    EXPECT_TRUE(result.status == cmn::health_status::degraded ||
                result.status == cmn::health_status::healthy);
}

TEST(MessagingHealthCheckTest, UnhealthyBus) {
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_unhealthy_stats);

    auto result = check->check();

    // High failure rate + high queue saturation + high latency
    EXPECT_NE(result.status, cmn::health_status::healthy);
}

TEST(MessagingHealthCheckTest, NoProvider) {
    msg_integration::messaging_health_check::stats_provider empty_provider;
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", empty_provider);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::unknown);
}

TEST(MessagingHealthCheckTest, IsCriticalByDefault) {
    auto check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    EXPECT_TRUE(check->is_critical());
}

// =============================================================================
// queue_health_check Tests
// =============================================================================

TEST(QueueHealthCheckTest, Name) {
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_healthy_stats);

    EXPECT_EQ(check->get_name(), "messaging.test_bus.queue");
}

TEST(QueueHealthCheckTest, HealthyQueue) {
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_healthy_stats);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::healthy);
    EXPECT_EQ(result.metadata.at("queue_depth"), "10");
    EXPECT_EQ(result.metadata.at("queue_capacity"), "1000");
}

TEST(QueueHealthCheckTest, DegradedQueue) {
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_degraded_stats, 0.7, 0.9);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::degraded);
}

TEST(QueueHealthCheckTest, CriticalQueue) {
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_unhealthy_stats, 0.7, 0.9);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::unhealthy);
}

TEST(QueueHealthCheckTest, CustomThresholds) {
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_degraded_stats, 0.8, 0.95);

    auto result = check->check();

    // 75% utilization is below custom warn threshold of 80%
    EXPECT_EQ(result.status, cmn::health_status::healthy);
}

TEST(QueueHealthCheckTest, IsNotCritical) {
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_healthy_stats);

    EXPECT_FALSE(check->is_critical());
}

TEST(QueueHealthCheckTest, NoProvider) {
    msg_integration::queue_health_check::stats_provider empty_provider;
    auto check = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", empty_provider);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::unknown);
}

// =============================================================================
// transport_health_check Tests
// =============================================================================

TEST(TransportHealthCheckTest, Name) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connected);
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    EXPECT_EQ(check->get_name(), "messaging.transport.ws_primary");
}

TEST(TransportHealthCheckTest, Type) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connected);
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    EXPECT_EQ(check->get_type(), cmn::health_check_type::dependency);
}

TEST(TransportHealthCheckTest, Connected) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connected);
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::healthy);
    EXPECT_EQ(result.metadata.at("transport_name"), "ws_primary");
}

TEST(TransportHealthCheckTest, Connecting) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connecting);
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::degraded);
}

TEST(TransportHealthCheckTest, Disconnected) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::disconnected);
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::unhealthy);
}

TEST(TransportHealthCheckTest, ErrorState) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::error);
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::unhealthy);
}

TEST(TransportHealthCheckTest, NullTransport) {
    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", nullptr);

    auto result = check->check();

    EXPECT_EQ(result.status, cmn::health_status::unknown);
}

TEST(TransportHealthCheckTest, StatisticsInMetadata) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connected);
    transport->stats_.messages_sent = 42;
    transport->stats_.messages_received = 38;
    transport->stats_.errors = 2;

    auto check = std::make_shared<msg_integration::transport_health_check>(
        "ws_primary", transport);

    auto result = check->check();

    EXPECT_EQ(result.metadata.at("messages_sent"), "42");
    EXPECT_EQ(result.metadata.at("messages_received"), "38");
    EXPECT_EQ(result.metadata.at("errors"), "2");
}

// =============================================================================
// Composite Health Check Tests
// =============================================================================

TEST(CompositeHealthCheckTest, CreatesComposite) {
    auto composite = msg_integration::create_messaging_composite_check(
        "test_bus", make_healthy_stats);

    EXPECT_EQ(composite->get_name(), "messaging.test_bus.composite");
    // Should have bus check + queue check = 2
    EXPECT_EQ(composite->size(), 2u);
}

TEST(CompositeHealthCheckTest, WithTransports) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connected);

    std::unordered_map<std::string,
        std::shared_ptr<msg_adapters::transport_interface>> transports;
    transports["ws_primary"] = transport;

    auto composite = msg_integration::create_messaging_composite_check(
        "test_bus", make_healthy_stats, transports);

    // bus + queue + 1 transport = 3
    EXPECT_EQ(composite->size(), 3u);
}

TEST(CompositeHealthCheckTest, HealthyResult) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::connected);

    std::unordered_map<std::string,
        std::shared_ptr<msg_adapters::transport_interface>> transports;
    transports["ws_primary"] = transport;

    auto composite = msg_integration::create_messaging_composite_check(
        "test_bus", make_healthy_stats, transports);

    auto result = composite->check();

    EXPECT_EQ(result.status, cmn::health_status::healthy);
}

TEST(CompositeHealthCheckTest, DegradedWhenTransportDisconnected) {
    auto transport = std::make_shared<mock_transport>(
        msg_adapters::transport_state::disconnected);

    std::unordered_map<std::string,
        std::shared_ptr<msg_adapters::transport_interface>> transports;
    transports["ws_primary"] = transport;

    auto composite = msg_integration::create_messaging_composite_check(
        "test_bus", make_healthy_stats, transports);

    auto result = composite->check();

    // Transport disconnected -> at least one unhealthy child
    EXPECT_NE(result.status, cmn::health_status::healthy);
}

// =============================================================================
// Health Monitor Registration Tests
// =============================================================================

TEST(HealthMonitorRegistrationTest, RegistersSuccessfully) {
    // Use a local monitor instead of global to avoid test interference
    cmn::health_monitor monitor;

    auto bus_check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    auto result = monitor.register_check(bus_check->get_name(), bus_check);
    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(monitor.has_check("messaging.test_bus"));
}

TEST(HealthMonitorRegistrationTest, CheckExecutesViaMonitor) {
    cmn::health_monitor monitor;

    auto bus_check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    monitor.register_check(bus_check->get_name(), bus_check);

    auto result = monitor.check("messaging.test_bus");
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value().status, cmn::health_status::healthy);
}

TEST(HealthMonitorRegistrationTest, MultipleChecksRefresh) {
    cmn::health_monitor monitor;

    auto bus_check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);
    auto queue_check_ptr = std::make_shared<msg_integration::queue_health_check>(
        "test_bus", make_healthy_stats);

    monitor.register_check(bus_check->get_name(), bus_check);
    monitor.register_check(queue_check_ptr->get_name(), queue_check_ptr);

    monitor.refresh();

    auto stats = monitor.get_stats();
    EXPECT_EQ(stats.total_checks, 2u);
    EXPECT_GE(stats.healthy_count, 1u);
}

TEST(HealthMonitorRegistrationTest, OverallStatusHealthy) {
    cmn::health_monitor monitor;

    auto bus_check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    monitor.register_check(bus_check->get_name(), bus_check);
    monitor.refresh();

    EXPECT_EQ(monitor.get_overall_status(), cmn::health_status::healthy);
}

TEST(HealthMonitorRegistrationTest, HealthReport) {
    cmn::health_monitor monitor;

    auto bus_check = std::make_shared<msg_integration::messaging_health_check>(
        "test_bus", make_healthy_stats);

    monitor.register_check(bus_check->get_name(), bus_check);
    monitor.refresh();

    auto report = monitor.get_health_report();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("messaging.test_bus"), std::string::npos);
}
