################################################################################
# validate_dependencies.cmake
#
# Validates the dependency chain for messaging_system (Tier 5).
#
# Dependency hierarchy:
#   Tier 5: messaging_system
#     ├── [REQUIRED] network_system (Tier 4)
#     │     └── logger_system (Tier 2)
#     │           └── thread_system (Tier 1)
#     │                 └── common_system (Tier 0)
#     ├── [REQUIRED] container_system (Tier 1)
#     │     └── common_system (Tier 0)
#     ├── [OPTIONAL] monitoring_system (Tier 3)
#     └── [OPTIONAL] database_system (Tier 5)
################################################################################

function(validate_messaging_dependencies)
    # =========================================================================
    # REQUIRED Dependencies
    # These are mandatory for messaging_system to function correctly.
    # =========================================================================
    set(REQUIRED_TARGETS
        CommonSystem::common      # Tier 0 - Foundation
        ThreadSystem::Core        # Tier 1 - Required via network_system chain
        ContainerSystem::container # Tier 1 - Required for serialization
        LoggerSystem::logger      # Tier 2 - Required via network_system chain
        NetworkSystem::network    # Tier 4 - Required for network transport
    )

    # =========================================================================
    # OPTIONAL Dependencies
    # These provide additional features but are not required for core functionality.
    # =========================================================================
    set(OPTIONAL_TARGETS
        MonitoringSystem::monitoring  # Tier 3 - Optional metrics/monitoring
        DatabaseSystem::database      # Tier 5 - Optional persistence
    )

    # =========================================================================
    # Validate Required Dependencies
    # =========================================================================
    set(MISSING_REQUIRED "")

    foreach(target ${REQUIRED_TARGETS})
        if(NOT TARGET ${target})
            list(APPEND MISSING_REQUIRED ${target})
        endif()
    endforeach()

    if(MISSING_REQUIRED)
        message(FATAL_ERROR
            "Required dependencies not found: ${MISSING_REQUIRED}\n"
            "\n"
            "messaging_system (Tier 5) requires the following dependency chain:\n"
            "  - CommonSystem::common (Tier 0)\n"
            "  - ThreadSystem::Core (Tier 1)\n"
            "  - ContainerSystem::container (Tier 1)\n"
            "  - LoggerSystem::logger (Tier 2)\n"
            "  - NetworkSystem::network (Tier 4)\n"
            "\n"
            "Please install missing packages or enable FetchContent:\n"
            "  cmake -DMESSAGING_USE_FETCHCONTENT=ON ..\n"
            "\n"
            "Or install packages:\n"
            "  Ubuntu/Debian: apt-get install lib<system>-dev\n"
            "  macOS: brew install <system>\n"
        )
    endif()

    message(STATUS "✓ All required messaging_system dependencies validated")

    # =========================================================================
    # Report Optional Dependencies
    # =========================================================================
    foreach(target ${OPTIONAL_TARGETS})
        if(TARGET ${target})
            message(STATUS "  ✓ Optional: ${target} (available)")
        else()
            message(STATUS "  ○ Optional: ${target} (not loaded)")
        endif()
    endforeach()
endfunction()
