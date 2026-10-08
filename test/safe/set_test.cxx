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

#include <StormByte/safe/set.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	Safe::Set<int> Ordered() {
		return Safe::Set<int>{4, 1, 4, 2, 3};
	}
}

// -------------------
// Algorithm
// -------------------

int test_algorithm_bounds_and_order() {
	const Safe::Set<int> values = Ordered();
	ASSERT_TRUE(std::is_sorted(values.begin(), values.end()));
	ASSERT_EQUAL(1, *std::min_element(values.begin(), values.end()));
	ASSERT_EQUAL(4, *std::max_element(values.begin(), values.end()));
	ASSERT_EQUAL(values.end(), std::adjacent_find(values.begin(), values.end()));
	ASSERT_EQUAL(4, static_cast<int>(std::distance(values.begin(), values.end())));
	const auto found = std::find(values.begin(), values.end(), 3);
	ASSERT_NOT_EQUAL(values.end(), found);
	ASSERT_EQUAL(1, static_cast<int>(std::count(values.begin(), values.end(), 2)));
	ASSERT_EQUAL(0, static_cast<int>(std::count(values.begin(), values.end(), 9)));
	std::vector<int> reversed;
	std::reverse_copy(values.begin(), values.end(), std::back_inserter(reversed));
	ASSERT_EQUAL(4, reversed.front());
	ASSERT_EQUAL(1, reversed.back());
	RETURN_TEST(0);
}

int test_algorithm_set_operations() {
	const Safe::Set<int> left{1, 2, 3, 5};
	const Safe::Set<int> right{2, 4, 5};
	std::vector<int> both;
	std::vector<int> either;
	std::vector<int> only_left;
	std::set_intersection(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(both));
	std::set_union(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(either));
	std::set_difference(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(only_left));
	ASSERT_TRUE(std::includes(left.begin(), left.end(), both.begin(), both.end()));
	ASSERT_EQUAL(2, static_cast<int>(both.size()));
	ASSERT_EQUAL(2, both.front());
	ASSERT_EQUAL(5, static_cast<int>(either.size()));
	ASSERT_EQUAL(2, static_cast<int>(only_left.size()));
	ASSERT_EQUAL(1, only_left.front());
	ASSERT_TRUE(std::equal(left.begin(), left.end(), std::vector<int>{1, 2, 3, 5}.begin()));
	int sum = 0;
	std::for_each(left.begin(), left.end(), [&sum](int value) { sum += value; });
	ASSERT_EQUAL(11, sum);
	RETURN_TEST(0);
}

// -------------------
// Bounds
// -------------------

int test_bounds_and_equal_range() {
	const Safe::Set<int> values = Ordered();
	ASSERT_EQUAL(2, *values.lower_bound(2));
	ASSERT_EQUAL(3, *values.upper_bound(2));
	const auto range = values.equal_range(2);
	ASSERT_EQUAL(2, *range.first);
	ASSERT_EQUAL(3, *range.second);
	ASSERT_EQUAL(values.end(), values.lower_bound(9));
	ASSERT_EQUAL(values.end(), values.upper_bound(9));
	Safe::Set<int> mutable_values = values;
	ASSERT_EQUAL(1, *mutable_values.lower_bound(0));
	ASSERT_EQUAL(4, *mutable_values.upper_bound(3));
	RETURN_TEST(0);
}

// -------------------
// Compare
// -------------------

int test_compare_lexicographical() {
	const Safe::Set<int> left{1, 2};
	const Safe::Set<int> same{1, 2};
	const Safe::Set<int> right{1, 3};
	ASSERT_TRUE(left == same);
	ASSERT_FALSE(left != same);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE(left <= right);
	ASSERT_TRUE(left <= same);
	ASSERT_TRUE(right > left);
	ASSERT_TRUE(right >= left);
	ASSERT_FALSE(left > same);
	RETURN_TEST(0);
}

