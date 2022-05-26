CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(LIBRARY_NAME utilities)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG /MDd")

PROJECT(${LIBRARY_NAME})

ADD_LIBRARY(${LIBRARY_NAME} STATIC
    logging_level.h logging.h logging.cpp
    folder_handler.h folder_handler.cpp 
    file_handler.h file_handler.cpp
    encrypting.h encrypting.cpp
    datetime_handler.h datetime_handler.cpp
    converting.h converting.cpp
    compressing.h compressing.cpp
    argument_parser.h argument_parser.cpp)

FIND_PACKAGE(lz4 CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC lz4::lz4)

FIND_PACKAGE(fmt CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC fmt::fmt-header-only)

FIND_PACKAGE(cryptopp CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC cryptopp-static)