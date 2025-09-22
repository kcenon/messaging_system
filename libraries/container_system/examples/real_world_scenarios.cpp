#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>

#include "container.h"

#ifdef HAS_MESSAGING_FEATURES
#include "integration/messaging_integration.h"
#endif

using namespace container_module;

/**
 * @brief Real-world scenarios demonstrating practical usage of the container system
 *
 * This example shows how the container system can be used in actual applications:
 * 1. IoT Data Collection System
 * 2. Financial Transaction Processing
 * 3. Gaming Event System
 * 4. Monitoring and Alerting System
 * 5. Content Management System
 */

namespace scenarios {

/**
 * @brief IoT Data Collection System
 *
 * Simulates collecting sensor data from multiple IoT devices,
 * aggregating them, and sending to a central processing system.
 */
class IoTDataCollectionSystem {
private:
    struct SensorReading {
        std::string device_id;
        std::string sensor_type;
        double value;
        std::chrono::system_clock::time_point timestamp;
    };

    std::atomic<int> readings_collected_{0};
    std::atomic<int> batches_sent_{0};

public:
    void simulate_iot_scenario() {
        std::cout << "\n=== IoT Data Collection Scenario ===" << std::endl;

        const int num_devices = 10;
        const int readings_per_device = 50;
        const int batch_size = 20;

        std::vector<std::string> device_types = {"temperature", "humidity", "pressure", "light", "motion"};
        std::vector<std::thread> device_threads;

        // Simulated sensor data queue
        std::queue<SensorReading> sensor_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        std::atomic<bool> collection_active{true};

        // Data aggregator thread
        std::thread aggregator_thread([&]() {
            std::vector<SensorReading> batch;
            batch.reserve(batch_size);

            while (collection_active || !sensor_queue.empty()) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [&]() { return !sensor_queue.empty() || !collection_active; });

                // Collect batch
                while (!sensor_queue.empty() && batch.size() < batch_size) {
                    batch.push_back(sensor_queue.front());
                    sensor_queue.pop();
                }

                if (!batch.empty()) {
                    lock.unlock();
                    send_iot_batch(batch);
                    batch.clear();
                    batches_sent_++;
                }
            }
        });

        // Device simulation threads
        std::random_device rd;
        for (int device_id = 0; device_id < num_devices; ++device_id) {
            device_threads.emplace_back([&, device_id]() {
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> temp_dist(18.0, 35.0);
                std::uniform_real_distribution<> humidity_dist(30.0, 80.0);
                std::uniform_real_distribution<> pressure_dist(990.0, 1030.0);
                std::uniform_int_distribution<> light_dist(0, 1000);
                std::uniform_int_distribution<> motion_dist(0, 1);

                for (int reading = 0; reading < readings_per_device; ++reading) {
                    for (const auto& sensor_type : device_types) {
                        SensorReading sensor_reading;
                        sensor_reading.device_id = "device_" + std::to_string(device_id);
                        sensor_reading.sensor_type = sensor_type;
                        sensor_reading.timestamp = std::chrono::system_clock::now();

                        // Generate realistic sensor values
                        if (sensor_type == "temperature") {
                            sensor_reading.value = temp_dist(gen);
                        } else if (sensor_type == "humidity") {
                            sensor_reading.value = humidity_dist(gen);
                        } else if (sensor_type == "pressure") {
                            sensor_reading.value = pressure_dist(gen);
                        } else if (sensor_type == "light") {
                            sensor_reading.value = light_dist(gen);
                        } else if (sensor_type == "motion") {
                            sensor_reading.value = motion_dist(gen);
                        }

                        {
                            std::lock_guard<std::mutex> lock(queue_mutex);
                            sensor_queue.push(sensor_reading);
                        }
                        queue_cv.notify_one();

                        readings_collected_++;

                        // Simulate sensor reading interval
                        std::this_thread::sleep_for(std::chrono::milliseconds(10 + (reading % 20)));
                    }
                }
            });
        }

