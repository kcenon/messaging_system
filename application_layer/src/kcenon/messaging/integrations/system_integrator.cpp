#include "kcenon/messaging/integrations/system_integrator.h"
#include "kcenon/messaging/services/container/container_service.h"
#include "kcenon/messaging/services/network/network_service.h"
#include "kcenon/messaging/integrations/external_system_adapter.h"

namespace kcenon::messaging::integrations {

    // messaging_system_orchestrator implementation
    messaging_system_orchestrator::messaging_system_orchestrator(const config::messaging_config& config)
        : config_(config) {

        // Create message bus with configuration
        message_bus_ = std::make_unique<core::message_bus>(config.message_bus);

        // Create external system manager
        external_systems_ = std::make_unique<external_system_manager>();
    }

    messaging_system_orchestrator::~messaging_system_orchestrator() {
        shutdown();
    }

    bool messaging_system_orchestrator::initialize() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (initialized_) {
            return true;
        }

        try {
            // Initialize message bus
            if (!message_bus_->initialize()) {
                return false;
            }

            // Register core services
            register_core_services();

            // Register messaging services
            register_messaging_services();

            // Setup external system integrations
            setup_external_integrations();

            // Initialize all adapters
            initialize_adapters();

            initialized_ = true;
            return true;

        } catch (const std::exception&) {
            return false;
        }
    }

    void messaging_system_orchestrator::shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_) {
            return;
        }

        // Shutdown adapters first
        shutdown_adapters();

        // Shutdown message bus
        if (message_bus_) {
            message_bus_->shutdown();
        }

        // Clear container
        container_.clear();

        initialized_ = false;
    }

    void messaging_system_orchestrator::register_core_services() {
        // Register message bus (convert unique_ptr to shared_ptr)
        std::shared_ptr<core::message_bus> message_bus_shared(message_bus_.get(), [](core::message_bus*){});
        container_.register_singleton<core::message_bus>("message_bus", message_bus_shared);

        // Register configuration
        auto config_ptr = std::make_shared<config::messaging_config>(config_);
        container_.register_singleton<config::messaging_config>("config", config_ptr);
    }

    void messaging_system_orchestrator::register_messaging_services() {
        // Register container service
        container_.register_service<services::container::container_service>(
            "container_service",
            [this]() {
                return std::make_shared<services::container::container_service>(config_.container);
            }
        );

        // Register network service
        container_.register_service<services::network::network_service>(
            "network_service",
            [this]() {
                return std::make_shared<services::network::network_service>(config_.network);
            }
        );

        // Register container adapter
        container_.register_service<services::container::container_service_adapter>(
            "container_adapter",
            [this]() {
                auto service = container_.resolve<services::container::container_service>("container_service");
                return std::make_shared<services::container::container_service_adapter>(service);
            }
        );

        // Register network adapter
        container_.register_service<services::network::network_service_adapter>(
            "network_adapter",
            [this]() {
                auto service = container_.resolve<services::network::network_service>("network_service");
                return std::make_shared<services::network::network_service_adapter>(service);
            }
        );
    }

    void messaging_system_orchestrator::register_service_adapter(
        const std::string& name,
        std::shared_ptr<services::service_adapter> adapter) {

        std::lock_guard<std::mutex> lock(mutex_);
        adapters_[name] = adapter;

        // If system is already initialized, register with bus immediately
        if (initialized_ && message_bus_) {
            adapter->register_with_bus(message_bus_.get());
        }
    }

    std::vector<std::string> messaging_system_orchestrator::get_registered_adapters() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        for (const auto& [name, adapter] : adapters_) {
            names.push_back(name);
        }
        return names;
    }

    void messaging_system_orchestrator::setup_external_integrations() {
        // Setup external system adapters

        // Database system adapter
        if (config_.enable_database_system) {
            auto db_adapter = create_database_adapter("sqlite:///messaging.db");
            external_systems_->register_adapter("database", std::move(db_adapter));
        }

        // Thread system adapter
        if (config_.enable_thread_system) {
            auto thread_adapter = create_thread_system_adapter();
            external_systems_->register_adapter("thread_system", std::move(thread_adapter));
        }

        // Connect all registered adapters
        external_systems_->connect_all();

#ifdef HAS_LOGGER_SYSTEM
        if (config_.enable_logger_system) {
            // TODO: Integrate with external logging system
        }
#endif

#ifdef HAS_MONITORING_SYSTEM
        if (config_.enable_monitoring_system) {
            // TODO: Integrate with external monitoring system
        }
#endif
    }

    void messaging_system_orchestrator::initialize_adapters() {
        // Initialize and register default adapters
        auto container_adapter = container_.resolve<services::container::container_service_adapter>("container_adapter");
        if (container_adapter) {
            register_service_adapter("container", container_adapter);
        }

        auto network_adapter = container_.resolve<services::network::network_service_adapter>("network_adapter");
        if (network_adapter) {
            register_service_adapter("network", network_adapter);
        }

        // Register all adapters with message bus
        for (const auto& [name, adapter] : adapters_) {
            adapter->register_with_bus(message_bus_.get());
            adapter->initialize();
        }
    }

    void messaging_system_orchestrator::shutdown_adapters() {
        for (const auto& [name, adapter] : adapters_) {
            adapter->shutdown();
        }
        adapters_.clear();
    }

    // system_integrator implementation
    system_integrator::system_integrator(const config::messaging_config& config)
        : orchestrator_(config) {
    }

    system_integrator::~system_integrator() {
        shutdown();
    }

    bool system_integrator::initialize() {
        if (initialized_) {
            return true;
        }

        bool success = orchestrator_.initialize();
        if (success) {
            initialized_ = true;
        }
        return success;
    }

    void system_integrator::shutdown() {
        if (initialized_) {
            orchestrator_.shutdown();
            initialized_ = false;
        }
    }

    bool system_integrator::is_running() const {
        return initialized_ && orchestrator_.is_running();
    }

    bool system_integrator::publish(const std::string& topic, const core::message_payload& payload,
                                   const std::string& sender) {
        auto bus = get_message_bus();
        return bus ? bus->publish(topic, payload, sender) : false;
    }

    void system_integrator::subscribe(const std::string& topic, core::message_handler handler) {
        auto bus = get_message_bus();
        if (bus) {
            bus->subscribe(topic, std::move(handler));
        }
    }

    system_integrator::system_health system_integrator::check_system_health() const {
        system_health health;
        health.last_check = std::chrono::system_clock::now();

        auto bus = get_message_bus();
        health.message_bus_healthy = bus && bus->is_running();

        if (health.message_bus_healthy) {
            auto stats = bus->get_statistics();
            health.total_messages_processed = stats.messages_processed;
        }

        // Check service health
        auto adapters = orchestrator_.get_registered_adapters();
        health.active_services = adapters.size();
        health.all_services_healthy = !adapters.empty();  // Simplified check

        return health;
    }

    std::unique_ptr<system_integrator> system_integrator::create_default() {
        config::config_builder builder;
        auto config = builder
            .set_environment("development")
            .set_worker_threads(4)
            .set_queue_size(10000)
            .enable_compression(true)
            .build();

        return std::make_unique<system_integrator>(config);
    }

    std::unique_ptr<system_integrator> system_integrator::create_for_environment(const std::string& environment) {
        config::config_builder builder;
        auto config = builder.set_environment(environment).build();
        return std::make_unique<system_integrator>(config);
    }

} // namespace kcenon::messaging::integrations