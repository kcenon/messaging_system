# Monitoring library
add_library(monitoring)

target_sources(monitoring
    PRIVATE
        monitoring/monitoring.cpp
        monitoring/multi_process_monitoring.cpp
        monitoring/thread_pool_analyzer.cpp
        monitoring/trend_analyzer.cpp
        monitoring/analysis_dashboard.cpp
        monitoring/optimized_storage.cpp
        monitoring/performance_optimizer.cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/monitoring.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/monitoring_interface.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/multi_process_monitoring.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/thread_pool_analyzer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/trend_analyzer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/analysis_dashboard.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/optimized_storage.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/performance_optimizer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interfaces/process_identifier.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interfaces/multi_process_monitoring_interface.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interfaces/monitoring_crash_safety.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/monitoring/storage/ring_buffer.h>
)

target_include_directories(monitoring
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/monitoring_system>
)

# Link with system libraries
target_link_libraries(monitoring
    PUBLIC
        Threads::Threads
)

# Set properties
set_target_properties(monitoring PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Compiler-specific options
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(monitoring PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(monitoring PRIVATE /W4)
endif()