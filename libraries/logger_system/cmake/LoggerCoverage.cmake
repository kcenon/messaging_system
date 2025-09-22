# LoggerCoverage.cmake
# Code coverage configuration for Logger System

# Coverage options
option(LOGGER_ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Function to add coverage flags to a target
function(logger_add_coverage target)
    if(NOT LOGGER_ENABLE_COVERAGE)
        return()
    endif()
    
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING "Code coverage is only supported in Debug builds")
        return()
    endif()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        # Coverage flags for GCC/Clang
        target_compile_options(${target} PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
            -O0  # No optimization for accurate coverage
            -g   # Debug symbols
        )
        
        target_link_options(${target} PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        
        message(STATUS "Enabled code coverage for ${target}")
        
    elseif(MSVC)
        message(WARNING "Code coverage is not yet supported for MSVC")
    else()
        message(WARNING "Code coverage is not supported for ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction()

# Function to setup coverage target
function(logger_setup_coverage_target)
    if(NOT LOGGER_ENABLE_COVERAGE)
        return()
    endif()
    
    # Find required tools
    find_program(GCOV_EXECUTABLE gcov)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    find_program(GCOVR_EXECUTABLE gcovr)
    
    if(GCOVR_EXECUTABLE)
        # Use gcovr for coverage report generation
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${GCOVR_EXECUTABLE}
                --root ${CMAKE_SOURCE_DIR}
                --filter ${CMAKE_SOURCE_DIR}/sources/
                --exclude ${CMAKE_SOURCE_DIR}/unittest/
                --exclude ${CMAKE_SOURCE_DIR}/tests/
                --html --html-details
                --output ${CMAKE_BINARY_DIR}/coverage/index.html
                --xml ${CMAKE_BINARY_DIR}/coverage/coverage.xml
                --json ${CMAKE_BINARY_DIR}/coverage/coverage.json
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report with gcovr"
        )
        
        message(STATUS "Coverage target 'coverage' created (using gcovr)")
        
    elseif(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        # Use lcov for coverage report generation
        add_custom_target(coverage
            # Cleanup
            COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
            
            # Run tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            
            # Capture coverage data
            COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
            
            # Remove unwanted files from coverage
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info 
                '/usr/*' '*/unittest/*' '*/tests/*' '*/build/*' 
                --output-file coverage.cleaned
            
            # Generate HTML report
            COMMAND ${GENHTML_EXECUTABLE} coverage.cleaned 
                --output-directory ${CMAKE_BINARY_DIR}/coverage
                --title "Logger System Coverage Report"
                --show-details --legend
            
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report with lcov"
        )
        
        message(STATUS "Coverage target 'coverage' created (using lcov)")
        
    else()
        message(WARNING "Coverage tools not found. Install gcovr or lcov+genhtml for coverage reports")
    endif()
    
    # Add coverage reset target
    add_custom_target(coverage-reset
        COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' -delete
        COMMENT "Resetting coverage data"
    )
endfunction()

# Macro to enable coverage for all targets
macro(logger_enable_coverage_for_all)
    if(LOGGER_ENABLE_COVERAGE)
        # Add coverage to main library
        if(TARGET logger_system)
            logger_add_coverage(logger_system)
        endif()
        
        if(TARGET logger)
            logger_add_coverage(logger)
        endif()
        
        # Add coverage to all test targets
        if(BUILD_TESTS)
            if(TARGET stress_test)
                logger_add_coverage(stress_test)
            endif()
            if(TARGET integration_test)
                logger_add_coverage(integration_test)
            endif()
            # Add more test targets as needed
        endif()
        
        # Setup coverage target
        logger_setup_coverage_target()
    endif()
endmacro()

# Print coverage configuration
if(LOGGER_ENABLE_COVERAGE)
    message(STATUS "========================================")
    message(STATUS "Coverage Configuration:")
    message(STATUS "  Coverage Enabled: YES")
    message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "========================================")
endif()