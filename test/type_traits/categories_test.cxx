/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This is the base for StormByte Suite libraries.
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
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/categories.hxx>

#include <cstdint>
#include <iostream>
#include <memory>

using namespace StormByte;

namespace {
	struct Probe {
		int value = 0;
	};

	union Word {
		int n;
	};

	enum class Color { Red };
	enum Plain { A };

	struct Handle {
		Probe object;
		Probe* get() const noexcept { return const_cast<Probe*>(&object); }
		Probe& operator*() const noexcept { return const_cast<Probe&>(object); }
		Probe* operator->() const noexcept { return const_cast<Probe*>(&object); }
	};

	struct NullableHandle: Handle {
		explicit operator bool() const noexcept { return true; }
	};

	struct BoolOnly {
		explicit operator bool() const noexcept { return true; }
	};
}

// -------------------
// Arithmetic
// -------------------

int test_arithmetic_integral_and_float() {
	static_assert(Type::Integral<bool>);
	static_assert(Type::Integral<char>);
	static_assert(Type::Integral<signed char>);
	static_assert(Type::Integral<unsigned char>);
	static_assert(Type::Integral<wchar_t>);
	static_assert(Type::Integral<char8_t>);
	static_assert(Type::Integral<char16_t>);
	static_assert(Type::Integral<char32_t>);
	static_assert(Type::Integral<short>);
	static_assert(Type::Integral<unsigned short>);
	static_assert(Type::Integral<int>);
	static_assert(Type::Integral<const unsigned>);
	static_assert(Type::Integral<volatile long>);
	static_assert(Type::Integral<long long>);
	static_assert(Type::Integral<unsigned long long>);
	static_assert(Type::Integral<std::size_t>);
	static_assert(Type::Integral<std::ptrdiff_t>);
	static_assert(Type::Integral<std::uint8_t>);
	static_assert(Type::Integral<std::int64_t>);
	static_assert(!Type::Integral<int&>);
	static_assert(!Type::Integral<int&&>);
	static_assert(!Type::Integral<const int&>);
	static_assert(!Type::Integral<int*>);
	static_assert(!Type::Integral<double>);
	static_assert(!Type::Integral<Size>);
	static_assert(!Type::Integral<ByteSize>);
	static_assert(!Type::Integral<Color>);
	static_assert(!Type::Integral<Plain>);
	static_assert(!Type::Integral<Safe::String>);
	static_assert(Type::FloatingPoint<float>);
	static_assert(Type::FloatingPoint<const double>);
	static_assert(Type::FloatingPoint<volatile long double>);
	static_assert(!Type::FloatingPoint<float&>);
	static_assert(!Type::FloatingPoint<float&&>);
	static_assert(!Type::FloatingPoint<int>);
	static_assert(!Type::FloatingPoint<Size>);
	static_assert(!Type::FloatingPoint<ByteSize>);
	static_assert(Type::Arithmetic<bool>);
	static_assert(Type::Arithmetic<int>);
	static_assert(Type::Arithmetic<const double>);
	static_assert(!Type::Arithmetic<int&>);
	static_assert(!Type::Arithmetic<int*>);
	static_assert(!Type::Arithmetic<Size>);
	static_assert(!Type::Arithmetic<ByteSize>);
	static_assert(!Type::Arithmetic<Color>);
	static_assert(!Type::Arithmetic<Safe::Binary>);
	static_assert(!Type::Arithmetic<Safe::Vector<int>>);
	ASSERT_TRUE(Type::Integral<unsigned long>);
	ASSERT_FALSE(Type::Arithmetic<Size>);
	ASSERT_FALSE(Type::FloatingPoint<ByteSize>);
	RETURN_TEST(0);
}

int test_arithmetic_numeral() {
	static_assert(Type::Numeral<bool>);
	static_assert(Type::Numeral<char>);
	static_assert(Type::Numeral<int>);
	static_assert(Type::Numeral<const unsigned&>);
	static_assert(Type::Numeral<volatile std::size_t>);
	static_assert(Type::Numeral<std::uint64_t>);
	static_assert(Type::Numeral<Size>);
	static_assert(Type::Numeral<const Size&>);
	static_assert(Type::Numeral<Size&&>);
	static_assert(Type::Numeral<ByteSize>);
	static_assert(Type::Numeral<const ByteSize&>);
	static_assert(Type::Numeral<ByteSize&&>);
	static_assert(Type::Numeral<const volatile ByteSize&>);
	static_assert(!Type::Numeral<float>);
	static_assert(!Type::Numeral<double>);
	static_assert(!Type::Numeral<long double>);
	static_assert(!Type::Numeral<int*>);
	static_assert(!Type::Numeral<Color>);
	static_assert(!Type::Numeral<Safe::String>);
	static_assert(!Type::Numeral<Safe::Binary>);
	static_assert(!Type::Numeral<Safe::Vector<int>>);
	ASSERT_TRUE(Type::Numeral<const ByteSize&>);
	ASSERT_FALSE(Type::Numeral<Safe::String>);
	RETURN_TEST(0);
}

