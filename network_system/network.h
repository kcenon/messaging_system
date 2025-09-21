/**
 * @file network.h
 * @brief Main include file for the network module
 * 
 * This file includes all the public headers from the network module.
 * Users should include this file to use the network functionality.
 */

#pragma once

// Core components
#include "network/core/messaging_client.h"
#include "network/core/messaging_server.h"

// Public session interfaces
#include "network/session/messaging_session.h"

// Note: Internal components like tcp_socket, send_coroutine, pipeline, and common_defs
// are not exposed in the public API