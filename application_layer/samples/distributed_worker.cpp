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
#include <iostream>
#include <thread>
#include <random>
#include <functional>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Task types for the distributed system
enum class TaskType {
    DATA_PROCESSING,
    IMAGE_ANALYSIS,
    REPORT_GENERATION,
    EMAIL_SENDING,
    CACHE_WARMING
};

// Task structure
struct Task {
    std::string id;
    TaskType type;
    std::string payload;
    int priority;
    std::chrono::steady_clock::time_point created_at;
    int retry_count = 0;

    // Serialization support
    std::string serialize() const {
        // In production, use proper serialization
        return id + "|" + std::to_string(static_cast<int>(type)) + "|" + payload;
    }

    static Task deserialize(const std::string& data) {
        // Simple deserialization for demo
        Task t;
        t.id = data.substr(0, data.find('|'));
        t.type = TaskType::DATA_PROCESSING;
        t.payload = data;
        t.created_at = std::chrono::steady_clock::now();
        return t;
    }
};

class DistributedWorker {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::unique_ptr<services::container_service> container_svc;
    std::unique_ptr<services::database_service> database_svc;

    std::string worker_id;
    std::atomic<bool> running{true};
    std::atomic<int> tasks_processed{0};
    std::atomic<int> tasks_failed{0};

    // Task processors
    std::unordered_map<TaskType, std::function<bool(const Task&)>> processors;

    // Performance metrics
    struct Metrics {
        std::atomic<double> avg_processing_time{0};
        std::atomic<int> total_tasks{0};
        std::chrono::steady_clock::time_point start_time;
    } metrics;

public:
    DistributedWorker(const std::string& id = "")
        : worker_id(id.empty() ? generateWorkerId() : id) {

        // Configure for distributed processing
        config::config_builder builder;
        auto config = builder
            .set_environment("distributed")
            .set_worker_threads(std::thread::hardware_concurrency())
            .set_queue_size(100000)
            .set_max_message_size(10 * 1024 * 1024)  // 10MB for large tasks
            .enable_persistence(true)
            .enable_monitoring(true)
            .set_timeout(30000)  // 30 second timeout
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        container_svc = std::make_unique<services::container_service>();
        database_svc = std::make_unique<services::database_service>();

        setupProcessors();
        setupMessageHandlers();

        metrics.start_time = std::chrono::steady_clock::now();
    }

    void setupProcessors() {
        // Register task processors for different task types
        processors[TaskType::DATA_PROCESSING] = [this](const Task& task) {
            return processData(task);
        };

        processors[TaskType::IMAGE_ANALYSIS] = [this](const Task& task) {
            return analyzeImage(task);
        };

        processors[TaskType::REPORT_GENERATION] = [this](const Task& task) {
            return generateReport(task);
        };

        processors[TaskType::EMAIL_SENDING] = [this](const Task& task) {
            return sendEmail(task);
        };

        processors[TaskType::CACHE_WARMING] = [this](const Task& task) {
            return warmCache(task);
        };
    }

    void setupMessageHandlers() {
        auto& bus = integrator->get_message_bus();

        // Handle incoming tasks
        bus.subscribe("task.new", [this](const core::message& msg) {
            handleNewTask(msg);
        });

        // Handle task cancellation
        bus.subscribe("task.cancel", [this](const core::message& msg) {
            handleTaskCancel(msg);
        });

        // Handle cluster coordination
        bus.subscribe("cluster.rebalance", [this](const core::message& msg) {
            handleRebalance(msg);
        });

        // Health check
        bus.subscribe("health.check", [this](const core::message& msg) {
            respondHealthCheck(msg);
        });
    }

