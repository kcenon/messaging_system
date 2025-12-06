// Simple compilation test for Phase 1 headers

#include "include/kcenon/messaging/error/error_codes.h"
#include "include/kcenon/messaging/utils/integration_detector.h"
// Cannot include interfaces yet as message class is not defined
// #include "include/kcenon/messaging/interfaces/message_handler_interface.h"
// #include "include/kcenon/messaging/interfaces/publisher_interface.h"
// #include "include/kcenon/messaging/interfaces/subscriber_interface.h"

#include <iostream>

int main() {
    // Test error codes
    std::cout << "Testing error codes:\n";
    std::cout << "  invalid_message = " << kcenon::messaging::error::invalid_message << "\n";
    std::cout << "  routing_failed = " << kcenon::messaging::error::routing_failed << "\n";
    std::cout << "  queue_full = " << kcenon::messaging::error::queue_full << "\n";
    std::cout << "  Message: " << kcenon::messaging::error::get_error_message(
        kcenon::messaging::error::invalid_message) << "\n";

    // Test integration detector
    std::cout << "\nTesting integration detector:\n";
    std::cout << "  has_common_system: "
              << kcenon::messaging::integration_detector::has_common_system() << "\n";
    std::cout << "  has_thread_system: "
              << kcenon::messaging::integration_detector::has_thread_system() << "\n";
    std::cout << "  has_monitoring_system: "
              << kcenon::messaging::integration_detector::has_monitoring_system() << "\n";
    std::cout << "  has_container_system: "
              << kcenon::messaging::integration_detector::has_container_system() << "\n";
    std::cout << "  has_any_integration: "
              << kcenon::messaging::integration_detector::has_any_integration() << "\n";

    std::cout << "\nPhase 1 headers test passed!\n";
    return 0;
}
