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
#include <StormByte/type_traits/conversions.hxx>

#include <iostream>
#include <memory>

using namespace StormByte;

namespace {
	struct ExplicitOnly {
		explicit operator int() const noexcept { return 7; }
	};

	struct ImplicitOnly {
		operator int() const noexcept { return 3; }
	};

	struct Silent {};
	struct Base {};
	struct Derived: Base {};
	struct PrivateDerived: private Base {};
}

// -------------------
// Explicit
// -------------------

int test_explicit_includes_implicit() {
	static_assert(Type::ExplicitlyConvertibleTo<int, int>);
	static_assert(Type::ExplicitlyConvertibleTo<int, long>);
	static_assert(Type::ExplicitlyConvertibleTo<int, double>);
	static_assert(Type::ExplicitlyConvertibleTo<int, bool>);
	static_assert(Type::ExplicitlyConvertibleTo<double, int>);
	static_assert(Type::ExplicitlyConvertibleTo<Size, std::size_t>);
	static_assert(Type::ExplicitlyConvertibleTo<const Size&, std::size_t>);
	static_assert(Type::ExplicitlyConvertibleTo<Size, int>);
	static_assert(Type::ExplicitlyConvertibleTo<Size, ByteSize>);
	static_assert(Type::ExplicitlyConvertibleTo<ByteSize, unsigned>);
	static_assert(Type::ExplicitlyConvertibleTo<const ByteSize&, int>);
	static_assert(Type::ExplicitlyConvertibleTo<Safe::String, std::string>);
	static_assert(Type::ExplicitlyConvertibleTo<ExplicitOnly, int>);
	static_assert(Type::ExplicitlyConvertibleTo<const ExplicitOnly&, int>);
	static_assert(Type::ExplicitlyConvertibleTo<ImplicitOnly, int>);
	static_assert(Type::ExplicitlyConvertibleTo<Derived&, Base&>);
	static_assert(Type::ExplicitlyConvertibleTo<Base&, Derived&>);
	static_assert(Type::ExplicitlyConvertibleTo<int*, const int*>);
	static_assert(Type::ExplicitlyConvertibleTo<int*, void*>);
	static_assert(Type::ExplicitlyConvertibleTo<std::unique_ptr<int>, bool>);
	static_assert(!Type::ExplicitlyConvertibleTo<Silent, int>);
	static_assert(!Type::ExplicitlyConvertibleTo<int*, int>);
	static_assert(!Type::ExplicitlyConvertibleTo<void, int>);
	const ExplicitOnly value;
	ASSERT_EQUAL(7, static_cast<int>(value));
	ASSERT_EQUAL(4, static_cast<int>(Size{4}));
	ASSERT_EQUAL(ByteSize{4}, static_cast<ByteSize>(Size{4}));
	ASSERT_TRUE((Type::ExplicitlyConvertibleTo<ByteSize, int>));
	ASSERT_FALSE((Type::ExplicitlyConvertibleTo<Silent, int>));
	RETURN_TEST(0);
}

// -------------------
// Implicit
// -------------------

int test_implicit_is_convertible_only() {
	static_assert(Type::ConvertibleTo<int, int>);
	static_assert(Type::ConvertibleTo<int, long>);
	static_assert(Type::ConvertibleTo<int, double>);
	static_assert(Type::ConvertibleTo<int, bool>);
	static_assert(Type::ConvertibleTo<Size, std::size_t>);
	static_assert(Type::ConvertibleTo<Size, int>);
	static_assert(Type::ConvertibleTo<const ByteSize&, std::size_t>);
	static_assert(Type::ConvertibleTo<ByteSize, int>);
	static_assert(Type::ConvertibleTo<Safe::String, std::string_view>);
	static_assert(Type::ConvertibleTo<const Safe::String&, std::string_view>);
	static_assert(Type::ConvertibleTo<ImplicitOnly, int>);
	static_assert(Type::ConvertibleTo<Derived*, Base*>);
	static_assert(Type::ConvertibleTo<int*, const int*>);
	static_assert(Type::ConvertibleTo<int*, void*>);
	static_assert(!Type::ConvertibleTo<ExplicitOnly, int>);
	static_assert(!Type::ConvertibleTo<Size, ByteSize>);
	static_assert(!Type::ConvertibleTo<ByteSize, Size>);
	static_assert(!Type::ConvertibleTo<Safe::String, std::string>);
	static_assert(!Type::ConvertibleTo<Silent, int>);
	static_assert(!Type::ConvertibleTo<int*, int>);
	static_assert(!Type::ConvertibleTo<Base*, Derived*>);
	static_assert(!Type::ConvertibleTo<PrivateDerived*, Base*>);
	static_assert(!Type::ConvertibleTo<std::unique_ptr<int>, bool>);
	const Size size{4};
	const std::size_t n = size;
	const int as_int = size;
	const Safe::String text("ab");
	const std::string_view view = text;
	const int from_implicit = ImplicitOnly{};
	ASSERT_EQUAL(static_cast<std::size_t>(4), n);
	ASSERT_EQUAL(4, as_int);
	ASSERT_EQUAL(std::size_t{2}, view.size());
	ASSERT_EQUAL(3, from_implicit);
	ASSERT_FALSE((Type::ConvertibleTo<Size, ByteSize>));
	ASSERT_FALSE((Type::ConvertibleTo<Safe::String, std::string>));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Explicit
	// -------------------
	result += test_explicit_includes_implicit();

	// -------------------
	// Implicit
	// -------------------
	result += test_implicit_is_convertible_only();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
