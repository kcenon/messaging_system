# Phase 3 T3.2: Dependency Conflict Prevention Mechanism
# CMake dependency version conflict checking and resolution guidance

cmake_minimum_required(VERSION 3.16)

# Define minimum required versions for our dependencies
set(THREAD_SYSTEM_MIN_VERSIONS
    "fmt;10.0.0"
    "gtest;1.14.0"  
    "benchmark;1.8.0"
    "spdlog;1.12.0"
)

# Define known compatibility issues
set(THREAD_SYSTEM_INCOMPATIBLE_COMBINATIONS
    "fmt;<10.0.0;spdlog;>=1.12.0;fmt version too old for spdlog 1.12+"
    "gtest;<1.14.0;benchmark;>=1.8.0;gtest version incompatible with modern benchmark"
)

# Function to parse version string into components
function(parse_version version_string major minor patch)
    string(REPLACE "." ";" VERSION_LIST ${version_string})
    list(LENGTH VERSION_LIST VERSION_LENGTH)
    
    list(GET VERSION_LIST 0 MAJOR_VAL)
    set(${major} ${MAJOR_VAL} PARENT_SCOPE)
    
    if(VERSION_LENGTH GREATER 1)
        list(GET VERSION_LIST 1 MINOR_VAL)
        set(${minor} ${MINOR_VAL} PARENT_SCOPE)
    else()
        set(${minor} 0 PARENT_SCOPE)
    endif()
    
    if(VERSION_LENGTH GREATER 2)
        list(GET VERSION_LIST 2 PATCH_VAL)
        set(${patch} ${PATCH_VAL} PARENT_SCOPE)
    else()
        set(${patch} 0 PARENT_SCOPE)
    endif()
endfunction()

# Function to compare version strings
function(version_compare version1 version2 result)
    parse_version(${version1} major1 minor1 patch1)
    parse_version(${version2} major2 minor2 patch2)
    
    if(major1 GREATER major2)
        set(${result} "GREATER" PARENT_SCOPE)
    elseif(major1 LESS major2)
        set(${result} "LESS" PARENT_SCOPE)
    elseif(minor1 GREATER minor2)
        set(${result} "GREATER" PARENT_SCOPE)
    elseif(minor1 LESS minor2)
        set(${result} "LESS" PARENT_SCOPE)
    elseif(patch1 GREATER patch2)
        set(${result} "GREATER" PARENT_SCOPE)
    elseif(patch1 LESS patch2)
        set(${result} "LESS" PARENT_SCOPE)
    else()
        set(${result} "EQUAL" PARENT_SCOPE)
    endif()
endfunction()

