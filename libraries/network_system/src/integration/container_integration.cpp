/**
 * @file container_integration.cpp
 * @brief Implementation of container system integration
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include "network_system/integration/container_integration.h"
#include <unordered_map>
#include <mutex>
#include <cstring>

namespace network_system::integration {

#ifdef BUILD_WITH_CONTAINER_SYSTEM
// container_system_adapter implementation
container_system_adapter::container_system_adapter(
    std::shared_ptr<container_module::value_container> container
) : container_(container) {
}

std::vector<uint8_t> container_system_adapter::serialize(const std::any& data) const {
    if (!container_) {
        return {};
    }

    // Convert std::any to container format and serialize
    // This is a simplified implementation - actual implementation would
    // depend on container_system's API
    try {
        if (data.type() == typeid(std::string)) {
            auto str = std::any_cast<std::string>(data);
            return std::vector<uint8_t>(str.begin(), str.end());
        }
        // Add more type conversions as needed
    } catch (const std::bad_any_cast&) {
        // Handle conversion error
    }

    return {};
}

std::any container_system_adapter::deserialize(const std::vector<uint8_t>& bytes) const {
    if (!container_ || bytes.empty()) {
        return std::any{};
    }

    // Deserialize bytes to container format and convert to std::any
    // This is a simplified implementation
    std::string str(bytes.begin(), bytes.end());
    return std::any(str);
}

std::string container_system_adapter::type_name() const {
    return "container_system_adapter";
}

bool container_system_adapter::is_valid() const {
    return container_ != nullptr;
}

std::shared_ptr<container_module::value_container>
container_system_adapter::get_container() const {
    return container_;
}
#endif

// basic_container implementation
class basic_container::impl {
public:
    impl() = default;

    std::vector<uint8_t> serialize(const std::any& data) const {
        if (custom_serializer_) {
            return custom_serializer_(data);
        }

        // Default serialization for common types
        try {
            if (data.type() == typeid(std::string)) {
                auto str = std::any_cast<std::string>(data);
                return std::vector<uint8_t>(str.begin(), str.end());
            } else if (data.type() == typeid(int)) {
                auto val = std::any_cast<int>(data);
                std::vector<uint8_t> bytes(sizeof(int));
                std::memcpy(bytes.data(), &val, sizeof(int));
                return bytes;
            } else if (data.type() == typeid(double)) {
                auto val = std::any_cast<double>(data);
                std::vector<uint8_t> bytes(sizeof(double));
                std::memcpy(bytes.data(), &val, sizeof(double));
                return bytes;
            } else if (data.type() == typeid(bool)) {
                auto val = std::any_cast<bool>(data);
                return {static_cast<uint8_t>(val ? 1 : 0)};
            }
        } catch (const std::bad_any_cast&) {
            // Return empty vector on error
        }

        return {};
    }

    std::any deserialize(const std::vector<uint8_t>& bytes) const {
        if (custom_deserializer_) {
            return custom_deserializer_(bytes);
        }

        // Default deserialization - assume string for simplicity
        if (!bytes.empty()) {
            return std::any(std::string(bytes.begin(), bytes.end()));
        }

        return std::any{};
    }

    void set_serializer(
        std::function<std::vector<uint8_t>(const std::any&)> serializer
    ) {
        custom_serializer_ = serializer;
    }

    void set_deserializer(
        std::function<std::any(const std::vector<uint8_t>&)> deserializer
    ) {
        custom_deserializer_ = deserializer;
    }

private:
    std::function<std::vector<uint8_t>(const std::any&)> custom_serializer_;
    std::function<std::any(const std::vector<uint8_t>&)> custom_deserializer_;
};

basic_container::basic_container()
    : pimpl_(std::make_unique<impl>()) {
}

basic_container::~basic_container() = default;

std::vector<uint8_t> basic_container::serialize(const std::any& data) const {
    return pimpl_->serialize(data);
}

std::any basic_container::deserialize(const std::vector<uint8_t>& bytes) const {
    return pimpl_->deserialize(bytes);
}

std::string basic_container::type_name() const {
    return "basic_container";
}

bool basic_container::is_valid() const {
    return true;
}

void basic_container::set_serializer(
    std::function<std::vector<uint8_t>(const std::any&)> serializer
) {
    pimpl_->set_serializer(serializer);
}

void basic_container::set_deserializer(
    std::function<std::any(const std::vector<uint8_t>&)> deserializer
) {
    pimpl_->set_deserializer(deserializer);
}

// container_manager implementation
class container_manager::impl {
public:
    impl() = default;

    void register_container(
        const std::string& name,
        std::shared_ptr<container_interface> container
    ) {
        std::unique_lock<std::mutex> lock(mutex_);
        containers_[name] = container;
    }

    std::shared_ptr<container_interface> get_container(
        const std::string& name
    ) const {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = containers_.find(name);
        if (it != containers_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void set_default_container(
        std::shared_ptr<container_interface> container
    ) {
        std::unique_lock<std::mutex> lock(mutex_);
        default_container_ = container;
    }

    std::shared_ptr<container_interface> get_default_container() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!default_container_) {
            default_container_ = std::make_shared<basic_container>();
        }
        return default_container_;
    }

    std::vector<uint8_t> serialize(const std::any& data) {
        return get_default_container()->serialize(data);
    }

    std::any deserialize(const std::vector<uint8_t>& bytes) {
        return get_default_container()->deserialize(bytes);
    }

    std::vector<std::string> list_containers() const {
        std::unique_lock<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        names.reserve(containers_.size());
        for (const auto& [name, _] : containers_) {
            names.push_back(name);
        }
        return names;
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<container_interface>> containers_;
    std::shared_ptr<container_interface> default_container_;
};

container_manager& container_manager::instance() {
    static container_manager instance;
    return instance;
}

container_manager::container_manager()
    : pimpl_(std::make_unique<impl>()) {
}

container_manager::~container_manager() = default;

void container_manager::register_container(
    const std::string& name,
    std::shared_ptr<container_interface> container
) {
    pimpl_->register_container(name, container);
}

std::shared_ptr<container_interface> container_manager::get_container(
    const std::string& name
) const {
    return pimpl_->get_container(name);
}

void container_manager::set_default_container(
    std::shared_ptr<container_interface> container
) {
    pimpl_->set_default_container(container);
}

std::shared_ptr<container_interface> container_manager::get_default_container() {
    return pimpl_->get_default_container();
}

std::vector<uint8_t> container_manager::serialize(const std::any& data) {
    return pimpl_->serialize(data);
}

std::any container_manager::deserialize(const std::vector<uint8_t>& bytes) {
    return pimpl_->deserialize(bytes);
}

std::vector<std::string> container_manager::list_containers() const {
    return pimpl_->list_containers();
}

} // namespace network_system::integration