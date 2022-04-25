CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(LIBRARY_NAME network)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

PROJECT(${LIBRARY_NAME})

ADD_LIBRARY(${LIBRARY_NAME} STATIC
    session_types.h
    data_modes.h data_lengths.h
    data_handling.h data_handling.cpp 
    messaging_client.h messaging_client.cpp 
    messaging_session.h messaging_session.cpp
    messaging_server.h messaging_server.cpp)
    
FIND_PACKAGE(asio CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC asio asio::asio)
    
FIND_PACKAGE(cpprestsdk CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC cpprestsdk::cpprest 
    cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)

INCLUDE_DIRECTORIES(../utilities)

ADD_DEPENDENCIES(${LIBRARY_NAME} utilities)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC utilities)

IF(DEFINED __USE_TYPE_CONTAINER__)
    INCLUDE_DIRECTORIES(../container)

    ADD_DEPENDENCIES(${LIBRARY_NAME} container)
    TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC container)
ENDIF()

INCLUDE_DIRECTORIES(../threads)

ADD_DEPENDENCIES(${LIBRARY_NAME} threads)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC threads)