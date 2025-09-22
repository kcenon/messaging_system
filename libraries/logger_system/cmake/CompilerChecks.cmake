# CompilerChecks.cmake for logger_system
# Enhanced version with Windows CI compatibility

##################################################
# Minimum Compiler Version Requirements
##################################################

# Define minimum compiler versions based on C++ standard
if(CMAKE_CXX_STANDARD EQUAL 17)
    # C++17 minimum requirements (more lenient)
    set(MIN_GCC_VERSION "7.0")
    set(MIN_CLANG_VERSION "5.0")
    set(MIN_APPLECLANG_VERSION "9.0")
    set(MIN_MSVC_VERSION "19.14")  # Visual Studio 2017 15.7
    message(STATUS "Using C++17 compatibility mode - relaxed compiler requirements")
else()
    # C++20 minimum requirements 
    set(MIN_GCC_VERSION "10.0")
    set(MIN_CLANG_VERSION "11.0")
    set(MIN_APPLECLANG_VERSION "12.0")
    set(MIN_MSVC_VERSION "19.26")  # Visual Studio 2019 16.6
    message(STATUS "Using C++20 mode - stricter compiler requirements")
endif()

# Check compiler version
function(check_compiler_version)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MIN_GCC_VERSION})
            message(FATAL_ERROR "GCC version ${CMAKE_CXX_COMPILER_VERSION} is not supported. Minimum required version is ${MIN_GCC_VERSION}")
        endif()
        message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION} - C++20 support verified")
        
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MIN_CLANG_VERSION})
            message(FATAL_ERROR "Clang version ${CMAKE_CXX_COMPILER_VERSION} is not supported. Minimum required version is ${MIN_CLANG_VERSION}")
        endif()
        message(STATUS "Clang ${CMAKE_CXX_COMPILER_VERSION} - C++20 support verified")
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MIN_APPLECLANG_VERSION})
            message(FATAL_ERROR "Apple Clang version ${CMAKE_CXX_COMPILER_VERSION} is not supported. Minimum required version is ${MIN_APPLECLANG_VERSION}")
        endif()
        message(STATUS "Apple Clang ${CMAKE_CXX_COMPILER_VERSION} - C++20 support verified")
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(MSVC_VERSION LESS 1926)
            message(FATAL_ERROR "MSVC version ${MSVC_VERSION} is not supported. Minimum required version is ${MIN_MSVC_VERSION} (Visual Studio 2019 16.6)")
        endif()
        message(STATUS "MSVC ${MSVC_VERSION} - C++20 support verified")
        
    else()
        message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
        message(WARNING "C++20 support cannot be verified. Build may fail.")
    endif()
endfunction()

##################################################
# Platform Configuration
##################################################
function(configure_platform_settings)
    message(STATUS "========================================")
    message(STATUS "Platform Configuration:")
    message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "  Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "========================================")
    
    # Platform-specific configurations
    if(WIN32)
        # Windows-specific settings
        add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
        if(MSVC)
            # MSVC-specific warnings and optimizations
            add_compile_options(/W4 /permissive- /Zc:__cplusplus)
            # Enable proper C++20 standard reporting
            add_compile_options(/std:c++20)
        endif()
    elseif(APPLE)
        # macOS-specific settings
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_warn_duplicate_libraries")
    elseif(UNIX)
        # Linux/Unix-specific settings
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_compile_options(-Wall -Wextra -Wpedantic)
        endif()
    endif()
endfunction()

##################################################
# Build Type Configuration
##################################################
function(configure_build_types)
    # Set default build type if not specified
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
        message(STATUS "Build type not specified, defaulting to Release")
    endif()
    
    # Configure build-specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_compile_options(-O3 -DNDEBUG)
            # Enable architecture-specific optimizations on non-CI builds
            if(NOT DEFINED ENV{CI})
                add_compile_options(-march=native)
            endif()
        elseif(MSVC)
            add_compile_options(/O2 /DNDEBUG)
        endif()
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_compile_options(-g -O0)
        elseif(MSVC)
            add_compile_options(/Od /Zi)
        endif()
    endif()
endfunction()

##################################################
# Required Headers Check
##################################################
function(check_required_headers)
    include(CheckIncludeFileCXX)
    
    set(REQUIRED_HEADERS
        algorithm
        atomic
        chrono
        condition_variable
        deque
        functional
        future
        iostream
        memory
        mutex
        optional
        string
        thread
        tuple
        utility
        vector
        string_view
        variant
        any
    )
    
    foreach(HEADER ${REQUIRED_HEADERS})
        check_include_file_cxx(${HEADER} HAS_${HEADER})
        if(NOT HAS_${HEADER})
            message(FATAL_ERROR "Required header <${HEADER}> not found")
        endif()
    endforeach()
    
    message(STATUS "All required C++ headers found")
endfunction()

