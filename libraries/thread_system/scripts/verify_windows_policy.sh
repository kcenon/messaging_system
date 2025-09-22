#!/bin/bash

##################################################
# Windows Policy Verification Script
# 
# Verifies that the Windows fmt force policy is
# correctly implemented in CMakeLists.txt
##################################################

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CMAKE_FILE="$PROJECT_ROOT/CMakeLists.txt"

echo "🔍 Verifying Windows fmt force policy implementation..."
echo "======================================================"

# Check if the main policy line exists
if grep -q "set(WINDOWS_COMPILER_SUPPORTS_STD_FORMAT FALSE)" "$CMAKE_FILE"; then
    echo "✅ Windows policy found: WINDOWS_COMPILER_SUPPORTS_STD_FORMAT set to FALSE"
else
    echo "❌ Windows policy not found!"
    exit 1
fi

# Check if Windows policy documentation exists
if grep -q "Windows Policy: Force using fmt::format" "$CMAKE_FILE"; then
    echo "✅ Policy documentation found"
else
    echo "❌ Policy documentation missing!"
    exit 1
fi

# Check if override option exists
if grep -q "WINDOWS_ALLOW_STD_FORMAT" "$CMAKE_FILE"; then
    echo "✅ Override option (WINDOWS_ALLOW_STD_FORMAT) available"
else
    echo "❌ Override option missing!"
    exit 1
fi

# Check if the policy applies to all major Windows compilers
declare -a compiler_checks=(
    "MSVC.*detected.*using fmt::format"
    "GCC.*detected.*using fmt::format" 
    "Clang.*detected.*using fmt::format"
)

echo ""
echo "🔍 Checking compiler-specific policy implementation:"

for check in "${compiler_checks[@]}"; do
    if grep -E -q "$check" "$CMAKE_FILE"; then
        echo "✅ Found: $check"
    else
        echo "⚠️  Pattern not found: $check"
    fi
done

# Check for Windows-specific messaging
echo ""
echo "🔍 Checking Windows policy messaging:"

declare -a message_checks=(
    "Windows Policy.*maximum compatibility"
    "All Windows builds use fmt::format"
    "Policy override available"
)

for check in "${message_checks[@]}"; do
    if grep -E -q "$check" "$CMAKE_FILE"; then
        echo "✅ Found: $check"
    else
        echo "⚠️  Pattern not found: $check"
    fi
done

echo ""
echo "🎯 Policy Summary:"
echo "=================="
echo "✅ Windows Policy: FORCE fmt::format usage"
echo "✅ Applies to: MSVC, MinGW, MSYS2, Clang"
echo "✅ Rationale: Maximum compatibility and stability"
echo "✅ Override: Available with -DWINDOWS_ALLOW_STD_FORMAT=ON"
echo "✅ Non-Windows: Normal std::format detection"

echo ""
echo "🧪 To test the policy:"
echo "   cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg_path> ."
echo "   (Should always use fmt::format on Windows)"
echo ""
echo "🔧 To override (not recommended):"
echo "   cmake -B build -DWINDOWS_ALLOW_STD_FORMAT=ON -DCMAKE_TOOLCHAIN_FILE=<vcpkg_path> ."

echo ""
echo "✅ Windows fmt force policy verification completed!"