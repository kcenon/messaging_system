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
#include <kcenon/logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <csignal>
#include <condition_variable>
#include <queue>
#include <functional>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Global shutdown handling
std::atomic<bool> g_shutdown_requested{false};
std::condition_variable g_shutdown_cv;
std::mutex g_shutdown_mutex;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived signal " << signal << ". Initiating graceful shutdown..." << std::endl;
        g_shutdown_requested = true;
        g_shutdown_cv.notify_all();
    }
}

class chat_server {
private:
    std::unique_ptr<integrations::system_integrator> system_integrator;
    std::unique_ptr<services::network::network_service> network_service;
    std::shared_ptr<logger_module::logger> m_logger;

    // User management
    struct User {
        std::string id;
        std::string nickname;
        std::chrono::steady_clock::time_point last_activity;
        int retry_count = 0;
        bool is_connected = true;
    };

    std::unordered_map<std::string, User> users;
    std::mutex users_mutex;
    std::atomic<bool> running{true};

    // Connection recovery
    std::queue<std::function<void()>> retry_queue;
    std::mutex retry_mutex;
    std::thread retry_thread;

    // Metrics
    struct ServerMetrics {
        std::atomic<uint64_t> messages_processed{0};
        std::atomic<uint64_t> failed_messages{0};
        std::atomic<uint64_t> reconnections{0};
        std::atomic<uint64_t> active_users{0};
    } metrics;

public:
    chat_server() {
        // Initialize logger with async mode and larger buffer size
        m_logger = std::make_shared<logger_module::logger>(true, 16384);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "chat_server.log", 10 * 1024 * 1024, 5)); // 10MB per file, 5 files

        m_logger->log(logger_module::log_level::info, "Initializing chat server with error recovery...");

        // Setup signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        // Initialize the messaging system with optimized configuration
        config::config_builder builder;
        auto config = builder
            .set_environment("production")
            .set_worker_threads(8)  // Handle multiple concurrent users
            .set_queue_size(50000)   // Large queue for message buffering
            .set_container_max_size(4096)
            .enable_external_monitoring(true)
            .build();

        try {
            system_integrator = std::make_unique<integrations::system_integrator>(config);

            if (!system_integrator->initialize()) {
                throw std::runtime_error("Failed to initialize system integrator");
            }

            network_service = std::make_unique<services::network::network_service>();

            setupMessageHandlers();
            startRetryWorker();
            startHealthMonitor();

            m_logger->log(logger_module::log_level::info, "Chat server initialized successfully");
        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::critical,
                         "Failed to initialize chat server: " + std::string(e.what()));
            throw;
        }
    }

    ~chat_server() {
        stop();
    }

    void setupMessageHandlers() {
        auto* message_bus = system_integrator->get_message_bus();

        // Handle user connections
        message_bus->subscribe("user.connect", [this](const core::message& incoming_msg) {
            handleUserConnect(incoming_msg);
        });

        // Handle user disconnections
        message_bus->subscribe("user.disconnect", [this](const core::message& disconnect_msg) {
            handleUserDisconnect(disconnect_msg);
        });

        // Handle chat messages
        message_bus->subscribe("chat.message", [this](const core::message& chat_msg) {
            handleChatMessage(chat_msg);
        });

        // Handle private messages
        message_bus->subscribe("chat.private", [this](const core::message& private_msg) {
            handlePrivateMessage(private_msg);
        });

        // Handle room operations
        message_bus->subscribe("room.join", [this](const core::message& join_msg) {
            handleRoomJoin(join_msg);
        });

        message_bus->subscribe("room.leave", [this](const core::message& leave_msg) {
            handleRoomLeave(leave_msg);
        });
    }

    void handleUserConnect(const core::message& connect_message) {
        try {
            auto it = connect_message.metadata.headers.find("user_id");
            auto user_id = (it != connect_message.metadata.headers.end()) ? it->second : "";
            auto nickname = connect_message.payload.get<std::string>("nickname", "");

            // Check for reconnection
            bool is_reconnection = false;
            {
                std::lock_guard<std::mutex> lock(users_mutex);
                if (auto it = users.find(user_id); it != users.end()) {
                    is_reconnection = true;
                    it->second.is_connected = true;
                    it->second.retry_count = 0;
                    it->second.last_activity = std::chrono::steady_clock::now();
                    metrics.reconnections++;
                } else {
                    users[user_id] = {user_id, nickname, std::chrono::steady_clock::now(), 0, true};
                    metrics.active_users++;
                }
            }

            // Broadcast appropriate message
            core::message broadcast;
            if (is_reconnection) {
                broadcast.payload.topic = "system.user_reconnected";
                broadcast.payload.set("message", nickname + " has reconnected");
                m_logger->log(logger_module::log_level::info,
                    "User reconnected: " + nickname + " (" + user_id + ")");
            } else {
                broadcast.payload.topic = "system.user_joined";
                broadcast.payload.set("message", nickname + " has joined the chat");
                m_logger->log(logger_module::log_level::info,
                    "New user connected: " + nickname + " (" + user_id + ")");
            }
            broadcast.set_priority(core::message_priority::high);

            broadcastToAll(broadcast);

        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error,
                "Error handling user connection: " + std::string(e.what()));
            metrics.failed_messages++;
        }
    }

    void handleUserDisconnect(const core::message& disconnect_message) {
        auto it = disconnect_message.metadata.headers.find("user_id");
        auto user_id = (it != disconnect_message.metadata.headers.end()) ? it->second : "";
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
            broadcast.payload.topic = "system.user_left";
            broadcast.payload.set("message", nickname + " has left the chat");

            broadcastToAll(broadcast);

            m_logger->log(logger_module::log_level::info,
                "User disconnected: " + nickname);
        }
    }

    void handleChatMessage(const core::message& chat_message) {
        try {
            auto it_user = chat_message.metadata.headers.find("user_id");
            auto it_room = chat_message.metadata.headers.find("room_id");
            auto sender_user_id = (it_user != chat_message.metadata.headers.end()) ? it_user->second : "";
            auto target_room_id = (it_room != chat_message.metadata.headers.end()) ? it_room->second : "";
            auto message_text = chat_message.payload.get<std::string>("message", "");

            std::string nickname;
            bool user_connected = false;
            {
                std::lock_guard<std::mutex> lock(users_mutex);
                if (auto user_iter = users.find(sender_user_id); user_iter != users.end()) {
                    nickname = user_iter->second.nickname;
                    user_connected = user_iter->second.is_connected;
                    user_iter->second.last_activity = std::chrono::steady_clock::now();
                }
            }

            if (!nickname.empty() && user_connected) {
                // Create formatted message
                core::message chat_msg;
                chat_msg.payload.topic = "chat.broadcast";
                chat_msg.metadata.headers["sender"] = nickname;
                chat_msg.metadata.headers["room"] = target_room_id;
                chat_msg.metadata.headers["timestamp"] = std::to_string(
                    std::chrono::system_clock::now().time_since_epoch().count()
                );
                chat_msg.payload.set("message", message_text);

                // Try to broadcast with retry on failure
                bool broadcast_success = false;
                for (int attempt = 0; attempt < 3; ++attempt) {
                    try {
                        if (!target_room_id.empty()) {
                            broadcastToRoom(target_room_id, chat_msg);
                        } else {
                            broadcastToAll(chat_msg);
                        }
                        broadcast_success = true;
                        metrics.messages_processed++;
                        break;
                    } catch (const std::exception& e) {
                        m_logger->log(logger_module::log_level::warning,
                            "Broadcast attempt " + std::to_string(attempt + 1) +
                            " failed: " + std::string(e.what()));
                        if (attempt < 2) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (attempt + 1)));
                        }
                    }
                }

                if (!broadcast_success) {
                    // Queue for retry
                    queueForRetry([this, chat_msg]() {
                        broadcastToAll(chat_msg);
                    });
                    metrics.failed_messages++;
                }

                // Log message for persistence
                logMessage(nickname, message_text, target_room_id);
            } else if (!user_connected) {
                m_logger->log(logger_module::log_level::warning,
                    "Message from disconnected user: " + sender_user_id);
            }
        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error,
                "Error handling chat message: " + std::string(e.what()));
            metrics.failed_messages++;
        }
    }

    void handlePrivateMessage(const core::message& private_message) {
        auto it_sender = private_message.metadata.headers.find("sender_id");
        auto it_recipient = private_message.metadata.headers.find("recipient_id");
        auto sender_id = (it_sender != private_message.metadata.headers.end()) ? it_sender->second : "";
        auto recipient_id = (it_recipient != private_message.metadata.headers.end()) ? it_recipient->second : "";
        auto message_content = private_message.payload.get<std::string>("message", "");

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
            pm.payload.topic = "chat.private_message";
            pm.metadata.headers["from"] = sender_name;
            pm.metadata.headers["to"] = recipient_name;
            pm.payload.set("message", message_content);

            // Send to recipient
            sendToUser(recipient_id, pm);

            // Send confirmation to sender
            core::message confirm;
            confirm.payload.topic = "chat.private_sent";
            confirm.metadata.headers["to"] = recipient_name;
            confirm.payload.set("message", std::string("Message sent"));
            sendToUser(sender_id, confirm);
        }
    }

    void handleRoomJoin(const core::message& join_message) {
        auto it = join_message.metadata.headers.find("user_id");
        auto joining_user_id = (it != join_message.metadata.headers.end()) ? it->second : "";
        auto target_room_id = join_message.payload.get<std::string>("room_id", "");

        // Add user to room (implementation would include room management)
        m_logger->log(logger_module::log_level::debug,
            "User " + joining_user_id + " joined room " + target_room_id);

        // Send room history to user
        sendRoomHistory(joining_user_id, target_room_id);
    }

    void handleRoomLeave(const core::message& leave_message) {
        auto it = leave_message.metadata.headers.find("user_id");
        auto leaving_user_id = (it != leave_message.metadata.headers.end()) ? it->second : "";
        auto leaving_room_id = leave_message.payload.get<std::string>("room_id", "");

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
        history.payload.topic = "room.history";
        history.metadata.headers["room_id"] = room_id;
        history.payload.set("message", "Welcome to room " + room_id);

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
                if (user_iter->second.is_connected) {
                    // Mark as disconnected first, will be cleaned up later
                    user_iter->second.is_connected = false;
                    m_logger->log(logger_module::log_level::warning,
                        "Marking user as disconnected: " + user_iter->second.nickname);
                    ++user_iter;
                } else if (user_iter->second.retry_count >= 3) {
                    // Remove after max retries
                    m_logger->log(logger_module::log_level::info,
                        "Removing inactive user after retries: " + user_iter->second.nickname);
                    metrics.active_users--;
                    user_iter = users.erase(user_iter);
                } else {
                    ++user_iter;
                }
            } else {
                ++user_iter;
            }
        }
    }

    // New helper methods for error recovery and resilience
    void startRetryWorker() {
        retry_thread = std::thread([this] {
            while (running && !g_shutdown_requested) {
                std::function<void()> task;
                {
                    std::lock_guard<std::mutex> lock(retry_mutex);
                    if (!retry_queue.empty()) {
                        task = retry_queue.front();
                        retry_queue.pop();
                    }
                }

                if (task) {
                    try {
                        task();
                    } catch (const std::exception& e) {
                        m_logger->log(logger_module::log_level::error,
                            "Retry task failed: " + std::string(e.what()));
                    }
                } else {
                    std::this_thread::sleep_for(100ms);
                }
            }
        });
    }

    void queueForRetry(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(retry_mutex);
        if (retry_queue.size() < 1000) { // Limit retry queue size
            retry_queue.push(task);
        } else {
            m_logger->log(logger_module::log_level::warning,
                "Retry queue full, dropping task");
        }
    }

    void startHealthMonitor() {
        std::thread([this] {
            while (running && !g_shutdown_requested) {
                std::unique_lock<std::mutex> lock(g_shutdown_mutex);
                if (g_shutdown_cv.wait_for(lock, 30s,
                    [] { return g_shutdown_requested.load(); })) {
                    break;
                }

                // Check system health
                try {
                    auto health = system_integrator->check_system_health();
                    if (!health.message_bus_healthy) {
                        m_logger->log(logger_module::log_level::error,
                            "System unhealthy, attempting recovery");
                        attemptRecovery();
                    }

                    // Report metrics
                    reportMetrics();
                } catch (const std::exception& e) {
                    m_logger->log(logger_module::log_level::error,
                        "Health monitor error: " + std::string(e.what()));
                }
            }
        }).detach();
    }

    void attemptRecovery() {
        m_logger->log(logger_module::log_level::info, "Attempting system recovery...");

        try {
            // Reinitialize message handlers
            setupMessageHandlers();

            // Reconnect disconnected users
            std::lock_guard<std::mutex> lock(users_mutex);
            for (auto& [user_id, user] : users) {
                if (!user.is_connected && user.retry_count < 3) {
                    user.retry_count++;
                    queueForRetry([this, user_id] {
                        attemptUserReconnection(user_id);
                    });
                }
            }
        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error,
                "Recovery failed: " + std::string(e.what()));
        }
    }

    void attemptUserReconnection(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(users_mutex);
        if (auto it = users.find(user_id); it != users.end()) {
            m_logger->log(logger_module::log_level::info,
                "Attempting to reconnect user: " + it->second.nickname);
            // In production, would attempt actual network reconnection
            it->second.is_connected = true;
            metrics.reconnections++;
        }
    }

    void reportMetrics() {
        std::stringstream ss;
        ss << "=== Chat Server Metrics ===\n"
           << "Active Users: " << metrics.active_users.load() << "\n"
           << "Messages Processed: " << metrics.messages_processed.load() << "\n"
           << "Failed Messages: " << metrics.failed_messages.load() << "\n"
           << "Reconnections: " << metrics.reconnections.load() << "\n"
           << "==========================";
        m_logger->log(logger_module::log_level::info, ss.str());
    }

    void start(int port = 8080) {
        m_logger->log(logger_module::log_level::info,
            "Chat server starting on port " + std::to_string(port) + "...");

        try {
            // Start network service with retry
            int retry_count = 0;
            while (retry_count < 3) {
                try {
                    // Network service doesn't have server functionality, it's just a messaging service
                    // In a real implementation, you would need a separate TCP/WebSocket server
                    break;
                } catch (const std::exception& e) {
                    retry_count++;
                    if (retry_count >= 3) {
                        throw std::runtime_error("Failed to start network service after 3 attempts: " +
                                                 std::string(e.what()));
                    }
                    m_logger->log(logger_module::log_level::warning,
                        "Network service start attempt " + std::to_string(retry_count) +
                        " failed, retrying...");
                    std::this_thread::sleep_for(std::chrono::seconds(retry_count));
                }
            }

            // Start cleanup thread
            std::thread cleanup_thread([this]() {
                while (running && !g_shutdown_requested) {
                    std::this_thread::sleep_for(30s);
                    if (!g_shutdown_requested) {
                        cleanupInactiveUsers();
                    }
                }
            });

            // The system integrator is automatically started upon initialization

            m_logger->log(logger_module::log_level::info,
                "Chat server is running. Press Ctrl+C to stop...");

            // Wait for shutdown signal
            std::unique_lock<std::mutex> lock(g_shutdown_mutex);
            g_shutdown_cv.wait(lock, [] { return g_shutdown_requested.load(); });

            stop();

            if (cleanup_thread.joinable()) {
                cleanup_thread.join();
            }
        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::critical,
                "Failed to start chat server: " + std::string(e.what()));
            throw;
        }
    }

    void stop() {
        if (!running.exchange(false)) {
            return;  // Already stopped
        }

        m_logger->log(logger_module::log_level::info, "Stopping chat server...");

        // Stop retry worker
        if (retry_thread.joinable()) {
            retry_thread.join();
        }

        // Notify all connected users
        core::message shutdown_msg;
        shutdown_msg.payload.topic = "system.shutdown";
        shutdown_msg.payload.set("message", std::string("Server is shutting down for maintenance"));
        try {
            broadcastToAll(shutdown_msg);
        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::warning,
                "Failed to broadcast shutdown message: " + std::string(e.what()));
        }

        // Stop services
        if (system_integrator) {
            system_integrator->shutdown();
        }
        if (network_service) {
            network_service->shutdown();
        }

        // Final metrics report
        reportMetrics();

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
            "Message bus stats: [messages published: " +
            std::to_string(system_integrator->get_message_bus()->get_statistics().messages_published) + "].");
        m_logger->log(logger_module::log_level::info, "========================\n");
    }
};

int main(int argc, char* argv[]) {
    try {
        int port = 8080;
        if (argc > 1) {
            port = std::stoi(argv[1]);
        }

        std::cout << "Starting enhanced chat server with error recovery..." << std::endl;
        std::cout << "Features: " << std::endl;
        std::cout << " - Automatic reconnection for disconnected users" << std::endl;
        std::cout << " - Message retry on failure" << std::endl;
        std::cout << " - Health monitoring and recovery" << std::endl;
        std::cout << " - Graceful shutdown (Ctrl+C)" << std::endl << std::endl;

        chat_server server;

        // Start statistics thread
        std::atomic<bool> stats_running{true};
        std::thread stats_thread([&server, &stats_running]() {
            while (stats_running && !g_shutdown_requested) {
                std::unique_lock<std::mutex> lock(g_shutdown_mutex);
                if (g_shutdown_cv.wait_for(lock, 60s,
                    [] { return g_shutdown_requested.load(); })) {
                    break;
                }
                if (!g_shutdown_requested) {
                    server.printStats();
                }
            }
        });

        server.start(port);

        stats_running = false;
        if (stats_thread.joinable()) {
            stats_thread.join();
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Chat server shut down successfully." << std::endl;
    return 0;
}