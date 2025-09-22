project(node_pool_sample)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Explicitly add include directories for Windows compatibility
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/sources
)

# Link required libraries
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE thread_base utilities logger "-framework CoreFoundation")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE thread_base utilities logger)
endif()