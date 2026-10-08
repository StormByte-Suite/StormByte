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

#include <StormByte/safe/deque.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

using namespace StormByte;

// -------------------
// Access
// -------------------

int test_at_throws_deque_out_of_bounds() {
	Safe::Deque<int> values;
	bool thrown = false;
	try {
		(void)values.at(0);
	}
	catch (const StormByte::Safe::OutOfBoundsError&) {
		thrown = true;
	}
	ASSERT_TRUE(thrown);
	RETURN_TEST(0);
}

int test_front_and_back() {
	Safe::Deque<int> values{1, 2, 3};
	ASSERT_EQUAL(1, values.front());
	ASSERT_EQUAL(3, values.back());
	values.front() = 8;
	values.back() = 9;
	ASSERT_EQUAL(8, values[0]);
	ASSERT_EQUAL(9, values[2]);
	RETURN_TEST(0);
}

int test_index_reads_and_writes() {
	Safe::Deque<int> values{4, 5, 6};
	ASSERT_EQUAL(5, values[1]);
	ASSERT_EQUAL(6, values.at(2));
	values[1] = 15;
	ASSERT_EQUAL(15, values.at(1));
	RETURN_TEST(0);
}

// -------------------
// Algorithm
// -------------------

int test_algorithm_count_find_reverse_and_sort() {
	Safe::Deque<int> values{4, 1, 4, 2};
	ASSERT_EQUAL(std::ptrdiff_t{2}, std::count(values.begin(), values.end(), 4));
	ASSERT_TRUE(std::find(values.cbegin(), values.cend(), 1) != values.cend());
	ASSERT_TRUE(std::find(values.cbegin(), values.cend(), 9) == values.cend());
	std::reverse(values.begin(), values.end());
	ASSERT_EQUAL(2, values.front());
	ASSERT_EQUAL(4, values.back());
	std::sort(values.begin(), values.end());
	ASSERT_EQUAL(1, values.front());
	ASSERT_EQUAL(4, values.back());
	ASSERT_TRUE(std::is_sorted(values.begin(), values.end()));
	RETURN_TEST(0);
}

// -------------------
// Compare
// -------------------

