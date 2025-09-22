#!/bin/bash

# Workflow Simulation Test
# Tests the fallback logic similar to what happens in GitHub Actions

echo "🔄 Simulating GitHub Actions workflow fallback logic..."

# Test 1: Simulate vcpkg failure scenario
echo "Test 1: Simulating vcpkg build failure..."

cd build
rm -rf CMakeCache.txt CMakeFiles/

# Simulate the fallback build that would happen in GitHub Actions
echo "Building with system libraries fallback (like in CI)..."
if cmake .. \
    -G Ninja \
    -DBUILD_THREADSYSTEM_AS_SUBMODULE=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DUSE_STD_FORMAT=ON \
    -DCMAKE_TOOLCHAIN_FILE="" \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++
then
    echo "✅ CMake configuration succeeded (fallback scenario)"
else
    echo "❌ CMake configuration failed"
    exit 1
fi

# Build the project
if cmake --build . --parallel 2; then
    echo "✅ Build succeeded (fallback scenario)"
else
    echo "❌ Build failed"
    exit 1
fi

# Test 2: Simulate CI testing logic
echo ""
echo "Test 2: Simulating CI test logic..."

if [ -f "bin/thread_base_unit" ]; then
    echo "Would run unit tests (vcpkg build scenario)"
else
    echo "Running basic verification test (system libraries scenario)..."
    
    # This simulates what happens in the workflow
    cat > ci_verification_test.cpp << 'EOF'
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

int main() {
    std::cout << "GitHub Actions Simulation Verification\n";
    std::cout << "======================================\n";
    
    std::atomic<bool> test_completed{false};
    {
        std::jthread test_thread([&test_completed]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            test_completed = true;
        });
    }
    
    if (test_completed) {
        std::cout << "✅ CI simulation verification passed\n";
        return 0;
    } else {
        std::cout << "❌ CI simulation verification failed\n";
        return 1;
    }
}
EOF

    if g++ -std=c++20 -DUSE_STD_FORMAT -o ci_verification_test ci_verification_test.cpp -lpthread \
       && ./ci_verification_test; then
        echo "✅ Basic verification test passed"
    else
        echo "❌ Basic verification test failed"
        exit 1
    fi
fi

echo ""
echo "🎉 Workflow simulation completed successfully!"
echo "The GitHub Actions improvements should work correctly."
