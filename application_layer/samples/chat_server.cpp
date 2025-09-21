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
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

class ChatServer {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::unique_ptr<services::network_service> network;

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
    ChatServer() {
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

        integrator = std::make_unique<integrations::system_integrator>(config);
        network = std::make_unique<services::network_service>();

        setupMessageHandlers();
    }

    void setupMessageHandlers() {
        auto& bus = integrator->get_message_bus();

        // Handle user connections
        bus.subscribe("user.connect", [this](const core::message& msg) {
            handleUserConnect(msg);
        });

        // Handle user disconnections
        bus.subscribe("user.disconnect", [this](const core::message& msg) {
            handleUserDisconnect(msg);
        });

        // Handle chat messages
        bus.subscribe("chat.message", [this](const core::message& msg) {
            handleChatMessage(msg);
        });

        // Handle private messages
        bus.subscribe("chat.private", [this](const core::message& msg) {
            handlePrivateMessage(msg);
        });

        // Handle room operations
        bus.subscribe("room.join", [this](const core::message& msg) {
            handleRoomJoin(msg);
        });

        bus.subscribe("room.leave", [this](const core::message& msg) {
            handleRoomLeave(msg);
        });
    }

    void handleUserConnect(const core::message& msg) {
        auto user_id = msg.get_header("user_id");
        auto nickname = msg.get_payload_as<std::string>();

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

        std::cout << "User connected: " << nickname << " (" << user_id << ")" << std::endl;
    }

    void handleUserDisconnect(const core::message& msg) {
        auto user_id = msg.get_header("user_id");
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

            std::cout << "User disconnected: " << nickname << std::endl;
        }
    }

    void handleChatMessage(const core::message& msg) {
        auto user_id = msg.get_header("user_id");
        auto room_id = msg.get_header("room_id");
        auto text = msg.get_payload_as<std::string>();

        std::string nickname;
        {
            std::lock_guard<std::mutex> lock(users_mutex);
            if (auto it = users.find(user_id); it != users.end()) {
                nickname = it->second.nickname;
                it->second.last_activity = std::chrono::steady_clock::now();
            }
        }

        if (!nickname.empty()) {
            // Create formatted message
            core::message chat_msg;
            chat_msg.set_type("chat.broadcast");
            chat_msg.set_header("sender", nickname);
            chat_msg.set_header("room", room_id);
            chat_msg.set_header("timestamp", std::to_string(
                std::chrono::system_clock::now().time_since_epoch().count()
            ));
            chat_msg.set_payload(text);

            // Broadcast to room or all users
            if (!room_id.empty()) {
                broadcastToRoom(room_id, chat_msg);
            } else {
                broadcastToAll(chat_msg);
            }

            // Log message for persistence
            logMessage(nickname, text, room_id);
        }
    }

    void handlePrivateMessage(const core::message& msg) {
        auto sender_id = msg.get_header("sender_id");
        auto recipient_id = msg.get_header("recipient_id");
        auto text = msg.get_payload_as<std::string>();

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
            pm.set_payload(text);

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

    void handleRoomJoin(const core::message& msg) {
        auto user_id = msg.get_header("user_id");
        auto room_id = msg.get_payload_as<std::string>();

        // Add user to room (implementation would include room management)
        std::cout << "User " << user_id << " joined room " << room_id << std::endl;

        // Send room history to user
        sendRoomHistory(user_id, room_id);
    }

    void handleRoomLeave(const core::message& msg) {
        auto user_id = msg.get_header("user_id");
        auto room_id = msg.get_payload_as<std::string>();

        std::cout << "User " << user_id << " left room " << room_id << std::endl;
    }

    void broadcastToAll(const core::message& msg) {
        std::lock_guard<std::mutex> lock(users_mutex);
        for (const auto& [id, user] : users) {
            network->send_message(id, msg);
        }
    }

    void broadcastToRoom(const std::string& room_id, const core::message& msg) {
        // In a real implementation, maintain room membership
        // For now, broadcast to all
        broadcastToAll(msg);
    }

    void sendToUser(const std::string& user_id, const core::message& msg) {
        network->send_message(user_id, msg);
    }

    void sendRoomHistory(const std::string& user_id, const std::string& room_id) {
        // In a real implementation, retrieve from database
        core::message history;
        history.set_type("room.history");
        history.set_header("room_id", room_id);
        history.set_payload("Welcome to room " + room_id);

        sendToUser(user_id, history);
    }

    void logMessage(const std::string& user, const std::string& text, const std::string& room) {
        // In production, write to database
        std::cout << "[" << (room.empty() ? "global" : room) << "] "
                  << user << ": " << text << std::endl;
    }

    void cleanupInactiveUsers() {
        auto now = std::chrono::steady_clock::now();
        auto timeout = 5min;

        std::lock_guard<std::mutex> lock(users_mutex);
        for (auto it = users.begin(); it != users.end(); ) {
            if (now - it->second.last_activity > timeout) {
                std::cout << "Removing inactive user: " << it->second.nickname << std::endl;
                it = users.erase(it);
            } else {
                ++it;
            }
        }
    }

    void start(int port = 8080) {
        std::cout << "Chat server starting on port " << port << "..." << std::endl;

        // Start network service
        network->start_server(port);

        // Start cleanup thread
        std::thread cleanup_thread([this]() {
            while (running) {
                std::this_thread::sleep_for(30s);
                cleanupInactiveUsers();
            }
        });

        // Start the message bus
        integrator->start();

        std::cout << "Chat server is running. Press Enter to stop..." << std::endl;
        std::cin.get();

        stop();
        cleanup_thread.join();
    }

    void stop() {
        running = false;
        integrator->stop();
        network->stop_server();
        std::cout << "Chat server stopped." << std::endl;
    }

    // Statistics
    void printStats() const {
        std::lock_guard<std::mutex> lock(users_mutex);
        std::cout << "\n=== Server Statistics ===" << std::endl;
        std::cout << "Active users: " << users.size() << std::endl;
        std::cout << "Message bus stats: " << integrator->get_statistics() << std::endl;
        std::cout << "========================\n" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        int port = 8080;
        if (argc > 1) {
            port = std::stoi(argv[1]);
        }

        ChatServer server;

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
        return 1;
    }

    return 0;
}