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
#include <StormByte/exception.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/safe.hxx>

#include <expected>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	enum class Tag: int { None, Slot };
	struct Foreign {};
	struct Leaf: Exception {
		Leaf(): Exception("leaf") {}
		~Leaf() override;
	};
	struct Cursor {
		using StormByteSafeCursor = void;
	};
	struct Provider {
		int value = 0;
		Provider() = default;
		Provider(const Provider&) = default;
		Provider& operator=(const Provider&) = default;
		Provider(Provider&&) = default;
		Provider& operator=(Provider&&) = default;
	};
}

Leaf::~Leaf() = default;
STORMBYTE_DECLARE_MAYBE_SAFE(Provider);

// -------------------
// Component
// -------------------

int test_component_strips_and_rejects_stl() {
	static_assert(Type::SafeComponent<int>);
	static_assert(Type::SafeComponent<const int&>);
	static_assert(Type::SafeComponent<Tag>);
	static_assert(Type::SafeComponent<Size>);
	static_assert(Type::SafeComponent<const ByteSize&>);
	static_assert(Type::SafeComponent<Safe::String>);
	static_assert(Type::SafeComponent<Safe::WString&&>);
	static_assert(Type::SafeComponent<Safe::Binary>);
	static_assert(Type::SafeComponent<Exception>);
	static_assert(Type::SafeComponent<Safe::Exception>);
	static_assert(Type::SafeComponent<Safe::Shared<int>>);
	static_assert(Type::SafeComponent<Safe::Unique<Size>>);
	static_assert(Type::SafeComponent<Safe::Weak<Safe::String>>);
	static_assert(Type::SafeComponent<Leaf>);
	static_assert(Type::SafeComponent<const Leaf&>);
	static_assert(Type::SafeComponent<Provider>);
	static_assert(Type::SafeComponent<std::expected<int, Exception>>);
	static_assert(Type::SafeComponent<std::expected<void, Leaf>>);
	static_assert(!Type::SafeComponent<std::string>);
	static_assert(!Type::SafeComponent<std::vector<int>>);
	static_assert(!Type::SafeComponent<std::unique_ptr<int>>);
	static_assert(!Type::SafeComponent<int*>);
	static_assert(!Type::SafeComponent<Foreign>);
	static_assert(!Type::SafeComponent<std::expected<std::string, Exception>>);
	static_assert(!Type::IsSafe<int&>::value);
	static_assert(Type::IsSafe<int>::value);
	static_assert(!Type::IsSafe<Leaf>::value);
	static_assert(Type::IsMaybeSafe<Leaf>::value);
	static_assert(Type::MaybeSafe<Leaf>);
	static_assert(Type::MaybeSafe<Provider>);
	static_assert(!Type::MaybeSafe<int>);
	static_assert(!Type::MaybeSafe<Exception>);
	static_assert(!Type::MaybeSafe<std::vector<int>>);
	ASSERT_TRUE(Type::SafeComponent<const Safe::Binary&>);
	ASSERT_TRUE(Type::MaybeSafe<Leaf>);
	ASSERT_FALSE(Type::SafeComponent<std::string>);
	ASSERT_FALSE(Type::IsSafe<Leaf>::value);
	RETURN_TEST(0);
}

// -------------------
// Value
// -------------------

int test_value_and_cursor() {
	static_assert(Type::SafeValue<int>);
	static_assert(Type::SafeValue<const int>);
	static_assert(Type::SafeValue<Tag>);
	static_assert(Type::SafeValue<Size>);
	static_assert(Type::SafeValue<ByteSize>);
	static_assert(Type::SafeValue<Safe::String>);
	static_assert(Type::SafeValue<Safe::WString>);
	static_assert(Type::SafeValue<Safe::Binary>);
	static_assert(Type::SafeValue<Safe::Shared<int>>);
	static_assert(Type::SafeValue<Cursor>);
	static_assert(Type::SafeValue<const Cursor>);
	static_assert(Type::SafeValue<Provider>);
	static_assert(Type::SafeCursor<Cursor>);
	static_assert(Type::SafeCursor<const Cursor&>);
	static_assert(!Type::SafeValue<int&>);
	static_assert(!Type::SafeValue<int*>);
	static_assert(!Type::SafeValue<Safe::Unique<int>>);
	static_assert(!Type::SafeValue<Safe::Weak<int>>);
	static_assert(!Type::SafeValue<std::string>);
	static_assert(!Type::SafeValue<std::vector<int>>);
	static_assert(!Type::SafeValue<Foreign>);
	static_assert(!Type::SafeCursor<int>);
	static_assert(!Type::SafeCursor<Safe::String>);
	ASSERT_TRUE(Type::SafeValue<const Safe::String>);
	ASSERT_TRUE(Type::SafeCursor<Cursor>);
	ASSERT_FALSE(Type::SafeValue<Safe::Unique<int>>);
	ASSERT_FALSE(Type::SafeValue<int&>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Component
	// -------------------
	result += test_component_strips_and_rejects_stl();

	// -------------------
	// Value
	// -------------------
	result += test_value_and_cursor();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