int test_equality_and_order() {
	Safe::Deque<int> left{1, 2, 3};
	Safe::Deque<int> same{1, 2, 3};
	Safe::Deque<int> greater{1, 2, 4};
	ASSERT_TRUE(left == same);
	ASSERT_FALSE(left == greater);
	ASSERT_TRUE((left <=> greater) < 0);
	ASSERT_TRUE((greater <=> left) > 0);
	ASSERT_TRUE((left <=> same) == 0);
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_copy_and_move() {
	Safe::Deque<int> source{1, 2, 3};
	Safe::Deque<int> copied(source);
	Safe::Deque<int> moved(std::move(source));
	ASSERT_EQUAL(std::size_t{3}, copied.size());
	ASSERT_EQUAL(1, copied.front());
	ASSERT_EQUAL(std::size_t{3}, moved.size());
	ASSERT_TRUE(source.empty());
	copied = moved;
	ASSERT_EQUAL(3, copied.back());
	Safe::Deque<int> assigned;
	assigned = std::move(moved);
	ASSERT_TRUE(moved.empty());
	ASSERT_EQUAL(1, assigned.front());
	RETURN_TEST(0);
}

int test_count_and_value() {
	Safe::Deque<int> filled(std::size_t{4}, 7);
	ASSERT_EQUAL(std::size_t{4}, filled.size());
	ASSERT_EQUAL(7, filled.front());
	ASSERT_EQUAL(7, filled.back());
	Safe::Deque<int> defaults(std::size_t{3});
	ASSERT_EQUAL(std::size_t{3}, defaults.size());
	ASSERT_EQUAL(0, defaults.front());
	RETURN_TEST(0);
}

int test_empty_starts_empty() {
	Safe::Deque<int> values;
	ASSERT_TRUE(values.empty());
	ASSERT_EQUAL(std::size_t{0}, values.size());
	ASSERT_TRUE(values.begin() == values.end());
	ASSERT_TRUE(values.max_size() > 0);
	RETURN_TEST(0);
}

int test_initializer_and_iterators() {
	const int raw[] = {2, 4, 6};
	Safe::Deque<int> values(std::begin(raw), std::end(raw));
	ASSERT_EQUAL(std::size_t{3}, values.size());
	ASSERT_EQUAL(2, values.front());
	ASSERT_EQUAL(6, values.back());
	Safe::Deque<int> listed{8, 9};
	ASSERT_EQUAL(8, listed.front());
	listed = {1, 2, 3};
	ASSERT_EQUAL(std::size_t{3}, listed.size());
	RETURN_TEST(0);
}

int test_std_deque_copy_and_move() {
	std::deque<int> source{1, 2, 3};
	Safe::Deque<int> copied(source);
	ASSERT_EQUAL(std::size_t{3}, source.size());
	ASSERT_EQUAL(1, copied.front());
	Safe::Deque<int> moved(std::move(source));
	ASSERT_TRUE(source.empty());
	ASSERT_EQUAL(3, moved.back());
	Safe::Deque<int> assigned;
	assigned = copied;
	std::deque<int> unchanged{7};
	assigned = unchanged;
	ASSERT_EQUAL(std::size_t{1}, unchanged.size());
	ASSERT_EQUAL(7, assigned.front());
	std::deque<int> donor{4, 5};
	assigned = std::move(donor);
	ASSERT_TRUE(donor.empty());
	ASSERT_EQUAL(5, assigned.back());
	std::deque<int> back = static_cast<std::deque<int>>(assigned);
	ASSERT_EQUAL(std::size_t{2}, back.size());
	ASSERT_EQUAL(4, back.front());
	RETURN_TEST(0);
}

// -------------------
// Iterate
// -------------------

int test_forward_and_reverse() {
	Safe::Deque<int> values{1, 2, 3};
	int forward = 0;
	for (int value : values)
		forward = forward * 10 + value;
	ASSERT_EQUAL(123, forward);
	int reverse = 0;
	for (auto it = values.rbegin(); it != values.rend(); ++it)
		reverse = reverse * 10 + *it;
	ASSERT_EQUAL(321, reverse);
	ASSERT_EQUAL(1, *values.cbegin());
	ASSERT_EQUAL(3, *values.crbegin());
	ASSERT_TRUE(values.cend() == values.end());
	ASSERT_TRUE(values.crend() == values.rend());
	ASSERT_EQUAL(std::ptrdiff_t{3}, values.end() - values.begin());
	ASSERT_EQUAL(3, values.begin()[2]);
	RETURN_TEST(0);
}

// -------------------
// Modify
// -------------------

int test_append_and_prepend_range() {
	Safe::Deque<int> values{3};
	values.append_range(std::vector<int>{4, 5});
	values.prepend_range(std::vector<int>{1, 2});
	ASSERT_EQUAL(std::size_t{5}, values.size());
	ASSERT_EQUAL(1, values.front());
	ASSERT_EQUAL(5, values.back());
	RETURN_TEST(0);
}

int test_assign_replaces() {
	Safe::Deque<int> values{1};
	values.assign(std::size_t{3}, 5);
	ASSERT_EQUAL(std::size_t{3}, values.size());
	ASSERT_EQUAL(5, values.back());
	const int raw[] = {8, 9};
	values.assign(std::begin(raw), std::end(raw));
	ASSERT_EQUAL(8, values.front());
	values.assign({4, 5, 6});
	ASSERT_EQUAL(std::size_t{3}, values.size());
	values.assign_range(std::vector<int>{1, 2});
	ASSERT_EQUAL(2, values.back());
	RETURN_TEST(0);
}

int test_clear_and_shrink() {
	Safe::Deque<int> values{1, 2, 3};
	values.clear();
	ASSERT_TRUE(values.empty());
	values.shrink_to_fit();
	ASSERT_EQUAL(std::size_t{0}, values.size());
	RETURN_TEST(0);
}

int test_emplace_insert_and_erase() {
	Safe::Deque<int> values{1, 4};
	auto inserted = values.emplace(values.begin() + 1, 2);
	ASSERT_EQUAL(2, *inserted);
	values.insert(values.end(), 5);
	values.insert(values.begin() + 3, std::size_t{1}, 4);
	const int raw[] = {9};
	values.insert(values.begin(), std::begin(raw), std::end(raw));
	values.insert(values.end(), {8});
	values.insert_range(values.begin() + 1, std::vector<int>{7});
	ASSERT_EQUAL(9, values.front());
	auto after = values.erase(values.begin());
	ASSERT_EQUAL(7, *after);
	values.erase(values.begin(), values.begin() + 1);
	ASSERT_EQUAL(1, values.front());
	RETURN_TEST(0);
}

int test_push_pop_both_ends() {
	Safe::Deque<int> values;
	values.push_back(2);
	values.push_front(1);
	values.emplace_back(3);
	values.emplace_front(0);
	ASSERT_EQUAL(std::size_t{4}, values.size());
	ASSERT_EQUAL(0, values.front());
	ASSERT_EQUAL(3, values.back());
	values.pop_front();
	values.pop_back();
	ASSERT_EQUAL(1, values.front());
	ASSERT_EQUAL(2, values.back());
	RETURN_TEST(0);
}

int test_resize_grows_and_shrinks() {
	Safe::Deque<int> values{1};
	values.resize(3);
	ASSERT_EQUAL(std::size_t{3}, values.size());
	ASSERT_EQUAL(0, values.back());
	values.resize(5, 6);
	ASSERT_EQUAL(6, values.back());
	values.resize(2);
	ASSERT_EQUAL(std::size_t{2}, values.size());
	ASSERT_EQUAL(1, values.front());
	RETURN_TEST(0);
}

int test_swap_exchanges() {
	Safe::Deque<int> left{1, 2};
	Safe::Deque<int> right{9};
	left.swap(right);
	ASSERT_EQUAL(9, left.front());
	ASSERT_EQUAL(1, right.front());
	swap(left, right);
	ASSERT_EQUAL(std::size_t{2}, left.size());
	ASSERT_EQUAL(2, left.back());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Access
	// -------------------
	result += test_at_throws_deque_out_of_bounds();
	result += test_front_and_back();
	result += test_index_reads_and_writes();

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_count_find_reverse_and_sort();

	// -------------------
	// Compare
	// -------------------
	result += test_equality_and_order();

	// -------------------
	// Construct
	// -------------------
	result += test_copy_and_move();
	result += test_count_and_value();
	result += test_empty_starts_empty();
	result += test_initializer_and_iterators();
	result += test_std_deque_copy_and_move();

	// -------------------
	// Iterate
	// -------------------
	result += test_forward_and_reverse();

	// -------------------
	// Modify
	// -------------------
	result += test_append_and_prepend_range();
	result += test_assign_replaces();
	result += test_clear_and_shrink();
	result += test_emplace_insert_and_erase();
	result += test_push_pop_both_ends();
	result += test_resize_grows_and_shrinks();
	result += test_swap_exchanges();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
