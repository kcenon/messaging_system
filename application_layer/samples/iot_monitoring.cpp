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
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <random>
#include <queue>
#include <map>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <sstream>

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
    std::unique_ptr<services::container::container_service> container_svc;
    std::unique_ptr<services::database::database_service> database_svc;
    std::unique_ptr<services::network::network_service> network_svc;
    std::shared_ptr<logger_module::logger> m_logger;

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
        // Initialize logger
        m_logger = std::make_shared<logger_module::logger>(true, 8192);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "iot_monitoring.log", 10 * 1024 * 1024, 5));

        m_logger->log(logger_module::log_level::info, "Initializing IoT Monitoring System");

        // Configure for IoT workload
        config::config_builder builder;
        auto config = builder
            .set_environment("iot_production")
            .set_worker_threads(std::thread::hardware_concurrency())
            .set_queue_size(100000)
            .set_container_max_size(64 * 1024)  // 64KB for device data
            .enable_compression(true)
            .enable_external_monitoring(true)
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        container_svc = std::make_unique<services::container::container_service>();
        database_svc = std::make_unique<services::database::database_service>();
        network_svc = std::make_unique<services::network::network_service>();

        setupMessageHandlers();
        initializeDevices();
    }

    void setupMessageHandlers() {
        auto* bus = integrator->get_message_bus();

        // Device telemetry
        bus->subscribe("device.telemetry", [this](const core::message& msg) {
            handleDeviceTelemetry(msg);
        });

        // Device registration
        bus->subscribe("device.register", [this](const core::message& msg) {
            handleDeviceRegistration(msg);
        });

        // Device commands
        bus->subscribe("device.command", [this](const core::message& msg) {
            handleDeviceCommand(msg);
        });

        // System queries
        bus->subscribe("system.query", [this](const core::message& msg) {
            handleSystemQuery(msg);
        });

        // Alert acknowledgment
        bus->subscribe("alert.acknowledge", [this](const core::message& msg) {
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

        m_logger->log(logger_module::log_level::info,
            "Initialized " + std::to_string(devices.size()) + " IoT devices");
    }

    void registerDevice(
        const std::string& device_id,
        device_type type,
        const std::string& location,
        double min_threshold,
        double max_threshold
    ) {
        device_config config;
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

        // In production, would store device configuration in database
        // Note: Container service doesn't have create_container method yet
        // and database service doesn't have store method
    }

    void handleDeviceTelemetry(const core::message& msg) {
        try {
            auto device_id = msg.metadata.headers.count("device_id") ? msg.metadata.headers.at("device_id") : "";
            auto value_str = msg.metadata.headers.count("value") ? msg.metadata.headers.at("value") : "0.0";
            auto value = std::stod(value_str);
            auto timestamp = std::chrono::system_clock::now();

            device_telemetry telemetry;
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
                m_logger->log(logger_module::log_level::debug,
                    "Processed " + std::to_string(total_messages.load()) + " telemetry messages");
            }

        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error,
                "Error processing telemetry: " + std::string(e.what()));
        }
    }

    void checkThresholds(const device_config& config, const device_telemetry& telemetry) {
        alert::severity severity = alert::INFO;
        std::string message;
        bool create_alert = false;

        if (telemetry.value < config.min_threshold) {
            double deviation = config.min_threshold - telemetry.value;
            double deviation_percent = (deviation / config.min_threshold) * 100;

            if (deviation_percent > 20) {
                severity = alert::CRITICAL;
                message = "Value critically below minimum threshold";
            } else if (deviation_percent > 10) {
                severity = alert::WARNING;
                message = "Value below minimum threshold";
            }
            create_alert = true;

        } else if (telemetry.value > config.max_threshold) {
            double deviation = telemetry.value - config.max_threshold;
            double deviation_percent = (deviation / config.max_threshold) * 100;

            if (deviation_percent > 20) {
                severity = alert::CRITICAL;
                message = "Value critically above maximum threshold";
            } else if (deviation_percent > 10) {
                severity = alert::WARNING;
                message = "Value above maximum threshold";
            }
            create_alert = true;
        }

        // Special case for motion detectors - always alert on motion
        if (config.type == device_type::MOTION_DETECTOR && telemetry.value > 0.5) {
            severity = alert::INFO;
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
        alert::severity severity,
        const std::string& message,
        double threshold,
        double actual
    ) {
        alert alert_obj;
        alert_obj.alert_id = generateAlertId();
        alert_obj.device_id = device_id;
        alert_obj.sev = severity;
        alert_obj.message = message;
        alert_obj.threshold_value = threshold;
        alert_obj.actual_value = actual;
        alert_obj.triggered_at = std::chrono::system_clock::now();

        {
            std::lock_guard<std::mutex> lock(alert_mutex);
            alert_queue.push(alert_obj);
            total_alerts++;
        }
        alert_cv.notify_one();

        // Publish alert
        publishAlert(alert_obj);

        m_logger->log(logger_module::log_level::warning,
            "[ALERT] " + getSeverityString(severity) + ": " + message +
            " (Device: " + device_id + ")");
    }

    void publishAlert(const alert& alert_obj) {
        core::message alert_msg;
        alert_msg.payload.topic = "alert.triggered";
        alert_msg.metadata.type = core::message_type::notification;
        alert_msg.metadata.headers["alert_id"] = alert_obj.alert_id;
        alert_msg.metadata.headers["device_id"] = alert_obj.device_id;
        alert_msg.metadata.headers["severity"] = std::to_string(static_cast<int>(alert_obj.sev));
        alert_msg.payload.set("message", alert_obj.message);

        // Set priority based on severity
        switch (alert_obj.sev) {
            case alert::EMERGENCY:
                alert_msg.set_priority(core::message_priority::critical);
                break;
            case alert::CRITICAL:
                alert_msg.set_priority(core::message_priority::high);
                break;
            case alert::WARNING:
                alert_msg.set_priority(core::message_priority::normal);
                break;
            default:
                alert_msg.set_priority(core::message_priority::low);
        }

        integrator->get_message_bus()->publish(alert_msg);
    }

    void handleDeviceRegistration(const core::message& msg) {
        auto device_id = msg.metadata.headers.count("device_id") ? msg.metadata.headers.at("device_id") : "";
        auto type_str = msg.metadata.headers.count("type") ? msg.metadata.headers.at("type") : "0";
        auto type = static_cast<device_type>(std::stoi(type_str));
        auto location = msg.metadata.headers.count("location") ? msg.metadata.headers.at("location") : "";

        registerDevice(device_id, type, location, 0.0, 100.0);

        m_logger->log(logger_module::log_level::info,
            "Registered new device: " + device_id + " at " + location);
    }

    void handleDeviceCommand(const core::message& msg) {
        auto device_id = msg.metadata.headers.count("device_id") ? msg.metadata.headers.at("device_id") : "";
        auto command = msg.metadata.headers.count("command") ? msg.metadata.headers.at("command") : "";

        m_logger->log(logger_module::log_level::info,
            "Executing command '" + command + "' on device " + device_id);

        // Forward command to device
        core::message cmd_msg("device.execute");
        cmd_msg.metadata.headers["device_id"] = device_id;
        cmd_msg.metadata.headers["command"] = command;
        cmd_msg.set_priority(core::message_priority::high);

        // In production, would use network service to send to device
        // network_svc->send_to_device(device_id, cmd_msg);
    }

    void handleSystemQuery(const core::message& msg) {
        auto query_type = msg.metadata.headers.count("query") ? msg.metadata.headers.at("query") : "";

        if (query_type == "status") {
            sendSystemStatus();
        } else if (query_type == "devices") {
            sendDeviceList();
        } else if (query_type == "telemetry") {
            sendLatestTelemetry();
        }
    }

    void handleAlertAcknowledgment(const core::message& msg) {
        auto alert_id = msg.metadata.headers.count("alert_id") ? msg.metadata.headers.at("alert_id") : "";
        auto user_id = msg.metadata.headers.count("user_id") ? msg.metadata.headers.at("user_id") : "";

        m_logger->log(logger_module::log_level::info,
            "Alert " + alert_id + " acknowledged by user " + user_id);

        // In production, would update alert status in database
        // database_svc->update("alerts", alert_id, "status", "acknowledged");
    }

    void storeTelemetry(const device_telemetry& telemetry) {
        // In production, would store telemetry in time-series database
        // Note: Container service doesn't have create_container method yet
        // and database service doesn't have store method
        // auto key = telemetry.device_id + "_" +
        //            std::to_string(telemetry.timestamp.time_since_epoch().count());
    }

    void processAnalytics(const device_telemetry& telemetry) {
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
                    createAlert(telemetry.device_id, alert::WARNING,
                               "Anomaly detected - unusual value pattern",
                               mean + 3 * stddev, telemetry.value);
                }
            }
        }
    }

    void sendSystemStatus() {
        core::message status("system.status");
        status.metadata.headers["total_devices"] = std::to_string(total_devices.load());
        status.metadata.headers["total_messages"] = std::to_string(total_messages.load());
        status.metadata.headers["total_alerts"] = std::to_string(total_alerts.load());
        status.metadata.headers["uptime"] = std::to_string(getUptime());

        integrator->get_message_bus()->publish(status);
    }

    void sendDeviceList() {
        core::message device_list("system.device_list");

        std::lock_guard<std::mutex> lock(devices_mutex);
        for (const auto& [id, config] : devices) {
            // In production, would serialize device info
            device_list.payload.set(id + "_id", id);
            device_list.payload.set(id + "_type", static_cast<int64_t>(config.type));
            device_list.payload.set(id + "_location", config.location);
            device_list.payload.set(id + "_enabled", config.enabled);
        }

        integrator->get_message_bus()->publish(device_list);
    }

    void sendLatestTelemetry() {
        core::message telemetry_msg("system.telemetry");

        std::lock_guard<std::mutex> lock(devices_mutex);
        for (const auto& [id, telemetry] : latest_telemetry) {
            // In production, would serialize telemetry data
            telemetry_msg.payload.set(id + "_device_id", id);
            telemetry_msg.payload.set(id + "_value", telemetry.value);
            telemetry_msg.payload.set(id + "_unit", telemetry.unit);
            telemetry_msg.payload.set(id + "_timestamp",
                static_cast<int64_t>(std::chrono::system_clock::to_time_t(telemetry.timestamp)));
        }

        integrator->get_message_bus()->publish(telemetry_msg);
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
                    core::message telemetry("device.telemetry");
                    telemetry.metadata.headers["device_id"] = id;

                    double value = 0.0;
                    switch (config.type) {
                        case device_type::TEMPERATURE_SENSOR:
                            value = temp_dist(gen);
                            break;
                        case device_type::HUMIDITY_SENSOR:
                            value = humidity_dist(gen);
                            break;
                        case device_type::ENERGY_METER:
                            value = energy_dist(gen);
                            break;
                        case device_type::MOTION_DETECTOR:
                            value = (motion_dist(gen) > 95) ? 1.0 : 0.0;
                            break;
                        case device_type::SMART_LIGHT:
                            value = light_dist(gen);
                            break;
                        default:
                            value = 0.0;
                    }

                    telemetry.metadata.headers["value"] = std::to_string(value);

                    integrator->get_message_bus()->publish(telemetry);
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
                    alert alert_obj = alert_queue.front();
                    alert_queue.pop();
                    lock.unlock();

                    // Process alert (send notifications, trigger workflows, etc.)
                    processAlert(alert_obj);

                    lock.lock();
                }
            }
        });
        alert_thread.detach();
    }

    void processAlert(const alert& alert_obj) {
        // In production, this would:
        // - Send push notifications
        // - Trigger automated responses
        // - Log to SIEM systems
        // - Execute remediation workflows

        // In production, would store alert in database
        // Note: Container service doesn't have create_container method yet
        // and database service doesn't have store method
    }

    std::string generateAlertId() const {
        static std::atomic<uint64_t> counter{0};
        return "alert-" + std::to_string(counter.fetch_add(1));
    }

    std::string getUnitForType(device_type type) const {
        switch (type) {
            case device_type::TEMPERATURE_SENSOR: return "°C";
            case device_type::HUMIDITY_SENSOR: return "%";
            case device_type::PRESSURE_SENSOR: return "hPa";
            case device_type::ENERGY_METER: return "W";
            case device_type::SMART_LIGHT: return "%";
            default: return "";
        }
    }

    std::string getSeverityString(alert::severity severity) const {
        switch (severity) {
            case alert::INFO: return "INFO";
            case alert::WARNING: return "WARNING";
            case alert::CRITICAL: return "CRITICAL";
            case alert::EMERGENCY: return "EMERGENCY";
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
        m_logger->log(logger_module::log_level::info, "\n=== IoT Monitoring System Starting ===");

        // Start the integrator (if it has a start method)
        // integrator->start();

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
        // Stop the integrator (if it has a stop method)
        // integrator->stop();

        std::string stats = "\n=== Final Statistics ===\n";
        stats += "Total devices monitored: " + std::to_string(total_devices) + "\n";
        stats += "Total messages processed: " + std::to_string(total_messages) + "\n";
        stats += "Total alerts generated: " + std::to_string(total_alerts) + "\n";
        stats += "======================";

        m_logger->log(logger_module::log_level::info, stats);
        m_logger->flush();
        m_logger->stop();
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
        std::stringstream dashboard;
        dashboard << "\n╔═══════════════════════════════════════════════════════╗\n";
        dashboard << "║           IoT Monitoring System Dashboard             ║\n";
        dashboard << "╠═══════════════════════════════════════════════════════╣\n";

        dashboard << "║ Active Devices: " << std::setw(38)
                  << total_devices.load() << " ║\n";

        dashboard << "║ Messages/sec: " << std::setw(40)
                  << (total_messages.load() / std::max(1, getUptime())) << " ║\n";

        dashboard << "║ Total Alerts: " << std::setw(40)
                  << total_alerts.load() << " ║\n";

        dashboard << "║ Uptime: " << std::setw(43)
                  << getUptime() << "s ║\n";

        dashboard << "╠═══════════════════════════════════════════════════════╣\n";
        dashboard << "║ Latest Telemetry:                                     ║\n";

        {
            std::lock_guard<std::mutex> lock(devices_mutex);
            int count = 0;
            for (const auto& [id, telemetry] : latest_telemetry) {
                if (count++ >= 5) break;  // Show top 5

                std::stringstream ss;
                ss << id << ": " << std::fixed << std::setprecision(2)
                   << telemetry.value << " " << telemetry.unit;

                dashboard << "║   " << std::left << std::setw(52)
                          << ss.str() << " ║\n";
            }
        }

        dashboard << "╚═══════════════════════════════════════════════════════╝";

        m_logger->log(logger_module::log_level::info, dashboard.str());
    }
};

int main(int argc, char* argv[]) {
    try {
        iot_monitoring_system iot_system;
        iot_system.start();

    } catch (const std::exception& e) {
        // Create a minimal logger for error reporting
        auto error_logger = std::make_shared<logger_module::logger>(true, 8192);
        error_logger->add_writer(std::make_unique<logger_module::console_writer>());
        error_logger->log(logger_module::log_level::error, "Error: " + std::string(e.what()));
        error_logger->stop();
        return 1;
    }

    return 0;
}