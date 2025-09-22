# LoggerCompatibility.cmake
# C++17/C++20 feature detection and compatibility layer

##################################################
# Feature Detection Functions
##################################################

# Test for std::format availability with vcpkg fmt fallback
function(check_std_format)
    include(CheckCXXSourceCompiles)
    
    # First try std::format
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <format>
        int main() {
            auto s = std::format(\"Hello {}\", \"World\");
            return 0;
        }
    " HAS_STD_FORMAT)
    
    if(HAS_STD_FORMAT)
        message(STATUS "✅ std::format is available")
        add_definitions(-DLOGGER_HAS_STD_FORMAT)
        add_definitions(-DLOGGER_USE_STD_FORMAT)
        set(LOGGER_FORMAT_BACKEND "std::format" PARENT_SCOPE)
    else()
        message(STATUS "⚠️ std::format not available - checking for fmt library")
        
        # Try to find fmt through vcpkg or system
        find_package(fmt CONFIG QUIET)
        if(fmt_FOUND)
            message(STATUS "✅ fmt library found - using fmt::format")
            add_definitions(-DLOGGER_USE_FMT)
            set(LOGGER_FORMAT_BACKEND "fmt::format" PARENT_SCOPE)
            set(LOGGER_FMT_TARGET fmt::fmt PARENT_SCOPE)
        else()
            # Try to find fmt through vcpkg explicitly
            if(DEFINED ENV{VCPKG_ROOT})
                message(STATUS "🔍 Trying to find fmt through vcpkg...")
                find_package(fmt CONFIG QUIET PATHS $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET})
                if(fmt_FOUND)
                    message(STATUS "✅ fmt library found via vcpkg - using fmt::format")
                    add_definitions(-DLOGGER_USE_FMT)
                    set(LOGGER_FORMAT_BACKEND "fmt::format" PARENT_SCOPE)
                    set(LOGGER_FMT_TARGET fmt::fmt PARENT_SCOPE)
                else()
                    message(STATUS "⚠️ fmt library not found via vcpkg")
                endif()
            endif()
            
            if(NOT fmt_FOUND)
                message(STATUS "⚠️ Neither std::format nor fmt available - using basic formatting")
                add_definitions(-DLOGGER_USE_BASIC_FORMAT)
                set(LOGGER_FORMAT_BACKEND "basic" PARENT_SCOPE)
            endif()
        endif()
    endif()
endfunction()

# Test for concepts availability
function(check_concepts)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        #include <concepts>
        template<typename T>
        concept Addable = requires(T a, T b) {
            a + b;
        };
        template<Addable T>
        T add(T a, T b) { return a + b; }
        int main() {
            return add(1, 2);
        }
    " HAS_CONCEPTS)
    
    if(HAS_CONCEPTS)
        message(STATUS "C++20 concepts are available")
        add_definitions(-DLOGGER_HAS_CONCEPTS)
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
        int main() {
            int arr[] = {1, 2, 3};
            std::span<int> s(arr);
            return s.size();
        }
    " HAS_STD_SPAN)
    
    if(HAS_STD_SPAN)
        message(STATUS "std::span is available")
        add_definitions(-DLOGGER_HAS_STD_SPAN)
    else()
        message(STATUS "std::span not available - using pointer/size pairs")
    endif()
endfunction()

