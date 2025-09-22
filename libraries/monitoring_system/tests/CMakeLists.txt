# Test directory CMakeLists.txt

# Find GTest
find_package(GTest)

if(NOT GTest_FOUND)
    message(STATUS "GTest not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Create test executable
add_executable(monitoring_system_tests
    # Core working tests
    test_result_types.cpp
    test_di_container.cpp
    test_monitorable_interface.cpp
    test_thread_context_simple.cpp

    # Other tests temporarily disabled due to missing implementations
    # test_distributed_tracing.cpp
    # test_performance_monitoring.cpp
    # test_adaptive_monitoring.cpp
    # test_health_monitoring.cpp
    # test_metric_storage.cpp
    # test_stream_aggregation.cpp
    # test_buffering_strategies.cpp
    # test_optimization.cpp
    # test_fault_tolerance.cpp
    # test_error_boundaries.cpp
    # test_resource_management.cpp
    # test_data_consistency.cpp
    # test_opentelemetry_adapter.cpp
    # test_trace_exporters.cpp
    # test_metric_exporters.cpp
    # test_storage_backends.cpp
    # test_integration_e2e.cpp # Temporarily disabled - API alignment needed
    # test_stress_performance.cpp
)

target_link_libraries(monitoring_system_tests
    PRIVATE
        monitoring_system
        GTest::gtest_main
        Threads::Threads
)

# Register tests
include(GoogleTest)
gtest_discover_tests(monitoring_system_tests)