int test_compare_stateful() {
	const Safe::Set<int, std::greater<int>> values({1, 3, 2}, std::greater<int>{});
	ASSERT_EQUAL(3, *values.begin());
	ASSERT_EQUAL(1, *values.rbegin());
	ASSERT_TRUE(values.key_comp()(3, 1));
	ASSERT_TRUE(values.value_comp()(3, 1));
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_assign_and_braces() {
	Safe::Set<int> empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(0, static_cast<int>(empty.size()));
	ASSERT_TRUE(empty.max_size() > 0);
	Safe::Set<int> braced{4, 1, 4, 2};
	ASSERT_EQUAL(3, static_cast<int>(braced.size()));
	ASSERT_EQUAL(1, *braced.begin());
	const int raw[] = {7, 5, 7};
	Safe::Set<int> ranged(std::begin(raw), std::end(raw));
	ASSERT_EQUAL(2, static_cast<int>(ranged.size()));
	Safe::Set<int> copied(braced);
	Safe::Set<int> moved(std::move(copied));
	ASSERT_TRUE(copied.empty());
	ASSERT_EQUAL(3, static_cast<int>(moved.size()));
	empty = moved;
	ASSERT_EQUAL(3, static_cast<int>(empty.size()));
	empty = {9, 8, 9};
	ASSERT_EQUAL(2, static_cast<int>(empty.size()));
	ASSERT_EQUAL(8, *empty.begin());
	Safe::Set<int> taken(std::move(empty));
	empty = std::move(taken);
	ASSERT_TRUE(taken.empty());
	ASSERT_EQUAL(2, static_cast<int>(empty.size()));
	const auto exported = static_cast<std::set<int>>(empty);
	ASSERT_EQUAL(2, static_cast<int>(exported.size()));
	ASSERT_TRUE(exported.contains(8));
	RETURN_TEST(0);
}

int test_construct_iterators() {
	const Safe::Set<int> values = Ordered();
	ASSERT_EQUAL(values.begin(), values.cbegin());
	ASSERT_EQUAL(values.end(), values.cend());
	ASSERT_EQUAL(4, *values.rbegin());
	ASSERT_EQUAL(1, *std::prev(values.rend()));
	ASSERT_EQUAL(values.rbegin(), values.crbegin());
	ASSERT_EQUAL(values.rend(), values.crend());
	Safe::Set<int> mutable_values = values;
	ASSERT_EQUAL(1, *mutable_values.begin());
	ASSERT_EQUAL(4, *mutable_values.rbegin());
	auto cursor = mutable_values.begin();
	ASSERT_EQUAL(1, *cursor++);
	ASSERT_EQUAL(2, *cursor);
	ASSERT_EQUAL(1, *--cursor);
	++cursor;
	++cursor;
	ASSERT_EQUAL(3, *cursor);
	RETURN_TEST(0);
}

// -------------------
// Erase
// -------------------

int test_erase_key_iterator_and_range() {
	Safe::Set<int> values = Ordered();
	ASSERT_EQUAL(1, static_cast<int>(values.erase(2)));
	ASSERT_EQUAL(0, static_cast<int>(values.erase(2)));
	ASSERT_FALSE(values.contains(2));
	auto next = values.erase(values.begin());
	ASSERT_EQUAL(3, *next);
	next = values.erase(values.cbegin());
	ASSERT_EQUAL(4, *next);
	values = Ordered();
	auto after = values.erase(std::next(values.begin()), std::prev(values.end()));
	ASSERT_EQUAL(4, *after);
	ASSERT_EQUAL(2, static_cast<int>(values.size()));
	values.clear();
	ASSERT_TRUE(values.empty());
	RETURN_TEST(0);
}

// -------------------
// Extract
// -------------------

int test_extract_insert_node_and_merge() {
	Safe::Set<int> values = Ordered();
	auto node = values.extract(3);
	ASSERT_TRUE(static_cast<bool>(node));
	ASSERT_EQUAL(3, node.value());
	ASSERT_FALSE(values.contains(3));
	auto inserted = values.insert(std::move(node));
	ASSERT_TRUE(inserted.inserted);
	ASSERT_TRUE(inserted.node.empty());
	ASSERT_EQUAL(3, *inserted.position);
	auto again = values.extract(values.find(3));
	auto rejected = values.insert(values.end(), std::move(again));
	ASSERT_EQUAL(3, *rejected);
	Safe::Set<int, std::greater<int>> other({3, 9}, std::greater<int>{});
	values.merge(other);
	ASSERT_TRUE(values.contains(9));
	ASSERT_TRUE(other.contains(3));
	ASSERT_FALSE(other.contains(9));
	auto missing = values.extract(100);
	ASSERT_TRUE(missing.empty());
	RETURN_TEST(0);
}

// -------------------
// Insert
// -------------------

int test_insert_emplace_and_range() {
	Safe::Set<int> values;
	auto first = values.insert(2);
	ASSERT_TRUE(first.second);
	auto duplicate = values.insert(2);
	ASSERT_FALSE(duplicate.second);
	ASSERT_EQUAL(values.begin(), duplicate.first);
	ASSERT_EQUAL(1, *values.insert(values.end(), 1));
	auto placed = values.emplace(3);
	ASSERT_TRUE(placed.second);
	ASSERT_EQUAL(3, *placed.first);
	ASSERT_EQUAL(4, *values.emplace_hint(values.end(), 4));
	const int extra[] = {4, 5};
	values.insert(std::begin(extra), std::end(extra));
	values.insert({0, 5});
	const std::vector<int> more{6, 0};
	values.insert_range(more);
	ASSERT_EQUAL(7, static_cast<int>(values.size()));
	ASSERT_EQUAL(0, *values.begin());
	ASSERT_EQUAL(6, *values.rbegin());
	RETURN_TEST(0);
}

// -------------------
// Lookup
// -------------------

int test_lookup_find_count_contains() {
	const Safe::Set<Safe::String> values{Safe::String("b"), Safe::String("a")};
	ASSERT_TRUE(values.contains(Safe::String("a")));
	ASSERT_FALSE(values.contains(Safe::String("z")));
	ASSERT_EQUAL(1, static_cast<int>(values.count(Safe::String("b"))));
	ASSERT_EQUAL(0, static_cast<int>(values.count(Safe::String("z"))));
	ASSERT_EQUAL(std::string_view{"a"}, std::string_view{*values.find(Safe::String("a"))});
	ASSERT_EQUAL(values.end(), values.find(Safe::String("missing")));
	Safe::Set<Safe::String> mutable_values = values;
	ASSERT_NOT_EQUAL(mutable_values.end(), mutable_values.find(Safe::String("b")));
	RETURN_TEST(0);
}

// -------------------
// Swap
// -------------------

int test_swap_member_and_free() {
	Safe::Set<int> left{1, 2};
	Safe::Set<int> right{9};
	left.swap(right);
	ASSERT_EQUAL(1, static_cast<int>(left.size()));
	ASSERT_TRUE(left.contains(9));
	swap(left, right);
	ASSERT_EQUAL(2, static_cast<int>(left.size()));
	ASSERT_TRUE(right.contains(9));
	RETURN_TEST(0);
}

int test_stateful_list_assignment() {
	struct Compare {
		bool descending = false;
		bool operator()(int left, int right) const { return descending ? left > right : left < right; }
	};
	Safe::Set<int, Compare> values(Compare{true});
	values = {1, 3, 2, 3};
	ASSERT_TRUE(values.key_comp()(3, 1));
	ASSERT_EQUAL(3, *values.begin());
	ASSERT_EQUAL(1, *values.rbegin());
	ASSERT_EQUAL(std::size_t{3}, values.size());
	RETURN_TEST(0);
}

int test_rejected_nodes_and_stable_iterators() {
	Safe::Set<int> source{1, 2};
	Safe::Set<int> target{1};
	auto stable = source.find(2);
	const auto* address = &*stable;
	auto handle = source.extract(1);
	auto rejected = target.insert(std::move(handle));
	ASSERT_FALSE(rejected.inserted);
	ASSERT_TRUE(handle.empty());
	ASSERT_FALSE(rejected.node.empty());
	ASSERT_EQUAL(1, rejected.node.value());
	ASSERT_TRUE(source.insert(std::move(rejected.node)).inserted);
	auto hinted = source.extract(1);
	ASSERT_EQUAL(1, *target.insert(target.end(), std::move(hinted)));
	ASSERT_FALSE(hinted.empty());
	ASSERT_EQUAL(1, *source.insert(source.end(), std::move(hinted)));
	ASSERT_TRUE(hinted.empty());
	source.insert(3);
	source.erase(1);
	ASSERT_TRUE(&*stable == address);
	ASSERT_EQUAL(2, *stable);
	RETURN_TEST(0);
}

int test_differential_insert_erase_and_bounds() {
	Safe::Set<int> values;
	std::set<int> model;
	for (int index = 0; index < 96; ++index) {
		const int key = (index * 37) % 61;
		ASSERT_EQUAL(model.insert(key).second, values.insert(key).second);
	}
	for (int key = 0; key < 61; key += 2) ASSERT_EQUAL(model.erase(key), values.erase(key));
	ASSERT_TRUE(std::equal(values.begin(), values.end(), model.begin(), model.end()));
	for (int key = -1; key < 63; ++key) {
		const auto lower = values.lower_bound(key);
		const auto standard_lower = model.lower_bound(key);
		ASSERT_EQUAL(standard_lower == model.end(), lower == values.end());
		if (lower != values.end()) ASSERT_EQUAL(*standard_lower, *lower);
		const auto upper = values.upper_bound(key);
		const auto standard_upper = model.upper_bound(key);
		ASSERT_EQUAL(standard_upper == model.end(), upper == values.end());
		if (upper != values.end()) ASSERT_EQUAL(*standard_upper, *upper);
		const auto range = values.equal_range(key);
		ASSERT_EQUAL(lower, range.first);
		ASSERT_EQUAL(upper, range.second);
	}
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_bounds_and_order();
	result += test_algorithm_set_operations();
	result += test_stateful_list_assignment();
	result += test_rejected_nodes_and_stable_iterators();
	result += test_differential_insert_erase_and_bounds();

	// -------------------
	// Bounds
	// -------------------
	result += test_bounds_and_equal_range();

	// -------------------
	// Compare
	// -------------------
	result += test_compare_lexicographical();
	result += test_compare_stateful();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_assign_and_braces();
	result += test_construct_iterators();

	// -------------------
	// Erase
	// -------------------
	result += test_erase_key_iterator_and_range();

	// -------------------
	// Extract
	// -------------------
	result += test_extract_insert_node_and_merge();

	// -------------------
	// Insert
	// -------------------
	result += test_insert_emplace_and_range();

	// -------------------
	// Lookup
	// -------------------
	result += test_lookup_find_count_contains();

	// -------------------
	// Swap
	// -------------------
	result += test_swap_member_and_free();

	return result;
}
