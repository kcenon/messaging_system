/**
 * @file test_interfaces_compile.cpp
 * @brief Compilation test for new monitoring interfaces
 *
 * This test ensures that all new interface headers compile correctly
 * and can be included without errors.
 */

#include <monitoring/interfaces/observer_interface.h>
#include <monitoring/interfaces/metric_collector_interface.h>
#include <monitoring/interfaces/event_bus_interface.h>
#include <memory>
#include <iostream>

using namespace monitoring_system;

// Test implementation of observer interface
class test_observer : public interface_monitoring_observer {
public:
    void on_metric_collected(const metric_event& event) override {
        std::cout << "Metric collected from: " << event.source() << std::endl;
    }

    void on_event_occurred(const system_event& event) override {
        std::cout << "System event from: " << event.component() << std::endl;
    }

    void on_system_state_changed(const state_change_event& event) override {
        std::cout << "State change in: " << event.component() << std::endl;
    }
};

// Test that interfaces can be used as base classes
class test_collector : public interface_metric_collector {
public:
    result<std::vector<metric>> collect_metrics() override {
        return result<std::vector<metric>>(std::vector<metric>{});
    }

    result_void start_collection(const collection_config& config) override {
        return result_void::success();
    }

    result_void stop_collection() override {
        return result_void::success();
    }

    bool is_collecting() const override {
        return false;
    }

    std::vector<std::string> get_metric_types() const override {
        return {"test_metric"};
    }

    collection_config get_config() const override {
        return collection_config{};
    }

    result_void update_config(const collection_config& config) override {
        return result_void::success();
    }

    result<std::vector<metric>> force_collect() override {
        return collect_metrics();
    }

    metric_stats get_stats() const override {
        return metric_stats{};
    }

    void reset_stats() override {}

    // Observable interface methods
    result_void register_observer(std::shared_ptr<interface_monitoring_observer> observer) override {
        return result_void::success();
    }

    result_void unregister_observer(std::shared_ptr<interface_monitoring_observer> observer) override {
        return result_void::success();
    }

    void notify_metric(const metric_event& event) override {}
    void notify_event(const system_event& event) override {}
    void notify_state_change(const state_change_event& event) override {}
};

int main() {
    std::cout << "=== Interface Compilation Test ===" << std::endl;

    // Test that interfaces can be instantiated through implementations
    auto observer = std::make_shared<test_observer>();
    auto collector = std::make_shared<test_collector>();

    // Test that interfaces can be used
    metric m{"test", metric_value{42.0}, {}};
    metric_event me("test_source", m);
    observer->on_metric_collected(me);

    system_event se(system_event::event_type::component_started, "test_component", "Started");
    observer->on_event_occurred(se);

    state_change_event sce("test_component",
                          state_change_event::state::healthy,
                          state_change_event::state::degraded);
    observer->on_system_state_changed(sce);

    // Test collector interface
    auto result = collector->collect_metrics();
    if (result) {
        std::cout << "Metrics collected successfully" << std::endl;
    }

    auto types = collector->get_metric_types();
    std::cout << "Collector supports " << types.size() << " metric type(s)" << std::endl;

    std::cout << "✅ All interface compilation tests passed!" << std::endl;

    return 0;
}