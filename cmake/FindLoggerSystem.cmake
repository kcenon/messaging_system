# FindLoggerSystem.cmake
# Find the LoggerSystem library
#
# This will define:
#   LoggerSystem_FOUND - System has LoggerSystem
#   LoggerSystem_INCLUDE_DIRS - The LoggerSystem include directories
#   LoggerSystem_LIBRARIES - The libraries needed to use LoggerSystem
#   LoggerSystem::logger - Imported target

find_path(LoggerSystem_INCLUDE_DIR
    NAMES logger_system/logger/logger.h
    PATHS
        ${LoggerSystem_ROOT}/include
        ${CMAKE_PREFIX_PATH}/include
        /usr/local/include
        /usr/include
)

find_library(LoggerSystem_LIBRARY
    NAMES logger
    PATHS
        ${LoggerSystem_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LoggerSystem DEFAULT_MSG
    LoggerSystem_LIBRARY LoggerSystem_INCLUDE_DIR
)

if(LoggerSystem_FOUND)
    set(LoggerSystem_LIBRARIES ${LoggerSystem_LIBRARY})
    set(LoggerSystem_INCLUDE_DIRS ${LoggerSystem_INCLUDE_DIR})
    
    # Create imported target
    if(NOT TARGET LoggerSystem::logger)
        add_library(LoggerSystem::logger STATIC IMPORTED)
        set_target_properties(LoggerSystem::logger PROPERTIES
            IMPORTED_LOCATION "${LoggerSystem_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LoggerSystem_INCLUDE_DIR}"
            INTERFACE_COMPILE_FEATURES cxx_std_20
        )
        
        # Add thread dependency
        find_package(Threads REQUIRED)
        set_property(TARGET LoggerSystem::logger APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
    endif()
endif()

mark_as_advanced(LoggerSystem_INCLUDE_DIR LoggerSystem_LIBRARY)