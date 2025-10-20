#pragma once

namespace messaging::error {

// Messaging system error codes: -200 to -299
constexpr int INVALID_MESSAGE = -200;
constexpr int ROUTING_FAILED = -201;
constexpr int SERIALIZATION_ERROR = -202;
constexpr int NETWORK_ERROR = -203;
constexpr int DATABASE_ERROR = -204;
constexpr int QUEUE_FULL = -205;
constexpr int TIMEOUT = -206;
constexpr int AUTHENTICATION_FAILED = -207;
constexpr int AUTHORIZATION_FAILED = -208;
constexpr int SUBSCRIPTION_FAILED = -209;
constexpr int PUBLICATION_FAILED = -210;
constexpr int UNKNOWN_TOPIC = -211;
constexpr int NO_SUBSCRIBERS = -212;

} // namespace messaging::error
