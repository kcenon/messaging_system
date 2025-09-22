# LoggerSanitizers.cmake
# Sanitizer configuration for Logger System

# Sanitizer options
option(LOGGER_ENABLE_SANITIZERS "Enable sanitizers in debug builds" OFF)
set(LOGGER_SANITIZER_TYPE "address" CACHE STRING "Type of sanitizer to use (address, thread, undefined, memory)")
set_property(CACHE LOGGER_SANITIZER_TYPE PROPERTY STRINGS address thread undefined memory)

# Function to add sanitizer flags to a target
function(logger_add_sanitizers target)
    if(NOT LOGGER_ENABLE_SANITIZERS)
        return()
    endif()
    
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING "Sanitizers are only supported in Debug builds")
        return()
    endif()
    
    # Check compiler support
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(WARNING "Sanitizers are only supported with GCC and Clang")
        return()
    endif()
    
    # Sanitizer flags based on type
    if(LOGGER_SANITIZER_TYPE STREQUAL "address")
        set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
        set(SANITIZER_LINK_FLAGS "-fsanitize=address")
        message(STATUS "Enabling AddressSanitizer for ${target}")
        
    elseif(LOGGER_SANITIZER_TYPE STREQUAL "thread")
        set(SANITIZER_FLAGS "-fsanitize=thread")
        set(SANITIZER_LINK_FLAGS "-fsanitize=thread")
        message(STATUS "Enabling ThreadSanitizer for ${target}")
        
    elseif(LOGGER_SANITIZER_TYPE STREQUAL "undefined")
        set(SANITIZER_FLAGS "-fsanitize=undefined -fno-sanitize-recover=all")
        set(SANITIZER_LINK_FLAGS "-fsanitize=undefined")
        message(STATUS "Enabling UndefinedBehaviorSanitizer for ${target}")
        
    elseif(LOGGER_SANITIZER_TYPE STREQUAL "memory")
        if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message(WARNING "MemorySanitizer is only supported with Clang")
            return()
        endif()
        set(SANITIZER_FLAGS "-fsanitize=memory -fno-omit-frame-pointer")
        set(SANITIZER_LINK_FLAGS "-fsanitize=memory")
        message(STATUS "Enabling MemorySanitizer for ${target}")
        
    else()
        message(WARNING "Unknown sanitizer type: ${LOGGER_SANITIZER_TYPE}")
        return()
    endif()
    
    # Apply sanitizer flags
    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
    target_link_options(${target} PRIVATE ${SANITIZER_LINK_FLAGS})
    
    # Additional debug flags for better sanitizer output
    target_compile_options(${target} PRIVATE -g -O1)
    
    # Platform-specific settings
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Linux-specific sanitizer settings
        if(LOGGER_SANITIZER_TYPE STREQUAL "address")
            target_compile_definitions(${target} PRIVATE _GLIBCXX_SANITIZE_VECTOR)
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # macOS-specific sanitizer settings
        if(LOGGER_SANITIZER_TYPE STREQUAL "address")
            # macOS requires different link flags for ASan
            target_link_options(${target} PRIVATE -Wl,-no_pie)
        endif()
    endif()
endfunction()

# Function to configure sanitizer runtime options
function(logger_configure_sanitizer_runtime)
    if(NOT LOGGER_ENABLE_SANITIZERS)
        return()
    endif()
    
    # Set runtime options for CTest
    if(LOGGER_SANITIZER_TYPE STREQUAL "address")
        set(ASAN_OPTIONS 
            "detect_leaks=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"
            CACHE STRING "AddressSanitizer runtime options")
        set(ENV{ASAN_OPTIONS} ${ASAN_OPTIONS})
        
    elseif(LOGGER_SANITIZER_TYPE STREQUAL "thread")
        set(TSAN_OPTIONS 
            "halt_on_error=1:history_size=7:second_deadlock_stack=1"
            CACHE STRING "ThreadSanitizer runtime options")
        set(ENV{TSAN_OPTIONS} ${TSAN_OPTIONS})
        
    elseif(LOGGER_SANITIZER_TYPE STREQUAL "undefined")
        set(UBSAN_OPTIONS 
            "print_stacktrace=1:halt_on_error=1:print_module_map=1"
            CACHE STRING "UndefinedBehaviorSanitizer runtime options")
        set(ENV{UBSAN_OPTIONS} ${UBSAN_OPTIONS})
        
    elseif(LOGGER_SANITIZER_TYPE STREQUAL "memory")
        set(MSAN_OPTIONS 
            "halt_on_error=1:print_stats=1"
            CACHE STRING "MemorySanitizer runtime options")
        set(ENV{MSAN_OPTIONS} ${MSAN_OPTIONS})
    endif()
endfunction()

# Macro to add sanitizers to all test targets
macro(logger_enable_sanitizers_for_tests)
    if(LOGGER_ENABLE_SANITIZERS AND BUILD_TESTS)
        # Find all test executables and add sanitizers
        get_property(test_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY TESTS)
        foreach(test_target ${test_targets})
            logger_add_sanitizers(${test_target})
        endforeach()
        
        # Configure runtime options
        logger_configure_sanitizer_runtime()
    endif()
endmacro()

# Print sanitizer configuration
if(LOGGER_ENABLE_SANITIZERS)
    message(STATUS "========================================")
    message(STATUS "Sanitizer Configuration:")
    message(STATUS "  Type: ${LOGGER_SANITIZER_TYPE}")
    message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "========================================")
endif()