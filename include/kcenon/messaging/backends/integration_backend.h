#pragma once

#include "backend_interface.h"

namespace kcenon::messaging {

/**
 * @class integration_backend
 * @brief Backend that uses external system services
 *
 * Integrates with thread_system, logger_system, and monitoring_system.
 * This backend does not own the services - it receives them from the caller
 * and uses them for messaging operations.
 */
class integration_backend : public backend_interface {
public:
	/**
	 * @brief Constructor
	 * @param executor Executor for async operations (required)
	 * @param logger Logger instance (optional)
	 * @param monitoring Monitoring instance (optional)
	 */
	integration_backend(
		std::shared_ptr<common::interfaces::IExecutor> executor,
		std::shared_ptr<common::interfaces::ILogger> logger = nullptr,
		std::shared_ptr<common::interfaces::IMonitor> monitoring = nullptr
	);

	/**
	 * @brief Destructor
	 */
	~integration_backend() override = default;

	// backend_interface implementation
	common::VoidResult initialize() override;
	common::VoidResult shutdown() override;
	std::shared_ptr<common::interfaces::IExecutor> get_executor() override;
	std::shared_ptr<common::interfaces::ILogger> get_logger() override;
	std::shared_ptr<common::interfaces::IMonitor> get_monitoring() override;
	bool is_ready() const override;

private:
	std::shared_ptr<common::interfaces::IExecutor> executor_;
	std::shared_ptr<common::interfaces::ILogger> logger_;
	std::shared_ptr<common::interfaces::IMonitor> monitoring_;
	std::atomic<bool> initialized_{false};
};

} // namespace kcenon::messaging