        // Wait for all devices to finish
        for (auto& thread : device_threads) {
            thread.join();
        }

        collection_active = false;
        queue_cv.notify_all();
        aggregator_thread.join();

        std::cout << "IoT simulation completed:" << std::endl;
        std::cout << "  Readings collected: " << readings_collected_.load() << std::endl;
        std::cout << "  Batches sent: " << batches_sent_.load() << std::endl;
    }

private:
    void send_iot_batch(const std::vector<SensorReading>& batch) {
#ifdef HAS_MESSAGING_FEATURES
        auto container = integration::messaging_container_builder()
            .source("iot_aggregator", "batch_processor")
            .target("iot_analytics_service", "data_processor")
            .message_type("sensor_data_batch")
            .add_value("batch_size", static_cast<int>(batch.size()))
            .add_value("batch_timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())
            .optimize_for_speed()
            .build();
#else
        auto container = std::make_shared<value_container>();
        container->set_source("iot_aggregator", "batch_processor");
        container->set_target("iot_analytics_service", "data_processor");
        container->set_message_type("sensor_data_batch");
        container->add_value(std::make_shared<int_value>("batch_size", static_cast<int>(batch.size())));
        container->add_value(std::make_shared<long_value>("batch_timestamp",
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));
#endif

        // Add individual sensor readings
        for (size_t i = 0; i < batch.size(); ++i) {
            const auto& reading = batch[i];

            // Create nested container for each reading
            auto reading_container = std::make_shared<value_container>();
            reading_container->set_message_type("sensor_reading");
            reading_container->add_value(std::make_shared<string_value>("device_id", reading.device_id));
            reading_container->add_value(std::make_shared<string_value>("sensor_type", reading.sensor_type));
            reading_container->add_value(std::make_shared<double_value>("value", reading.value));
            reading_container->add_value(std::make_shared<long_value>("timestamp",
                std::chrono::duration_cast<std::chrono::milliseconds>(reading.timestamp.time_since_epoch()).count()));

            std::string key = "reading_" + std::to_string(i);
            container->add_value(std::make_shared<container_value>(key, reading_container));
        }

        // Simulate sending to analytics service
        std::string serialized = container->serialize();
        // In real scenario, this would be sent over network
        std::cout << "  Sent IoT batch: " << batch.size() << " readings, "
                  << serialized.size() << " bytes" << std::endl;
    }
};

/**
 * @brief Financial Transaction Processing System
 *
 * Simulates processing financial transactions with fraud detection,
 * compliance checks, and real-time notifications.
 */
class FinancialTransactionSystem {
private:
    struct Transaction {
        std::string transaction_id;
        std::string account_from;
        std::string account_to;
        double amount;
        std::string currency;
        std::string transaction_type;
        std::chrono::system_clock::time_point timestamp;
    };

