cmake_minimum_required(VERSION 3.16)

project(messaging_system
    VERSION 0.1.0.0
    DESCRIPTION "Production-ready messaging infrastructure with advanced patterns"
    LANGUAGES CXX
)

# =============================================================================
# C++ Standard Configuration
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Explicit C++20 flags for all compilers
if(MSVC)
    add_compile_options(/std:c++20)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-std=c++20)
endif()

# =============================================================================
# Build Options
# =============================================================================

# Feature flags
option(MESSAGING_USE_LOCKFREE "Enable lock-free data structures" OFF)
option(MESSAGING_ENABLE_MONITORING "Enable runtime monitoring" ON)
option(MESSAGING_ENABLE_LOGGING "Enable logging system" ON)
option(MESSAGING_ENABLE_METRICS "Enable metrics collection" ON)
option(MESSAGING_ENABLE_TLS "Enable TLS/SSL support" OFF)
option(MESSAGING_ENABLE_TRACING "Enable distributed tracing" ON)
option(MESSAGING_ENABLE_HEALTH_CHECKS "Enable health monitoring and circuit breakers" ON)

# Performance optimization flags
option(MESSAGING_ENABLE_MOVE_SEMANTICS "Enable zero-copy move semantics optimization" ON)
option(MESSAGING_ENABLE_SIMD "Enable SIMD optimizations for message processing" OFF)
option(MESSAGING_ENABLE_INLINE_OPTIMIZATION "Enable aggressive inlining for hot paths" ON)

# Dependency strategy (mapped to unified module options)
option(MESSAGING_USE_EXTERNAL_SYSTEMS "Use external system packages" ON)
option(MESSAGING_USE_FETCHCONTENT "Use FetchContent for dependencies" OFF)
option(MESSAGING_USE_LOCAL_SYSTEMS "Use local sibling directories for development" OFF)

