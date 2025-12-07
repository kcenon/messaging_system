// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include "kcenon/messaging/task/cron_parser.h"

#include <kcenon/messaging/error/error_codes.h>

#include <algorithm>
#include <ctime>
#include <sstream>
#include <vector>

namespace kcenon::messaging::task {

namespace {

// Helper function to split a string by delimiter
std::vector<std::string> split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, delimiter)) {
		if (!token.empty()) {
			tokens.push_back(token);
		}
	}
	return tokens;
}

// Helper function to trim whitespace from a string
std::string trim(const std::string& str) {
	const char* whitespace = " \t\n\r";
	size_t start = str.find_first_not_of(whitespace);
	if (start == std::string::npos) {
		return "";
	}
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}

// Helper function to parse an integer with validation
bool parse_int(const std::string& str, int& out) {
	if (str.empty()) {
		return false;
	}
	try {
		size_t pos;
		out = std::stoi(str, &pos);
		return pos == str.length();
	} catch (...) {
		return false;
	}
}

// Generate all values in a range with optional step
std::set<int> generate_range(int start, int end, int step = 1) {
	std::set<int> result;
	for (int i = start; i <= end; i += step) {
		result.insert(i);
	}
	return result;
}

}  // anonymous namespace

// ============================================================================
// Field parsing
// ============================================================================

common::Result<std::set<int>> cron_parser::parse_field(const std::string& field,
													   int min,
													   int max) {
	std::string trimmed = trim(field);
	if (trimmed.empty()) {
		return common::Result<std::set<int>>(
			common::error_info{error::task_invalid_argument, "Empty cron field"});
	}

	std::set<int> result;

	// Handle wildcard (*)
	if (trimmed == "*") {
		return common::Result<std::set<int>>(generate_range(min, max));
	}

	// Handle step value (*/n or range/n)
	size_t step_pos = trimmed.find('/');
	if (step_pos != std::string::npos) {
		std::string base = trimmed.substr(0, step_pos);
		std::string step_str = trimmed.substr(step_pos + 1);

		int step;
		if (!parse_int(step_str, step) || step <= 0) {
			return common::Result<std::set<int>>(common::error_info{
				error::task_invalid_argument,
				"Invalid step value in cron field: " + trimmed});
		}

		if (base == "*") {
			// */n means every n starting from min
			return common::Result<std::set<int>>(generate_range(min, max, step));
		}

		// Handle range/step (e.g., 1-10/2)
		auto range_result = parse_range(base, min, max);
		if (!range_result.is_ok()) {
			return range_result;
		}

		auto range_values = range_result.unwrap();
		int range_min = *range_values.begin();
		int range_max = *range_values.rbegin();
		return common::Result<std::set<int>>(generate_range(range_min, range_max, step));
	}

	// Handle list (comma-separated values)
	if (trimmed.find(',') != std::string::npos) {
		auto parts = split(trimmed, ',');
		for (const auto& part : parts) {
			auto part_result = parse_field(part, min, max);
			if (!part_result.is_ok()) {
				return part_result;
			}
			auto part_values = part_result.unwrap();
			result.insert(part_values.begin(), part_values.end());
		}
		return common::Result<std::set<int>>(result);
	}

	// Handle range (a-b)
	if (trimmed.find('-') != std::string::npos) {
		return parse_range(trimmed, min, max);
	}

	// Handle single value
	int value;
	if (!parse_int(trimmed, value)) {
		return common::Result<std::set<int>>(common::error_info{
			error::task_invalid_argument, "Invalid cron field value: " + trimmed});
	}

	if (value < min || value > max) {
		return common::Result<std::set<int>>(common::error_info{
			error::task_invalid_argument,
			"Cron field value out of range [" + std::to_string(min) + "-"
				+ std::to_string(max) + "]: " + std::to_string(value)});
	}

	result.insert(value);
	return common::Result<std::set<int>>(result);
}

