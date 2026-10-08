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

#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/wrappers.hxx>

#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace StormByte;

namespace {
	struct Named {
		int first;
		int second;
	};
	struct Silent {};
}

// -------------------
// Aggregates
// -------------------

int test_aggregates_pair_members() {
	static_assert(Type::Pair<std::pair<int, int>>);
	static_assert(Type::Pair<const std::pair<int, Safe::String>&>);
	static_assert(Type::Pair<Safe::Pair<int, int>>);
	static_assert(Type::Pair<const Safe::Pair<int, Safe::String>&>);
	static_assert(Type::Pair<Named>);
	static_assert(!Type::Pair<std::tuple<int, int>>);
	static_assert(!Type::Pair<Silent>);
	static_assert(!Type::Pair<int>);
	const Safe::Pair<int, int> pair{1, 2};
	ASSERT_EQUAL(1, pair.first);
	ASSERT_EQUAL(2, pair.second);
	ASSERT_TRUE(Type::Pair<Named>);
	ASSERT_FALSE((Type::Pair<std::tuple<int, int>>));
	RETURN_TEST(0);
}

// -------------------
// Wrappers
// -------------------

int test_wrappers_optional_queue_variant() {
	static_assert(Type::Optional<std::optional<int>>);
	static_assert(Type::Optional<std::optional<Safe::String>>);
	static_assert(Type::Optional<Safe::Optional<int>>);
	static_assert(!Type::Optional<const Safe::Optional<int>&>);
	static_assert(!Type::Optional<std::optional<int>&>);
	static_assert(!Type::Optional<int>);
	static_assert(!Type::Optional<std::vector<int>>);
	static_assert(!Type::Optional<Safe::Queue<int>>);
	static_assert(Type::Queue<std::queue<int>>);
	static_assert(Type::Queue<std::queue<int, std::vector<int>>>);
	static_assert(Type::Queue<Safe::Queue<int>>);
	static_assert(!Type::Queue<const Safe::Queue<Safe::String>&>);
	static_assert(!Type::Queue<Safe::Queue<int>&>);
	static_assert(!Type::Queue<std::vector<int>>);
	static_assert(!Type::Queue<std::optional<int>>);
	static_assert(Type::Variant<std::variant<int, Safe::String>>);
	static_assert(Type::Variant<const std::variant<int>&>);
	static_assert(Type::Variant<std::variant<int>&&>);
	static_assert(!Type::Variant<int>);
	static_assert(!Type::Variant<std::optional<int>>);
	static_assert(Type::VariantHasType<std::variant<int, Safe::String>, int>);
	static_assert(Type::VariantHasType<const std::variant<int, Safe::String>&, Safe::String>);
	static_assert(Type::VariantHasType<std::variant<const int>, int>);
	static_assert(!Type::VariantHasType<std::variant<int, Safe::String>, std::string>);
	static_assert(!Type::VariantHasType<std::variant<int>, Safe::String>);
	static_assert(!Type::VariantHasType<int, int>);
	ASSERT_TRUE(Type::Optional<Safe::Optional<int>>);
	ASSERT_TRUE(Type::Queue<Safe::Queue<int>>);
	ASSERT_TRUE((Type::VariantHasType<std::variant<int, Safe::String>, const Safe::String&>));
	ASSERT_FALSE(Type::Optional<Safe::Queue<int>>);
	ASSERT_FALSE((Type::VariantHasType<std::variant<int>, Safe::String>));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Aggregates
	// -------------------
	result += test_aggregates_pair_members();

	// -------------------
	// Wrappers
	// -------------------
	result += test_wrappers_optional_queue_variant();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
