/**
 * @file iot_monitoring.cpp
 * @brief IoT device monitoring and management system
 *
 * This example demonstrates how to build an IoT monitoring system
 * that collects data from multiple devices, processes telemetry,
 * and triggers alerts based on thresholds.
 */

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/services/container/container_service.h>
#include <kcenon/messaging/services/database/database_service.h>
#include <kcenon/messaging/services/network/network_service.h>
#include <iostream>
#include <thread>
#include <random>
#include <queue>
#include <map>
#include <atomic>
#include <cmath>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Device types in the IoT ecosystem
enum class device_type {
    TEMPERATURE_SENSOR,
    HUMIDITY_SENSOR,
    PRESSURE_SENSOR,
    MOTION_DETECTOR,
    SMART_LIGHT,
    SMART_LOCK,
    CAMERA,
    ENERGY_METER
};

// Device telemetry data
struct device_telemetry {
    std::string device_id;
    device_type type;
    double value;
    std::string unit;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, double> additional_data;
};

// Alert definition
struct alert {
    enum severity {
        INFO,
        WARNING,
        CRITICAL,
        EMERGENCY
    };

    std::string alert_id;
    std::string device_id;
    severity sev;
    std::string message;
    double threshold_value;
    double actual_value;
    std::chrono::system_clock::time_point triggered_at;
};

// Device configuration
struct device_config {
    std::string device_id;
    device_type type;
    std::string location;
    bool enabled;
    double min_threshold;
    double max_threshold;
    std::chrono::seconds reporting_interval{30};
    std::map<std::string, std::string> metadata;
};

class iot_monitoring_system {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::unique_ptr<services::container_service> container_svc;
    std::unique_ptr<services::database_service> database_svc;
    std::unique_ptr<services::network_service> network_svc;

    // Device management
    std::map<std::string, device_config> devices;
    std::map<std::string, device_telemetry> latest_telemetry;
    std::mutex devices_mutex;

    // Alert management
    std::queue<alert> alert_queue;
    std::mutex alert_mutex;
    std::condition_variable alert_cv;

    // Metrics
    std::atomic<uint64_t> total_messages{0};
    std::atomic<uint64_t> total_alerts{0};
    std::atomic<uint64_t> total_devices{0};

    // System state
    std::atomic<bool> running{true};

    // Simulation
    std::random_device rd;
    std::mt19937 gen;

public:
    iot_monitoring_system() : gen(rd()) {
        // Configure for IoT workload
        config::config_builder builder;
        auto config = builder
            .set_environment("iot_production")
            .set_worker_threads(std::thread::hardware_concurrency())
            .set_queue_size(100000)
            .set_max_message_size(64 * 1024)  // 64KB for device data
            .enable_persistence(true)
            .enable_monitoring(true)
            .enable_compression(true)
            .set_timeout(5000)  // 5 second timeout for device messages
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        container_svc = std::make_unique<services::container_service>();
        database_svc = std::make_unique<services::database_service>();
        network_svc = std::make_unique<services::network_service>();

        setupMessageHandlers();
        initializeDevices();
    }

    void setupMessageHandlers() {
        auto& bus = integrator->get_message_bus();

        // Device telemetry
        bus.subscribe("device.telemetry", [this](const core::message& msg) {
            handleDeviceTelemetry(msg);
        });

        // Device registration
        bus.subscribe("device.register", [this](const core::message& msg) {
            handleDeviceRegistration(msg);
        });

        // Device commands
        bus.subscribe("device.command", [this](const core::message& msg) {
            handleDeviceCommand(msg);
        });

        // System queries
        bus.subscribe("system.query", [this](const core::message& msg) {
            handleSystemQuery(msg);
        });

        // Alert acknowledgment
        bus.subscribe("alert.acknowledge", [this](const core::message& msg) {
            handleAlertAcknowledgment(msg);
        });
    }

