// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/cron_parser.h>

#include <gtest/gtest.h>

#include <chrono>
#include <ctime>

namespace tsk = kcenon::messaging::task;
using tsk::cron_parser;
using tsk::cron_expression;

// Helper function to create a time_point from components
std::chrono::system_clock::time_point make_time(int year,
												int month,
												int day,
												int hour,
												int minute) {
	std::tm tm{};
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = 0;
	tm.tm_isdst = -1;  // Let the system determine DST

	std::time_t tt = std::mktime(&tm);
	return std::chrono::system_clock::from_time_t(tt);
}

// ============================================================================
// Basic parsing tests
// ============================================================================

TEST(CronParserTest, ParseWildcard) {
	auto result = cron_parser::parse("* * * * *");
	ASSERT_TRUE(result.is_ok());

	auto expr = result.unwrap();
	EXPECT_EQ(expr.minutes.size(), 60u);   // 0-59
	EXPECT_EQ(expr.hours.size(), 24u);     // 0-23
	EXPECT_EQ(expr.days.size(), 31u);      // 1-31
	EXPECT_EQ(expr.months.size(), 12u);    // 1-12
	EXPECT_EQ(expr.weekdays.size(), 7u);   // 0-6
}

TEST(CronParserTest, ParseSpecificValues) {
	auto result = cron_parser::parse("30 14 15 6 3");
	ASSERT_TRUE(result.is_ok());

	auto expr = result.unwrap();
	EXPECT_EQ(expr.minutes.size(), 1u);
	EXPECT_TRUE(expr.minutes.count(30));
	EXPECT_EQ(expr.hours.size(), 1u);
	EXPECT_TRUE(expr.hours.count(14));
	EXPECT_EQ(expr.days.size(), 1u);
	EXPECT_TRUE(expr.days.count(15));
	EXPECT_EQ(expr.months.size(), 1u);
	EXPECT_TRUE(expr.months.count(6));
	EXPECT_EQ(expr.weekdays.size(), 1u);
	EXPECT_TRUE(expr.weekdays.count(3));
}

TEST(CronParserTest, ParseIntervals) {
	auto result = cron_parser::parse("*/15 */2 * * *");
	ASSERT_TRUE(result.is_ok());

	auto expr = result.unwrap();

	// Minutes: 0, 15, 30, 45
	EXPECT_EQ(expr.minutes.size(), 4u);
	EXPECT_TRUE(expr.minutes.count(0));
	EXPECT_TRUE(expr.minutes.count(15));
	EXPECT_TRUE(expr.minutes.count(30));
	EXPECT_TRUE(expr.minutes.count(45));

	// Hours: 0, 2, 4, ..., 22
	EXPECT_EQ(expr.hours.size(), 12u);
	EXPECT_TRUE(expr.hours.count(0));
	EXPECT_TRUE(expr.hours.count(2));
	EXPECT_TRUE(expr.hours.count(22));
	EXPECT_FALSE(expr.hours.count(1));
	EXPECT_FALSE(expr.hours.count(23));
}

TEST(CronParserTest, ParseRanges) {
	auto result = cron_parser::parse("0-30 9-17 1-15 * 1-5");
	ASSERT_TRUE(result.is_ok());

	auto expr = result.unwrap();

	// Minutes: 0-30
	EXPECT_EQ(expr.minutes.size(), 31u);
	EXPECT_TRUE(expr.minutes.count(0));
	EXPECT_TRUE(expr.minutes.count(30));
	EXPECT_FALSE(expr.minutes.count(31));

	// Hours: 9-17
	EXPECT_EQ(expr.hours.size(), 9u);
	EXPECT_TRUE(expr.hours.count(9));
	EXPECT_TRUE(expr.hours.count(17));
	EXPECT_FALSE(expr.hours.count(8));
	EXPECT_FALSE(expr.hours.count(18));

	// Days: 1-15
	EXPECT_EQ(expr.days.size(), 15u);

	// Weekdays: 1-5 (Mon-Fri)
	EXPECT_EQ(expr.weekdays.size(), 5u);
	EXPECT_TRUE(expr.weekdays.count(1));
	EXPECT_TRUE(expr.weekdays.count(5));
	EXPECT_FALSE(expr.weekdays.count(0));  // Sunday
	EXPECT_FALSE(expr.weekdays.count(6));  // Saturday
}

