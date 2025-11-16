#pragma once

#include <kcenon/common/patterns/result.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/interfaces/logger_interface.h>
#include <kcenon/common/interfaces/monitoring_interface.h>
#include <memory>

namespace kcenon::messaging {

/**
 * @interface backend_interface
 * @brief Abstract backend for messaging system execution
 *
 * Pattern borrowed from logger_system for consistent backend abstraction.
 * Provides core services needed by the messaging system:
 * - Async execution (executor)
 * - Optional logging integration
 * - Optional monitoring integration
 */
class backend_interface {
public:
	virtual ~backend_interface() = default;

	/**
	 * @brief Initialize backend
	 * @return Result indicating success or error
	 */
	virtual common::VoidResult initialize() = 0;

	/**
	 * @brief Shutdown backend
	 * @return Result indicating success or error
	 */
	virtual common::VoidResult shutdown() = 0;

	/**
	 * @brief Get executor for async operations
	 * @return Shared pointer to executor interface
	 */
	virtual std::shared_ptr<common::interfaces::IExecutor> get_executor() = 0;

	/**
	 * @brief Get logger instance (optional)
	 * @return Shared pointer to logger interface or nullptr if not available
	 */
	virtual std::shared_ptr<common::interfaces::ILogger> get_logger() {
		return nullptr;
	}

	/**
	 * @brief Get monitoring instance (optional)
	 * @return Shared pointer to monitoring interface or nullptr if not available
	 */
	virtual std::shared_ptr<common::interfaces::IMonitor> get_monitoring() {
		return nullptr;
	}

	/**
	 * @brief Check if backend is ready
	 * @return true if backend is initialized and ready for use
	 */
	virtual bool is_ready() const = 0;
};

} // namespace kcenon::messaging
