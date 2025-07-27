# FindThreadSystemCore.cmake
# Find the ThreadSystemCore library
#
# This will define:
#   ThreadSystemCore_FOUND - System has ThreadSystemCore
#   ThreadSystemCore_INCLUDE_DIRS - The ThreadSystemCore include directories
#   ThreadSystemCore_LIBRARIES - The libraries needed to use ThreadSystemCore
#   ThreadSystem::Core - Imported target

# Allow the user to specify the root directory
if(NOT THREAD_SYSTEM_ROOT)
    set(THREAD_SYSTEM_ROOT "" CACHE PATH "Root directory of ThreadSystem installation")
endif()

# Find ThreadSystemCore headers
find_path(ThreadSystemCore_INCLUDE_DIR
    NAMES thread_pool/core/thread_pool.h
    PATHS
        ${THREAD_SYSTEM_ROOT}/sources
        ${CMAKE_PREFIX_PATH}/include/thread_system
        /usr/local/include/thread_system
        /usr/include/thread_system
)

# Find all required libraries
find_library(ThreadSystemCore_THREAD_POOL_LIBRARY
    NAMES thread_pool
    PATHS
        ${THREAD_SYSTEM_ROOT}/build/lib
        ${THREAD_SYSTEM_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

find_library(ThreadSystemCore_THREAD_BASE_LIBRARY
    NAMES thread_base
    PATHS
        ${THREAD_SYSTEM_ROOT}/build/lib
        ${THREAD_SYSTEM_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

find_library(ThreadSystemCore_UTILITIES_LIBRARY
    NAMES utilities
    PATHS
        ${THREAD_SYSTEM_ROOT}/build/lib
        ${THREAD_SYSTEM_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

find_library(ThreadSystemCore_TYPED_THREAD_POOL_LIBRARY
    NAMES typed_thread_pool
    PATHS
        ${THREAD_SYSTEM_ROOT}/build/lib
        ${THREAD_SYSTEM_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

find_library(ThreadSystemCore_INTERFACES_LIBRARY
    NAMES interfaces
    PATHS
        ${THREAD_SYSTEM_ROOT}/build/lib
        ${THREAD_SYSTEM_ROOT}/lib
        ${CMAKE_PREFIX_PATH}/lib
        /usr/local/lib
        /usr/lib
)

# Handle required components
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ThreadSystemCore 
    REQUIRED_VARS
        ThreadSystemCore_INCLUDE_DIR
        ThreadSystemCore_THREAD_POOL_LIBRARY
        ThreadSystemCore_THREAD_BASE_LIBRARY
        ThreadSystemCore_UTILITIES_LIBRARY
        ThreadSystemCore_INTERFACES_LIBRARY
)

if(ThreadSystemCore_FOUND)
    # Combine all libraries
    set(ThreadSystemCore_LIBRARIES 
        ${ThreadSystemCore_THREAD_POOL_LIBRARY}
        ${ThreadSystemCore_THREAD_BASE_LIBRARY}
        ${ThreadSystemCore_UTILITIES_LIBRARY}
        ${ThreadSystemCore_TYPED_THREAD_POOL_LIBRARY}
        ${ThreadSystemCore_INTERFACES_LIBRARY}
    )
    set(ThreadSystemCore_INCLUDE_DIRS ${ThreadSystemCore_INCLUDE_DIR})
    
    # Create imported target
    if(NOT TARGET ThreadSystem::Core)
        add_library(ThreadSystem::Core INTERFACE IMPORTED)
        set_target_properties(ThreadSystem::Core PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${ThreadSystemCore_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${ThreadSystemCore_LIBRARIES}"
            INTERFACE_COMPILE_FEATURES cxx_std_20
        )
        
        # Add thread dependency
        find_package(Threads REQUIRED)
        set_property(TARGET ThreadSystem::Core APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
        
        # Check if we need fmt
        include(CheckCXXSourceCompiles)
        set(CMAKE_REQUIRED_FLAGS "-std=c++20")
        check_cxx_source_compiles("
            #include <format>
            int main() { 
                std::string s = std::format(\"{}\", 42);
                return 0; 
            }" HAS_STD_FORMAT)
        
        if(HAS_STD_FORMAT)
            set_property(TARGET ThreadSystem::Core APPEND PROPERTY
                INTERFACE_COMPILE_DEFINITIONS USE_STD_FORMAT
            )
        else()
            find_package(fmt REQUIRED)
            set_property(TARGET ThreadSystem::Core APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES fmt::fmt
            )
        endif()
    endif()
endif()

mark_as_advanced(
    ThreadSystemCore_INCLUDE_DIR 
    ThreadSystemCore_THREAD_POOL_LIBRARY
    ThreadSystemCore_THREAD_BASE_LIBRARY
    ThreadSystemCore_UTILITIES_LIBRARY
    ThreadSystemCore_TYPED_THREAD_POOL_LIBRARY
    ThreadSystemCore_INTERFACES_LIBRARY
)