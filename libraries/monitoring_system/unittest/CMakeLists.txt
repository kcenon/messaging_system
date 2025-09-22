##################################################
# Monitoring System Unit Tests CMakeLists.txt
# 
# This script configures and builds the test suite for validating 
# the Monitoring System functionality.
# Tests are organized by component and run automatically during build if enabled.
##################################################

# Check if we're on a supported platform (Linux or macOS)
if(UNIX AND NOT WIN32)
  message(STATUS "Building unit tests for ${CMAKE_SYSTEM_NAME}")
  
  # Monitoring component tests
  # Validates core monitoring functionality, metrics collection, and storage
  add_subdirectory(monitoring_test)
  
  # Storage tests
  # Validates ring buffer, metrics storage, and historical data management
  add_subdirectory(storage_test)
  
else()
  message(STATUS "Unit tests are disabled on ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Unit tests are only supported on Linux and macOS platforms")
  message(STATUS "Skipping test suite compilation...")
endif()