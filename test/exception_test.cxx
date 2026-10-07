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

#include <StormByte/exception.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <exception>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

using namespace StormByte;

namespace {
	class CryptoError: public Exception {
		public:
			template <typename... Args>
			explicit CryptoError(std::format_string<Args...> fmt, Args&&... args)
				: Exception(Path{"Crypto"}, fmt, std::forward<Args>(args)...) {}

			~CryptoError() override = default;

		protected:
			template <typename... Args>
			explicit CryptoError(Path child, std::format_string<Args...> fmt, Args&&... args)
				: Exception(Path{std::string("Crypto.") + std::string(child.text)}, fmt, std::forward<Args>(args)...) {}
	};

	class CrypterError: public CryptoError {
		public:
			template <typename... Args>
			explicit CrypterError(std::format_string<Args...> fmt, Args&&... args)
				: CryptoError(Path{"Crypter"}, fmt, std::forward<Args>(args)...) {}

			~CrypterError() override = default;
	};

	class EncryptError: public CrypterError {
		public:
			using CrypterError::CrypterError;
			~EncryptError() override = default;
	};
}

// -------------------
// Assign
// -------------------

int test_assign_copy_and_move() {
	Exception original("payload");
	Exception copy("other");
	copy = original;
	ASSERT_EQUAL(std::string{"StormByte: payload"}, std::string{copy.what()});
	ASSERT_EQUAL(std::string{"StormByte: payload"}, std::string{original.what()});
	Exception taken("empty");
	taken = std::move(original);
	ASSERT_EQUAL(std::string{"StormByte: payload"}, std::string{taken.what()});
	ASSERT_NOT_NULL(original.what());
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_from_lvalue_string() {
	const std::string message("from lvalue");
	Exception error(message);
	ASSERT_EQUAL(std::string{"StormByte: from lvalue"}, std::string{error.what()});
	ASSERT_EQUAL(std::string{"from lvalue"}, message);
	RETURN_TEST(0);
}

int test_construct_from_view_and_safe_string() {
	const std::string source("partial suffix");
	const std::string_view view(source.data(), 7);
	const Exception from_view(view);
	ASSERT_EQUAL(std::string{"StormByte: partial"}, std::string{from_view.what()});
	const Safe::String message("owned");
	const Exception from_owned(message);
	ASSERT_EQUAL(std::string{"StormByte: owned"}, std::string{from_owned.what()});
	RETURN_TEST(0);
}

int test_formatted_message_with_args() {
	Exception error("value is {}", 42);
	ASSERT_EQUAL(std::string{"StormByte: value is 42"}, std::string{error.what()});
	RETURN_TEST(0);
}

int test_parent_prepends_segment() {
	const CryptoError crypto("failed with code {}", 7);
	const EncryptError formatted("failed with code {}", 7);
	const EncryptError plain("plain text");
	ASSERT_EQUAL(std::string{"StormByte.Crypto: failed with code 7"}, std::string{crypto.what()});
	ASSERT_EQUAL(std::string{"StormByte.Crypto.Crypter: failed with code 7"}, std::string{formatted.what()});
	ASSERT_EQUAL(std::string{"StormByte.Crypto.Crypter: plain text"}, std::string{plain.what()});
	RETURN_TEST(0);
}

int test_plain_message_no_args() {
	Exception error("Key not found in Iterable::operator[]");
	ASSERT_EQUAL(std::string{"StormByte: Key not found in Iterable::operator[]"}, std::string{error.what()});
	ASSERT_NOT_NULL(error.what());
	RETURN_TEST(0);
}

int test_zero_args_format_string_is_as_is() {
	constexpr std::string_view text = "literal {{brace}} as-is";
	Exception error(text);
	ASSERT_EQUAL(std::string{"StormByte: literal {{brace}} as-is"}, std::string{error.what()});
	RETURN_TEST(0);
}

// -------------------
// Copy
// -------------------

int test_copy_keeps_what() {
	Exception original("payload");
	Exception copy(original);
	ASSERT_EQUAL(std::string{"StormByte: payload"}, std::string{copy.what()});
	ASSERT_EQUAL(std::string{"StormByte: payload"}, std::string{original.what()});
	RETURN_TEST(0);
}

int test_move_keeps_what_and_source_stays_valid() {
	Exception original("payload");
	Exception taken(std::move(original));
	ASSERT_EQUAL(std::string{"StormByte: payload"}, std::string{taken.what()});
	ASSERT_NOT_NULL(original.what());
	RETURN_TEST(0);
}

// -------------------
// Derived
// -------------------

int test_allocation_error_does_not_allocate_text() {
	const Safe::AllocationError error;
	ASSERT_EQUAL(std::string{"StormByte.Safe: Memory allocation failed"}, std::string{error.what()});
	const Safe::AllocationError copy(error);
	ASSERT_EQUAL(std::string{error.what()}, std::string{copy.what()});
	RETURN_TEST(0);
}

int test_bad_optional_access_is_static() {
	const Safe::BadOptionalAccess error;
	ASSERT_EQUAL(std::string{"StormByte.Safe: Optional has no value"}, std::string{error.what()});
	RETURN_TEST(0);
}

int test_root_leaves_add_no_segment() {
	ASSERT_EQUAL(std::string{"StormByte: wire"}, std::string{DeserializeError("wire").what()});
	ASSERT_EQUAL(std::string{"StormByte: alien"}, std::string{OperationError("alien").what()});
	ASSERT_EQUAL(std::string{"StormByte: bad alphabet"}, std::string{Base64Error("bad alphabet").what()});
	ASSERT_EQUAL(std::string{"StormByte: slot 3"}, std::string{DeserializeError("slot {}", 3).what()});
	const Safe::String body("owned leaf");
	ASSERT_EQUAL(std::string{"StormByte: owned leaf"}, std::string{OperationError(body).what()});
	RETURN_TEST(0);
}

int test_safe_path_and_catch() {
	try {
		throw Safe::OutOfBoundsError("index");
	} catch (const Safe::OutOfBoundsError& error) {
		ASSERT_EQUAL(std::string{"StormByte.Safe: index"}, std::string{error.what()});
	} catch (const Exception&) {
		ASSERT_FAIL("OutOfBoundsError missed its own catch");
	}
	try {
		throw Safe::ExpiredWeakPointerError("expired");
	} catch (const Safe::Exception& error) {
		ASSERT_EQUAL(std::string{"StormByte.Safe: expired"}, std::string{error.what()});
	}
	try {
		throw Base64Error("bad");
	} catch (const Exception& error) {
		ASSERT_EQUAL(std::string{"StormByte: bad"}, std::string{error.what()});
	}
	static_assert(std::is_base_of_v<Exception, Base64Error>);
	static_assert(std::is_base_of_v<Exception, Safe::Exception>);
	static_assert(std::is_base_of_v<Safe::Exception, Safe::AllocationError>);
	static_assert(!std::is_base_of_v<std::exception, Exception>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Assign
	// -------------------
	result += test_assign_copy_and_move();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_from_lvalue_string();
	result += test_construct_from_view_and_safe_string();
	result += test_formatted_message_with_args();
	result += test_parent_prepends_segment();
	result += test_plain_message_no_args();
	result += test_zero_args_format_string_is_as_is();

	// -------------------
	// Copy
	// -------------------
	result += test_copy_keeps_what();
	result += test_move_keeps_what_and_source_stays_valid();

	// -------------------
	// Derived
	// -------------------
	result += test_allocation_error_does_not_allocate_text();
	result += test_bad_optional_access_is_static();
	result += test_root_leaves_add_no_segment();
	result += test_safe_path_and_catch();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
