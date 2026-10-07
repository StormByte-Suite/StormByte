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

#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/enums.hxx>

#include <cstdint>
#include <iostream>
#include <type_traits>

using namespace StormByte;

namespace {
	enum class UnsignedTag: std::uint16_t { None = 0, Slot = 3 };
	enum class SignedTag: std::int32_t { Minus = -4, Plus = 9 };
	enum Plain: std::uint8_t { Zero, One };
	enum SignedPlain: int { Down = -1, Up = 2 };
}

// -------------------
// Concepts
// -------------------

int test_concepts_enum_and_scope() {
	static_assert(Type::Enum<UnsignedTag>);
	static_assert(Type::Enum<const SignedTag>);
	static_assert(Type::Enum<volatile Plain>);
	static_assert(Type::Enum<SignedPlain>);
	static_assert(!Type::Enum<UnsignedTag&>);
	static_assert(!Type::Enum<int>);
	static_assert(!Type::Enum<Size>);
	static_assert(!Type::Enum<Safe::String>);
	static_assert(Type::ScopedEnum<UnsignedTag>);
	static_assert(Type::ScopedEnum<const SignedTag>);
	static_assert(!Type::ScopedEnum<Plain>);
	static_assert(!Type::ScopedEnum<SignedPlain>);
	static_assert(!Type::ScopedEnum<int>);
	static_assert(!Type::ScopedEnum<UnsignedTag&>);
	static_assert(Type::UnsignedEnum<UnsignedTag>);
	static_assert(Type::UnsignedEnum<const Plain>);
	static_assert(!Type::UnsignedEnum<SignedTag>);
	static_assert(!Type::UnsignedEnum<SignedPlain>);
	static_assert(!Type::UnsignedEnum<int>);
	static_assert(!Type::UnsignedEnum<Size>);
	ASSERT_TRUE(Type::Enum<UnsignedTag>);
	ASSERT_TRUE(Type::ScopedEnum<SignedTag>);
	ASSERT_FALSE(Type::ScopedEnum<Plain>);
	ASSERT_TRUE(Type::UnsignedEnum<Plain>);
	ASSERT_FALSE(Type::UnsignedEnum<SignedTag>);
	RETURN_TEST(0);
}

// -------------------
// Helpers
// -------------------

int test_helpers_underlying() {
	static_assert(std::is_same_v<Type::UnderlyingType<UnsignedTag>, std::uint16_t>);
	static_assert(std::is_same_v<Type::UnderlyingType<const SignedTag>, std::int32_t>);
	static_assert(std::is_same_v<Type::UnderlyingType<Plain>, std::uint8_t>);
	static_assert(Type::ToUnderlying(UnsignedTag::Slot) == std::uint16_t{3});
	static_assert(Type::ToUnderlying(SignedTag::Minus) == std::int32_t{-4});
	static_assert(noexcept(Type::ToUnderlying(UnsignedTag::None)));
	ASSERT_EQUAL(std::uint16_t{0}, Type::ToUnderlying(UnsignedTag::None));
	ASSERT_EQUAL(std::uint16_t{3}, Type::ToUnderlying(UnsignedTag::Slot));
	ASSERT_EQUAL(std::int32_t{9}, Type::ToUnderlying(SignedTag::Plus));
	ASSERT_EQUAL(std::uint8_t{1}, Type::ToUnderlying(One));
	ASSERT_EQUAL(-1, Type::ToUnderlying(Down));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Concepts
	// -------------------
	result += test_concepts_enum_and_scope();

	// -------------------
	// Helpers
	// -------------------
	result += test_helpers_underlying();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
