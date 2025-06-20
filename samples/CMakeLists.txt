##################################################
# Samples CMakeLists.txt
# 
# Builds sample applications demonstrating MessagingSystem usage.
# Provides comprehensive examples for common use cases.
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

# Define consolidated sample applications
set(SAMPLE_APPS
    basic_demo             # Basic functionality demonstration
    # network_examples        # Comprehensive networking demonstrations (needs API update)
    # performance_benchmark   # Performance testing and benchmarking (needs API update)
    # container_usage        # Container manipulation examples (needs API update)
    # database_integration   # Database integration examples (needs API update)
    # interop_example       # Language interoperability example (needs API update)
)

##################################################
# Common Configuration for All Samples
##################################################

# Common include directories for all samples
set(COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/container
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/thread_system/sources
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
    ${CMAKE_THREAD_LIBS_INIT}
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
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/samples
        )
        
        # Add include directories
        target_include_directories(${SAMPLE} PRIVATE ${COMMON_INCLUDES})
        
        # Link libraries
        target_link_libraries(${SAMPLE} PRIVATE ${COMMON_LIBS})
        
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
        message(WARNING "  Sample: ${SAMPLE} (main.cpp not found - skipping)")
    endif()
endforeach()

##################################################
# Sample Documentation
##################################################

# Create comprehensive README for samples
file(WRITE ${CMAKE_BINARY_DIR}/bin/samples/README.md
"# MessagingSystem Sample Applications

This directory contains comprehensive sample applications demonstrating various features of the MessagingSystem library.

## Sample Applications

### 1. network_examples
Comprehensive networking demonstrations including:
- Basic client-server communication
- Multiple concurrent clients
- Asynchronous messaging patterns
- Error handling and recovery

**Usage:**
\`\`\`bash
./network_examples [basic|multiple|async|errors|all]
\`\`\`

### 2. performance_benchmark
Performance testing and benchmarking suite:
- Message throughput measurement
- Latency profiling
- Scalability testing
- Resource usage monitoring

**Usage:**
\`\`\`bash
./performance_benchmark [latency|throughput|scalability|all]
\`\`\`

### 3. container_usage
Container manipulation and usage examples:
- Creating and modifying containers
- Serialization/deserialization
- Thread-safe operations
- SIMD optimizations
- All supported value types

**Usage:**
\`\`\`bash
./container_usage [basic|serial|nested|thread|simd|types|all]
\`\`\`

### 4. database_integration
PostgreSQL database integration examples:
- Connection management
- Storing/retrieving containers
- Transaction handling
- Bulk operations
- Advanced queries

**Usage:**
\`\`\`bash
./database_integration \"host=localhost port=5432 dbname=messaging user=postgres password=secret\" [create|store|retrieve|transaction|bulk|query|all]
\`\`\`

### 5. interop_example
Cross-language interoperability demonstration:
- C++ client communicating with Python server
- Protocol compatibility testing
- Message format validation

**Usage:**
\`\`\`bash
# First start the Python server:
python3 -m messaging_system.samples.simple_server 0.0.0.0 8080

# Then run the C++ client:
./interop_example 127.0.0.1 8080
\`\`\`

## Building Samples

All samples are built automatically with the main project:

\`\`\`bash
cd /path/to/messaging_system
./build.sh --clean
\`\`\`

## Prerequisites

1. **Network Examples**: No special requirements
2. **Performance Benchmark**: Sufficient system resources for stress testing
3. **Container Usage**: No special requirements
4. **Database Integration**: PostgreSQL server running and accessible
5. **Interop Example**: Python messaging_system module installed

## Common Patterns

All samples demonstrate best practices including:
- Proper error handling
- Resource management (RAII)
- Thread safety where applicable
- Performance considerations
- Clear code organization

## Extending Samples

To add new functionality to existing samples:
1. Follow the existing code structure
2. Add new demonstration functions
3. Update the command-line interface
4. Document new features in this README

For more information, see the main project documentation.
")

##################################################
# Installation Rules
##################################################

if(NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    # Install samples
    install(TARGETS ${SAMPLE_APPS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/samples
    )
    
    # Install documentation
    install(FILES ${CMAKE_BINARY_DIR}/bin/samples/README.md
        DESTINATION ${CMAKE_INSTALL_BINDIR}/samples
    )
endif()

##################################################
# Summary
##################################################

message(STATUS "Sample applications configured:")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin/samples")
message(STATUS "  Configured Samples:")