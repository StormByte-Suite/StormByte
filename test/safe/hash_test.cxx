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

#include <StormByte/byte_size.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <span>

using namespace StormByte;

enum class Sample { Red, Blue };

// -------------------
// Binary
// -------------------

int test_binary_bytes_and_empty() {
	Safe::Binary empty;
	Safe::Binary other;
	ASSERT_EQUAL(Safe::Hash<Safe::Binary>{}(empty), Safe::Hash<Safe::Binary>{}(other));
	ASSERT_EQUAL(Safe::HashBytes({}), Safe::Hash<Safe::Binary>{}(empty));
	Safe::Binary bytes{std::byte{1}, std::byte{0}, std::byte{2}};
	Safe::Binary same{std::byte{1}, std::byte{0}, std::byte{2}};
	Safe::Binary truncated{std::byte{1}};
	ASSERT_EQUAL(Safe::Hash<Safe::Binary>{}(bytes), Safe::Hash<Safe::Binary>{}(same));
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::Binary>{}(bytes), Safe::Hash<Safe::Binary>{}(truncated));
	ASSERT_EQUAL(std::hash<Safe::Binary>{}(bytes), std::hash<Safe::Binary>{}(same));
	RETURN_TEST(0);
}

// -------------------
// ByteSize
// -------------------

int test_byte_size_count() {
	ASSERT_EQUAL(Safe::Hash<ByteSize>{}(ByteSize(0)), Safe::Hash<ByteSize>{}(ByteSize(0)));
	ASSERT_NOT_EQUAL(Safe::Hash<ByteSize>{}(ByteSize(1)), Safe::Hash<ByteSize>{}(ByteSize(2)));
	ASSERT_EQUAL(Safe::Hash<ByteSize>{}(ByteSize::KiB(1)), Safe::Hash<ByteSize>{}(ByteSize(1024)));
	ASSERT_EQUAL(std::hash<ByteSize>{}(ByteSize(9)), std::hash<ByteSize>{}(ByteSize(9)));
	RETURN_TEST(0);
}

// -------------------
// Combine
// -------------------

int test_combine_order() {
	const std::size_t left = Safe::HashCombine(1, 2);
	const std::size_t right = Safe::HashCombine(2, 1);
	ASSERT_NOT_EQUAL(left, right);
	ASSERT_EQUAL(left, Safe::HashCombine(1, 2));
	ASSERT_EQUAL(Safe::HashBytes({}), Safe::HashBytes(std::span<const std::byte>{}));
	RETURN_TEST(0);
}

// -------------------
// Enum
// -------------------

int test_enum_identity() {
	ASSERT_EQUAL(Safe::Hash<Sample>{}(Sample::Red), Safe::Hash<Sample>{}(Sample::Red));
	ASSERT_NOT_EQUAL(Safe::Hash<Sample>{}(Sample::Red), Safe::Hash<Sample>{}(Sample::Blue));
	RETURN_TEST(0);
}

// -------------------
// Integral
// -------------------

int test_integral_and_floating() {
	ASSERT_EQUAL(Safe::Hash<int>{}(7), Safe::Hash<int>{}(7));
	ASSERT_NOT_EQUAL(Safe::Hash<int>{}(7), Safe::Hash<int>{}(8));
	ASSERT_EQUAL(Safe::Hash<int>{}(-1), Safe::Hash<int>{}(-1));
	ASSERT_NOT_EQUAL(Safe::Hash<int>{}(-1), Safe::Hash<int>{}(1));
	ASSERT_EQUAL(Safe::Hash<bool>{}(true), Safe::Hash<bool>{}(true));
	ASSERT_NOT_EQUAL(Safe::Hash<bool>{}(true), Safe::Hash<bool>{}(false));
	ASSERT_EQUAL(Safe::Hash<char>{}('a'), Safe::Hash<char>{}('a'));
	ASSERT_NOT_EQUAL(Safe::Hash<char>{}('a'), Safe::Hash<char>{}('b'));
	ASSERT_EQUAL(Safe::Hash<unsigned long long>{}(1ull), Safe::Hash<unsigned long long>{}(1ull));
	ASSERT_EQUAL(Safe::Hash<double>{}(1.5), Safe::Hash<double>{}(1.5));
	ASSERT_NOT_EQUAL(Safe::Hash<double>{}(1.5), Safe::Hash<double>{}(1.25));
	ASSERT_EQUAL(Safe::Hash<float>{}(2.0f), Safe::Hash<float>{}(2.0f));
	ASSERT_NOT_EQUAL(Safe::Hash<float>{}(2.0f), Safe::Hash<float>{}(-2.0f));
	ASSERT_EQUAL(Safe::Hash<long double>{}(3.0L), Safe::Hash<long double>{}(3.0L));
	RETURN_TEST(0);
}

