cmake_minimum_required(VERSION 3.16)
project(messaging_system)

# Set C++20 as the default standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add debug information
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /Zc:__cplusplus")
endif()

# Include necessary CMake modules
include(CheckCXXSourceCompiles)

option(SET_STD_FORMAT "Set use std::format if it can use" ON)
option(SET_STD_JTHREAD "Set use std::jthread if it can use" ON)
option(SET_STD_CHRONO_CURRENT_ZONE "Set use std::chrono::current_zone if it can use" ON)

# Function to check C++20 features
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
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has support for std::format.")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have support for std::format, so will use the fmt::format instead.")
endif()

# Check for std::jthread
check_cxx20_feature(std_jthread "
  #include <thread>
  #include <stop_token>
  int main() {
    std::jthread t([](std::stop_token st) {});
    return 0;
  }
" HAS_STD_JTHREAD)

if(HAS_STD_JTHREAD AND SET_STD_JTHREAD)
  add_definitions(-DUSE_STD_JTHREAD)
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has support for std::jthread.")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have support for std::jthread, so will use the std::thread instead.")
endif()

# Check for std::chrono::current_zone
check_cxx20_feature(std_jthread "
  #include <chrono>
  int main() {
    const auto now = std::chrono::system_clock::now();
    const auto local_time = std::chrono::current_zone()->to_local(now);
    return 0;
  }
" HAS_STD_CHRONO_CURRENT_ZONE)

if(HAS_STD_CHRONO_CURRENT_ZONE AND SET_STD_CHRONO_CURRENT_ZONE)
  add_definitions(-DUSE_STD_CHRONO_CURRENT_ZONE)
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has support for std::chrono::current_zone.")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have support for std::chrono::current_zone, so will use the time_t instead.")
endif()

# Set the project output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Check if the VCPKG_TOOLCHAIN is provided
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(FATAL_ERROR "Please provide the path to vcpkg toolchain file using -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg_toolchain>")
endif()

option(BUILD_THREADSYSTEM_AS_SUBMODULE "Build ThreadSystem as submodule" ON)

# cpp_libraries
ADD_SUBDIRECTORY(thread_system)
ADD_SUBDIRECTORY(container)
ADD_SUBDIRECTORY(database)
ADD_SUBDIRECTORY(network)
#IF(USE_UNIT_TEST)
    #ADD_SUBDIRECTORY(unittest)
#ENDIF()