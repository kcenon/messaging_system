##################################################
# Utilities Component (new layout)
##################################################

project(utilities)

file(GLOB_RECURSE UTIL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
# Source files are now in the main src/utils directory
file(GLOB_RECURSE UTIL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils/*.cpp)

# Only create library if we have sources
if(UTIL_SOURCES)
    add_library(${PROJECT_NAME} STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
else()
    # Create interface library if no sources
    add_library(${PROJECT_NAME} INTERFACE)
endif()

if(UTIL_SOURCES)
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/thread_system/utilities>
    )

    if(COMMAND set_compiler_warnings)
        set_compiler_warnings(${PROJECT_NAME})
    endif()

    # Fallback to fmt when std::format is not used
    if(NOT USE_STD_FORMAT)
        find_package(fmt CONFIG QUIET)
        if(fmt_FOUND)
            target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt-header-only)
            message(STATUS "✅ Found fmt library - using fmt::format")
        else()
            message(STATUS "⚠️ fmt library not found - using basic string operations fallback")
        endif()
    endif()

    # Iconv for string conversions
    find_package(Iconv REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Iconv::Iconv)
else()
    target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/thread_system/utilities>
    )

    # Fallback to fmt when std::format is not used
    if(NOT USE_STD_FORMAT)
        find_package(fmt CONFIG QUIET)
        if(fmt_FOUND)
            target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt-header-only)
            message(STATUS "✅ Found fmt library - using fmt::format (interface)")
        else()
            message(STATUS "⚠️ fmt library not found - using basic string operations fallback (interface)")
        endif()
    endif()

    # Iconv for string conversions
    find_package(Iconv REQUIRED)
    target_link_libraries(${PROJECT_NAME} INTERFACE Iconv::Iconv)
endif()

