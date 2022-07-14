CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(LIBRARY_NAME container)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(HEADER_FILES
    container.h value.h value_types.h 
    values/bool_value.h values/bytes_value.h
    values/container_value.h values/double_value.h 
    values/float_value.h values/int_value.h 
    values/llong_value.h values/long_value.h
    values/short_value.h values/string_value.h 
    values/uint_value.h values/ullong_value.h 
    values/ulong_value.h values/ushort_value.h)

SET(SOURCE_FILES
    container.cpp value.cpp value_types.cpp
    values/bool_value.cpp values/bytes_value.cpp
    values/container_value.cpp values/double_value.cpp
    values/float_value.cpp values/int_value.cpp
    values/llong_value.cpp values/long_value.cpp
    values/short_value.cpp values/string_value.cpp
    values/uint_value.cpp values/ullong_value.cpp
    values/ulong_value.cpp values/ushort_value.cpp)

IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG /MDd")
ELSE()
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

PROJECT(${LIBRARY_NAME} VERSION 1.0.0.0)

IF(USE_SWIG_INTERFACE)
    ADD_LIBRARY(${LIBRARY_NAME} ${HEADER_FILES} ${SOURCE_FILES} container_wrap.cxx)
ELSE()
    ADD_LIBRARY(${LIBRARY_NAME} ${HEADER_FILES} ${SOURCE_FILES})
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${LIBRARY_NAME} PUBLIC ../utilities)

ADD_DEPENDENCIES(${LIBRARY_NAME} utilities)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} PUBLIC utilities)