##################################################
# C++20 Feature Checks (Enhanced for Windows CI)
##################################################
function(check_cpp_features)
    include(CheckCXXSourceCompiles)
    
    # Save original settings
    set(ORIGINAL_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(ORIGINAL_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    set(ORIGINAL_CMAKE_TRY_COMPILE_TARGET_TYPE ${CMAKE_TRY_COMPILE_TARGET_TYPE})
    
    # Windows-specific configuration (for both CI and local builds)
    if(WIN32)
        message(STATUS "Configuring Windows-specific C++ feature detection")
        
        # Always use static library for try_compile on Windows to avoid linking issues
        set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY" CACHE STRING "" FORCE)
        
        if(MSVC)
            # MSVC needs these flags for proper C++20 feature detection
            set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /std:c++20 /permissive- /Zc:__cplusplus /EHsc")
            
            # Add Windows SDK libraries if needed
            if(DEFINED ENV{WindowsSdkDir})
                message(STATUS "Windows SDK found at: $ENV{WindowsSdkDir}")
            endif()
            
            # For GitHub Actions Windows runners
            if(DEFINED ENV{GITHUB_ACTIONS})
                message(STATUS "GitHub Actions Windows runner detected")
                # GitHub Actions specific workarounds
                set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /bigobj")
            endif()
            
        endif()
    elseif(UNIX)
        # Unix/Linux configuration
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++20")
        endif()
    endif()
    
    # CI environment detection
    if(DEFINED ENV{CI} OR DEFINED ENV{GITHUB_ACTIONS})
        message(STATUS "CI environment detected - applying additional workarounds")
    endif()
    
    # Check for C++20 concepts
    message(STATUS "Checking for C++20 concepts...")
    if(MSVC AND MSVC_VERSION LESS 1923)
        message(STATUS "⚠️  MSVC version too old for concepts (need VS 2019 16.3+)")
        set(HAS_CONCEPTS FALSE)
    else()
        check_cxx_source_compiles("
            #include <concepts>
            template<typename T>
            concept Incrementable = requires(T t) {
                ++t;
            };
            template<Incrementable T>
            void increment(T& t) { ++t; }
            int main() { 
                int x = 0;
                increment(x);
                return 0; 
            }
        " HAS_CONCEPTS)
    endif()
    
    if(HAS_CONCEPTS)
        message(STATUS "✅ C++20 concepts support detected")
        add_compile_definitions(HAS_CONCEPTS=1)
    else()
        message(STATUS "⚠️  C++20 concepts not fully supported")
    endif()
    
    # Check for std::format
    message(STATUS "Checking for std::format...")
    if(MSVC AND MSVC_VERSION LESS 1929)
        message(STATUS "⚠️  MSVC version too old for std::format (need VS 2019 16.10+)")
        set(HAS_STD_FORMAT FALSE)
    else()
        check_cxx_source_compiles("
            #include <format>
            #include <string>
            int main() {
                std::string s = std::format(\"Hello, {}!\", \"World\");
                return s.empty() ? 1 : 0;
            }
        " HAS_STD_FORMAT)
    endif()
    
    if(HAS_STD_FORMAT)
        message(STATUS "✅ std::format support detected")
        add_compile_definitions(USE_STD_FORMAT=1)
    else()
        message(STATUS "⚠️  std::format not available, using fmt library as fallback")
    endif()
    
    # Check for std::jthread
    message(STATUS "Checking for std::jthread...")
    if(MSVC AND MSVC_VERSION LESS 1928)
        message(STATUS "⚠️  MSVC version too old for std::jthread (need VS 2019 16.8+)")
        set(HAS_STD_JTHREAD FALSE)
    else()
        check_cxx_source_compiles("
            #include <thread>
            #include <stop_token>
            int main() {
                std::jthread t([](){});
                return 0;
            }
        " HAS_STD_JTHREAD)
    endif()
    
    if(HAS_STD_JTHREAD)
        message(STATUS "✅ std::jthread support detected")
        add_compile_definitions(HAS_STD_JTHREAD=1)
    else()
        message(STATUS "⚠️  std::jthread not available, using std::thread")
    endif()
    
    # Additional C++17 features for compatibility (like thread_system)
    message(STATUS "Checking for C++17 compatibility features...")
    
    # Check for std::optional
    check_cxx_source_compiles("
        #include <optional>
        int main() {
            std::optional<int> opt = 42;
            return opt.value();
        }
    " HAS_STD_OPTIONAL)
    
    if(HAS_STD_OPTIONAL)
        add_compile_definitions(HAS_STD_OPTIONAL=1)
        message(STATUS "✅ std::optional is available")
    endif()
    
    # Check for std::variant
    check_cxx_source_compiles("
        #include <variant>
        int main() {
            std::variant<int, double> v = 42;
            return std::get<int>(v);
        }
    " HAS_STD_VARIANT)
    
    if(HAS_STD_VARIANT)
        add_compile_definitions(HAS_STD_VARIANT=1)
        message(STATUS "✅ std::variant is available")
    endif()
    
    # Check for std::string_view
    check_cxx_source_compiles("
        #include <string_view>
        int main() {
            std::string_view sv = \"hello\";
            return sv.length();
        }
    " HAS_STD_STRING_VIEW)
    
    if(HAS_STD_STRING_VIEW)
        add_compile_definitions(HAS_STD_STRING_VIEW=1)
        message(STATUS "✅ std::string_view is available")
    endif()
    
    # Restore original settings
    set(CMAKE_REQUIRED_FLAGS ${ORIGINAL_CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_LIBRARIES ${ORIGINAL_CMAKE_REQUIRED_LIBRARIES})
    
    # Reset Windows-specific settings
    if(WIN32)
        if(NOT "${ORIGINAL_CMAKE_TRY_COMPILE_TARGET_TYPE}" STREQUAL "")
            set(CMAKE_TRY_COMPILE_TARGET_TYPE ${ORIGINAL_CMAKE_TRY_COMPILE_TARGET_TYPE} CACHE STRING "" FORCE)
        else()
            unset(CMAKE_TRY_COMPILE_TARGET_TYPE CACHE)
        endif()
    endif()
    
    message(STATUS "C++ feature detection completed")
endfunction()

##################################################
# Optional: Add thread_system's check_cpp_stdlib_features for compatibility
##################################################
function(check_cpp_stdlib_features)
    # This function can be called by code expecting thread_system compatibility
    message(STATUS "check_cpp_stdlib_features called - redirecting to check_cpp_features")
    check_cpp_features()
endfunction()