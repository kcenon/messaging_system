##################################################
# Unit Test CMakeLists.txt
# 
# Builds unit tests for the messaging system.
# Uses Google Test framework for comprehensive testing.
##################################################

# Project definition
project(unittest 
    VERSION 1.0.0
    DESCRIPTION "Unit tests for MessagingSystem"
    LANGUAGES CXX
)

##################################################
# Enable Testing
##################################################

enable_testing()

##################################################
# Find Google Test
##################################################

find_package(GTest CONFIG REQUIRED)

##################################################
# Source Files Configuration
##################################################

# Test source files
set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variant_container_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/container_thread_safe_test.cpp
)

##################################################
# Test Executable Configuration
##################################################

# Create test executable
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

##################################################
# Include Directories
##################################################

# Include directories for tests
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/container
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/thread_system/sources
    ${CMAKE_SOURCE_DIR}/thread_system/sources/utilities
    ${CMAKE_SOURCE_DIR}/thread_system/sources/utilities/parsing
    ${CMAKE_SOURCE_DIR}/thread_system/sources/thread_base
    ${CMAKE_SOURCE_DIR}/thread_system/sources/thread_pool
    ${CMAKE_SOURCE_DIR}/thread_system/sources/logger
    ${CMAKE_SOURCE_DIR}/thread_system/sources/monitoring
)

##################################################
# Dependencies
##################################################

# Link Google Test
target_link_libraries(${PROJECT_NAME} PRIVATE 
    GTest::gtest 
    GTest::gtest_main
)

# Link MessagingSystem libraries
set(MESSAGING_LIBS network container database)
foreach(lib ${MESSAGING_LIBS})
    if(TARGET ${lib})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
    else()
        message(WARNING "${lib} target not found for unit tests")
    endif()
endforeach()

# Link ThreadSystem libraries
set(THREAD_SYSTEM_LIBS thread_pool thread_base logger utilities)
foreach(lib ${THREAD_SYSTEM_LIBS})
    if(TARGET ${lib})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
    else()
        message(WARNING "${lib} target not found for unit tests")
    endif()
endforeach()

# Ensure all dependencies are built before tests
add_dependencies(${PROJECT_NAME} 
    network 
    container 
    database 
    thread_pool 
    thread_base 
    logger 
    utilities
)

##################################################
# Platform-Specific Configuration
##################################################

if(WIN32)
    # Windows-specific test configuration
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

##################################################
# Compiler Options
##################################################

# Add compiler warnings for tests
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /WX-
        /wd4100  # unreferenced formal parameter
        /wd4996  # deprecated functions
    )
endif()

##################################################
# Test Registration
##################################################

# Register tests with CTest
add_test(NAME MessagingSystemTests 
    COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Set test properties
set_tests_properties(MessagingSystemTests PROPERTIES
    TIMEOUT 300  # 5 minutes timeout
    ENVIRONMENT "GTEST_COLOR=1"
)

##################################################
# Test Discovery (if CMake >= 3.10)
##################################################

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.10")
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        PROPERTIES TIMEOUT 300
    )
endif()

##################################################
# Coverage Support (Optional)
##################################################

option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
    target_link_options(${PROJECT_NAME} PRIVATE --coverage)
    
    # Add custom target for coverage report
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory coverage
        COMMAND gcovr --root ${CMAKE_SOURCE_DIR} --html --html-details -o coverage/index.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${PROJECT_NAME}
        COMMENT "Generating code coverage report..."
    )
endif()

##################################################
# Valgrind Support (Optional)
##################################################

option(ENABLE_VALGRIND "Enable Valgrind memory checking" OFF)
if(ENABLE_VALGRIND)
    find_program(VALGRIND_EXECUTABLE valgrind)
    if(VALGRIND_EXECUTABLE)
        add_custom_target(valgrind
            COMMAND ${VALGRIND_EXECUTABLE} 
                --leak-check=full 
                --show-leak-kinds=all 
                --track-origins=yes 
                --verbose 
                --log-file=valgrind.log 
                ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${PROJECT_NAME}
            COMMENT "Running Valgrind memory check..."
        )
    else()
        message(WARNING "Valgrind not found - memory checking disabled")
    endif()
endif()

##################################################
# Summary
##################################################

message(STATUS "Unit tests configured:")
message(STATUS "  Test Sources: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Output: ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}")
message(STATUS "  Dependencies: GTest, network, container, database, thread_system libs")
if(ENABLE_COVERAGE)
    message(STATUS "  Code Coverage: ENABLED")
endif()
if(ENABLE_VALGRIND AND VALGRIND_EXECUTABLE)
    message(STATUS "  Valgrind Support: ENABLED")
endif()