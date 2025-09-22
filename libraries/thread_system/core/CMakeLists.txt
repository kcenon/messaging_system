##################################################
# Core Component (thread_base + jobs + sync)
##################################################

project(thread_base)

# Find headers in include directory
file(GLOB_RECURSE CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/kcenon/thread/core/*.h)

# Find sources in src directory
file(GLOB_RECURSE CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/impl/thread_pool/*.cpp)

add_library(${PROJECT_NAME} STATIC
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../interfaces>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    utilities
    interfaces
)

if(NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
endif()

if(COMMAND set_compiler_warnings)
    set_compiler_warnings(${PROJECT_NAME})
endif()
