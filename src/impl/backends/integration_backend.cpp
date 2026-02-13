#include <kcenon/messaging/backends/integration_backend.h>
#include <kcenon/messaging/error/messaging_error_category.h>
#include <kcenon/common/logging/log_functions.h>

namespace kcenon::messaging {

integration_backend::integration_backend(
	std::shared_ptr<common::interfaces::IExecutor> executor,
	std::shared_ptr<common::interfaces::ILogger> logger,
	std::shared_ptr<common::interfaces::IMonitor> monitoring)
	: executor_(executor)
	, logger_(logger)
	, monitoring_(monitoring)
	, initialized_(false) {
}

common::VoidResult integration_backend::initialize() {
	if (initialized_.exchange(true)) {
		common::logging::log_warning("Integration backend already initialized");
		return common::Result<std::monostate>::err(
			make_typed_error_code(messaging_error_category::already_running));
	}

	common::logging::log_info("Initializing integration backend");

	if (!executor_) {
		initialized_.store(false);
		common::logging::log_error("Integration backend initialization failed: executor is null");
		return common::Result<std::monostate>::err(
			make_typed_error_code(messaging_error_category::backend_not_ready));
	}

	common::logging::log_info("Integration backend initialized successfully");
	return common::ok();
}

common::VoidResult integration_backend::shutdown() {
	if (!initialized_.exchange(false)) {
		common::logging::log_debug("Integration backend shutdown called but not initialized");
		return common::Result<std::monostate>::err(
			make_typed_error_code(messaging_error_category::not_running));
	}

	common::logging::log_info("Integration backend shutting down");

	// Note: We don't own these services, so we just reset our references
	// The caller is responsible for their lifecycle
	common::logging::log_info("Integration backend shutdown complete");
	return common::ok();
}

std::shared_ptr<common::interfaces::IExecutor> integration_backend::get_executor() {
	return executor_;
}

std::shared_ptr<common::interfaces::ILogger> integration_backend::get_logger() {
	return logger_;
}

std::shared_ptr<common::interfaces::IMonitor> integration_backend::get_monitoring() {
	return monitoring_;
}

bool integration_backend::is_ready() const {
	return initialized_.load() && executor_ != nullptr;
}

} // namespace kcenon::messaging