    void handleNewTask(const core::message& msg) {
        try {
            // Deserialize task
            Task task = Task::deserialize(msg.get_payload_as<std::string>());

            // Log task reception
            logTaskReceived(task);

            // Process task
            auto start_time = std::chrono::steady_clock::now();
            bool success = processTask(task);
            auto end_time = std::chrono::steady_clock::now();

            // Update metrics
            updateMetrics(start_time, end_time);

            if (success) {
                tasks_processed++;
                sendTaskComplete(task);
            } else {
                tasks_failed++;
                if (task.retry_count < 3) {
                    retryTask(task);
                } else {
                    sendTaskFailed(task);
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Error handling task: " << e.what() << std::endl;
            tasks_failed++;
        }
    }

    bool processTask(const Task& task) {
        std::cout << "Worker " << worker_id << " processing task "
                  << task.id << " of type " << static_cast<int>(task.type) << std::endl;

        // Find appropriate processor
        if (auto it = processors.find(task.type); it != processors.end()) {
            return it->second(task);
        }

        std::cerr << "No processor found for task type" << std::endl;
        return false;
    }

    // Task processors implementation
    bool processData(const Task& task) {
        // Simulate data processing
        std::cout << "Processing data: " << task.payload.substr(0, 50) << "..." << std::endl;

        // Use container service for data transformation
        auto container = container_svc->create_container();
        container->set("task_id", task.id);
        container->set("processed_at", std::chrono::system_clock::now());

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 900));

        // Store result in database
        database_svc->store("results", task.id, container->serialize());

        return true;
    }

    bool analyzeImage(const Task& task) {
        std::cout << "Analyzing image: " << task.id << std::endl;

        // Simulate image analysis with random success rate
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 1500));

        // 90% success rate
        return (rand() % 10) < 9;
    }

    bool generateReport(const Task& task) {
        std::cout << "Generating report: " << task.id << std::endl;

        // Fetch data from database
        auto data = database_svc->fetch("report_data", task.payload);

        // Generate report (simulated)
        std::this_thread::sleep_for(std::chrono::milliseconds(200 + rand() % 800));

        // Store report
        database_svc->store("reports", task.id, "Report content here");

        return true;
    }

    bool sendEmail(const Task& task) {
        std::cout << "Sending email for task: " << task.id << std::endl;

        // Simulate email sending
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 200));

        return true;
    }

    bool warmCache(const Task& task) {
        std::cout << "Warming cache: " << task.payload << std::endl;

        // Fetch frequently accessed data
        auto data = database_svc->fetch_batch("cache_data", 100);

        // Store in container for fast access
        for (const auto& item : data) {
            container_svc->cache(item.key, item.value);
        }

        return true;
    }

    void handleTaskCancel(const core::message& msg) {
        auto task_id = msg.get_payload_as<std::string>();
        std::cout << "Cancelling task: " << task_id << std::endl;
        // In production, would need to track and cancel running tasks
    }

    void handleRebalance(const core::message& msg) {
        std::cout << "Rebalancing work distribution..." << std::endl;
        // Implement load balancing logic
    }

    void respondHealthCheck(const core::message& msg) {
        core::message response;
        response.set_type("health.response");
        response.set_header("worker_id", worker_id);
        response.set_header("status", "healthy");
        response.set_header("tasks_processed", std::to_string(tasks_processed.load()));
        response.set_header("tasks_failed", std::to_string(tasks_failed.load()));
        response.set_header("uptime", std::to_string(getUptime()));

        integrator->get_message_bus().publish(response);
    }

    void retryTask(Task task) {
        task.retry_count++;

        core::message retry_msg;
        retry_msg.set_type("task.retry");
        retry_msg.set_payload(task.serialize());
        retry_msg.set_priority(core::priority::LOW);  // Lower priority for retries
        retry_msg.set_header("retry_count", std::to_string(task.retry_count));

        // Schedule retry with exponential backoff
        auto delay = std::chrono::seconds(std::pow(2, task.retry_count));
        integrator->get_message_bus().publish_delayed(retry_msg, delay);
    }

    void sendTaskComplete(const Task& task) {
        core::message complete_msg;
        complete_msg.set_type("task.complete");
        complete_msg.set_header("task_id", task.id);
        complete_msg.set_header("worker_id", worker_id);
        complete_msg.set_header("processing_time", std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count() -
            task.created_at.time_since_epoch().count()
        ));

        integrator->get_message_bus().publish(complete_msg);
    }

    void sendTaskFailed(const Task& task) {
        core::message failed_msg;
        failed_msg.set_type("task.failed");
        failed_msg.set_header("task_id", task.id);
        failed_msg.set_header("worker_id", worker_id);
        failed_msg.set_header("reason", "Max retries exceeded");

        integrator->get_message_bus().publish(failed_msg);
    }

    void logTaskReceived(const Task& task) {
        std::cout << "[" << worker_id << "] Received task " << task.id
                  << " with priority " << task.priority << std::endl;
    }

    void updateMetrics(
        const std::chrono::steady_clock::time_point& start,
        const std::chrono::steady_clock::time_point& end
    ) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        int total = metrics.total_tasks.fetch_add(1) + 1;
        double current_avg = metrics.avg_processing_time.load();
        double new_avg = (current_avg * (total - 1) + duration.count()) / total;

        metrics.avg_processing_time.store(new_avg);
    }

    int getUptime() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(
            now - metrics.start_time
        ).count();
    }

    std::string generateWorkerId() const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        return "worker-" + std::to_string(dis(gen));
    }

    void start() {
        std::cout << "Starting distributed worker: " << worker_id << std::endl;

        integrator->start();

        // Announce worker availability
        core::message announce;
        announce.set_type("worker.online");
        announce.set_header("worker_id", worker_id);
        announce.set_header("capabilities", "all");  // In production, list specific capabilities

        integrator->get_message_bus().publish(announce);

        // Run until stopped
        while (running) {
            std::this_thread::sleep_for(1s);

            // Periodic status update
            if (tasks_processed % 10 == 0 && tasks_processed > 0) {
                printStatus();
            }
        }

        // Announce worker going offline
        core::message offline;
        offline.set_type("worker.offline");
        offline.set_header("worker_id", worker_id);

        integrator->get_message_bus().publish(offline);
        integrator->stop();
    }

    void stop() {
        running = false;
    }

    void printStatus() const {
        std::cout << "\n=== Worker Status ===" << std::endl;
        std::cout << "Worker ID: " << worker_id << std::endl;
        std::cout << "Tasks processed: " << tasks_processed << std::endl;
        std::cout << "Tasks failed: " << tasks_failed << std::endl;
        std::cout << "Success rate: "
                  << (100.0 * tasks_processed / (tasks_processed + tasks_failed)) << "%" << std::endl;
        std::cout << "Avg processing time: " << metrics.avg_processing_time << " ms" << std::endl;
        std::cout << "Uptime: " << getUptime() << " seconds" << std::endl;
        std::cout << "===================\n" << std::endl;
    }
};

