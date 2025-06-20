##################################################
# MessagingSystem CMakeLists.txt
# 
# Main build configuration for the MessagingSystem library.
# This builds a high-performance, C++20 network messaging framework.
##################################################

# Specify minimum required CMake version for this project
cmake_minimum_required(VERSION 3.16)

# Define the project name, version, and language
project(MessagingSystem 
    VERSION 1.0.0
    DESCRIPTION "High-performance C++20 network messaging framework"
    HOMEPAGE_URL "https://github.com/kcenon/messaging_system"
    LANGUAGES CXX
)

# Option to control whether MessagingSystem is built as a standalone project or as a submodule
# - OFF (default): Builds as standalone project with samples and tests
# - ON: Builds only the core libraries for integration into other projects
option(BUILD_MESSAGING_SYSTEM_AS_SUBMODULE "Build MessagingSystem as submodule" OFF)

##################################################
# C++ Standard Configuration
##################################################

# Set C++20 as the default standard for all targets
set(CMAKE_CXX_STANDARD 20)           # Use C++20 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Require C++20 support
set(CMAKE_CXX_EXTENSIONS OFF)        # Disable compiler-specific extensions
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json for tooling

# Output compiler information for debugging purposes
message(STATUS "========================================")
message(STATUS "Compiler Configuration:")
message(STATUS "  ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Path: ${CMAKE_CXX_COMPILER}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "========================================")

##################################################
# Compiler-Specific Configuration
##################################################

# Configure compiler flags based on the compiler being used
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # For GCC and Clang compilers
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  
  # Add additional flags for macOS
  if(APPLE)
    # Suppress duplicate library warnings on macOS (common with vcpkg)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_warn_duplicate_libraries")
  endif()
elseif(MSVC)
  # For Microsoft Visual C++ compiler
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /Zc:__cplusplus")
endif()

##################################################
# Include Custom CMake Modules
##################################################

# Add custom cmake modules path (if we have one)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##################################################
# Platform-Specific Threading Configuration
##################################################

# Handle threading differently for macOS vs other platforms
if(APPLE)
  # On macOS, we handle pthread differently
  add_definitions(-DHAVE_PTHREAD)           # Define that we have pthread support
  add_definitions(-DAPPLE_PLATFORM)         # Define that we're on Apple platform
  
  # Set thread-related variables without adding explicit link flags
  set(THREADS_FOUND TRUE)
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(Threads_FOUND TRUE)
else()
  # On other platforms, use CMake's thread package finder
  set(THREADS_PREFER_PTHREAD_FLAG ON)       # Prefer using -pthread flag when available
  find_package(Threads REQUIRED)            # Find threading library
endif()

##################################################
# Feature Detection
##################################################

# Include the module for checking C++ source compilation
include(CheckCXXSourceCompiles)

# Options to control whether specific C++20 features are used
option(SET_STD_FORMAT "Set use std::format if it can use" ON)
option(SET_STD_JTHREAD "Set use std::jthread if it can use" ON)
option(SET_STD_CHRONO_CURRENT_ZONE "Set use std::chrono::current_zone if it can use" ON)

# Function to test for C++20 features at configure time
# Parameters:
# - FEATURE_NAME: Name of the feature being checked (for logging)
# - TEST_CODE: C++ code snippet that tests the feature
# - RESULT_VAR: Variable name to store the result (TRUE/FALSE)
function(check_cxx20_feature FEATURE_NAME TEST_CODE RESULT_VAR)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_EXE_LINKER_FLAGS}")
  
  check_cxx_source_compiles("
    #include <cstddef>
    #include <cstdint>
    #include <utility>
    ${TEST_CODE}
  " ${RESULT_VAR})
endfunction()

##################################################
# Test for std::format (C++20 feature)
##################################################

# Check for std::format
check_cxx20_feature(std_format "
  #include <format>
  #include <string>
  int main() {
    std::string s = std::format(\"{} {}\", \"Hello\", \"World\");
    return 0;
  }
" HAS_STD_FORMAT)

if(HAS_STD_FORMAT AND SET_STD_FORMAT)
  add_definitions(-DUSE_STD_FORMAT)
  message(STATUS "✅ Using std::format - compiler support verified")
else()
  message(STATUS "❌ Using fmt::format fallback - std::format not supported")
  # Ensure fmt is available as fallback
  find_package(fmt CONFIG REQUIRED)
endif()

##################################################
# Test for std::jthread (C++20 feature)
##################################################

check_cxx20_feature(std_jthread "
  #include <thread>
  #include <stop_token>
  int main() {
    std::jthread t([](std::stop_token st) {});
    return 0;
  }
" HAS_STD_JTHREAD)

if(HAS_STD_JTHREAD AND SET_STD_JTHREAD)
  add_definitions(-DUSE_STD_JTHREAD)  # Define macro to use std::jthread in code
  message(STATUS "✅ Using std::jthread - compiler support verified")
else()
  message(STATUS "❌ Using std::thread fallback - std::jthread not supported")
endif()

##################################################
# Test for std::chrono::current_zone (C++20 feature)
##################################################

check_cxx20_feature(std_chrono_current_zone "
  #include <chrono>
  int main() {
    const auto now = std::chrono::system_clock::now();
    const auto local_time = std::chrono::current_zone()->to_local(now);
    return 0;
  }
" HAS_STD_CHRONO_CURRENT_ZONE)

if(HAS_STD_CHRONO_CURRENT_ZONE AND SET_STD_CHRONO_CURRENT_ZONE)
  add_definitions(-DUSE_STD_CHRONO_CURRENT_ZONE)  # Define macro to use std::chrono::current_zone in code
  message(STATUS "✅ Using std::chrono::current_zone - compiler support verified")
else()
  message(STATUS "❌ Using time_t fallback - std::chrono::current_zone not supported")
endif()

##################################################
# Build Configuration
##################################################

if(NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
  # When building as a standalone project:
  
  # Set output directories for binaries, libraries, and archives
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

  # Verify that vcpkg toolchain is provided
  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Please provide the path to vcpkg toolchain file using -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg_toolchain>")
  endif()
endif()

##################################################
# Subproject Configuration
##################################################

# ThreadSystem is a required dependency - always build it as a submodule
option(BUILD_THREADSYSTEM_AS_SUBMODULE "Build ThreadSystem as submodule" ON)
add_subdirectory(thread_system)

# Core messaging system libraries
add_subdirectory(container)
add_subdirectory(database)
add_subdirectory(network)

# Conditional builds for standalone mode
if(NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
  # Unit tests
  option(BUILD_UNIT_TESTS "Build unit tests" ON)
  if(BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(unittest)
  endif()

  # Sample applications
  option(BUILD_SAMPLES "Build sample applications" ON)
  if(BUILD_SAMPLES)
    add_subdirectory(samples)
  endif()
endif()

##################################################
# Installation Configuration
##################################################

# Only configure installation when building as standalone
if(NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
  include(GNUInstallDirs)
  
  # Install header files with proper directory structure
  install(DIRECTORY container/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/container
          FILES_MATCHING PATTERN "*.h" PATTERN "*.tpp"
          PATTERN "build" EXCLUDE)
          
  install(DIRECTORY database/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/database
          FILES_MATCHING PATTERN "*.h"
          PATTERN "build" EXCLUDE)
          
  install(DIRECTORY network/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/network
          FILES_MATCHING PATTERN "*.h"
          PATTERN "build" EXCLUDE)
  
  # Install libraries - include thread_system dependencies
  install(TARGETS container database network utilities thread_base thread_pool logger monitoring
          EXPORT MessagingSystemTargets
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  
  # Install export targets
  install(EXPORT MessagingSystemTargets
          FILE MessagingSystemTargets.cmake
          NAMESPACE MessagingSystem::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MessagingSystem)
  
  # Create and install config file
  include(CMakePackageConfigHelpers)
  
  # Create a simple config file if template doesn't exist
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/MessagingSystemConfig.cmake.in
"@PACKAGE_INIT@

include(\"\${CMAKE_CURRENT_LIST_DIR}/MessagingSystemTargets.cmake\")

# Find dependencies
find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Verify that all required targets exist
check_required_components(MessagingSystem)
")
  
  configure_package_config_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MessagingSystemConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MessagingSystemConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MessagingSystem
  )
  
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MessagingSystemConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MessagingSystemConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MessagingSystemConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MessagingSystem
  )
endif()

##################################################
# Summary
##################################################

message(STATUS "========================================")
message(STATUS "MessagingSystem Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build as Submodule: ${BUILD_MESSAGING_SYSTEM_AS_SUBMODULE}")
if(NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
  message(STATUS "  Build Unit Tests: ${BUILD_UNIT_TESTS}")
  message(STATUS "  Build Samples: ${BUILD_SAMPLES}")
endif()
message(STATUS "  C++ Features:")
message(STATUS "    - std::format: ${HAS_STD_FORMAT}")
message(STATUS "    - std::jthread: ${HAS_STD_JTHREAD}")
message(STATUS "    - std::chrono::current_zone: ${HAS_STD_CHRONO_CURRENT_ZONE}")
message(STATUS "========================================")