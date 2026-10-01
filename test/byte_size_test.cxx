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
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>
#include <StormByte/safe/wstring.hxx>

#include <cstdint>
#include <iostream>
#include <string>

using namespace StormByte;

namespace {
	std::uint64_t U64(const ByteSize& size) {
		return static_cast<std::uint64_t>(size);
	}
}

// -------------------
// Arithmetic
// -------------------

int test_arithmetic_add_sub() {
	int result = 0;
	const ByteSize left{100};
	const ByteSize right{40};
	ASSERT_EQUAL("test_arithmetic_add_sub", 140ull, U64(left + right));
	ASSERT_EQUAL("test_arithmetic_add_sub", 60ull, U64(left - right));
	ByteSize acc{10};
	acc += ByteSize{5};
	ASSERT_EQUAL("test_arithmetic_add_sub", 15ull, U64(acc));
	acc -= ByteSize{3};
	ASSERT_EQUAL("test_arithmetic_add_sub", 12ull, U64(acc));
	RETURN_TEST("test_arithmetic_add_sub", result);
}

int test_arithmetic_expression_to_size_t() {
	int result = 0;
	const ByteSize size1{10};
	const ByteSize size2{4};
	const std::size_t calc = size1 + size2 + size2 + size2;
	ASSERT_EQUAL("test_arithmetic_expression_to_size_t", static_cast<std::size_t>(22), calc);
	RETURN_TEST("test_arithmetic_expression_to_size_t", result);
}

int test_arithmetic_increment() {
	int result = 0;
	ByteSize size{10};
	++size;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(size));
	const ByteSize post = size++;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(post));
	ASSERT_EQUAL("test_arithmetic_increment", 12ull, U64(size));
	--size;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(size));
	const ByteSize posted = size--;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(posted));
	ASSERT_EQUAL("test_arithmetic_increment", 10ull, U64(size));
	RETURN_TEST("test_arithmetic_increment", result);
}

int test_arithmetic_mixed_integer() {
	int result = 0;
	const ByteSize size{10};
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(size + 16));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(16 + size));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(size + 16u));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(16u + size));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(size + 16ull));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(16ull + size));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(size + static_cast<short>(16)));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(static_cast<unsigned short>(16) + size));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(size + 16L));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 26ull, U64(16UL + size));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 6ull, U64(size - 4));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 6ull, U64(size - 4u));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 0ull, U64(size - 10));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 11ull, U64(size + static_cast<std::ptrdiff_t>(-(-1))));
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 9ull, U64(size + static_cast<std::ptrdiff_t>(-1)));
	ByteSize acc = 10;
	acc += 5;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 15ull, U64(acc));
	acc += 5u;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 20ull, U64(acc));
	acc -= 3;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 17ull, U64(acc));
	acc -= 1ull;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 16ull, U64(acc));
	acc += static_cast<char>(1);
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 17ull, U64(acc));
	RETURN_TEST("test_arithmetic_mixed_integer", result);
}

int test_arithmetic_scale_not_area() {
	int result = 0;
	const ByteSize left{10};
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 40ull, U64(static_cast<ByteSize>(left * Size{4})));
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 40ull, U64(static_cast<ByteSize>(Size{4} * left)));
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 2ull, U64(static_cast<ByteSize>(left / Size{4})));
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 2ull, U64(static_cast<ByteSize>(left % Size{4})));
	ByteSize acc{10};
	acc = static_cast<ByteSize>(acc * Size{3});
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 30ull, U64(acc));
	acc = static_cast<ByteSize>(acc / Size{5});
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 6ull, U64(acc));
	acc = static_cast<ByteSize>(acc % Size{4});
	ASSERT_EQUAL("test_arithmetic_scale_not_area", 2ull, U64(acc));
	RETURN_TEST("test_arithmetic_scale_not_area", result);
}

int test_arithmetic_zero() {
	int result = 0;
	const ByteSize empty;
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(static_cast<ByteSize>(empty / Size{4})));
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(static_cast<ByteSize>(empty % Size{4})));
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(static_cast<ByteSize>(empty * Size{8})));
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(empty + ByteSize{0}));
	ASSERT_EQUAL("test_arithmetic_zero", 4ull, U64(ByteSize{4} + empty));
	RETURN_TEST("test_arithmetic_zero", result);
}

// -------------------
// Compare
// -------------------

int test_compare_equal_and_order() {
	int result = 0;
	const ByteSize a{10};
	const ByteSize b{10};
	const ByteSize c{11};
	ASSERT_TRUE("test_compare_equal_and_order", a == b);
	ASSERT_TRUE("test_compare_equal_and_order", a != c);
	ASSERT_TRUE("test_compare_equal_and_order", a < c);
	ASSERT_TRUE("test_compare_equal_and_order", c > a);
	ASSERT_TRUE("test_compare_equal_and_order", a <= b);
	ASSERT_TRUE("test_compare_equal_and_order", c >= a);
	ASSERT_TRUE("test_compare_equal_and_order", (a <=> b) == std::strong_ordering::equal);
	ASSERT_TRUE("test_compare_equal_and_order", (a <=> c) == std::strong_ordering::less);
	RETURN_TEST("test_compare_equal_and_order", result);
}

