#include "messaging_di_container.h"

namespace kcenon::messaging::di {

messaging_di_container& get_global_container() {
    static messaging_di_container instance;
    return instance;
}

} // namespace kcenon::messaging::di
