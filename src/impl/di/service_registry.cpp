#include "service_registry.h"

namespace kcenon::messaging::di {

service_registry& get_global_registry() {
    static service_registry instance;
    return instance;
}

} // namespace kcenon::messaging::di
