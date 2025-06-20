##################################################
# Samples CMakeLists.txt
# 
# Builds sample applications demonstrating MessagingSystem usage.
# Provides example code for common use cases.
##################################################

# Project definition
project(samples 
    VERSION 1.0.0
    DESCRIPTION "Sample applications for MessagingSystem"
    LANGUAGES CXX
)

##################################################
# Sample Applications List
##################################################

# Define sample applications
set(SAMPLE_APPS
    simple_client_server
    container_demo
    database_example
    async_messaging
    performance_test
    lockfree_network_demo
    unified_network_benchmark
)

##################################################
# Common Configuration for All Samples
##################################################

# Common include directories for all samples
set(COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}/container
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/thread_system/sources
    ${CMAKE_SOURCE_DIR}/thread_system/sources/utilities
    ${CMAKE_SOURCE_DIR}/thread_system/sources/thread_base
    ${CMAKE_SOURCE_DIR}/thread_system/sources/thread_pool
    ${CMAKE_SOURCE_DIR}/thread_system/sources/logger
)

# Common libraries for all samples
set(COMMON_LIBS
    network
    container
    database
    thread_pool
    thread_base
    logger
    utilities
)

##################################################
# Build Each Sample Application
##################################################

foreach(SAMPLE ${SAMPLE_APPS})
    # Check if sample directory exists
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE}/main.cpp)
        # Create executable
        add_executable(${SAMPLE} ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE}/main.cpp)
        
        # Set properties
        set_target_properties(${SAMPLE} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        
        # Add include directories
        target_include_directories(${SAMPLE} PRIVATE ${COMMON_INCLUDES})
        
        # Link libraries
        foreach(lib ${COMMON_LIBS})
            if(TARGET ${lib})
                target_link_libraries(${SAMPLE} PRIVATE ${lib})
            endif()
        endforeach()
        
        # Platform-specific configuration
        if(WIN32)
            target_compile_definitions(${SAMPLE} PRIVATE
                _WIN32_WINNT=0x0601
                WIN32_LEAN_AND_MEAN
                NOMINMAX
            )
        endif()
        
        # Compiler options
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${SAMPLE} PRIVATE
                -Wall
                -Wextra
                -Wno-unused-parameter
            )
        elseif(MSVC)
            target_compile_options(${SAMPLE} PRIVATE
                /W4
                /WX-
            )
        endif()
        
        message(STATUS "  Sample: ${SAMPLE}")
    else()
        message(STATUS "  Sample: ${SAMPLE} (source not found - creating placeholder)")
        
        # Create a placeholder source file with basic template
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE}.cpp
"/**
 * @file ${SAMPLE}.cpp
 * @brief Sample application demonstrating ${SAMPLE} functionality
 * 
 * This is a placeholder file. Implement your sample code here.
 */

#include <iostream>
#include <memory>
#include \"container/container.h\"
#include \"network/messaging_server.h\"
#include \"network/messaging_client.h\"

int main(int argc, char* argv[])
{
    try
    {
        std::cout << \"${SAMPLE} sample application\\n\";
        std::cout << \"TODO: Implement ${SAMPLE} functionality\\n\";
        
        // Example: Create a container
        auto container = std::make_shared<container::value_container>();
        container->set_message_type(\"sample_message\");
        
        // TODO: Add your sample implementation here
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << \"Error: \" << e.what() << std::endl;
        return 1;
    }
}
")
    endif()
endforeach()

##################################################
# Additional Sample Resources
##################################################

# Copy sample configuration files if they exist
file(GLOB SAMPLE_CONFIGS ${CMAKE_CURRENT_SOURCE_DIR}/*.json ${CMAKE_CURRENT_SOURCE_DIR}/*.xml)
if(SAMPLE_CONFIGS)
    file(COPY ${SAMPLE_CONFIGS} DESTINATION ${CMAKE_BINARY_DIR}/bin/samples)
endif()

# Create README for samples
file(WRITE ${CMAKE_BINARY_DIR}/bin/samples/README.md
"# MessagingSystem Sample Applications

This directory contains sample applications demonstrating various features of the MessagingSystem library.

## Available Samples

1. **simple_client_server** - Basic client-server communication example
2. **container_demo** - Demonstrates container serialization and manipulation
3. **database_example** - Shows database integration with PostgreSQL
4. **async_messaging** - Asynchronous messaging patterns
5. **performance_test** - Performance benchmarking tools
6. **lockfree_network_demo** - Lock-free networking demonstration
7. **unified_network_benchmark** - Comprehensive network performance benchmark

## Running Samples

Each sample can be run directly from the command line:

```bash
./simple_client_server [options]
./container_demo
./database_example
./async_messaging
./performance_test
./lockfree_network_demo
./unified_network_benchmark
```

## Building Your Own Applications

Use these samples as templates for building your own applications with MessagingSystem.

For more information, see the main project documentation.
")

##################################################
# Installation Rules (Optional)
##################################################

if(NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    # Get list of actual built targets
    set(BUILT_SAMPLES)
    foreach(SAMPLE ${SAMPLE_APPS})
        if(TARGET ${SAMPLE})
            list(APPEND BUILT_SAMPLES ${SAMPLE})
        endif()
    endforeach()
    
    # Install samples to a separate directory
    if(BUILT_SAMPLES)
        install(TARGETS ${BUILT_SAMPLES}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/samples
        )
    endif()
    
    # Install sample resources
    if(SAMPLE_CONFIGS)
        install(FILES ${SAMPLE_CONFIGS}
            DESTINATION ${CMAKE_INSTALL_BINDIR}/samples
        )
    endif()
    
    install(FILES ${CMAKE_BINARY_DIR}/bin/samples/README.md
        DESTINATION ${CMAKE_INSTALL_BINDIR}/samples
    )
endif()

##################################################
# Summary
##################################################

message(STATUS "Sample applications configured:")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin/samples")
message(STATUS "  Available Samples:")