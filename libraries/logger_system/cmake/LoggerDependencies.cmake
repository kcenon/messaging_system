##################################################
# Logger System Dependency Detection Module
#
# This module handles finding and configuring external dependencies
##################################################

# Function to find and configure thread_system dependency
function(logger_find_thread_system)
    if(NOT USE_THREAD_SYSTEM OR LOGGER_STANDALONE)
        message(STATUS "Skipping thread_system (standalone mode or disabled)")
        return()
    endif()
    
    # First check if thread_system is available as a sibling directory
    if(EXISTS "${CMAKE_SOURCE_DIR}/../thread_system" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/../thread_system")
        message(STATUS "Found thread_system as sibling directory")
        
        # Add thread_system subdirectory
        add_subdirectory(${CMAKE_SOURCE_DIR}/../thread_system ${CMAKE_BINARY_DIR}/thread_system)
        
        # Set flags to indicate thread_system is available
        set(THREAD_SYSTEM_FOUND TRUE PARENT_SCOPE)
        set(USE_THREAD_SYSTEM TRUE PARENT_SCOPE)
        
        # Add compile definition
        add_compile_definitions(USE_THREAD_SYSTEM)
        
        message(STATUS "Using thread_system for interfaces and utilities")
    else()
        # Try to find installed thread_system
        find_package(ThreadSystem QUIET)
        
        if(ThreadSystem_FOUND)
            message(STATUS "Found installed ThreadSystem: ${ThreadSystem_DIR}")
            set(THREAD_SYSTEM_FOUND TRUE PARENT_SCOPE)
            set(USE_THREAD_SYSTEM TRUE PARENT_SCOPE)
            add_compile_definitions(USE_THREAD_SYSTEM)
        else()
            message(STATUS "thread_system not found - building in standalone mode")
            set(THREAD_SYSTEM_FOUND FALSE PARENT_SCOPE)
            set(USE_THREAD_SYSTEM FALSE PARENT_SCOPE)
            set(LOGGER_STANDALONE TRUE PARENT_SCOPE)
            add_compile_definitions(LOGGER_STANDALONE)
        endif()
    endif()
endfunction()

# Function to find optional compression libraries
function(logger_find_compression)
    if(NOT LOGGER_USE_COMPRESSION)
        return()
    endif()
    
    # Try to find zlib for compression
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
        message(STATUS "Found ZLIB for compression support: ${ZLIB_VERSION}")
        set(LOGGER_HAS_COMPRESSION TRUE PARENT_SCOPE)
    else()
        message(WARNING "ZLIB not found - compression support disabled")
        set(LOGGER_USE_COMPRESSION OFF PARENT_SCOPE)
        set(LOGGER_HAS_COMPRESSION FALSE PARENT_SCOPE)
    endif()
endfunction()

# Function to find optional encryption libraries
function(logger_find_encryption)
    if(NOT LOGGER_USE_ENCRYPTION)
        return()
    endif()
    
    # Try to find OpenSSL for encryption
    find_package(OpenSSL QUIET)
    if(OpenSSL_FOUND)
        message(STATUS "Found OpenSSL for encryption support: ${OPENSSL_VERSION}")
        set(LOGGER_HAS_ENCRYPTION TRUE PARENT_SCOPE)
    else()
        message(WARNING "OpenSSL not found - encryption support disabled")
        set(LOGGER_USE_ENCRYPTION OFF PARENT_SCOPE)
        set(LOGGER_HAS_ENCRYPTION FALSE PARENT_SCOPE)
    endif()
endfunction()

# Function to find testing dependencies
function(logger_find_test_dependencies)
    if(NOT BUILD_TESTS)
        return()
    endif()
    
    # Try to find GTest
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        message(STATUS "GTest not found, attempting to fetch...")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        FetchContent_MakeAvailable(googletest)
    else()
        message(STATUS "Found GTest: ${GTest_DIR}")
    endif()
endfunction()

# Function to find benchmark dependencies
function(logger_find_benchmark_dependencies)
    if(NOT BUILD_BENCHMARKS)
        return()
    endif()
    
    # Try to find Google Benchmark
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        message(STATUS "Google Benchmark not found, attempting to fetch...")
        include(FetchContent)
        FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
        )
        FetchContent_MakeAvailable(googlebenchmark)
    else()
        message(STATUS "Found Google Benchmark: ${benchmark_DIR}")
    endif()
endfunction()

# Main function to find all dependencies
function(logger_find_all_dependencies)
    message(STATUS "========================================")
    message(STATUS "Detecting Logger System Dependencies...")
    
    # Required dependencies
    find_package(Threads REQUIRED)
    
    # Optional dependencies
    logger_find_thread_system()
    logger_find_compression()
    logger_find_encryption()
    logger_find_test_dependencies()
    logger_find_benchmark_dependencies()
    
    message(STATUS "Dependency Detection Complete")
    message(STATUS "========================================")
endfunction()