function(validate_messaging_dependencies)
    set(REQUIRED_TARGETS
        CommonSystem::common
        ThreadSystem::Core
        LoggerSystem::logger
        MonitoringSystem::monitoring
        ContainerSystem::container
        DatabaseSystem::database
        NetworkSystem::network
    )

    set(MISSING_TARGETS "")

    foreach(target ${REQUIRED_TARGETS})
        if(NOT TARGET ${target})
            list(APPEND MISSING_TARGETS ${target})
        endif()
    endforeach()

    if(MISSING_TARGETS)
        message(FATAL_ERROR
            "Required dependencies not found: ${MISSING_TARGETS}\n"
            "Please install missing packages or enable FetchContent:\n"
            "  cmake -DMESSAGING_USE_FETCHCONTENT=ON ..\n"
            "\n"
            "Or install packages:\n"
            "  Ubuntu/Debian: apt-get install lib<system>-dev\n"
            "  macOS: brew install <system>\n"
        )
    endif()

    message(STATUS "✓ All messaging_system dependencies validated")
endfunction()
