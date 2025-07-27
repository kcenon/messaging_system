##################################################
# ThreadSystem Samples CMakeLists.txt
# 
# This script builds sample applications demonstrating the ThreadSystem library usage.
# Each sample focuses on a specific component or feature of the library.
##################################################

# Minimal thread pool sample without logger
# Demonstrates basic thread pool usage without logger dependency
add_subdirectory(minimal_thread_pool)

# Composition-based design sample
# Demonstrates modern dependency injection and composition patterns
add_subdirectory(composition_example)

# Basic logger usage sample
# Demonstrates configuration and usage of the logging system
# TEMPORARILY DISABLED: Logger is being moved to separate project
# add_subdirectory(logger_sample)

# Thread pool usage sample
# Shows how to create and use thread pools for parallel task execution
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(thread_pool_sample)


# Typed thread pool sample
# Demonstrates type-based task scheduling in thread pools
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(typed_thread_pool_sample)

# Advanced typed thread pool sample
# Shows more complex usage patterns of typed thread pools
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(typed_thread_pool_sample_2)

# MPMC queue sample
# Demonstrates lock-free MPMC queue usage patterns
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(mpmc_queue_sample)

# Adaptive queue sample
# Shows adaptive queue strategy selection and usage
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(adaptive_queue_sample)

# Typed job queue sample
# Demonstrates typed job queue usage patterns
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(typed_job_queue_sample)

# Hazard pointer sample
# Shows hazard pointer usage for thread-safe memory reclamation
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(hazard_pointer_sample)

# Node pool sample
# Demonstrates high-performance memory pool usage
# TEMPORARILY DISABLED: Uses logger directly, needs update to use interface
# add_subdirectory(node_pool_sample)

# Real-time monitoring sample
# Shows real-time performance monitoring and metrics collection
# TEMPORARILY DISABLED: Monitoring is being moved to separate project
# add_subdirectory(monitoring_sample)

# Integration example
# Demonstrates integration with external logger and monitoring systems
add_subdirectory(integration_example)

# Multi-process monitoring integration
# Demonstrates integration with multi-process monitoring features
add_subdirectory(multi_process_monitoring_integration)