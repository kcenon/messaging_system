/**
 * @file microservices_orchestrator.cpp
 * @brief Microservices orchestration and service mesh implementation
 *
 * This example demonstrates how to build a microservices orchestrator
 * that handles service discovery, load balancing, health checks, and
 * circuit breaking patterns.
 */

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/services/container/container_service.h>
#include <kcenon/messaging/services/database/database_service.h>
#include <kcenon/messaging/services/network/network_service.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <random>
#include <atomic>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Service definition
struct service_definition {
    std::string service_name;
    std::string version;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> config;
    int min_instances;
    int max_instances;
    std::chrono::seconds health_check_interval{10};
    std::chrono::seconds startup_timeout{30};
};

// Service instance
struct service_instance {
    std::string instance_id;
    std::string service_name;
    std::string host;
    int port;
    std::string version;

    enum state {
        STARTING,
        HEALTHY,
        UNHEALTHY,
        DRAINING,
        STOPPED
    } m_state;

    std::chrono::steady_clock::time_point last_health_check;
    int consecutive_failures{0};
    double cpu_usage{0.0};
    double memory_usage{0.0};
    int active_connections{0};
    double response_time_ms{0.0};
};

// Circuit breaker for service calls
class circuit_breaker {
public:
    enum state {
        CLOSED,     // Normal operation
        OPEN,       // Failures exceeded threshold, blocking calls
        HALF_OPEN   // Testing if service recovered
    };

private:
    state m_state{CLOSED};
    int failure_count{0};
    int success_count{0};
    int failure_threshold{5};
    int success_threshold{3};
    std::chrono::steady_clock::time_point last_failure_time;
    std::chrono::seconds timeout{30s};
    mutable std::mutex mutex;

public:
    bool canAttempt() {
        std::lock_guard<std::mutex> lock(mutex);

        if (m_state == CLOSED) {
            return true;
        }

        if (m_state == OPEN) {
            auto now = std::chrono::steady_clock::now();
            if (now - last_failure_time > timeout) {
                m_state = HALF_OPEN;
                return true;
            }
            return false;
        }

        // HALF_OPEN
        return true;
    }

    void recordSuccess() {
        std::lock_guard<std::mutex> lock(mutex);

        if (m_state == HALF_OPEN) {
            success_count++;
            if (success_count >= success_threshold) {
                m_state = CLOSED;
                failure_count = 0;
                success_count = 0;
            }
        } else if (m_state == CLOSED) {
            failure_count = 0;
        }
    }

    void recordFailure() {
        std::lock_guard<std::mutex> lock(mutex);

        failure_count++;
        last_failure_time = std::chrono::steady_clock::now();

        if (m_state == HALF_OPEN || failure_count >= failure_threshold) {
            m_state = OPEN;
            success_count = 0;
        }
    }

    state getState() const {
        std::lock_guard<std::mutex> lock(mutex);
        return m_state;
    }
};

// Load balancer strategies
class load_balancer {
public:
    enum strategy {
        ROUND_ROBIN,
        LEAST_CONNECTIONS,
        RANDOM,
        WEIGHTED_RESPONSE_TIME,
        IP_HASH
    };

private:
    strategy m_strategy;
    std::atomic<size_t> round_robin_counter{0};
    std::mt19937 random_gen{std::random_device{}()};

public:
    load_balancer(strategy s = ROUND_ROBIN) : m_strategy(s) {}

    service_instance* selectInstance(
        std::vector<service_instance>& instances,
        const std::string& client_ip = ""
    ) {
        // Filter healthy instances
        std::vector<service_instance*> healthy;
        for (auto& instance : instances) {
            if (instance.m_state == service_instance::HEALTHY) {
                healthy.push_back(&instance);
            }
        }

        if (healthy.empty()) {
            return nullptr;
        }

        switch (m_strategy) {
            case ROUND_ROBIN:
                return healthy[round_robin_counter++ % healthy.size()];

            case LEAST_CONNECTIONS:
                return *std::min_element(healthy.begin(), healthy.end(),
                    [](service_instance* a, service_instance* b) {
                        return a->active_connections < b->active_connections;
                    });

            case RANDOM: {
                std::uniform_int_distribution<> dist(0, healthy.size() - 1);
                return healthy[dist(random_gen)];
            }

            case WEIGHTED_RESPONSE_TIME:
                return *std::min_element(healthy.begin(), healthy.end(),
                    [](service_instance* a, service_instance* b) {
                        return a->response_time_ms < b->response_time_ms;
                    });

            case IP_HASH: {
                if (!client_ip.empty()) {
                    std::hash<std::string> hasher;
                    size_t hash = hasher(client_ip);
                    return healthy[hash % healthy.size()];
                }
                return healthy[0];
            }

            default:
                return healthy[0];
        }
    }
};