int test_compare_mixed_integer() {
	int result = 0;
	const ByteSize size{5};
	ASSERT_TRUE("test_compare_mixed_integer", U64(size) == 5ull);
	ASSERT_TRUE("test_compare_mixed_integer", 5ull == U64(size));
	ASSERT_TRUE("test_compare_mixed_integer", U64(size) == 5u);
	ASSERT_TRUE("test_compare_mixed_integer", size == ByteSize{5});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{5} == size);
	ASSERT_TRUE("test_compare_mixed_integer", size == ByteSize{5u});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{5u} == size);
	ASSERT_TRUE("test_compare_mixed_integer", size == ByteSize{5ull});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{5ull} == size);
	ASSERT_TRUE("test_compare_mixed_integer", U64(size) != 6ull);
	ASSERT_TRUE("test_compare_mixed_integer", 6ull != U64(size));
	ASSERT_TRUE("test_compare_mixed_integer", size != ByteSize{6});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{6} != size);
	ASSERT_TRUE("test_compare_mixed_integer", size != ByteSize{6u});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{6u} != size);
	ASSERT_TRUE("test_compare_mixed_integer", size < ByteSize{6});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{4} < size);
	ASSERT_TRUE("test_compare_mixed_integer", size < ByteSize{6u});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{4u} < size);
	ASSERT_TRUE("test_compare_mixed_integer", size > ByteSize{0});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{10} > size);
	ASSERT_TRUE("test_compare_mixed_integer", size > ByteSize{0u});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{10u} > size);
	ASSERT_TRUE("test_compare_mixed_integer", size <= ByteSize{5});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{5} <= size);
	ASSERT_TRUE("test_compare_mixed_integer", size >= ByteSize{5});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{5} >= size);
	const ByteSize empty;
	ASSERT_TRUE("test_compare_mixed_integer", empty == ByteSize{0});
	ASSERT_TRUE("test_compare_mixed_integer", ByteSize{0} == empty);
	ASSERT_TRUE("test_compare_mixed_integer", empty != ByteSize{1});
	ASSERT_TRUE("test_compare_mixed_integer", empty < ByteSize{1});
	ASSERT_TRUE("test_compare_mixed_integer", U64(empty) == 0ull);
	RETURN_TEST("test_compare_mixed_integer", result);
}

// -------------------
// Construct
// -------------------

int test_construct_assign_integer() {
	int result = 0;
	ByteSize size;
	size = 0;
	ASSERT_EQUAL("test_construct_assign_integer", 0ull, U64(size));
	size = 16;
	ASSERT_EQUAL("test_construct_assign_integer", 16ull, U64(size));
	size = 32u;
	ASSERT_EQUAL("test_construct_assign_integer", 32ull, U64(size));
	size = 64ull;
	ASSERT_EQUAL("test_construct_assign_integer", 64ull, U64(size));
	size = static_cast<short>(8);
	ASSERT_EQUAL("test_construct_assign_integer", 8ull, U64(size));
	size = static_cast<unsigned char>(3);
	ASSERT_EQUAL("test_construct_assign_integer", 3ull, U64(size));
	size = 0u;
	ASSERT_EQUAL("test_construct_assign_integer", 0ull, U64(size));
	RETURN_TEST("test_construct_assign_integer", result);
}

int test_construct_copy_move() {
	int result = 0;
	const ByteSize original{64};
	const ByteSize copied{original};
	ByteSize moved{ByteSize{32}};
	ASSERT_EQUAL("test_construct_copy_move", 64ull, U64(copied));
	ASSERT_EQUAL("test_construct_copy_move", 32ull, U64(moved));
	RETURN_TEST("test_construct_copy_move", result);
}

int test_construct_implicit_from_int() {
	int result = 0;
	const ByteSize from_zero = 0;
	ASSERT_EQUAL("test_construct_implicit_from_int", 0ull, U64(from_zero));
	const ByteSize from_int = 100;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_int));
	const ByteSize from_unsigned = 100u;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_unsigned));
	const ByteSize from_ull = 100ull;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_ull));
	const ByteSize from_long = 100L;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_long));
	const ByteSize from_char = static_cast<char>(7);
	ASSERT_EQUAL("test_construct_implicit_from_int", 7ull, U64(from_char));
	RETURN_TEST("test_construct_implicit_from_int", result);
}

