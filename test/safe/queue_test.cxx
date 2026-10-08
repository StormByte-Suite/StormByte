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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <ranges>
#include <utility>
#include <vector>

using namespace StormByte;

// -------------------
// Algorithm
// -------------------

int test_algorithm_front_to_back() {
	Safe::Queue<int> values(std::from_range, std::vector<int>{3, 1, 2, 2, 0});
	const std::size_t owned = values.size();
	std::sort(values.begin(), values.end());
	std::stable_sort(values.begin(), values.end());
	std::ranges::sort(values);
	ASSERT_EQUAL(owned, values.size());
	ASSERT_TRUE(std::is_sorted(values.begin(), values.end()));
	ASSERT_EQUAL(0, *std::min_element(values.begin(), values.end()));
	ASSERT_EQUAL(3, *std::max_element(values.begin(), values.end()));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count(values.begin(), values.end(), 2));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(1), std::count_if(values.begin(), values.end(), [](int n) { return n == 0; }));
	ASSERT_TRUE(std::all_of(values.begin(), values.end(), [](int n) { return n >= 0; }));
	ASSERT_TRUE(std::any_of(values.begin(), values.end(), [](int n) { return n == 3; }));
	ASSERT_TRUE(std::none_of(values.begin(), values.end(), [](int n) { return n < 0; }));
	ASSERT_TRUE(std::binary_search(values.begin(), values.end(), 1));
	ASSERT_TRUE(std::find(values.begin(), values.end(), 3) != values.end());
	ASSERT_TRUE(std::find_if(values.begin(), values.end(), [](int n) { return n == 2; }) != values.end());
	ASSERT_TRUE(std::ranges::find(values, 0) == values.begin());
	std::for_each(values.begin(), values.end(), [](int& n) { n = n; });
	std::ranges::for_each(values, [](int& n) { n = n; });
	std::reverse(values.begin(), values.end());
	std::rotate(values.begin(), values.begin() + 1, values.end());
	std::replace(values.begin(), values.end(), 0, 9);
	std::fill(values.begin(), values.end(), 4);
	std::generate(values.begin(), values.end(), [n = 0]() mutable { return n++; });
	std::transform(values.begin(), values.end(), values.begin(), [](int n) { return n; });
	Safe::Queue<int> other(values);
	ASSERT_TRUE(std::equal(values.begin(), values.end(), other.begin()));
	ASSERT_TRUE(std::ranges::equal(values, other));
	ASSERT_TRUE(std::mismatch(values.begin(), values.end(), other.begin()).first == values.end());
	std::vector<int> copied(owned);
	std::copy(values.begin(), values.end(), copied.begin());
	std::reverse_copy(values.begin(), values.end(), copied.begin());
	std::ranges::copy(values, copied.begin());
	std::iter_swap(values.begin(), values.end() - 1);
	std::partition(values.begin(), values.end(), [](int n) { return n < 3; });
	std::sort(values.begin(), values.end());
	ASSERT_TRUE(std::unique(values.begin(), values.end()) <= values.end());
	ASSERT_EQUAL(owned, values.size());
	RETURN_TEST(0);
}

// -------------------
// Life
// -------------------

int test_construct_copy_move_and_export() {
	Safe::Queue<int> empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(static_cast<std::size_t>(0), empty.size());
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.cbegin() == empty.cend());
	std::queue<int> caller;
	caller.push(4);
	Safe::Queue<int> from_std(caller);
	ASSERT_EQUAL(static_cast<std::size_t>(1), caller.size());
	std::queue<int> stolen;
	stolen.push(5);
	Safe::Queue<int> from_moved(std::move(stolen));
	ASSERT_TRUE(stolen.empty());
	Safe::Vector<int> owned{6, 7};
	Safe::Queue<int> from_container(owned);
	ASSERT_EQUAL(Size{2}, owned.size());
	Safe::Queue<int> taken(std::move(owned));
	ASSERT_TRUE(owned.empty());
	Safe::Queue<int> from_range(std::from_range, std::vector<int>{8});
	Safe::Queue<int> copy(from_range);
	copy = from_range;
	Safe::Queue<int> moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	moved = std::move(from_range);
	moved = caller;
	std::queue<int> assigned;
	assigned.push(9);
	moved = std::move(assigned);
	ASSERT_TRUE(assigned.empty());
	const std::queue<int> exported(moved);
	ASSERT_EQUAL(static_cast<std::size_t>(1), exported.size());
	ASSERT_EQUAL(9, moved.front());
	ASSERT_EQUAL(9, moved.back());
	const Safe::Queue<int>& read = moved;
	ASSERT_EQUAL(9, read.front());
	ASSERT_EQUAL(9, read.back());
	ASSERT_EQUAL(9, *read.begin());
	ASSERT_TRUE(read.cend() != read.cbegin());
	RETURN_TEST(0);
}

