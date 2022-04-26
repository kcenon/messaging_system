CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(PROJECT_NAME messaging_system)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
PROJECT(${PROJECT_NAME} VERSION 1.0)

INCLUDE(../vcpkg/scripts/buildsystems/vcpkg.cmake)

# cpp_libraries
ADD_SUBDIRECTORY(cpp_libraries/utilities)
IF(DEFINED __USE_TYPE_CONTAINER__)
    ADD_SUBDIRECTORY(cpp_libraries/container)
ENDIF()
ADD_SUBDIRECTORY(cpp_libraries/threads)
ADD_SUBDIRECTORY(cpp_libraries/network)

# cpp_samples
ADD_SUBDIRECTORY(cpp_samples/logging_sample)
IF(DEFINED __USE_TYPE_CONTAINER__)
    ADD_SUBDIRECTORY(cpp_samples/container_sample)
ENDIF()
ADD_SUBDIRECTORY(cpp_samples/threads_sample)
ADD_SUBDIRECTORY(cpp_samples/upload_sample)
ADD_SUBDIRECTORY(cpp_samples/download_sample)
ADD_SUBDIRECTORY(cpp_samples/restapi_client_sample)

# micro_services
ADD_SUBDIRECTORY(micro_services/file_managing/main_server)
ADD_SUBDIRECTORY(micro_services/file_managing/middle_server)
ADD_SUBDIRECTORY(micro_services/file_managing/restapi_gateway)