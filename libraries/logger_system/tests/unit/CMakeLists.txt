##################################################
# Logger System Unit Tests CMakeLists.txt
# 
# This script configures and builds the test suite for validating 
# the Logger System functionality.
# Tests are organized by component and run automatically during build if enabled.
##################################################

# Check if we're on a supported platform (Linux or macOS)
if(UNIX AND NOT WIN32)
  message(STATUS "Building unit tests for ${CMAKE_SYSTEM_NAME}")
  
  # Logger component tests
  # Validates core logger functionality, async/sync modes, and level filtering
  add_subdirectory(logger_test)
  
  # Writers tests
  # Validates console writer, file writer, and base writer functionality
  add_subdirectory(writers_test)
  
  # Log collectors tests
  # Validates async log collection, buffering, and thread safety
  add_subdirectory(collectors_test)
  
  # Configuration tests
  # Validates configuration validation and builder pattern
  add_subdirectory(config_test)
  
  # DI Container tests
  # Validates dependency injection container implementations
  add_subdirectory(di_test)
  
  # Monitoring tests
  # Validates monitoring interface and implementations
  add_subdirectory(monitoring_test)
  
  # Crash safety tests
  # Validates crash safety mechanisms and recovery
  add_subdirectory(safety_test)
  
  # Health check tests
  # Validates health monitoring and checking system
  add_subdirectory(health_test)
  
  # Flow control tests
  # Validates overflow policies and adaptive backpressure
  add_subdirectory(flow_test)
  
  # Stress tests
  # Validates system behavior under extreme load conditions
  add_subdirectory(stress_test)
  
  # Integration tests
  # Validates end-to-end scenarios with multiple components
  add_subdirectory(integration_test)
  
else()
  message(STATUS "Unit tests are disabled on ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Unit tests are only supported on Linux and macOS platforms")
  message(STATUS "Skipping test suite compilation...")
endif()