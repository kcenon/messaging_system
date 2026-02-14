// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file container_aliases.h
 * @brief Centralized type and namespace aliases for container_system migration
 *
 * This header provides a single point of change for the upcoming
 * container_system namespace migration:
 *   container_module  ->  kcenon::container   (namespace)
 *   value_container   ->  message_buffer      (class rename)
 *
 * Migration guide reference: container_system docs/guides/NAMESPACE_MIGRATION.md
 *
 * Usage:
 * @code
 * #include <kcenon/messaging/compat/container_aliases.h>
 *
 * using kcenon::messaging::compat::container_data;
 * using kcenon::messaging::compat::container_data_ptr;
 *
 * container_data payload;
 * container_data_ptr shared_payload = std::make_shared<container_data>();
 * @endcode
 *
 * When container_system v2.0 enforces the new namespace, update only this
 * file and all downstream code will pick up the change automatically.
 *
 * @see https://github.com/kcenon/messaging_system/issues/224
 */

#pragma once

#include <core/container.h>

#include <memory>

namespace kcenon::messaging::compat {

// ---------------------------------------------------------------------------
// Namespace alias
// ---------------------------------------------------------------------------
// Current:  container_module
// Future:   kcenon::container  (when container_system v2.0 is enforced)
namespace container_ns = container_module;

// ---------------------------------------------------------------------------
// Primary type aliases
// ---------------------------------------------------------------------------
// Current:  container_module::value_container
// Future:   kcenon::container::message_buffer
using container_data = container_ns::value_container;

// Shared pointer convenience alias
using container_data_ptr = std::shared_ptr<container_data>;

// ---------------------------------------------------------------------------
// Value type aliases
// ---------------------------------------------------------------------------
// Current:  container_module::value_types
// Future:   kcenon::container::value_types
using container_value_types = container_ns::value_types;

}  // namespace kcenon::messaging::compat