# Development options
option(MESSAGING_BUILD_TESTS "Build unit tests" ON)
option(MESSAGING_BUILD_INTEGRATION_TESTS "Build integration tests" ON)
option(MESSAGING_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(MESSAGING_BUILD_EXAMPLES "Build example applications" ON)
option(MESSAGING_BUILD_DOCS "Build documentation with Doxygen" OFF)

# Build profile
set(MESSAGING_BUILD_PROFILE "Release" CACHE STRING "Build profile")
set_property(CACHE MESSAGING_BUILD_PROFILE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel"
    "ASAN" "TSAN" "UBSAN" "MSAN" "Coverage"
)

# ABI Version
set(MESSAGING_ABI_VERSION 2 CACHE STRING "Messaging system ABI version")

# Performance tuning defaults
set(MESSAGING_DEFAULT_QUEUE_SIZE 10000 CACHE STRING "Default message queue size")
set(MESSAGING_DEFAULT_BATCH_SIZE 100 CACHE STRING "Default batch processing size")
set(MESSAGING_DEFAULT_WORKER_THREADS 4 CACHE STRING "Default number of worker threads")

# =============================================================================
# Compiler Settings
# =============================================================================

set(MESSAGING_WARNING_LEVEL "High" CACHE STRING "Compiler warning level")

if(MESSAGING_WARNING_LEVEL STREQUAL "High" OR MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic)
        if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
            add_compile_options(-Werror -Wconversion -Wsign-conversion)
        endif()
    elseif(MSVC)
        add_compile_options(/W4)
        if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
            add_compile_options(/WX)
        endif()
    endif()
endif()

# Sanitizer flags
if(MESSAGING_BUILD_PROFILE STREQUAL "ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "TSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "UBSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "MSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=memory")
endif()

# =============================================================================
# External Dependencies (Unified Management)
# =============================================================================

if(MESSAGING_USE_EXTERNAL_SYSTEMS)
    # Map messaging options to unified module options
    set(UNIFIED_USE_LOCAL ${MESSAGING_USE_LOCAL_SYSTEMS} CACHE BOOL "" FORCE)
    set(UNIFIED_USE_FETCHCONTENT ${MESSAGING_USE_FETCHCONTENT} CACHE BOOL "" FORCE)

    # Include unified dependency management module
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include(UnifiedDependencies)

    # Setup dependency mode (LOCAL, FETCHCONTENT, or find_package)
    unified_setup_dependency_mode()

    # Required dependencies - ordered by dependency chain
    # IMPORTANT: container_system MUST be last because it defines its own
    # unified_find_dependency macro which overwrites ours. Any calls after
    # container_system would use its macro (which lacks our configurations).
    #
    # NOTE: logger_system is no longer a direct dependency (Issue #94).
    # Logging is now provided through common_system's ILogger interface
    # with runtime binding via GlobalLoggerRegistry.
    unified_find_dependency(common_system REQUIRED)      # Tier 0: Base utilities + ILogger
    unified_find_dependency(thread_system REQUIRED)      # Tier 1: Threading
    unified_find_dependency(network_system REQUIRED)     # Tier 2: Network I/O

    # Optional dependencies (must come before container_system)
    unified_find_dependency(monitoring_system OPTIONAL)  # Metrics
    unified_find_dependency(database_system OPTIONAL)    # Persistence

    # container_system MUST be loaded last (macro conflict workaround)
    unified_find_dependency(container_system REQUIRED)   # Tier 4: Serialization

    # Create standard aliases for consistent linking
    unified_create_standard_aliases()
else()
    message(STATUS "Building header-only library without external systems")
endif()

# Propagate options to external systems
if(MESSAGING_USE_LOCKFREE)
    set(USE_LOCKFREE_BY_DEFAULT ON CACHE BOOL "" FORCE)
    set(USE_LOCKFREE_THREAD_POOL ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_MONITORING)
    set(ENABLE_MONITORING ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_LOGGING)
    set(ENABLE_LOGGING ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_TLS)
    set(ENABLE_TLS ON CACHE BOOL "" FORCE)
endif()

# =============================================================================
# Messaging System Core Library
# =============================================================================

set(MESSAGING_CORE_SOURCES
    src/impl/core/message.cpp
    src/impl/core/message_queue.cpp
    src/impl/core/topic_router.cpp
    src/impl/core/message_bus.cpp
    src/impl/backends/standalone_backend.cpp
    src/impl/backends/integration_backend.cpp
    src/impl/di/messaging_di_container.cpp
    src/impl/di/service_registry.cpp
    src/impl/patterns/pub_sub.cpp
    src/impl/patterns/request_reply.cpp
    src/impl/patterns/event_streaming.cpp
    src/impl/patterns/message_pipeline.cpp
    # Task module (Distributed Task Queue System)
    src/impl/task/task.cpp
    src/impl/task/task_context.cpp
    src/impl/task/task_queue.cpp
    src/impl/task/memory_result_backend.cpp
    src/impl/task/worker_pool.cpp
    src/impl/task/async_result.cpp
    src/impl/task/task_client.cpp
    src/impl/task/cron_parser.cpp
    src/impl/task/scheduler.cpp
    src/impl/task/monitor.cpp
    src/impl/task/task_system.cpp
)

add_library(messaging_system_core STATIC ${MESSAGING_CORE_SOURCES})

set_target_properties(messaging_system_core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(messaging_system_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/impl
)

# Link dependencies using unified target variables
if(container_system_TARGET)
    target_link_libraries(messaging_system_core PUBLIC ${container_system_TARGET})
elseif(TARGET container)
    target_link_libraries(messaging_system_core PUBLIC container)
elseif(TARGET ContainerSystem::container)
    target_link_libraries(messaging_system_core PUBLIC ContainerSystem::container)
endif()

if(common_system_TARGET)
    target_link_libraries(messaging_system_core PUBLIC ${common_system_TARGET})
elseif(TARGET common)
    target_link_libraries(messaging_system_core PUBLIC common)
elseif(TARGET CommonSystem::common)
    target_link_libraries(messaging_system_core PUBLIC CommonSystem::common)
endif()

# Link thread_system (required for task_queue's delayed_task_worker)
if(thread_system_TARGET)
    target_link_libraries(messaging_system_core PUBLIC ${thread_system_TARGET})
elseif(TARGET thread_pool)
    target_link_libraries(messaging_system_core PUBLIC thread_pool)
elseif(TARGET ThreadSystem::thread_pool)
    target_link_libraries(messaging_system_core PUBLIC ThreadSystem::thread_pool)
endif()

# Compile definitions
target_compile_definitions(messaging_system_core PUBLIC
    KCENON_MESSAGING_SYSTEM_AVAILABLE
    MESSAGING_ABI_VERSION=${MESSAGING_ABI_VERSION}
    MESSAGING_DEFAULT_QUEUE_SIZE=${MESSAGING_DEFAULT_QUEUE_SIZE}
    MESSAGING_DEFAULT_BATCH_SIZE=${MESSAGING_DEFAULT_BATCH_SIZE}
    MESSAGING_DEFAULT_WORKER_THREADS=${MESSAGING_DEFAULT_WORKER_THREADS}
)

# Feature-specific compile definitions and options
if(MESSAGING_ENABLE_MOVE_SEMANTICS)
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_ENABLE_MOVE_SEMANTICS)
endif()

if(MESSAGING_ENABLE_SIMD)
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_ENABLE_SIMD)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        target_compile_options(messaging_system_core PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:-mavx2 -mfma>
            $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
        )
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64|ARM64")
        target_compile_options(messaging_system_core PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:-march=armv8-a+simd>
        )
    endif()
endif()

if(MESSAGING_ENABLE_INLINE_OPTIMIZATION)
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_ENABLE_INLINE_OPTIMIZATION)
    target_compile_options(messaging_system_core PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-finline-functions -finline-limit=600>
        $<$<CXX_COMPILER_ID:MSVC>:/Ob2>
    )
