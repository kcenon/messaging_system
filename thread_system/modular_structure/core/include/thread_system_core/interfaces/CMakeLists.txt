# interfaces library
set(INTERFACE_SOURCES
    logger_interface.cpp
)

set(INTERFACE_HEADERS
    logger_interface.h
    error_handler.h
    monitoring_interface.h
)

add_library(interfaces STATIC ${INTERFACE_SOURCES})

target_include_directories(interfaces PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/sources>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/thread_system>
    $<INSTALL_INTERFACE:include/thread_system/interfaces>
)

# Set compile features
target_compile_features(interfaces PUBLIC cxx_std_20)

# Install headers
install(FILES ${INTERFACE_HEADERS}
    DESTINATION include/thread_system/interfaces
)