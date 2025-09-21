# Messaging System Services CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# Service library target
add_library(messaging_services)

# Include directories
target_include_directories(messaging_services
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files for Phase 2
target_sources(messaging_services
    PRIVATE
        # Core messaging infrastructure
        src/kcenon/messaging/core/message_bus.cpp

        # Service implementations
        src/services/container/container_service.cpp
        src/services/network/network_service.cpp

        # Integration layer
        src/kcenon/messaging/integrations/system_integrator.cpp

    PUBLIC
        # Core headers
        include/kcenon/messaging/core/message_bus.h
        include/kcenon/messaging/core/message_types.h
        include/kcenon/messaging/core/config.h

        # Service interfaces
        include/kcenon/messaging/services/service_interface.h
        include/kcenon/messaging/services/container/container_service.h
        include/kcenon/messaging/services/network/network_service.h

        # Integration interfaces
        include/kcenon/messaging/integrations/system_integrator.h
        include/kcenon/messaging/integrations/service_container.h
)

# Link dependencies (conditionally based on availability)
if(TARGET ThreadSystem::Core)
    target_link_libraries(messaging_services PUBLIC ThreadSystem::Core)
    target_compile_definitions(messaging_services PUBLIC HAS_THREAD_SYSTEM_CORE)
endif()

if(TARGET LoggerSystem::logger)
    target_link_libraries(messaging_services PUBLIC LoggerSystem::logger)
    target_compile_definitions(messaging_services PUBLIC HAS_LOGGER_SYSTEM)
endif()

if(TARGET MonitoringSystem::monitoring)
    target_link_libraries(messaging_services PUBLIC MonitoringSystem::monitoring)
    target_compile_definitions(messaging_services PUBLIC HAS_MONITORING_SYSTEM)
endif()

# Compiler features
target_compile_features(messaging_services PUBLIC cxx_std_20)

# Install configuration
install(TARGETS messaging_services
    EXPORT messaging_services_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)