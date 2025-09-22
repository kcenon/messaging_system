#pragma once

/**
 * @file container_integration.h
 * @brief Container system integration interface for network_system
 *
 * This interface provides enhanced integration with container_system
 * for message serialization and deserialization.
 *
 * @author kcenon
 * @date 2025-09-20
 */

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <any>

#ifdef BUILD_WITH_CONTAINER_SYSTEM
#include "container.h"
#endif

namespace network_system::integration {

/**
 * @class container_interface
 * @brief Abstract interface for container operations
 *
 * This interface allows network_system to work with any container
 * implementation for message serialization.
 */
class container_interface {
public:
    virtual ~container_interface() = default;

    /**
     * @brief Serialize data to bytes
     * @param data The data to serialize
     * @return Serialized bytes
     */
    virtual std::vector<uint8_t> serialize(const std::any& data) const = 0;

    /**
     * @brief Deserialize bytes to data
     * @param bytes The bytes to deserialize
     * @return Deserialized data
     */
    virtual std::any deserialize(const std::vector<uint8_t>& bytes) const = 0;

    /**
     * @brief Get container type name
     * @return Type name string
     */
    virtual std::string type_name() const = 0;

    /**
     * @brief Check if container is valid
     * @return true if valid, false otherwise
     */
    virtual bool is_valid() const = 0;
};

#ifdef BUILD_WITH_CONTAINER_SYSTEM
/**
 * @class container_system_adapter
 * @brief Adapter for container_system integration
 *
 * This adapter wraps container_system functionality to work with
 * the network_system's container interface.
 */
class container_system_adapter : public container_interface {
public:
    /**
     * @brief Construct with a value_container
     * @param container The container to wrap
     */
    explicit container_system_adapter(
        std::shared_ptr<container_module::value_container> container
    );

    // container_interface implementation
    std::vector<uint8_t> serialize(const std::any& data) const override;
    std::any deserialize(const std::vector<uint8_t>& bytes) const override;
    std::string type_name() const override;
    bool is_valid() const override;

    /**
     * @brief Get the wrapped container
     * @return The underlying value_container
     */
    std::shared_ptr<container_module::value_container> get_container() const;

private:
    std::shared_ptr<container_module::value_container> container_;
};
#endif

/**
 * @class basic_container
 * @brief Basic container implementation for standalone use
 *
 * This provides a simple container implementation for when
 * container_system is not available.
 */
class basic_container : public container_interface {
public:
    basic_container();
    ~basic_container();

    // container_interface implementation
    std::vector<uint8_t> serialize(const std::any& data) const override;
    std::any deserialize(const std::vector<uint8_t>& bytes) const override;
    std::string type_name() const override;
    bool is_valid() const override;

    /**
     * @brief Set custom serializer
     * @param serializer Custom serialization function
     */
    void set_serializer(
        std::function<std::vector<uint8_t>(const std::any&)> serializer
    );

    /**
     * @brief Set custom deserializer
     * @param deserializer Custom deserialization function
     */
    void set_deserializer(
        std::function<std::any(const std::vector<uint8_t>&)> deserializer
    );

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

/**
 * @class container_manager
 * @brief Manager for container system integration
 *
 * This class manages the integration between network_system and
 * container implementations.
 */
class container_manager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static container_manager& instance();

    /**
     * @brief Register a container implementation
     * @param name Container name
     * @param container Container implementation
     */
    void register_container(
        const std::string& name,
        std::shared_ptr<container_interface> container
    );

    /**
     * @brief Get a registered container
     * @param name Container name
     * @return Container implementation or nullptr if not found
     */
    std::shared_ptr<container_interface> get_container(
        const std::string& name
    ) const;

    /**
     * @brief Set default container
     * @param container Default container to use
     */
    void set_default_container(
        std::shared_ptr<container_interface> container
    );

    /**
     * @brief Get default container
     * @return Default container (creates basic container if none set)
     */
    std::shared_ptr<container_interface> get_default_container();

    /**
     * @brief Serialize using default container
     * @param data Data to serialize
     * @return Serialized bytes
     */
    std::vector<uint8_t> serialize(const std::any& data);

    /**
     * @brief Deserialize using default container
     * @param bytes Bytes to deserialize
     * @return Deserialized data
     */
    std::any deserialize(const std::vector<uint8_t>& bytes);

    /**
     * @brief Get list of registered container names
     * @return Vector of container names
     */
    std::vector<std::string> list_containers() const;

private:
    container_manager();
    ~container_manager();

    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace network_system::integration