    std::atomic<int> transactions_processed_{0};
    std::atomic<int> fraud_alerts_{0};
    std::atomic<double> total_amount_{0.0};

public:
    void simulate_financial_scenario() {
        std::cout << "\n=== Financial Transaction Processing Scenario ===" << std::endl;

        const int num_transactions = 1000;
        std::vector<std::thread> processing_threads;

        // Transaction queue
        std::queue<Transaction> transaction_queue;
        std::mutex transaction_mutex;
        std::condition_variable transaction_cv;
        std::atomic<bool> processing_active{true};

        // Fraud detection thread
        std::thread fraud_detector([&]() {
            while (processing_active || !transaction_queue.empty()) {
                std::unique_lock<std::mutex> lock(transaction_mutex);
                transaction_cv.wait(lock, [&]() { return !transaction_queue.empty() || !processing_active; });

                if (!transaction_queue.empty()) {
                    Transaction transaction = transaction_queue.front();
                    transaction_queue.pop();
                    lock.unlock();

                    process_transaction(transaction);
                    transactions_processed_++;
                }
            }
        });

        // Generate transactions
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> amount_dist(10.0, 10000.0);
        std::uniform_int_distribution<> account_dist(1000, 9999);
        std::vector<std::string> currencies = {"USD", "EUR", "GBP", "JPY", "CAD"};
        std::vector<std::string> types = {"transfer", "payment", "withdrawal", "deposit"};

        for (int i = 0; i < num_transactions; ++i) {
            Transaction transaction;
            transaction.transaction_id = "TXN" + std::to_string(1000000 + i);
            transaction.account_from = "ACC" + std::to_string(account_dist(gen));
            transaction.account_to = "ACC" + std::to_string(account_dist(gen));
            transaction.amount = amount_dist(gen);
            transaction.currency = currencies[i % currencies.size()];
            transaction.transaction_type = types[i % types.size()];
            transaction.timestamp = std::chrono::system_clock::now();

            {
                std::lock_guard<std::mutex> lock(transaction_mutex);
                transaction_queue.push(transaction);
            }
            transaction_cv.notify_one();

            // Simulate transaction arrival rate
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + (i % 10)));
        }

        processing_active = false;
        transaction_cv.notify_all();
        fraud_detector.join();

        std::cout << "Financial processing completed:" << std::endl;
        std::cout << "  Transactions processed: " << transactions_processed_.load() << std::endl;
        std::cout << "  Fraud alerts generated: " << fraud_alerts_.load() << std::endl;
        std::cout << "  Total amount processed: $" << std::fixed << std::setprecision(2)
                  << total_amount_.load() << std::endl;
    }

private:
    void process_transaction(const Transaction& transaction) {
        // Fraud detection logic
        bool is_suspicious = (transaction.amount > 5000.0) ||
                           (transaction.account_from == transaction.account_to);

        total_amount_ += transaction.amount;

#ifdef HAS_MESSAGING_FEATURES
        auto container = integration::messaging_container_builder()
            .source("transaction_processor", "fraud_detection")
            .target("compliance_service", "transaction_monitor")
            .message_type(is_suspicious ? "suspicious_transaction" : "normal_transaction")
            .add_value("transaction_id", transaction.transaction_id)
            .add_value("account_from", transaction.account_from)
            .add_value("account_to", transaction.account_to)
            .add_value("amount", transaction.amount)
            .add_value("currency", transaction.currency)
            .add_value("transaction_type", transaction.transaction_type)
            .add_value("timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                transaction.timestamp.time_since_epoch()).count())
            .add_value("risk_score", is_suspicious ? 85.0 : 15.0)
            .optimize_for_speed()
            .build();
#else
        auto container = std::make_shared<value_container>();
        container->set_source("transaction_processor", "fraud_detection");
        container->set_target("compliance_service", "transaction_monitor");
        container->set_message_type(is_suspicious ? "suspicious_transaction" : "normal_transaction");
        container->add_value(std::make_shared<string_value>("transaction_id", transaction.transaction_id));
        container->add_value(std::make_shared<string_value>("account_from", transaction.account_from));
        container->add_value(std::make_shared<string_value>("account_to", transaction.account_to));
        container->add_value(std::make_shared<double_value>("amount", transaction.amount));
        container->add_value(std::make_shared<string_value>("currency", transaction.currency));
        container->add_value(std::make_shared<string_value>("transaction_type", transaction.transaction_type));
        container->add_value(std::make_shared<long_value>("timestamp",
            std::chrono::duration_cast<std::chrono::milliseconds>(transaction.timestamp.time_since_epoch()).count()));
        container->add_value(std::make_shared<double_value>("risk_score", is_suspicious ? 85.0 : 15.0));
#endif

        if (is_suspicious) {
            fraud_alerts_++;

            // Add additional fraud-related data
            container->add_value(std::make_shared<string_value>("alert_reason",
                transaction.amount > 5000.0 ? "high_amount" : "same_account"));
            container->add_value(std::make_shared<bool_value>("requires_manual_review", true));

            std::cout << "  FRAUD ALERT: " << transaction.transaction_id
                      << " Amount: $" << transaction.amount << std::endl;
        }

        // Simulate processing and compliance checking
        std::string serialized = container->serialize();
        // In real scenario, this would trigger compliance workflows
    }
};

