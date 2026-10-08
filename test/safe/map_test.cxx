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
#include <StormByte/safe/map.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

using namespace StormByte;

using Entry = std::pair<const int, int>;

// -------------------
// Algorithm
// -------------------

int test_algorithm_bidirectional() {
	Safe::Map<int, int> values{{3, 30}, {1, 10}, {2, 20}, {2, 21}};
	ASSERT_EQUAL(static_cast<std::size_t>(3), values.size());
	ASSERT_TRUE(std::is_sorted(values.begin(), values.end(), values.value_comp()));
	ASSERT_EQUAL(1, std::min_element(values.begin(), values.end(), values.value_comp())->first);
	ASSERT_EQUAL(3, std::max_element(values.begin(), values.end(), values.value_comp())->first);
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(1), std::count_if(values.begin(), values.end(), [](const auto& entry) {
		return entry.first == 2;
	}));
	ASSERT_TRUE(std::find_if(values.begin(), values.end(), [](const auto& entry) { return entry.second == 20; }) != values.end());
	ASSERT_TRUE(std::all_of(values.begin(), values.end(), [](const auto& entry) { return entry.second > 0; }));
	ASSERT_TRUE(std::any_of(values.begin(), values.end(), [](const auto& entry) { return entry.first == 1; }));
	ASSERT_TRUE(std::none_of(values.begin(), values.end(), [](const auto& entry) { return entry.first < 0; }));
	ASSERT_TRUE(std::adjacent_find(values.begin(), values.end(), values.value_comp()) != values.end());
	std::for_each(values.begin(), values.end(), [](auto& entry) { entry.second += 0; });
	std::vector<int> keys;
	std::transform(values.begin(), values.end(), std::back_inserter(keys), [](const auto& entry) { return entry.first; });
	ASSERT_EQUAL(1, keys.front());
	ASSERT_EQUAL(3, keys.back());
	Safe::Map<int, int> copy(values);
	ASSERT_TRUE(std::equal(values.begin(), values.end(), copy.begin(), [](const auto& left, const auto& right) {
		return left.first == right.first && left.second == right.second;
	}));
	ASSERT_TRUE(std::mismatch(values.begin(), values.end(), copy.begin(), [](const auto& left, const auto& right) {
		return left.first == right.first;
	}).first == values.end());
	ASSERT_FALSE(std::lexicographical_compare(values.begin(), values.end(), copy.begin(), copy.end(), values.value_comp()));
	ASSERT_EQUAL(static_cast<std::size_t>(3), values.size());
	RETURN_TEST(0);
}

// -------------------
// Life
// -------------------

int test_construct_copy_move_and_lookup() {
	Safe::Map<int, int> empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(static_cast<std::size_t>(0), empty.size());
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.cbegin() == empty.cend());
	ASSERT_TRUE(empty.rbegin() == empty.rend());
	ASSERT_TRUE(empty.crbegin() == empty.crend());
	ASSERT_TRUE(empty.max_size() > 0);
	Safe::Map<int, int> compared(std::less<int>{});
	ASSERT_TRUE(compared.key_comp()(1, 2));
	ASSERT_TRUE(compared.value_comp()(Entry(1, 0), Entry(2, 0)));
	Safe::Map<int, int> listed{{2, 20}, {1, 10}};
	ASSERT_EQUAL(10, listed.at(1));
	ASSERT_EQUAL(10, listed.find(1)->second);
	const Safe::Map<int, int>& read = listed;
	ASSERT_EQUAL(10, read.at(1));
	ASSERT_EQUAL(10, read.find(1)->second);
	ASSERT_TRUE(read.contains(1));
	ASSERT_EQUAL(static_cast<std::size_t>(1), read.count(2));
	ASSERT_EQUAL(static_cast<std::size_t>(0), read.count(9));
	ASSERT_TRUE(listed.lower_bound(2)->first == 2);
	ASSERT_TRUE(read.lower_bound(2)->first == 2);
	ASSERT_TRUE(listed.upper_bound(1)->first == 2);
	ASSERT_TRUE(read.upper_bound(1)->first == 2);
	ASSERT_TRUE(listed.equal_range(1).first != listed.equal_range(1).second);
	ASSERT_TRUE(read.equal_range(9).first == read.equal_range(9).second);
	std::map<int, int> caller{{4, 40}};
	Safe::Map<int, int> from_std(caller);
	ASSERT_EQUAL(static_cast<std::size_t>(1), caller.size());
	std::map<int, int> stolen{{5, 50}};
	Safe::Map<int, int> from_moved_std(std::move(stolen));
	ASSERT_TRUE(stolen.empty());
	Safe::Map<int, int> from_range(std::from_range, std::vector<std::pair<int, int>>{{6, 60}});
	ASSERT_EQUAL(60, from_range.at(6));
	Safe::Map<int, int> copy(listed);
	copy = listed;
	Safe::Map<int, int> moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	moved = std::move(listed);
	ASSERT_TRUE(listed.empty());
	moved = caller;
	moved = std::move(caller);
	ASSERT_TRUE(caller.empty());
	moved = {{7, 70}};
	const std::map<int, int> exported(moved);
	ASSERT_EQUAL(static_cast<std::size_t>(1), exported.size());
	ASSERT_THROWS(moved.at(99), Safe::OutOfBoundsError);
	ASSERT_THROWS(read.at(99), Safe::OutOfBoundsError);
	int fresh = 8;
	ASSERT_EQUAL(0, moved[fresh]);
	ASSERT_EQUAL(0, moved[std::move(fresh)]);
	RETURN_TEST(0);
}

