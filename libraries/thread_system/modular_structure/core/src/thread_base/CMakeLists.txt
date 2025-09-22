##################################################
# ThreadBase Component CMakeLists.txt
# 
# Build configuration for the reorganized thread base library component.
# This component provides fundamental threading abstractions and primitives
# that serve as the foundation for higher-level thread management facilities.
##################################################

# Define the component name as a project
project(thread_base)

# Collect source files from reorganized directory structure
file(GLOB_RECURSE CORE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.h)
file(GLOB_RECURSE CORE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)

file(GLOB_RECURSE JOBS_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/jobs/*.h)
file(GLOB_RECURSE JOBS_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/jobs/*.cpp)

file(GLOB_RECURSE SYNC_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sync/*.h)
file(GLOB_RECURSE SYNC_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sync/*.cpp)

file(GLOB_RECURSE DETAIL_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/detail/*.h)
file(GLOB_RECURSE DETAIL_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/detail/*.cpp)

# Collect lock-free implementation files from reorganized structure
file(GLOB_RECURSE LOCKFREE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lockfree/*.h)
file(GLOB_RECURSE LOCKFREE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lockfree/*.cpp)
file(GLOB_RECURSE LOCKFREE_TEMPLATE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lockfree/*.tpp)

# Create a static library from the collected source files
add_library(${PROJECT_NAME} STATIC 
    ${CORE_HEADER_FILES}
    ${CORE_SOURCE_FILES}
    ${JOBS_HEADER_FILES}
    ${JOBS_SOURCE_FILES}
    ${SYNC_HEADER_FILES}
    ${SYNC_SOURCE_FILES}
    ${DETAIL_HEADER_FILES}
    ${DETAIL_SOURCE_FILES}
    ${LOCKFREE_HEADER_FILES}
    ${LOCKFREE_SOURCE_FILES}
    ${LOCKFREE_TEMPLATE_FILES}
)

# Configure include directories with proper path structure
# PUBLIC: For both building this library and consuming it
# Generator expressions ensure correct paths for build and install trees
target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/sources>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/jobs>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sync>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lockfree>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/detail>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/thread_system>
    $<INSTALL_INTERFACE:include/thread_system/thread_base>
)

# Apply compiler warnings if the function is available
if(COMMAND set_compiler_warnings)
    set_compiler_warnings(${PROJECT_NAME})
endif()

# Setup precompiled headers if supported
if(COMMAND target_precompile_headers_if_supported)
    target_precompile_headers_if_supported(${PROJECT_NAME})
endif()

# Platform-specific threading library configuration
if(APPLE)
    # On macOS, pthread is automatically linked by the system
    # We only need to link our own utilities library and interfaces
    target_link_libraries(${PROJECT_NAME} PUBLIC utilities interfaces)
else()
    # On other platforms, we need to explicitly find and link the threading library
    set(THREADS_PREFER_PTHREAD_FLAG ON)  # Prefer -pthread flag if available
    find_package(Threads REQUIRED)       # Find the threading library
    
    # Link against both our utilities and the system's threading library
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        utilities           # Our utility functions
        interfaces          # Logger and error handler interfaces
        Threads::Threads    # System threading library (usually pthread)
    )
endif()