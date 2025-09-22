/**
 * @file network/network.h
 * @brief Network module public API
 * 
 * This header provides the main entry point for the network module.
 * It includes only the public-facing components that users need.
 */

#pragma once

// Core networking components
#include "core/messaging_client.h"
#include "core/messaging_server.h"

// Session management
#include "session/messaging_session.h"

/**
 * @namespace network_module
 * @brief Main namespace for all network-related functionality
 * 
 * The network module provides:
 * - TCP/IP client-server communication
 * - Asynchronous I/O with ASIO
 * - Message serialization/deserialization
 * - Thread-safe session management
 */