/**
 * @brief Gaming Event System
 *
 * Simulates a multiplayer game event system handling player actions,
 * achievements, leaderboards, and real-time updates.
 */
class GamingEventSystem {
private:
    struct GameEvent {
        std::string player_id;
        std::string event_type;
        std::map<std::string, std::string> event_data;
        std::chrono::system_clock::time_point timestamp;
    };

    std::atomic<int> events_processed_{0};
    std::atomic<int> achievements_unlocked_{0};
    std::map<std::string, int> player_scores_;
    std::mutex scores_mutex_;

public:
    void simulate_gaming_scenario() {
        std::cout << "\n=== Gaming Event System Scenario ===" << std::endl;

        const int num_players = 20;
        const int events_per_player = 100;
        std::vector<std::thread> player_threads;

        // Event processing queue
        std::queue<GameEvent> event_queue;
        std::mutex event_mutex;
        std::condition_variable event_cv;
        std::atomic<bool> game_active{true};

        // Event processor thread
        std::thread event_processor([&]() {
            while (game_active || !event_queue.empty()) {
                std::unique_lock<std::mutex> lock(event_mutex);
                event_cv.wait(lock, [&]() { return !event_queue.empty() || !game_active; });

                if (!event_queue.empty()) {
                    GameEvent event = event_queue.front();
                    event_queue.pop();
                    lock.unlock();

                    process_game_event(event);
                    events_processed_++;
                }
            }
        });

        // Player simulation threads
        std::random_device rd;
        for (int player_id = 0; player_id < num_players; ++player_id) {
            player_threads.emplace_back([&, player_id]() {
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> action_dist(0, 4);
                std::uniform_int_distribution<> score_dist(10, 500);
                std::uniform_int_distribution<> level_dist(1, 50);

                std::vector<std::string> actions = {"kill", "death", "level_up", "item_collected", "quest_completed"};

                for (int event_count = 0; event_count < events_per_player; ++event_count) {
                    GameEvent event;
                    event.player_id = "player_" + std::to_string(player_id);
                    event.event_type = actions[action_dist(gen)];
                    event.timestamp = std::chrono::system_clock::now();

                    // Add event-specific data
                    if (event.event_type == "kill") {
                        event.event_data["target"] = "player_" + std::to_string((player_id + 1) % num_players);
                        event.event_data["weapon"] = "rifle";
                        event.event_data["score"] = std::to_string(score_dist(gen));
                    } else if (event.event_type == "level_up") {
                        event.event_data["new_level"] = std::to_string(level_dist(gen));
                        event.event_data["experience_gained"] = std::to_string(score_dist(gen) * 10);
                    } else if (event.event_type == "item_collected") {
                        event.event_data["item_type"] = "health_potion";
                        event.event_data["rarity"] = "rare";
                    }

                    {
                        std::lock_guard<std::mutex> lock(event_mutex);
                        event_queue.push(event);
                    }
                    event_cv.notify_one();

                    // Simulate player action rate
                    std::this_thread::sleep_for(std::chrono::milliseconds(50 + (event_count % 100)));
                }
            });
        }

        // Wait for all players to finish
        for (auto& thread : player_threads) {
            thread.join();
        }

        game_active = false;
        event_cv.notify_all();
        event_processor.join();

        // Print leaderboard
        print_leaderboard();

        std::cout << "Gaming simulation completed:" << std::endl;
        std::cout << "  Events processed: " << events_processed_.load() << std::endl;
        std::cout << "  Achievements unlocked: " << achievements_unlocked_.load() << std::endl;
    }

private:
    void process_game_event(const GameEvent& event) {
#ifdef HAS_MESSAGING_FEATURES
        auto container = integration::messaging_container_builder()
            .source("game_client", event.player_id)
            .target("game_server", "event_processor")
            .message_type("game_event")
            .add_value("player_id", event.player_id)
            .add_value("event_type", event.event_type)
            .add_value("timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                event.timestamp.time_since_epoch()).count())
            .optimize_for_speed()
            .build();
#else
        auto container = std::make_shared<value_container>();
        container->set_source("game_client", event.player_id);
        container->set_target("game_server", "event_processor");
        container->set_message_type("game_event");
        container->add_value(std::make_shared<string_value>("player_id", event.player_id));
        container->add_value(std::make_shared<string_value>("event_type", event.event_type));
        container->add_value(std::make_shared<long_value>("timestamp",
            std::chrono::duration_cast<std::chrono::milliseconds>(event.timestamp.time_since_epoch()).count()));
#endif

        // Add event-specific data
        for (const auto& data_pair : event.event_data) {
            container->add_value(std::make_shared<string_value>(data_pair.first, data_pair.second));
        }

        // Update player scores
        if (event.event_data.find("score") != event.event_data.end()) {
            int score = std::stoi(event.event_data.at("score"));
            {
                std::lock_guard<std::mutex> lock(scores_mutex_);
                player_scores_[event.player_id] += score;
            }
        }

        // Check for achievements
        if (event.event_type == "level_up" && event.event_data.find("new_level") != event.event_data.end()) {
            int level = std::stoi(event.event_data.at("new_level"));
            if (level >= 25) {
                achievements_unlocked_++;
                send_achievement_notification(event.player_id, "High Level Achiever");
            }
        }

        // Simulate event processing
        std::string serialized = container->serialize();
    }

    void send_achievement_notification(const std::string& player_id, const std::string& achievement) {
#ifdef HAS_MESSAGING_FEATURES
        auto notification = integration::messaging_container_builder()
            .source("achievement_system", "unlock_processor")
            .target("notification_service", "player_notifier")
            .message_type("achievement_unlocked")
            .add_value("player_id", player_id)
            .add_value("achievement_name", achievement)
            .add_value("timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())
            .build();
#else
        auto notification = std::make_shared<value_container>();
        notification->set_source("achievement_system", "unlock_processor");
        notification->set_target("notification_service", "player_notifier");
        notification->set_message_type("achievement_unlocked");
        notification->add_value(std::make_shared<string_value>("player_id", player_id));
        notification->add_value(std::make_shared<string_value>("achievement_name", achievement));
        notification->add_value(std::make_shared<long_value>("timestamp",
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));
#endif

        std::cout << "  ACHIEVEMENT: " << player_id << " unlocked '" << achievement << "'" << std::endl;
    }

    void print_leaderboard() {
        std::cout << "\n  === Leaderboard ===" << std::endl;

        std::vector<std::pair<std::string, int>> leaderboard;
        {
            std::lock_guard<std::mutex> lock(scores_mutex_);
            for (const auto& entry : player_scores_) {
                leaderboard.emplace_back(entry.first, entry.second);
            }
        }

        std::sort(leaderboard.begin(), leaderboard.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        for (size_t i = 0; i < std::min(size_t(5), leaderboard.size()); ++i) {
            std::cout << "  " << (i + 1) << ". " << leaderboard[i].first
                      << ": " << leaderboard[i].second << " points" << std::endl;
        }
        std::cout << "  ===================" << std::endl;
    }
};

/**
 * @brief Content Management System
 *
 * Simulates a content management system handling document uploads,
 * processing, indexing, and search operations.
 */
class ContentManagementSystem {
private:
    struct Document {
        std::string document_id;
        std::string title;
        std::string content;
        std::string author;
        std::string category;
        std::vector<std::string> tags;
        std::chrono::system_clock::time_point upload_time;
    };

    std::atomic<int> documents_processed_{0};
    std::atomic<int> documents_indexed_{0};

public:
    void simulate_cms_scenario() {
        std::cout << "\n=== Content Management System Scenario ===" << std::endl;

        const int num_documents = 200;
        std::vector<std::thread> upload_threads;

        // Document processing queue
        std::queue<Document> document_queue;
        std::mutex document_mutex;
        std::condition_variable document_cv;
        std::atomic<bool> uploading_active{true};

        // Document processor thread
        std::thread document_processor([&]() {
            while (uploading_active || !document_queue.empty()) {
                std::unique_lock<std::mutex> lock(document_mutex);
                document_cv.wait(lock, [&]() { return !document_queue.empty() || !uploading_active; });

                if (!document_queue.empty()) {
                    Document document = document_queue.front();
                    document_queue.pop();
                    lock.unlock();

                    process_document(document);
                    documents_processed_++;
                }
            }
        });

        // Document upload simulation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::vector<std::string> categories = {"article", "report", "manual", "tutorial", "reference"};
        std::vector<std::string> authors = {"john_smith", "jane_doe", "bob_wilson", "alice_johnson", "mike_brown"};
        std::vector<std::vector<std::string>> tag_sets = {
            {"programming", "cpp", "tutorial"},
            {"business", "report", "analysis"},
            {"technical", "manual", "guide"},
            {"science", "research", "data"},
            {"marketing", "strategy", "planning"}
        };

        for (int i = 0; i < num_documents; ++i) {
            Document document;
            document.document_id = "DOC" + std::to_string(10000 + i);
            document.title = "Document Title " + std::to_string(i);
            document.content = generate_sample_content(i);
            document.author = authors[i % authors.size()];
            document.category = categories[i % categories.size()];
            document.tags = tag_sets[i % tag_sets.size()];
            document.upload_time = std::chrono::system_clock::now();

            {
                std::lock_guard<std::mutex> lock(document_mutex);
                document_queue.push(document);
            }
            document_cv.notify_one();

            // Simulate upload rate
            std::this_thread::sleep_for(std::chrono::milliseconds(20 + (i % 30)));
        }

        uploading_active = false;
        document_cv.notify_all();
        document_processor.join();

        std::cout << "CMS simulation completed:" << std::endl;
        std::cout << "  Documents processed: " << documents_processed_.load() << std::endl;
        std::cout << "  Documents indexed: " << documents_indexed_.load() << std::endl;
    }

private:
    std::string generate_sample_content(int index) {
        std::stringstream content;
        content << "This is sample content for document " << index << ". ";
        content << "It contains important information about various topics including ";
        content << "technology, business processes, and technical documentation. ";
        content << "The content is generated for demonstration purposes and shows ";
        content << "how the container system handles different types of text data. ";
        content << "Document creation timestamp: " << std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return content.str();
    }

    void process_document(const Document& document) {
        // Document processing and indexing
#ifdef HAS_MESSAGING_FEATURES
        auto container = integration::messaging_container_builder()
            .source("cms_upload_service", "document_processor")
            .target("search_indexer", "text_analyzer")
            .message_type("document_processing")
            .add_value("document_id", document.document_id)
            .add_value("title", document.title)
            .add_value("author", document.author)
            .add_value("category", document.category)
            .add_value("content_length", static_cast<int>(document.content.length()))
            .add_value("upload_timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                document.upload_time.time_since_epoch()).count())
            .add_value("tag_count", static_cast<int>(document.tags.size()))
            .optimize_for_memory()
            .build();
#else
        auto container = std::make_shared<value_container>();
        container->set_source("cms_upload_service", "document_processor");
        container->set_target("search_indexer", "text_analyzer");
        container->set_message_type("document_processing");
        container->add_value(std::make_shared<string_value>("document_id", document.document_id));
        container->add_value(std::make_shared<string_value>("title", document.title));
        container->add_value(std::make_shared<string_value>("author", document.author));
        container->add_value(std::make_shared<string_value>("category", document.category));
        container->add_value(std::make_shared<int_value>("content_length", static_cast<int>(document.content.length())));
        container->add_value(std::make_shared<long_value>("upload_timestamp",
            std::chrono::duration_cast<std::chrono::milliseconds>(document.upload_time.time_since_epoch()).count()));
        container->add_value(std::make_shared<int_value>("tag_count", static_cast<int>(document.tags.size())));
#endif

        // Add content (potentially large)
        container->add_value(std::make_shared<string_value>("content", document.content));

        // Add tags as separate values
        for (size_t i = 0; i < document.tags.size(); ++i) {
            std::string tag_key = "tag_" + std::to_string(i);
            container->add_value(std::make_shared<string_value>(tag_key, document.tags[i]));
        }

        // Simulate text analysis and indexing
        std::string serialized = container->serialize();

        // Create search index entry
        create_search_index_entry(document);

        documents_indexed_++;

        if (documents_processed_ % 50 == 0) {
            std::cout << "  Processed " << documents_processed_.load() << " documents..." << std::endl;
        }
    }

    void create_search_index_entry(const Document& document) {
#ifdef HAS_MESSAGING_FEATURES
        auto index_container = integration::messaging_container_builder()
            .source("text_analyzer", "indexing_service")
            .target("search_service", "index_updater")
            .message_type("search_index_update")
            .add_value("document_id", document.document_id)
            .add_value("indexed_title", document.title)
            .add_value("indexed_category", document.category)
            .add_value("word_count", static_cast<int>(count_words(document.content)))
            .add_value("index_timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())
            .build();
#else
        auto index_container = std::make_shared<value_container>();
        index_container->set_source("text_analyzer", "indexing_service");
        index_container->set_target("search_service", "index_updater");
        index_container->set_message_type("search_index_update");
        index_container->add_value(std::make_shared<string_value>("document_id", document.document_id));
        index_container->add_value(std::make_shared<string_value>("indexed_title", document.title));
        index_container->add_value(std::make_shared<string_value>("indexed_category", document.category));
        index_container->add_value(std::make_shared<int_value>("word_count", static_cast<int>(count_words(document.content))));
        index_container->add_value(std::make_shared<long_value>("index_timestamp",
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));
#endif

        // Simulate search index update
        std::string index_serialized = index_container->serialize();
    }

    size_t count_words(const std::string& text) {
        std::istringstream iss(text);
        std::string word;
        size_t count = 0;
        while (iss >> word) {
            count++;
        }
        return count;
    }
};

} // namespace scenarios

int main() {
    try {
        std::cout << "=== Real-World Container System Scenarios ===" << std::endl;
        std::cout << "Demonstrating practical applications of the container system" << std::endl;

        // Run all scenarios
        scenarios::IoTDataCollectionSystem iot_system;
        iot_system.simulate_iot_scenario();

        scenarios::FinancialTransactionSystem financial_system;
        financial_system.simulate_financial_scenario();

        scenarios::GamingEventSystem gaming_system;
        gaming_system.simulate_gaming_scenario();

        scenarios::ContentManagementSystem cms_system;
        cms_system.simulate_cms_scenario();

        std::cout << "\n=== All Real-World Scenarios Completed Successfully ===" << std::endl;
        std::cout << "The container system demonstrated versatility across:" << std::endl;
        std::cout << "• IoT data aggregation and processing" << std::endl;
        std::cout << "• Financial transaction processing with fraud detection" << std::endl;
        std::cout << "• Gaming event systems with real-time processing" << std::endl;
        std::cout << "• Content management with search indexing" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error in real-world scenarios: " << e.what() << std::endl;
        return 1;
    }
}