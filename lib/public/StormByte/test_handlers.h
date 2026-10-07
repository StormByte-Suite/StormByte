/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte source in this repository.
 * They do not cover other StormByte modules or any third-party material
 * shipped with this repository (including everything under thirdparty/),
 * which remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#pragma once

#include <exception>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string_view>

namespace StormByte::Test::Detail {
	template<typename T>
	void PrintValue(std::ostream& stream, const T& value) {
		if constexpr (requires { stream << value; }) {
			stream << value;
		} else {
			stream << "<unprintable>";
		}
	}
}

/**
 * @def RETURN_TEST
 * @brief Prints `FAILED` to `stderr` when @p fn_result is not `0`, then `return`s that value.
 * @param fn_result Integer status (`0` = pass).
 *
 * The test name is `__func__` of the function that expands the macro.
 */
#define RETURN_TEST(fn_result) do { \
	const int stormbyte_test_result = (fn_result); \
	if (stormbyte_test_result != 0) { \
		std::cerr << "Test " << __func__ << " FAILED!" << std::endl; \
	} \
	return stormbyte_test_result; \
} while (false)

/**
 * @def CurrentFileDirectory
 * @brief Directory of the translation unit (`std::filesystem::path` of `__FILE__`).
 */
#define CurrentFileDirectory std::filesystem::path(__FILE__).parent_path()

/**
 * @def ASSERT_EQUAL
 * @brief Fails the test (`return 1`) when @p expected != @p actual.
 * @param expected Expected value.
 * @param actual Observed value.
 */
#define ASSERT_EQUAL(expected, actual) do { \
	const auto& stormbyte_expected = (expected); \
	const auto& stormbyte_actual = (actual); \
	if (!(stormbyte_expected == stormbyte_actual)) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected \""; \
		::StormByte::Test::Detail::PrintValue(std::cerr, stormbyte_expected); \
		std::cerr << "\", got \""; \
		::StormByte::Test::Detail::PrintValue(std::cerr, stormbyte_actual); \
		std::cerr << "\"" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NOT_EQUAL
 * @brief Fails the test (`return 1`) when @p expected == @p actual.
 * @param expected Value that must differ.
 * @param actual Observed value.
 */
#define ASSERT_NOT_EQUAL(expected, actual) do { \
	const auto& stormbyte_expected = (expected); \
	const auto& stormbyte_actual = (actual); \
	if (stormbyte_expected == stormbyte_actual) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": values should differ; both were \""; \
		::StormByte::Test::Detail::PrintValue(std::cerr, stormbyte_actual); \
		std::cerr << "\"" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_FALSE
 * @brief Fails the test (`return 1`) when @p condition is true.
 * @param condition Expression that must be false.
 */
#define ASSERT_FALSE(condition) do { \
	if ((condition)) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": condition is true, expected false" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_TRUE
 * @brief Fails the test (`return 1`) when @p condition is false.
 * @param condition Expression that must be true.
 */
#define ASSERT_TRUE(condition) do { \
	if (!(condition)) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": condition is false, expected true" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_THROWS
 * @brief Fails when @p expression does not throw @p exception_type.
 * @param expression Expression that must throw.
 * @param exception_type Exception type that must be caught.
 */
#define ASSERT_THROWS(expression, exception_type) do { \
	bool stormbyte_threw = false; \
	try { \
		(void)(expression); \
	} catch (const exception_type&) { \
		stormbyte_threw = true; \
	} catch (...) { \
	} \
	if (!stormbyte_threw) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected " << #exception_type << " to be thrown" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NO_THROW
 * @brief Fails when @p expression throws any exception.
 * @param expression Expression that must not throw.
 */
#define ASSERT_NO_THROW(expression) do { \
	try { \
		(void)(expression); \
	} catch (const std::exception& stormbyte_exception) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": unexpected exception: " << stormbyte_exception.what() << std::endl; \
		return 1; \
	} catch (...) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": unexpected non-standard exception" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NEAR
 * @brief Fails when two arithmetic values differ by more than @p tolerance.
 * @param expected Expected value.
 * @param actual Observed value.
 * @param tolerance Maximum accepted difference.
 */
