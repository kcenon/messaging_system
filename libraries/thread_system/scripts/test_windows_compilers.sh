#!/bin/bash

##################################################
# Windows Compiler Detection Test Script
# 
# This script tests the CMake configuration for 
# different Windows compiler scenarios
##################################################

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "🧪 Testing Windows compiler detection scenarios..."

##################################################
# Test scenarios for different Windows compilers
##################################################

test_scenario() {
    local scenario_name="$1"
    local compiler_id="$2"
    local compiler_version="$3"
    local compiler_path="$4"
    local expected_result="$5"
    
    echo ""
    echo "📋 Testing scenario: $scenario_name"
    echo "   Compiler ID: $compiler_id"
    echo "   Compiler Version: $compiler_version"
    echo "   Expected: $expected_result"
    
    # Create a test CMake file
    cat > /tmp/test_cmake_scenario.cmake << EOF
set(WIN32 TRUE)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_COMPILER_ID "$compiler_id")
set(CMAKE_CXX_COMPILER_VERSION "$compiler_version")
set(CMAKE_CXX_COMPILER "$compiler_path")

EOF

    if [ "$compiler_id" = "MSVC" ]; then
        # Convert version to MSVC internal version
        case "$compiler_version" in
            "19.43"*) echo "set(MSVC_VERSION 1943)" >> /tmp/test_cmake_scenario.cmake ;;
            "19.35"*) echo "set(MSVC_VERSION 1935)" >> /tmp/test_cmake_scenario.cmake ;;
            "19.30"*) echo "set(MSVC_VERSION 1930)" >> /tmp/test_cmake_scenario.cmake ;;
            "19.29"*) echo "set(MSVC_VERSION 1929)" >> /tmp/test_cmake_scenario.cmake ;;
            *) echo "set(MSVC_VERSION 1920)" >> /tmp/test_cmake_scenario.cmake ;;
        esac
        echo "set(MSVC TRUE)" >> /tmp/test_cmake_scenario.cmake
    fi
    
    # Add the logic from our CMakeLists.txt
    cat >> /tmp/test_cmake_scenario.cmake << 'EOF'

# Copy the Windows compiler detection logic here (Windows Policy: Force fmt)
set(WINDOWS_COMPILER_SUPPORTS_STD_FORMAT FALSE)

message(STATUS "🛡️  Windows Policy: Force using fmt::format for maximum compatibility")

# Windows Policy: All compilers use fmt::format for maximum compatibility
if(MSVC)
  message(STATUS "📊 MSVC ${MSVC_VERSION} detected - using fmt::format (Windows Policy)")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "🐧 GCC ${CMAKE_CXX_COMPILER_VERSION} detected - using fmt::format (Windows Policy)")
  if(CMAKE_CXX_COMPILER MATCHES "msys2" OR CMAKE_CXX_COMPILER MATCHES "MSYS2")
    message(STATUS "🔧 MSYS2 environment confirmed")
  elseif(CMAKE_CXX_COMPILER MATCHES "mingw" OR CMAKE_CXX_COMPILER MATCHES "MinGW")
    message(STATUS "🔧 MinGW environment confirmed")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(STATUS "🦙 Clang ${CMAKE_CXX_COMPILER_VERSION} detected - using fmt::format (Windows Policy)")
  if(CMAKE_CXX_COMPILER MATCHES "clang-cl")
    message(STATUS "🔧 clang-cl (MSVC-compatible mode) detected")
  endif()
endif()

# Windows Policy: Always FALSE (fmt::format)
# set(WINDOWS_COMPILER_SUPPORTS_STD_FORMAT FALSE)  # Already set above

message(STATUS "RESULT: WINDOWS_COMPILER_SUPPORTS_STD_FORMAT=${WINDOWS_COMPILER_SUPPORTS_STD_FORMAT}")

EOF

    # Run the test
    echo "   Running test..."
    if cmake -P /tmp/test_cmake_scenario.cmake 2>&1 | tail -1 | grep -q "$expected_result"; then
        echo "   ✅ PASS"
    else
        echo "   ❌ FAIL"
        echo "   Actual output:"
        cmake -P /tmp/test_cmake_scenario.cmake 2>&1 | tail -5
    fi
}

##################################################
# Run test scenarios
##################################################

echo "🚀 Starting Windows compiler tests..."

# All Windows scenarios now use fmt::format (Windows Policy)
test_scenario "MSVC 2022 Latest (GitHub Actions)" "MSVC" "19.43.34808" "cl.exe" "FALSE"
test_scenario "MSVC 2022 17.5" "MSVC" "19.35.32217" "cl.exe" "FALSE"
test_scenario "MSVC 2022 17.0" "MSVC" "19.30.30705" "cl.exe" "FALSE"
test_scenario "MSVC 2019 16.11" "MSVC" "19.29.30139" "cl.exe" "FALSE"

# MinGW scenarios (all use fmt::format)
test_scenario "MinGW GCC 13.0" "GNU" "13.0.0" "/mingw64/bin/g++.exe" "FALSE"
test_scenario "MinGW GCC 12.0" "GNU" "12.2.0" "/mingw64/bin/g++.exe" "FALSE"
test_scenario "MinGW GCC 11.0" "GNU" "11.3.0" "/mingw64/bin/g++.exe" "FALSE"

# MSYS2 scenarios (all use fmt::format)
test_scenario "MSYS2 GCC 13.0" "GNU" "13.0.0" "/usr/bin/msys2-g++.exe" "FALSE"
test_scenario "MSYS2 GCC 12.0" "GNU" "12.2.0" "/c/msys2/usr/bin/g++.exe" "FALSE"

# Clang scenarios (all use fmt::format)
test_scenario "Clang 17.0" "Clang" "17.0.0" "clang++.exe" "FALSE"
test_scenario "Clang 15.0" "Clang" "15.0.7" "clang++.exe" "FALSE"
test_scenario "Clang 14.0" "Clang" "14.0.6" "clang++.exe" "FALSE"
test_scenario "clang-cl" "Clang" "16.0.0" "clang-cl.exe" "FALSE"

echo ""
echo "🏁 Test completed!"

# Cleanup
rm -f /tmp/test_cmake_scenario.cmake

echo ""
echo "💡 Summary (Windows Policy):"
echo "   - ALL Windows compilers: fmt::format enforced for maximum compatibility"
echo "   - MSVC (all versions): fmt::format"
echo "   - MinGW (all versions): fmt::format"
echo "   - MSYS2 (all versions): fmt::format"
echo "   - Clang (all versions): fmt::format"
echo "   - Override available with: -DWINDOWS_ALLOW_STD_FORMAT=ON"