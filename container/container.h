/**
 * @file container.h
 * @brief Main include file for the container module
 * 
 * This file includes all the public headers from the container module.
 * Users should include this file to use the container functionality.
 */

#pragma once

// Core components
#include "container/core/container.h"
#include "container/core/value.h"
#include "container/core/value_types.h"

// Note: Internal components like variant_value, thread_safe_container, and simd_processor
// are not exposed in the public API