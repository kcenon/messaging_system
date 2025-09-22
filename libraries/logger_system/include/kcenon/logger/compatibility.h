#ifndef KCENON_LOGGER_COMPATIBILITY_H
#define KCENON_LOGGER_COMPATIBILITY_H

// Backward compatibility namespace aliases
// These will be removed in future versions

namespace kcenon::logger {
    namespace core {}
    namespace interfaces {}
    namespace impl {}
    namespace utils {}
    namespace writers {}
    namespace formatters {}
}

// Old namespace aliases (deprecated)
// Note: Direct aliasing removed - use kcenon::logger directly
namespace logger_interfaces = kcenon::logger::interfaces;
namespace logger_core = kcenon::logger::core;

#endif // KCENON_LOGGER_COMPATIBILITY_H
