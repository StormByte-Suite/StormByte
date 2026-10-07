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

#include <StormByte/safe/iterable.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	class Numbers: public Safe::Iterable<Safe::Vector<int>> {
		public:
			Numbers() = default;
			explicit Numbers(Safe::Vector<int> values): Safe::Iterable<Safe::Vector<int>>(std::move(values)) {}
			void Push(int value) { Container().push_back(value); }
	};
}

// -------------------
// Algorithm
// -------------------

int test_algorithm_on_cursor() {
	Numbers values(Safe::Vector<int>{3, 1, 2, 2});
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(1), std::count(values.begin(), values.end(), 3));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count_if(values.begin(), values.end(), [](int n) { return n == 2; }));
	ASSERT_TRUE(std::find(values.begin(), values.end(), 1) != values.end());
	ASSERT_TRUE(std::find_if(values.begin(), values.end(), [](int n) { return n == 2; }) != values.end());
	ASSERT_TRUE(std::all_of(values.begin(), values.end(), [](int n) { return n > 0; }));
	ASSERT_TRUE(std::any_of(values.begin(), values.end(), [](int n) { return n == 3; }));
	ASSERT_TRUE(std::none_of(values.begin(), values.end(), [](int n) { return n < 0; }));
	std::for_each(values.begin(), values.end(), [](int& n) { n += 0; });
	Numbers copy(values);
	ASSERT_TRUE(std::equal(values.begin(), values.end(), copy.begin()));
	ASSERT_TRUE(std::mismatch(values.begin(), values.end(), copy.begin()).first == values.end());
	std::vector<int> exported(static_cast<std::size_t>(values.size()));
	std::copy(values.begin(), values.end(), exported.begin());
	ASSERT_EQUAL(3, exported.front());
	ASSERT_EQUAL(static_cast<std::size_t>(4), values.size());
	RETURN_TEST(0);
}

// -------------------
// Life
// -------------------

int test_construct_copy_move_and_empty() {
	Numbers empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(static_cast<std::size_t>(0), empty.size());
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.cbegin() == empty.cend());
	empty.Push(4);
	ASSERT_FALSE(empty.empty());
	ASSERT_EQUAL(static_cast<std::size_t>(1), empty.size());
	ASSERT_EQUAL(4, *empty.begin());
	Numbers copy(empty);
	ASSERT_EQUAL(static_cast<std::size_t>(1), copy.size());
	copy = empty;
	Numbers moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	ASSERT_EQUAL(4, *moved.begin());
	Numbers assigned;
	assigned = std::move(moved);
	ASSERT_TRUE(moved.empty());
	ASSERT_EQUAL(4, *assigned.cbegin());
	const Numbers& read = assigned;
	ASSERT_EQUAL(4, *read.begin());
	ASSERT_TRUE(read.cend() != read.cbegin());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_on_cursor();

	// -------------------
	// Life
	// -------------------
	result += test_construct_copy_move_and_empty();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
