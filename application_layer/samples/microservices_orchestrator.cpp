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
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <random>
#include <atomic>
#include <algorithm>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

// Service definition
struct ServiceDefinition {
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
struct ServiceInstance {
    std::string instance_id;
    std::string service_name;
    std::string host;
    int port;
    std::string version;

    enum State {
        STARTING,
        HEALTHY,
        UNHEALTHY,
        DRAINING,
        STOPPED
    } state;

    std::chrono::steady_clock::time_point last_health_check;
    int consecutive_failures{0};
    double cpu_usage{0.0};
    double memory_usage{0.0};
    int active_connections{0};
    double response_time_ms{0.0};
};

// Circuit breaker for service calls
class CircuitBreaker {
public:
    enum State {
        CLOSED,     // Normal operation
        OPEN,       // Failures exceeded threshold, blocking calls
        HALF_OPEN   // Testing if service recovered
    };

private:
    State state{CLOSED};
    int failure_count{0};
    int success_count{0};
    int failure_threshold{5};
    int success_threshold{3};
    std::chrono::steady_clock::time_point last_failure_time;
    std::chrono::seconds timeout{30s};
    std::mutex mutex;

public:
    bool canAttempt() {
        std::lock_guard<std::mutex> lock(mutex);

        if (state == CLOSED) {
            return true;
        }

        if (state == OPEN) {
            auto now = std::chrono::steady_clock::now();
            if (now - last_failure_time > timeout) {
                state = HALF_OPEN;
                return true;
            }
            return false;
        }

        // HALF_OPEN
        return true;
    }

    void recordSuccess() {
        std::lock_guard<std::mutex> lock(mutex);

        if (state == HALF_OPEN) {
            success_count++;
            if (success_count >= success_threshold) {
                state = CLOSED;
                failure_count = 0;
                success_count = 0;
            }
        } else if (state == CLOSED) {
            failure_count = 0;
        }
    }

    void recordFailure() {
        std::lock_guard<std::mutex> lock(mutex);

        failure_count++;
        last_failure_time = std::chrono::steady_clock::now();

        if (state == HALF_OPEN || failure_count >= failure_threshold) {
            state = OPEN;
            success_count = 0;
        }
    }

    State getState() const {
        std::lock_guard<std::mutex> lock(mutex);
        return state;
    }
};

// Load balancer strategies
class LoadBalancer {
public:
    enum Strategy {
        ROUND_ROBIN,
        LEAST_CONNECTIONS,
        RANDOM,
        WEIGHTED_RESPONSE_TIME,
        IP_HASH
    };

private:
    Strategy strategy;
    std::atomic<size_t> round_robin_counter{0};
    std::mt19937 random_gen{std::random_device{}()};

public:
    LoadBalancer(Strategy s = ROUND_ROBIN) : strategy(s) {}

