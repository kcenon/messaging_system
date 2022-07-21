CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(LIBRARY_NAME utilities)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)

SET(HEADER_FILES 
    logging_level.h logging.h folder_handler.h
    file_handler.h binary_combiner.h encrypting.h
    datetime_handler.h converting.h compressing.h
    argument_parser.h)

SET(SOURCE_FILES 
    logging.cpp folder_handler.cpp file_handler.cpp
    binary_combiner.cpp encrypting.cpp datetime_handler.cpp
    converting.cpp compressing.cpp argument_parser.cpp)

PROJECT(${LIBRARY_NAME} VERSION 1.0.0.0)

ADD_LIBRARY(${LIBRARY_NAME} ${HEADER_FILES} ${SOURCE_FILES})

FIND_PACKAGE(lz4 CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC lz4::lz4)

FIND_PACKAGE(fmt CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC fmt::fmt-header-only)

FIND_PACKAGE(cryptopp CONFIG REQUIRED)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC cryptopp-static)