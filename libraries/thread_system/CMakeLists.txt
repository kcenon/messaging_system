##################################################
# ThreadSystem CMakeLists.txt
# 
# Main build configuration for the ThreadSystem library.
# This builds a high-performance, C++20 thread management system.
##################################################

# Specify minimum required CMake version for this project
cmake_minimum_required(VERSION 3.16)

# Define the project name, version, and language
project(ThreadSystem 
    VERSION 1.0.0
    DESCRIPTION "High-performance C++20 multithreading framework"
    HOMEPAGE_URL "https://github.com/kcenon/thread_system"
    LANGUAGES CXX
)

# Option to control whether ThreadSystem is built as a standalone project or as a submodule
# - OFF (default): Builds as standalone project with samples and tests
# - ON: Builds only the core libraries for integration into other projects
# Automatically detect if we're being built as a submodule
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # We're being built as a subdirectory, so default to submodule mode
    set(BUILD_THREADSYSTEM_AS_SUBMODULE ON CACHE BOOL "Build ThreadSystem as submodule" FORCE)
else()
    # We're the top-level project, so default to standalone mode
    option(BUILD_THREADSYSTEM_AS_SUBMODULE "Build ThreadSystem as submodule" OFF)
endif()
option(BUILD_DOCUMENTATION "Build Doxygen documentation" ON)

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

# Add custom cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include compiler checks module
include(CompilerChecks)

# Include coverage configuration
include(Coverage)

# Phase 3 T3.2: Include dependency conflict checker
include(dependency_checker)

# Perform compiler version check
check_compiler_version()

# Configure platform-specific settings
configure_platform_settings()

# Configure build types
configure_build_types()

# Check required headers
check_required_headers()

# Check C++ standard library features
check_cpp_stdlib_features()

