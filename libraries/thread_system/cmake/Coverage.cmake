# Coverage configuration for thread_system
# This module provides functions to enable code coverage analysis

option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Configuring code coverage...")
        
        # Add coverage compiler flags
        set(COVERAGE_COMPILER_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
            CACHE INTERNAL "")
        
        # Add coverage linker flags
        set(COVERAGE_LINKER_FLAGS "--coverage"
            CACHE INTERNAL "")
        
        # Function to setup target for coverage
        function(setup_target_for_coverage)
            set(options)
            set(oneValueArgs NAME TARGET OUTPUT)
            set(multiValueArgs DEPENDENCIES)
            cmake_parse_arguments(Coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
            
            if(NOT Coverage_NAME)
                message(FATAL_ERROR "setup_target_for_coverage: NAME not specified")
            endif()
            
            if(NOT Coverage_TARGET)
                message(FATAL_ERROR "setup_target_for_coverage: TARGET not specified")
            endif()
            
            if(NOT Coverage_OUTPUT)
                set(Coverage_OUTPUT ${CMAKE_BINARY_DIR}/coverage)
            endif()
            
            # Setup target
            add_custom_target(${Coverage_NAME}
                # Cleanup lcov
                COMMAND ${CMAKE_COMMAND} -E make_directory ${Coverage_OUTPUT}
                COMMAND lcov --directory . --zerocounters
                
                # Run tests
                COMMAND ${Coverage_TARGET}
                
                # Capture coverage info
                COMMAND lcov --directory . --capture --output-file ${Coverage_OUTPUT}/${Coverage_NAME}.info
                
                # Remove system headers and external libraries
                COMMAND lcov --remove ${Coverage_OUTPUT}/${Coverage_NAME}.info 
                    '/usr/*' 
                    '*/vcpkg/*' 
                    '*/unittest/*' 
                    '*/benchmarks/*' 
                    '*/samples/*'
                    --output-file ${Coverage_OUTPUT}/${Coverage_NAME}.cleaned.info
                
                # Generate HTML report
                COMMAND genhtml -o ${Coverage_OUTPUT}/${Coverage_NAME} ${Coverage_OUTPUT}/${Coverage_NAME}.cleaned.info
                
                # Print summary
                COMMAND lcov --summary ${Coverage_OUTPUT}/${Coverage_NAME}.cleaned.info
                
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS ${Coverage_DEPENDENCIES}
                COMMENT "Running coverage analysis for ${Coverage_NAME}"
            )
            
            # Print report location
            add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Coverage report: ${Coverage_OUTPUT}/${Coverage_NAME}/index.html"
            )
        endfunction()
        
        # Function to add coverage flags to a target
        function(target_enable_coverage TARGET)
            if(TARGET ${TARGET})
                target_compile_options(${TARGET} PRIVATE ${COVERAGE_COMPILER_FLAGS})
                target_link_options(${TARGET} PRIVATE ${COVERAGE_LINKER_FLAGS})
            endif()
        endfunction()
        
        # Enable coverage for all targets
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_LINKER_FLAGS}")
        
        message(STATUS "Code coverage enabled")
        message(STATUS "  Compiler flags: ${COVERAGE_COMPILER_FLAGS}")
        message(STATUS "  Linker flags: ${COVERAGE_LINKER_FLAGS}")
        
        # Check for required tools
        find_program(LCOV lcov)
        if(NOT LCOV)
            message(WARNING "lcov not found! Coverage reports will not be available.")
            message(WARNING "Install lcov: brew install lcov (macOS) or apt-get install lcov (Linux)")
        endif()
        
        find_program(GENHTML genhtml)
        if(NOT GENHTML)
            message(WARNING "genhtml not found! HTML coverage reports will not be available.")
        endif()
        
    else()
        message(WARNING "Code coverage is only supported for GCC and Clang compilers")
        set(ENABLE_COVERAGE OFF CACHE BOOL "Enable code coverage analysis" FORCE)
    endif()
endif()

# Function to create a combined coverage report for all tests
function(create_coverage_report)
    if(ENABLE_COVERAGE AND LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage/all
            COMMAND lcov --directory . --zerocounters
            
            # Run all tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            
            # Capture coverage data
            COMMAND lcov --directory . --capture --output-file ${CMAKE_BINARY_DIR}/coverage/all.info
            
            # Remove unwanted files from coverage
            COMMAND lcov --remove ${CMAKE_BINARY_DIR}/coverage/all.info
                '/usr/*'
                '*/vcpkg/*'
                '*/build/*'
                '*/unittest/*'
                '*/benchmarks/*'
                '*/samples/*'
                '*/test/*'
                --output-file ${CMAKE_BINARY_DIR}/coverage/all.cleaned.info
            
            # Generate HTML report
            COMMAND genhtml 
                --demangle-cpp
                --num-spaces 2
                --sort
                --title "Thread System Coverage Report"
                --function-coverage
                --branch-coverage
                --output-directory ${CMAKE_BINARY_DIR}/coverage/all
                ${CMAKE_BINARY_DIR}/coverage/all.cleaned.info
            
            # Print summary
            COMMAND lcov --summary ${CMAKE_BINARY_DIR}/coverage/all.cleaned.info
            
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report for all tests"
        )
        
        # Print instructions
        add_custom_command(TARGET coverage POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo ""
            COMMAND ${CMAKE_COMMAND} -E echo "===================================="
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated:"
            COMMAND ${CMAKE_COMMAND} -E echo "  ${CMAKE_BINARY_DIR}/coverage/all/index.html"
            COMMAND ${CMAKE_COMMAND} -E echo ""
            COMMAND ${CMAKE_COMMAND} -E echo "To view: open ${CMAKE_BINARY_DIR}/coverage/all/index.html"
            COMMAND ${CMAKE_COMMAND} -E echo "===================================="
        )
    endif()
endfunction()