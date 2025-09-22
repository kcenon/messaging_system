project(monitoring_unit)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Direct linking for macOS
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

# Try to find GTest (vcpkg first, then system)
find_package(GTest CONFIG)
if(NOT GTest_FOUND)
    find_package(GTest REQUIRED)
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    monitoring
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

enable_testing()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

add_custom_target(${PROJECT_NAME}_test ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
    DEPENDS ${PROJECT_NAME}
)