#define ASSERT_NEAR(expected, actual, tolerance) do { \
	const auto stormbyte_expected = (expected); \
	const auto stormbyte_actual = (actual); \
	const auto stormbyte_tolerance = (tolerance); \
	const auto stormbyte_difference = stormbyte_expected > stormbyte_actual ? \
		stormbyte_expected - stormbyte_actual : stormbyte_actual - stormbyte_expected; \
	if (stormbyte_difference > stormbyte_tolerance) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": values differ beyond tolerance" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_CONTAINS
 * @brief Fails when @p haystack does not contain @p needle.
 * @param haystack Text that must contain the needle.
 * @param needle Text that must be found.
 */
#define ASSERT_CONTAINS(haystack, needle) do { \
	const auto& stormbyte_haystack = (haystack); \
	const auto& stormbyte_needle = (needle); \
	if (stormbyte_haystack.find(stormbyte_needle) == std::string_view::npos) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected text to contain \"" << stormbyte_needle << "\"" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NOT_CONTAINS
 * @brief Fails when @p haystack contains @p needle.
 * @param haystack Text that must not contain the needle.
 * @param needle Text that must be absent.
 */
#define ASSERT_NOT_CONTAINS(haystack, needle) do { \
	const auto& stormbyte_haystack = (haystack); \
	const auto& stormbyte_needle = (needle); \
	if (stormbyte_haystack.find(stormbyte_needle) != std::string_view::npos) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected text not to contain \"" << stormbyte_needle << "\"" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NOT_NULL
 * @brief Fails when @p pointer is null.
 * @param pointer Pointer that must not be null.
 */
#define ASSERT_NOT_NULL(pointer) do { \
	if ((pointer) == nullptr) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": pointer is null" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NULL
 * @brief Fails when @p pointer is not null.
 * @param pointer Pointer that must be null.
 */
#define ASSERT_NULL(pointer) do { \
	if ((pointer) != nullptr) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": pointer is not null" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_EMPTY
 * @brief Fails when @p container is not empty.
 * @param container Container with `empty()` and `size()`.
 */
#define ASSERT_EMPTY(container) do { \
	const auto& stormbyte_container = (container); \
	if (!stormbyte_container.empty()) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected empty, size is "; \
		::StormByte::Test::Detail::PrintValue(std::cerr, stormbyte_container.size()); \
		std::cerr << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_NOT_EMPTY
 * @brief Fails when @p container is empty.
 * @param container Container with `empty()`.
 */
#define ASSERT_NOT_EMPTY(container) do { \
	const auto& stormbyte_container = (container); \
	if (stormbyte_container.empty()) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected not empty" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_SIZE
 * @brief Fails when @p container `size()` is not @p expected.
 * @param container Container with `size()`.
 * @param expected Expected size.
 */
#define ASSERT_SIZE(container, expected) do { \
	const auto& stormbyte_container = (container); \
	const auto stormbyte_expected = (expected); \
	if (!(stormbyte_container.size() == stormbyte_expected)) { \
		std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected size \""; \
		::StormByte::Test::Detail::PrintValue(std::cerr, stormbyte_expected); \
		std::cerr << "\", got \""; \
		::StormByte::Test::Detail::PrintValue(std::cerr, stormbyte_container.size()); \
		std::cerr << "\"" << std::endl; \
		return 1; \
	} \
} while (false)

/**
 * @def ASSERT_FAIL
 * @brief Fails the test. For a branch that must not be reached.
 * @param reason Text written to the log.
 */
#define ASSERT_FAIL(reason) do { \
	std::cerr << __func__ << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": " << (reason) << std::endl; \
	return 1; \
} while (false)
