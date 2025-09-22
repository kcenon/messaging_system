##################################################
# Logger System Feature Configuration Module
#
# This module defines feature flags and configurations for the Logger System
##################################################

# Feature flags for logger customization
option(LOGGER_USE_DI "Enable dependency injection support" ON)
option(LOGGER_USE_MONITORING "Enable monitoring and metrics collection" ON)
option(LOGGER_USE_EXTERNAL_DI "Use external DI container if available" OFF)
option(LOGGER_FORCE_LIGHTWEIGHT "Force lightweight implementations only" ON)
option(LOGGER_ENABLE_SANITIZERS "Enable address and thread sanitizers" OFF)
option(LOGGER_ENABLE_COVERAGE "Enable code coverage" OFF)
option(LOGGER_USE_COMPRESSION "Enable log compression support" OFF)
option(LOGGER_USE_ENCRYPTION "Enable log encryption support" OFF)
option(LOGGER_ENABLE_CRASH_HANDLER "Enable crash handler integration" ON)
option(LOGGER_ENABLE_STRUCTURED_LOGGING "Enable structured logging (JSON)" OFF)
option(LOGGER_USE_LOCK_FREE_QUEUE "Use lock-free queue implementation" ${USE_LOCKFREE})
option(LOGGER_ENABLE_NETWORK_WRITER "Enable network log writer" OFF)
option(LOGGER_ENABLE_FILE_ROTATION "Enable file rotation support" ON)
option(LOGGER_ENABLE_ASYNC "Enable asynchronous logging" ON)

# Performance tuning options
set(LOGGER_DEFAULT_BUFFER_SIZE "8192" CACHE STRING "Default buffer size in bytes")
set(LOGGER_DEFAULT_BATCH_SIZE "100" CACHE STRING "Default batch size for processing")
set(LOGGER_DEFAULT_QUEUE_SIZE "10000" CACHE STRING "Default maximum queue size")
set(LOGGER_MAX_WRITERS "10" CACHE STRING "Maximum number of concurrent writers")

# Validation of options
if(LOGGER_USE_EXTERNAL_DI AND LOGGER_FORCE_LIGHTWEIGHT)
    message(WARNING "LOGGER_USE_EXTERNAL_DI and LOGGER_FORCE_LIGHTWEIGHT are mutually exclusive. Disabling LOGGER_USE_EXTERNAL_DI.")
    set(LOGGER_USE_EXTERNAL_DI OFF CACHE BOOL "Use external DI container if available" FORCE)
endif()

if(NOT LOGGER_ENABLE_ASYNC AND LOGGER_USE_LOCK_FREE_QUEUE)
    message(WARNING "Lock-free queue requires async mode. Disabling LOGGER_USE_LOCK_FREE_QUEUE.")
    set(LOGGER_USE_LOCK_FREE_QUEUE OFF CACHE BOOL "Use lock-free queue implementation" FORCE)
endif()

# Create feature summary
function(logger_print_features)
    message(STATUS "========================================")
    message(STATUS "Logger System Feature Configuration:")
    message(STATUS "  Core Features:")
    message(STATUS "    Dependency Injection: ${LOGGER_USE_DI}")
    message(STATUS "    Monitoring: ${LOGGER_USE_MONITORING}")
    message(STATUS "    Async Logging: ${LOGGER_ENABLE_ASYNC}")
    message(STATUS "    Crash Handler: ${LOGGER_ENABLE_CRASH_HANDLER}")
    message(STATUS "  Advanced Features:")
    message(STATUS "    External DI: ${LOGGER_USE_EXTERNAL_DI}")
    message(STATUS "    Lock-free Queue: ${LOGGER_USE_LOCK_FREE_QUEUE}")
    message(STATUS "    Structured Logging: ${LOGGER_ENABLE_STRUCTURED_LOGGING}")
    message(STATUS "    Network Writer: ${LOGGER_ENABLE_NETWORK_WRITER}")
    message(STATUS "    File Rotation: ${LOGGER_ENABLE_FILE_ROTATION}")
    message(STATUS "    Compression: ${LOGGER_USE_COMPRESSION}")
    message(STATUS "    Encryption: ${LOGGER_USE_ENCRYPTION}")
    message(STATUS "  Performance Settings:")
    message(STATUS "    Buffer Size: ${LOGGER_DEFAULT_BUFFER_SIZE}")
    message(STATUS "    Batch Size: ${LOGGER_DEFAULT_BATCH_SIZE}")
    message(STATUS "    Queue Size: ${LOGGER_DEFAULT_QUEUE_SIZE}")
    message(STATUS "    Max Writers: ${LOGGER_MAX_WRITERS}")
    message(STATUS "  Build Options:")
    message(STATUS "    Force Lightweight: ${LOGGER_FORCE_LIGHTWEIGHT}")
    message(STATUS "    Sanitizers: ${LOGGER_ENABLE_SANITIZERS}")
    message(STATUS "    Coverage: ${LOGGER_ENABLE_COVERAGE}")
    message(STATUS "========================================")
