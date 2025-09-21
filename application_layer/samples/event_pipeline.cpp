/**
 * @file event_pipeline.cpp
 * @brief Event-driven data processing pipeline
 *
 * This example demonstrates how to build complex event-driven pipelines
 * with multiple processing stages, transformations, and error handling.
 */

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/services/container/container_service.h>
#include <kcenon/messaging/services/database/database_service.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <queue>
#include <functional>
#include <variant>
#include <regex>
#include <numeric>
#include <iomanip>
#include <sstream>
#include <optional>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Event types flowing through the pipeline
struct raw_event {
    std::string id;
    std::string source;
    std::string type;
    std::map<std::string, std::variant<std::string, double, int>> data;
    std::chrono::system_clock::time_point timestamp;
};

struct processed_event {
    std::string id;
    std::string category;
    double score;
    std::vector<std::string> tags;
    std::map<std::string, double> metrics;
    bool valid;
    std::string validation_error;
};

struct aggregated_data {
    std::string window_id;
    std::chrono::system_clock::time_point window_start;
    std::chrono::system_clock::time_point window_end;
    int event_count;
    double avg_score;
    std::map<std::string, int> tag_frequency;
    std::map<std::string, double> metric_sums;
};

// Pipeline stage definition
template<typename TInput, typename TOutput>
class pipeline_stage {
public:
    using ProcessFunc = std::function<TOutput(const TInput&)>;
    using FilterFunc = std::function<bool(const TInput&)>;
    using ErrorHandler = std::function<void(const TInput&, const std::exception&)>;

private:
    std::string stage_name;
    ProcessFunc processor;
    FilterFunc filter;
    ErrorHandler error_handler;
    std::atomic<uint64_t> processed_count{0};
    std::atomic<uint64_t> error_count{0};
    std::atomic<uint64_t> filtered_count{0};

public:
    pipeline_stage(
        const std::string& name,
        ProcessFunc proc,
        FilterFunc filt = nullptr,
        ErrorHandler err = nullptr
    ) : stage_name(name), processor(proc), filter(filt), error_handler(err) {}

    std::optional<TOutput> process(const TInput& input) {
        try {
            // Apply filter if provided
            if (filter && !filter(input)) {
                filtered_count++;
                return std::nullopt;
            }

            // Process the input
            TOutput output = processor(input);
            processed_count++;
            return output;

        } catch (const std::exception& e) {
            error_count++;
            if (error_handler) {
                error_handler(input, e);
            }
            return std::nullopt;
        }
    }

    void printStats(std::shared_ptr<logger_module::logger> logger = nullptr) const {
        std::stringstream stats;
        stats << "Stage: " << stage_name
              << " | Processed: " << processed_count
              << " | Filtered: " << filtered_count
              << " | Errors: " << error_count;

        if (logger) {
            logger->log(logger_module::log_level::info, stats.str());
        }
    }
};

class event_pipeline {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::unique_ptr<services::container_service> container_svc;
    std::unique_ptr<services::database_service> database_svc;
    std::shared_ptr<logger_module::logger> m_logger;

    // Pipeline stages
    std::unique_ptr<pipeline_stage<raw_event, raw_event>> validation_stage;
    std::unique_ptr<pipeline_stage<raw_event, processed_event>> enrichment_stage;
    std::unique_ptr<pipeline_stage<processed_event, processed_event>> transformation_stage;
    std::unique_ptr<pipeline_stage<processed_event, aggregated_data>> aggregation_stage;

    // Event queues
    std::queue<raw_event> raw_events;
    std::queue<processed_event> processed_events;
    std::queue<aggregated_data> aggregated_data;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;

    // Windowing for aggregation
    std::chrono::seconds window_size{60};
    std::map<std::string, std::vector<processed_event>> window_buffers;
    std::mutex window_mutex;

    // Metrics
    std::atomic<uint64_t> total_events{0};
    std::atomic<uint64_t> events_per_second{0};
    std::atomic<bool> running{true};