// Task generator for testing
class TaskGenerator {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::atomic<int> task_counter{0};
    std::random_device rd;
    std::mt19937 gen;

public:
    TaskGenerator() : gen(rd()) {
        config::config_builder builder;
        auto config = builder
            .set_environment("generator")
            .set_worker_threads(2)
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        integrator->start();
    }

    void generateTasks(int count, int delay_ms = 1000) {
        std::uniform_int_distribution<> type_dis(0, 4);
        std::uniform_int_distribution<> priority_dis(1, 10);

        for (int i = 0; i < count; ++i) {
            Task task;
            task.id = "task-" + std::to_string(task_counter++);
            task.type = static_cast<TaskType>(type_dis(gen));
            task.payload = "Sample data for task " + task.id;
            task.priority = priority_dis(gen);
            task.created_at = std::chrono::steady_clock::now();

            core::message task_msg;
            task_msg.set_type("task.new");
            task_msg.set_payload(task.serialize());
            task_msg.set_priority(static_cast<core::priority>(task.priority));

            integrator->get_message_bus().publish(task_msg);

            std::cout << "Generated task: " << task.id << std::endl;

            if (delay_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            }
        }
    }

    ~TaskGenerator() {
        integrator->stop();
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
            std::cout << "Running as task generator" << std::endl;
            TaskGenerator generator;

            // Generate tasks continuously
            while (true) {
                generator.generateTasks(10, 2000);  // 10 tasks every 2 seconds
                std::this_thread::sleep_for(10s);
            }

        } else {
            // Run as worker(s)
            std::vector<std::unique_ptr<DistributedWorker>> workers;
            std::vector<std::thread> worker_threads;

            for (int i = 0; i < worker_count; ++i) {
                workers.push_back(std::make_unique<DistributedWorker>());
            }

            for (auto& worker : workers) {
                worker_threads.emplace_back([&worker]() {
                    worker->start();
                });
            }

            std::cout << "Started " << worker_count << " workers. Press Enter to stop..." << std::endl;
            std::cin.get();

            for (auto& worker : workers) {
                worker->stop();
            }

            for (auto& thread : worker_threads) {
                thread.join();
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}