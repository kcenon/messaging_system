# MonitoringCompatibility.cmake
# C++17/C++20 feature detection and compatibility layer for monitoring_system

##################################################
# Feature Detection Functions
##################################################

# Test for std::jthread availability
function(check_std_jthread)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <thread>
        #include <stop_token>
        int main() {
            std::jthread t([](){});
            t.request_stop();
            return 0;
        }
    " HAS_STD_JTHREAD)
    
    if(HAS_STD_JTHREAD)
        message(STATUS "std::jthread is available")
        add_definitions(-DMONITORING_HAS_STD_JTHREAD)
    else()
        message(STATUS "std::jthread not available - using std::thread with manual stop management")
    endif()
endfunction()

# Test for concepts availability
function(check_concepts)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <concepts>
        template<typename T>
        concept Numeric = std::integral<T> || std::floating_point<T>;
        template<Numeric T>
        T process(T value) { return value * 2; }
        int main() {
            return process(42);
        }
    " HAS_CONCEPTS)
    
    if(HAS_CONCEPTS)
        message(STATUS "C++20 concepts are available")
        add_definitions(-DMONITORING_HAS_CONCEPTS)
    else()
        message(STATUS "C++20 concepts not available - using SFINAE fallback")
    endif()
endfunction()

# Test for std::span availability
function(check_std_span)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <span>
        #include <array>
        int main() {
            std::array<int, 5> arr = {1, 2, 3, 4, 5};
            std::span<int> s(arr);
            return s.size();
        }
    " HAS_STD_SPAN)
    
    if(HAS_STD_SPAN)
        message(STATUS "std::span is available")
        add_definitions(-DMONITORING_HAS_STD_SPAN)
    else()
        message(STATUS "std::span not available - using pointer/size pairs")
    endif()
endfunction()

# Test for atomic wait/notify operations
function(check_atomic_wait)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <atomic>
        int main() {
            std::atomic<int> flag{0};
            flag.wait(0);
            flag.notify_one();
            return 0;
        }
    " HAS_ATOMIC_WAIT)
    
    if(HAS_ATOMIC_WAIT)
        message(STATUS "Atomic wait/notify operations are available")
        add_definitions(-DMONITORING_HAS_ATOMIC_WAIT)
    else()
        message(STATUS "Atomic wait/notify not available - using condition variables")
    endif()
endfunction()

# Test for std::barrier availability
function(check_std_barrier)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <barrier>
        int main() {
            std::barrier sync_point(2);
            sync_point.arrive_and_wait();
            return 0;
        }
    " HAS_STD_BARRIER)
    
    if(HAS_STD_BARRIER)
        message(STATUS "std::barrier is available")
        add_definitions(-DMONITORING_HAS_STD_BARRIER)
    else()
        message(STATUS "std::barrier not available - using condition variable implementation")
    endif()
endfunction()

##################################################
# Main Compatibility Check Function
##################################################

function(configure_monitoring_compatibility)
    message(STATUS "========================================")
    message(STATUS "Monitoring System - Feature Detection:")
    message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
    
    if(CMAKE_CXX_STANDARD EQUAL 17)
        message(STATUS "  Mode: C++17 Compatibility")
        add_definitions(-DMONITORING_CPP17_MODE)
        
        # In C++17 mode, disable all C++20 features but still check formatting
        message(STATUS "  - std::jthread: DISABLED (using std::thread)")
        message(STATUS "  - concepts: DISABLED (using SFINAE)")
        message(STATUS "  - std::span: DISABLED (using pointer/size)")
        message(STATUS "  - atomic wait/notify: DISABLED (using condition variables)")
        message(STATUS "  - std::barrier: DISABLED (using custom implementation)")
        
        # Check formatting support even in C++17 mode
        check_formatting_support()
        
    else()
        message(STATUS "  Mode: C++20 Enhanced")
        add_definitions(-DMONITORING_CPP20_MODE)
        
        # Test each C++20 feature
        check_std_jthread()
        check_concepts()
        check_std_span()
        check_atomic_wait()
        check_std_barrier()
        check_formatting_support()
    endif()
    
    message(STATUS "========================================")
endfunction()

##################################################
# Formatting Library Management
##################################################

