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
  
else()
  message(STATUS "Unit tests are disabled on ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Unit tests are only supported on Linux and macOS platforms")
  message(STATUS "Skipping test suite compilation...")
endif()