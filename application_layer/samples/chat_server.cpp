/**
 * @file chat_server.cpp
 * @brief Real-time chat server using the messaging system
 *
 * This example demonstrates how to build a complete chat server
 * using the application layer's messaging capabilities.
 */

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/services/network/network_service.h>
#include <logger_system/sources/logger/logger.h>
#include <logger_system/sources/logger/writers/console_writer.h>
#include <logger_system/sources/logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

class chat_server {
private:
    std::unique_ptr<integrations::system_integrator> system_integrator;
    std::unique_ptr<services::network_service> network_service;
    std::shared_ptr<logger_module::logger> m_logger;

    // User management
    struct User {
        std::string id;
        std::string nickname;
        std::chrono::steady_clock::time_point last_activity;
    };

    std::unordered_map<std::string, User> users;
    std::mutex users_mutex;
    std::atomic<bool> running{true};

public:
    chat_server() {
        // Initialize logger
        m_logger = std::make_shared<logger_module::logger>(true, 8192);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "chat_server.log", 10 * 1024 * 1024, 5)); // 10MB per file, 5 files
        m_logger->set_min_level(logger_module::log_level::info);
        m_logger->start();

        m_logger->log(logger_module::log_level::info, "Initializing chat server...");

        // Initialize the messaging system with optimized configuration
        config::config_builder builder;
        auto config = builder
            .set_environment("production")
            .set_worker_threads(8)  // Handle multiple concurrent users
            .set_queue_size(50000)   // Large queue for message buffering
            .set_max_message_size(4096)
            .enable_persistence(true)
            .enable_monitoring(true)
            .build();

        system_integrator = std::make_unique<integrations::system_integrator>(config);
        network_service = std::make_unique<services::network_service>();

        setupMessageHandlers();
    }

    void setupMessageHandlers() {
        auto& message_bus = system_integrator->get_message_bus();

        // Handle user connections
        message_bus.subscribe("user.connect", [this](const core::message& incoming_msg) {
            handleUserConnect(incoming_msg);
        });

        // Handle user disconnections
        message_bus.subscribe("user.disconnect", [this](const core::message& disconnect_msg) {
            handleUserDisconnect(disconnect_msg);
        });

        // Handle chat messages
        message_bus.subscribe("chat.message", [this](const core::message& chat_msg) {
            handleChatMessage(chat_msg);
        });

        // Handle private messages
        message_bus.subscribe("chat.private", [this](const core::message& private_msg) {
            handlePrivateMessage(private_msg);
        });

        // Handle room operations
        message_bus.subscribe("room.join", [this](const core::message& join_msg) {
            handleRoomJoin(join_msg);
        });

        message_bus.subscribe("room.leave", [this](const core::message& leave_msg) {
            handleRoomLeave(leave_msg);
        });
    }

    void handleUserConnect(const core::message& connect_message) {
        auto user_id = connect_message.get_header("user_id");
        auto nickname = connect_message.get_payload_as<std::string>();

        {
            std::lock_guard<std::mutex> lock(users_mutex);
            users[user_id] = {user_id, nickname, std::chrono::steady_clock::now()};
        }

        // Broadcast user joined message
        core::message broadcast;
        broadcast.set_type("system.user_joined");
        broadcast.set_payload(nickname + " has joined the chat");
        broadcast.set_priority(core::priority::HIGH);

        broadcastToAll(broadcast);

        m_logger->log(logger_module::log_level::info,
            "User connected: " + nickname + " (" + user_id + ")");
    }

    void handleUserDisconnect(const core::message& disconnect_message) {
        auto user_id = disconnect_message.get_header("user_id");
        std::string nickname;

        {
            std::lock_guard<std::mutex> lock(users_mutex);
            if (auto it = users.find(user_id); it != users.end()) {
                nickname = it->second.nickname;
                users.erase(it);
            }
        }

        if (!nickname.empty()) {
            core::message broadcast;
            broadcast.set_type("system.user_left");
            broadcast.set_payload(nickname + " has left the chat");

            broadcastToAll(broadcast);

            m_logger->log(logger_module::log_level::info,
                "User disconnected: " + nickname);
        }
    }

    void handleChatMessage(const core::message& chat_message) {
        auto sender_user_id = chat_message.get_header("user_id");
        auto target_room_id = chat_message.get_header("room_id");
        auto message_text = chat_message.get_payload_as<std::string>();

        std::string nickname;
        {
            std::lock_guard<std::mutex> lock(users_mutex);
            if (auto user_iter = users.find(sender_user_id); user_iter != users.end()) {
                nickname = user_iter->second.nickname;
                user_iter->second.last_activity = std::chrono::steady_clock::now();
            }
        }

        if (!nickname.empty()) {
            // Create formatted message
            core::message chat_msg;
            chat_msg.set_type("chat.broadcast");
            chat_msg.set_header("sender", nickname);
            chat_msg.set_header("room", target_room_id);
            chat_msg.set_header("timestamp", std::to_string(
                std::chrono::system_clock::now().time_since_epoch().count()
            ));
            chat_msg.set_payload(message_text);

            // Broadcast to room or all users
            if (!target_room_id.empty()) {
                broadcastToRoom(target_room_id, chat_msg);
            } else {
                broadcastToAll(chat_msg);
            }

            // Log message for persistence
            logMessage(nickname, message_text, target_room_id);
        }
    }

    void handlePrivateMessage(const core::message& private_message) {
        auto sender_id = private_message.get_header("sender_id");
        auto recipient_id = private_message.get_header("recipient_id");
        auto message_content = private_message.get_payload_as<std::string>();

        // Find sender and recipient
        std::string sender_name, recipient_name;
        {
            std::lock_guard<std::mutex> lock(users_mutex);
            if (auto it = users.find(sender_id); it != users.end()) {
                sender_name = it->second.nickname;
            }
            if (auto it = users.find(recipient_id); it != users.end()) {
                recipient_name = it->second.nickname;
            }
        }

        if (!sender_name.empty() && !recipient_name.empty()) {
            core::message pm;
            pm.set_type("chat.private_message");
            pm.set_header("from", sender_name);
            pm.set_header("to", recipient_name);
            pm.set_payload(message_content);

            // Send to recipient
            sendToUser(recipient_id, pm);

            // Send confirmation to sender
            core::message confirm;
            confirm.set_type("chat.private_sent");
            confirm.set_header("to", recipient_name);
            confirm.set_payload("Message sent");
            sendToUser(sender_id, confirm);
        }
    }

    void handleRoomJoin(const core::message& join_message) {
        auto joining_user_id = join_message.get_header("user_id");
        auto target_room_id = join_message.get_payload_as<std::string>();

        // Add user to room (implementation would include room management)
        m_logger->log(logger_module::log_level::debug,
            "User " + joining_user_id + " joined room " + target_room_id);

        // Send room history to user
        sendRoomHistory(joining_user_id, target_room_id);
    }

    void handleRoomLeave(const core::message& leave_message) {
        auto leaving_user_id = leave_message.get_header("user_id");
        auto leaving_room_id = leave_message.get_payload_as<std::string>();

        m_logger->log(logger_module::log_level::debug,
            "User " + leaving_user_id + " left room " + leaving_room_id);
    }

    void broadcastToAll(const core::message& broadcast_message) {
        std::lock_guard<std::mutex> lock(users_mutex);
        for (const auto& [user_id, user_data] : users) {
            network_service->send_message(user_id, broadcast_message);
        }
    }

    void broadcastToRoom(const std::string& room_id, const core::message& room_message) {
        // In a real implementation, maintain room membership
        // For now, broadcast to all
        broadcastToAll(room_message);
    }

    void sendToUser(const std::string& recipient_user_id, const core::message& user_message) {
        network_service->send_message(recipient_user_id, user_message);
    }

    void sendRoomHistory(const std::string& user_id, const std::string& room_id) {
        // In a real implementation, retrieve from database
        core::message history;
        history.set_type("room.history");
        history.set_header("room_id", room_id);
        history.set_payload("Welcome to room " + room_id);

        sendToUser(user_id, history);
    }

    void logMessage(const std::string& sender_nickname, const std::string& message_text, const std::string& room_id) {
        // In production, write to database
        std::string log_msg = "[" + (room_id.empty() ? "global" : room_id) + "] " +
                             sender_nickname + ": " + message_text;
        m_logger->log(logger_module::log_level::info, log_msg);
    }

    void cleanupInactiveUsers() {
        auto now = std::chrono::steady_clock::now();
        auto timeout = 5min;

        std::lock_guard<std::mutex> lock(users_mutex);
        for (auto user_iter = users.begin(); user_iter != users.end(); ) {
            if (now - user_iter->second.last_activity > timeout) {
                m_logger->log(logger_module::log_level::warning,
                    "Removing inactive user: " + user_iter->second.nickname);
                user_iter = users.erase(user_iter);
            } else {
                ++user_iter;
            }
        }
    }

    void start(int port = 8080) {
        m_logger->log(logger_module::log_level::info,
            "Chat server starting on port " + std::to_string(port) + "...");

        // Start network service
        network_service->start_server(port);

        // Start cleanup thread
        std::thread cleanup_thread([this]() {
            while (running) {
                std::this_thread::sleep_for(30s);
                cleanupInactiveUsers();
            }
        });

        // Start the message bus
        system_integrator->start();

        m_logger->log(logger_module::log_level::info,
            "Chat server is running. Press Enter to stop...");
        std::cout << "Chat server is running. Press Enter to stop..." << std::endl;
        std::cin.get();

        stop();
        cleanup_thread.join();
    }

    void stop() {
        running = false;
        system_integrator->stop();
        network_service->stop_server();
        m_logger->log(logger_module::log_level::info, "Chat server stopped.");
        m_logger->flush();
        m_logger->stop();
    }

    // Statistics
    void printStats() {
        std::lock_guard<std::mutex> lock(users_mutex);
        m_logger->log(logger_module::log_level::info, "\n=== Server Statistics ===");
        m_logger->log(logger_module::log_level::info,
            "Active users: " + std::to_string(users.size()));
        m_logger->log(logger_module::log_level::info,
            "Message bus stats: " + system_integrator->get_statistics());
        m_logger->log(logger_module::log_level::info, "========================\n");
    }
};

int main(int argc, char* argv[]) {
    try {
        int port = 8080;
        if (argc > 1) {
            port = std::stoi(argv[1]);
        }

        chat_server server;

        // Start statistics thread
        std::thread stats_thread([&server]() {
            while (true) {
                std::this_thread::sleep_for(60s);
                server.printStats();
            }
        });
        stats_thread.detach();

        server.start(port);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        // Note: Logger might not be initialized yet, so we also use std::cerr
        return 1;
    }

    return 0;
}