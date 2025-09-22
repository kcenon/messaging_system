/**
 * @file di_container_factory.h
 * @brief Factory for creating DI container instances
 */

#pragma once

#include <kcenon/logger/core/di/di_container_interface.h>
#include <memory>

namespace kcenon::logger::di {

/**
 * @brief Factory class for creating DI container instances
 */
class di_container_factory {
public:
    /**
     * @brief Container type enumeration
     */
    enum class container_type {
        automatic,
        basic,
        advanced
    };
    /**
     * @brief Create a default DI container instance
     * @return Shared pointer to DI container interface
     */
    static std::shared_ptr<di_container_interface> create_default() {
        return std::make_shared<basic_di_container>();
    }

    /**
     * @brief Create a DI container with specific configuration
     * @param config Configuration parameters for the container
     * @return Shared pointer to DI container interface
     */
    template<typename ConfigType>
    static std::shared_ptr<di_container_interface> create_with_config(const ConfigType& config) {
        (void)config; // Suppress unused parameter warning
        return create_default();
    }

    /**
     * @brief Get the global DI container instance
     * @return Reference to the global DI container
     */
    static di_container_interface& get_global_container() {
        static basic_di_container global_instance;
        return global_instance;
    }
};

} // namespace kcenon::logger::di