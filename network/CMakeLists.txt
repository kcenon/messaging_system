project(network)

file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})
    
find_package(asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC asio asio::asio)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC container priority_thread_pool)