// -------------------
// Mutate
// -------------------

int test_insert_extract_merge_and_order() {
	Safe::Map<int, int> values;
	Entry kept(1, 10);
	auto inserted = values.insert(kept);
	ASSERT_TRUE(inserted.second);
	ASSERT_EQUAL(1, inserted.first->first);
	ASSERT_FALSE(values.insert(Entry(1, 11)).second);
	values.insert(values.end(), Entry(3, 30));
	values.insert(values.end(), Entry(4, 40));
	values.insert(std::pair<int, int>(2, 20));
	const Entry extra[] = {Entry(5, 50)};
	values.insert(extra, extra + 1);
	values.insert({Entry(6, 60)});
	values.insert_range(std::vector<std::pair<int, int>>{{7, 70}});
	ASSERT_TRUE(values.try_emplace(8, 80).second);
	ASSERT_FALSE(values.try_emplace(8, 81).second);
	int key = 9;
	values.try_emplace(std::move(key), 90);
	values.try_emplace(values.end(), 10, 100);
	int hinted = 11;
	values.try_emplace(values.end(), std::move(hinted), 110);
	ASSERT_TRUE(values.emplace(12, 120).second);
	values.emplace_hint(values.end(), 13, 130);
	ASSERT_FALSE(values.insert_or_assign(1, 12).second);
	ASSERT_EQUAL(12, values.at(1));
	values.insert_or_assign(1, 13);
	int assign_key = 14;
	values.insert_or_assign(std::move(assign_key), 140);
	int assign_key_2 = 15;
	int assign_value = 150;
	values.insert_or_assign(std::move(assign_key_2), std::move(assign_value));
	values.insert_or_assign(values.end(), 16, 160);
	values.insert_or_assign(values.end(), 16, 161);
	int hint_key = 17;
	values.insert_or_assign(values.end(), std::move(hint_key), 170);
	int hint_key_2 = 18;
	int hint_value = 180;
	values.insert_or_assign(values.end(), std::move(hint_key_2), std::move(hint_value));
	auto handle = values.extract(18);
	ASSERT_TRUE(static_cast<bool>(handle));
	ASSERT_FALSE(handle.empty());
	auto rejected = values.insert(std::move(handle));
	ASSERT_TRUE(rejected.inserted);
	ASSERT_TRUE(rejected.node.empty());
	auto again = values.extract(values.find(17));
	values.insert(values.end(), std::move(again));
	Safe::Map<int, int> other{{1, 1}, {19, 190}};
	values.merge(other);
	ASSERT_TRUE(other.contains(1));
	ASSERT_FALSE(other.contains(19));
	ASSERT_TRUE(values.contains(19));
	values.erase(values.find(19));
	values.erase(values.cbegin());
	values.erase(values.begin(), std::next(values.begin()));
	ASSERT_EQUAL(static_cast<std::size_t>(1), values.erase(16));
	ASSERT_EQUAL(static_cast<std::size_t>(0), values.erase(999));
	Safe::Map<int, int> swapped;
	values.swap(swapped);
	swap(values, swapped);
	Safe::Map<int, int> left{{1, 1}};
	Safe::Map<int, int> right{{1, 2}};
	ASSERT_TRUE(left == left);
	ASSERT_TRUE(left != right);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE(left <= left);
	ASSERT_TRUE(right > left);
	ASSERT_TRUE(right >= left);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::less);
	values.clear();
	ASSERT_TRUE(values.empty());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_bidirectional();

	// -------------------
	// Life
	// -------------------
	result += test_construct_copy_move_and_lookup();

	// -------------------
	// Mutate
	// -------------------
	result += test_insert_extract_merge_and_order();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
