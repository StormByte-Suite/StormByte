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
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/comparison.hxx>

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	struct EqualOnly {
		int value = 0;
		bool operator==(const EqualOnly&) const = default;
	};

	struct Ordered {
		int value = 0;
		auto operator<=>(const Ordered&) const = default;
	};

	struct Silent {};

	struct Hashed {
		int value = 0;
		bool operator==(const Hashed&) const = default;
	};
}

template<>
struct std::hash<Hashed> {
	std::size_t operator()(const Hashed& value) const noexcept {
		return std::hash<int>{}(value.value);
	}
};

// -------------------
// Equality
// -------------------

int test_equality_same_type_only() {
	static_assert(Type::EqualityComparable<int>);
	static_assert(Type::EqualityComparable<const int&>);
	static_assert(Type::EqualityComparable<double>);
	static_assert(Type::EqualityComparable<std::string>);
	static_assert(Type::EqualityComparable<const std::string&>);
	static_assert(Type::EqualityComparable<std::pair<int, int>>);
	static_assert(Type::EqualityComparable<std::vector<int>>);
	static_assert(Type::EqualityComparable<Size>);
	static_assert(Type::EqualityComparable<const ByteSize&>);
	static_assert(Type::EqualityComparable<Safe::String>);
	static_assert(Type::EqualityComparable<const Safe::String&>);
	static_assert(Type::EqualityComparable<EqualOnly>);
	static_assert(Type::EqualityComparable<Ordered>);
	static_assert(Type::EqualityComparable<int*>);
	static_assert(!Type::EqualityComparable<Silent>);
	static_assert(!Type::EqualityComparable<void>);
	ASSERT_TRUE(Type::EqualityComparable<Safe::String>);
	ASSERT_TRUE((EqualOnly{1} == EqualOnly{1}));
	ASSERT_TRUE((EqualOnly{1} != EqualOnly{2}));
	ASSERT_FALSE(Type::EqualityComparable<Silent>);
	RETURN_TEST(0);
}

// -------------------
// Hash
// -------------------

int test_hash_specialization() {
	static_assert(Type::Hashable<int>);
	static_assert(Type::Hashable<const unsigned&>);
	static_assert(Type::Hashable<bool>);
	static_assert(Type::Hashable<double>);
	static_assert(Type::Hashable<std::string>);
	static_assert(Type::Hashable<const std::string&>);
	static_assert(Type::Hashable<int*>);
	static_assert(Type::Hashable<std::nullptr_t>);
	static_assert(Type::Hashable<Hashed>);
	static_assert(Type::Hashable<const Hashed&>);
	static_assert(!Type::Hashable<EqualOnly>);
	static_assert(!Type::Hashable<Ordered>);
	static_assert(!Type::Hashable<Silent>);
	static_assert(!Type::Hashable<std::pair<int, int>>);
	const Hashed value{7};
	ASSERT_EQUAL(std::hash<int>{}(7), std::hash<Hashed>{}(value));
	ASSERT_TRUE(Type::Hashable<std::string>);
	ASSERT_FALSE(Type::Hashable<Silent>);
	RETURN_TEST(0);
}

// -------------------
// Order
// -------------------

int test_order_three_way() {
	static_assert(Type::ThreeWayComparable<int>);
	static_assert(Type::ThreeWayComparable<const unsigned&>);
	static_assert(Type::ThreeWayComparable<double>);
	static_assert(Type::ThreeWayComparable<std::string>);
	static_assert(Type::ThreeWayComparable<std::pair<int, int>>);
	static_assert(Type::ThreeWayComparable<std::vector<int>>);
	static_assert(Type::ThreeWayComparable<Size>);
	static_assert(Type::ThreeWayComparable<const ByteSize&>);
	static_assert(Type::ThreeWayComparable<Safe::String>);
	static_assert(Type::ThreeWayComparable<const Safe::String&>);
	static_assert(Type::ThreeWayComparable<Ordered>);
	static_assert(Type::ThreeWayComparable<int*>);
	static_assert(!Type::ThreeWayComparable<EqualOnly>);
	static_assert(!Type::ThreeWayComparable<Silent>);
	ASSERT_TRUE((Ordered{1} <=> Ordered{2}) == std::strong_ordering::less);
	ASSERT_TRUE((Size{1} <=> Size{1}) == std::strong_ordering::equal);
	ASSERT_FALSE(Type::ThreeWayComparable<EqualOnly>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Equality
	// -------------------
	result += test_equality_same_type_only();

	// -------------------
	// Hash
	// -------------------
	result += test_hash_specialization();

	// -------------------
	// Order
	// -------------------
	result += test_order_three_way();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