##################################################
# Sanitizers and Static Analysis
##################################################

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if(CLANG_TIDY_EXE)
    message(STATUS "Enabling clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
  else()
    message(WARNING "clang-tidy not found; ENABLE_CLANG_TIDY ignored")
  endif()
endif()

function(apply_sanitizers target)
  if(ENABLE_ASAN)
    target_compile_options(${target} PRIVATE -fsanitize=address)
    target_link_libraries(${target} PRIVATE -fsanitize=address)
  endif()
  if(ENABLE_UBSAN)
    target_compile_options(${target} PRIVATE -fsanitize=undefined)
    target_link_libraries(${target} PRIVATE -fsanitize=undefined)
  endif()
  if(ENABLE_TSAN)
    target_compile_options(${target} PRIVATE -fsanitize=thread)
    target_link_libraries(${target} PRIVATE -fsanitize=thread)
  endif()
endfunction()

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
option(SET_STD_FILESYSTEM "Set use std::filesystem if it can use" ON)
option(SET_STD_RANGES "Set use std::ranges if it can use" ON)
option(SET_STD_CONCEPTS "Set use std::concepts if it can use" ON)


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

# First check basic std::format availability
check_cxx20_feature(std_format_basic "
  #include <format>
  #include <string>
  
  int main() {
    std::string s = std::format(\"{} {}\", \"Hello\", \"World\");
    return 0;
  }
" HAS_STD_FORMAT_BASIC)

# Then check formatter specialization support (more comprehensive test)
check_cxx20_feature(std_format_specialization "
  #include <format>
  #include <string>
  #include <string_view>
  
  // Test custom type formatting
  struct test_type {
    std::string to_string() const { return \"test\"; }
  };
  
  template<>
  struct std::formatter<test_type> : std::formatter<std::string_view> {
    template<typename FormatContext>
    auto format(const test_type& t, FormatContext& ctx) const {
      return std::formatter<std::string_view>::format(t.to_string(), ctx);
    }
  };
  
  int main() {
    // Test basic formatting
    std::string basic = std::format(\"{} {}\", \"Hello\", \"World\");
    
    // Test custom formatter
    test_type t;
    std::string custom = std::format(\"{}\", t);
    
    return 0;
  }
" HAS_STD_FORMAT_SPECIALIZATION)

# Windows-specific policy: Force fmt usage for maximum compatibility
if(WIN32)
  message(STATUS "🪟 Windows platform detected")
  message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
  message(STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")
  
  # Force fmt::format on Windows for maximum stability and compatibility
  set(WINDOWS_COMPILER_SUPPORTS_STD_FORMAT FALSE)
  
  message(STATUS "🛡️  Windows Policy: Force using fmt::format for maximum compatibility")
  message(STATUS "   Rationale:")
  message(STATUS "   - Avoids std::format inconsistencies across different Windows compilers")
  message(STATUS "   - Ensures consistent behavior between MSVC, MSYS2, and Clang")
  message(STATUS "   - Eliminates runtime compatibility issues with different Windows versions")
  message(STATUS "   - Provides stable, well-tested formatting across all Windows environments")
  
  if(MSVC)
    message(STATUS "📊 MSVC ${MSVC_VERSION} detected - using fmt::format")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "🐧 GCC ${CMAKE_CXX_COMPILER_VERSION} detected - using fmt::format")
    
    # Identify specific Windows GCC environments
    if(CMAKE_CXX_COMPILER MATCHES "msys2" OR CMAKE_CXX_COMPILER MATCHES "MSYS2")
      message(STATUS "🔧 MSYS2 environment confirmed")
    else()
      message(STATUS "🔧 Windows GCC environment detected")
    endif()
    
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "🦙 Clang ${CMAKE_CXX_COMPILER_VERSION} detected - using fmt::format")
    
    if(CMAKE_CXX_COMPILER MATCHES "clang-cl")
      message(STATUS "🔧 clang-cl (MSVC-compatible mode) detected")
    else()
      message(STATUS "🔧 Standard Clang on Windows detected")
    endif()
    
  else()
    message(STATUS "❓ Unknown Windows compiler: ${CMAKE_CXX_COMPILER_ID} - using fmt::format")
  endif()
  
  # Option to override Windows policy (for advanced users)
  option(WINDOWS_ALLOW_STD_FORMAT "Allow std::format on Windows (not recommended)" OFF)
  if(WINDOWS_ALLOW_STD_FORMAT)
    message(STATUS "⚠️  WARNING: WINDOWS_ALLOW_STD_FORMAT enabled!")
    message(STATUS "   This may cause compatibility issues. Use at your own risk.")
    set(WINDOWS_COMPILER_SUPPORTS_STD_FORMAT TRUE)
  endif()
  
else()
  # Non-Windows platforms (Linux, macOS, etc.)
  set(WINDOWS_COMPILER_SUPPORTS_STD_FORMAT TRUE)  # Will be checked by other tests
  message(STATUS "🐧🍎 Non-Windows platform detected: ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# Determine if we should use std::format
set(USE_STD_FORMAT_ENABLED FALSE)

if(HAS_STD_FORMAT_BASIC AND HAS_STD_FORMAT_SPECIALIZATION AND WINDOWS_COMPILER_SUPPORTS_STD_FORMAT)
  # Final verification: compile a real-world example that matches our usage
  message(STATUS "All preliminary std::format checks passed - running compile test...")
  
  try_compile(STD_FORMAT_COMPILE_TEST
    ${CMAKE_BINARY_DIR}/format_test
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_std_format.cpp
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_VARIABLE COMPILE_OUTPUT
  )
  
  if(STD_FORMAT_COMPILE_TEST)
    set(USE_STD_FORMAT_ENABLED TRUE)
    message(STATUS "std::format compile test: PASSED")
  else()
    message(STATUS "std::format compile test: FAILED")
    if(COMPILE_OUTPUT)
      message(STATUS "Compile error output: ${COMPILE_OUTPUT}")
    endif()
  endif()
else()
  message(STATUS "Skipping std::format compile test - prerequisite checks failed")
  message(STATUS "  - Basic format: ${HAS_STD_FORMAT_BASIC}")
  message(STATUS "  - Specialization: ${HAS_STD_FORMAT_SPECIALIZATION}")  
  message(STATUS "  - Compiler support: ${WINDOWS_COMPILER_SUPPORTS_STD_FORMAT}")
endif()

# Windows policy enforcement
if(WIN32 AND USE_STD_FORMAT_ENABLED)
  # This should rarely happen due to the Windows policy above
  message(STATUS "⚠️  Windows with std::format enabled (policy override detected)")
  message(STATUS "   This configuration is not recommended for production use")
  message(STATUS "   To disable override: set WINDOWS_ALLOW_STD_FORMAT to OFF")
endif()

# Global option to force fmt usage (applies to all platforms)
option(FORCE_FMT_FORMAT "Force use of fmt library instead of std::format on all platforms" OFF)
if(FORCE_FMT_FORMAT)
  set(USE_STD_FORMAT_ENABLED FALSE)
  message(STATUS "🔧 Global override: Forced to use fmt::format on all platforms")
endif()

# Apply the decision
if(USE_STD_FORMAT_ENABLED)
  add_definitions(-DUSE_STD_FORMAT)
  set(USE_STD_FORMAT TRUE CACHE BOOL "Using std::format" FORCE)
  message(STATUS "✅ Using std::format - all compatibility checks passed")
  message(STATUS "   - Basic std::format: ${HAS_STD_FORMAT_BASIC}")
  message(STATUS "   - Formatter specialization: ${HAS_STD_FORMAT_SPECIALIZATION}")
  message(STATUS "   - Compiler compatibility: ${WINDOWS_COMPILER_SUPPORTS_STD_FORMAT}")
  message(STATUS "   - Compile test: ${STD_FORMAT_COMPILE_TEST}")
else()
  set(USE_STD_FORMAT FALSE CACHE BOOL "Not using std::format" FORCE)
  if(WIN32)
    message(STATUS "🛡️  Using fmt::format (Windows Policy)")
    message(STATUS "   Windows builds use fmt::format for maximum compatibility")
    message(STATUS "   - Platform: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "   - Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    message(STATUS "   - Policy override available: -DWINDOWS_ALLOW_STD_FORMAT=ON")
  else()
    message(STATUS "❌ Using fmt::format fallback - std::format not fully supported")
    message(STATUS "   - Basic std::format: ${HAS_STD_FORMAT_BASIC}")
    message(STATUS "   - Formatter specialization: ${HAS_STD_FORMAT_SPECIALIZATION}")
    message(STATUS "   - Compiler compatibility: ${WINDOWS_COMPILER_SUPPORTS_STD_FORMAT}")
    message(STATUS "   - Compile test: ${STD_FORMAT_COMPILE_TEST}")
    message(STATUS "   - Platform: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "   - Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
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
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has support for std::jthread.")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have support for std::jthread, so will use the std::thread instead.")
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
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has support for std::chrono::current_zone.")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have support for std::chrono::current_zone, so will use the time_t instead.")
endif()

##################################################
# Test for std::span (C++20 feature)
##################################################

# Options to control whether std::span is used
option(SET_STD_SPAN "Set use std::span if it can use" ON)

check_cxx20_feature(std_span_basic "
  #include <span>
  #include <vector>
  
  int main() {
    std::vector<int> vec = {1, 2, 3};
    std::span<int> s(vec);
    return s.size();
  }
" HAS_STD_SPAN_BASIC)

check_cxx20_feature(std_span_advanced "
  #include <span>
  #include <vector>
  #include <array>
  
  int main() {
    // Test vector constructor
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::span<int> s1(vec);
    
    // Test array constructor
    std::array<int, 3> arr = {1, 2, 3};
    std::span<int> s2(arr);
    
    // Test C-array constructor
    int raw_array[4] = {1, 2, 3, 4};
    std::span<int> s3(raw_array, 4);
    
    // Test subspan
    auto subview = s1.subspan(1, 2);
    
    return 0;
  }
" HAS_STD_SPAN_ADVANCED)

# Final verification: compile a real-world span test
if(HAS_STD_SPAN_BASIC AND HAS_STD_SPAN_ADVANCED)
  message(STATUS "All preliminary std::span checks passed - running compile test...")
  
  try_compile(STD_SPAN_COMPILE_TEST
    ${CMAKE_BINARY_DIR}/span_test
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_std_span.cpp
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_VARIABLE COMPILE_OUTPUT
  )
  
  if(STD_SPAN_COMPILE_TEST AND SET_STD_SPAN)
    add_definitions(-DUSE_STD_SPAN)  # Define macro to use std::span in code
    message(STATUS "✅ Using std::span - all compatibility checks passed")
    message(STATUS "   - Basic std::span: ${HAS_STD_SPAN_BASIC}")
    message(STATUS "   - Advanced features: ${HAS_STD_SPAN_ADVANCED}")
    message(STATUS "   - Compile test: ${STD_SPAN_COMPILE_TEST}")
  else()
    message(STATUS "❌ Using fallback span implementation - std::span not fully supported")
    message(STATUS "   - Basic std::span: ${HAS_STD_SPAN_BASIC}")
    message(STATUS "   - Advanced features: ${HAS_STD_SPAN_ADVANCED}")
    message(STATUS "   - Compile test: ${STD_SPAN_COMPILE_TEST}")
    message(STATUS "   - Platform: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "   - Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
else()
  message(STATUS "Skipping std::span compile test - prerequisite checks failed")
  message(STATUS "  - Basic span: ${HAS_STD_SPAN_BASIC}")
  message(STATUS "  - Advanced features: ${HAS_STD_SPAN_ADVANCED}")  
endif()

##################################################
# Test for std::filesystem (C++17/20 feature)
##################################################

# Check for std::filesystem support
check_cxx_source_compiles("
  #include <filesystem>
  int main() {
    std::filesystem::path p(\"/tmp\");
    return std::filesystem::exists(p) ? 0 : 1;
  }
" HAS_STD_FILESYSTEM)

if(SET_STD_FILESYSTEM AND HAS_STD_FILESYSTEM)
  try_compile(STD_FILESYSTEM_COMPILE_TEST
    ${CMAKE_BINARY_DIR}/filesystem_test
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_std_filesystem.cpp
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_VARIABLE COMPILE_OUTPUT
  )
  
  if(STD_FILESYSTEM_COMPILE_TEST)
    add_definitions(-DUSE_STD_FILESYSTEM)
    message(STATUS "✅ Using std::filesystem - compile test passed")
  else()
    message(STATUS "❌ std::filesystem compile test failed")
  endif()
endif()

##################################################
# Test for std::ranges (C++20 feature)
##################################################

# Check for std::ranges support
check_cxx_source_compiles("
  #include <ranges>
  #include <vector>
  int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto even = v | std::views::filter([](int i) { return i % 2 == 0; });
    return 0;
  }
" HAS_STD_RANGES)

if(SET_STD_RANGES AND HAS_STD_RANGES)
  try_compile(STD_RANGES_COMPILE_TEST
    ${CMAKE_BINARY_DIR}/ranges_test
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_std_ranges.cpp
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_VARIABLE COMPILE_OUTPUT
  )
  
  if(STD_RANGES_COMPILE_TEST)
    add_definitions(-DUSE_STD_RANGES)
    message(STATUS "✅ Using std::ranges - compile test passed")
  else()
    message(STATUS "❌ std::ranges compile test failed")
  endif()
endif()

##################################################
# Test for std::concepts (C++20 feature)
##################################################

# Check for std::concepts support
check_cxx_source_compiles("
  #include <concepts>
  template<typename T>
  concept Addable = requires(T a, T b) { a + b; };
  int main() {
    static_assert(Addable<int>);
    return 0;
  }
" HAS_STD_CONCEPTS)

if(SET_STD_CONCEPTS AND HAS_STD_CONCEPTS)
  try_compile(STD_CONCEPTS_COMPILE_TEST
    ${CMAKE_BINARY_DIR}/concepts_test
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_std_concepts.cpp
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_VARIABLE COMPILE_OUTPUT
  )
  
  if(STD_CONCEPTS_COMPILE_TEST)
    add_definitions(-DUSE_STD_CONCEPTS)
    message(STATUS "✅ Using std::concepts - compile test passed")
  else()
    message(WARNING "⚠️  std::concepts compile test failed")
    message(STATUS "Build will continue without concepts support")
  endif()
endif()

##################################################
# Build Configuration
##################################################

if(NOT BUILD_THREADSYSTEM_AS_SUBMODULE)
  # When building as a standalone project:
  
  # Set output directories for binaries, libraries, and archives
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

  # Warn if vcpkg toolchain is not provided (but allow build to continue)
  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(WARNING "vcpkg toolchain file not provided. Some dependencies may not be found automatically.")
    message(STATUS "To use vcpkg, provide -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg_toolchain>")
  else()
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
  endif()

  # Phase 3 T3.2: Run dependency conflict checks if requested
  if(CHECK_DEPENDENCIES)
    check_dependency_conflicts()
  endif()

  # Set up include directories for new structure
  set(THREAD_SYSTEM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
  set(THREAD_SYSTEM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

  # Add include directories for both new and old structures
  include_directories(${THREAD_SYSTEM_INCLUDE_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})  # For legacy structure

  # Check if we have the new structure or old structure
  if(EXISTS ${THREAD_SYSTEM_INCLUDE_DIR}/kcenon/thread AND EXISTS ${THREAD_SYSTEM_SOURCE_DIR})
    message(STATUS "Using new directory structure")

    # Collect source files from new structure
    file(GLOB_RECURSE THREAD_SYSTEM_HEADERS
      ${THREAD_SYSTEM_INCLUDE_DIR}/kcenon/thread/*.h
    )

    file(GLOB_RECURSE THREAD_SYSTEM_SOURCES
      ${THREAD_SYSTEM_SOURCE_DIR}/*.cpp
    )

    # If no sources found in new structure, this is a partial migration
    list(LENGTH THREAD_SYSTEM_SOURCES SOURCE_COUNT)
    if(SOURCE_COUNT LESS 10)
      message(STATUS "New structure has limited sources, using hybrid approach")
      set(USE_LEGACY_BUILD TRUE)
    else()
      # Create the main library with new structure
      add_library(ThreadSystem STATIC
        ${THREAD_SYSTEM_SOURCES}
        ${THREAD_SYSTEM_HEADERS}
      )

      # Set target properties
      target_include_directories(ThreadSystem
        PUBLIC
          $<BUILD_INTERFACE:${THREAD_SYSTEM_INCLUDE_DIR}>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:include>
      )
      set(USE_LEGACY_BUILD FALSE)
    endif()
  else()
    message(STATUS "New structure not complete, using legacy build")
    set(USE_LEGACY_BUILD TRUE)
  endif()

  # Add legacy subdirectories if using legacy build
  if(USE_LEGACY_BUILD)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/utilities)
      add_subdirectory(utilities)        # Utilities library
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interfaces)
      add_subdirectory(interfaces)       # Public interfaces
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core)
      add_subdirectory(core)             # Core (thread_base, jobs, sync)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/implementations)
      add_subdirectory(implementations)  # Implementations (thread_pool, typed_thread_pool, lockfree)
    endif()
  endif()

  # Examples and tests
  if(NOT DEFINED BUILD_SAMPLES OR BUILD_SAMPLES)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
      add_subdirectory(examples)          # Example applications
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples)
      add_subdirectory(samples)          # Legacy sample applications
    endif()
  endif()

  # Tests
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)             # New test structure
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest)
    add_subdirectory(unittest)         # Legacy unit tests
  endif()

  # Add benchmarks directory if requested
  option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)
  if(BUILD_BENCHMARKS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmarks)
      add_subdirectory(tests/benchmarks)  # New benchmark location
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
      add_subdirectory(benchmarks)  # Legacy benchmarks
    endif()
  endif()

  # Documentation (Doxygen)
  if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
      add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
    else()
      message(WARNING "Doxygen not found; 'docs' target will not be available")
    endif()
  endif()
else()
  # When building as a submodule of another project:
  # Only build the core library (no samples or tests)
  # Build core components with new structure
  add_subdirectory(interfaces)
  add_subdirectory(utilities)
  add_subdirectory(core)
endif()

##################################################
# Installation Configuration
##################################################

# Only configure installation when building as standalone
if(NOT BUILD_THREADSYSTEM_AS_SUBMODULE)
  include(GNUInstallDirs)
  
  # Phase 2 T2.2: Install header files with priority to interface headers
  
  # Phase 2 T2.2: Priority installation of interface headers
  
  # Install headers from new structure
  install(DIRECTORY include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          COMPONENT Development
          FILES_MATCHING PATTERN "*.h")

  # Install legacy interfaces for backward compatibility
  install(DIRECTORY interfaces/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thread_system/interfaces
          COMPONENT Development
          FILES_MATCHING PATTERN "*.h")
  
  # Utility headers (commonly used)
  install(DIRECTORY utilities/include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thread_system/utilities
          COMPONENT Development
          FILES_MATCHING PATTERN "*.h")

  # Core implementation headers are now in include/kcenon/thread/core/

  # Implementation headers (optional, for advanced customization)
  install(DIRECTORY implementations/thread_pool/include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thread_system/implementations/thread_pool
          COMPONENT Implementation
          FILES_MATCHING PATTERN "*.h")

  install(DIRECTORY implementations/typed_thread_pool/include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thread_system/implementations/typed_thread_pool
          COMPONENT Implementation
          FILES_MATCHING PATTERN "*.h" PATTERN "*.tpp")

  install(DIRECTORY implementations/lockfree/include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thread_system/implementations/lockfree
          COMPONENT Implementation
          FILES_MATCHING PATTERN "*.h")
  
  # Monitoring headers disabled - moved to separate project
  # install(DIRECTORY sources/monitoring/
  #         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thread_system/monitoring
  #         FILES_MATCHING PATTERN "*.h")
  
  # Install libraries based on what was built
  if(USE_LEGACY_BUILD)
    # Legacy targets
    set(INSTALL_TARGETS "")
    if(TARGET utilities)
      list(APPEND INSTALL_TARGETS utilities)
    endif()
    if(TARGET interfaces)
      list(APPEND INSTALL_TARGETS interfaces)
    endif()
    if(TARGET thread_base)
      list(APPEND INSTALL_TARGETS thread_base)
    endif()
    if(TARGET thread_pool)
      list(APPEND INSTALL_TARGETS thread_pool)
    endif()
    if(TARGET typed_thread_pool)
      list(APPEND INSTALL_TARGETS typed_thread_pool)
    endif()
    if(TARGET lockfree)
      list(APPEND INSTALL_TARGETS lockfree)
    endif()

    if(INSTALL_TARGETS)
      install(TARGETS ${INSTALL_TARGETS}
              EXPORT ThreadSystemTargets
              ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
              LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
              RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
  else()
    # New structure - install ThreadSystem library
    if(TARGET ThreadSystem)
      install(TARGETS ThreadSystem
              EXPORT ThreadSystemTargets
              ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
              LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
              RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
  endif()
  
  # Phase 2 T2.2: Install targets with standardized namespace thread_system::
  install(EXPORT ThreadSystemTargets
          FILE thread_system-targets.cmake
          NAMESPACE thread_system::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/thread_system)
          
  # Legacy compatibility: also install with old naming and namespace
  install(EXPORT ThreadSystemTargets
          FILE ThreadSystemTargets.cmake
          NAMESPACE ThreadSystem::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadSystem)
  
  # Create and install config file
  include(CMakePackageConfigHelpers)
  
  # Phase 2 T2.1: Standardized CMake config files with hyphenated names
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/thread_system-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/thread_system-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/thread_system
  )
  
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/thread_system-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/thread_system-config-version.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/thread_system
  )
  
  # Legacy compatibility: also generate old-style config files
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ThreadSystemConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadSystem
  )
  
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  
  # Install new standardized config files (Phase 2 T2.1)
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/thread_system-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/thread_system-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/thread_system
  )
  
  # Install legacy config files for backward compatibility
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadSystem
  )
  
  # Phase 2 T2.2: Create and install pkg-config file
  set(PKG_CONFIG_FEATURE_FLAGS "")
  if(USE_STD_FORMAT)
    set(PKG_CONFIG_FEATURE_FLAGS "${PKG_CONFIG_FEATURE_FLAGS} -DUSE_STD_FORMAT")
  endif()
  if(USE_STD_JTHREAD)
    set(PKG_CONFIG_FEATURE_FLAGS "${PKG_CONFIG_FEATURE_FLAGS} -DUSE_STD_JTHREAD")
  endif()
  if(USE_STD_CHRONO_CURRENT_ZONE)
    set(PKG_CONFIG_FEATURE_FLAGS "${PKG_CONFIG_FEATURE_FLAGS} -DUSE_STD_CHRONO_CURRENT_ZONE")
  endif()
  
  # Get Iconv libraries for pkg-config
  if(TARGET Iconv::Iconv)
    get_target_property(ICONV_LIBRARIES Iconv::Iconv INTERFACE_LINK_LIBRARIES)
    if(NOT ICONV_LIBRARIES)
      set(ICONV_LIBRARIES "")
    endif()
  else()
    set(ICONV_LIBRARIES "")
  endif()
  
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/thread_system.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/thread_system.pc
    @ONLY
  )
  
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/thread_system.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  )
  
  # Phase 3 T3.1: Install dependency documentation
  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/dependency_compatibility_matrix.md
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/license_compatibility.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/dependencies
    COMPONENT Documentation
  )
  
  # Install main documentation
  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
    ${CMAKE_CURRENT_SOURCE_DIR}/DEPENDENCY_IMPROVEMENT_SRD.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT Documentation
  )
endif()

##################################################
# Code Coverage Configuration
##################################################

# Create coverage report target if coverage is enabled
if(ENABLE_COVERAGE)
  create_coverage_report()
  
  message(STATUS "")
  message(STATUS "Code coverage is enabled.")
  message(STATUS "To generate coverage report:")
  message(STATUS "  1. Build with: cmake -B build -DENABLE_COVERAGE=ON")
  message(STATUS "  2. Run tests: cmake --build build")
  message(STATUS "  3. Generate report: cmake --build build --target coverage")
  message(STATUS "")
endif()
