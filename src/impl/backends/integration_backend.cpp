#include <kcenon/messaging/backends/integration_backend.h>
#include <kcenon/messaging/error/error_codes.h>

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
		return common::make_error<std::monostate>(
			error::base,
			"Backend already initialized",
			"messaging_system"
		);
	}

	if (!executor_) {
		initialized_.store(false);
		return common::make_error<std::monostate>(
			error::base,
			"Executor is required for integration backend",
			"messaging_system"
		);
	}

	return common::ok();
}

common::VoidResult integration_backend::shutdown() {
	if (!initialized_.exchange(false)) {
		return common::make_error<std::monostate>(
			error::base,
			"Backend not initialized",
			"messaging_system"
		);
	}

	// Note: We don't own these services, so we just reset our references
	// The caller is responsible for their lifecycle
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
