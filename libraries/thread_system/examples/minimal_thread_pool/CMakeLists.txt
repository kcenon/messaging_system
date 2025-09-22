# Minimal thread pool sample without logger dependency
project(minimal_thread_pool)

add_executable(${PROJECT_NAME} minimal_thread_pool.cpp)

# Link with thread_base (which includes core functionality)
target_link_libraries(${PROJECT_NAME} PRIVATE thread_base)

# Include headers from the new structure
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Set as a sample application
set_target_properties(${PROJECT_NAME} PROPERTIES
    FOLDER "samples"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
