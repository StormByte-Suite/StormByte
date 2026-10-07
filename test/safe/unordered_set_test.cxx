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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/unordered_set.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	struct IdentityHash {
		std::size_t operator()(int value) const noexcept {
			return static_cast<std::size_t>(value);
		}
	};

	Safe::UnorderedSet<int> Filled() {
		return Safe::UnorderedSet<int>{1, 6, 7, 6};
	}
}

// -------------------
// Algorithm
// -------------------

int test_algorithm_forward_and_set_ops() {
	const Safe::UnorderedSet<int> values = Filled();
	ASSERT_NOT_EQUAL(values.end(), std::find(values.begin(), values.end(), 1));
	ASSERT_EQUAL(1, std::count(values.begin(), values.end(), 1));
	ASSERT_EQUAL(0, std::count(values.begin(), values.end(), 9));
	int sum = 0;
	std::for_each(values.begin(), values.end(), [&sum](int value) { sum += value; });
	ASSERT_EQUAL(14, sum);
	std::vector<int> left(values.begin(), values.end());
	std::vector<int> right{6, 8};
	std::sort(left.begin(), left.end());
	std::sort(right.begin(), right.end());
	std::vector<int> common;
	std::set_intersection(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(common));
	ASSERT_EQUAL(std::size_t{1}, common.size());
	ASSERT_EQUAL(6, common.front());
	RETURN_TEST(0);
}

// -------------------
// Bucket
// -------------------

int test_bucket_rehash_and_reserve() {
	Safe::UnorderedSet<int> values = Filled();
	ASSERT_TRUE(values.bucket_count() > 0);
	ASSERT_TRUE(values.load_factor() > 0.0f);
	ASSERT_EQUAL(1.0f, values.max_load_factor());
	const auto before = values.bucket_count();
	values.rehash(before + 8);
	ASSERT_TRUE(values.bucket_count() >= before);
	ASSERT_TRUE(values.contains(6));
	values.reserve(64);
	ASSERT_TRUE(values.contains(1));
	values.max_load_factor(0.5f);
	ASSERT_EQUAL(0.5f, values.max_load_factor());
	ASSERT_EQUAL(std::size_t{0}, Safe::UnorderedSet<int>{}.bucket_count());
	RETURN_TEST(0);
}

// -------------------
// Compare
// -------------------

