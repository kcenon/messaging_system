project(logger_sample)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Explicitly add include directories for Windows compatibility
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/sources
)

# Link against logger library which provides proper include directories
# The logger library includes utilities as a dependency
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE logger utilities "-framework CoreFoundation")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE logger utilities)
endif()