int test_arithmetic_signed_unsigned() {
	static_assert(Type::Signed<signed char>);
	static_assert(Type::Signed<short>);
	static_assert(Type::Signed<int>);
	static_assert(Type::Signed<long>);
	static_assert(Type::Signed<long long>);
	static_assert(Type::Signed<float>);
	static_assert(Type::Signed<double>);
	static_assert(Type::Signed<long double>);
	static_assert(!Type::Signed<bool>);
	static_assert(!Type::Signed<unsigned char>);
	static_assert(!Type::Signed<unsigned>);
	static_assert(!Type::Signed<std::size_t>);
	static_assert(!Type::Signed<Size>);
	static_assert(!Type::Signed<ByteSize>);
	static_assert(Type::Unsigned<bool>);
	static_assert(Type::Unsigned<unsigned char>);
	static_assert(Type::Unsigned<char8_t>);
	static_assert(Type::Unsigned<unsigned short>);
	static_assert(Type::Unsigned<unsigned>);
	static_assert(Type::Unsigned<unsigned long>);
	static_assert(Type::Unsigned<unsigned long long>);
	static_assert(Type::Unsigned<std::size_t>);
	static_assert(!Type::Unsigned<int>);
	static_assert(!Type::Unsigned<double>);
	static_assert(!Type::Unsigned<Size>);
	static_assert(!Type::Unsigned<ByteSize>);
	ASSERT_TRUE(Type::Signed<double>);
	ASSERT_TRUE(Type::Unsigned<bool>);
	ASSERT_FALSE(Type::Signed<Size>);
	RETURN_TEST(0);
}

// -------------------
// Pointers
// -------------------

int test_pointers_nullable() {
	static_assert(Type::NullablePointer<std::unique_ptr<int>>);
	static_assert(Type::NullablePointer<const std::unique_ptr<Probe>&>);
	static_assert(Type::NullablePointer<std::shared_ptr<int>>);
	static_assert(Type::NullablePointer<volatile std::shared_ptr<Probe>&&>);
	static_assert(Type::NullablePointer<NullableHandle>);
	static_assert(Type::NullablePointer<const NullableHandle&>);
	static_assert(!Type::NullablePointer<Handle>);
	static_assert(!Type::NullablePointer<BoolOnly>);
	static_assert(!Type::NullablePointer<int*>);
	static_assert(!Type::NullablePointer<std::weak_ptr<int>>);
	static_assert(!Type::NullablePointer<Safe::String>);
	static_assert(!Type::NullablePointer<Safe::Vector<int>>);
	ASSERT_TRUE(Type::NullablePointer<std::shared_ptr<Safe::String>>);
	ASSERT_FALSE(Type::NullablePointer<Safe::Binary>);
	RETURN_TEST(0);
}

int test_pointers_raw_and_smart() {
	static_assert(Type::Pointer<int*>);
	static_assert(Type::Pointer<const int*>);
	static_assert(Type::Pointer<int* const>);
	static_assert(Type::Pointer<const volatile Probe*>);
	static_assert(Type::Pointer<int**>);
	static_assert(Type::Pointer<void*>);
	static_assert(Type::Pointer<void (*)()>);
	static_assert(Type::Pointer<Safe::String*>);
	static_assert(!Type::Pointer<int>);
	static_assert(!Type::Pointer<int&>);
	static_assert(!Type::Pointer<int*&>);
	static_assert(!Type::Pointer<std::nullptr_t>);
	static_assert(!Type::Pointer<int Probe::*>);
	static_assert(!Type::Pointer<std::unique_ptr<int>>);
	static_assert(!Type::Pointer<std::shared_ptr<int>>);
	static_assert(!Type::Pointer<Safe::String>);
	static_assert(Type::SmartPointer<std::unique_ptr<int>>);
	static_assert(Type::SmartPointer<std::shared_ptr<Probe>>);
	static_assert(Type::SmartPointer<const Handle&>);
	static_assert(Type::SmartPointer<NullableHandle>);
	static_assert(Type::SmartPointer<std::unique_ptr<Safe::Binary>>);
	static_assert(!Type::SmartPointer<int*>);
	static_assert(!Type::SmartPointer<std::weak_ptr<int>>);
	static_assert(!Type::SmartPointer<Safe::String>);
	static_assert(!Type::SmartPointer<Safe::Vector<int>>);
	static_assert(!Type::SmartPointer<BoolOnly>);
	ASSERT_TRUE(Type::Pointer<Safe::String*>);
	ASSERT_TRUE(Type::SmartPointer<std::shared_ptr<Size>>);
	ASSERT_FALSE(Type::SmartPointer<Safe::Vector<int>>);
	RETURN_TEST(0);
}

