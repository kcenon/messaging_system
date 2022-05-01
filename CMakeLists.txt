CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(PROJECT_NAME messaging_system)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(USE_PYTHON ON)
SET(USE_TYPE_CONTAINER OFF)

IF(USE_PYTHON)
    ADD_DEFINITIONS(-D__USE_PYTHON__)
ENDIF()

IF(USE_TYPE_CONTAINER)
    ADD_DEFINITIONS(-D__USE_TYPE_CONTAINER__)
ENDIF()

# set the project name
PROJECT(${PROJECT_NAME} VERSION 1.0)

# cpp_libraries
ADD_SUBDIRECTORY(cpp_libraries/utilities)
IF(DEFINED USE_TYPE_CONTAINER)
    ADD_SUBDIRECTORY(cpp_libraries/container)
ENDIF()
ADD_SUBDIRECTORY(cpp_libraries/threads)
ADD_SUBDIRECTORY(cpp_libraries/database)
ADD_SUBDIRECTORY(cpp_libraries/network)

# cpp_samples
ADD_SUBDIRECTORY(cpp_samples/logging_sample)
IF(DEFINED USE_TYPE_CONTAINER)
    ADD_SUBDIRECTORY(cpp_samples/container_sample)
ENDIF()
ADD_SUBDIRECTORY(cpp_samples/threads_sample)
ADD_SUBDIRECTORY(cpp_samples/upload_sample)
ADD_SUBDIRECTORY(cpp_samples/download_sample)
ADD_SUBDIRECTORY(cpp_samples/restapi_client_sample)
ADD_SUBDIRECTORY(cpp_samples/echo_client)
ADD_SUBDIRECTORY(cpp_samples/echo_server)

# micro_services
ADD_SUBDIRECTORY(micro_services/file_managing/main_server)
ADD_SUBDIRECTORY(micro_services/file_managing/middle_server)
ADD_SUBDIRECTORY(micro_services/file_managing/restapi_gateway)