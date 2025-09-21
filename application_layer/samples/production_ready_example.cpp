/**
 * @file production_ready_example.cpp
 * @brief Production-ready messaging system with signal handling, config, metrics, and error recovery
 *
 * This example demonstrates production-ready features including:
 * - Signal handling for graceful shutdown (SIGINT, SIGTERM)
 * - Configuration file loading
 * - Error recovery and retry mechanisms
 * - Metric collection and reporting
 * - Health check monitoring
 */

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/config.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <csignal>
#include <atomic>
#include <map>
#include <mutex>
#include <condition_variable>

using namespace kcenon::messaging::integrations;
using namespace kcenon::messaging::config;
using namespace kcenon::messaging::core;
using namespace std::chrono_literals;

// Global shutdown flag for signal handling
std::atomic<bool> g_shutdown_requested{false};
std::condition_variable g_shutdown_cv;
std::mutex g_shutdown_mutex;

// Metrics collection
class metrics_collector {
public:
    struct metrics {
        std::atomic<uint64_t> messages_sent{0};
        std::atomic<uint64_t> messages_received{0};
        std::atomic<uint64_t> errors_encountered{0};
        std::atomic<uint64_t> retries_attempted{0};
        std::atomic<uint64_t> successful_retries{0};
        std::chrono::steady_clock::time_point start_time;

        metrics() : start_time(std::chrono::steady_clock::now()) {}
    };

private:
    metrics m_metrics;
    std::shared_ptr<logger_module::logger> m_logger;
    std::thread m_reporter_thread;
    std::atomic<bool> m_running{false};

public:
    explicit metrics_collector(std::shared_ptr<logger_module::logger> logger)
        : m_logger(logger) {}

    ~metrics_collector() {
        stop();
    }

    void start(std::chrono::seconds report_interval = 10s) {
        if (m_running.exchange(true)) {
            return;
        }

        m_reporter_thread = std::thread([this, report_interval] {
            while (m_running) {
                std::unique_lock<std::mutex> lock(g_shutdown_mutex);
                if (g_shutdown_cv.wait_for(lock, report_interval,
                    [] { return g_shutdown_requested.load(); })) {
                    break;
                }
                report();
            }
        });
    }

    void stop() {
        if (!m_running.exchange(false)) {
            return;
        }

        if (m_reporter_thread.joinable()) {
            m_reporter_thread.join();
        }

        // Final report
        report();
    }

    void increment_sent() { m_metrics.messages_sent++; }
    void increment_received() { m_metrics.messages_received++; }
    void increment_errors() { m_metrics.errors_encountered++; }
    void increment_retries() { m_metrics.retries_attempted++; }
    void increment_successful_retries() { m_metrics.successful_retries++; }

    void report() {
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - m_metrics.start_time).count();

        std::stringstream ss;
        ss << "=== Metrics Report ===" << std::endl
           << "Uptime: " << uptime << " seconds" << std::endl
           << "Messages Sent: " << m_metrics.messages_sent.load() << std::endl
           << "Messages Received: " << m_metrics.messages_received.load() << std::endl
           << "Errors Encountered: " << m_metrics.errors_encountered.load() << std::endl
           << "Retries Attempted: " << m_metrics.retries_attempted.load() << std::endl
           << "Successful Retries: " << m_metrics.successful_retries.load() << std::endl
           << "Success Rate: "
           << (m_metrics.messages_sent > 0
               ? (100.0 * m_metrics.messages_received / m_metrics.messages_sent)
               : 0.0)
           << "%" << std::endl
           << "===================";

        m_logger->log(logger_module::log_level::info, ss.str());
    }
};

// Configuration loader
class config_loader {
public:
    struct app_config {
        std::string environment = "development";
        int worker_threads = 4;
        size_t queue_size = 10000;
        bool enable_compression = false;
        int max_retries = 3;
        std::chrono::milliseconds retry_delay{100};
        std::chrono::seconds health_check_interval{30};
        std::string log_file = "messaging_system.log";
        logger_module::log_level log_level = logger_module::log_level::info;
    };