    // Dead letter queue for failed events
    std::queue<std::pair<std::string, raw_event>> dead_letter_queue;
    std::mutex dlq_mutex;

public:
    event_pipeline() {
        // Initialize logger
        logger_module::logger_config logger_config;
        logger_config.min_level = logger_module::log_level::debug;
        logger_config.pattern = "[{timestamp}] [{level}] [Pipeline] {message}";
        logger_config.enable_async = true;
        logger_config.async_queue_size = 8192;

        m_logger = std::make_shared<logger_module::logger>(logger_config);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "event_pipeline.log", 10 * 1024 * 1024, 5));
        m_logger->start();

        m_logger->log(logger_module::log_level::info, "Initializing Event Pipeline");

        // Configure for event processing
        config::config_builder builder;
        auto config = builder
            .set_environment("event_processing")
            .set_worker_threads(std::thread::hardware_concurrency())
            .set_queue_size(1000000)
            .set_max_message_size(256 * 1024)
            .enable_persistence(true)
            .enable_monitoring(true)
            .enable_compression(true)
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        container_svc = std::make_unique<services::container_service>();
        database_svc = std::make_unique<services::database_service>();

        setupPipeline();
        setupMessageHandlers();
    }

    void setupPipeline() {
        // Stage 1: Validation
        validation_stage = std::make_unique<pipeline_stage<raw_event, raw_event>>(
            "Validation",
            [this](const raw_event& event) {
                return validateEvent(event);
            },
            [](const raw_event& event) {
                // Filter out test events
                return event.source != "test";
            },
            [this](const raw_event& event, const std::exception& e) {
                sendToDeadLetterQueue(event, e.what());
            }
        );

        // Stage 2: Enrichment
        enrichment_stage = std::make_unique<pipeline_stage<raw_event, processed_event>>(
            "Enrichment",
            [this](const raw_event& event) {
                return enrichEvent(event);
            }
        );

        // Stage 3: Transformation
        transformation_stage = std::make_unique<pipeline_stage<processed_event, processed_event>>(
            "Transformation",
            [this](const processed_event& event) {
                return transformEvent(event);
            },
            [](const processed_event& event) {
                // Filter invalid events
                return event.valid;
            }
        );

        // Stage 4: Aggregation
        aggregation_stage = std::make_unique<pipeline_stage<processed_event, aggregated_data>>(
            "Aggregation",
            [this](const processed_event& event) {
                return aggregateEvent(event);
            }
        );
    }

    void setupMessageHandlers() {
        auto& bus = integrator->get_message_bus();

        // Incoming raw events
        bus.subscribe("event.raw", [this](const core::message& msg) {
            handleRawEvent(msg);
        });

        // Pipeline control
        bus.subscribe("pipeline.control", [this](const core::message& msg) {
            handlePipelineControl(msg);
        });

        // Query processed data
        bus.subscribe("pipeline.query", [this](const core::message& msg) {
            handleDataQuery(msg);
        });

        // Dead letter queue management
        bus.subscribe("dlq.retry", [this](const core::message& msg) {
            retryDeadLetterEvents();
        });
    }

    raw_event validateEvent(const raw_event& event) {
        // Validation rules
        if (event.id.empty()) {
            throw std::runtime_error("Event ID is required");
        }

        if (event.source.empty()) {
            throw std::runtime_error("Event source is required");
        }

        if (event.type.empty()) {
            throw std::runtime_error("Event type is required");
        }

        // Check timestamp is reasonable (not future, not too old)
        auto now = std::chrono::system_clock::now();
        auto age = now - event.timestamp;

        if (event.timestamp > now) {
            throw std::runtime_error("Event timestamp is in the future");
        }

        if (age > 24h) {
            throw std::runtime_error("Event is too old (>24 hours)");
        }

        return event;
    }

    processed_event enrichEvent(const raw_event& event) {
        processed_event processed;
        processed.id = event.id;
        processed.valid = true;

        // Categorize event
        if (event.type.find("error") != std::string::npos) {
            processed.category = "error";
            processed.score = 1.0;
        } else if (event.type.find("warning") != std::string::npos) {
            processed.category = "warning";
            processed.score = 0.5;
        } else if (event.type.find("info") != std::string::npos) {
            processed.category = "info";
            processed.score = 0.1;
        } else {
            processed.category = "other";
            processed.score = 0.0;
        }

        // Extract tags from event data
        for (const auto& [key, value] : event.data) {
            if (key.find("tag") != std::string::npos) {
                if (auto* str_val = std::get_if<std::string>(&value)) {
                    processed.tags.push_back(*str_val);
                }
            }
        }

        // Calculate metrics
        processed.metrics["event_size"] = static_cast<double>(event.data.size());
        processed.metrics["processing_delay"] =
            std::chrono::duration<double>(
                std::chrono::system_clock::now() - event.timestamp
            ).count();

        // Add enrichment from external sources (simulated)
        if (event.source == "sensor") {
            processed.tags.push_back("iot");
            processed.metrics["priority"] = 0.8;
        } else if (event.source == "application") {
            processed.tags.push_back("app");
            processed.metrics["priority"] = 0.5;
        }

        return processed;
    }

    processed_event transformEvent(const processed_event& event) {
        processed_event transformed = event;

        // Apply transformations
        // Normalize score to 0-100 range
        transformed.score *= 100.0;

        // Add derived metrics
        transformed.metrics["weighted_score"] =
            transformed.score * transformed.metrics["priority"];

        // Clean up tags (remove duplicates, sort)
        std::sort(transformed.tags.begin(), transformed.tags.end());
        transformed.tags.erase(
            std::unique(transformed.tags.begin(), transformed.tags.end()),
            transformed.tags.end()
        );

        // Apply business rules
        if (transformed.category == "error" && transformed.metrics["priority"] > 0.7) {
            transformed.tags.push_back("critical");
        }

        return transformed;
    }

    aggregated_data aggregateEvent(const processed_event& event) {
        // Time-based windowing
        auto now = std::chrono::system_clock::now();
        auto window_start = std::chrono::floor<std::chrono::seconds>(now);
        auto window_id = std::to_string(window_start.time_since_epoch().count());

        std::lock_guard<std::mutex> lock(window_mutex);

        // Add event to window
        window_buffers[window_id].push_back(event);

        // Check if window is complete
        if (window_buffers[window_id].size() >= 100 ||
            (now - window_start) > window_size) {

            // Aggregate the window
            aggregated_data aggregated;
            aggregated.window_id = window_id;
            aggregated.window_start = window_start;
            aggregated.window_end = now;
            aggregated.event_count = window_buffers[window_id].size();

            // Calculate averages and sums
            double total_score = 0.0;
            for (const auto& e : window_buffers[window_id]) {
                total_score += e.score;

                // Count tags
                for (const auto& tag : e.tags) {
                    aggregated.tag_frequency[tag]++;
                }

                // Sum metrics
                for (const auto& [metric, value] : e.metrics) {
                    aggregated.metric_sums[metric] += value;
                }
            }

            aggregated.avg_score = total_score / aggregated.event_count;

            // Clear window
            window_buffers.erase(window_id);

            return aggregated;
        }

        // Return empty aggregation if window not complete
        return aggregated_data{};
    }

    void handleRawEvent(const core::message& msg) {
        try {
            raw_event event;
            event.id = msg.get_header("event_id");
            event.source = msg.get_header("source");
            event.type = msg.get_header("type");
            event.timestamp = std::chrono::system_clock::now();

            // Parse event data from payload
            auto payload = msg.get_payload_as<std::string>();
            // In production, parse JSON or other format

            // Add to processing queue
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                raw_events.push(event);
                total_events++;
            }
            queue_cv.notify_one();

        } catch (const std::exception& e) {
            m_logger->log(logger_module::log_level::error,
                "Error handling raw event: " + std::string(e.what()));
        }
    }

    void handlePipelineControl(const core::message& msg) {
        auto command = msg.get_header("command");

        if (command == "pause") {
            pausePipeline();
        } else if (command == "resume") {
            resumePipeline();
        } else if (command == "stats") {
            printPipelineStats();
        } else if (command == "flush") {
            flushPipeline();
        }
    }

    void handleDataQuery(const core::message& msg) {
        auto query_type = msg.get_header("query");

        if (query_type == "aggregated") {
            sendAggregatedData();
        } else if (query_type == "processed") {
            sendProcessedEvents();
        } else if (query_type == "metrics") {
            sendPipelineMetrics();
        }
    }

    void sendToDeadLetterQueue(const raw_event& event, const std::string& reason) {
        std::lock_guard<std::mutex> lock(dlq_mutex);
        dead_letter_queue.push({reason, event});

        m_logger->log(logger_module::log_level::warning,
            "Event " + event.id + " sent to DLQ. Reason: " + reason);

        // Persist to database
        auto container = container_svc->create_container();
        container->set("event_id", event.id);
        container->set("reason", reason);
        container->set("timestamp", std::chrono::system_clock::to_time_t(event.timestamp));

        database_svc->store("dead_letter_queue", event.id, container->serialize());
    }

    void retryDeadLetterEvents() {
        std::lock_guard<std::mutex> lock(dlq_mutex);

        m_logger->log(logger_module::log_level::info,
            "Retrying " + std::to_string(dead_letter_queue.size()) +
            " events from dead letter queue");

        while (!dead_letter_queue.empty()) {
            auto [reason, event] = dead_letter_queue.front();
            dead_letter_queue.pop();

            // Re-submit for processing
            std::lock_guard<std::mutex> queue_lock(queue_mutex);
            raw_events.push(event);
        }

        queue_cv.notify_all();
    }

    void startProcessingThreads() {
        // Stage 1 & 2: Validation and Enrichment
        std::thread([this]() {
            while (running) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [this] {
                    return !raw_events.empty() || !running;
                });

                while (!raw_events.empty()) {
                    raw_event event = raw_events.front();
                    raw_events.pop();
                    lock.unlock();

                    // Process through validation
                    if (auto validated = validation_stage->process(event)) {
                        // Process through enrichment
                        if (auto enriched = enrichment_stage->process(*validated)) {
                            std::lock_guard<std::mutex> proc_lock(queue_mutex);
                            processed_events.push(*enriched);
                        }
                    }

                    lock.lock();
                }
            }
        }).detach();

        // Stage 3 & 4: Transformation and Aggregation
        std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(100ms);

                std::lock_guard<std::mutex> lock(queue_mutex);

                while (!processed_events.empty()) {
                    processed_event event = processed_events.front();
                    processed_events.pop();

                    // Process through transformation
                    if (auto transformed = transformation_stage->process(event)) {
                        // Process through aggregation
                        if (auto aggregated = aggregation_stage->process(*transformed)) {
                            if (aggregated->event_count > 0) {
                                aggregated_data.push(*aggregated);
                                publishAggregatedData(*aggregated);
                            }
                        }
                    }
                }
            }
        }).detach();
    }

    void publishAggregatedData(const aggregated_data& data) {
        core::message msg;
        msg.set_type("pipeline.aggregated");
        msg.set_header("window_id", data.window_id);
        msg.set_header("event_count", std::to_string(data.event_count));
        msg.set_header("avg_score", std::to_string(data.avg_score));

        // Serialize tag frequency
        std::string tags_str;
        for (const auto& [tag, count] : data.tag_frequency) {
            tags_str += tag + ":" + std::to_string(count) + ";";
        }
        msg.set_header("tags", tags_str);

        integrator->get_message_bus().publish(msg);

        // Store in database
        auto container = container_svc->create_container();
        container->set("window_id", data.window_id);
        container->set("event_count", data.event_count);
        container->set("avg_score", data.avg_score);

        database_svc->store("aggregated_data", data.window_id, container->serialize());
    }

    void startEventGenerator() {
        std::thread([this]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<> type_dist(0, 4);
            std::uniform_int_distribution<> source_dist(0, 2);
            std::uniform_real_distribution<> value_dist(0.0, 100.0);

            std::vector<std::string> types = {"info", "warning", "error", "debug", "trace"};
            std::vector<std::string> sources = {"sensor", "application", "user"};

            while (running) {
                core::message event;
                event.set_type("event.raw");
                event.set_header("event_id", "evt-" + std::to_string(total_events.load()));
                event.set_header("type", types[type_dist(gen)]);
                event.set_header("source", sources[source_dist(gen)]);

                // Add random data
                auto container = container_svc->create_container();
                container->set("value", value_dist(gen));
                container->set("tag_category", types[type_dist(gen)]);
                event.set_payload(container->serialize());

                integrator->get_message_bus().publish(event);

                events_per_second++;

                std::this_thread::sleep_for(std::chrono::milliseconds(1 + gen() % 10));
            }
        }).detach();
    }

    void startMetricsCollector() {
        std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(1s);

                // Reset per-second counter
                uint64_t current_eps = events_per_second.exchange(0);

                // Calculate pipeline throughput
                static uint64_t last_total = 0;
                uint64_t current_total = total_events.load();
                uint64_t throughput = current_total - last_total;
                last_total = current_total;

                // Publish metrics
                core::message metrics;
                metrics.set_type("pipeline.metrics");
                metrics.set_header("events_per_second", std::to_string(current_eps));
                metrics.set_header("throughput", std::to_string(throughput));
                metrics.set_header("total_events", std::to_string(current_total));
                metrics.set_header("dlq_size", std::to_string(dead_letter_queue.size()));

                integrator->get_message_bus().publish(metrics);
            }
        }).detach();
    }

    void pausePipeline() {
        m_logger->log(logger_module::log_level::info, "Pipeline paused");
        // Implementation would pause processing
    }

    void resumePipeline() {
        m_logger->log(logger_module::log_level::info, "Pipeline resumed");
        // Implementation would resume processing
    }

    void flushPipeline() {
        m_logger->log(logger_module::log_level::info, "Flushing pipeline...");

        // Process all pending events
        while (!raw_events.empty() || !processed_events.empty()) {
            std::this_thread::sleep_for(100ms);
        }

        m_logger->log(logger_module::log_level::info, "Pipeline flushed");
    }

    void printPipelineStats() {
        std::stringstream stats;
        stats << "\n╔══════════════════════════════════════════════════════════╗\n";
        stats << "║              Event Processing Pipeline Stats             ║\n";
        stats << "╠══════════════════════════════════════════════════════════╣\n";

        validation_stage->printStats(m_logger);
        enrichment_stage->printStats(m_logger);
        transformation_stage->printStats(m_logger);
        aggregation_stage->printStats(m_logger);

        stats << "╠══════════════════════════════════════════════════════════╣\n";
        stats << "║ Queue Sizes:                                             ║\n";
        stats << "║   Raw Events: " << std::setw(43)
              << raw_events.size() << " ║\n";
        stats << "║   Processed Events: " << std::setw(37)
              << processed_events.size() << " ║\n";
        stats << "║   Aggregated Data: " << std::setw(38)
              << aggregated_data.size() << " ║\n";
        stats << "║   Dead Letter Queue: " << std::setw(36)
              << dead_letter_queue.size() << " ║\n";
        stats << "╠══════════════════════════════════════════════════════════╣\n";
        stats << "║ Total Events Processed: " << std::setw(33)
              << total_events.load() << " ║\n";
        stats << "╚══════════════════════════════════════════════════════════╝";

        m_logger->log(logger_module::log_level::info, stats.str());
    }

    void sendAggregatedData() {
        std::lock_guard<std::mutex> lock(queue_mutex);

        while (!aggregated_data.empty()) {
            auto data = aggregated_data.front();
            aggregated_data.pop();
            publishAggregatedData(data);
        }
    }

    void sendProcessedEvents() {
        // Send latest processed events
        core::message response;
        response.set_type("pipeline.processed_events");
        response.set_header("count", std::to_string(processed_events.size()));

        integrator->get_message_bus().publish(response);
    }

    void sendPipelineMetrics() {
        core::message metrics;
        metrics.set_type("pipeline.detailed_metrics");
        metrics.set_header("total_events", std::to_string(total_events.load()));
        metrics.set_header("events_per_second", std::to_string(events_per_second.load()));
        metrics.set_header("dlq_size", std::to_string(dead_letter_queue.size()));

        integrator->get_message_bus().publish(metrics);
    }

