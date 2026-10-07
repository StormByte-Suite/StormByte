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

#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

using namespace StormByte;

// -------------------
// Construct
// -------------------

int test_construct_convert_and_get() {
	Safe::Pair<int, int> empty;
	ASSERT_EQUAL(0, empty.first);
	ASSERT_EQUAL(0, empty.second);
	Safe::Pair<int, Safe::String> pair(3, Safe::String("ab"));
	ASSERT_EQUAL(3, pair.first);
	ASSERT_EQUAL(std::size_t{2}, pair.second.size());
	std::pair<int, int> standard(1, 2);
	Safe::Pair<int, int> from_std(standard);
	ASSERT_EQUAL(1, from_std.first);
	ASSERT_EQUAL(2, from_std.second);
	Safe::Pair<int, int> piecewise(std::piecewise_construct, std::tuple<int>(8), std::tuple<int>(9));
	ASSERT_EQUAL(8, Safe::get<0>(piecewise));
	ASSERT_EQUAL(9, Safe::get<1>(piecewise));
	Safe::Pair<int, int> copy(from_std);
	Safe::Pair<int, int> moved(std::move(copy));
	ASSERT_EQUAL(1, moved.first);
	ASSERT_EQUAL(2, moved.second);
	moved = standard;
	ASSERT_EQUAL(1, moved.first);
	moved = std::pair<int, int>(4, 5);
	ASSERT_EQUAL(5, moved.second);
	RETURN_TEST(0);
}

// -------------------
// Order
// -------------------

int test_order_swap_and_algorithm() {
	Safe::Pair<int, int> left(1, 2);
	Safe::Pair<int, int> right(1, 3);
	Safe::Pair<int, int> same(1, 2);
	ASSERT_TRUE(left == same);
	ASSERT_TRUE(left != right);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE(right > left);
	ASSERT_TRUE(left <= same);
	ASSERT_TRUE(right >= left);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::less);
	left.swap(right);
	ASSERT_EQUAL(3, left.second);
	ASSERT_EQUAL(2, right.second);
	swap(left, right);
	ASSERT_EQUAL(2, left.second);
	std::vector<Safe::Pair<int, int>> values;
	values.emplace_back(2, 1);
	values.emplace_back(1, 9);
	values.emplace_back(1, 0);
	std::sort(values.begin(), values.end());
	ASSERT_EQUAL(1, values[0].first);
	ASSERT_EQUAL(0, values[0].second);
	ASSERT_EQUAL(1, values[1].first);
	ASSERT_EQUAL(9, values[1].second);
	ASSERT_EQUAL(2, values[2].first);
	auto found = std::find(values.begin(), values.end(), Safe::Pair<int, int>(1, 9));
	ASSERT_TRUE(found != values.end());
	std::vector<int> keys(values.size());
	std::transform(values.begin(), values.end(), keys.begin(), [](const Safe::Pair<int, int>& item) {
		return item.first;
	});
	ASSERT_EQUAL(1, keys.front());
	ASSERT_EQUAL(2, keys.back());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Construct
	// -------------------
	result += test_construct_convert_and_get();

	// -------------------
	// Order
	// -------------------
	result += test_order_swap_and_algorithm();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
