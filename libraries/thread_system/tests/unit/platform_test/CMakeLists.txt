project(platform_test)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Create the test executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Add GTest include path (if vcpkg is used)
if(DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../build/vcpkg_installed")
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/vcpkg_installed/arm64-osx/include)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/vcpkg_installed/x64-osx/include)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/vcpkg_installed/x64-linux/include)
    endif()
endif()

# Include new headers for core and thread_pool
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/base/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/jobs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/sync/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../implementations/thread_pool/include
)

# Try to find GTest (vcpkg first, then system)
find_package(GTest CONFIG)
if(NOT GTest_FOUND)
    find_package(GTest REQUIRED)
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    thread_base
    utilities
    interfaces
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Link atomic library on Linux for 16-byte atomic operations
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME} PRIVATE atomic)
endif()

enable_testing()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

add_custom_target(${PROJECT_NAME}_test ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
    DEPENDS ${PROJECT_NAME}
)

# Apply sanitizers if enabled
if(COMMAND apply_sanitizers)
    apply_sanitizers(${PROJECT_NAME})
endif()
