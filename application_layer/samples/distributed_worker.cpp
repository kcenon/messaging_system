/**
 * @file distributed_worker.cpp
 * @brief Distributed task processing system using messaging
 *
 * This example shows how to build a distributed worker system
 * that processes tasks across multiple nodes using the application layer.
 */

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/services/container/container_service.h>
#include <kcenon/messaging/services/database/database_service.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <random>
#include <functional>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Task types for the distributed system
enum class task_type {
    DATA_PROCESSING,
    IMAGE_ANALYSIS,
    REPORT_GENERATION,
    EMAIL_SENDING,
    CACHE_WARMING
};

// Task structure
struct task {
    std::string id;
    task_type type;
    std::string payload;
    int priority;
    std::chrono::steady_clock::time_point created_at;
    int retry_count = 0;

    // Serialization support
    std::string serialize() const {
        // In production, use proper serialization
        return id + "|" + std::to_string(static_cast<int>(type)) + "|" + payload;
    }

    static task deserialize(const std::string& serialized_data) {
        // Simple deserialization for demo
        task deserialized_task;
        deserialized_task.id = serialized_data.substr(0, serialized_data.find('|'));
        deserialized_task.type = task_type::DATA_PROCESSING;
        deserialized_task.payload = serialized_data;
        deserialized_task.created_at = std::chrono::steady_clock::now();
        return deserialized_task;
    }
};

class distributed_worker {
private:
    std::unique_ptr<integrations::system_integrator> system_integrator;
    std::unique_ptr<services::container::container_service> container_service;
    std::unique_ptr<services::database::database_service> database_service;
    std::shared_ptr<logger_module::logger> m_logger;

    std::string worker_id;
    std::atomic<bool> running{true};
    std::atomic<int> tasks_processed{0};
    std::atomic<int> tasks_failed{0};

    // Task processors
    std::unordered_map<task_type, std::function<bool(const task&)>> processors;