// -------------------
// Optional
// -------------------

int test_optional_empty_and_engaged() {
	Safe::Optional<int> empty;
	Safe::Optional<int> zero(0);
	Safe::Optional<int> one(1);
	ASSERT_EQUAL(Safe::Hash<Safe::Optional<int>>{}(empty), Safe::Hash<Safe::Optional<int>>{}(Safe::Optional<int>{}));
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::Optional<int>>{}(empty), Safe::Hash<Safe::Optional<int>>{}(zero));
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::Optional<int>>{}(zero), Safe::Hash<Safe::Optional<int>>{}(one));
	ASSERT_EQUAL(std::hash<Safe::Optional<int>>{}(one), std::hash<Safe::Optional<int>>{}(Safe::Optional<int>(1)));
	RETURN_TEST(0);
}

// -------------------
// Owner
// -------------------

int test_owner_identity() {
	Safe::Shared<int> empty_shared;
	Safe::Unique<int> empty_unique;
	Safe::Weak<int> empty_weak;
	ASSERT_EQUAL(Safe::Hash<int*>{}(nullptr), Safe::Hash<Safe::Shared<int>>{}(empty_shared));
	ASSERT_EQUAL(Safe::Hash<int*>{}(nullptr), Safe::Hash<Safe::Unique<int>>{}(empty_unique));
	ASSERT_EQUAL(Safe::Hash<int*>{}(nullptr), Safe::Hash<Safe::Weak<int>>{}(empty_weak));
	Safe::Shared<int> shared = Safe::MakeShared<int>(4);
	Safe::Shared<int> same = shared;
	Safe::Shared<int> other = Safe::MakeShared<int>(4);
	Safe::Weak<int> observer(shared);
	ASSERT_EQUAL(Safe::Hash<Safe::Shared<int>>{}(shared), Safe::Hash<Safe::Shared<int>>{}(same));
	ASSERT_EQUAL(Safe::Hash<Safe::Shared<int>>{}(shared), Safe::Hash<Safe::Weak<int>>{}(observer));
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::Shared<int>>{}(shared), Safe::Hash<Safe::Shared<int>>{}(other));
	ASSERT_EQUAL(std::hash<Safe::Shared<int>>{}(shared), Safe::Hash<Safe::Shared<int>>{}(shared));
	ASSERT_EQUAL(std::hash<Safe::Weak<int>>{}(observer), Safe::Hash<Safe::Weak<int>>{}(observer));
	shared.reset();
	same.reset();
	ASSERT_TRUE(observer.expired());
	ASSERT_EQUAL(Safe::Hash<int*>{}(nullptr), Safe::Hash<Safe::Weak<int>>{}(observer));
	Safe::Unique<int> unique = Safe::MakeUnique<int>(9);
	Safe::Unique<int> another = Safe::MakeUnique<int>(9);
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::Unique<int>>{}(unique), Safe::Hash<Safe::Unique<int>>{}(another));
	ASSERT_EQUAL(std::hash<Safe::Unique<int>>{}(unique), Safe::Hash<Safe::Unique<int>>{}(unique));
	RETURN_TEST(0);
}

// -------------------
// Pair
// -------------------