class microservices_orchestrator {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::unique_ptr<services::container::container_service> container_svc;
    std::unique_ptr<services::database::database_service> database_svc;
    std::unique_ptr<services::network::network_service> network_svc;
    std::shared_ptr<logger_module::logger> m_logger;

    // Service registry
    std::map<std::string, service_definition> service_definitions;
    std::map<std::string, std::vector<service_instance>> service_instances;
    std::map<std::string, circuit_breaker> circuit_breakers;
    std::map<std::string, load_balancer> load_balancers;
    std::mutex registry_mutex;

    // Metrics
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> successful_requests{0};
    std::atomic<uint64_t> failed_requests{0};
    std::atomic<uint64_t> circuit_breaker_trips{0};

    // System state
    std::atomic<bool> running{true};

public:
    microservices_orchestrator() {
        // Initialize logger
        m_logger = std::make_shared<logger_module::logger>(true, 16384);
        m_logger->add_writer(std::make_unique<logger_module::console_writer>());
        m_logger->add_writer(std::make_unique<logger_module::rotating_file_writer>(
            "microservices_orchestrator.log", 20 * 1024 * 1024, 5));

        m_logger->log(logger_module::log_level::info, "Initializing Microservices Orchestrator");

        // Configure for microservices workload
        config::config_builder builder;
        auto config = builder
            .set_environment("microservices")
            .set_worker_threads(std::thread::hardware_concurrency() * 2)
            .set_queue_size(500000)
            .set_container_max_size(1024 * 1024)  // 1MB for service payloads
            .enable_external_monitoring(true)
            .enable_compression(true)
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        container_svc = std::make_unique<services::container::container_service>();
        database_svc = std::make_unique<services::database::database_service>();
        network_svc = std::make_unique<services::network::network_service>();

        setupMessageHandlers();
        initializeServices();
    }

    void setupMessageHandlers() {
        auto* bus = integrator->get_message_bus();

        // Service registration
        bus->subscribe("service.register", [this](const core::message& msg) {
            handleServiceRegistration(msg);
        });

        // Service discovery
        bus->subscribe("service.discover", [this](const core::message& msg) {
            handleServiceDiscovery(msg);
        });

        // Service health check
        bus->subscribe("service.health", [this](const core::message& msg) {
            handleHealthCheck(msg);
        });

        // Service request routing
        bus->subscribe("service.request", [this](const core::message& msg) {
            handleServiceRequest(msg);
        });

        // Service scaling
        bus->subscribe("service.scale", [this](const core::message& msg) {
            handleServiceScaling(msg);
        });

        // Service deployment
        bus->subscribe("service.deploy", [this](const core::message& msg) {
            handleServiceDeployment(msg);
        });
    }

    void initializeServices() {
        // Define core services
        defineService("api-gateway", "1.0.0", {}, 2, 10);
        defineService("auth-service", "2.1.0", {}, 2, 5);
        defineService("user-service", "1.5.0", {"auth-service", "database-service"}, 3, 8);
        defineService("product-service", "1.2.0", {"database-service", "cache-service"}, 3, 10);
        defineService("order-service", "1.0.0", {"user-service", "product-service", "payment-service"}, 2, 6);
        defineService("payment-service", "1.1.0", {"auth-service"}, 2, 4);
        defineService("notification-service", "1.0.0", {"user-service"}, 1, 3);
        defineService("database-service", "1.0.0", {}, 3, 5);
        defineService("cache-service", "1.0.0", {}, 2, 4);
        defineService("analytics-service", "1.0.0", {"database-service"}, 1, 3);

        // Start initial instances
        for (const auto& [name, definition] : service_definitions) {
            for (int i = 0; i < definition.min_instances; i++) {
                deploy_service_instance(name);
            }
        }

        m_logger->log(logger_module::log_level::info,
            "Initialized " + std::to_string(service_definitions.size()) + " service definitions");
    }

