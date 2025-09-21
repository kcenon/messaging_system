#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>

#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/message_types.h>
#include <kcenon/messaging/core/config.h>

namespace py = pybind11;
using namespace kcenon::messaging;

// Helper function to convert Python dict to message_payload
core::message_payload dict_to_payload(const std::string& topic, const py::dict& data) {
    core::message_payload payload;
    payload.topic = topic;

    for (auto item : data) {
        std::string key = py::str(item.first);
        py::handle value_handle = item.second;

        if (py::isinstance<py::str>(value_handle)) {
            payload.data[key] = py::cast<std::string>(value_handle);
        } else if (py::isinstance<py::int_>(value_handle)) {
            payload.data[key] = py::cast<int64_t>(value_handle);
        } else if (py::isinstance<py::float_>(value_handle)) {
            payload.data[key] = py::cast<double>(value_handle);
        } else if (py::isinstance<py::bool_>(value_handle)) {
            payload.data[key] = py::cast<bool>(value_handle);
        } else if (py::isinstance<py::bytes>(value_handle)) {
            std::string bytes_str = py::cast<std::string>(value_handle);
            std::vector<uint8_t> bytes_vec(bytes_str.begin(), bytes_str.end());
            payload.data[key] = bytes_vec;
        }
    }

    return payload;
}

// Helper function to convert message_payload to Python dict
py::dict payload_to_dict(const core::message_payload& payload) {
    py::dict result;
    result["topic"] = payload.topic;

    py::dict data_dict;
    for (const auto& [key, value] : payload.data) {
        if (std::holds_alternative<std::string>(value)) {
            data_dict[key.c_str()] = std::get<std::string>(value);
        } else if (std::holds_alternative<int64_t>(value)) {
            data_dict[key.c_str()] = std::get<int64_t>(value);
        } else if (std::holds_alternative<double>(value)) {
            data_dict[key.c_str()] = std::get<double>(value);
        } else if (std::holds_alternative<bool>(value)) {
            data_dict[key.c_str()] = std::get<bool>(value);
        } else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
            const auto& bytes_vec = std::get<std::vector<uint8_t>>(value);
            std::string bytes_str(bytes_vec.begin(), bytes_vec.end());
            data_dict[key.c_str()] = py::bytes(bytes_str);
        }
    }
    result["data"] = data_dict;

    if (!payload.binary_data.empty()) {
        std::string binary_str(payload.binary_data.begin(), payload.binary_data.end());
        result["binary_data"] = py::bytes(binary_str);
    }

    return result;
}

// Python wrapper for message handler
class PyMessageHandler {
public:
    PyMessageHandler(py::function func) : py_func_(func) {}

    void operator()(const core::message& msg) {
        try {
            py::dict msg_dict = payload_to_dict(msg.payload);
            msg_dict["sender"] = msg.metadata.sender;
            msg_dict["timestamp"] = msg.metadata.timestamp;
            msg_dict["priority"] = static_cast<int>(msg.metadata.priority);

            py_func_(msg_dict);
        } catch (const py::error_already_set& e) {
            // Log Python exception
            py::print("Python handler error:", e.what());
        }
    }

private:
    py::function py_func_;
};

PYBIND11_MODULE(messaging_cpp, m) {
    m.doc() = "KCENON Messaging System Python Bindings";

    // Message Priority enum
    py::enum_<core::message_priority>(m, "MessagePriority")
        .value("LOW", core::message_priority::low)
        .value("NORMAL", core::message_priority::normal)
        .value("HIGH", core::message_priority::high)
        .value("CRITICAL", core::message_priority::critical);

    // Configuration classes
    py::class_<config::messaging_config>(m, "MessagingConfig")
        .def(py::init<>())
        .def_readwrite("system_name", &config::messaging_config::system_name)
        .def_readwrite("environment", &config::messaging_config::environment)
        .def_readwrite("version", &config::messaging_config::version);

    // Configuration Builder
    py::class_<config::config_builder>(m, "ConfigBuilder")
        .def(py::init<>())
        .def("set_worker_threads", &config::config_builder::set_worker_threads,
             "Set number of worker threads")
        .def("set_queue_size", &config::config_builder::set_queue_size,
             "Set maximum queue size")
        .def("enable_priority_queue", &config::config_builder::enable_priority_queue,
             "Enable/disable priority queue", py::arg("enable") = true)
        .def("enable_compression", &config::config_builder::enable_compression,
             "Enable/disable compression", py::arg("enable") = true)
        .def("set_environment", &config::config_builder::set_environment,
             "Set environment (development, staging, production)")
        .def("set_system_name", &config::config_builder::set_system_name,
             "Set system name")
        .def("enable_external_logger", &config::config_builder::enable_external_logger,
             "Enable external logger system", py::arg("enable") = true)
        .def("enable_external_monitoring", &config::config_builder::enable_external_monitoring,
             "Enable external monitoring system", py::arg("enable") = true)
        .def("build", &config::config_builder::build,
             "Build the configuration object");

    // System Health
    py::class_<integrations::system_integrator::system_health>(m, "SystemHealth")
        .def_readonly("message_bus_healthy", &integrations::system_integrator::system_health::message_bus_healthy)
        .def_readonly("all_services_healthy", &integrations::system_integrator::system_health::all_services_healthy)
        .def_readonly("active_services", &integrations::system_integrator::system_health::active_services)
        .def_readonly("total_messages_processed", &integrations::system_integrator::system_health::total_messages_processed)
        .def_readonly("last_check", &integrations::system_integrator::system_health::last_check);

    // System Integrator (main messaging system interface)
    py::class_<integrations::system_integrator>(m, "MessagingSystem")
        .def(py::init<const config::messaging_config&>(),
             "Create messaging system with configuration")
        .def("initialize", &integrations::system_integrator::initialize,
             "Initialize the messaging system")
        .def("shutdown", &integrations::system_integrator::shutdown,
             "Shutdown the messaging system")
        .def("is_running", &integrations::system_integrator::is_running,
             "Check if the system is running")
        .def("publish", [](integrations::system_integrator& self, const std::string& topic,
                          const py::dict& data, const std::string& sender = "") {
            auto payload = dict_to_payload(topic, data);
            return self.publish(topic, payload, sender);
        }, "Publish a message", py::arg("topic"), py::arg("data"), py::arg("sender") = "")
        .def("subscribe", [](integrations::system_integrator& self, const std::string& topic,
                            py::function handler) {
            auto cpp_handler = PyMessageHandler(handler);
            self.subscribe(topic, cpp_handler);
        }, "Subscribe to a topic with message handler")
        .def("check_system_health", &integrations::system_integrator::check_system_health,
             "Get system health information")
        .def_static("create_default", &integrations::system_integrator::create_default,
                   "Create default messaging system instance")
        .def_static("create_for_environment", &integrations::system_integrator::create_for_environment,
                   "Create messaging system for specific environment");

    // Utility functions
    m.def("create_default_system", []() {
        return integrations::system_integrator::create_default();
    }, "Create a default messaging system");

    m.def("create_system_for_environment", [](const std::string& environment) {
        return integrations::system_integrator::create_for_environment(environment);
    }, "Create messaging system for specific environment");

    // Message creation helpers
    m.def("create_message", [](const std::string& topic, const py::dict& data) {
        return payload_to_dict(dict_to_payload(topic, data));
    }, "Create a message from topic and data");

    // Version information
    m.attr("__version__") = "2.0.0";
}