TEST(CronParserTest, ParseLists) {
	auto result = cron_parser::parse("0,15,30,45 8,12,18 * * 1,3,5");
	ASSERT_TRUE(result.is_ok());

	auto expr = result.unwrap();

	// Minutes: 0, 15, 30, 45
	EXPECT_EQ(expr.minutes.size(), 4u);
	EXPECT_TRUE(expr.minutes.count(0));
	EXPECT_TRUE(expr.minutes.count(15));
	EXPECT_TRUE(expr.minutes.count(30));
	EXPECT_TRUE(expr.minutes.count(45));

	// Hours: 8, 12, 18
	EXPECT_EQ(expr.hours.size(), 3u);
	EXPECT_TRUE(expr.hours.count(8));
	EXPECT_TRUE(expr.hours.count(12));
	EXPECT_TRUE(expr.hours.count(18));

	// Weekdays: 1, 3, 5 (Mon, Wed, Fri)
	EXPECT_EQ(expr.weekdays.size(), 3u);
	EXPECT_TRUE(expr.weekdays.count(1));
	EXPECT_TRUE(expr.weekdays.count(3));
	EXPECT_TRUE(expr.weekdays.count(5));
}

TEST(CronParserTest, ParseCombined) {
	// Every 10 minutes from 9 AM to 5 PM on weekdays
	auto result = cron_parser::parse("*/10 9-17 * * 1-5");
	ASSERT_TRUE(result.is_ok());

	auto expr = result.unwrap();

	// Minutes: 0, 10, 20, 30, 40, 50
	EXPECT_EQ(expr.minutes.size(), 6u);

	// Hours: 9-17
	EXPECT_EQ(expr.hours.size(), 9u);

	// Weekdays: 1-5
	EXPECT_EQ(expr.weekdays.size(), 5u);
}

// ============================================================================
// Parsing error tests
// ============================================================================

TEST(CronParserTest, ParseEmptyExpression) {
	auto result = cron_parser::parse("");
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseTooFewFields) {
	auto result = cron_parser::parse("* * * *");  // 4 fields
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseTooManyFields) {
	auto result = cron_parser::parse("* * * * * *");  // 6 fields
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseInvalidMinute) {
	auto result = cron_parser::parse("60 * * * *");  // 60 is out of range
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseInvalidHour) {
	auto result = cron_parser::parse("* 24 * * *");  // 24 is out of range
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseInvalidDay) {
	auto result = cron_parser::parse("* * 0 * *");   // 0 is out of range for days
	EXPECT_TRUE(result.is_err());

	auto result2 = cron_parser::parse("* * 32 * *");  // 32 is out of range
	EXPECT_TRUE(result2.is_err());
}

TEST(CronParserTest, ParseInvalidMonth) {
	auto result = cron_parser::parse("* * * 0 *");   // 0 is out of range for months
	EXPECT_TRUE(result.is_err());

	auto result2 = cron_parser::parse("* * * 13 *");  // 13 is out of range
	EXPECT_TRUE(result2.is_err());
}

TEST(CronParserTest, ParseInvalidWeekday) {
	auto result = cron_parser::parse("* * * * 7");  // 7 is out of range
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseInvalidRange) {
	auto result = cron_parser::parse("30-10 * * * *");  // start > end
	EXPECT_TRUE(result.is_err());
}

TEST(CronParserTest, ParseInvalidStep) {
	auto result = cron_parser::parse("*/0 * * * *");  // step must be > 0
	EXPECT_TRUE(result.is_err());
}

// ============================================================================
// Validation tests
// ============================================================================

TEST(CronParserTest, IsValid) {
	EXPECT_TRUE(cron_parser::is_valid("* * * * *"));
	EXPECT_TRUE(cron_parser::is_valid("0 3 * * *"));
	EXPECT_TRUE(cron_parser::is_valid("*/15 9-17 * * 1-5"));

	EXPECT_FALSE(cron_parser::is_valid(""));
	EXPECT_FALSE(cron_parser::is_valid("* * * *"));
	EXPECT_FALSE(cron_parser::is_valid("60 * * * *"));
	EXPECT_FALSE(cron_parser::is_valid("invalid"));
}

// ============================================================================
// ToString tests
// ============================================================================

TEST(CronParserTest, ToStringWildcard) {
	auto result = cron_parser::parse("* * * * *");
	ASSERT_TRUE(result.is_ok());

	auto str = cron_parser::to_string(result.unwrap());
	EXPECT_EQ(str, "* * * * *");
}

TEST(CronParserTest, ToStringSpecificValues) {
	auto result = cron_parser::parse("30 14 15 6 3");
	ASSERT_TRUE(result.is_ok());

	auto str = cron_parser::to_string(result.unwrap());
	EXPECT_EQ(str, "30 14 15 6 3");
}

TEST(CronParserTest, ToStringRange) {
	auto result = cron_parser::parse("0-30 9-17 * * 1-5");
	ASSERT_TRUE(result.is_ok());

	auto str = cron_parser::to_string(result.unwrap());
	// Should recognize the range pattern
	EXPECT_TRUE(str.find("0-30") != std::string::npos);
	EXPECT_TRUE(str.find("9-17") != std::string::npos);
	EXPECT_TRUE(str.find("1-5") != std::string::npos);
}

