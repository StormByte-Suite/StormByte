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
#include <StormByte/safe/unordered_map.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace StormByte;

using Table = Safe::UnorderedMap<int, int>;

// -------------------
// Algorithm
// -------------------

int test_algorithm_walks_every_bucket() {
	Table table{{Table::value_type(1, 10), Table::value_type(2, 20), Table::value_type(3, 30), Table::value_type(4, 40)}};
	std::vector<int> keys;
	std::for_each(table.begin(), table.end(), [&keys](const Table::value_type& entry) {
		keys.push_back(entry.first);
	});
	ASSERT_EQUAL(table.size(), keys.size());
	ASSERT_EQUAL(std::distance(table.begin(), table.end()), static_cast<std::ptrdiff_t>(table.size()));
	ASSERT_TRUE(std::find_if(table.begin(), table.end(), [](const Table::value_type& entry) { return entry.first == 3; }) != table.end());
	ASSERT_EQUAL(1u, static_cast<unsigned>(std::count_if(table.begin(), table.end(), [](const Table::value_type& entry) { return entry.second == 20; })));
	ASSERT_TRUE(std::any_of(table.cbegin(), table.cend(), [](const Table::value_type& entry) { return entry.first == 1; }));
	ASSERT_FALSE(std::none_of(table.cbegin(), table.cend(), [](const Table::value_type& entry) { return entry.first == 4; }));
	int sum = 0;
	std::for_each(table.begin(), table.end(), [&sum](Table::value_type& entry) { sum += entry.second; });
	ASSERT_EQUAL(100, sum);
	RETURN_TEST(0);
}

// -------------------
// Assign
// -------------------

int test_assign_copy_move_and_list() {
	Table source{{Table::value_type(1, 2), Table::value_type(3, 4)}};
	Table copied;
	copied = source;
	ASSERT_EQUAL(source.size(), copied.size());
	ASSERT_TRUE(copied == source);
	Table moved;
	moved = std::move(copied);
	ASSERT_TRUE(copied.empty());
	ASSERT_EQUAL(2u, moved.size());
	moved = {Table::value_type(8, 9)};
	ASSERT_EQUAL(1u, moved.size());
	ASSERT_EQUAL(9, moved.at(8));
	RETURN_TEST(0);
}

// -------------------
// Bucket
// -------------------