    void defineService(
        const std::string& name,
        const std::string& version,
        const std::vector<std::string>& dependencies,
        int min_instances,
        int max_instances
    ) {
        service_definition def;
        def.service_name = name;
        def.version = version;
        def.dependencies = dependencies;
        def.min_instances = min_instances;
        def.max_instances = max_instances;

        service_definitions[name] = def;
        circuit_breakers.try_emplace(name);
        load_balancers.try_emplace(name, load_balancer::LEAST_CONNECTIONS);
    }

    void deploy_service_instance(const std::string& service_name) {
        static int port_counter = 8000;

        service_instance instance;
        instance.instance_id = generateInstanceId(service_name);
        instance.service_name = service_name;
        instance.host = "10.0.0." + std::to_string(1 + (port_counter % 254));
        instance.port = port_counter++;
        instance.version = service_definitions[service_name].version;
        instance.m_state = service_instance::STARTING;
        instance.last_health_check = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            service_instances[service_name].push_back(instance);
        }

        // Simulate startup
        std::thread([this, instance]() {
            std::this_thread::sleep_for(2s);
            updateInstanceState(instance.instance_id, service_instance::HEALTHY);

            m_logger->log(logger_module::log_level::info,
                "Started " + instance.service_name + " instance " + instance.instance_id +
                " at " + instance.host + ":" + std::to_string(instance.port));
        }).detach();
    }

    void handleServiceRegistration(const core::message& msg) {
        auto service_name = msg.metadata.headers.count("service_name") ? msg.metadata.headers.at("service_name") : "";
        auto instance_id = msg.metadata.headers.count("instance_id") ? msg.metadata.headers.at("instance_id") : "";
        auto host = msg.metadata.headers.count("host") ? msg.metadata.headers.at("host") : "";
        auto port_str = msg.metadata.headers.count("port") ? msg.metadata.headers.at("port") : "8080";
        auto port = std::stoi(port_str);

        service_instance instance;
        instance.instance_id = instance_id;
        instance.service_name = service_name;
        instance.host = host;
        instance.port = port;
        instance.m_state = service_instance::HEALTHY;
        instance.last_health_check = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            service_instances[service_name].push_back(instance);
        }

        m_logger->log(logger_module::log_level::info,
            "Registered service instance: " + service_name + " (" + instance_id +
            ") at " + host + ":" + std::to_string(port));

        // Notify dependent services
        notifyDependents(service_name);
    }

    void handleServiceDiscovery(const core::message& msg) {
        auto service_name = msg.metadata.headers.count("service_name") ? msg.metadata.headers.at("service_name") : "";
        auto client_id = msg.metadata.headers.count("client_id") ? msg.metadata.headers.at("client_id") : "";

        std::lock_guard<std::mutex> lock(registry_mutex);

        if (auto it = service_instances.find(service_name); it != service_instances.end()) {
            // Select instance using load balancer
            auto* instance = load_balancers[service_name].selectInstance(it->second);

            if (instance) {
                core::message response("service.discovered");
                response.metadata.type = core::message_type::response;
                response.metadata.headers["service_name"] = service_name;
                response.metadata.headers["instance_id"] = instance->instance_id;
                response.metadata.headers["host"] = instance->host;
                response.metadata.headers["port"] = std::to_string(instance->port);
                response.metadata.headers["version"] = instance->version;

                integrator->get_message_bus()->publish(response);

                m_logger->log(logger_module::log_level::debug,
                    "Service discovery: " + client_id + " -> " + instance->instance_id);
            } else {
                // No healthy instances available
                sendServiceUnavailable(service_name, client_id);
            }
        } else {
            sendServiceNotFound(service_name, client_id);
        }
    }

    void handleHealthCheck(const core::message& msg) {
        auto instance_id = msg.metadata.headers.count("instance_id") ? msg.metadata.headers.at("instance_id") : "";
        auto status = msg.metadata.headers.count("status") ? msg.metadata.headers.at("status") : "unknown";
        auto cpu_str = msg.metadata.headers.count("cpu_usage") ? msg.metadata.headers.at("cpu_usage") : "0.0";
        auto memory_str = msg.metadata.headers.count("memory_usage") ? msg.metadata.headers.at("memory_usage") : "0.0";
        auto connections_str = msg.metadata.headers.count("active_connections") ? msg.metadata.headers.at("active_connections") : "0";
        auto response_time_str = msg.metadata.headers.count("response_time_ms") ? msg.metadata.headers.at("response_time_ms") : "0.0";
        auto cpu = std::stod(cpu_str);
        auto memory = std::stod(memory_str);
        auto connections = std::stoi(connections_str);
        auto response_time = std::stod(response_time_str);

        std::lock_guard<std::mutex> lock(registry_mutex);

        for (auto& [service_name, instances] : service_instances) {
            for (auto& instance : instances) {
                if (instance.instance_id == instance_id) {
                    instance.last_health_check = std::chrono::steady_clock::now();
                    instance.cpu_usage = cpu;
                    instance.memory_usage = memory;
                    instance.active_connections = connections;
                    instance.response_time_ms = response_time;

                    if (status == "healthy") {
                        instance.m_state = service_instance::HEALTHY;
                        instance.consecutive_failures = 0;
                    } else {
                        instance.consecutive_failures++;
                        if (instance.consecutive_failures >= 3) {
                            instance.m_state = service_instance::UNHEALTHY;
                            handleUnhealthyInstance(instance);
                        }
                    }

                    return;
                }
            }
        }
    }

    void handleServiceRequest(const core::message& msg) {
        total_requests++;

        auto service_name = msg.metadata.headers.count("service_name") ? msg.metadata.headers.at("service_name") : "";
        auto client_ip = msg.metadata.headers.count("client_ip") ? msg.metadata.headers.at("client_ip") : "";
        auto request_id = msg.metadata.headers.count("request_id") ? msg.metadata.headers.at("request_id") : "";

        // Check circuit breaker
        if (!circuit_breakers[service_name].canAttempt()) {
            circuit_breaker_trips++;
            send_circuit_breaker_open(service_name, request_id);
            failed_requests++;
            return;
        }

        // Route request
        std::lock_guard<std::mutex> lock(registry_mutex);

        if (auto it = service_instances.find(service_name); it != service_instances.end()) {
            auto* instance = load_balancers[service_name].selectInstance(it->second, client_ip);

            if (instance) {
                // Forward request to instance
                bool success = forwardRequest(instance, msg);

                if (success) {
                    circuit_breakers[service_name].recordSuccess();
                    successful_requests++;
                    instance->active_connections++;
                } else {
                    circuit_breakers[service_name].recordFailure();
                    failed_requests++;
                }
            } else {
                failed_requests++;
                sendServiceUnavailable(service_name, request_id);
            }
        }
    }

    void handleServiceScaling(const core::message& msg) {
        auto service_name = msg.metadata.headers.count("service_name") ? msg.metadata.headers.at("service_name") : "";
        auto action = msg.metadata.headers.count("action") ? msg.metadata.headers.at("action") : "";
        auto count_str = msg.metadata.headers.count("count") ? msg.metadata.headers.at("count") : "1";
        auto count = std::stoi(count_str);

        if (action == "scale_up") {
            scaleUp(service_name, count);
        } else if (action == "scale_down") {
            scaleDown(service_name, count);
        } else if (action == "auto") {
            autoScale(service_name);
        }
    }

    void handleServiceDeployment(const core::message& msg) {
        auto service_name = msg.metadata.headers.count("service_name") ? msg.metadata.headers.at("service_name") : "";
        auto version = msg.metadata.headers.count("version") ? msg.metadata.headers.at("version") : "";
        auto strategy = msg.metadata.headers.count("strategy") ? msg.metadata.headers.at("strategy") : "rolling";  // rolling, blue_green, canary

        if (strategy == "rolling") {
            performRollingUpdate(service_name, version);
        } else if (strategy == "blue_green") {
            performBlueGreenDeployment(service_name, version);
        } else if (strategy == "canary") {
            performCanaryDeployment(service_name, version);
        }
    }

    void scaleUp(const std::string& service_name, int count) {
        auto& definition = service_definitions[service_name];

        std::lock_guard<std::mutex> lock(registry_mutex);
        auto current_count = service_instances[service_name].size();

        for (int i = 0; i < count && current_count + i < definition.max_instances; i++) {
            deploy_service_instance(service_name);
        }

        m_logger->log(logger_module::log_level::info,
            "Scaling up " + service_name + " by " + std::to_string(count) + " instances");
    }

    void scaleDown(const std::string& service_name, int count) {
        std::lock_guard<std::mutex> lock(registry_mutex);
        auto& instances = service_instances[service_name];
        auto& definition = service_definitions[service_name];

        // Don't scale below minimum
        int to_remove = std::min(count,
            static_cast<int>(instances.size()) - definition.min_instances);

        // Mark instances for draining
        for (int i = 0; i < to_remove; i++) {
            instances[i].m_state = service_instance::DRAINING;
        }

        // Remove drained instances after grace period
        std::thread([this, service_name, to_remove]() {
            std::this_thread::sleep_for(30s);  // Grace period

            std::lock_guard<std::mutex> lock(registry_mutex);
            auto& instances = service_instances[service_name];
            instances.erase(
                std::remove_if(instances.begin(), instances.end(),
                    [](const service_instance& i) {
                        return i.m_state == service_instance::DRAINING;
                    }),
                instances.end()
            );

            m_logger->log(logger_module::log_level::info,
                "Scaled down " + service_name + " by " + std::to_string(to_remove) + " instances");
        }).detach();
    }

    void autoScale(const std::string& service_name) {
        std::lock_guard<std::mutex> lock(registry_mutex);
        auto& instances = service_instances[service_name];

        // Calculate average metrics
        double avg_cpu = 0.0;
        double avg_connections = 0.0;
        int healthy_count = 0;

        for (const auto& instance : instances) {
            if (instance.m_state == service_instance::HEALTHY) {
                avg_cpu += instance.cpu_usage;
                avg_connections += instance.active_connections;
                healthy_count++;
            }
        }

        if (healthy_count > 0) {
            avg_cpu /= healthy_count;
            avg_connections /= healthy_count;

            // Scale up if CPU > 70% or connections > 100
            if (avg_cpu > 70.0 || avg_connections > 100) {
                scaleUp(service_name, 2);
            }
            // Scale down if CPU < 20% and connections < 10
            else if (avg_cpu < 20.0 && avg_connections < 10) {
                scaleDown(service_name, 1);
            }
        }
    }

    void performRollingUpdate(const std::string& service_name, const std::string& new_version) {
        m_logger->log(logger_module::log_level::info,
            "Starting rolling update for " + service_name + " to version " + new_version);

        std::thread([this, service_name, new_version]() {
            std::lock_guard<std::mutex> lock(registry_mutex);
            auto& instances = service_instances[service_name];

            for (auto& instance : instances) {
                // Update one instance at a time
                instance.m_state = service_instance::DRAINING;
                std::this_thread::sleep_for(10s);  // Drain connections

                instance.version = new_version;
                instance.m_state = service_instance::STARTING;
                std::this_thread::sleep_for(5s);  // Startup time

                instance.m_state = service_instance::HEALTHY;
                m_logger->log(logger_module::log_level::info,
                    "Updated " + instance.instance_id + " to version " + new_version);
            }

            service_definitions[service_name].version = new_version;
            m_logger->log(logger_module::log_level::info,
                "Rolling update complete for " + service_name);
        }).detach();
    }

    void performBlueGreenDeployment(const std::string& service_name, const std::string& new_version) {
        m_logger->log(logger_module::log_level::info,
            "Starting blue-green deployment for " + service_name);

        // Deploy green environment
        int instance_count = service_instances[service_name].size();
        for (int i = 0; i < instance_count; i++) {
            deploy_service_instance(service_name + "-green");
        }

        // Switch traffic after validation
        std::thread([this, service_name, new_version]() {
            std::this_thread::sleep_for(30s);  // Validation period

            // Swap blue and green
            std::lock_guard<std::mutex> lock(registry_mutex);
            service_instances[service_name] = service_instances[service_name + "-green"];
            service_instances.erase(service_name + "-green");

            m_logger->log(logger_module::log_level::info,
                "Blue-green deployment complete for " + service_name);
        }).detach();
    }

    void performCanaryDeployment(const std::string& service_name, const std::string& new_version) {
        m_logger->log(logger_module::log_level::info,
            "Starting canary deployment for " + service_name);

        // Deploy canary instance (10% of traffic)
        deploy_service_instance(service_name);

        // Monitor canary metrics
        std::thread([this, service_name, new_version]() {
            std::this_thread::sleep_for(60s);  // Monitoring period

            // If canary is healthy, continue rollout
            autoScale(service_name);  // This would check canary metrics

            m_logger->log(logger_module::log_level::info,
                "Canary deployment validated for " + service_name);
        }).detach();
    }

    bool forwardRequest(service_instance* instance, const core::message& msg) {
        // Simulate request forwarding
        core::message forward("request.forward");
        forward.metadata.type = core::message_type::request;
        forward.metadata.headers["instance_id"] = instance->instance_id;
        forward.metadata.headers["host"] = instance->host;
        forward.metadata.headers["port"] = std::to_string(instance->port);
        forward.payload = msg.payload;

        // Send to the instance via network service
        std::string destination = instance->host + ":" + std::to_string(instance->port);
        return network_svc->send_message(destination, forward);
    }

    void handleUnhealthyInstance(const service_instance& instance) {
        m_logger->log(logger_module::log_level::warning,
            "Instance " + instance.instance_id + " marked unhealthy");

        // Replace unhealthy instance
        deploy_service_instance(instance.service_name);

        // Alert operations team
        core::message alert("ops.alert");
        alert.metadata.type = core::message_type::notification;
        alert.metadata.headers["severity"] = "warning";
        alert.metadata.headers["service"] = instance.service_name;
        alert.metadata.headers["instance"] = instance.instance_id;
        alert.payload.set("message", std::string("Service instance unhealthy - replacement initiated"));

        integrator->get_message_bus()->publish(alert);
    }

    void notifyDependents(const std::string& service_name) {
        for (const auto& [name, definition] : service_definitions) {
            if (std::find(definition.dependencies.begin(),
                         definition.dependencies.end(),
                         service_name) != definition.dependencies.end()) {

                core::message notify("dependency.updated");
                notify.metadata.type = core::message_type::notification;
                notify.metadata.headers["service"] = name;
                notify.metadata.headers["dependency"] = service_name;

                integrator->get_message_bus()->publish(notify);
            }
        }
    }

    void sendServiceUnavailable(const std::string& service_name, const std::string& request_id) {
        core::message response("service.unavailable");
        response.metadata.type = core::message_type::response;
        response.metadata.headers["service_name"] = service_name;
        response.metadata.headers["request_id"] = request_id;
        response.metadata.headers["error"] = "No healthy instances available";

        integrator->get_message_bus()->publish(response);
    }

    void sendServiceNotFound(const std::string& service_name, const std::string& client_id) {
        core::message response("service.not_found");
        response.metadata.type = core::message_type::response;
        response.metadata.headers["service_name"] = service_name;
        response.metadata.headers["client_id"] = client_id;
        response.metadata.headers["error"] = "Service not registered";

        integrator->get_message_bus()->publish(response);
    }

    void send_circuit_breaker_open(const std::string& service_name, const std::string& request_id) {
        core::message response("circuit_breaker.open");
        response.metadata.type = core::message_type::response;
        response.metadata.headers["service_name"] = service_name;
        response.metadata.headers["request_id"] = request_id;
        response.metadata.headers["error"] = "Circuit breaker is open";

        integrator->get_message_bus()->publish(response);
    }

    void updateInstanceState(const std::string& instance_id, service_instance::state new_state) {
        std::lock_guard<std::mutex> lock(registry_mutex);

        for (auto& [service_name, instances] : service_instances) {
            for (auto& instance : instances) {
                if (instance.instance_id == instance_id) {
                    instance.m_state = new_state;
                    return;
                }
            }
        }
    }

    std::string generateInstanceId(const std::string& service_name) const {
        static std::atomic<int> counter{0};
        return service_name + "-" + std::to_string(counter++);
    }

    void startHealthCheckMonitor() {
        std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(10s);

                std::lock_guard<std::mutex> lock(registry_mutex);
                auto now = std::chrono::steady_clock::now();

                for (auto& [service_name, instances] : service_instances) {
                    for (auto& instance : instances) {
                        auto elapsed = now - instance.last_health_check;

                        if (elapsed > 30s && instance.m_state == service_instance::HEALTHY) {
                            instance.m_state = service_instance::UNHEALTHY;
                            handleUnhealthyInstance(instance);
                        }
                    }
                }
            }
        }).detach();
    }

    void startAutoScaler() {
        std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(30s);

                for (const auto& [service_name, _] : service_definitions) {
                    autoScale(service_name);
                }
            }
        }).detach();
    }

    void printServiceMap() {
        std::stringstream map_output;
        map_output << "\n╔══════════════════════════════════════════════════════════════╗\n";
        map_output << "║                    Service Mesh Topology                     ║\n";
        map_output << "╠══════════════════════════════════════════════════════════════╣\n";

        std::lock_guard<std::mutex> lock(registry_mutex);

        for (const auto& [service_name, instances] : service_instances) {
            int healthy = 0, unhealthy = 0;
            for (const auto& instance : instances) {
                if (instance.m_state == service_instance::HEALTHY) healthy++;
                else if (instance.m_state == service_instance::UNHEALTHY) unhealthy++;
            }

            map_output << "║ " << std::left << std::setw(20) << service_name
                      << " │ Instances: " << std::setw(2) << instances.size()
                      << " │ Healthy: " << std::setw(2) << healthy
                      << " │ Unhealthy: " << std::setw(2) << unhealthy
                      << "    ║\n";

            // Show dependencies
            if (!service_definitions[service_name].dependencies.empty()) {
                map_output << "║   └─ Dependencies: ";
                for (const auto& dep : service_definitions[service_name].dependencies) {
                    map_output << dep << " ";
                }
                map_output << "\n";
            }
        }

        map_output << "╠══════════════════════════════════════════════════════════════╣\n";
        map_output << "║ Metrics:                                                      ║\n";
        map_output << "║   Total Requests: " << std::setw(43)
                  << total_requests.load() << " ║\n";
        map_output << "║   Success Rate: " << std::setw(44)
                  << std::fixed << std::setprecision(2)
                  << (100.0 * successful_requests / std::max(static_cast<uint64_t>(1), total_requests.load()))
                  << "% ║\n";
        map_output << "║   Circuit Breaker Trips: " << std::setw(36)
                  << circuit_breaker_trips.load() << " ║\n";
        map_output << "╚══════════════════════════════════════════════════════════════╝";

        m_logger->log(logger_module::log_level::info, map_output.str());
    }

