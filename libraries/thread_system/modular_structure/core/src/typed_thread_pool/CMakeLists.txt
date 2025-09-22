##################################################
# TypedThreadPool Component CMakeLists.txt
# 
# Build configuration for the type-based thread pool library component.
# This component extends the base thread pool with task type specialization capabilities,
# allowing different types of tasks to be processed by specialized worker threads.
##################################################

# Define the component name as a project
project(typed_thread_pool)

# Collect source files for explicit template instantiations
set(TYPED_THREAD_POOL_SOURCES
    scheduling/adaptive_typed_job_queue.cpp
    scheduling/typed_lockfree_job_queue.cpp
)

# Create a static library (now contains template instantiations)
add_library(${PROJECT_NAME} STATIC ${TYPED_THREAD_POOL_SOURCES})

# Configure include directories with proper path structure
# PUBLIC: Both for this library and its consumers
# Generator expressions ensure correct paths for build and install trees
target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/sources>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/thread_system>
    $<INSTALL_INTERFACE:include/thread_system/typed_thread_pool>
)

# Link against required dependencies:
# - thread_base: For core threading primitives
# - thread_pool: This component extends the thread pool implementation
# - interfaces: For diagnostic and error logging interfaces
target_link_libraries(${PROJECT_NAME} PUBLIC thread_base thread_pool interfaces)