int test_bucket_load_rehash_and_reserve() {
	Table table(1);
	ASSERT_TRUE(table.bucket_count() >= 8);
	ASSERT_EQUAL(0.0f, table.load_factor());
	table.max_load_factor(2.0f);
	ASSERT_EQUAL(2.0f, table.max_load_factor());
	table.insert(Table::value_type(1, 1));
	table.insert(Table::value_type(2, 2));
	ASSERT_EQUAL(table.bucket(1), table.hash_function()(1) & (table.bucket_count() - 1));
	ASSERT_TRUE(table.bucket_size(table.bucket(1)) >= 1);
	const auto before = table.bucket_count();
	table.rehash(before * 4);
	ASSERT_TRUE(table.bucket_count() >= before * 4);
	ASSERT_TRUE(table.contains(1));
	ASSERT_TRUE(table.contains(2));
	table.reserve(1000);
	ASSERT_TRUE(table.bucket_count() >= 500);
	ASSERT_EQUAL(table.hash_function()(1), Safe::Hash<int>{}(1));
	ASSERT_TRUE(table.key_eq()(1, 1));
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_copy_move_std_and_range() {
	Table empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(0u, empty.size());
	ASSERT_TRUE(empty.max_size() > 0);
	Table listed{{Table::value_type(1, 10), Table::value_type(2, 20)}};
	Table copied(listed);
	ASSERT_EQUAL(10, copied.at(1));
	Table moved(std::move(copied));
	ASSERT_TRUE(copied.empty());
	ASSERT_EQUAL(20, moved.at(2));
	std::unordered_map<int, int> standard{{4, 40}, {5, 50}};
	Table from_std(standard);
	ASSERT_EQUAL(2u, standard.size());
	ASSERT_EQUAL(40, from_std.at(4));
	Table from_moved(std::move(standard));
	ASSERT_TRUE(standard.empty());
	ASSERT_EQUAL(50, from_moved.at(5));
	std::vector<Table::value_type> range{Table::value_type(7, 70)};
	Table from_range(std::from_range, range);
	ASSERT_EQUAL(70, from_range.at(7));
	auto converted = static_cast<std::unordered_map<int, int>>(moved);
	ASSERT_EQUAL(2u, converted.size());
	ASSERT_EQUAL(10, converted.at(1));
	RETURN_TEST(0);
}

// -------------------
// Erase
// -------------------

int test_erase_key_iterator_and_clear() {
	Table table{{Table::value_type(1, 10), Table::value_type(2, 20), Table::value_type(3, 30)}};
	ASSERT_EQUAL(1u, table.erase(2));
	ASSERT_FALSE(table.contains(2));
	ASSERT_EQUAL(0u, table.erase(2));
	auto next = table.erase(table.find(1));
	ASSERT_TRUE(next == table.end() || next->first == 3);
	ASSERT_FALSE(table.contains(1));
	table.clear();
	ASSERT_TRUE(table.empty());
	ASSERT_EQUAL(table.begin(), table.end());
	RETURN_TEST(0);
}

// -------------------
// Find
// -------------------

int test_find_count_contains_and_at() {
	Table table{{Table::value_type(1, 10)}};
	ASSERT_TRUE(table.find(1) != table.end());
	ASSERT_EQUAL(table.find(9), table.end());
	const Table& read = table;
	ASSERT_EQUAL(10, read.find(1)->second);
	ASSERT_EQUAL(1u, table.count(1));
	ASSERT_EQUAL(0u, table.count(9));
	ASSERT_TRUE(table.contains(1));
	ASSERT_FALSE(table.contains(9));
	ASSERT_EQUAL(10, table.at(1));
	ASSERT_EQUAL(10, read.at(1));
	ASSERT_THROWS(table.at(9), Safe::OutOfBoundsError);
	ASSERT_THROWS(read.at(9), Safe::OutOfBoundsError);
	RETURN_TEST(0);
}

// -------------------
// Insert
// -------------------

int test_insert_emplace_assign_and_index() {
	Table table;
	auto inserted = table.insert(Table::value_type(1, 10));
	ASSERT_TRUE(inserted.second);
	ASSERT_EQUAL(10, inserted.first->second);
	auto duplicate = table.insert(Table::value_type(1, 99));
	ASSERT_FALSE(duplicate.second);
	ASSERT_EQUAL(10, table.at(1));
	table.insert({Table::value_type(2, 20), Table::value_type(3, 30)});
	ASSERT_EQUAL(3u, table.size());
	std::vector<Table::value_type> extra{Table::value_type(4, 40)};
	table.insert_range(extra);
	ASSERT_TRUE(table.contains(4));
	auto emplaced = table.emplace(5, 50);
	ASSERT_TRUE(emplaced.second);
	auto tried = table.try_emplace(5, 500);
	ASSERT_FALSE(tried.second);
	ASSERT_EQUAL(50, table.at(5));
	auto replaced = table.insert_or_assign(5, 55);
	ASSERT_FALSE(replaced.second);
	ASSERT_EQUAL(55, table.at(5));
	table[6] = 60;
	ASSERT_EQUAL(60, table.at(6));
	table[6] = 61;
	ASSERT_EQUAL(61, table[6]);
	RETURN_TEST(0);
}

// -------------------
// Node
// -------------------

int test_node_extract_insert_and_merge() {
	Table table{{Table::value_type(1, 10), Table::value_type(2, 20)}};
	Table::node_type extracted = table.extract(1);
	ASSERT_FALSE(extracted.empty());
	ASSERT_TRUE(static_cast<bool>(extracted));
	ASSERT_EQUAL(1, extracted.key());
	ASSERT_EQUAL(10, extracted.mapped());
	extracted.mapped() = 11;
	ASSERT_FALSE(table.contains(1));
	Table::node_type missing = table.extract(9);
	ASSERT_TRUE(missing.empty());
	auto back = table.insert(std::move(extracted));
	ASSERT_TRUE(back.inserted);
	ASSERT_TRUE(extracted.empty());
	ASSERT_EQUAL(11, table.at(1));
	Table::node_type again = table.extract(table.find(1));
	Table other{{Table::value_type(1, 99), Table::value_type(3, 30)}};
	auto rejected = other.insert(std::move(again));
	ASSERT_FALSE(rejected.inserted);
	ASSERT_FALSE(rejected.node.empty());
	ASSERT_EQUAL(99, other.at(1));
	Table donor{{Table::value_type(1, 1), Table::value_type(4, 40)}};
	other.merge(donor);
	ASSERT_TRUE(other.contains(4));
	ASSERT_TRUE(donor.contains(1));
	ASSERT_FALSE(donor.contains(4));
	Table::node_type left = other.extract(3);
	Table::node_type right = other.extract(4);
	left.swap(right);
	ASSERT_EQUAL(4, left.key());
	ASSERT_EQUAL(3, right.key());
	RETURN_TEST(0);
}

// -------------------
// Order
// -------------------

int test_order_swap_and_equality() {
	Table left{{Table::value_type(1, 10), Table::value_type(2, 20)}};
	Table same{{Table::value_type(2, 20), Table::value_type(1, 10)}};
	Table different{{Table::value_type(1, 10), Table::value_type(2, 21)}};
	ASSERT_TRUE(left == same);
	ASSERT_TRUE(left != different);
	left.swap(different);
	ASSERT_EQUAL(21, left.at(2));
	swap(left, different);
	ASSERT_EQUAL(20, left.at(2));
	ASSERT_EQUAL(left.cbegin(), left.begin());
	ASSERT_EQUAL(left.cend(), left.end());
	RETURN_TEST(0);
}

int test_const_key_import_and_standard_pairs() {
	using Strings = Safe::UnorderedMap<Safe::String, int>;
	Strings values;
	Strings::value_type entry(Safe::String("original key with heap storage"), 10);
	ASSERT_TRUE(values.insert(std::move(entry)).second);
	ASSERT_TRUE(entry.first == Safe::String("original key with heap storage"));
	std::unordered_map<Safe::String, int> standard;
	standard.emplace(Safe::String("imported key with heap storage"), 20);
	Strings imported(std::move(standard));
	ASSERT_TRUE(standard.empty());
	ASSERT_EQUAL(20, imported.at(Safe::String("imported key with heap storage")));
	Table integers;
	std::pair<int, int> pair(1, 10);
	ASSERT_TRUE(integers.insert(pair).second);
	ASSERT_EQUAL(10, pair.second);
	ASSERT_TRUE(integers.insert(std::pair<int, int>(2, 20)).second);
	ASSERT_FALSE(integers.insert(std::pair<int, int>(1, 99)).second);
	ASSERT_EQUAL(10, integers.at(1));
	RETURN_TEST(0);
}

int test_stateful_assignment_and_node_transfer() {
	struct Hash {
		std::size_t seed = 0;
		std::size_t operator()(int key) const { return static_cast<std::size_t>(key) ^ seed; }
	};
	struct Equal {
		int state = 0;
		bool operator()(int left, int right) const { return left == right; }
	};
	using Stateful = Safe::UnorderedMap<int, int, Hash, Equal>;
	Stateful source(8, Hash{7}, Equal{3});
	source.max_load_factor(0.5f);
	source = {Stateful::value_type(1, 10), Stateful::value_type(2, 20)};
	ASSERT_EQUAL(std::size_t{7}, source.hash_function().seed);
	ASSERT_EQUAL(3, source.key_eq().state);
	ASSERT_EQUAL(0.5f, source.max_load_factor());
	Stateful target(8, Hash{19}, Equal{4});
	target.emplace(1, 99);
	auto handle = source.extract(1);
	auto rejected = target.insert(std::move(handle));
	ASSERT_FALSE(rejected.inserted);
	ASSERT_TRUE(handle.empty());
	ASSERT_FALSE(rejected.node.empty());
	ASSERT_EQUAL(10, rejected.node.mapped());
	ASSERT_TRUE(source.insert(std::move(rejected.node)).inserted);
	auto transferred = target.insert(source.extract(2));
	ASSERT_TRUE(transferred.inserted);
	ASSERT_EQUAL(20, target.at(2));
	source.emplace(3, 30);
	target.merge(source);
	ASSERT_EQUAL(30, target.at(3));
	ASSERT_TRUE(source.contains(1));
	ASSERT_FALSE(source.contains(3));
	RETURN_TEST(0);
}

int test_iterator_stability_and_standard_model() {
	Table values;
	values.reserve(128);
	values.emplace(100, 1000);
	auto stable = values.find(100);
	auto* address = &*stable;
	std::unordered_map<int, int> model{{100, 1000}};
	for (int index = 0; index < 96; ++index) {
		const int key = (index * 37) % 61;
		ASSERT_EQUAL(model.emplace(key, index).second, values.insert(std::pair<int, int>(key, index)).second);
	}
	for (int key = 0; key < 61; key += 2) ASSERT_EQUAL(model.erase(key), values.erase(key));
	ASSERT_TRUE(&*stable == address);
	ASSERT_EQUAL(1000, stable->second);
	values.rehash(values.bucket_count() * 2);
	ASSERT_TRUE(&*values.find(100) == address);
	ASSERT_EQUAL(model.size(), values.size());
	for (const auto& entry : model) ASSERT_EQUAL(entry.second, values.at(entry.first));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_walks_every_bucket();

	// -------------------
	// Assign
	// -------------------
	result += test_assign_copy_move_and_list();

	// -------------------
	// Bucket
	// -------------------
	result += test_bucket_load_rehash_and_reserve();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_copy_move_std_and_range();

	// -------------------
	// Erase
	// -------------------
	result += test_erase_key_iterator_and_clear();

	// -------------------
	// Find
	// -------------------
	result += test_find_count_contains_and_at();

	// -------------------
	// Insert
	// -------------------
	result += test_insert_emplace_assign_and_index();
	result += test_const_key_import_and_standard_pairs();
	result += test_stateful_assignment_and_node_transfer();
	result += test_iterator_stability_and_standard_model();

	// -------------------
	// Node
	// -------------------
	result += test_node_extract_insert_and_merge();

	// -------------------
	// Order
	// -------------------
	result += test_order_swap_and_equality();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