    static app_config load_from_file(const std::string& filename,
                                     std::shared_ptr<logger_module::logger> logger) {
        app_config config;

        std::ifstream file(filename);
        if (!file.is_open()) {
            logger->log(logger_module::log_level::warning,
                       "Config file not found: " + filename + ". Using defaults.");
            return config;
        }

        logger->log(logger_module::log_level::info, "Loading configuration from: " + filename);

        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;

            auto equals_pos = line.find('=');
            if (equals_pos == std::string::npos) continue;

            std::string key = line.substr(0, equals_pos);
            std::string value = line.substr(equals_pos + 1);

            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Parse known configuration keys
            if (key == "environment") {
                config.environment = value;
            } else if (key == "worker_threads") {
                config.worker_threads = std::stoi(value);
            } else if (key == "queue_size") {
                config.queue_size = std::stoull(value);
            } else if (key == "enable_compression") {
                config.enable_compression = (value == "true" || value == "1");
            } else if (key == "max_retries") {
                config.max_retries = std::stoi(value);
            } else if (key == "retry_delay_ms") {
                config.retry_delay = std::chrono::milliseconds(std::stoi(value));
            } else if (key == "health_check_interval_sec") {
                config.health_check_interval = std::chrono::seconds(std::stoi(value));
            } else if (key == "log_file") {
                config.log_file = value;
            } else if (key == "log_level") {
                if (value == "debug") config.log_level = logger_module::log_level::debug;
                else if (value == "info") config.log_level = logger_module::log_level::info;
                else if (value == "warning") config.log_level = logger_module::log_level::warning;
                else if (value == "error") config.log_level = logger_module::log_level::error;
                else if (value == "critical") config.log_level = logger_module::log_level::critical;
            }
        }

        logger->log(logger_module::log_level::info, "Configuration loaded successfully");
        return config;
    }

    static void save_default_config(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return;
        }

        file << "# Messaging System Configuration\n"
             << "# Generated default configuration file\n\n"
             << "# Environment (development, staging, production)\n"
             << "environment=development\n\n"
             << "# Number of worker threads for message processing\n"
             << "worker_threads=4\n\n"
             << "# Maximum queue size for messages\n"
             << "queue_size=10000\n\n"
             << "# Enable message compression\n"
             << "enable_compression=false\n\n"
             << "# Maximum retry attempts for failed messages\n"
             << "max_retries=3\n\n"
             << "# Delay between retry attempts (milliseconds)\n"
             << "retry_delay_ms=100\n\n"
             << "# Health check interval (seconds)\n"
             << "health_check_interval_sec=30\n\n"
             << "# Log file path\n"
             << "log_file=messaging_system.log\n\n"
             << "# Log level (debug, info, warning, error, critical)\n"
             << "log_level=info\n";
    }
};

// Retry mechanism for message publishing
class retry_handler {
private:
    std::shared_ptr<logger_module::logger> m_logger;
    metrics_collector* m_metrics;
    int m_max_retries;
    std::chrono::milliseconds m_retry_delay;

public:
    retry_handler(std::shared_ptr<logger_module::logger> logger,
                  metrics_collector* metrics,
                  int max_retries = 3,
                  std::chrono::milliseconds retry_delay = 100ms)
        : m_logger(logger)
        , m_metrics(metrics)
        , m_max_retries(max_retries)
        , m_retry_delay(retry_delay) {}

    template<typename Func>
    bool execute_with_retry(Func&& func, const std::string& operation_name) {
        for (int attempt = 0; attempt < m_max_retries; ++attempt) {
            try {
                func();
                if (attempt > 0) {
                    m_logger->log(logger_module::log_level::info,
                        "Operation '" + operation_name + "' succeeded after " +
                        std::to_string(attempt) + " retries");
                    if (m_metrics) {
                        m_metrics->increment_successful_retries();
                    }
                }
                return true;
            } catch (const std::exception& e) {
                if (m_metrics) {
                    m_metrics->increment_errors();
                }

                if (attempt < m_max_retries - 1) {
                    m_logger->log(logger_module::log_level::warning,
                        "Operation '" + operation_name + "' failed (attempt " +
                        std::to_string(attempt + 1) + "/" + std::to_string(m_max_retries) +
                        "): " + e.what() + ". Retrying...");
                    if (m_metrics) {
                        m_metrics->increment_retries();
                    }
                    std::this_thread::sleep_for(m_retry_delay * (attempt + 1));
                } else {
                    m_logger->log(logger_module::log_level::error,
                        "Operation '" + operation_name + "' failed after " +
                        std::to_string(m_max_retries) + " attempts: " + e.what());
                }
            }
        }
        return false;
    }
};

