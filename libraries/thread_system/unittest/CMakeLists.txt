##################################################
# ThreadSystem Unit Tests CMakeLists.txt
# 
# This script configures and builds the test suite for validating 
# the ThreadSystem library functionality.
# Tests are organized by component and run automatically during build if enabled.
# 
# Platform Support: Linux and macOS only
##################################################

# Check if we're on a supported platform (Linux or macOS)
if(UNIX AND NOT WIN32)
  message(STATUS "Building unit tests for ${CMAKE_SYSTEM_NAME}")
  
  # Utility component tests
  # Validates core utilities, data structures, and helper functions
  add_subdirectory(utilities_test)
  
  # Thread base tests
  # Validates fundamental threading components, job queues, and cancellation mechanisms
  add_subdirectory(thread_base_test)
  
  # Thread pool tests
  # Validates standard thread pool operations, task queueing, and execution
  add_subdirectory(thread_pool_test)
  
  # Interfaces tests
  add_subdirectory(interfaces_test)
  
  # Typed thread pool tests
  # Validates type-based scheduling, task routing, and edge cases
  # TEMPORARILY DISABLED: typed_thread_pool implementation not available in current branch
  # add_subdirectory(typed_thread_pool_test)
  
  # Monitoring tests
  # Validates metrics collection, ring buffer, and monitoring functionality
  # TEMPORARILY DISABLED: Monitoring is being moved to separate project
  # add_subdirectory(monitoring_test)
  
  # Platform-specific tests
  # Validates platform-specific features, atomic operations, and cache performance
  add_subdirectory(platform_test)
  
else()
  message(STATUS "Unit tests are disabled on ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Unit tests are only supported on Linux and macOS platforms")
  message(STATUS "Skipping test suite compilation...")
endif()
