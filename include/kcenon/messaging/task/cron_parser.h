// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file cron_parser.h
 * @brief Cron expression parser for scheduled task execution
 *
 * Provides parsing and next execution time calculation for cron expressions.
 * Supports standard 5-field cron format: minute hour day month weekday
 */

#pragma once

#include <kcenon/common/patterns/result.h>

#include <chrono>
#include <set>
#include <string>

namespace kcenon::messaging::task {

/**
 * @struct cron_expression
 * @brief Parsed cron expression fields
 *
 * Each field contains a set of valid values for that time component.
 * Empty set means all values are valid (equivalent to *).
 */
struct cron_expression {
	std::set<int> minutes;   // 0-59
	std::set<int> hours;     // 0-23
	std::set<int> days;      // 1-31
	std::set<int> months;    // 1-12
	std::set<int> weekdays;  // 0-6 (0 = Sunday)

	bool operator==(const cron_expression& other) const {
		return minutes == other.minutes && hours == other.hours && days == other.days
			   && months == other.months && weekdays == other.weekdays;
	}
};

/**
 * @class cron_parser
 * @brief Static utility class for parsing cron expressions
 *
 * Cron Expression Format:
 * ```
 * * * * * *
 * │ │ │ │ │
 * │ │ │ │ └─ Day of week (0-6, 0=Sunday)
 * │ │ │ └─── Month (1-12)
 * │ │ └───── Day of month (1-31)
 * │ └─────── Hour (0-23)
 * └───────── Minute (0-59)
 * ```
 *
 * Supported Syntax:
 * - `*`     : All values
 * - `5`     : Specific value
 * - `* /15` : Interval (every 15 units, no space in actual use)
 * - `1-5`   : Range (1 through 5)
 * - `1,3,5` : List
 *
 * @example
 * ```cpp
 * // Every 2 hours on weekdays
 * auto expr = cron_parser::parse("0 * /2 * * 1-5").value();
 * auto next = cron_parser::next_run_time(expr).value();
 * ```
 */
class cron_parser {
public:
	cron_parser() = delete;

	/**
	 * @brief Parse a cron expression string
	 * @param expr Cron expression (5 fields separated by spaces)
	 * @return Result containing parsed cron_expression or error
	 */
	static common::Result<cron_expression> parse(const std::string& expr);

	/**
	 * @brief Calculate the next run time for a cron expression
	 * @param expr Parsed cron expression
	 * @param from Starting time point (defaults to now)
	 * @return Result containing the next execution time or error
	 */
	static common::Result<std::chrono::system_clock::time_point> next_run_time(
		const cron_expression& expr,
		std::chrono::system_clock::time_point from = std::chrono::system_clock::now());

	/**
	 * @brief Validate a cron expression string without parsing
	 * @param expr Cron expression string
	 * @return true if the expression is valid
	 */
	static bool is_valid(const std::string& expr);

	/**
	 * @brief Convert a cron expression back to string representation
	 * @param expr Parsed cron expression
	 * @return String representation of the expression
	 */
	static std::string to_string(const cron_expression& expr);

private:
	// Parse a single cron field (e.g., star-slash-15, 1-5, 1,3,5)
	static common::Result<std::set<int>> parse_field(const std::string& field, int min, int max);

	/**
	 * @brief Parse a range expression (e.g., "1-5")
	 * @param range Range string
	 * @param min Minimum allowed value
	 * @param max Maximum allowed value
	 * @return Result containing set of values in range or error
	 */
	static common::Result<std::set<int>> parse_range(const std::string& range, int min, int max);

	/**
	 * @brief Check if a given time matches the cron expression
	 * @param expr Parsed cron expression
	 * @param time Time point to check
	 * @return true if the time matches the expression
	 */
	static bool matches(const cron_expression& expr,
						std::chrono::system_clock::time_point time);

	/**
	 * @brief Get the day of week for a time point
	 * @param time Time point
	 * @return Day of week (0 = Sunday, 6 = Saturday)
	 */
	static int get_weekday(std::chrono::system_clock::time_point time);

	/**
	 * @brief Get the number of days in a month
	 * @param year Year
	 * @param month Month (1-12)
	 * @return Number of days in the month
	 */
	static int days_in_month(int year, int month);

	/**
	 * @brief Check if a year is a leap year
	 * @param year Year to check
	 * @return true if the year is a leap year
	 */
	static bool is_leap_year(int year);
};

}  // namespace kcenon::messaging::task
