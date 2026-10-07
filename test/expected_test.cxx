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

#include <StormByte/expected.hxx>
#include <StormByte/test_handlers.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

using namespace StormByte;

namespace {
	struct TestError {
		explicit TestError(std::string message): message(std::move(message)) {}
		std::string message;
	};

	struct TestBaseError {
		virtual ~TestBaseError() = default;
		explicit TestBaseError(std::string message): message(std::move(message)) {}
		std::string message;
	};

	struct TestDerivedError: TestBaseError {
		using TestBaseError::TestBaseError;
	};
}

// -------------------
// Value
// -------------------

int test_expected_holds_value() {
	constexpr const char* test = "test_expected_holds_value";
	int result = 0;
	Expected<int, TestError> success = 42;
	ASSERT_TRUE(test, success.has_value());
	ASSERT_EQUAL(test, 42, success.value());
	RETURN_TEST(test, result);
}

// -------------------
// Error
// -------------------

int test_expected_holds_error() {
	constexpr const char* test = "test_expected_holds_error";
	int result = 0;
	Expected<int, TestError> failure = Unexpected<TestError>(TestError{"failed"});
	ASSERT_FALSE(test, failure.has_value());
	ASSERT_TRUE(test, failure.error() != nullptr);
	ASSERT_EQUAL(test, std::string("failed"), failure.error()->message);
	RETURN_TEST(test, result);
}

int test_unexpected_from_shared() {
	constexpr const char* test = "test_unexpected_from_shared";
	int result = 0;
	Safe::Shared<TestError> ptr = Safe::MakeShared<TestError>("via pointer");
	Expected<int, TestError> failure = Unexpected(ptr);
	ASSERT_FALSE(test, failure.has_value());
	ASSERT_TRUE(test, failure.error() == ptr);
	RETURN_TEST(test, result);
}

int test_unexpected_from_format() {
	constexpr const char* test = "test_unexpected_from_format";
	int result = 0;
	Expected<int, TestError> failure = Unexpected<TestError>("code {}", 7);
	ASSERT_FALSE(test, failure.has_value());
	ASSERT_EQUAL(test, std::string("code 7"), failure.error()->message);
	RETURN_TEST(test, result);
}

int test_unexpected_from_format_no_args() {
	constexpr const char* test = "test_unexpected_from_format_no_args";
	int result = 0;
	Expected<int, TestError> failure = Unexpected<TestError>("plain");
	ASSERT_FALSE(test, failure.has_value());
	ASSERT_EQUAL(test, std::string("plain"), failure.error()->message);
	RETURN_TEST(test, result);
}

int test_unexpected_same_type_not_upcast() {
	constexpr const char* test = "test_unexpected_same_type_not_upcast";
	int result = 0;
	Expected<int, TestError> failure = Unexpected<TestError>(TestError("same"));
	ASSERT_FALSE(test, failure.has_value());
	ASSERT_EQUAL(test, std::string("same"), failure.error()->message);
	RETURN_TEST(test, result);
}

int test_unexpected_derived_error_upcast() {
	constexpr const char* test = "test_unexpected_derived_error_upcast";
	int result = 0;
	Expected<int, TestBaseError> failure = Unexpected<TestBaseError>(TestDerivedError("derived"));
	ASSERT_FALSE(test, failure.has_value());
	ASSERT_TRUE(test, failure.error() != nullptr);
	ASSERT_EQUAL(test, std::string("derived"), failure.error()->message);
	RETURN_TEST(test, result);
}

// -------------------
// Reference
// -------------------

int test_expected_reference() {
	constexpr const char* test = "test_expected_reference";
	int result = 0;
	int value = 7;
	Expected<int&, TestError> reference = std::ref(value);
	ASSERT_TRUE(test, reference.has_value());
	reference.value().get() = 11;
	ASSERT_EQUAL(test, 11, value);
	RETURN_TEST(test, result);
}

int main() {
	int result = 0;

	// -------------------
	// Value
	// -------------------
	result += test_expected_holds_value();

	// -------------------
	// Error
	// -------------------
	result += test_expected_holds_error();
	result += test_unexpected_from_shared();
	result += test_unexpected_from_format();
	result += test_unexpected_from_format_no_args();
	result += test_unexpected_same_type_not_upcast();
	result += test_unexpected_derived_error_upcast();

	// -------------------
	// Reference
	// -------------------
	result += test_expected_reference();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
