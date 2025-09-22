project(composition_example)

# Create the example executable
add_executable(${PROJECT_NAME} composition_example.cpp)

# Link against required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    thread_pool
    typed_thread_pool
    thread_base
    interfaces
    utilities
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Set C++ standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)