// Signal handlers
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived signal " << signal << ". Initiating graceful shutdown..." << std::endl;
        g_shutdown_requested = true;
        g_shutdown_cv.notify_all();
    }
}

void setup_signal_handlers() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

// Health check monitor
class health_monitor {
private:
    std::shared_ptr<logger_module::logger> m_logger;
    system_integrator* m_integrator;
    std::thread m_monitor_thread;
    std::atomic<bool> m_running{false};
    std::chrono::seconds m_check_interval;

public:
    health_monitor(std::shared_ptr<logger_module::logger> logger,
                   system_integrator* integrator,
                   std::chrono::seconds interval = 30s)
        : m_logger(logger)
        , m_integrator(integrator)
        , m_check_interval(interval) {}

    ~health_monitor() {
        stop();
    }

    void start() {
        if (m_running.exchange(true)) {
            return;
        }

        m_monitor_thread = std::thread([this] {
            while (m_running) {
                std::unique_lock<std::mutex> lock(g_shutdown_mutex);
                if (g_shutdown_cv.wait_for(lock, m_check_interval,
                    [] { return g_shutdown_requested.load(); })) {
                    break;
                }

                perform_health_check();
            }
        });
    }

    void stop() {
        if (!m_running.exchange(false)) {
            return;
        }

        if (m_monitor_thread.joinable()) {
            m_monitor_thread.join();
        }
    }

private:
    void perform_health_check() {
        try {
            auto health = m_integrator->check_system_health();

            std::string status = health.message_bus_healthy ? "HEALTHY" : "UNHEALTHY";
            m_logger->log(health.message_bus_healthy ?
                         logger_module::log_level::debug :
                         logger_module::log_level::warning,
                         "Health Check: " + status +
                         " | Active Services: " + std::to_string(health.active_services) +
                         " | Messages Processed: " + std::to_string(health.total_messages_processed));

            if (!health.message_bus_healthy) {
                m_logger->log(logger_module::log_level::error,
                             "System health check failed! Attempting recovery...");
                // Implement recovery logic here
            }
        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error,
                         "Health check failed with exception: " + std::string(e.what()));
        }
    }
};

