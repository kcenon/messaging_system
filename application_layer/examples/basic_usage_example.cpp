#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/config.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace kcenon::messaging::integrations;
using namespace kcenon::messaging::config;
using namespace kcenon::messaging::core;

int main() {
    std::cout << "Messaging System Basic Usage Example\n";
    std::cout << "=====================================\n\n";

    try {
        // 1. Create and configure the messaging system
        std::cout << "1. Creating messaging system...\n";

        config_builder builder;
        auto config = builder
            .set_environment("development")
            .set_worker_threads(4)
            .set_queue_size(10000)
            .enable_compression(true)
            .build();

        auto integrator = std::make_unique<system_integrator>(config);

        // 2. Initialize the system
        std::cout << "2. Initializing system...\n";
        if (!integrator->initialize()) {
            std::cerr << "Failed to initialize messaging system!\n";
            return 1;
        }

        std::cout << "   System initialized successfully!\n\n";

        // 3. Set up message subscribers
        std::cout << "3. Setting up message subscribers...\n";

        // Simple message handler
        integrator->subscribe("user.login", [](const message& msg) {
            std::cout << "   [Login Handler] User logged in!\n";

            auto user_it = msg.payload.data.find("username");
            if (user_it != msg.payload.data.end() &&
                std::holds_alternative<std::string>(user_it->second)) {
                std::cout << "   [Login Handler] Username: "
                          << std::get<std::string>(user_it->second) << "\n";
            }
        });

        // Order processing handler
        integrator->subscribe("order.created", [](const message& msg) {
            std::cout << "   [Order Handler] New order received!\n";

            auto order_id_it = msg.payload.data.find("order_id");
            auto amount_it = msg.payload.data.find("amount");

            if (order_id_it != msg.payload.data.end() &&
                std::holds_alternative<int64_t>(order_id_it->second)) {
                std::cout << "   [Order Handler] Order ID: "
                          << std::get<int64_t>(order_id_it->second) << "\n";
            }

            if (amount_it != msg.payload.data.end() &&
                std::holds_alternative<double>(amount_it->second)) {
                std::cout << "   [Order Handler] Amount: $"
                          << std::get<double>(amount_it->second) << "\n";
            }
        });

        // Notification handler
        integrator->subscribe("notification.*", [](const message& msg) {
            std::cout << "   [Notification Handler] Topic: " << msg.payload.topic << "\n";

            auto message_it = msg.payload.data.find("message");
            if (message_it != msg.payload.data.end() &&
                std::holds_alternative<std::string>(message_it->second)) {
                std::cout << "   [Notification Handler] Message: "
                          << std::get<std::string>(message_it->second) << "\n";
            }
        });

        std::cout << "   Subscribers registered!\n\n";

        // 4. Publish messages
        std::cout << "4. Publishing messages...\n";

        // User login message
        {
            message_payload login_payload;
            login_payload.topic = "user.login";
            login_payload.data["username"] = std::string("john_doe");
            login_payload.data["timestamp"] = int64_t(std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

            integrator->publish("user.login", login_payload, "auth_service");
            std::cout << "   Published user login message\n";
        }

        // Order creation message
        {
            message_payload order_payload;
            order_payload.topic = "order.created";
            order_payload.data["order_id"] = int64_t(12345);
            order_payload.data["amount"] = double(99.99);
            order_payload.data["customer_id"] = std::string("customer_456");

            integrator->publish("order.created", order_payload, "order_service");
            std::cout << "   Published order creation message\n";
        }

        // Notification messages
        {
            message_payload notification_payload;
            notification_payload.topic = "notification.email";
            notification_payload.data["message"] = std::string("Welcome to our service!");
            notification_payload.data["recipient"] = std::string("john_doe@example.com");

            integrator->publish("notification.email", notification_payload, "notification_service");
            std::cout << "   Published email notification\n";
        }

        {
            message_payload sms_payload;
            sms_payload.topic = "notification.sms";
            sms_payload.data["message"] = std::string("Your order has been confirmed");
            sms_payload.data["phone"] = std::string("+1234567890");

            integrator->publish("notification.sms", sms_payload, "notification_service");
            std::cout << "   Published SMS notification\n";
        }

        std::cout << "\n   All messages published!\n\n";

        // 5. Wait for message processing
        std::cout << "5. Processing messages...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "   Message processing complete!\n\n";

        // 6. Check system health
        std::cout << "6. Checking system health...\n";
        auto health = integrator->check_system_health();

        std::cout << "   System Health Report:\n";
        std::cout << "   - Message bus healthy: " << (health.message_bus_healthy ? "Yes" : "No") << "\n";
        std::cout << "   - Active services: " << health.active_services << "\n";
        std::cout << "   - Total messages processed: " << health.total_messages_processed << "\n";
        std::cout << "   - Last check: " << std::chrono::duration_cast<std::chrono::seconds>(
            health.last_check.time_since_epoch()).count() << " (Unix timestamp)\n\n";

        // 7. Demonstrate service access
        std::cout << "7. Accessing services through container...\n";

        auto& container = integrator->get_container();
        auto registered_services = container.get_registered_services();

        std::cout << "   Registered services (" << registered_services.size() << "):\n";
        for (const auto& service_name : registered_services) {
            std::cout << "   - " << service_name << "\n";
        }
        std::cout << "\n";

        // 8. Configuration access
        std::cout << "8. System configuration:\n";
        const auto& sys_config = integrator->get_config();
        std::cout << "   - Environment: " << sys_config.environment << "\n";
        std::cout << "   - System name: " << sys_config.system_name << "\n";
        std::cout << "   - Version: " << sys_config.version << "\n";
        std::cout << "   - Worker threads: " << sys_config.message_bus.worker_threads << "\n";
        std::cout << "   - Max queue size: " << sys_config.message_bus.max_queue_size << "\n";
        std::cout << "\n";

        // 9. Shutdown
        std::cout << "9. Shutting down system...\n";
        integrator->shutdown();
        std::cout << "   System shutdown complete!\n\n";

        std::cout << "Example completed successfully!\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}