    // Performance metrics
    struct metrics {
        std::atomic<double> avg_processing_time{0};
        std::atomic<int> total_tasks{0};
        std::chrono::steady_clock::time_point start_time;
    } m_metrics;

public:
    distributed_worker(const std::string& id = "")
        : worker_id(id.empty() ? generateWorkerId() : id) {

        // Initialize logger
        m_logger = std::make_shared<logger_module::logger>(true, 8192);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "distributed_worker_" + worker_id + ".log", 10 * 1024 * 1024, 5));

        m_logger->log(logger_module::log_level::info, "Initializing distributed worker: " + worker_id);

        // Configure for distributed processing
        config::config_builder builder;
        auto config = builder
            .set_environment("distributed")
            .set_worker_threads(std::thread::hardware_concurrency())
            .set_queue_size(100000)
            .set_container_max_size(10 * 1024 * 1024)  // 10MB for large tasks
            .enable_compression(true)
            .enable_external_monitoring(true)
            .build();

        system_integrator = std::make_unique<integrations::system_integrator>(config);
        container_service = std::make_unique<services::container::container_service>();
        database_service = std::make_unique<services::database::database_service>();

        setupProcessors();
        setupMessageHandlers();

        m_metrics.start_time = std::chrono::steady_clock::now();
    }

    void setupProcessors() {
        // Register task processors for different task types
        processors[task_type::DATA_PROCESSING] = [this](const task& task_to_process) {
            return processData(task_to_process);
        };

        processors[task_type::IMAGE_ANALYSIS] = [this](const task& image_task) {
            return analyzeImage(image_task);
        };

        processors[task_type::REPORT_GENERATION] = [this](const task& report_task) {
            return generateReport(report_task);
        };

        processors[task_type::EMAIL_SENDING] = [this](const task& email_task) {
            return sendEmail(email_task);
        };

        processors[task_type::CACHE_WARMING] = [this](const task& cache_task) {
            return warmCache(cache_task);
        };
    }

    void setupMessageHandlers() {
        auto* message_bus = system_integrator->get_message_bus();

        // Handle incoming tasks
        message_bus->subscribe("task.new", [this](const core::message& task_message) {
            handleNewTask(task_message);
        });

        // Handle task cancellation
        message_bus->subscribe("task.cancel", [this](const core::message& cancel_message) {
            handleTaskCancel(cancel_message);
        });

        // Handle cluster coordination
        message_bus->subscribe("cluster.rebalance", [this](const core::message& rebalance_message) {
            handleRebalance(rebalance_message);
        });

        // Health check
        message_bus->subscribe("health.check", [this](const core::message& health_message) {
            respondHealthCheck(health_message);
        });
    }

    void handleNewTask(const core::message& task_message) {
        try {
            // Deserialize task - get task data from message payload
            std::string task_data = task_message.payload.get<std::string>("task_data", "");
            task received_task = task::deserialize(task_data);

            // Log task reception
            logTaskReceived(received_task);

            // Process task
            auto processing_start_time = std::chrono::steady_clock::now();
            bool task_succeeded = processTask(received_task);
            auto processing_end_time = std::chrono::steady_clock::now();

            // Update metrics
            updateMetrics(processing_start_time, processing_end_time);

            if (task_succeeded) {
                tasks_processed++;
                sendTaskComplete(received_task);
            } else {
                tasks_failed++;
                if (received_task.retry_count < 3) {
                    retryTask(received_task);
                } else {
                    sendTaskFailed(received_task);
                }
            }

        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error, "Error handling task: " + std::string(e.what()));
            tasks_failed++;
        }
    }

    bool processTask(const task& t) {
        m_logger->log(logger_module::log_level::info,
            "Worker " + worker_id + " processing task " + t.id +
            " of type " + std::to_string(static_cast<int>(t.type)));

        // Find appropriate processor
        if (auto it = processors.find(t.type); it != processors.end()) {
            return it->second(t);
        }

        m_logger->log(logger_module::log_level::error, "No processor found for task type");
        return false;
    }

    // Task processors implementation
    bool processData(const task& t) {
        // Simulate data processing
        m_logger->log(logger_module::log_level::debug,
            "Processing data: " + t.payload.substr(0, 50) + "...");

        // Note: In production, would use actual container and database services
        // The container service doesn't have create_container method,
        // and database service doesn't have store method yet.

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 900));

        return true;
    }

    bool analyzeImage(const task& t) {
        m_logger->log(logger_module::log_level::debug, "Analyzing image: " + t.id);

        // Simulate image analysis with random success rate
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 1500));

        // 90% success rate
        return (rand() % 10) < 9;
    }

    bool generateReport(const task& t) {
        m_logger->log(logger_module::log_level::debug, "Generating report: " + t.id);

        // In production, would fetch data from database
        // Note: database_service doesn't have fetch/store methods yet

        // Generate report (simulated)
        std::this_thread::sleep_for(std::chrono::milliseconds(200 + rand() % 800));

        return true;
    }

    bool sendEmail(const task& t) {
        m_logger->log(logger_module::log_level::debug, "Sending email for task: " + t.id);

        // Simulate email sending
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 200));

        return true;
    }

    bool warmCache(const task& t) {
        m_logger->log(logger_module::log_level::debug, "Warming cache: " + t.payload);

        // In production, would fetch and cache data
        // Note: Services don't have fetch_batch/cache methods yet
        // for (const auto& item : data) {
        //     container_service->cache(item.key, item.value);
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return true;
    }

    void handleTaskCancel(const core::message& cancel_message) {
        auto task_id_to_cancel = cancel_message.payload.get<std::string>("task_id", "");
        m_logger->log(logger_module::log_level::info, "Cancelling task: " + task_id_to_cancel);
        // In production, would need to track and cancel running tasks
    }

    void handleRebalance(const core::message& msg) {
        m_logger->log(logger_module::log_level::info, "Rebalancing work distribution...");
        // Implement load balancing logic
    }

    void respondHealthCheck(const core::message& msg) {
        core::message response("health.response");
        response.metadata.headers["worker_id"] = worker_id;
        response.metadata.headers["status"] = "healthy";
        response.metadata.headers["tasks_processed"] = std::to_string(tasks_processed.load());
        response.metadata.headers["tasks_failed"] = std::to_string(tasks_failed.load());
        response.metadata.headers["uptime"] = std::to_string(getUptime());

        system_integrator->get_message_bus()->publish(response);
    }

    void retryTask(task t) {
        t.retry_count++;

        core::message retry_msg("task.retry");
        retry_msg.payload.set("task_data", t.serialize());
        retry_msg.set_priority(core::message_priority::low);  // Lower priority for retries
        retry_msg.metadata.headers["retry_count"] = std::to_string(t.retry_count);

        // In production, would schedule retry with exponential backoff
        // auto delay = std::chrono::seconds(std::pow(2, t.retry_count));
        system_integrator->get_message_bus()->publish(retry_msg);
    }

    void sendTaskComplete(const task& completed_task) {
        core::message completion_message("task.complete");
        completion_message.metadata.headers["task_id"] = completed_task.id;
        completion_message.metadata.headers["worker_id"] = worker_id;
        completion_message.metadata.headers["processing_time"] = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count() -
            completed_task.created_at.time_since_epoch().count()
        );

        system_integrator->get_message_bus()->publish(completion_message);
    }

    void sendTaskFailed(const task& failed_task) {
        core::message failure_message("task.failed");
        failure_message.metadata.headers["task_id"] = failed_task.id;
        failure_message.metadata.headers["worker_id"] = worker_id;
        failure_message.metadata.headers["reason"] = "Max retries exceeded";

        system_integrator->get_message_bus()->publish(failure_message);
    }

    void logTaskReceived(const task& received_task) {
        m_logger->log(logger_module::log_level::info,
            "[" + worker_id + "] Received task " + received_task.id +
            " with priority " + std::to_string(received_task.priority));
    }

    void updateMetrics(
        const std::chrono::steady_clock::time_point& start,
        const std::chrono::steady_clock::time_point& end
    ) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        int total_task_count = m_metrics.total_tasks.fetch_add(1) + 1;
        double current_average = m_metrics.avg_processing_time.load();
        double new_average = (current_average * (total_task_count - 1) + duration.count()) / total_task_count;

        m_metrics.avg_processing_time.store(new_average);
    }

    int getUptime() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(
            now - m_metrics.start_time
        ).count();
    }

    std::string generateWorkerId() const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        return "worker-" + std::to_string(dis(gen));
    }

    void start() {
        m_logger->log(logger_module::log_level::info, "Starting distributed worker: " + worker_id);

        // Start the integrator (if it has a start method)
        // system_integrator->start();

        // Announce worker availability
        core::message availability_announcement("worker.online");
        availability_announcement.metadata.headers["worker_id"] = worker_id;
        availability_announcement.metadata.headers["capabilities"] = "all";  // In production, list specific capabilities

        system_integrator->get_message_bus()->publish(availability_announcement);

        // Run until stopped
        while (running) {
            std::this_thread::sleep_for(1s);

            // Periodic status update
            if (tasks_processed % 10 == 0 && tasks_processed > 0) {
                printStatus();
            }
        }

        // Announce worker going offline
        core::message offline_announcement("worker.offline");
        offline_announcement.metadata.headers["worker_id"] = worker_id;

        system_integrator->get_message_bus()->publish(offline_announcement);
        // Stop the integrator (if it has a stop method)
        // system_integrator->stop();
    }

    void stop() {
        running = false;
        if (m_logger) {
            m_logger->log(logger_module::log_level::info, "Stopping distributed worker: " + worker_id);
            m_logger->flush();
            m_logger->stop();
        }
    }

    void printStatus() const {
        std::string status = "\n=== Worker Status ===\n";
        status += "Worker ID: " + worker_id + "\n";
        status += "Tasks processed: " + std::to_string(tasks_processed) + "\n";
        status += "Tasks failed: " + std::to_string(tasks_failed) + "\n";
        double success_rate = (tasks_processed + tasks_failed) > 0 ?
            (100.0 * tasks_processed / (tasks_processed + tasks_failed)) : 0.0;
        status += "Success rate: " + std::to_string(success_rate) + "%\n";
        status += "Avg processing time: " + std::to_string(m_metrics.avg_processing_time.load()) + " ms\n";
        status += "Uptime: " + std::to_string(getUptime()) + " seconds\n";
        status += "===================";

        if (m_logger) {
            m_logger->log(logger_module::log_level::info, status);
        }
    }
};