endif()

if(MESSAGING_ENABLE_TRACING)
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_ENABLE_TRACING)
endif()

if(MESSAGING_ENABLE_HEALTH_CHECKS)
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_ENABLE_HEALTH_CHECKS)
endif()

# =============================================================================
# Tests, Examples, Documentation
# =============================================================================

if(MESSAGING_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(test)

    if(MESSAGING_BUILD_INTEGRATION_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration_tests/CMakeLists.txt)
        add_subdirectory(integration_tests)
    endif()
endif()

if(MESSAGING_BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
    add_subdirectory(examples)
endif()

if(MESSAGING_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/docs)
        add_subdirectory(docs)
    endif()
endif()

# =============================================================================
# Installation
# =============================================================================

if(NOT MESSAGING_USE_LOCAL_SYSTEMS
   AND NOT MESSAGING_USE_FETCHCONTENT
   AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(GNUInstallDirs)

    install(TARGETS messaging_system_core
        EXPORT messaging_system_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT messaging_system_targets
        FILE messaging_system-targets.cmake
        NAMESPACE MessagingSystem::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/messaging_system
    )
endif()

# =============================================================================
# Configuration Summary
# =============================================================================

message(STATUS "")
message(STATUS "=== Messaging System Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION} (ABI: ${MESSAGING_ABI_VERSION})")
message(STATUS "Build: ${CMAKE_BUILD_TYPE} / ${MESSAGING_BUILD_PROFILE}")
message(STATUS "")
message(STATUS "Features: Lock-free=${MESSAGING_USE_LOCKFREE}, TLS=${MESSAGING_ENABLE_TLS}")
message(STATUS "          Tracing=${MESSAGING_ENABLE_TRACING}, Health=${MESSAGING_ENABLE_HEALTH_CHECKS}")
message(STATUS "")
message(STATUS "Build options: Tests=${MESSAGING_BUILD_TESTS}, Examples=${MESSAGING_BUILD_EXAMPLES}")
message(STATUS "")

if(MESSAGING_USE_EXTERNAL_SYSTEMS)
    unified_print_dependency_summary()
endif()

message(STATUS "=======================================")
message(STATUS "")