// -------------------
// Mutate
// -------------------

int test_self_assignment_and_moved_from_reuse() {
	Safe::Queue<Safe::Vector<int>> values;
	values.emplace(std::initializer_list<int>{11, 12});
	auto& alias = values;
	ASSERT_TRUE(&(values = alias) == &values);
	ASSERT_TRUE(&(values = std::move(alias)) == &values);
	ASSERT_EQUAL(std::size_t{1}, values.size());
	ASSERT_EQUAL(11, values.front().front());
	ASSERT_EQUAL(12, values.back().back());
	Safe::Queue<Safe::Vector<int>> moved(std::move(values));
	ASSERT_TRUE(values.empty());
	values.emplace(std::initializer_list<int>{21});
	ASSERT_EQUAL(21, values.front().front());
	values = std::move(moved);
	ASSERT_TRUE(moved.empty());
	moved.emplace(std::initializer_list<int>{31});
	ASSERT_EQUAL(31, moved.front().front());
	ASSERT_EQUAL(11, values.front().front());
	RETURN_TEST(0);
}

int test_aliased_modifiers_across_growth() {
	Safe::Queue<Safe::Vector<int>> values;
	values.emplace(std::initializer_list<int>{41, 42});
	values.push(values.front());
	ASSERT_EQUAL(std::size_t{2}, values.size());
	ASSERT_EQUAL(41, values.front().front());
	ASSERT_EQUAL(42, values.back().back());
	values.emplace(values.back());
	ASSERT_EQUAL(std::size_t{3}, values.size());
	ASSERT_EQUAL(41, values.back().front());
	values.back().front() = 99;
	ASSERT_EQUAL(41, values.front().front());
	Safe::Queue<Safe::Vector<int>> moving;
	moving.emplace(std::initializer_list<int>{51});
	moving.push(std::move(moving.front()));
	ASSERT_TRUE(moving.front().empty());
	ASSERT_EQUAL(51, moving.back().front());
	RETURN_TEST(0);
}

int test_empty_boundaries_preserve_reusability() {
	Safe::Queue<Safe::Vector<int>> values;
	const auto& read = values;
	ASSERT_THROWS(values.pop(), Safe::OutOfBoundsError);
	ASSERT_THROWS(values.front(), Safe::OutOfBoundsError);
	ASSERT_THROWS(values.back(), Safe::OutOfBoundsError);
	ASSERT_THROWS(read.front(), Safe::OutOfBoundsError);
	ASSERT_THROWS(read.back(), Safe::OutOfBoundsError);
	ASSERT_TRUE(values.empty());
	ASSERT_TRUE(values.begin() == values.end());
	values.emplace(std::initializer_list<int>{61});
	values.pop();
	ASSERT_TRUE(values.empty());
	ASSERT_THROWS(values.pop(), Safe::OutOfBoundsError);
	values.emplace(std::initializer_list<int>{62});
	ASSERT_EQUAL(62, values.front().front());
	RETURN_TEST(0);
}

int test_push_pop_erase_and_order() {
	Safe::Queue<int> values;
	values.push(1);
	int moved = 2;
	values.push(std::move(moved));
	ASSERT_EQUAL(3, values.emplace(3));
	values.push_range(std::vector<int>{4});
	ASSERT_EQUAL(1, values.front());
	ASSERT_EQUAL(4, values.back());
	values.front() = 1;
	values.back() = 4;
	values.pop();
	ASSERT_EQUAL(2, values.front());
	ASSERT_THROWS(Safe::Queue<int>().pop(), Safe::OutOfBoundsError);
	ASSERT_THROWS(Safe::Queue<int>().front(), Safe::OutOfBoundsError);
	ASSERT_THROWS(Safe::Queue<int>().back(), Safe::OutOfBoundsError);
	auto after = values.erase(values.begin());
	ASSERT_TRUE(after == values.begin() || values.empty());
	values.erase(values.begin(), values.end());
	ASSERT_TRUE(values.empty());
	values.push(1);
	values.push(3);
	Safe::Queue<int> other;
	other.push(9);
	values.swap(other);
	swap(values, other);
	Safe::Queue<int> left;
	left.push(1);
	Safe::Queue<int> right;
	right.push(2);
	ASSERT_TRUE(left == left);
	ASSERT_TRUE(left != right);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE(left <= left);
	ASSERT_TRUE(right > left);
	ASSERT_TRUE(right >= left);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::less);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_front_to_back();

	// -------------------
	// Life
	// -------------------
	result += test_construct_copy_move_and_export();

	// -------------------
	// Mutate
	// -------------------
	result += test_self_assignment_and_moved_from_reuse();
	result += test_aliased_modifiers_across_growth();
	result += test_empty_boundaries_preserve_reusability();
	result += test_push_pop_erase_and_order();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