# Test for designated initializers
function(check_designated_initializers)
    include(CheckCXXSourceCompiles)
    
    set(CMAKE_REQUIRED_FLAGS "-std=c++20")
    check_cxx_source_compiles("
        struct Point { int x, y; };
        int main() {
            Point p{.x = 1, .y = 2};
            return p.x + p.y;
        }
    " HAS_DESIGNATED_INITIALIZERS)
    
    if(HAS_DESIGNATED_INITIALIZERS)
        message(STATUS "Designated initializers are available")
        add_definitions(-DLOGGER_HAS_DESIGNATED_INITIALIZERS)
    else()
        message(STATUS "Designated initializers not available - using traditional initialization")
    endif()
endfunction()

##################################################
# Main Compatibility Check Function
##################################################

function(configure_logger_compatibility)
    message(STATUS "========================================")
    message(STATUS "Logger System - Feature Detection:")
    message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
    
    if(CMAKE_CXX_STANDARD EQUAL 17)
        message(STATUS "  Mode: C++17 Compatibility")
        add_definitions(-DLOGGER_CPP17_MODE)
        
        # In C++17 mode, disable all C++20 features
        message(STATUS "  - std::format: DISABLED (using fmt library)")
        message(STATUS "  - concepts: DISABLED (using SFINAE)")
        message(STATUS "  - std::span: DISABLED (using pointer/size)")
        message(STATUS "  - designated initializers: DISABLED")
        
    else()
        message(STATUS "  Mode: C++20 Enhanced")
        add_definitions(-DLOGGER_CPP20_MODE)
        
        # Test each C++20 feature
        check_std_format()
        check_concepts()
        check_std_span()
        check_designated_initializers()
    endif()
    
    message(STATUS "========================================")
endfunction()

##################################################
# External Dependency Management
##################################################

# Enhanced formatting library setup with vcpkg support
function(setup_formatting_library TARGET_NAME)
    message(STATUS "========================================")
    message(STATUS "Setting up formatting library for ${TARGET_NAME}")
    
    if(DEFINED LOGGER_FORMAT_BACKEND)
        message(STATUS "  Backend: ${LOGGER_FORMAT_BACKEND}")
        
        if(LOGGER_FORMAT_BACKEND STREQUAL "std::format")
            message(STATUS "  ✅ Using std::format (C++20 native)")
            target_compile_definitions(${TARGET_NAME} PRIVATE LOGGER_USE_STD_FORMAT)
            
        elseif(LOGGER_FORMAT_BACKEND STREQUAL "fmt::format")
            message(STATUS "  ✅ Using fmt::format library")
            target_compile_definitions(${TARGET_NAME} PRIVATE LOGGER_USE_FMT)
            if(DEFINED LOGGER_FMT_TARGET)
                target_link_libraries(${TARGET_NAME} PRIVATE ${LOGGER_FMT_TARGET})
                message(STATUS "  📦 Linked fmt target: ${LOGGER_FMT_TARGET}")
            endif()
            
        elseif(LOGGER_FORMAT_BACKEND STREQUAL "basic")
            message(STATUS "  ⚠️  Using basic formatting fallback")
            target_compile_definitions(${TARGET_NAME} PRIVATE LOGGER_USE_BASIC_FORMAT)
        endif()
    else()
        message(WARNING "LOGGER_FORMAT_BACKEND not set - using basic formatting")
        target_compile_definitions(${TARGET_NAME} PRIVATE LOGGER_USE_BASIC_FORMAT)
    endif()
    
    message(STATUS "========================================")
endfunction()

# Convenience function for setting up vcpkg fmt if std::format not available
function(setup_vcpkg_fmt_fallback)
    if(NOT DEFINED LOGGER_FORMAT_BACKEND OR LOGGER_FORMAT_BACKEND STREQUAL "basic")
        message(STATUS "🔍 Attempting to install fmt via vcpkg...")
        
        # Check if vcpkg is available
        find_program(VCPKG_EXECUTABLE vcpkg 
            PATHS 
                $ENV{VCPKG_ROOT} 
                $ENV{VCPKG_ROOT}/vcpkg
                ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg
                ${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg
        )
        
        if(VCPKG_EXECUTABLE)
            message(STATUS "📦 Found vcpkg at: ${VCPKG_EXECUTABLE}")
            message(STATUS "💡 Consider running: ${VCPKG_EXECUTABLE} install fmt")
        else()
            message(STATUS "⚠️  vcpkg not found. To install fmt:")
            message(STATUS "    1. Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git")
            message(STATUS "    2. Set VCPKG_ROOT environment variable")  
            message(STATUS "    3. Run: vcpkg install fmt")
        endif()
    endif()
endfunction()