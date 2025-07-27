##################################################
# ThreadSystem Sources CMakeLists.txt
# 
# This build script organizes the compilation of core components of the ThreadSystem library.
# Components are built in order of dependency, ensuring proper build sequence.
##################################################

# Basic utility components (no thread-specific dependencies)
add_subdirectory(utilities)

# Interface definitions for dependency injection
add_subdirectory(interfaces)

# Core thread foundation classes 
add_subdirectory(thread_base)

# Standard thread pool implementation (depends on thread_base)
add_subdirectory(thread_pool)

# Type-based thread pool implementation (depends on thread_pool)
add_subdirectory(typed_thread_pool)

# Monitoring system (depends on thread_base and utilities)
# TEMPORARILY DISABLED: Monitoring is being moved to separate project
# add_subdirectory(monitoring)