    void initializeDevices() {
        // Register sample devices for simulation
        registerDevice("temp-001", device_type::TEMPERATURE_SENSOR, "Living Room", 18.0, 26.0);
        registerDevice("temp-002", device_type::TEMPERATURE_SENSOR, "Bedroom", 20.0, 24.0);
        registerDevice("temp-003", device_type::TEMPERATURE_SENSOR, "Server Room", 15.0, 22.0);

        registerDevice("hum-001", device_type::HUMIDITY_SENSOR, "Bathroom", 30.0, 70.0);
        registerDevice("hum-002", device_type::HUMIDITY_SENSOR, "Kitchen", 35.0, 65.0);

        registerDevice("motion-001", device_type::MOTION_DETECTOR, "Entrance", 0.0, 1.0);
        registerDevice("motion-002", device_type::MOTION_DETECTOR, "Hallway", 0.0, 1.0);

        registerDevice("energy-001", device_type::ENERGY_METER, "Main Panel", 0.0, 10000.0);
        registerDevice("cam-001", device_type::CAMERA, "Front Door", 0.0, 1.0);

        registerDevice("light-001", device_type::SMART_LIGHT, "Living Room", 0.0, 100.0);
        registerDevice("light-002", device_type::SMART_LIGHT, "Bedroom", 0.0, 100.0);

        registerDevice("lock-001", device_type::SMART_LOCK, "Front Door", 0.0, 1.0);

        std::cout << "Initialized " << devices.size() << " IoT devices" << std::endl;
    }

    void registerDevice(
        const std::string& device_id,
        DeviceType type,
        const std::string& location,
        double min_threshold,
        double max_threshold
    ) {
        DeviceConfig config;
        config.device_id = device_id;
        config.type = type;
        config.location = location;
        config.enabled = true;
        config.min_threshold = min_threshold;
        config.max_threshold = max_threshold;
        config.reporting_interval = std::chrono::seconds(10 + (gen() % 20));

        std::lock_guard<std::mutex> lock(devices_mutex);
        devices[device_id] = config;
        total_devices++;

        // Store device configuration in database
        auto container = container_svc->create_container();
        container->set("device_id", device_id);
        container->set("type", static_cast<int>(type));
        container->set("location", location);
        container->set("enabled", config.enabled);

        database_svc->store("devices", device_id, container->serialize());
    }

