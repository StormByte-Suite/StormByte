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

#include <StormByte/size.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <cstdint>
#include <iostream>
#include <string>

using namespace StormByte;

namespace {
	std::uint64_t U64(Size size) {
		return static_cast<std::uint64_t>(size);
	}
}

// -------------------
// Arithmetic
// -------------------

int test_arithmetic_add_sub() {
	int result = 0;
	const Size left{100};
	const Size right{40};
	ASSERT_EQUAL("test_arithmetic_add_sub", 140ull, U64(left + right));
	ASSERT_EQUAL("test_arithmetic_add_sub", 60ull, U64(left - right));
	Size acc{10};
	acc += Size{5};
	ASSERT_EQUAL("test_arithmetic_add_sub", 15ull, U64(acc));
	acc -= Size{3};
	ASSERT_EQUAL("test_arithmetic_add_sub", 12ull, U64(acc));
	RETURN_TEST("test_arithmetic_add_sub", result);
}

int test_arithmetic_expression_to_size_t() {
	int result = 0;
	const Size size1{10};
	const Size size2{4};
	const std::size_t calc = size1 + 3 * size2;
	ASSERT_EQUAL("test_arithmetic_expression_to_size_t", static_cast<std::size_t>(22), calc);
	RETURN_TEST("test_arithmetic_expression_to_size_t", result);
}

int test_arithmetic_increment() {
	int result = 0;
	Size size{10};
	++size;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(size));
	const Size post = size++;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(post));
	ASSERT_EQUAL("test_arithmetic_increment", 12ull, U64(size));
	--size;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(size));
	const Size posted = size--;
	ASSERT_EQUAL("test_arithmetic_increment", 11ull, U64(posted));
	ASSERT_EQUAL("test_arithmetic_increment", 10ull, U64(size));
	RETURN_TEST("test_arithmetic_increment", result);
}

int test_arithmetic_mixed_integer() {
	int result = 0;
	const Size size{10};
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
	Size acc = 10;
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
	acc *= 2;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 34ull, U64(acc));
	acc /= 2;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 17ull, U64(acc));
	acc %= 5;
	ASSERT_EQUAL("test_arithmetic_mixed_integer", 2ull, U64(acc));
	RETURN_TEST("test_arithmetic_mixed_integer", result);
}

int test_arithmetic_mul_div_mod() {
	int result = 0;
	const Size left{10};
	const Size right{4};
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 40ull, U64(left * right));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 40ull, U64(left * 4));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 40ull, U64(4 * left));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(left / right));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(left / 4));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(10 / right));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(left % right));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(left % 4));
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(10 % right));
	Size acc{10};
	acc *= Size{3};
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 30ull, U64(acc));
	acc /= Size{5};
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 6ull, U64(acc));
	acc %= Size{4};
	ASSERT_EQUAL("test_arithmetic_mul_div_mod", 2ull, U64(acc));
	RETURN_TEST("test_arithmetic_mul_div_mod", result);
}

int test_arithmetic_zero() {
	int result = 0;
	const Size empty;
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(empty / 4));
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(empty % 4));
	ASSERT_EQUAL("test_arithmetic_zero", 0ull, U64(empty * 8));
	RETURN_TEST("test_arithmetic_zero", result);
}

// -------------------
// Compare
// -------------------

int test_compare_equal_and_order() {
	int result = 0;
	const Size a{10};
	const Size b{10};
	const Size c{11};
	ASSERT_TRUE("test_compare_equal_and_order", a == b);
	ASSERT_TRUE("test_compare_equal_and_order", a != c);
	ASSERT_TRUE("test_compare_equal_and_order", a < c);
	ASSERT_TRUE("test_compare_equal_and_order", c > a);
	ASSERT_TRUE("test_compare_equal_and_order", a <= b);
	ASSERT_TRUE("test_compare_equal_and_order", c >= a);
	RETURN_TEST("test_compare_equal_and_order", result);
}