endfunction()

# Configure compile definitions based on features
function(logger_configure_target target)
    if(LOGGER_USE_DI)
        target_compile_definitions(${target} PUBLIC LOGGER_USE_DI)
    endif()
    
    if(LOGGER_USE_MONITORING)
        target_compile_definitions(${target} PUBLIC LOGGER_USE_MONITORING)
    endif()
    
    if(LOGGER_USE_EXTERNAL_DI)
        target_compile_definitions(${target} PUBLIC LOGGER_USE_EXTERNAL_DI)
    endif()
    
    if(LOGGER_FORCE_LIGHTWEIGHT)
        target_compile_definitions(${target} PUBLIC LOGGER_FORCE_LIGHTWEIGHT)
    endif()
    
    if(LOGGER_ENABLE_CRASH_HANDLER)
        target_compile_definitions(${target} PUBLIC LOGGER_ENABLE_CRASH_HANDLER)
    endif()
    
    if(LOGGER_ENABLE_STRUCTURED_LOGGING)
        target_compile_definitions(${target} PUBLIC LOGGER_ENABLE_STRUCTURED_LOGGING)
    endif()
    
    if(LOGGER_USE_LOCK_FREE_QUEUE)
        target_compile_definitions(${target} PUBLIC LOGGER_USE_LOCK_FREE_QUEUE)
    endif()
    
    if(LOGGER_ENABLE_NETWORK_WRITER)
        target_compile_definitions(${target} PUBLIC LOGGER_ENABLE_NETWORK_WRITER)
    endif()
    
    if(LOGGER_ENABLE_FILE_ROTATION)
        target_compile_definitions(${target} PUBLIC LOGGER_ENABLE_FILE_ROTATION)
    endif()
    
    if(LOGGER_USE_COMPRESSION)
        target_compile_definitions(${target} PUBLIC LOGGER_USE_COMPRESSION)
    endif()
    
    if(LOGGER_USE_ENCRYPTION)
        target_compile_definitions(${target} PUBLIC LOGGER_USE_ENCRYPTION)
    endif()
    
    if(LOGGER_ENABLE_ASYNC)
        target_compile_definitions(${target} PUBLIC LOGGER_ENABLE_ASYNC)
    endif()
    
    # Add performance settings as compile definitions
    target_compile_definitions(${target} PUBLIC 
        LOGGER_DEFAULT_BUFFER_SIZE=${LOGGER_DEFAULT_BUFFER_SIZE}
        LOGGER_DEFAULT_BATCH_SIZE=${LOGGER_DEFAULT_BATCH_SIZE}
        LOGGER_DEFAULT_QUEUE_SIZE=${LOGGER_DEFAULT_QUEUE_SIZE}
        LOGGER_MAX_WRITERS=${LOGGER_MAX_WRITERS}
    )
    
    # Add sanitizers if enabled
    # Note: Don't add sanitizers here - they're handled by LoggerSanitizers.cmake
    # This prevents conflicts between address and thread sanitizers
    
    # Add coverage flags if enabled
    if(LOGGER_ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${target} PUBLIC --coverage -fprofile-arcs -ftest-coverage)
            target_link_options(${target} PUBLIC --coverage)
        else()
            message(WARNING "Coverage is only supported with Clang and GCC")
        endif()
    endif()
endfunction()