common::Result<std::set<int>> cron_parser::parse_range(const std::string& range,
													   int min,
													   int max) {
	auto parts = split(range, '-');
	if (parts.size() != 2) {
		return common::Result<std::set<int>>(
			common::error_info{error::task_invalid_argument, "Invalid range: " + range});
	}

	int start, end;
	if (!parse_int(parts[0], start) || !parse_int(parts[1], end)) {
		return common::Result<std::set<int>>(common::error_info{
			error::task_invalid_argument, "Invalid range values: " + range});
	}

	if (start < min || end > max || start > end) {
		return common::Result<std::set<int>>(common::error_info{
			error::task_invalid_argument,
			"Range out of bounds [" + std::to_string(min) + "-" + std::to_string(max)
				+ "]: " + range});
	}

	return common::Result<std::set<int>>(generate_range(start, end));
}

// ============================================================================
// Main parsing
// ============================================================================

common::Result<cron_expression> cron_parser::parse(const std::string& expr) {
	std::string trimmed = trim(expr);
	if (trimmed.empty()) {
		return common::Result<cron_expression>(
			common::error_info{error::task_invalid_argument, "Empty cron expression"});
	}

	// Split by whitespace
	std::vector<std::string> fields;
	std::stringstream ss(trimmed);
	std::string token;
	while (ss >> token) {
		fields.push_back(token);
	}

	if (fields.size() != 5) {
		return common::Result<cron_expression>(common::error_info{
			error::task_invalid_argument,
			"Cron expression must have exactly 5 fields, got " + std::to_string(fields.size())});
	}

	cron_expression result;

	// Parse minutes (0-59)
	auto minutes_result = parse_field(fields[0], 0, 59);
	if (!minutes_result.is_ok()) {
		return common::Result<cron_expression>(
			common::error_info{error::task_invalid_argument,
							   "Invalid minutes field: " + minutes_result.error().message});
	}
	result.minutes = minutes_result.unwrap();

	// Parse hours (0-23)
	auto hours_result = parse_field(fields[1], 0, 23);
	if (!hours_result.is_ok()) {
		return common::Result<cron_expression>(
			common::error_info{error::task_invalid_argument,
							   "Invalid hours field: " + hours_result.error().message});
	}
	result.hours = hours_result.unwrap();

	// Parse days (1-31)
	auto days_result = parse_field(fields[2], 1, 31);
	if (!days_result.is_ok()) {
		return common::Result<cron_expression>(
			common::error_info{error::task_invalid_argument,
							   "Invalid days field: " + days_result.error().message});
	}
	result.days = days_result.unwrap();

	// Parse months (1-12)
	auto months_result = parse_field(fields[3], 1, 12);
	if (!months_result.is_ok()) {
		return common::Result<cron_expression>(
			common::error_info{error::task_invalid_argument,
							   "Invalid months field: " + months_result.error().message});
	}
	result.months = months_result.unwrap();

	// Parse weekdays (0-6)
	auto weekdays_result = parse_field(fields[4], 0, 6);
	if (!weekdays_result.is_ok()) {
		return common::Result<cron_expression>(common::error_info{
			error::task_invalid_argument,
			"Invalid weekdays field: " + weekdays_result.error().message});
	}
	result.weekdays = weekdays_result.unwrap();

	return common::Result<cron_expression>(result);
}

// ============================================================================
// Time calculation helpers
// ============================================================================

