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

#include <StormByte/safe/list.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <vector>

using namespace StormByte;

using Chain = Safe::List<int>;

// -------------------
// Algorithm
// -------------------

int test_algorithm_walks_both_directions() {
	Chain chain{1, 2, 3, 4};
	std::vector<int> forward;
	std::for_each(chain.begin(), chain.end(), [&forward](int value) { forward.push_back(value); });
	ASSERT_EQUAL(4u, forward.size());
	ASSERT_EQUAL(1, forward.front());
	ASSERT_EQUAL(4, forward.back());
	ASSERT_EQUAL(std::distance(chain.begin(), chain.end()), static_cast<std::ptrdiff_t>(chain.size()));
	ASSERT_TRUE(std::find(chain.begin(), chain.end(), 3) != chain.end());
	ASSERT_EQUAL(1, static_cast<int>(std::count(chain.begin(), chain.end(), 2)));
	ASSERT_TRUE(std::any_of(chain.cbegin(), chain.cend(), [](int value) { return value == 4; }));
	ASSERT_FALSE(std::none_of(chain.cbegin(), chain.cend(), [](int value) { return value == 1; }));
	std::vector<int> backward;
	std::for_each(chain.rbegin(), chain.rend(), [&backward](int value) { backward.push_back(value); });
	ASSERT_EQUAL(4, backward.front());
	ASSERT_EQUAL(1, backward.back());
	std::for_each(chain.begin(), chain.end(), [](int& value) { value += 1; });
	ASSERT_EQUAL(2, chain.front());
	RETURN_TEST(0);
}

// -------------------
// Assign
// -------------------

