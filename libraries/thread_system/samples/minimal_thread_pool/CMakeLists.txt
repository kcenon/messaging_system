# Minimal thread pool sample without logger dependency
project(minimal_thread_pool)

add_executable(${PROJECT_NAME} minimal_thread_pool.cpp)

# Link only with thread_pool (which includes thread_base and interfaces)
target_link_libraries(${PROJECT_NAME} PRIVATE thread_pool)

# Set as a sample application
set_target_properties(${PROJECT_NAME} PROPERTIES
    FOLDER "samples"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)