public:
    void start() {
        m_logger->log(logger_module::log_level::info,
            "\n=== Event Processing Pipeline Starting ===");

        integrator->start();

        // Start processing threads
        startProcessingThreads();
        startEventGenerator();
        startMetricsCollector();

        // Periodic stats display
        std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(30s);
                printPipelineStats();
            }
        }).detach();

        std::cout << "Event Pipeline is running. Press Enter to stop..." << std::endl;
        std::cin.get();

        stop();
    }

    void stop() {
        running = false;
        queue_cv.notify_all();

        // Flush remaining events
        flushPipeline();

        integrator->stop();

        m_logger->log(logger_module::log_level::info, "\n=== Final Statistics ===");
        printPipelineStats();
        m_logger->log(logger_module::log_level::info, "========================");
        m_logger->flush();
        m_logger->stop();
    }
};

int main(int argc, char* argv[]) {
    try {
        event_pipeline pipeline;
        pipeline.start();

    } catch (const std::exception& e) {
        // Create a minimal logger for error reporting
        logger_module::logger_config error_logger_config;
        error_logger_config.min_level = logger_module::log_level::error;
        auto error_logger = std::make_shared<logger_module::logger>(error_logger_config);
        error_logger->add_writer(std::make_unique<logger_module::console_writer>());
        error_logger->start();
        error_logger->log(logger_module::log_level::error, "Error: " + std::string(e.what()));
        error_logger->stop();
        return 1;
    }

    return 0;
}