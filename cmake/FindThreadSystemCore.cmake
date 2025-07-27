# FindThreadSystemCore.cmake
# Find the ThreadSystemCore library
#
# This will define:
#   ThreadSystemCore_FOUND - System has ThreadSystemCore
#   ThreadSystemCore_INCLUDE_DIRS - The ThreadSystemCore include directories
#   ThreadSystemCore_LIBRARIES - The libraries needed to use ThreadSystemCore
#   ThreadSystem::Core - Imported target

find_path(ThreadSystemCore_INCLUDE_DIR
    NAMES thread_system_core/thread_pool/core/thread_pool.h thread_system/thread_pool/core/thread_pool.h
    PATHS
        ${ThreadSystemCore_ROOT}/include
        ${CMAKE_PREFIX_PATH}/include
        /usr/local/include
        /usr/include
)

find_library(ThreadSystemCore_LIBRARY
    NAMES thread_system_core
    PATHS
        ${ThreadSystemCore_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ThreadSystemCore DEFAULT_MSG
    ThreadSystemCore_LIBRARY ThreadSystemCore_INCLUDE_DIR
)

if(ThreadSystemCore_FOUND)
    set(ThreadSystemCore_LIBRARIES ${ThreadSystemCore_LIBRARY})
    set(ThreadSystemCore_INCLUDE_DIRS ${ThreadSystemCore_INCLUDE_DIR})
    
    # Create imported target
    if(NOT TARGET ThreadSystem::Core)
        add_library(ThreadSystem::Core STATIC IMPORTED)
        set_target_properties(ThreadSystem::Core PROPERTIES
            IMPORTED_LOCATION "${ThreadSystemCore_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ThreadSystemCore_INCLUDE_DIR}"
            INTERFACE_COMPILE_FEATURES cxx_std_20
        )
        
        # Add thread dependency
        find_package(Threads REQUIRED)
        set_property(TARGET ThreadSystem::Core APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
        
        # Check if we need to define USE_STD_FORMAT
        include(CheckCXXSourceCompiles)
        check_cxx_source_compiles("
            #include <format>
            int main() { 
                std::string s = std::format(\"{}\", 42);
                return 0; 
            }" HAS_STD_FORMAT)
        
        if(NOT HAS_STD_FORMAT)
            find_package(fmt QUIET)
            if(fmt_FOUND)
                set_property(TARGET ThreadSystem::Core APPEND PROPERTY
                    INTERFACE_LINK_LIBRARIES fmt::fmt
                )
            else()
                set_property(TARGET ThreadSystem::Core APPEND PROPERTY
                    INTERFACE_COMPILE_DEFINITIONS USE_STD_FORMAT
                )
            endif()
        else()
            set_property(TARGET ThreadSystem::Core APPEND PROPERTY
                INTERFACE_COMPILE_DEFINITIONS USE_STD_FORMAT
            )
        endif()
    endif()
endif()

mark_as_advanced(ThreadSystemCore_INCLUDE_DIR ThreadSystemCore_LIBRARY)