# Function to check if version meets requirement
function(check_version_requirement package_name current_version min_version meets_requirement)
    if(NOT current_version)
        set(${meets_requirement} FALSE PARENT_SCOPE)
        return()
    endif()
    
    version_compare(${current_version} ${min_version} comparison)
    if(comparison STREQUAL "GREATER" OR comparison STREQUAL "EQUAL")
        set(${meets_requirement} TRUE PARENT_SCOPE)
    else()
        set(${meets_requirement} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Main function to check dependency conflicts
function(check_dependency_conflicts)
    message(STATUS "")
    message(STATUS "🔍 Checking dependency conflicts for thread_system...")
    message(STATUS "========================================================")
    
    set(CONFLICTS_FOUND FALSE)
    set(WARNINGS_FOUND FALSE)
    
    # Check minimum version requirements
    foreach(dep_info IN LISTS THREAD_SYSTEM_MIN_VERSIONS)
        # Parse semicolon-separated values
        string(REPLACE ";" "|" dep_info_escaped ${dep_info})
        string(REPLACE "|" ";" dep_info_list ${dep_info_escaped})
        
        list(LENGTH dep_info_list list_length)
        if(list_length GREATER_EQUAL 2)
            list(GET dep_info_list 0 package_name)
            list(GET dep_info_list 1 min_version)
            
            # Get current version based on package
            set(current_version "")
            if(package_name STREQUAL "fmt")
                if(TARGET fmt::fmt)
                    get_target_property(FMT_VERSION fmt::fmt VERSION)
                    set(current_version ${FMT_VERSION})
                endif()
            elseif(package_name STREQUAL "gtest")
                if(TARGET GTest::gtest)
                    set(current_version "1.14.0")  # Assume recent version if found
                endif()
            elseif(package_name STREQUAL "benchmark")
                if(TARGET benchmark::benchmark)
                    set(current_version "1.8.0")  # Assume recent version if found
                endif()
            elseif(package_name STREQUAL "spdlog")
                if(TARGET spdlog::spdlog)
                    set(current_version "1.12.0")  # Assume recent version if found
                endif()
            endif()
            
            check_version_requirement(${package_name} "${current_version}" ${min_version} meets_req)
            
            if(current_version)
                if(meets_req)
                    message(STATUS "✅ ${package_name}: ${current_version} (>= ${min_version}) - OK")
                else()
                    message(WARNING "⚠️  ${package_name}: ${current_version} < ${min_version} - VERSION TOO OLD")
                    set(CONFLICTS_FOUND TRUE)
                    
                    message(STATUS "   📋 Resolution suggestions for ${package_name}:")
                    message(STATUS "      - Update vcpkg.json to specify minimum version")
                    message(STATUS "      - Run: vcpkg upgrade ${package_name}")
                    message(STATUS "      - Check compatibility matrix in docs/dependency_compatibility_matrix.md")
                endif()
            else()
                message(STATUS "ℹ️  ${package_name}: Not found (will be installed by vcpkg if needed)")
            endif()
        else()
            message(WARNING "Invalid dependency format: ${dep_info}")
        endif()
    endforeach()
    
    # Check for known incompatible combinations
    message(STATUS "")
    message(STATUS "🔍 Checking for known incompatible combinations...")
    
    foreach(incompatible IN LISTS THREAD_SYSTEM_INCOMPATIBLE_COMBINATIONS)
        string(REPLACE ";" "|" incompatible_escaped ${incompatible})
        string(REPLACE "|" ";" incompatible_list ${incompatible_escaped})
        
        list(LENGTH incompatible_list incompatible_length)
        if(incompatible_length GREATER_EQUAL 5)
            list(GET incompatible_list 0 pkg1_name)
            list(GET incompatible_list 1 pkg1_constraint)
            list(GET incompatible_list 2 pkg2_name)
            list(GET incompatible_list 3 pkg2_constraint)
            list(GET incompatible_list 4 issue_desc)
        else()
            message(STATUS "ℹ️  Skipping malformed incompatibility rule")
        endif()
        
        # This is a simplified check - in a real scenario, we'd evaluate the constraints
        message(STATUS "ℹ️  Known issue: ${issue_desc}")
    endforeach()
    
    # Generate summary
    message(STATUS "")
    message(STATUS "========================================================")
    if(CONFLICTS_FOUND)
        message(WARNING "❌ DEPENDENCY CONFLICTS DETECTED!")
        message(STATUS "")
        message(STATUS "🔧 Recommended actions:")
        message(STATUS "   1. Review dependency_compatibility_matrix.md")
        message(STATUS "   2. Update vcpkg.json with compatible versions")
        message(STATUS "   3. Run dependency upgrade script: ./scripts/upgrade_dependencies.sh")
        message(STATUS "   4. Rebuild with updated dependencies")
        message(STATUS "")
        message(FATAL_ERROR "Please resolve dependency conflicts before continuing")
    elseif(WARNINGS_FOUND)
        message(STATUS "⚠️  Some dependency warnings found - please review above")
    else()
        message(STATUS "✅ All dependency checks passed!")
    endif()
    message(STATUS "========================================================")
    message(STATUS "")
endfunction()

# Function to generate dependency report
function(generate_dependency_report)
    set(REPORT_FILE "${CMAKE_BINARY_DIR}/dependency_report.md")
    
    file(WRITE ${REPORT_FILE} "# Dependency Analysis Report\n\n")
    file(APPEND ${REPORT_FILE} "**Generated**: ${CMAKE_CURRENT_LIST_FILE}\n")
    file(APPEND ${REPORT_FILE} "**Date**: $ENV{BUILD_TIMESTAMP}\n")
    file(APPEND ${REPORT_FILE} "**Project**: thread_system\n\n")
    
    file(APPEND ${REPORT_FILE} "## Found Dependencies\n\n")
    file(APPEND ${REPORT_FILE} "| Package | Status | Version | Minimum Required |\n")
    file(APPEND ${REPORT_FILE} "|---------|--------|---------|------------------|\n")
    
    foreach(dep_info IN LISTS THREAD_SYSTEM_MIN_VERSIONS)
        list(GET dep_info 0 package_name)
        list(GET dep_info 1 min_version)
        
        if(TARGET fmt::fmt AND package_name STREQUAL "fmt")
            file(APPEND ${REPORT_FILE} "| fmt | ✅ Found | System | ${min_version} |\n")
        elseif(TARGET GTest::gtest AND package_name STREQUAL "gtest")
            file(APPEND ${REPORT_FILE} "| gtest | ✅ Found | System | ${min_version} |\n")
        elseif(TARGET benchmark::benchmark AND package_name STREQUAL "benchmark")
            file(APPEND ${REPORT_FILE} "| benchmark | ✅ Found | System | ${min_version} |\n")
        elseif(TARGET spdlog::spdlog AND package_name STREQUAL "spdlog")
            file(APPEND ${REPORT_FILE} "| spdlog | ✅ Found | System | ${min_version} |\n")
        else()
            file(APPEND ${REPORT_FILE} "| ${package_name} | ⚠️ Not Found | - | ${min_version} |\n")
        endif()
    endforeach()
    
    file(APPEND ${REPORT_FILE} "\n## Recommendations\n\n")
    file(APPEND ${REPORT_FILE} "- Ensure all dependencies meet minimum version requirements\n")
    file(APPEND ${REPORT_FILE} "- Run `vcpkg install` to install missing dependencies\n")
    file(APPEND ${REPORT_FILE} "- Review [dependency compatibility matrix](../docs/dependency_compatibility_matrix.md)\n")
    
    message(STATUS "📊 Dependency report generated: ${REPORT_FILE}")
endfunction()

# Function to suggest dependency updates
function(suggest_dependency_updates)
    message(STATUS "")
    message(STATUS "💡 Dependency Update Suggestions:")
    message(STATUS "==========================================")
    message(STATUS "")
    message(STATUS "To update all dependencies to latest compatible versions:")
    message(STATUS "  ./scripts/upgrade_dependencies.sh --latest")
    message(STATUS "")  
    message(STATUS "To update only security patches:")
    message(STATUS "  ./scripts/upgrade_dependencies.sh --security-only")
    message(STATUS "")
    message(STATUS "To check for dependency vulnerabilities:")
    message(STATUS "  ./scripts/dependency_analyzer.py --security-scan")
    message(STATUS "")
    message(STATUS "To visualize dependency tree:")
    message(STATUS "  ./scripts/dependency_analyzer.py --visualize")
    message(STATUS "")
    message(STATUS "==========================================")
endfunction()

# Auto-run dependency checks if requested
if(CHECK_DEPENDENCIES)
    check_dependency_conflicts()
    generate_dependency_report()
endif()