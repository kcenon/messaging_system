# Basic monitoring sample
add_executable(basic_monitoring basic_monitoring.cpp)
target_link_libraries(basic_monitoring PRIVATE monitoring)
set_target_properties(basic_monitoring PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Multi-process monitoring sample
add_executable(multi_process_example multi_process_example.cpp)
target_link_libraries(multi_process_example PRIVATE monitoring)
set_target_properties(multi_process_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Thread pool analysis sample
add_executable(thread_pool_analysis_example thread_pool_analysis_example.cpp)
target_link_libraries(thread_pool_analysis_example PRIVATE monitoring)
set_target_properties(thread_pool_analysis_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Advanced analysis sample
add_executable(advanced_analysis_example advanced_analysis_example.cpp)
target_link_libraries(advanced_analysis_example PRIVATE monitoring)
set_target_properties(advanced_analysis_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Optimization demo sample
add_executable(optimization_demo optimization_demo.cpp)
target_link_libraries(optimization_demo PRIVATE monitoring)
set_target_properties(optimization_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)