public:
    void start() {
        m_logger->log(logger_module::log_level::info,
            "\n=== Microservices Orchestrator Starting ===");

        // The integrator is automatically started upon initialization

        // Start monitoring threads
        startHealthCheckMonitor();
        startAutoScaler();

        // Start dashboard
        std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(20s);
                printServiceMap();
            }
        }).detach();

        // Simulate traffic
        std::thread([this]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<> service_dist(0, service_definitions.size() - 1);

            std::vector<std::string> services;
            for (const auto& [name, _] : service_definitions) {
                services.push_back(name);
            }

            while (running) {
                core::message request("service.request");
                request.metadata.type = core::message_type::request;
                request.metadata.headers["service_name"] = services[service_dist(gen)];
                request.metadata.headers["request_id"] = "req-" + std::to_string(total_requests.load());
                request.metadata.headers["client_ip"] = "192.168.1." + std::to_string(gen() % 255);
                request.payload.set("message", std::string("Sample request payload"));

                integrator->get_message_bus()->publish(request);

                std::this_thread::sleep_for(std::chrono::milliseconds(10 + gen() % 100));
            }
        }).detach();

        std::cout << "Microservices Orchestrator is running. Press Enter to stop..." << std::endl;
        printServiceMap();

        std::cin.get();
        stop();
    }

    void stop() {
        running = false;
        integrator->shutdown();

        std::stringstream stats;
        stats << "\n=== Final Statistics ===\n";
        stats << "Total services managed: " << service_definitions.size() << "\n";
        stats << "Total instances:\n";
        for (const auto& [name, instances] : service_instances) {
            stats << "  " << name << ": " << instances.size() << "\n";
        }
        stats << "Total requests processed: " << total_requests << "\n";
        stats << "Success rate: "
              << std::fixed << std::setprecision(2)
              << (100.0 * successful_requests / std::max(static_cast<uint64_t>(1), total_requests.load()))
              << "%\n";
        stats << "========================";

        m_logger->log(logger_module::log_level::info, stats.str());
        m_logger->flush();
        m_logger->stop();
    }
};

int main(int argc, char* argv[]) {
    try {
        microservices_orchestrator orchestrator;
        orchestrator.start();

    } catch (const std::exception& e) {
        // Create a minimal logger for error reporting
        auto error_logger = std::make_shared<logger_module::logger>(true, 8192);
        error_logger->add_writer(std::make_unique<logger_module::console_writer>());
        error_logger->log(logger_module::log_level::error, "Error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}