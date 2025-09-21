#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/config.h>
#include <logger_system/sources/logger/logger.h>
#include <logger_system/sources/logger/writers/console_writer.h>
#include <logger_system/sources/logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

using namespace kcenon::messaging::integrations;
using namespace kcenon::messaging::config;
using namespace kcenon::messaging::core;

int main() {
    // Initialize logger
    logger_module::logger_config logger_config;
    logger_config.min_level = logger_module::log_level::info;
    logger_config.pattern = "[{timestamp}] [{level}] {message}";
    logger_config.enable_async = true;

    auto logger = std::make_shared<logger_module::logger>(logger_config);
    logger->add_writer(std::make_unique<logger_module::console_writer>());
    logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
        "basic_usage_example.log", 5 * 1024 * 1024, 3));
    logger->start();

    logger->log(logger_module::log_level::info, "Messaging System Basic Usage Example");
    logger->log(logger_module::log_level::info, "=====================================");

    try {
        // 1. Create and configure the messaging system
        logger->log(logger_module::log_level::info, "1. Creating messaging system...");

        config_builder builder;
        auto config = builder
            .set_environment("development")
            .set_worker_threads(4)
            .set_queue_size(10000)
            .enable_compression(true)
            .build();

        auto integrator = std::make_unique<system_integrator>(config);

        // 2. Initialize the system
        logger->log(logger_module::log_level::info, "2. Initializing system...");
        if (!integrator->initialize()) {
            logger->log(logger_module::log_level::error, "Failed to initialize messaging system!");
            logger->stop();
            return 1;
        }

        logger->log(logger_module::log_level::info, "   System initialized successfully!");

        // 3. Set up message subscribers
        logger->log(logger_module::log_level::info, "3. Setting up message subscribers...");

        // Simple message handler
        integrator->subscribe("user.login", [logger](const message& msg) {
            logger->log(logger_module::log_level::info, "   [Login Handler] User logged in!");

            auto user_it = msg.payload.data.find("username");
            if (user_it != msg.payload.data.end() &&
                std::holds_alternative<std::string>(user_it->second)) {
                logger->log(logger_module::log_level::info,
                    "   [Login Handler] Username: " + std::get<std::string>(user_it->second));
            }
        });

        // Order processing handler
        integrator->subscribe("order.created", [logger](const message& msg) {
            logger->log(logger_module::log_level::info, "   [Order Handler] New order received!");

            auto order_id_it = msg.payload.data.find("order_id");
            auto amount_it = msg.payload.data.find("amount");

            if (order_id_it != msg.payload.data.end() &&
                std::holds_alternative<int64_t>(order_id_it->second)) {
                logger->log(logger_module::log_level::info,
                    "   [Order Handler] Order ID: " +
                    std::to_string(std::get<int64_t>(order_id_it->second)));
            }

            if (amount_it != msg.payload.data.end() &&
                std::holds_alternative<double>(amount_it->second)) {
                std::stringstream ss;
                ss << "   [Order Handler] Amount: $" << std::get<double>(amount_it->second);
                logger->log(logger_module::log_level::info, ss.str());
            }
        });

        // Notification handler
        integrator->subscribe("notification.*", [logger](const message& msg) {
            logger->log(logger_module::log_level::info,
                "   [Notification Handler] Topic: " + msg.payload.topic);

            auto message_it = msg.payload.data.find("message");
            if (message_it != msg.payload.data.end() &&
                std::holds_alternative<std::string>(message_it->second)) {
                logger->log(logger_module::log_level::info,
                    "   [Notification Handler] Message: " +
                    std::get<std::string>(message_it->second));
            }
        });

        logger->log(logger_module::log_level::info, "   Subscribers registered!");

        // 4. Publish messages
        logger->log(logger_module::log_level::info, "4. Publishing messages...");

        // User login message
        {
            message_payload login_payload;
            login_payload.topic = "user.login";
            login_payload.data["username"] = std::string("john_doe");
            login_payload.data["timestamp"] = int64_t(std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

            integrator->publish("user.login", login_payload, "auth_service");
            logger->log(logger_module::log_level::debug, "   Published user login message");
        }

        // Order creation message
        {
            message_payload order_payload;
            order_payload.topic = "order.created";
            order_payload.data["order_id"] = int64_t(12345);
            order_payload.data["amount"] = double(99.99);
            order_payload.data["customer_id"] = std::string("customer_456");

            integrator->publish("order.created", order_payload, "order_service");
            logger->log(logger_module::log_level::debug, "   Published order creation message");
        }

        // Notification messages
        {
            message_payload notification_payload;
            notification_payload.topic = "notification.email";
            notification_payload.data["message"] = std::string("Welcome to our service!");
            notification_payload.data["recipient"] = std::string("john_doe@example.com");

            integrator->publish("notification.email", notification_payload, "notification_service");
            logger->log(logger_module::log_level::debug, "   Published email notification");
        }

        {
            message_payload sms_payload;
            sms_payload.topic = "notification.sms";
            sms_payload.data["message"] = std::string("Your order has been confirmed");
            sms_payload.data["phone"] = std::string("+1234567890");

            integrator->publish("notification.sms", sms_payload, "notification_service");
            logger->log(logger_module::log_level::debug, "   Published SMS notification");
        }

        logger->log(logger_module::log_level::info, "   All messages published!");

        // 5. Wait for message processing
        logger->log(logger_module::log_level::info, "5. Processing messages...");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        logger->log(logger_module::log_level::info, "   Message processing complete!");

        // 6. Check system health
        logger->log(logger_module::log_level::info, "6. Checking system health...");
        auto health = integrator->check_system_health();

        logger->log(logger_module::log_level::info, "   System Health Report:");
        logger->log(logger_module::log_level::info,
            "   - Message bus healthy: " + std::string(health.message_bus_healthy ? "Yes" : "No"));
        logger->log(logger_module::log_level::info,
            "   - Active services: " + std::to_string(health.active_services));
        logger->log(logger_module::log_level::info,
            "   - Total messages processed: " + std::to_string(health.total_messages_processed));
        logger->log(logger_module::log_level::info,
            "   - Last check: " + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                health.last_check.time_since_epoch()).count()) + " (Unix timestamp)");

        // 7. Demonstrate service access
        logger->log(logger_module::log_level::info, "7. Accessing services through container...");

        auto& container = integrator->get_container();
        auto registered_services = container.get_registered_services();

        logger->log(logger_module::log_level::info,
            "   Registered services (" + std::to_string(registered_services.size()) + "):");
        for (const auto& service_name : registered_services) {
            logger->log(logger_module::log_level::info, "   - " + service_name);
        }

        // 8. Configuration access
        logger->log(logger_module::log_level::info, "8. System configuration:");
        const auto& sys_config = integrator->get_config();
        logger->log(logger_module::log_level::info, "   - Environment: " + sys_config.environment);
        logger->log(logger_module::log_level::info, "   - System name: " + sys_config.system_name);
        logger->log(logger_module::log_level::info, "   - Version: " + sys_config.version);
        logger->log(logger_module::log_level::info,
            "   - Worker threads: " + std::to_string(sys_config.message_bus.worker_threads));
        logger->log(logger_module::log_level::info,
            "   - Max queue size: " + std::to_string(sys_config.message_bus.max_queue_size));

        // 9. Shutdown
        logger->log(logger_module::log_level::info, "9. Shutting down system...");
        integrator->shutdown();
        logger->log(logger_module::log_level::info, "   System shutdown complete!");

        logger->log(logger_module::log_level::info, "Example completed successfully!");
        logger->flush();
        logger->stop();

    } catch (const std::exception& e) {
        if (logger) {
            logger->log(logger_module::log_level::error, "Error: " + std::string(e.what()));
            logger->stop();
        }
        return 1;
    }

    return 0;
}