int main(int argc, char* argv[]) {
    // Determine config file path
    std::string config_file = "messaging_config.ini";
    if (argc > 1) {
        config_file = argv[1];
    }

    // Initialize logger early for startup messages
    logger_module::logger_config logger_config;
    logger_config.min_level = logger_module::log_level::debug; // Start with debug
    logger_config.pattern = "[{timestamp}] [{level}] [{thread}] {message}";
    logger_config.enable_async = true;
    logger_config.async_queue_size = 8192;
    logger_config.enable_file_line = true;

    auto logger = std::make_shared<logger_module::logger>(logger_config);
    logger->add_writer(std::make_unique<logger_module::console_writer>());

    try {
        // Load configuration
        auto app_config = config_loader::load_from_file(config_file, logger);

        // Create default config file if it doesn't exist
        std::ifstream test_file(config_file);
        if (!test_file.is_open()) {
            config_loader::save_default_config(config_file);
            logger->log(logger_module::log_level::info,
                       "Created default configuration file: " + config_file);
        }

        // Update logger based on config
        logger_config.min_level = app_config.log_level;
        logger = std::make_shared<logger_module::logger>(logger_config);
        logger->add_writer(std::make_unique<logger_module::console_writer>());
        logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            app_config.log_file, 10 * 1024 * 1024, 5));
        logger->start();

        logger->log(logger_module::log_level::info, "Production-Ready Messaging System");
        logger->log(logger_module::log_level::info, "=====================================");
        logger->log(logger_module::log_level::info, "Environment: " + app_config.environment);

        // Setup signal handlers
        setup_signal_handlers();
        logger->log(logger_module::log_level::info, "Signal handlers installed (SIGINT, SIGTERM)");

        // Initialize metrics collector
        metrics_collector metrics(logger);
        metrics.start(10s);
        logger->log(logger_module::log_level::info, "Metrics collection started");

        // Initialize retry handler
        retry_handler retry(logger, &metrics, app_config.max_retries, app_config.retry_delay);

        // Create and configure the messaging system
        logger->log(logger_module::log_level::info, "Initializing messaging system...");

        config_builder builder;
        auto config = builder
            .set_environment(app_config.environment)
            .set_worker_threads(app_config.worker_threads)
            .set_queue_size(app_config.queue_size)
            .enable_compression(app_config.enable_compression)
            .build();

        auto integrator = std::make_unique<system_integrator>(config);

        // Initialize with retry
        bool init_success = retry.execute_with_retry([&integrator] {
            if (!integrator->initialize()) {
                throw std::runtime_error("Failed to initialize messaging system");
            }
        }, "system_initialization");

        if (!init_success) {
            logger->log(logger_module::log_level::critical, "Failed to initialize system after retries!");
            return 1;
        }

        logger->log(logger_module::log_level::info, "System initialized successfully!");

        // Start health monitor
        health_monitor health(logger, integrator.get(), app_config.health_check_interval);
        health.start();
        logger->log(logger_module::log_level::info, "Health monitoring started");

        // Set up message subscribers with error handling
        logger->log(logger_module::log_level::info, "Setting up message subscribers...");

        // User login handler with error recovery
        integrator->subscribe("user.login", [&logger, &metrics, &retry](const message& msg) {
            metrics.increment_received();

            try {
                logger->log(logger_module::log_level::info, "[Login Handler] Processing login event");

                auto user_it = msg.payload.data.find("username");
                if (user_it != msg.payload.data.end() &&
                    std::holds_alternative<std::string>(user_it->second)) {
                    std::string username = std::get<std::string>(user_it->second);

                    // Simulate processing with potential failure
                    if (username == "error_user") {
                        throw std::runtime_error("Simulated login error");
                    }

                    logger->log(logger_module::log_level::info,
                        "[Login Handler] User logged in: " + username);
                }
            } catch (const std::exception& e) {
                logger->log(logger_module::log_level::error,
                    "[Login Handler] Error processing login: " + std::string(e.what()));
                metrics.increment_errors();
                // Could implement retry or error queue here
            }
        });

        // Order processing with validation
        integrator->subscribe("order.created", [&logger, &metrics](const message& msg) {
            metrics.increment_received();

            try {
                logger->log(logger_module::log_level::info, "[Order Handler] Processing new order");

                // Validate required fields
                auto order_id_it = msg.payload.data.find("order_id");
                auto amount_it = msg.payload.data.find("amount");

                if (order_id_it == msg.payload.data.end() ||
                    !std::holds_alternative<int64_t>(order_id_it->second)) {
                    throw std::runtime_error("Invalid or missing order_id");
                }

                if (amount_it == msg.payload.data.end() ||
                    !std::holds_alternative<double>(amount_it->second)) {
                    throw std::runtime_error("Invalid or missing amount");
                }

                int64_t order_id = std::get<int64_t>(order_id_it->second);
                double amount = std::get<double>(amount_it->second);

                // Validate business rules
                if (amount <= 0) {
                    throw std::runtime_error("Invalid order amount: " + std::to_string(amount));
                }

                std::stringstream ss;
                ss << "[Order Handler] Order processed - ID: " << order_id
                   << ", Amount: $" << std::fixed << std::setprecision(2) << amount;
                logger->log(logger_module::log_level::info, ss.str());

            } catch (const std::exception& e) {
                logger->log(logger_module::log_level::error,
                    "[Order Handler] Failed to process order: " + std::string(e.what()));
                metrics.increment_errors();
            }
        });

        // System metrics handler
        integrator->subscribe("system.metrics", [&logger, &metrics](const message& msg) {
            metrics.increment_received();
            logger->log(logger_module::log_level::debug, "[Metrics] Received system metrics update");
        });

        logger->log(logger_module::log_level::info, "Subscribers registered!");

        // Main message publishing loop with graceful shutdown
        logger->log(logger_module::log_level::info, "Starting message publishing...");
        logger->log(logger_module::log_level::info, "Press Ctrl+C for graceful shutdown");

        std::thread publisher_thread([&] {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> order_dist(1000, 9999);
            std::uniform_real_distribution<> amount_dist(10.0, 500.0);

            int message_count = 0;

            while (!g_shutdown_requested) {
                // Publish various message types

                // User login message
                if (message_count % 5 == 0) {
                    message_payload login_payload;
                    login_payload.topic = "user.login";
                    login_payload.data["username"] = (message_count % 20 == 0) ?
                        std::string("error_user") : std::string("user_" + std::to_string(message_count));
                    login_payload.data["timestamp"] = int64_t(std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count());

                    bool success = retry.execute_with_retry([&] {
                        integrator->publish("user.login", login_payload, "auth_service");
                        metrics.increment_sent();
                    }, "publish_login");

                    if (!success) {
                        logger->log(logger_module::log_level::error, "Failed to publish login message");
                    }
                }

                // Order message
                if (message_count % 3 == 0) {
                    message_payload order_payload;
                    order_payload.topic = "order.created";
                    order_payload.data["order_id"] = int64_t(order_dist(gen));
                    order_payload.data["amount"] = amount_dist(gen);
                    order_payload.data["customer_id"] = std::string("customer_" + std::to_string(message_count));

                    bool success = retry.execute_with_retry([&] {
                        integrator->publish("order.created", order_payload, "order_service");
                        metrics.increment_sent();
                    }, "publish_order");

                    if (!success) {
                        logger->log(logger_module::log_level::error, "Failed to publish order message");
                    }
                }

                // System metrics
                if (message_count % 10 == 0) {
                    message_payload metrics_payload;
                    metrics_payload.topic = "system.metrics";
                    metrics_payload.data["cpu_usage"] = double(rand() % 100);
                    metrics_payload.data["memory_usage"] = double(rand() % 100);
                    metrics_payload.data["timestamp"] = int64_t(std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count());

                    integrator->publish("system.metrics", metrics_payload, "monitoring_service");
                    metrics.increment_sent();
                }

                message_count++;

                // Check for shutdown every 100ms
                std::unique_lock<std::mutex> lock(g_shutdown_mutex);
                g_shutdown_cv.wait_for(lock, 100ms, [] { return g_shutdown_requested.load(); });
            }
        });

        // Wait for shutdown signal
        {
            std::unique_lock<std::mutex> lock(g_shutdown_mutex);
            g_shutdown_cv.wait(lock, [] { return g_shutdown_requested.load(); });
        }

        // Graceful shutdown sequence
        logger->log(logger_module::log_level::info, "Initiating graceful shutdown...");

        // Stop publisher thread
        if (publisher_thread.joinable()) {
            publisher_thread.join();
        }

        // Allow pending messages to process
        logger->log(logger_module::log_level::info, "Processing remaining messages...");
        std::this_thread::sleep_for(1s);

        // Stop health monitor
        health.stop();
        logger->log(logger_module::log_level::info, "Health monitor stopped");

        // Stop metrics collection
        metrics.stop();
        logger->log(logger_module::log_level::info, "Metrics collection stopped");

        // Shutdown messaging system
        integrator->shutdown();
        logger->log(logger_module::log_level::info, "Messaging system shutdown complete");

        logger->log(logger_module::log_level::info, "Application terminated successfully!");
        logger->flush();
        logger->stop();

    } catch (const std::exception& e) {
        if (logger) {
            logger->log(logger_module::log_level::critical,
                       "Fatal error: " + std::string(e.what()));
            logger->stop();
        } else {
            std::cerr << "Fatal error: " << e.what() << std::endl;
        }
        return 1;
    }

    return 0;
}