int test_construct_units() {
	int result = 0;
	ASSERT_EQUAL("test_construct_units", 1024ull, U64(1 * KiB));
	ASSERT_EQUAL("test_construct_units", 2048ull, U64(2 * KiB));
	ASSERT_EQUAL("test_construct_units", 1048576ull, U64(1 * MiB));
	ASSERT_EQUAL("test_construct_units", 1073741824ull, U64(1 * GiB));
	ASSERT_TRUE("test_construct_units", (1 * KiB) == ByteSize{1024});
	ASSERT_TRUE("test_construct_units", (2 * KiB) == ByteSize{2048});
	RETURN_TEST("test_construct_units", result);
}

int test_construct_zero() {
	int result = 0;
	const ByteSize empty;
	ASSERT_EQUAL("test_construct_zero", 0ull, U64(empty));
	RETURN_TEST("test_construct_zero", result);
}

// -------------------
// Convert
// -------------------

int test_convert_cstring_wcstring_human() {
	int result = 0;
	ASSERT_EQUAL("test_convert_cstring_wcstring_human", std::string("0 B"), std::string(static_cast<Safe::String>(ByteSize{})));
	ASSERT_EQUAL("test_convert_cstring_wcstring_human", std::string("1023 B"), std::string(static_cast<Safe::String>(ByteSize{1023})));
	const ByteSize kib = 1 * KiB;
	const Safe::String owned = static_cast<Safe::String>(kib);
	const Safe::WString wide = static_cast<Safe::WString>(kib);
	ASSERT_EQUAL("test_convert_cstring_wcstring_human", std::string("1.00 KiB"), std::string(owned));
	ASSERT_TRUE("test_convert_cstring_wcstring_human", wide == L"1.00 KiB");
	const Safe::String mib = static_cast<Safe::String>((1 * MiB) + (512 * KiB));
	ASSERT_EQUAL("test_convert_cstring_wcstring_human", std::string("1.50 MiB"), std::string(mib));
	const std::string text = kib;
	ASSERT_EQUAL("test_convert_cstring_wcstring_human", std::string("1.00 KiB"), text);
	RETURN_TEST("test_convert_cstring_wcstring_human", result);
}

int test_convert_explicit_integrals() {
	int result = 0;
	const ByteSize size{42};
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<int>(42), static_cast<int>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<unsigned>(42), static_cast<unsigned>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<short>(42), static_cast<short>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<unsigned short>(42), static_cast<unsigned short>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<long>(42), static_cast<long>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<unsigned long>(42), static_cast<unsigned long>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<long long>(42), static_cast<long long>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", 42ull, static_cast<unsigned long long>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<std::ptrdiff_t>(42), static_cast<std::ptrdiff_t>(size));
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<std::uint8_t>(42), static_cast<std::uint8_t>(size));
	const ByteSize wide{300};
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<std::uint8_t>(255), static_cast<std::uint8_t>(wide));
	RETURN_TEST("test_convert_explicit_integrals", result);
}

int test_convert_implicit_size_t() {
	int result = 0;
	const ByteSize size{42};
	const std::size_t n = size;
	ASSERT_EQUAL("test_convert_implicit_size_t", static_cast<std::size_t>(42), n);
	const std::size_t empty = ByteSize{};
	ASSERT_EQUAL("test_convert_implicit_size_t", static_cast<std::size_t>(0), empty);
	RETURN_TEST("test_convert_implicit_size_t", result);
}

// -------------------
// Traits
// -------------------

int test_traits_numeral() {
	int result = 0;
	static_assert(Type::Numeral<int>);
	static_assert(Type::Numeral<std::size_t>);
	static_assert(Type::Numeral<ByteSize>);
	static_assert(Type::Numeral<const ByteSize&>);
	static_assert(Type::Numeral<Size>);
	static_assert(!Type::Numeral<double>);
	ASSERT_TRUE("test_traits_numeral", Type::Numeral<ByteSize>);
	RETURN_TEST("test_traits_numeral", result);
}

int main() {
	int result = 0;

	// -------------------
	// Arithmetic
	// -------------------
	result += test_arithmetic_add_sub();
	result += test_arithmetic_expression_to_size_t();
	result += test_arithmetic_increment();
	result += test_arithmetic_mixed_integer();
	result += test_arithmetic_scale_not_area();
	result += test_arithmetic_zero();

	// -------------------
	// Compare
	// -------------------
	result += test_compare_equal_and_order();
	result += test_compare_mixed_integer();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_assign_integer();
	result += test_construct_copy_move();
	result += test_construct_implicit_from_int();
	result += test_construct_units();
	result += test_construct_zero();

	// -------------------
	// Convert
	// -------------------
	result += test_convert_cstring_wcstring_human();
	result += test_convert_explicit_integrals();
	result += test_convert_implicit_size_t();

	// -------------------
	// Traits
	// -------------------
	result += test_traits_numeral();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