// Task generator for testing
class task_generator {
private:
    std::unique_ptr<integrations::system_integrator> system_integrator;
    std::shared_ptr<logger_module::logger> m_logger;
    std::atomic<int> generated_task_counter{0};
    std::random_device random_device;
    std::mt19937 random_generator;

public:
    task_generator() : random_generator(random_device()) {
        // Initialize logger
        m_logger = std::make_shared<logger_module::logger>(true, 8192);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "task_generator.log", 10 * 1024 * 1024, 5));

        m_logger->log(logger_module::log_level::info, "Initializing task generator");

        config::config_builder builder;
        auto generator_config = builder
            .set_environment("generator")
            .set_worker_threads(2)
            .build();

        system_integrator = std::make_unique<integrations::system_integrator>(generator_config);
        // Start the integrator (if it has a start method)
        // system_integrator->start();
    }

    void generateTasks(int task_count, int delay_between_tasks_ms = 1000) {
        std::uniform_int_distribution<> type_distribution(0, 4);
        std::uniform_int_distribution<> priority_distribution(1, 10);

        for (int task_index = 0; task_index < task_count; ++task_index) {
            task generated_task;
            generated_task.id = "task-" + std::to_string(generated_task_counter++);
            generated_task.type = static_cast<task_type>(type_distribution(random_generator));
            generated_task.payload = "Sample data for task " + generated_task.id;
            generated_task.priority = priority_distribution(random_generator);
            generated_task.created_at = std::chrono::steady_clock::now();

            core::message task_message("task.new");
            task_message.payload.set("task_data", generated_task.serialize());
            // Set priority based on task priority level
            if (generated_task.priority >= 8) {
                task_message.set_priority(core::message_priority::critical);
            } else if (generated_task.priority >= 5) {
                task_message.set_priority(core::message_priority::high);
            } else if (generated_task.priority >= 3) {
                task_message.set_priority(core::message_priority::normal);
            } else {
                task_message.set_priority(core::message_priority::low);
            }

            system_integrator->get_message_bus()->publish(task_message);

            m_logger->log(logger_module::log_level::info, "Generated task: " + generated_task.id);

            if (delay_between_tasks_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_tasks_ms));
            }
        }
    }

    ~task_generator() {
        // Stop the integrator (if it has a stop method)
        // system_integrator->stop();
        if (m_logger) {
            m_logger->log(logger_module::log_level::info, "Shutting down task generator");
            m_logger->flush();
            m_logger->stop();
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        bool is_generator = false;
        int worker_count = 1;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--generator") {
                is_generator = true;
            } else if (arg == "--workers" && i + 1 < argc) {
                worker_count = std::stoi(argv[++i]);
            }
        }

        if (is_generator) {
            // Run as task generator
            // Initialize logger for main
            auto main_logger = std::make_shared<logger_module::logger>(true, 8192);
            main_logger->add_writer(std::make_unique<logger_module::console_writer>());

            main_logger->log(logger_module::log_level::info, "Running as task generator");
            task_generator generator;

            // Generate tasks continuously
            while (true) {
                generator.generateTasks(10, 2000);  // 10 tasks every 2 seconds
                std::this_thread::sleep_for(10s);
            }

        } else {
            // Run as worker(s)
            std::vector<std::unique_ptr<distributed_worker>> worker_instances;
            std::vector<std::thread> worker_threads;

            for (int worker_index = 0; worker_index < worker_count; ++worker_index) {
                worker_instances.push_back(std::make_unique<distributed_worker>());
            }

            for (auto& worker_instance : worker_instances) {
                worker_threads.emplace_back([&worker_instance]() {
                    worker_instance->start();
                });
            }

            std::cout << "Started " << worker_count << " workers. Press Enter to stop..." << std::endl;
            std::cin.get();

            for (auto& worker_instance : worker_instances) {
                worker_instance->stop();
            }

            for (auto& worker_thread : worker_threads) {
                worker_thread.join();
            }
        }

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