/*
 * BSD 3-Clause License
 * Copyright (c) 2025, DongCheol Shin
 */

#pragma once

#include <any>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>

namespace kcenon::thread {

/**
 * @brief Lightweight service registry for dependency lookup.
 */
class service_registry {
private:
    static inline std::unordered_map<std::type_index, std::any> services_{};
    static inline std::shared_mutex mutex_{};

public:
    template <typename Interface>
    static auto register_service(std::shared_ptr<Interface> service) -> void {
        std::unique_lock lock(mutex_);
        services_[std::type_index(typeid(Interface))] = std::move(service);
    }

    template <typename Interface>
    static auto get_service() -> std::shared_ptr<Interface> {
        std::shared_lock lock(mutex_);
        auto it = services_.find(std::type_index(typeid(Interface)));
        if (it != services_.end()) {
            return std::any_cast<std::shared_ptr<Interface>>(it->second);
        }
        return nullptr;
    }

    static auto clear_services() -> void {
        std::unique_lock lock(mutex_);
        services_.clear();
    }

    static auto get_service_count() -> std::size_t {
        std::shared_lock lock(mutex_);
        return services_.size();
    }
};

} // namespace kcenon::thread

