cmake_minimum_required(VERSION 3.16)
project(MessagingSystemIntegration)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find network_system (assuming it's built in ../../build)
set(NetworkSystem_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(NetworkSystem_BUILD_DIR "${NetworkSystem_DIR}/build")

# Add include directories
include_directories(${NetworkSystem_DIR}/include)

# Link to network_system library
link_directories(${NetworkSystem_BUILD_DIR})

# Legacy compatibility example
add_executable(legacy_compatibility legacy_compatibility.cpp)
target_link_libraries(legacy_compatibility
    PRIVATE
    ${NetworkSystem_BUILD_DIR}/libNetworkSystem.a
    pthread
)

# Modern usage example
add_executable(modern_usage modern_usage.cpp)
target_link_libraries(modern_usage
    PRIVATE
    ${NetworkSystem_BUILD_DIR}/libNetworkSystem.a
    pthread
)

# Add container_system include if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../container_system")
    target_include_directories(legacy_compatibility
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../container_system
    )
    target_include_directories(modern_usage
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../container_system
    )
    target_compile_definitions(legacy_compatibility PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    target_compile_definitions(modern_usage PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
endif()

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FMT REQUIRED IMPORTED_TARGET fmt)

    target_link_libraries(legacy_compatibility PRIVATE PkgConfig::FMT)
    target_link_libraries(modern_usage PRIVATE PkgConfig::FMT)

    # Add ASIO include path
    target_include_directories(legacy_compatibility PRIVATE /opt/homebrew/include)
    target_include_directories(modern_usage PRIVATE /opt/homebrew/include)

    target_compile_definitions(legacy_compatibility PRIVATE ASIO_STANDALONE)
    target_compile_definitions(modern_usage PRIVATE ASIO_STANDALONE)
elseif(WIN32)
    # Windows specific settings
    target_link_libraries(legacy_compatibility PRIVATE ws2_32 mswsock)
    target_link_libraries(modern_usage PRIVATE ws2_32 mswsock)
    target_compile_definitions(legacy_compatibility PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(modern_usage PRIVATE _WIN32_WINNT=0x0601)
endif()

# Output directory
set_target_properties(legacy_compatibility modern_usage
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)

# Installation rules (optional)
install(TARGETS legacy_compatibility modern_usage
    RUNTIME DESTINATION bin
)

message(STATUS "Messaging System Integration Examples configured")
message(STATUS "  - legacy_compatibility: Demonstrates backward compatibility")
message(STATUS "  - modern_usage: Shows modern API features")