    void handleDeviceTelemetry(const core::message& msg) {
        try {
            auto device_id = msg.get_header("device_id");
            auto value = std::stod(msg.get_header("value"));
            auto timestamp = std::chrono::system_clock::now();

            DeviceTelemetry telemetry;
            telemetry.device_id = device_id;
            telemetry.value = value;
            telemetry.timestamp = timestamp;

            // Update latest telemetry
            {
                std::lock_guard<std::mutex> lock(devices_mutex);
                if (auto it = devices.find(device_id); it != devices.end()) {
                    telemetry.type = it->second.type;
                    telemetry.unit = getUnitForType(it->second.type);
                    latest_telemetry[device_id] = telemetry;

                    // Check thresholds
                    checkThresholds(it->second, telemetry);
                }
            }

            // Store telemetry in time-series database
            storeTelemetry(telemetry);

            // Process analytics
            processAnalytics(telemetry);

            total_messages++;

            // Log high-frequency updates less frequently
            if (total_messages % 100 == 0) {
                std::cout << "Processed " << total_messages << " telemetry messages" << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error processing telemetry: " << e.what() << std::endl;
        }
    }

    void checkThresholds(const DeviceConfig& config, const DeviceTelemetry& telemetry) {
        Alert::Severity severity = Alert::INFO;
        std::string message;
        bool create_alert = false;

        if (telemetry.value < config.min_threshold) {
            double deviation = config.min_threshold - telemetry.value;
            double deviation_percent = (deviation / config.min_threshold) * 100;

            if (deviation_percent > 20) {
                severity = Alert::CRITICAL;
                message = "Value critically below minimum threshold";
            } else if (deviation_percent > 10) {
                severity = Alert::WARNING;
                message = "Value below minimum threshold";
            }
            create_alert = true;

        } else if (telemetry.value > config.max_threshold) {
            double deviation = telemetry.value - config.max_threshold;
            double deviation_percent = (deviation / config.max_threshold) * 100;

            if (deviation_percent > 20) {
                severity = Alert::CRITICAL;
                message = "Value critically above maximum threshold";
            } else if (deviation_percent > 10) {
                severity = Alert::WARNING;
                message = "Value above maximum threshold";
            }
            create_alert = true;
        }

        // Special case for motion detectors - always alert on motion
        if (config.type == DeviceType::MOTION_DETECTOR && telemetry.value > 0.5) {
            severity = Alert::INFO;
            message = "Motion detected at " + config.location;
            create_alert = true;
        }

        if (create_alert) {
            createAlert(config.device_id, severity, message,
                       config.max_threshold, telemetry.value);
        }
    }

    void createAlert(
        const std::string& device_id,
        Alert::Severity severity,
        const std::string& message,
        double threshold,
        double actual
    ) {
        Alert alert;
        alert.alert_id = generateAlertId();
        alert.device_id = device_id;
        alert.severity = severity;
        alert.message = message;
        alert.threshold_value = threshold;
        alert.actual_value = actual;
        alert.triggered_at = std::chrono::system_clock::now();

        {
            std::lock_guard<std::mutex> lock(alert_mutex);
            alert_queue.push(alert);
            total_alerts++;
        }
        alert_cv.notify_one();

        // Publish alert
        publishAlert(alert);

        std::cout << "[ALERT] " << getSeverityString(severity)
                  << ": " << message
                  << " (Device: " << device_id << ")" << std::endl;
    }

    void publishAlert(const Alert& alert) {
        core::message alert_msg;
        alert_msg.set_type("alert.triggered");
        alert_msg.set_header("alert_id", alert.alert_id);
        alert_msg.set_header("device_id", alert.device_id);
        alert_msg.set_header("severity", std::to_string(static_cast<int>(alert.severity)));
        alert_msg.set_payload(alert.message);

        // Set priority based on severity
        switch (alert.severity) {
            case Alert::EMERGENCY:
                alert_msg.set_priority(core::priority::CRITICAL);
                break;
            case Alert::CRITICAL:
                alert_msg.set_priority(core::priority::HIGH);
                break;
            case Alert::WARNING:
                alert_msg.set_priority(core::priority::NORMAL);
                break;
            default:
                alert_msg.set_priority(core::priority::LOW);
        }

        integrator->get_message_bus().publish(alert_msg);
    }

    void handleDeviceRegistration(const core::message& msg) {
        auto device_id = msg.get_header("device_id");
        auto type = static_cast<DeviceType>(std::stoi(msg.get_header("type")));
        auto location = msg.get_header("location");

        registerDevice(device_id, type, location, 0.0, 100.0);

        std::cout << "Registered new device: " << device_id
                  << " at " << location << std::endl;
    }

    void handleDeviceCommand(const core::message& msg) {
        auto device_id = msg.get_header("device_id");
        auto command = msg.get_header("command");

        std::cout << "Executing command '" << command
                  << "' on device " << device_id << std::endl;

        // Forward command to device
        core::message cmd_msg;
        cmd_msg.set_type("device.execute");
        cmd_msg.set_header("device_id", device_id);
        cmd_msg.set_header("command", command);
        cmd_msg.set_priority(core::priority::HIGH);

        network_svc->send_to_device(device_id, cmd_msg);
    }

    void handleSystemQuery(const core::message& msg) {
        auto query_type = msg.get_header("query");

        if (query_type == "status") {
            sendSystemStatus();
        } else if (query_type == "devices") {
            sendDeviceList();
        } else if (query_type == "telemetry") {
            sendLatestTelemetry();
        }
    }

    void handleAlertAcknowledgment(const core::message& msg) {
        auto alert_id = msg.get_header("alert_id");
        auto user_id = msg.get_header("user_id");

        std::cout << "Alert " << alert_id
                  << " acknowledged by user " << user_id << std::endl;

        // Update alert status in database
        database_svc->update("alerts", alert_id, "status", "acknowledged");
    }

    void storeTelemetry(const DeviceTelemetry& telemetry) {
        auto container = container_svc->create_container();
        container->set("device_id", telemetry.device_id);
        container->set("value", telemetry.value);
        container->set("unit", telemetry.unit);
        container->set("timestamp", std::chrono::system_clock::to_time_t(telemetry.timestamp));

        // Use time-based key for time-series storage
        auto key = telemetry.device_id + "_" +
                   std::to_string(telemetry.timestamp.time_since_epoch().count());

        database_svc->store("telemetry", key, container->serialize());
    }

    void processAnalytics(const DeviceTelemetry& telemetry) {
        // Calculate running averages, detect anomalies, etc.
        // This would connect to a real analytics engine in production

        // Simple anomaly detection using z-score
        static std::map<std::string, std::vector<double>> history;
        auto& device_history = history[telemetry.device_id];

        device_history.push_back(telemetry.value);
        if (device_history.size() > 100) {
            device_history.erase(device_history.begin());

            // Calculate mean and standard deviation
            double sum = 0, sum_sq = 0;
            for (double val : device_history) {
                sum += val;
                sum_sq += val * val;
            }

            double mean = sum / device_history.size();
            double variance = (sum_sq / device_history.size()) - (mean * mean);
            double stddev = std::sqrt(variance);

            // Check for anomaly (z-score > 3)
            if (stddev > 0) {
                double z_score = std::abs((telemetry.value - mean) / stddev);
                if (z_score > 3) {
                    createAlert(telemetry.device_id, Alert::WARNING,
                               "Anomaly detected - unusual value pattern",
                               mean + 3 * stddev, telemetry.value);
                }
            }
        }
    }

    void sendSystemStatus() {
        core::message status;
        status.set_type("system.status");
        status.set_header("total_devices", std::to_string(total_devices.load()));
        status.set_header("total_messages", std::to_string(total_messages.load()));
        status.set_header("total_alerts", std::to_string(total_alerts.load()));
        status.set_header("uptime", std::to_string(getUptime()));

        integrator->get_message_bus().publish(status);
    }

    void sendDeviceList() {
        core::message device_list;
        device_list.set_type("system.device_list");

        std::lock_guard<std::mutex> lock(devices_mutex);
        for (const auto& [id, config] : devices) {
            auto container = container_svc->create_container();
            container->set("device_id", id);
            container->set("type", static_cast<int>(config.type));
            container->set("location", config.location);
            container->set("enabled", config.enabled);

            device_list.add_payload_item(id, container->serialize());
        }

        integrator->get_message_bus().publish(device_list);
    }

    void sendLatestTelemetry() {
        core::message telemetry_msg;
        telemetry_msg.set_type("system.telemetry");

        std::lock_guard<std::mutex> lock(devices_mutex);
        for (const auto& [id, telemetry] : latest_telemetry) {
            auto container = container_svc->create_container();
            container->set("device_id", id);
            container->set("value", telemetry.value);
            container->set("unit", telemetry.unit);
            container->set("timestamp",
                std::chrono::system_clock::to_time_t(telemetry.timestamp));

            telemetry_msg.add_payload_item(id, container->serialize());
        }

        integrator->get_message_bus().publish(telemetry_msg);
    }

    // Simulation methods
    void startDeviceSimulation() {
        std::thread simulation_thread([this]() {
            std::uniform_real_distribution<> temp_dist(15.0, 30.0);
            std::uniform_real_distribution<> humidity_dist(20.0, 80.0);
            std::uniform_real_distribution<> energy_dist(500.0, 5000.0);
            std::uniform_int_distribution<> motion_dist(0, 100);
            std::uniform_real_distribution<> light_dist(0.0, 100.0);

            while (running) {
                // Simulate telemetry from all devices
                std::lock_guard<std::mutex> lock(devices_mutex);
                for (const auto& [id, config] : devices) {
                    core::message telemetry;
                    telemetry.set_type("device.telemetry");
                    telemetry.set_header("device_id", id);

                    double value = 0.0;
                    switch (config.type) {
                        case DeviceType::TEMPERATURE_SENSOR:
                            value = temp_dist(gen);
                            break;
                        case DeviceType::HUMIDITY_SENSOR:
                            value = humidity_dist(gen);
                            break;
                        case DeviceType::ENERGY_METER:
                            value = energy_dist(gen);
                            break;
                        case DeviceType::MOTION_DETECTOR:
                            value = (motion_dist(gen) > 95) ? 1.0 : 0.0;
                            break;
                        case DeviceType::SMART_LIGHT:
                            value = light_dist(gen);
                            break;
                        default:
                            value = 0.0;
                    }

                    telemetry.set_header("value", std::to_string(value));

                    integrator->get_message_bus().publish(telemetry);
                }

                std::this_thread::sleep_for(5s);
            }
        });
        simulation_thread.detach();
    }

    void startAlertProcessor() {
        std::thread alert_thread([this]() {
            while (running) {
                std::unique_lock<std::mutex> lock(alert_mutex);
                alert_cv.wait_for(lock, 1s, [this] {
                    return !alert_queue.empty() || !running;
                });

                while (!alert_queue.empty()) {
                    Alert alert = alert_queue.front();
                    alert_queue.pop();
                    lock.unlock();

                    // Process alert (send notifications, trigger workflows, etc.)
                    processAlert(alert);

                    lock.lock();
                }
            }
        });
        alert_thread.detach();
    }

    void processAlert(const Alert& alert) {
        // In production, this would:
        // - Send push notifications
        // - Trigger automated responses
        // - Log to SIEM systems
        // - Execute remediation workflows

        // Store alert in database
        auto container = container_svc->create_container();
        container->set("alert_id", alert.alert_id);
        container->set("device_id", alert.device_id);
        container->set("severity", static_cast<int>(alert.severity));
        container->set("message", alert.message);
        container->set("threshold", alert.threshold_value);
        container->set("actual", alert.actual_value);
        container->set("timestamp",
            std::chrono::system_clock::to_time_t(alert.triggered_at));

        database_svc->store("alerts", alert.alert_id, container->serialize());
    }

    std::string generateAlertId() const {
        static std::atomic<uint64_t> counter{0};
        return "alert-" + std::to_string(counter.fetch_add(1));
    }

    std::string getUnitForType(DeviceType type) const {
        switch (type) {
            case DeviceType::TEMPERATURE_SENSOR: return "°C";
            case DeviceType::HUMIDITY_SENSOR: return "%";
            case DeviceType::PRESSURE_SENSOR: return "hPa";
            case DeviceType::ENERGY_METER: return "W";
            case DeviceType::SMART_LIGHT: return "%";
            default: return "";
        }
    }

    std::string getSeverityString(Alert::Severity severity) const {
        switch (severity) {
            case Alert::INFO: return "INFO";
            case Alert::WARNING: return "WARNING";
            case Alert::CRITICAL: return "CRITICAL";
            case Alert::EMERGENCY: return "EMERGENCY";
            default: return "UNKNOWN";
        }
    }

    int getUptime() const {
        static auto start_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(
            now - start_time
        ).count();
    }

public:
    void start() {
        std::cout << "\n=== IoT Monitoring System Starting ===\n" << std::endl;

        integrator->start();

        // Start background tasks
        startDeviceSimulation();
        startAlertProcessor();

        // Start dashboard
        startDashboard();

        std::cout << "IoT Monitoring System is running. Press Enter to stop..." << std::endl;
        std::cin.get();

        stop();
    }

    void stop() {
        running = false;
        alert_cv.notify_all();
        integrator->stop();

        std::cout << "\n=== Final Statistics ===" << std::endl;
        std::cout << "Total devices monitored: " << total_devices << std::endl;
        std::cout << "Total messages processed: " << total_messages << std::endl;
        std::cout << "Total alerts generated: " << total_alerts << std::endl;
        std::cout << "======================\n" << std::endl;
    }

    void startDashboard() {
        std::thread dashboard_thread([this]() {
            while (running) {
                std::this_thread::sleep_for(30s);
                printDashboard();
            }
        });
        dashboard_thread.detach();
    }

    void printDashboard() {
        std::cout << "\n╔═══════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║           IoT Monitoring System Dashboard             ║" << std::endl;
        std::cout << "╠═══════════════════════════════════════════════════════╣" << std::endl;

        std::cout << "║ Active Devices: " << std::setw(38)
                  << total_devices.load() << " ║" << std::endl;

        std::cout << "║ Messages/sec: " << std::setw(40)
                  << (total_messages.load() / std::max(1, getUptime())) << " ║" << std::endl;

        std::cout << "║ Total Alerts: " << std::setw(40)
                  << total_alerts.load() << " ║" << std::endl;

        std::cout << "║ Uptime: " << std::setw(43)
                  << getUptime() << "s ║" << std::endl;

        std::cout << "╠═══════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ Latest Telemetry:                                     ║" << std::endl;

        {
            std::lock_guard<std::mutex> lock(devices_mutex);
            int count = 0;
            for (const auto& [id, telemetry] : latest_telemetry) {
                if (count++ >= 5) break;  // Show top 5

                std::stringstream ss;
                ss << id << ": " << std::fixed << std::setprecision(2)
                   << telemetry.value << " " << telemetry.unit;

                std::cout << "║   " << std::left << std::setw(52)
                          << ss.str() << " ║" << std::endl;
            }
        }

        std::cout << "╚═══════════════════════════════════════════════════════╝" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        iot_monitoring_system iot_system;
        iot_system.start();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}