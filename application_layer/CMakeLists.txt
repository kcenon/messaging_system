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
        include/kcenon/messaging/core/production_optimizations.h

        # Routing headers
        include/kcenon/messaging/routing/advanced_router.h
        include/kcenon/messaging/routing/message_transformer.h

        # Cluster headers
        include/kcenon/messaging/cluster/distributed_messaging.h

        # Persistence headers
        include/kcenon/messaging/persistence/message_persistence.h

        # Security headers
        include/kcenon/messaging/security/message_security.h

        # Management headers
        include/kcenon/messaging/management/system_dashboard.h

        # Service interfaces
        include/kcenon/messaging/services/service_interface.h
        include/kcenon/messaging/services/container/container_service.h
        include/kcenon/messaging/services/network/network_service.h

        # Integration interfaces
        include/kcenon/messaging/integrations/system_integrator.h
        include/kcenon/messaging/integrations/service_container.h
        include/kcenon/messaging/integrations/external_system_adapter.h
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

# Testing configuration
if(BUILD_TESTING)
    enable_testing()

    # Find GTest
    find_package(GTest REQUIRED)

    # Create test executable for messaging services
    add_executable(messaging_services_tests
        tests/unit/test_message_bus.cpp
        tests/unit/test_service_container.cpp
        tests/unit/test_system_integrator.cpp
        tests/unit/test_container_service.cpp
        tests/unit/test_network_service.cpp
        tests/integration/test_messaging_integration.cpp
        tests/performance/test_message_bus_performance.cpp
    )

    # Link test executable with messaging services and GTest
    target_link_libraries(messaging_services_tests
        PRIVATE
            messaging_services
            GTest::gtest
            GTest::gtest_main
    )

    # Include directories for tests
    target_include_directories(messaging_services_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    # Compiler features for tests
    target_compile_features(messaging_services_tests PRIVATE cxx_std_20)

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(messaging_services_tests)

    # Individual test targets for fine-grained control
    add_test(NAME MessageBusTests COMMAND messaging_services_tests --gtest_filter="MessageBus*")
    add_test(NAME ServiceContainerTests COMMAND messaging_services_tests --gtest_filter="ServiceContainer*")
    add_test(NAME SystemIntegratorTests COMMAND messaging_services_tests --gtest_filter="SystemIntegrator*")
    add_test(NAME IntegrationTests COMMAND messaging_services_tests --gtest_filter="Integration*")
    add_test(NAME PerformanceTests COMMAND messaging_services_tests --gtest_filter="Performance*")
endif()

# Build samples if enabled
if(BUILD_MESSAGING_SAMPLES)
    add_subdirectory(samples)
endif()

# Python bindings (optional, enabled if pybind11 is available)
find_package(pybind11 QUIET)
if(pybind11_FOUND)
    message(STATUS "pybind11 found, building Python bindings")

    pybind11_add_module(messaging_cpp python_bindings/messaging_python.cpp)

    target_link_libraries(messaging_cpp
        PRIVATE
            messaging_services
    )

    target_include_directories(messaging_cpp
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_features(messaging_cpp PRIVATE cxx_std_20)

    # Set properties for Python module
    target_compile_definitions(messaging_cpp PRIVATE VERSION_INFO=${PROJECT_VERSION})

    # Install Python bindings
    install(TARGETS messaging_cpp
        DESTINATION python_bindings
    )

    # Copy Python wrapper
    install(FILES python_bindings/messaging_system_python.py
        DESTINATION python_bindings
        RENAME messaging_system.py
    )
else()
    message(STATUS "pybind11 not found, skipping Python bindings")
endif()