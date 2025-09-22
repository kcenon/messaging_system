# CompilerChecks.cmake
# Comprehensive compiler version and feature checks for C++17/20 support

##################################################
# Minimum Compiler Version Requirements
##################################################

# Define minimum compiler versions for C++20 support
set(MIN_GCC_VERSION "10.0")
set(MIN_CLANG_VERSION "11.0")
set(MIN_APPLECLANG_VERSION "12.0")
set(MIN_MSVC_VERSION "19.26")  # Visual Studio 2019 16.6

# Check compiler version
function(check_compiler_version)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MIN_GCC_VERSION})
            message(FATAL_ERROR "GCC version ${CMAKE_CXX_COMPILER_VERSION} is not supported. Minimum required version is ${MIN_GCC_VERSION}")
        endif()
        message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION} - C++20 support verified")
        
        # Special handling for MinGW
        if(MINGW)
            message(STATUS "MinGW detected - applying compatibility settings")
            add_definitions(-D__MINGW32__)
            # MinGW often needs explicit std library linking
            set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lstdc++fs")
        endif()
        
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
# Compiler Warning Flags
##################################################

function(set_compiler_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Base warning flags for all GNU/Clang compilers
        set(WARNING_FLAGS
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )
        
        # MinGW-specific warning adjustments
        if(MINGW)
            # Disable warnings that are problematic on MinGW
            list(APPEND WARNING_FLAGS
                -Wno-unknown-pragmas  # Ignore unknown pragma warnings
                -Wno-format           # MinGW has issues with format string warnings
            )
        endif()
        
        target_compile_options(${target} PRIVATE ${WARNING_FLAGS})
        
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target} PRIVATE
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wuseless-cast
            )
        endif()
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /w14242  # 'identifier': conversion from 'type1' to 'type2', possible loss of data
            /w14254  # 'operator': conversion from 'type1' to 'type2', possible loss of data
            /w14263  # 'function': member function does not override any base class virtual member function
            /w14265  # 'classname': class has virtual functions, but destructor is not virtual
            /w14287  # 'operator': unsigned/negative constant mismatch
            /we4289  # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
            /w14296  # 'operator': expression is always 'boolean_value'
            /w14311  # 'variable': pointer truncation from 'type1' to 'type2'
            /w14545  # expression before comma evaluates to a function which is missing an argument list
            /w14546  # function call before comma missing argument list
            /w14547  # 'operator': operator before comma has no effect; expected operator with side-effect
            /w14549  # 'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555  # expression has no effect; expected expression with side-effect
            /w14619  # pragma warning: there is no warning number 'number'
            /w14640  # Enable warning on thread un-safe static member initialization
            /w14826  # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
            /w14905  # wide string literal cast to 'LPSTR'
            /w14906  # string literal cast to 'LPWSTR'
            /w14928  # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        )
    endif()
endfunction()

##################################################
# Required Header Checks
##################################################

include(CheckIncludeFileCXX)

function(check_required_headers)
    # C++ Standard headers
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
    )
    
    # C++17 headers
    set(CXX17_HEADERS
        string_view
        variant
        any
    )
    
    # C++20 headers  
    set(CXX20_HEADERS
        concepts
        format
        ranges
        span
        stop_token
    )
    
    # Check standard headers
    foreach(header ${REQUIRED_HEADERS})
        check_include_file_cxx(${header} HAVE_${header})
        if(NOT HAVE_${header})
            message(FATAL_ERROR "Required header <${header}> not found!")
        endif()
    endforeach()
    
    # Check C++17 headers
    foreach(header ${CXX17_HEADERS})
        check_include_file_cxx(${header} HAVE_${header})
        if(NOT HAVE_${header})
            message(WARNING "C++17 header <${header}> not found!")
        endif()
    endforeach()
    
    # Check C++20 headers (optional)
    foreach(header ${CXX20_HEADERS})
        check_include_file_cxx(${header} HAVE_${header})
        if(HAVE_${header})
            message(STATUS "C++20 header <${header}> found")
        else()
            message(STATUS "C++20 header <${header}> not found - will use fallback")
        endif()
    endforeach()
endfunction()

##################################################
# Platform-Specific Settings
##################################################

function(configure_platform_settings)
    # Windows-specific settings
    if(WIN32)
        add_definitions(-DNOMINMAX)  # Prevent Windows.h from defining min/max macros
        add_definitions(-DWIN32_LEAN_AND_MEAN)  # Exclude rarely-used Windows headers
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # Disable MSVC secure warnings
        
        # Enable parallel compilation on MSVC
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            add_compile_options(/MP)
        endif()
    endif()
    
    # Linux-specific settings
    if(UNIX AND NOT APPLE)
        add_definitions(-D_GNU_SOURCE)  # Enable GNU extensions
    endif()
    
    # macOS-specific settings
    if(APPLE)
        # Already handled in main CMakeLists.txt
    endif()
endfunction()

##################################################
# Precompiled Header Support
##################################################