// -------------------
// Qualifiers
// -------------------

int test_qualifiers_class_and_const() {
	static_assert(Type::Class<Probe>);
	static_assert(Type::Class<const Size>);
	static_assert(Type::Class<volatile ByteSize>);
	static_assert(Type::Class<Safe::String>);
	static_assert(Type::Class<const Safe::Binary>);
	static_assert(Type::Class<Safe::Vector<int>>);
	static_assert(!Type::Class<Size&>);
	static_assert(!Type::Class<Safe::String&&>);
	static_assert(!Type::Class<int>);
	static_assert(!Type::Class<int*>);
	static_assert(!Type::Class<Color>);
	static_assert(!Type::Class<Plain>);
	static_assert(!Type::Class<Word>);
	static_assert(Type::Const<const int>);
	static_assert(Type::Const<const volatile Probe>);
	static_assert(Type::Const<int* const>);
	static_assert(Type::Const<const Safe::String>);
	static_assert(Type::Const<Safe::Vector<int>* const>);
	static_assert(!Type::Const<int>);
	static_assert(!Type::Const<const int&>);
	static_assert(!Type::Const<const Safe::String&>);
	static_assert(!Type::Const<const int*>);
	ASSERT_TRUE(Type::Class<Safe::Binary>);
	ASSERT_FALSE(Type::Class<Word>);
	ASSERT_FALSE(Type::Const<const ByteSize&>);
	RETURN_TEST(0);
}

// -------------------
// References
// -------------------

int test_references_lvalue_and_rvalue() {
	static_assert(Type::Reference<int&>);
	static_assert(Type::Reference<const int&>);
	static_assert(Type::Reference<volatile Probe&>);
	static_assert(Type::Reference<int&&>);
	static_assert(Type::Reference<const int&&>);
	static_assert(Type::Reference<int*&>);
	static_assert(Type::Reference<Safe::String&>);
	static_assert(Type::Reference<const Safe::Binary&>);
	static_assert(Type::Reference<Safe::Vector<int>&&>);
	static_assert(!Type::Reference<int>);
	static_assert(!Type::Reference<const int>);
	static_assert(!Type::Reference<int*>);
	static_assert(!Type::Reference<Safe::String>);
	static_assert(!Type::Reference<const Safe::Binary>);
	static_assert(Type::LvalueReference<int&>);
	static_assert(Type::LvalueReference<const int&>);
	static_assert(Type::LvalueReference<const volatile Probe&>);
	static_assert(Type::LvalueReference<Size&>);
	static_assert(Type::LvalueReference<const Safe::String&>);
	static_assert(!Type::LvalueReference<int>);
	static_assert(!Type::LvalueReference<const Safe::String>);
	static_assert(!Type::LvalueReference<int&&>);
	static_assert(!Type::LvalueReference<Safe::Binary&&>);
	static_assert(Type::RvalueReference<int&&>);
	static_assert(Type::RvalueReference<const int&&>);
	static_assert(Type::RvalueReference<ByteSize&&>);
	static_assert(Type::RvalueReference<Safe::Vector<int>&&>);
	static_assert(!Type::RvalueReference<int>);
	static_assert(!Type::RvalueReference<int&>);
	static_assert(!Type::RvalueReference<const Safe::String&>);
	static_assert(!Type::RvalueReference<int*>);
	ASSERT_TRUE(Type::LvalueReference<Safe::Binary&>);
	ASSERT_TRUE(Type::RvalueReference<Safe::String&&>);
	ASSERT_FALSE(Type::RvalueReference<const Safe::Vector<int>&>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Arithmetic
	// -------------------
	result += test_arithmetic_integral_and_float();
	result += test_arithmetic_numeral();
	result += test_arithmetic_signed_unsigned();

	// -------------------
	// Pointers
	// -------------------
	result += test_pointers_nullable();
	result += test_pointers_raw_and_smart();

	// -------------------
	// Qualifiers
	// -------------------
	result += test_qualifiers_class_and_const();

	// -------------------
	// References
	// -------------------
	result += test_references_lvalue_and_rvalue();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