int test_compare_mixed_integer() {
	int result = 0;
	const Size size{5};
	ASSERT_TRUE("test_compare_mixed_integer", size == 5);
	ASSERT_TRUE("test_compare_mixed_integer", 5 == size);
	ASSERT_TRUE("test_compare_mixed_integer", size == 5u);
	ASSERT_TRUE("test_compare_mixed_integer", 5u == size);
	ASSERT_TRUE("test_compare_mixed_integer", size == 5ull);
	ASSERT_TRUE("test_compare_mixed_integer", 5ull == size);
	ASSERT_TRUE("test_compare_mixed_integer", size != 6);
	ASSERT_TRUE("test_compare_mixed_integer", 6 != size);
	ASSERT_TRUE("test_compare_mixed_integer", size != 6u);
	ASSERT_TRUE("test_compare_mixed_integer", 6u != size);
	ASSERT_TRUE("test_compare_mixed_integer", size < 6);
	ASSERT_TRUE("test_compare_mixed_integer", 4 < size);
	ASSERT_TRUE("test_compare_mixed_integer", size < 6u);
	ASSERT_TRUE("test_compare_mixed_integer", 4u < size);
	ASSERT_TRUE("test_compare_mixed_integer", size > 0);
	ASSERT_TRUE("test_compare_mixed_integer", 10 > size);
	ASSERT_TRUE("test_compare_mixed_integer", size > 0u);
	ASSERT_TRUE("test_compare_mixed_integer", 10u > size);
	ASSERT_TRUE("test_compare_mixed_integer", size <= 5);
	ASSERT_TRUE("test_compare_mixed_integer", 5 <= size);
	ASSERT_TRUE("test_compare_mixed_integer", size >= 5);
	ASSERT_TRUE("test_compare_mixed_integer", 5 >= size);
	const Size empty;
	ASSERT_TRUE("test_compare_mixed_integer", empty == 0);
	ASSERT_TRUE("test_compare_mixed_integer", 0 == empty);
	ASSERT_TRUE("test_compare_mixed_integer", empty != 1);
	ASSERT_TRUE("test_compare_mixed_integer", empty < 1);
	RETURN_TEST("test_compare_mixed_integer", result);
}

// -------------------
// Construct
// -------------------

int test_construct_assign_integer() {
	int result = 0;
	Size size;
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
	const Size original{64};
	const Size copied{original};
	Size moved{Size{32}};
	ASSERT_EQUAL("test_construct_copy_move", 64ull, U64(copied));
	ASSERT_EQUAL("test_construct_copy_move", 32ull, U64(moved));
	RETURN_TEST("test_construct_copy_move", result);
}

int test_construct_implicit_from_int() {
	int result = 0;
	const Size from_zero = 0;
	ASSERT_EQUAL("test_construct_implicit_from_int", 0ull, U64(from_zero));
	const Size from_int = 100;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_int));
	const Size from_unsigned = 100u;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_unsigned));
	const Size from_ull = 100ull;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_ull));
	const Size from_long = 100L;
	ASSERT_EQUAL("test_construct_implicit_from_int", 100ull, U64(from_long));
	const Size from_char = static_cast<char>(7);
	ASSERT_EQUAL("test_construct_implicit_from_int", 7ull, U64(from_char));
	RETURN_TEST("test_construct_implicit_from_int", result);
}

int test_construct_zero() {
	int result = 0;
	const Size empty;
	ASSERT_EQUAL("test_construct_zero", 0ull, U64(empty));
	RETURN_TEST("test_construct_zero", result);
}

// -------------------
// Convert
// -------------------

int test_convert_cstring_wcstring_decimal() {
	int result = 0;
	const Size size{1024};
	const Safe::String owned = static_cast<Safe::String>(size);
	const Safe::WString wide = static_cast<Safe::WString>(size);
	const std::string text = size;
	ASSERT_EQUAL("test_convert_cstring_wcstring_decimal", "1024", std::string(owned));
	ASSERT_EQUAL("test_convert_cstring_wcstring_decimal", "1024", text);
	ASSERT_TRUE("test_convert_cstring_wcstring_decimal", wide == L"1024");
	ASSERT_EQUAL("test_convert_cstring_wcstring_decimal", "0", std::string(Size{}));
	RETURN_TEST("test_convert_cstring_wcstring_decimal", result);
}

int test_convert_explicit_integrals() {
	int result = 0;
	const Size size{42};
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
	const Size wide{300};
	ASSERT_EQUAL("test_convert_explicit_integrals", static_cast<std::uint8_t>(255), static_cast<std::uint8_t>(wide));
	RETURN_TEST("test_convert_explicit_integrals", result);
}

int test_convert_implicit_size_t() {
	int result = 0;
	const Size size{42};
	const std::size_t n = size;
	ASSERT_EQUAL("test_convert_implicit_size_t", static_cast<std::size_t>(42), n);
	const std::size_t empty = Size{};
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
	static_assert(Type::Numeral<Size>);
	static_assert(Type::Numeral<const Size&>);
	static_assert(!Type::Numeral<double>);
	ASSERT_TRUE("test_traits_numeral", Type::Numeral<Size>);
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
	result += test_arithmetic_mul_div_mod();
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
	result += test_construct_zero();

	// -------------------
	// Convert
	// -------------------
	result += test_convert_cstring_wcstring_decimal();
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