int test_equal_and_predicates() {
	const Safe::UnorderedSet<int> left = Filled();
	Safe::UnorderedSet<int> right{7, 1, 6};
	ASSERT_EQUAL(left, right);
	right.insert(8);
	ASSERT_NOT_EQUAL(left, right);
	ASSERT_TRUE(left.hash_function()(6) == Safe::Hash<int>{}(6));
	ASSERT_TRUE(left.key_eq()(6, 6));
	ASSERT_FALSE(left.key_eq()(6, 7));
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_convert_and_assign() {
	const Safe::UnorderedSet<int> empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(Size{0}, empty.size());
	ASSERT_EQUAL(Size{3}, Filled().size());
	const int raw[] = {4, 4, 5};
	const Safe::UnorderedSet<int> from_range(std::begin(raw), std::end(raw));
	ASSERT_EQUAL(Size{2}, from_range.size());
	const std::unordered_set<int> standard{1, 6, 7};
	const Safe::UnorderedSet<int> from_std(standard);
	ASSERT_EQUAL(Size{3}, from_std.size());
	const std::unordered_set<int> exported(static_cast<std::unordered_set<int>>(from_std));
	ASSERT_EQUAL(std::size_t{3}, exported.size());
	ASSERT_TRUE(exported.contains(6));
	Safe::UnorderedSet<int> assigned;
	assigned = Filled();
	ASSERT_EQUAL(Filled(), assigned);
	Safe::UnorderedSet<int> moved = std::move(assigned);
	ASSERT_EQUAL(Size{3}, moved.size());
	moved = {9, 9, 8};
	ASSERT_EQUAL(Size{2}, moved.size());
	ASSERT_TRUE(moved.contains(9));
	RETURN_TEST(0);
}

int test_construct_iterators() {
	const Safe::UnorderedSet<int> values = Filled();
	ASSERT_EQUAL(Size{3}, Size{static_cast<std::size_t>(std::distance(values.begin(), values.end()))});
	ASSERT_EQUAL(values.begin(), values.cbegin());
	ASSERT_EQUAL(values.end(), values.cend());
	Safe::UnorderedSet<int> mutable_values = values;
	ASSERT_TRUE(mutable_values.contains(*mutable_values.begin()));
	ASSERT_EQUAL(Size{3}, mutable_values.size());
	RETURN_TEST(0);
}

// -------------------
// Erase
// -------------------

int test_erase_clear_and_count() {
	Safe::UnorderedSet<int> values = Filled();
	ASSERT_EQUAL(Size{1}, values.erase(6));
	ASSERT_EQUAL(Size{0}, values.erase(6));
	ASSERT_FALSE(values.contains(6));
	ASSERT_EQUAL(Size{1}, values.count(1));
	ASSERT_EQUAL(Size{0}, values.count(6));
	const auto erased = values.erase(values.find(1));
	ASSERT_FALSE(values.contains(1));
	ASSERT_EQUAL(Size{1}, values.size());
	if (erased != values.end())
		ASSERT_NOT_EQUAL(1, *erased);
	auto after = values.erase(values.begin(), values.end());
	ASSERT_EQUAL(values.end(), after);
	ASSERT_TRUE(values.empty());
	values.clear();
	ASSERT_TRUE(values.empty());
	ASSERT_EQUAL(values.end(), values.erase(values.end(), values.end()));
	RETURN_TEST(0);
}

// -------------------
// Extract
// -------------------

int test_extract_insert_node_and_merge() {
	Safe::UnorderedSet<int> values = Filled();
	auto node = values.extract(6);
	ASSERT_FALSE(node.empty());
	ASSERT_EQUAL(6, node.value());
	ASSERT_FALSE(values.contains(6));
	auto inserted = values.insert(std::move(node));
	ASSERT_TRUE(inserted.inserted);
	ASSERT_EQUAL(6, *inserted.position);
	ASSERT_TRUE(values.insert(values.extract(99)).node.empty());
	Safe::UnorderedSet<int, IdentityHash> other{6, 8};
	values.merge(other);
	ASSERT_TRUE(values.contains(8));
	ASSERT_TRUE(other.contains(6));
	ASSERT_FALSE(other.contains(8));
	Safe::UnorderedSet<int> same_hash{9};
	values.merge(same_hash);
	ASSERT_TRUE(values.contains(9));
	ASSERT_TRUE(same_hash.empty());
	RETURN_TEST(0);
}

// -------------------
// Insert
// -------------------

int test_insert_emplace_and_range() {
	Safe::UnorderedSet<int> values;
	const auto first = values.insert(4);
	ASSERT_TRUE(first.second);
	ASSERT_EQUAL(4, *first.first);
	ASSERT_FALSE(values.insert(4).second);
	const auto emplaced = values.emplace(5);
	ASSERT_TRUE(emplaced.second);
	ASSERT_EQUAL(5, *emplaced.first);
	const auto hinted = values.insert(values.begin(), 8);
	ASSERT_EQUAL(8, *hinted);
	const int raw[] = {8, 9};
	values.insert(std::begin(raw), std::end(raw));
	values.insert({9, 10});
	ASSERT_EQUAL(Size{5}, values.size());
	values.insert_range(std::vector<int>{10, 11});
	ASSERT_TRUE(values.contains(11));
	RETURN_TEST(0);
}

// -------------------
// Lookup
// -------------------

int test_lookup_bounds_and_equal_range() {
	const Safe::UnorderedSet<int> values = Filled();
	ASSERT_NOT_EQUAL(values.end(), values.find(7));
	ASSERT_EQUAL(values.end(), values.find(9));
	ASSERT_TRUE(values.contains(1));
	const auto range = values.equal_range(6);
	ASSERT_NOT_EQUAL(range.first, range.second);
	ASSERT_EQUAL(6, *range.first);
	const auto missing = values.equal_range(9);
	ASSERT_EQUAL(missing.first, missing.second);
	ASSERT_TRUE(values.bucket(1) < values.bucket_count());
	RETURN_TEST(0);
}

// -------------------
// Swap
// -------------------

int test_swap_member_and_free() {
	Safe::UnorderedSet<int> left = Filled();
	Safe::UnorderedSet<int> right{9};
	left.swap(right);
	ASSERT_TRUE(left.contains(9));
	ASSERT_TRUE(right.contains(6));
	swap(left, right);
	ASSERT_EQUAL(Filled(), left);
	Safe::UnorderedSet<int> empty;
	swap(left, empty);
	ASSERT_TRUE(left.empty());
	ASSERT_EQUAL(Size{3}, empty.size());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_forward_and_set_ops();

	// -------------------
	// Bucket
	// -------------------
	result += test_bucket_rehash_and_reserve();

	// -------------------
	// Compare
	// -------------------
	result += test_equal_and_predicates();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_convert_and_assign();
	result += test_construct_iterators();

	// -------------------
	// Erase
	// -------------------
	result += test_erase_clear_and_count();

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
	result += test_lookup_bounds_and_equal_range();

	// -------------------
	// Swap
	// -------------------
	result += test_swap_member_and_free();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
