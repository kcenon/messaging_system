# LoggerWarnings.cmake
# Compiler warning configuration for Logger System

# Warning options
option(LOGGER_ENABLE_WARNINGS "Enable comprehensive compiler warnings" ON)
option(LOGGER_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Function to add warning flags to a target
function(logger_add_warnings target)
    if(NOT LOGGER_ENABLE_WARNINGS)
        return()
    endif()
    
    # Base warning flags for all compilers
    set(WARNING_FLAGS "")
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        # Common warnings for GCC and Clang
        list(APPEND WARNING_FLAGS
            -Wall                       # Basic warnings
            -Wextra                     # Extra warnings
            -Wpedantic                  # Strict ISO C++ compliance
            -Wcast-align                # Warn about pointer cast alignment issues
            -Wcast-qual                 # Warn about cast removing qualifiers
            -Wconversion                # Warn about type conversions
            -Wctor-dtor-privacy         # Warn about useless constructors
            -Wdisabled-optimization     # Warn when optimization is disabled
            -Wformat=2                  # Format string checking
            -Winit-self                 # Warn about self-initialization
            -Wlogical-op                # Warn about suspicious logical operations
            -Wmissing-declarations      # Warn about missing declarations
            -Wmissing-include-dirs      # Warn about missing include directories
            -Wnoexcept                  # Warn about noexcept issues
            -Wold-style-cast            # Warn about C-style casts
            -Woverloaded-virtual        # Warn about hidden virtual functions
            -Wredundant-decls           # Warn about redundant declarations
            -Wshadow                    # Warn about variable shadowing
            -Wsign-conversion           # Warn about sign conversions
            -Wsign-promo                # Warn about sign promotion
            -Wstrict-null-sentinel      # Warn about null sentinel issues
            -Wstrict-overflow=5         # Strict overflow checking
            -Wswitch-default            # Warn about missing default in switch
            -Wundef                     # Warn about undefined macros
            -Wunused                    # Warn about unused entities
        )
        
        # Compiler-specific warnings
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            # GCC-specific warnings
            list(APPEND WARNING_FLAGS
                -Wduplicated-cond       # Warn about duplicated conditions
                -Wduplicated-branches   # Warn about duplicated branches
                -Wnull-dereference      # Warn about null dereferences
                -Wuseless-cast          # Warn about useless casts
                -Wlogical-op            # Warn about suspicious logical operations
                -Wsuggest-override      # Suggest override keyword
                -Wsuggest-final-types   # Suggest final for types
                -Wsuggest-final-methods # Suggest final for methods
            )
            
            # GCC version-specific warnings
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
                list(APPEND WARNING_FLAGS
                    -Wextra-semi        # Warn about extra semicolons
                    -Wimplicit-fallthrough=5  # Warn about switch fallthrough
                )
            endif()
            
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
            # Clang-specific warnings
            list(APPEND WARNING_FLAGS
                -Weverything            # Enable all warnings (then disable specific ones)
                # Disable overly pedantic warnings
                -Wno-c++98-compat       # We're using modern C++
                -Wno-c++98-compat-pedantic
                -Wno-padded             # Padding is often necessary
                -Wno-packed             # Packing is sometimes needed
                -Wno-global-constructors # Global constructors can be useful
                -Wno-exit-time-destructors # Exit-time destructors are often fine
                -Wno-weak-vtables       # Weak vtables are acceptable
                -Wno-float-equal        # Float comparison is sometimes needed
                -Wno-double-promotion   # Double promotion is often intentional
                -Wno-disabled-macro-expansion # Macro expansion in macros is sometimes needed
            )
        endif()
        
    elseif(MSVC)
        # MSVC warning flags
        list(APPEND WARNING_FLAGS
            /W4                         # Warning level 4 (highest reasonable)
            /permissive-                # Strict conformance mode
            /w14242                     # Conversion warnings
            /w14254                     # Operator conversion warnings
            /w14263                     # Member function hiding
            /w14265                     # Class has virtual functions but no virtual destructor
            /w14287                     # Unsigned/negative constant mismatch
            /w14289                     # Loop control variable issues
            /w14296                     # Expression is always false
            /w14311                     # Pointer truncation
            /w14545                     # Expression before comma
            /w14546                     # Function call before comma
            /w14547                     # Operator before comma has no effect
            /w14549                     # Operator before comma has no effect
            /w14555                     # Expression has no effect
            /w14619                     # Invalid warning number
            /w14640                     # Thread unsafe static member
            /w14826                     # Conversion from pointer to int
            /w14905                     # String literal cast
            /w14906                     # String literal cast to non-const
            /w14928                     # Illegal copy initialization
        )
        
        # MSVC-specific defines for better warnings
        target_compile_definitions(${target} PRIVATE
            _CRT_SECURE_NO_WARNINGS  # Disable CRT secure warnings
            NOMINMAX                  # Prevent min/max macros
        )
    endif()
    
    # Apply warning flags to target
    target_compile_options(${target} PRIVATE ${WARNING_FLAGS})
    
    # Treat warnings as errors if requested
    if(LOGGER_WARNINGS_AS_ERRORS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            target_compile_options(${target} PRIVATE -Werror)
        elseif(MSVC)
            target_compile_options(${target} PRIVATE /WX)
        endif()
        message(STATUS "Treating warnings as errors for ${target}")
    endif()
    
    message(STATUS "Enabled comprehensive warnings for ${target}")
endfunction()

# Function to suppress specific warnings for third-party code
function(logger_suppress_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE -w)  # Suppress all warnings
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W0)  # Warning level 0
    endif()
endfunction()

# Macro to add warnings to all targets
macro(logger_enable_warnings_for_all)
    if(LOGGER_ENABLE_WARNINGS)
        # Add warnings to main library
        if(TARGET logger_system)
            logger_add_warnings(logger_system)
        endif()
        
        # Add warnings to all test targets if tests are built
        if(BUILD_TESTS)
            # Add warnings to each test executable
            if(TARGET stress_test)
                logger_add_warnings(stress_test)
            endif()
            if(TARGET integration_test)
                logger_add_warnings(integration_test)
            endif()
            # Add more test targets as needed
        endif()
    endif()
endmacro()

# Print warning configuration
if(LOGGER_ENABLE_WARNINGS)
    message(STATUS "========================================")
    message(STATUS "Warning Configuration:")
    message(STATUS "  Warnings Enabled: YES")
    message(STATUS "  Warnings as Errors: ${LOGGER_WARNINGS_AS_ERRORS}")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "========================================")
endif()