    ServiceInstance* selectInstance(
        std::vector<ServiceInstance>& instances,
        const std::string& client_ip = ""
    ) {
        // Filter healthy instances
        std::vector<ServiceInstance*> healthy;
        for (auto& instance : instances) {
            if (instance.state == ServiceInstance::HEALTHY) {
                healthy.push_back(&instance);
            }
        }

        if (healthy.empty()) {
            return nullptr;
        }

        switch (strategy) {
            case ROUND_ROBIN:
                return healthy[round_robin_counter++ % healthy.size()];

            case LEAST_CONNECTIONS:
                return *std::min_element(healthy.begin(), healthy.end(),
                    [](ServiceInstance* a, ServiceInstance* b) {
                        return a->active_connections < b->active_connections;
                    });

            case RANDOM: {
                std::uniform_int_distribution<> dist(0, healthy.size() - 1);
                return healthy[dist(random_gen)];
            }

            case WEIGHTED_RESPONSE_TIME:
                return *std::min_element(healthy.begin(), healthy.end(),
                    [](ServiceInstance* a, ServiceInstance* b) {
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

class MicroservicesOrchestrator {
private:
    std::unique_ptr<integrations::system_integrator> integrator;
    std::unique_ptr<services::container_service> container_svc;
    std::unique_ptr<services::database_service> database_svc;
    std::unique_ptr<services::network_service> network_svc;

    // Service registry
    std::map<std::string, ServiceDefinition> service_definitions;
    std::map<std::string, std::vector<ServiceInstance>> service_instances;
    std::map<std::string, CircuitBreaker> circuit_breakers;
    std::map<std::string, LoadBalancer> load_balancers;
    std::mutex registry_mutex;

    // Metrics
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> successful_requests{0};
    std::atomic<uint64_t> failed_requests{0};
    std::atomic<uint64_t> circuit_breaker_trips{0};

    // System state
    std::atomic<bool> running{true};

public:
    MicroservicesOrchestrator() {
        // Configure for microservices workload
        config::config_builder builder;
        auto config = builder
            .set_environment("microservices")
            .set_worker_threads(std::thread::hardware_concurrency() * 2)
            .set_queue_size(500000)
            .set_max_message_size(1024 * 1024)  // 1MB for service payloads
            .enable_persistence(true)
            .enable_monitoring(true)
            .enable_compression(true)
            .set_timeout(10000)  // 10 second timeout
            .build();

        integrator = std::make_unique<integrations::system_integrator>(config);
        container_svc = std::make_unique<services::container_service>();
        database_svc = std::make_unique<services::database_service>();
        network_svc = std::make_unique<services::network_service>();

        setupMessageHandlers();
        initializeServices();
    }

    void setupMessageHandlers() {
        auto& bus = integrator->get_message_bus();

        // Service registration
        bus.subscribe("service.register", [this](const core::message& msg) {
            handleServiceRegistration(msg);
        });

        // Service discovery
        bus.subscribe("service.discover", [this](const core::message& msg) {
            handleServiceDiscovery(msg);
        });

        // Service health check
        bus.subscribe("service.health", [this](const core::message& msg) {
            handleHealthCheck(msg);
        });

        // Service request routing
        bus.subscribe("service.request", [this](const core::message& msg) {
            handleServiceRequest(msg);
        });

        // Service scaling
        bus.subscribe("service.scale", [this](const core::message& msg) {
            handleServiceScaling(msg);
        });

        // Service deployment
        bus.subscribe("service.deploy", [this](const core::message& msg) {
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
                deployServiceInstance(name);
            }
        }

        std::cout << "Initialized " << service_definitions.size()
                  << " service definitions" << std::endl;
    }

    void defineService(
        const std::string& name,
        const std::string& version,
        const std::vector<std::string>& dependencies,
        int min_instances,
        int max_instances
    ) {
        ServiceDefinition def;
        def.service_name = name;
        def.version = version;
        def.dependencies = dependencies;
        def.min_instances = min_instances;
        def.max_instances = max_instances;

        service_definitions[name] = def;
        circuit_breakers[name] = CircuitBreaker();
        load_balancers[name] = LoadBalancer(LoadBalancer::LEAST_CONNECTIONS);
    }

    void deployServiceInstance(const std::string& service_name) {
        static int port_counter = 8000;

        ServiceInstance instance;
        instance.instance_id = generateInstanceId(service_name);
        instance.service_name = service_name;
        instance.host = "10.0.0." + std::to_string(1 + (port_counter % 254));
        instance.port = port_counter++;
        instance.version = service_definitions[service_name].version;
        instance.state = ServiceInstance::STARTING;
        instance.last_health_check = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            service_instances[service_name].push_back(instance);
        }

        // Simulate startup
        std::thread([this, instance]() {
            std::this_thread::sleep_for(2s);
            updateInstanceState(instance.instance_id, ServiceInstance::HEALTHY);

            std::cout << "Started " << instance.service_name
                      << " instance " << instance.instance_id
                      << " at " << instance.host << ":" << instance.port << std::endl;
        }).detach();
    }

    void handleServiceRegistration(const core::message& msg) {
        auto service_name = msg.get_header("service_name");
        auto instance_id = msg.get_header("instance_id");
        auto host = msg.get_header("host");
        auto port = std::stoi(msg.get_header("port"));

        ServiceInstance instance;
        instance.instance_id = instance_id;
        instance.service_name = service_name;
        instance.host = host;
        instance.port = port;
        instance.state = ServiceInstance::HEALTHY;
        instance.last_health_check = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            service_instances[service_name].push_back(instance);
        }

        std::cout << "Registered service instance: " << service_name
                  << " (" << instance_id << ") at " << host << ":" << port << std::endl;

        // Notify dependent services
        notifyDependents(service_name);
    }

    void handleServiceDiscovery(const core::message& msg) {
        auto service_name = msg.get_header("service_name");
        auto client_id = msg.get_header("client_id");

        std::lock_guard<std::mutex> lock(registry_mutex);

        if (auto it = service_instances.find(service_name); it != service_instances.end()) {
            // Select instance using load balancer
            auto* instance = load_balancers[service_name].selectInstance(it->second);

            if (instance) {
                core::message response;
                response.set_type("service.discovered");
                response.set_header("service_name", service_name);
                response.set_header("instance_id", instance->instance_id);
                response.set_header("host", instance->host);
                response.set_header("port", std::to_string(instance->port));
                response.set_header("version", instance->version);

                integrator->get_message_bus().publish(response);

                std::cout << "Service discovery: " << client_id
                          << " -> " << instance->instance_id << std::endl;
            } else {
                // No healthy instances available
                sendServiceUnavailable(service_name, client_id);
            }
        } else {
            sendServiceNotFound(service_name, client_id);
        }
    }

    void handleHealthCheck(const core::message& msg) {
        auto instance_id = msg.get_header("instance_id");
        auto status = msg.get_header("status");
        auto cpu = std::stod(msg.get_header("cpu_usage"));
        auto memory = std::stod(msg.get_header("memory_usage"));
        auto connections = std::stoi(msg.get_header("active_connections"));
        auto response_time = std::stod(msg.get_header("response_time_ms"));

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
                        instance.state = ServiceInstance::HEALTHY;
                        instance.consecutive_failures = 0;
                    } else {
                        instance.consecutive_failures++;
                        if (instance.consecutive_failures >= 3) {
                            instance.state = ServiceInstance::UNHEALTHY;
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

        auto service_name = msg.get_header("service_name");
        auto client_ip = msg.get_header("client_ip");
        auto request_id = msg.get_header("request_id");

        // Check circuit breaker
        if (!circuit_breakers[service_name].canAttempt()) {
            circuit_breaker_trips++;
            sendCircuitBreakerOpen(service_name, request_id);
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
        auto service_name = msg.get_header("service_name");
        auto action = msg.get_header("action");
        auto count = std::stoi(msg.get_header("count"));

        if (action == "scale_up") {
            scaleUp(service_name, count);
        } else if (action == "scale_down") {
            scaleDown(service_name, count);
        } else if (action == "auto") {
            autoScale(service_name);
        }
    }

    void handleServiceDeployment(const core::message& msg) {
        auto service_name = msg.get_header("service_name");
        auto version = msg.get_header("version");
        auto strategy = msg.get_header("strategy");  // rolling, blue_green, canary

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
            deployServiceInstance(service_name);
        }

        std::cout << "Scaling up " << service_name << " by " << count << " instances" << std::endl;
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
            instances[i].state = ServiceInstance::DRAINING;
        }

        // Remove drained instances after grace period
        std::thread([this, service_name, to_remove]() {
            std::this_thread::sleep_for(30s);  // Grace period

            std::lock_guard<std::mutex> lock(registry_mutex);
            auto& instances = service_instances[service_name];
            instances.erase(
                std::remove_if(instances.begin(), instances.end(),
                    [](const ServiceInstance& i) {
                        return i.state == ServiceInstance::DRAINING;
                    }),
                instances.end()
            );

            std::cout << "Scaled down " << service_name
                      << " by " << to_remove << " instances" << std::endl;
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
            if (instance.state == ServiceInstance::HEALTHY) {
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
        std::cout << "Starting rolling update for " << service_name
                  << " to version " << new_version << std::endl;

        std::thread([this, service_name, new_version]() {
            std::lock_guard<std::mutex> lock(registry_mutex);
            auto& instances = service_instances[service_name];

            for (auto& instance : instances) {
                // Update one instance at a time
                instance.state = ServiceInstance::DRAINING;
                std::this_thread::sleep_for(10s);  // Drain connections

                instance.version = new_version;
                instance.state = ServiceInstance::STARTING;
                std::this_thread::sleep_for(5s);  // Startup time

                instance.state = ServiceInstance::HEALTHY;
                std::cout << "Updated " << instance.instance_id
                          << " to version " << new_version << std::endl;
            }

            service_definitions[service_name].version = new_version;
            std::cout << "Rolling update complete for " << service_name << std::endl;
        }).detach();
    }

    void performBlueGreenDeployment(const std::string& service_name, const std::string& new_version) {
        std::cout << "Starting blue-green deployment for " << service_name << std::endl;

        // Deploy green environment
        int instance_count = service_instances[service_name].size();
        for (int i = 0; i < instance_count; i++) {
            deployServiceInstance(service_name + "-green");
        }

        // Switch traffic after validation
        std::thread([this, service_name, new_version]() {
            std::this_thread::sleep_for(30s);  // Validation period

            // Swap blue and green
            std::lock_guard<std::mutex> lock(registry_mutex);
            service_instances[service_name] = service_instances[service_name + "-green"];
            service_instances.erase(service_name + "-green");

            std::cout << "Blue-green deployment complete for " << service_name << std::endl;
        }).detach();
    }

    void performCanaryDeployment(const std::string& service_name, const std::string& new_version) {
        std::cout << "Starting canary deployment for " << service_name << std::endl;

        // Deploy canary instance (10% of traffic)
        deployServiceInstance(service_name);

        // Monitor canary metrics
        std::thread([this, service_name, new_version]() {
            std::this_thread::sleep_for(60s);  // Monitoring period

            // If canary is healthy, continue rollout
            autoScale(service_name);  // This would check canary metrics

            std::cout << "Canary deployment validated for " << service_name << std::endl;
        }).detach();
    }

    bool forwardRequest(ServiceInstance* instance, const core::message& msg) {
        // Simulate request forwarding
        core::message forward;
        forward.set_type("request.forward");
        forward.set_header("instance_id", instance->instance_id);
        forward.set_header("host", instance->host);
        forward.set_header("port", std::to_string(instance->port));
        forward.set_payload(msg.get_payload());

        return network_svc->send_to_host(instance->host, instance->port, forward);
    }

    void handleUnhealthyInstance(const ServiceInstance& instance) {
        std::cout << "Instance " << instance.instance_id
                  << " marked unhealthy" << std::endl;

        // Replace unhealthy instance
        deployServiceInstance(instance.service_name);

        // Alert operations team
        core::message alert;
        alert.set_type("ops.alert");
        alert.set_header("severity", "warning");
        alert.set_header("service", instance.service_name);
        alert.set_header("instance", instance.instance_id);
        alert.set_payload("Service instance unhealthy - replacement initiated");

        integrator->get_message_bus().publish(alert);
    }

    void notifyDependents(const std::string& service_name) {
        for (const auto& [name, definition] : service_definitions) {
            if (std::find(definition.dependencies.begin(),
                         definition.dependencies.end(),
                         service_name) != definition.dependencies.end()) {

                core::message notify;
                notify.set_type("dependency.updated");
                notify.set_header("service", name);
                notify.set_header("dependency", service_name);

                integrator->get_message_bus().publish(notify);
            }
        }
    }

    void sendServiceUnavailable(const std::string& service_name, const std::string& request_id) {
        core::message response;
        response.set_type("service.unavailable");
        response.set_header("service_name", service_name);
        response.set_header("request_id", request_id);
        response.set_header("error", "No healthy instances available");

        integrator->get_message_bus().publish(response);
    }

    void sendServiceNotFound(const std::string& service_name, const std::string& client_id) {
        core::message response;
        response.set_type("service.not_found");
        response.set_header("service_name", service_name);
        response.set_header("client_id", client_id);
        response.set_header("error", "Service not registered");

        integrator->get_message_bus().publish(response);
    }

    void sendCircuitBreakerOpen(const std::string& service_name, const std::string& request_id) {
        core::message response;
        response.set_type("circuit_breaker.open");
        response.set_header("service_name", service_name);
        response.set_header("request_id", request_id);
        response.set_header("error", "Circuit breaker is open");

        integrator->get_message_bus().publish(response);
    }

    void updateInstanceState(const std::string& instance_id, ServiceInstance::State new_state) {
        std::lock_guard<std::mutex> lock(registry_mutex);

        for (auto& [service_name, instances] : service_instances) {
            for (auto& instance : instances) {
                if (instance.instance_id == instance_id) {
                    instance.state = new_state;
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

                        if (elapsed > 30s && instance.state == ServiceInstance::HEALTHY) {
                            instance.state = ServiceInstance::UNHEALTHY;
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
        std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    Service Mesh Topology                     ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;

        std::lock_guard<std::mutex> lock(registry_mutex);

        for (const auto& [service_name, instances] : service_instances) {
            int healthy = 0, unhealthy = 0;
            for (const auto& instance : instances) {
                if (instance.state == ServiceInstance::HEALTHY) healthy++;
                else if (instance.state == ServiceInstance::UNHEALTHY) unhealthy++;
            }

            std::cout << "║ " << std::left << std::setw(20) << service_name
                      << " │ Instances: " << std::setw(2) << instances.size()
                      << " │ Healthy: " << std::setw(2) << healthy
                      << " │ Unhealthy: " << std::setw(2) << unhealthy
                      << "    ║" << std::endl;

            // Show dependencies
            if (!service_definitions[service_name].dependencies.empty()) {
                std::cout << "║   └─ Dependencies: ";
                for (const auto& dep : service_definitions[service_name].dependencies) {
                    std::cout << dep << " ";
                }
                std::cout << std::endl;
            }
        }

        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ Metrics:                                                      ║" << std::endl;
        std::cout << "║   Total Requests: " << std::setw(43)
                  << total_requests.load() << " ║" << std::endl;
        std::cout << "║   Success Rate: " << std::setw(44)
                  << std::fixed << std::setprecision(2)
                  << (100.0 * successful_requests / std::max(1UL, total_requests.load()))
                  << "% ║" << std::endl;
        std::cout << "║   Circuit Breaker Trips: " << std::setw(36)
                  << circuit_breaker_trips.load() << " ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    }

public:
    void start() {
        std::cout << "\n=== Microservices Orchestrator Starting ===\n" << std::endl;

        integrator->start();

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
                core::message request;
                request.set_type("service.request");
                request.set_header("service_name", services[service_dist(gen)]);
                request.set_header("request_id", "req-" + std::to_string(total_requests.load()));
                request.set_header("client_ip", "192.168.1." + std::to_string(gen() % 255));
                request.set_payload("Sample request payload");

                integrator->get_message_bus().publish(request);

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
        integrator->stop();

        std::cout << "\n=== Final Statistics ===" << std::endl;
        std::cout << "Total services managed: " << service_definitions.size() << std::endl;
        std::cout << "Total instances: " << std::endl;
        for (const auto& [name, instances] : service_instances) {
            std::cout << "  " << name << ": " << instances.size() << std::endl;
        }
        std::cout << "Total requests processed: " << total_requests << std::endl;
        std::cout << "Success rate: "
                  << (100.0 * successful_requests / std::max(1UL, total_requests.load()))
                  << "%" << std::endl;
        std::cout << "========================\n" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        MicroservicesOrchestrator orchestrator;
        orchestrator.start();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}