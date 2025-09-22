#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace kcenon::thread {

/**
 * @brief Modern service container for dependency injection
 *
 * This container supports:
 * - Registration of interfaces with concrete implementations
 * - Factory functions for lazy instantiation
 * - Singleton and transient lifetime management
 * - Thread-safe operations
 */
class service_container {
public:
    /**
     * @brief Service lifetime scope
     */
    enum class lifetime {
        singleton,  // Single instance shared across all requests
        transient   // New instance for each request
    };

    /**
     * @brief Register a service with singleton lifetime
     * @tparam Interface The interface type
     * @tparam Implementation The concrete implementation type
     * @param instance Shared pointer to the implementation
     */
    template<typename Interface, typename Implementation>
    void register_singleton(std::shared_ptr<Implementation> instance) {
        static_assert(std::is_base_of_v<Interface, Implementation>,
                      "Implementation must derive from Interface");
        
        std::lock_guard<std::mutex> lock(mutex_);
        services_[std::type_index(typeid(Interface))] = 
            service_entry{lifetime::singleton, instance, nullptr};
    }

    /**
     * @brief Register a service with factory function
     * @tparam Interface The interface type
     * @param factory Factory function that creates the service
     * @param lt Service lifetime (default: transient)
     */
    template<typename Interface>
    void register_factory(std::function<std::shared_ptr<Interface>()> factory,
                         lifetime lt = lifetime::transient) {
        std::lock_guard<std::mutex> lock(mutex_);
        services_[std::type_index(typeid(Interface))] = 
            service_entry{lt, nullptr, 
                [factory]() -> std::shared_ptr<void> { 
                    return factory(); 
                }};
    }

    /**
     * @brief Register a transient service
     * @tparam Interface The interface type
     * @tparam Implementation The concrete implementation type
     * @tparam Args Constructor argument types
     */
    template<typename Interface, typename Implementation, typename... Args>
    void register_transient() {
        static_assert(std::is_base_of_v<Interface, Implementation>,
                      "Implementation must derive from Interface");
        
        register_factory<Interface>(
            []() { return std::make_shared<Implementation>(); },
            lifetime::transient);
    }

    /**
     * @brief Resolve a service
     * @tparam Interface The interface type to resolve
     * @return Shared pointer to the service, or nullptr if not found
     */
    template<typename Interface>
    std::shared_ptr<Interface> resolve() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = services_.find(std::type_index(typeid(Interface)));
        if (it == services_.end()) {
            return nullptr;
        }

        auto& entry = it->second;
        
        // For singletons with existing instance
        if (entry.lifetime_scope == lifetime::singleton && entry.instance) {
            return std::static_pointer_cast<Interface>(entry.instance);
        }
        
        // Create new instance using factory
        if (entry.factory) {
            auto instance = entry.factory();
            
            // Store singleton instances
            if (entry.lifetime_scope == lifetime::singleton) {
                entry.instance = instance;
            }
            
            return std::static_pointer_cast<Interface>(instance);
        }
        
        return nullptr;
    }

    /**
     * @brief Check if a service is registered
     * @tparam Interface The interface type to check
     * @return true if the service is registered
     */
    template<typename Interface>
    bool is_registered() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return services_.find(std::type_index(typeid(Interface))) != services_.end();
    }

    /**
     * @brief Clear all registered services
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        services_.clear();
    }

    /**
     * @brief Get the global service container instance
     * @return Reference to the global container
     */
    static service_container& global() {
        static service_container instance;
        return instance;
    }

private:
    struct service_entry {
        lifetime lifetime_scope;
        mutable std::shared_ptr<void> instance;
        std::function<std::shared_ptr<void>()> factory;
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::type_index, service_entry> services_;
};

/**
 * @brief RAII helper for scoped service registration
 */
template<typename Interface>
class scoped_service {
public:
    scoped_service(std::shared_ptr<Interface> service) {
        service_container::global().register_singleton<Interface>(service);
    }

    ~scoped_service() {
        // Note: In a real implementation, you might want to support unregistration
        // For now, services remain registered for the application lifetime
    }
};

} // namespace kcenon::thread