int test_pair_order() {
	Safe::Pair<int, int> left(1, 2);
	Safe::Pair<int, int> same(1, 2);
	Safe::Pair<int, int> swapped(2, 1);
	ASSERT_EQUAL((Safe::Hash<Safe::Pair<int, int>>{}(left)), (Safe::Hash<Safe::Pair<int, int>>{}(same)));
	ASSERT_NOT_EQUAL((Safe::Hash<Safe::Pair<int, int>>{}(left)), (Safe::Hash<Safe::Pair<int, int>>{}(swapped)));
	ASSERT_EQUAL((std::hash<Safe::Pair<int, int>>{}(left)), (std::hash<Safe::Pair<int, int>>{}(same)));
	RETURN_TEST(0);
}

// -------------------
// Pointer
// -------------------

int test_pointer_identity() {
	int value = 4;
	int other = 4;
	ASSERT_EQUAL(Safe::Hash<int*>{}(&value), Safe::Hash<int*>{}(&value));
	ASSERT_EQUAL(Safe::Hash<int*>{}(nullptr), Safe::Hash<int*>{}(static_cast<int*>(nullptr)));
	ASSERT_NOT_EQUAL(Safe::Hash<int*>{}(&value), Safe::Hash<int*>{}(&other));
	ASSERT_NOT_EQUAL(Safe::Hash<int*>{}(&value), Safe::Hash<int*>{}(nullptr));
	RETURN_TEST(0);
}

// -------------------
// Size
// -------------------

int test_size_count() {
	ASSERT_EQUAL(Safe::Hash<Size>{}(Size(0)), Safe::Hash<Size>{}(Size(0)));
	ASSERT_NOT_EQUAL(Safe::Hash<Size>{}(Size(1)), Safe::Hash<Size>{}(Size(2)));
	ASSERT_EQUAL(std::hash<Size>{}(Size(9)), std::hash<Size>{}(Size(9)));
	RETURN_TEST(0);
}

// -------------------
// String
// -------------------

int test_string_embedded_nul() {
	Safe::String empty;
	ASSERT_EQUAL(Safe::Hash<Safe::String>{}(empty), Safe::Hash<Safe::String>{}(Safe::String()));
	Safe::String text("ab");
	Safe::String same("ab");
	Safe::String nul("a");
	nul.push_back('\0');
	nul.push_back('b');
	ASSERT_EQUAL(Safe::Hash<Safe::String>{}(text), Safe::Hash<Safe::String>{}(same));
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::String>{}(text), Safe::Hash<Safe::String>{}(nul));
	ASSERT_EQUAL(std::hash<Safe::String>{}(text), std::hash<Safe::String>{}(same));
	RETURN_TEST(0);
}

// -------------------
// WString
// -------------------

int test_wstring_units() {
	Safe::WString empty;
	ASSERT_EQUAL(Safe::Hash<Safe::WString>{}(empty), Safe::Hash<Safe::WString>{}(Safe::WString()));
	Safe::WString text(L"ab");
	Safe::WString same(L"ab");
	Safe::WString other(L"ac");
	ASSERT_EQUAL(Safe::Hash<Safe::WString>{}(text), Safe::Hash<Safe::WString>{}(same));
	ASSERT_NOT_EQUAL(Safe::Hash<Safe::WString>{}(text), Safe::Hash<Safe::WString>{}(other));
	ASSERT_EQUAL(std::hash<Safe::WString>{}(text), std::hash<Safe::WString>{}(same));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Binary
	// -------------------
	result += test_binary_bytes_and_empty();

	// -------------------
	// ByteSize
	// -------------------
	result += test_byte_size_count();

	// -------------------
	// Combine
	// -------------------
	result += test_combine_order();

	// -------------------
	// Enum
	// -------------------
	result += test_enum_identity();

	// -------------------
	// Integral
	// -------------------
	result += test_integral_and_floating();

	// -------------------
	// Optional
	// -------------------
	result += test_optional_empty_and_engaged();

	// -------------------
	// Owner
	// -------------------
	result += test_owner_identity();

	// -------------------
	// Pair
	// -------------------
	result += test_pair_order();

	// -------------------
	// Pointer
	// -------------------
	result += test_pointer_identity();

	// -------------------
	// Size
	// -------------------
	result += test_size_count();

	// -------------------
	// String
	// -------------------
	result += test_string_embedded_nul();

	// -------------------
	// WString
	// -------------------
	result += test_wstring_units();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
