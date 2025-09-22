project(service_registry_sample)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    thread_base
    interfaces
    utilities
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/base/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/jobs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/sync/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../implementations/thread_pool/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

