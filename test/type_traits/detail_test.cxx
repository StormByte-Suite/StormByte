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

#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/detail.hxx>

#include <cstdint>
#include <cstring>
#include <iostream>

using namespace StormByte;

namespace {
	enum class Tag: std::uint16_t { Value = 0x0102 };

	struct PairBytes {
		std::uint8_t first;
		std::uint8_t second;
	};
}

// -------------------
// Endian
// -------------------

int test_endian_integral_byteswap() {
	static_assert(Type::Detail::swap_endian(std::uint8_t{0xAB}) == std::uint8_t{0xAB});
	static_assert(Type::Detail::swap_endian(std::uint16_t{0x0102}) == std::uint16_t{0x0201});
	static_assert(Type::Detail::swap_endian(std::uint32_t{0x01020304}) == std::uint32_t{0x04030201});
	static_assert(Type::Detail::swap_endian(std::uint64_t{0x0102030405060708}) == std::uint64_t{0x0807060504030201});
	static_assert(noexcept(Type::Detail::swap_endian(std::uint16_t{1})));
	ASSERT_EQUAL(std::uint16_t{0x0201}, Type::Detail::swap_endian(std::uint16_t{0x0102}));
	ASSERT_EQUAL(std::int32_t{0x04030201}, Type::Detail::swap_endian(std::int32_t{0x01020304}));
	RETURN_TEST(0);
}

int test_endian_roundtrip() {
	const auto once = Type::Detail::swap_endian(std::uint32_t{0xA1B2C3D4});
	const auto twice = Type::Detail::swap_endian(once);
	ASSERT_EQUAL(std::uint32_t{0xD4C3B2A1}, once);
	ASSERT_EQUAL(std::uint32_t{0xA1B2C3D4}, twice);
	const float original = 1.0f;
	const float swapped = Type::Detail::swap_endian(original);
	const float restored = Type::Detail::swap_endian(swapped);
	ASSERT_EQUAL(original, restored);
	ASSERT_TRUE(swapped != original);
	RETURN_TEST(0);
}

int test_endian_trivial_not_integral() {
	const Tag tag = Tag::Value;
	const Tag swapped = Type::Detail::swap_endian(tag);
	ASSERT_EQUAL(std::uint16_t{0x0201}, static_cast<std::uint16_t>(swapped));
	ASSERT_EQUAL(std::uint16_t{0x0102}, static_cast<std::uint16_t>(Type::Detail::swap_endian(swapped)));
	const PairBytes pair{0x11, 0x22};
	const PairBytes reversed = Type::Detail::swap_endian(pair);
	ASSERT_EQUAL(std::uint8_t{0x22}, reversed.first);
	ASSERT_EQUAL(std::uint8_t{0x11}, reversed.second);
	ASSERT_EQUAL(std::uint8_t{0x11}, Type::Detail::swap_endian(reversed).first);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Endian
	// -------------------
	result += test_endian_integral_byteswap();
	result += test_endian_roundtrip();
	result += test_endian_trivial_not_integral();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