int test_assign_copy_move_count_and_range() {
	Chain source{1, 2};
	Chain copied;
	copied = source;
	ASSERT_TRUE(copied == source);
	Chain moved;
	moved = std::move(copied);
	ASSERT_TRUE(copied.empty());
	ASSERT_EQUAL(2u, moved.size());
	moved = {8, 9};
	ASSERT_EQUAL(8, moved.front());
	moved.assign(3, 4);
	ASSERT_EQUAL(3u, moved.size());
	ASSERT_EQUAL(4, moved.back());
	moved.assign({5});
	ASSERT_EQUAL(5, moved.front());
	std::vector<int> range{6, 7};
	moved.assign_range(range);
	ASSERT_EQUAL(7, moved.back());
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_count_std_and_range() {
	Chain empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(0u, empty.size());
	ASSERT_TRUE(empty.max_size() > 0);
	ASSERT_EQUAL(empty.begin(), empty.end());
	Chain counted(3);
	ASSERT_EQUAL(3u, counted.size());
	ASSERT_EQUAL(0, counted.front());
	Chain filled(2, 9);
	ASSERT_EQUAL(9, filled.back());
	Chain copied(filled);
	ASSERT_TRUE(copied == filled);
	Chain moved(std::move(copied));
	ASSERT_TRUE(copied.empty());
	ASSERT_EQUAL(9, moved.front());
	std::list<int> standard{4, 5};
	Chain from_std(standard);
	ASSERT_EQUAL(2u, standard.size());
	ASSERT_EQUAL(4, from_std.front());
	Chain from_moved(std::move(standard));
	ASSERT_TRUE(standard.empty());
	ASSERT_EQUAL(5, from_moved.back());
	std::vector<int> range{7};
	Chain from_range(std::from_range, range);
	ASSERT_EQUAL(7, from_range.front());
	auto converted = static_cast<std::list<int>>(moved);
	ASSERT_EQUAL(2u, converted.size());
	ASSERT_EQUAL(9, converted.front());
	RETURN_TEST(0);
}

// -------------------
// Erase
// -------------------

int test_erase_one_range_and_clear() {
	Chain chain{1, 2, 3, 4};
	auto next = chain.erase(chain.begin());
	ASSERT_EQUAL(2, *next);
	next = chain.erase(chain.begin(), std::next(chain.begin(), 2));
	ASSERT_EQUAL(4, *next);
	ASSERT_EQUAL(1u, chain.size());
	chain.clear();
	ASSERT_TRUE(chain.empty());
	ASSERT_EQUAL(chain.begin(), chain.end());
	RETURN_TEST(0);
}

// -------------------
// Insert
// -------------------

int test_insert_emplace_and_ends() {
	Chain chain;
	chain.push_back(2);
	chain.push_front(1);
	chain.push_back(3);
	chain.emplace_back(4);
	chain.emplace_front(0);
	ASSERT_EQUAL(0, chain.front());
	ASSERT_EQUAL(4, chain.back());
	const Chain& read = chain;
	ASSERT_EQUAL(0, read.front());
	ASSERT_EQUAL(4, read.back());
	auto inserted = chain.insert(chain.end(), 6);
	ASSERT_EQUAL(6, *inserted);
	chain.insert(chain.end(), 7);
	chain.insert(std::prev(chain.end()), 2, 5);
	ASSERT_EQUAL(7, chain.back());
	ASSERT_EQUAL(5, *std::prev(chain.end(), 2));
	chain.pop_back();
	chain.pop_front();
	ASSERT_EQUAL(1, chain.front());
	std::vector<int> extra{8, 9};
	chain.insert_range(chain.end(), extra);
	chain.insert(chain.end(), {10});
	ASSERT_EQUAL(10, chain.back());
	chain.resize(2);
	ASSERT_EQUAL(2u, chain.size());
	chain.resize(4, 11);
	ASSERT_EQUAL(11, chain.back());
	RETURN_TEST(0);
}

// -------------------
// Node
// -------------------

int test_node_extract_insert_and_splice() {
	Chain chain{1, 2, 3};
	Chain::node_type extracted = chain.extract(chain.begin());
	ASSERT_FALSE(extracted.empty());
	ASSERT_TRUE(static_cast<bool>(extracted));
	ASSERT_EQUAL(1, extracted.value());
	extracted.value() = 9;
	auto back = chain.insert(chain.end(), std::move(extracted));
	ASSERT_TRUE(extracted.empty());
	ASSERT_EQUAL(9, *back);
	Chain other{4, 5};
	chain.splice(chain.begin(), other);
	ASSERT_TRUE(other.empty());
	ASSERT_EQUAL(4, chain.front());
	other = Chain{6, 7, 8};
	chain.splice(chain.end(), other, other.begin());
	ASSERT_EQUAL(6, chain.back());
	ASSERT_EQUAL(2u, other.size());
	chain.splice(chain.begin(), other, other.begin(), other.end());
	ASSERT_TRUE(other.empty());
	Chain::node_type left = chain.extract(chain.begin());
	Chain::node_type right = chain.extract(chain.begin());
	left.swap(right);
	ASSERT_EQUAL(right.value(), 7);
	RETURN_TEST(0);
}

// -------------------
// Order
// -------------------

int test_order_merge_unique_sort_and_reverse() {
	Chain chain{1, 1, 2, 2, 3};
	ASSERT_EQUAL(2u, chain.unique());
	ASSERT_EQUAL(3u, chain.size());
	ASSERT_EQUAL(1u, chain.unique([](int left, int right) { return left + 1 == right; }));
	chain = Chain{1, 2, 2, 3};
	ASSERT_EQUAL(2u, chain.remove(2));
	ASSERT_EQUAL(1u, chain.remove_if([](int value) { return value == 3; }));
	chain = Chain{3, 1, 2};
	chain.sort();
	ASSERT_EQUAL(1, chain.front());
	ASSERT_EQUAL(3, chain.back());
	chain.sort(std::greater<int>{});
	ASSERT_EQUAL(3, chain.front());
	chain.reverse();
	ASSERT_EQUAL(1, chain.front());
	Chain donor{0, 2, 4};
	chain.merge(donor);
	ASSERT_TRUE(donor.empty());
	ASSERT_EQUAL(0, chain.front());
	Chain greater{5, 3};
	chain = Chain{4, 2};
	chain.merge(greater, std::greater<int>{});
	ASSERT_EQUAL(5, chain.front());
	ASSERT_TRUE(greater.empty());
	Chain same{1, 2};
	Chain other{1, 2};
	Chain shorter{1};
	ASSERT_TRUE(same == other);
	ASSERT_TRUE(same != shorter);
	ASSERT_TRUE((same <=> other) == 0);
	ASSERT_TRUE((shorter <=> same) < 0);
	same.swap(shorter);
	ASSERT_EQUAL(1u, same.size());
	swap(same, shorter);
	ASSERT_EQUAL(2u, same.size());
	ASSERT_EQUAL(same.cbegin(), same.begin());
	ASSERT_EQUAL(same.cend(), same.end());
	ASSERT_EQUAL(*same.crbegin(), same.back());
	ASSERT_EQUAL(*same.rbegin(), same.back());
	ASSERT_EQUAL(*std::prev(same.crend()), same.front());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_walks_both_directions();

	// -------------------
	// Assign
	// -------------------
	result += test_assign_copy_move_count_and_range();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_count_std_and_range();

	// -------------------
	// Erase
	// -------------------
	result += test_erase_one_range_and_clear();

	// -------------------
	// Insert
	// -------------------
	result += test_insert_emplace_and_ends();

	// -------------------
	// Node
	// -------------------
	result += test_node_extract_insert_and_splice();

	// -------------------
	// Order
	// -------------------
	result += test_order_merge_unique_sort_and_reverse();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
