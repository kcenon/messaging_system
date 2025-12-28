// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file priority.h
 * @brief Message and task priority levels
 *
 * Extracted from message.h to allow task to use priority without
 * depending on the full message class (Issue #192).
 */

#pragma once

#include <cstdint>

namespace kcenon::messaging {

/**
 * @enum message_priority
 * @brief Priority levels for messages and tasks
 *
 * Used by both message and task classes for prioritized queue processing.
 * Higher priority values indicate more urgent items.
 */
enum class message_priority : uint8_t {
	lowest = 0,
	low = 1,
	normal = 2,
	high = 3,
	highest = 4,
	critical = 5
};

}  // namespace kcenon::messaging
