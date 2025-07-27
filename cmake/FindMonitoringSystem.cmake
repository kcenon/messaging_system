# FindMonitoringSystem.cmake
# Find the MonitoringSystem library
#
# This will define:
#   MonitoringSystem_FOUND - System has MonitoringSystem
#   MonitoringSystem_INCLUDE_DIRS - The MonitoringSystem include directories
#   MonitoringSystem_LIBRARIES - The libraries needed to use MonitoringSystem
#   MonitoringSystem::monitoring - Imported target

find_path(MonitoringSystem_INCLUDE_DIR
    NAMES monitoring_system/monitoring/monitoring.h
    PATHS
        ${MonitoringSystem_ROOT}/include
        ${CMAKE_PREFIX_PATH}/include
        /usr/local/include
        /usr/include
)

find_library(MonitoringSystem_LIBRARY
    NAMES monitoring
    PATHS
        ${MonitoringSystem_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MonitoringSystem DEFAULT_MSG
    MonitoringSystem_LIBRARY MonitoringSystem_INCLUDE_DIR
)

if(MonitoringSystem_FOUND)
    set(MonitoringSystem_LIBRARIES ${MonitoringSystem_LIBRARY})
    set(MonitoringSystem_INCLUDE_DIRS ${MonitoringSystem_INCLUDE_DIR})
    
    # Create imported target
    if(NOT TARGET MonitoringSystem::monitoring)
        add_library(MonitoringSystem::monitoring STATIC IMPORTED)
        set_target_properties(MonitoringSystem::monitoring PROPERTIES
            IMPORTED_LOCATION "${MonitoringSystem_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${MonitoringSystem_INCLUDE_DIR}"
            INTERFACE_COMPILE_FEATURES cxx_std_20
        )
        
        # Add thread dependency
        find_package(Threads REQUIRED)
        set_property(TARGET MonitoringSystem::monitoring APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
    endif()
endif()

mark_as_advanced(MonitoringSystem_INCLUDE_DIR MonitoringSystem_LIBRARY)