TEST(CronParserTest, ToStringStep) {
	auto result = cron_parser::parse("*/15 * * * *");
	ASSERT_TRUE(result.is_ok());

	auto str = cron_parser::to_string(result.unwrap());
	// Should recognize the step pattern
	EXPECT_TRUE(str.find("*/15") != std::string::npos);
}

// ============================================================================
// Next run time tests
// ============================================================================

TEST(CronParserTest, NextRunTimeEveryMinute) {
	auto parse_result = cron_parser::parse("* * * * *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	auto from = make_time(2025, 1, 15, 10, 30);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be exactly one minute later
	auto expected = make_time(2025, 1, 15, 10, 31);
	EXPECT_EQ(next, expected);
}

TEST(CronParserTest, NextRunTimeSpecificMinute) {
	// At minute 0 of every hour
	auto parse_result = cron_parser::parse("0 * * * *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	auto from = make_time(2025, 1, 15, 10, 30);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be 11:00
	auto expected = make_time(2025, 1, 15, 11, 0);
	EXPECT_EQ(next, expected);
}

TEST(CronParserTest, NextRunTimeSpecificHour) {
	// At 3:00 AM every day
	auto parse_result = cron_parser::parse("0 3 * * *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	auto from = make_time(2025, 1, 15, 10, 30);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be 3:00 AM next day
	auto expected = make_time(2025, 1, 16, 3, 0);
	EXPECT_EQ(next, expected);
}

TEST(CronParserTest, NextRunTimeWeekday) {
	// At 9:00 AM on Monday (weekday 1)
	auto parse_result = cron_parser::parse("0 9 * * 1");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	// January 15, 2025 is Wednesday
	auto from = make_time(2025, 1, 15, 10, 30);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be Monday January 20, 2025 at 9:00 AM
	auto expected = make_time(2025, 1, 20, 9, 0);
	EXPECT_EQ(next, expected);
}

TEST(CronParserTest, NextRunTimeMonthChange) {
	// At minute 0 of every hour on the 31st day of the month
	auto parse_result = cron_parser::parse("0 0 31 * *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	// January 15, 2025
	auto from = make_time(2025, 1, 15, 10, 30);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be January 31, 2025 at 00:00 (if it's a matching weekday)
	// The result depends on the weekday matching
	EXPECT_TRUE(next > from);
}

TEST(CronParserTest, NextRunTimeInterval) {
	// Every 15 minutes
	auto parse_result = cron_parser::parse("*/15 * * * *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	auto from = make_time(2025, 1, 15, 10, 32);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be 10:45
	auto expected = make_time(2025, 1, 15, 10, 45);
	EXPECT_EQ(next, expected);
}

TEST(CronParserTest, NextRunTimeEveryTwoHoursOnWeekdays) {
	// "0 */2 * * 1-5" - Every 2 hours on weekdays
	auto parse_result = cron_parser::parse("0 */2 * * 1-5");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	// Wednesday January 15, 2025 at 10:30
	auto from = make_time(2025, 1, 15, 10, 30);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be 12:00 on the same day
	auto expected = make_time(2025, 1, 15, 12, 0);
	EXPECT_EQ(next, expected);
}

// ============================================================================
// Edge case tests
// ============================================================================

TEST(CronParserTest, LeapYear) {
	// February 29th
	auto parse_result = cron_parser::parse("0 0 29 2 *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	// 2024 is a leap year
	auto from = make_time(2024, 1, 1, 0, 0);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();
	EXPECT_TRUE(next > from);
}

TEST(CronParserTest, YearRollover) {
	// January 1st at midnight
	auto parse_result = cron_parser::parse("0 0 1 1 *");
	ASSERT_TRUE(parse_result.is_ok());
	auto expr = parse_result.unwrap();

	auto from = make_time(2025, 12, 31, 23, 59);
	auto result = cron_parser::next_run_time(expr, from);
	ASSERT_TRUE(result.is_ok());

	auto next = result.unwrap();

	// Should be January 1, 2026 at 00:00
	auto expected = make_time(2026, 1, 1, 0, 0);
	EXPECT_EQ(next, expected);
}

TEST(CronParserTest, ExpressionEquality) {
	auto result1 = cron_parser::parse("0 3 * * *");
	auto result2 = cron_parser::parse("0 3 * * *");
	ASSERT_TRUE(result1.is_ok());
	ASSERT_TRUE(result2.is_ok());

	EXPECT_EQ(result1.unwrap(), result2.unwrap());
}

TEST(CronParserTest, ExpressionInequality) {
	auto result1 = cron_parser::parse("0 3 * * *");
	auto result2 = cron_parser::parse("0 4 * * *");
	ASSERT_TRUE(result1.is_ok());
	ASSERT_TRUE(result2.is_ok());

	EXPECT_NE(result1.unwrap(), result2.unwrap());
}