function(target_precompile_headers_if_supported target)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        # MinGW has issues with PCH, so disable it
        if(MINGW)
            message(STATUS "Precompiled headers disabled for MinGW")
            return()
        endif()
        
        # Check if PCH is supported
        if(NOT DEFINED CMAKE_CXX_COMPILER_PRECOMPILE_HEADERS)
            # Test PCH support
            include(CheckCXXCompilerFlag)
            check_cxx_compiler_flag("-x c++-header" COMPILER_SUPPORTS_PCH)
            set(CMAKE_CXX_COMPILER_PRECOMPILE_HEADERS ${COMPILER_SUPPORTS_PCH} CACHE INTERNAL "")
        endif()
        
        if(CMAKE_CXX_COMPILER_PRECOMPILE_HEADERS)
            # Only include standard library headers in PCH
            # Project headers can cause issues with different build configurations
            target_precompile_headers(${target} PRIVATE
                # Standard library headers used frequently
                <algorithm>
                <atomic>
                <chrono>
                <functional>
                <memory>
                <mutex>
                <string>
                <vector>
                <string_view>
                <type_traits>
            )
            message(STATUS "Precompiled headers enabled for ${target}")
        endif()
    endif()
endfunction()

##################################################
# Debug/Release Build Configuration
##################################################

function(configure_build_types)
    # Set default build type if not specified
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif()
    
    # Debug build settings
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG_BUILD)
        
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_compile_options(-g3 -O0 -fno-omit-frame-pointer)
            
            # Enable sanitizers in debug mode
            option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
            if(ENABLE_SANITIZERS)
                add_compile_options(-fsanitize=address,undefined)
                add_link_options(-fsanitize=address,undefined)
            endif()
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            add_compile_options(/Od /Zi /RTC1)
        endif()
    endif()
    
    # Release build settings
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_compile_options(-O3 -DNDEBUG)
            
            # Enable link-time optimization
            include(CheckIPOSupported)
            check_ipo_supported(RESULT IPO_SUPPORTED)
            if(IPO_SUPPORTED)
                set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
            endif()
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            add_compile_options(/O2 /DNDEBUG)
        endif()
    endif()
endfunction()

##################################################
# C++ Standard Library Feature Checks
##################################################

function(check_cpp_stdlib_features)
    include(CheckCXXSourceCompiles)
    
    # Check for std::shared_mutex (C++17)
    check_cxx_source_compiles("
        #include <shared_mutex>
        int main() {
            std::shared_mutex sm;
            return 0;
        }
    " HAS_STD_SHARED_MUTEX)
    
    if(HAS_STD_SHARED_MUTEX)
        add_definitions(-DHAS_STD_SHARED_MUTEX)
        message(STATUS "C++17 feature std::shared_mutex is available")
    endif()
    
    # Check for std::optional (C++17)
    check_cxx_source_compiles("
        #include <optional>
        int main() {
            std::optional<int> opt = 42;
            return opt.value();
        }
    " HAS_STD_OPTIONAL)
    
    if(HAS_STD_OPTIONAL)
        add_definitions(-DHAS_STD_OPTIONAL)
        message(STATUS "C++17 feature std::optional is available")
    endif()
    
    # Check for std::variant (C++17)
    check_cxx_source_compiles("
        #include <variant>
        int main() {
            std::variant<int, double> v = 42;
            return std::get<int>(v);
        }
    " HAS_STD_VARIANT)
    
    if(HAS_STD_VARIANT)
        add_definitions(-DHAS_STD_VARIANT)
        message(STATUS "C++17 feature std::variant is available")
    endif()
    
    # Check for std::string_view (C++17)
    check_cxx_source_compiles("
        #include <string_view>
        int main() {
            std::string_view sv = \"hello\";
            return sv.length();
        }
    " HAS_STD_STRING_VIEW)
    
    if(HAS_STD_STRING_VIEW)
        add_definitions(-DHAS_STD_STRING_VIEW)
        message(STATUS "C++17 feature std::string_view is available")
    endif()
    
    # Check for structured bindings (C++17)
    check_cxx_source_compiles("
        #include <tuple>
        int main() {
            std::tuple<int, double> t{1, 2.0};
            auto [a, b] = t;
            return a;
        }
    " HAS_STRUCTURED_BINDINGS)
    
    if(HAS_STRUCTURED_BINDINGS)
        add_definitions(-DHAS_STRUCTURED_BINDINGS)
        message(STATUS "C++17 feature structured bindings is available")
    endif()
    
    # Check for if constexpr (C++17)
    check_cxx_source_compiles("
        template<typename T>
        int test() {
            if constexpr (sizeof(T) > 4) {
                return 1;
            } else {
                return 0;
            }
        }
        int main() {
            return test<int>();
        }
    " HAS_IF_CONSTEXPR)
    
    if(HAS_IF_CONSTEXPR)
        add_definitions(-DHAS_IF_CONSTEXPR)
        message(STATUS "C++17 feature if constexpr is available")
    endif()
    
    # Check for std::execution (C++17)
    check_cxx_source_compiles("
        #include <execution>
        #include <algorithm>
        #include <vector>
        int main() {
            std::vector<int> v = {1, 2, 3};
            std::sort(std::execution::par, v.begin(), v.end());
            return 0;
        }
    " HAS_STD_EXECUTION)
    
    if(HAS_STD_EXECUTION)
        add_definitions(-DHAS_STD_EXECUTION)
        message(STATUS "C++17 feature std::execution is available")
    endif()
    
    # Check for std::pmr (C++17)
    check_cxx_source_compiles("
        #include <memory_resource>
        #include <vector>
        int main() {
            std::pmr::monotonic_buffer_resource pool;
            std::pmr::vector<int> v(&pool);
            v.push_back(42);
            return v[0];
        }
    " HAS_STD_PMR)
    
    if(HAS_STD_PMR)
        add_definitions(-DHAS_STD_PMR)
        message(STATUS "C++17 feature std::pmr is available")
    endif()
endfunction()