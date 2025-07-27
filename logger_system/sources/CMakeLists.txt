# Logger library
add_library(logger)

target_sources(logger
    PRIVATE
        logger/logger.cpp
        logger/core/log_collector.cpp
        logger/writers/console_writer.cpp
        logger/writers/file_writer.cpp
        logger/writers/rotating_file_writer.cpp
        logger/writers/network_writer.cpp
        logger/writers/encrypted_writer.cpp
        logger/server/log_server.cpp
        logger/analysis/log_analyzer.cpp
        logger/security/log_sanitizer.cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/logger.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/logger_interface.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/writers/base_writer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/writers/console_writer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/core/log_collector.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/metrics/logger_metrics.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/structured/structured_logger.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/writers/file_writer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/writers/rotating_file_writer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/writers/network_writer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/writers/encrypted_writer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/filters/log_filter.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/routing/log_router.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/server/log_server.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/analysis/log_analyzer.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logger/security/log_sanitizer.h>
)

target_include_directories(logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/logger_system>
)

# Link with system libraries
target_link_libraries(logger
    PUBLIC
        Threads::Threads
)

# Link filesystem library if needed
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(logger PUBLIC stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(logger PUBLIC c++fs)
endif()

# Set properties
set_target_properties(logger PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Compiler-specific options
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(logger PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(logger PRIVATE /W4)
endif()