# Test for formatting libraries with vcpkg fmt fallback
function(check_formatting_support)
    include(CheckCXXSourceCompiles)
    
    # First try std::format (skip in C++17 mode to force fallback testing)
    if(NOT CMAKE_CXX_STANDARD EQUAL 17 AND NOT DEFINED MONITORING_CPP17_MODE)
        set(CMAKE_REQUIRED_FLAGS "-std=c++20")
        check_cxx_source_compiles("
            #include <format>
            int main() {
                auto s = std::format(\"Performance: {:.2f}%\", 95.5);
                return 0;
            }
        " HAS_STD_FORMAT)
    else()
        message(STATUS "🔒 Skipping std::format check in C++17 mode")
        set(HAS_STD_FORMAT FALSE)
    endif()
    
    if(HAS_STD_FORMAT)
        message(STATUS "✅ std::format is available for monitoring")
        add_definitions(-DMONITORING_USE_STD_FORMAT)
        set(MONITORING_FORMAT_BACKEND "std::format" CACHE INTERNAL "Monitoring format backend")
        set(MONITORING_FORMAT_BACKEND "std::format" PARENT_SCOPE)
    else()
        message(STATUS "⚠️ std::format not available - checking for fmt library")
        
        # Try to find fmt through vcpkg or system
        find_package(fmt CONFIG QUIET)
        if(fmt_FOUND)
            message(STATUS "✅ fmt library found - using fmt::format for monitoring")
            add_definitions(-DMONITORING_USE_FMT)
            set(MONITORING_FORMAT_BACKEND "fmt::format" CACHE INTERNAL "Monitoring format backend")
            set(MONITORING_FORMAT_BACKEND "fmt::format" PARENT_SCOPE)
            set(MONITORING_FMT_TARGET fmt::fmt CACHE INTERNAL "Monitoring fmt target")
            set(MONITORING_FMT_TARGET fmt::fmt PARENT_SCOPE)
        else()
            # Try to find fmt through vcpkg explicitly
            if(DEFINED ENV{VCPKG_ROOT})
                message(STATUS "🔍 Trying to find fmt through vcpkg for monitoring...")
                find_package(fmt CONFIG QUIET PATHS $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET})
                if(fmt_FOUND)
                    message(STATUS "✅ fmt library found via vcpkg - using fmt::format for monitoring")
                    add_definitions(-DMONITORING_USE_FMT)
                    set(MONITORING_FORMAT_BACKEND "fmt::format" CACHE INTERNAL "Monitoring format backend")
                    set(MONITORING_FORMAT_BACKEND "fmt::format" PARENT_SCOPE)
                    set(MONITORING_FMT_TARGET fmt::fmt CACHE INTERNAL "Monitoring fmt target")
                    set(MONITORING_FMT_TARGET fmt::fmt PARENT_SCOPE)
                endif()
            endif()
            
            if(NOT fmt_FOUND)
                message(STATUS "⚠️ Neither std::format nor fmt available - using basic formatting for monitoring")
                add_definitions(-DMONITORING_USE_BASIC_FORMAT)
                set(MONITORING_FORMAT_BACKEND "basic" CACHE INTERNAL "Monitoring format backend")
                set(MONITORING_FORMAT_BACKEND "basic" PARENT_SCOPE)
            endif()
        endif()
    endif()
endfunction()

# Enhanced formatting library setup for monitoring
function(setup_monitoring_formatting TARGET_NAME)
    message(STATUS "========================================")
    message(STATUS "Setting up formatting library for ${TARGET_NAME}")
    
    if(DEFINED MONITORING_FORMAT_BACKEND)
        message(STATUS "  Backend: ${MONITORING_FORMAT_BACKEND}")
        
        if(MONITORING_FORMAT_BACKEND STREQUAL "std::format")
            message(STATUS "  ✅ Using std::format (C++20 native)")
            target_compile_definitions(${TARGET_NAME} PRIVATE MONITORING_USE_STD_FORMAT)
            
        elseif(MONITORING_FORMAT_BACKEND STREQUAL "fmt::format")
            message(STATUS "  ✅ Using fmt::format library")
            target_compile_definitions(${TARGET_NAME} PRIVATE MONITORING_USE_FMT)
            if(DEFINED MONITORING_FMT_TARGET)
                target_link_libraries(${TARGET_NAME} PRIVATE ${MONITORING_FMT_TARGET})
                message(STATUS "  📦 Linked fmt target: ${MONITORING_FMT_TARGET}")
            endif()
            
        elseif(MONITORING_FORMAT_BACKEND STREQUAL "basic")
            message(STATUS "  ⚠️  Using basic formatting fallback")
            target_compile_definitions(${TARGET_NAME} PRIVATE MONITORING_USE_BASIC_FORMAT)
        endif()
    else()
        message(WARNING "MONITORING_FORMAT_BACKEND not set - using basic formatting")
        target_compile_definitions(${TARGET_NAME} PRIVATE MONITORING_USE_BASIC_FORMAT)
    endif()
    
    message(STATUS "========================================")
endfunction()

##################################################
# Performance Feature Management
##################################################

function(setup_performance_features)
    if(CMAKE_CXX_STANDARD EQUAL 17)
        # Use C++17 compatible performance features
        message(STATUS "Configuring C++17 compatible performance monitoring")
        add_definitions(-DMONITORING_USE_LEGACY_PERFORMANCE)
    else()
        # Use C++20 enhanced performance features
        message(STATUS "Configuring C++20 enhanced performance monitoring")
        add_definitions(-DMONITORING_USE_ENHANCED_PERFORMANCE)
    endif()
endfunction()

##################################################
# Thread Safety Feature Management
##################################################

function(setup_threading_features)
    if(CMAKE_CXX_STANDARD EQUAL 17)
        # Use traditional threading with manual management
        message(STATUS "Using traditional thread management (C++17)")
        add_definitions(-DMONITORING_USE_TRADITIONAL_THREADING)
    else()
        # Use modern threading with jthread and stop tokens
        message(STATUS "Using modern thread management with jthread (C++20)")
        add_definitions(-DMONITORING_USE_MODERN_THREADING)
    endif()
endfunction()