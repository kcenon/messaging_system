project(network)

file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})
    
# Find required packages
find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link with necessary libraries
target_link_libraries(${PROJECT_NAME} PUBLIC 
    asio::asio
    container 
    thread_pool
    priority_thread_pool
    fmt::fmt
)