bool cron_parser::is_leap_year(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int cron_parser::days_in_month(int year, int month) {
	static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (month == 2 && is_leap_year(year)) {
		return 29;
	}
	return days[month];
}

int cron_parser::get_weekday(std::chrono::system_clock::time_point time) {
	std::time_t tt = std::chrono::system_clock::to_time_t(time);
	std::tm tm_time;
#ifdef _WIN32
	localtime_s(&tm_time, &tt);
#else
	localtime_r(&tt, &tm_time);
#endif
	return tm_time.tm_wday;  // 0 = Sunday
}

bool cron_parser::matches(const cron_expression& expr,
						  std::chrono::system_clock::time_point time) {
	std::time_t tt = std::chrono::system_clock::to_time_t(time);
	std::tm tm_time;
#ifdef _WIN32
	localtime_s(&tm_time, &tt);
#else
	localtime_r(&tt, &tm_time);
#endif

	// Check minute
	if (expr.minutes.find(tm_time.tm_min) == expr.minutes.end()) {
		return false;
	}

	// Check hour
	if (expr.hours.find(tm_time.tm_hour) == expr.hours.end()) {
		return false;
	}

	// Check month
	int month = tm_time.tm_mon + 1;  // tm_mon is 0-based
	if (expr.months.find(month) == expr.months.end()) {
		return false;
	}

	// Check day of month and day of week
	// In standard cron, if both are specified (not *), either can match
	int day = tm_time.tm_mday;
	int weekday = tm_time.tm_wday;

	bool day_match = expr.days.find(day) != expr.days.end();
	bool weekday_match = expr.weekdays.find(weekday) != expr.weekdays.end();

	// Both must match
	return day_match && weekday_match;
}

// ============================================================================
// Next run time calculation
// ============================================================================

common::Result<std::chrono::system_clock::time_point> cron_parser::next_run_time(
	const cron_expression& expr,
	std::chrono::system_clock::time_point from) {
	// Start from the next minute
	auto time = from + std::chrono::seconds(60);

	// Round down to the start of the minute
	std::time_t tt = std::chrono::system_clock::to_time_t(time);
	std::tm tm_time;
#ifdef _WIN32
	localtime_s(&tm_time, &tt);
#else
	localtime_r(&tt, &tm_time);
#endif
	tm_time.tm_sec = 0;

	// Search for up to 4 years (covers all possible cron patterns)
	constexpr int max_iterations = 4 * 366 * 24 * 60;  // 4 years in minutes
	int iterations = 0;

	while (iterations < max_iterations) {
		// Check if current time matches
		std::time_t current_tt = std::mktime(&tm_time);
		auto current_time = std::chrono::system_clock::from_time_t(current_tt);

		if (matches(expr, current_time)) {
			return common::Result<std::chrono::system_clock::time_point>(current_time);
		}

		// Try to skip efficiently
		bool found_skip = false;

		// Skip to next valid month
		int month = tm_time.tm_mon + 1;
		if (expr.months.find(month) == expr.months.end()) {
			auto it = expr.months.upper_bound(month);
			if (it != expr.months.end()) {
				tm_time.tm_mon = *it - 1;
				tm_time.tm_mday = 1;
				tm_time.tm_hour = 0;
				tm_time.tm_min = 0;
				found_skip = true;
			} else {
				// Wrap to next year
				tm_time.tm_year++;
				tm_time.tm_mon = *expr.months.begin() - 1;
				tm_time.tm_mday = 1;
				tm_time.tm_hour = 0;
				tm_time.tm_min = 0;
				found_skip = true;
			}
		}

		if (!found_skip) {
			// Skip to next valid day
			int day = tm_time.tm_mday;
			int year = tm_time.tm_year + 1900;
			int max_day = days_in_month(year, month);

			// Find next valid day considering both day of month and weekday
			bool day_found = false;
			for (int d = day; d <= max_day && !day_found; d++) {
				tm_time.tm_mday = d;
				std::mktime(&tm_time);  // Normalize the time

				if (expr.days.find(tm_time.tm_mday) != expr.days.end()
					&& expr.weekdays.find(tm_time.tm_wday) != expr.weekdays.end()) {
					day_found = true;
					if (d > day) {
						tm_time.tm_hour = 0;
						tm_time.tm_min = 0;
						found_skip = true;
					}
				}
			}

			if (!day_found) {
				// Move to next month
				tm_time.tm_mon++;
				if (tm_time.tm_mon > 11) {
					tm_time.tm_mon = 0;
					tm_time.tm_year++;
				}
				tm_time.tm_mday = 1;
				tm_time.tm_hour = 0;
				tm_time.tm_min = 0;
				found_skip = true;
			}
		}

		if (!found_skip) {
			// Skip to next valid hour
			int hour = tm_time.tm_hour;
			if (expr.hours.find(hour) == expr.hours.end()) {
				auto it = expr.hours.upper_bound(hour);
				if (it != expr.hours.end()) {
					tm_time.tm_hour = *it;
					tm_time.tm_min = 0;
					found_skip = true;
				} else {
					// Wrap to next day
					tm_time.tm_mday++;
					tm_time.tm_hour = *expr.hours.begin();
					tm_time.tm_min = 0;
					std::mktime(&tm_time);  // Normalize
					found_skip = true;
				}
			}
		}

		if (!found_skip) {
			// Skip to next valid minute
			int minute = tm_time.tm_min;
			if (expr.minutes.find(minute) == expr.minutes.end()) {
				auto it = expr.minutes.upper_bound(minute);
				if (it != expr.minutes.end()) {
					tm_time.tm_min = *it;
					found_skip = true;
				} else {
					// Wrap to next hour
					tm_time.tm_hour++;
					tm_time.tm_min = *expr.minutes.begin();
					std::mktime(&tm_time);  // Normalize
					found_skip = true;
				}
			}
		}

		if (!found_skip) {
			// Just increment by one minute
			tm_time.tm_min++;
			std::mktime(&tm_time);  // Normalize
		}

		iterations++;
	}

	return common::Result<std::chrono::system_clock::time_point>(common::error_info{
		error::task_operation_failed,
		"Could not find next run time within 4 years"});
}

// ============================================================================
// Utility methods
// ============================================================================

bool cron_parser::is_valid(const std::string& expr) {
	auto result = parse(expr);
	return result.is_ok();
}

std::string cron_parser::to_string(const cron_expression& expr) {
	auto field_to_string = [](const std::set<int>& values, int min, int max) -> std::string {
		if (values.empty()) {
			return "*";
		}

		// Check if it's all values (wildcard)
		if (static_cast<int>(values.size()) == (max - min + 1)) {
			bool is_wildcard = true;
			int expected = min;
			for (int v : values) {
				if (v != expected++) {
					is_wildcard = false;
					break;
				}
			}
			if (is_wildcard) {
				return "*";
			}
		}

		// Check if it's a simple range
		auto first = *values.begin();
		auto last = *values.rbegin();
		if (static_cast<int>(values.size()) == (last - first + 1)) {
			bool is_range = true;
			int expected = first;
			for (int v : values) {
				if (v != expected++) {
					is_range = false;
					break;
				}
			}
			if (is_range) {
				if (values.size() == 1) {
					return std::to_string(first);
				}
				return std::to_string(first) + "-" + std::to_string(last);
			}
		}

		// Check if it's a step pattern
		if (values.size() > 1) {
			auto it = values.begin();
			int first_val = *it++;
			int step = *it - first_val;
			bool is_step = step > 0;
			int expected = first_val + step;

			while (is_step && it != values.end()) {
				if (*it != expected) {
					is_step = false;
				}
				expected += step;
				++it;
			}

			if (is_step && first_val == min) {
				return "*/" + std::to_string(step);
			}
		}

		// Fall back to comma-separated list
		std::ostringstream oss;
		bool first_item = true;
		for (int v : values) {
			if (!first_item) {
				oss << ",";
			}
			oss << v;
			first_item = false;
		}
		return oss.str();
	};

	return field_to_string(expr.minutes, 0, 59) + " " + field_to_string(expr.hours, 0, 23)
		   + " " + field_to_string(expr.days, 1, 31) + " "
		   + field_to_string(expr.months, 1, 12) + " "
		   + field_to_string(expr.weekdays, 0, 6);
}

}  // namespace kcenon::messaging::task
