cmake_minimum_required(VERSION 3.16)
project(thread_system_config_test CXX)

# Phase 2 T2.1 Validation Test
message(STATUS "Testing Phase 2 T2.1 CMake Config files...")

# Set CMAKE_PREFIX_PATH to find our installed config files
set(CMAKE_PREFIX_PATH "/tmp/thread_system_install/usr/local" ${CMAKE_PREFIX_PATH})

# Test 1: Find package with new standardized naming
message(STATUS "")
message(STATUS "Test 1: find_package(thread_system CONFIG REQUIRED)")
find_package(thread_system CONFIG REQUIRED)

if(thread_system_FOUND)
    message(STATUS "✅ PASS: thread_system found with new config files")
    
    # Check if component targets exist
    set(expected_targets 
        thread_system::thread_pool
        thread_system::service_container 
        thread_system::thread_utilities
        thread_system::thread_base
        thread_system::lockfree)
    
    foreach(target ${expected_targets})
        if(TARGET ${target})
            message(STATUS "   ✓ Target available: ${target}")
        else()
            message(STATUS "   ✗ Target missing: ${target}")
        endif()
    endforeach()
else()
    message(FATAL_ERROR "❌ FAIL: thread_system not found")
endif()

# Test 2: Legacy compatibility
message(STATUS "")
message(STATUS "Test 2: find_package(ThreadSystem CONFIG REQUIRED)")
find_package(ThreadSystem CONFIG REQUIRED)

if(ThreadSystem_FOUND)
    message(STATUS "✅ PASS: ThreadSystem found (legacy compatibility)")
else()
    message(STATUS "⚠️ WARNING: ThreadSystem legacy compatibility not working")
endif()

message(STATUS "")
message(STATUS "✅ Phase 2 T2.1 CMake Config Validation Complete")
message(STATUS "==========================================")

# Create a dummy target to complete the test
add_library(config_test_dummy INTERFACE)