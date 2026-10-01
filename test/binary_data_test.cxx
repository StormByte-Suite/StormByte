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

#include <StormByte/binary_data.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/serializable.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using StormByte::BinaryData;
using StormByte::ByteSize;
using StormByte::OutOfBoundsError;
using StormByte::Size;

namespace {
	BinaryData Bytes(std::initializer_list<unsigned char> list) {
		BinaryData data;
		data.reserve(ByteSize{list.size()});
		for (unsigned char value : list)
			data.push_back(static_cast<std::byte>(value));
		return data;
	}

	bool SameBytes(const BinaryData& data, std::initializer_list<unsigned char> list) {
		if (data.size() != ByteSize{list.size()})
			return false;
		auto it = data.begin();
		for (unsigned char value : list) {
			if (*it != static_cast<std::byte>(value))
				return false;
			++it;
		}
		return true;
	}
}

// -------------------
// Algorithms — binary search
// -------------------

int test_binary_data_algorithm_binary_search() {
	BinaryData data = Bytes({1, 3, 5, 7});
	ASSERT_TRUE("test_binary_data_algorithm_binary_search",
		std::binary_search(data.begin(), data.end(), std::byte{5}));
	ASSERT_TRUE("test_binary_data_algorithm_binary_search",
		!std::binary_search(data.begin(), data.end(), std::byte{4}));
	RETURN_TEST("test_binary_data_algorithm_binary_search", 0);
}

int test_binary_data_algorithm_equal_range() {
	BinaryData data = Bytes({1, 2, 2, 2, 3});
	const auto [lo, hi] = std::equal_range(data.begin(), data.end(), std::byte{2});
	ASSERT_EQUAL("test_binary_data_algorithm_equal_range", 3, hi - lo);
	ASSERT_TRUE("test_binary_data_algorithm_equal_range", lo == data.begin() + 1);
	RETURN_TEST("test_binary_data_algorithm_equal_range", 0);
}

int test_binary_data_algorithm_lower_bound() {
	BinaryData data = Bytes({1, 3, 5, 7});
	auto it = std::lower_bound(data.begin(), data.end(), std::byte{5});
	ASSERT_TRUE("test_binary_data_algorithm_lower_bound", it == data.begin() + 2);
	RETURN_TEST("test_binary_data_algorithm_lower_bound", 0);
}

int test_binary_data_algorithm_upper_bound() {
	BinaryData data = Bytes({1, 3, 5, 7});
	auto it = std::upper_bound(data.begin(), data.end(), std::byte{5});
	ASSERT_TRUE("test_binary_data_algorithm_upper_bound", it == data.begin() + 3);
	RETURN_TEST("test_binary_data_algorithm_upper_bound", 0);
}

// -------------------
// Algorithms — comparison
// -------------------

int test_binary_data_algorithm_equal() {
	BinaryData a = Bytes({9, 8, 7});
	BinaryData b = Bytes({9, 8, 7});
	ASSERT_TRUE("test_binary_data_algorithm_equal", std::equal(a.begin(), a.end(), b.begin(), b.end()));
	RETURN_TEST("test_binary_data_algorithm_equal", 0);
}

int test_binary_data_algorithm_lexicographical_compare() {
	BinaryData a = Bytes({1, 2});
	BinaryData b = Bytes({1, 3});
	ASSERT_TRUE("test_binary_data_algorithm_lexicographical_compare",
		std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end()));
	RETURN_TEST("test_binary_data_algorithm_lexicographical_compare", 0);
}

int test_binary_data_algorithm_lexicographical_compare_three_way() {
	BinaryData a = Bytes({1, 2});
	BinaryData b = Bytes({1, 2});
	ASSERT_TRUE("test_binary_data_algorithm_lexicographical_compare_three_way",
		std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end())
			== std::strong_ordering::equal);
	RETURN_TEST("test_binary_data_algorithm_lexicographical_compare_three_way", 0);
}

int test_binary_data_algorithm_mismatch() {
	BinaryData a = Bytes({1, 2, 3, 4});
	BinaryData b = Bytes({1, 2, 9, 4});
	const auto [ia, ib] = std::mismatch(a.begin(), a.end(), b.begin());
	ASSERT_TRUE("test_binary_data_algorithm_mismatch", *ia == std::byte{3});
	ASSERT_TRUE("test_binary_data_algorithm_mismatch", *ib == std::byte{9});
	RETURN_TEST("test_binary_data_algorithm_mismatch", 0);
}

// -------------------
// Algorithms — heap
// -------------------

int test_binary_data_algorithm_heap() {
	BinaryData data = Bytes({3, 1, 4, 2});
	std::make_heap(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_heap", std::is_heap(data.begin(), data.end()));
	ASSERT_TRUE("test_binary_data_algorithm_heap", data.front() == std::byte{4});
	std::pop_heap(data.begin(), data.end());
	data.pop_back();
	ASSERT_TRUE("test_binary_data_algorithm_heap", std::is_heap(data.begin(), data.end()));
	data.push_back(std::byte{5});
	std::push_heap(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_heap", data.front() == std::byte{5});
	std::sort_heap(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_heap", SameBytes(data, {1, 2, 3, 5}));
	RETURN_TEST("test_binary_data_algorithm_heap", 0);
}

int test_binary_data_algorithm_is_heap_until() {
	BinaryData data = Bytes({5, 3, 4, 1, 9});
	std::make_heap(data.begin(), data.begin() + 4);
	auto it = std::is_heap_until(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_is_heap_until", it == data.begin() + 4);
	RETURN_TEST("test_binary_data_algorithm_is_heap_until", 0);
}

// -------------------
// Algorithms — min / max
// -------------------

int test_binary_data_algorithm_max_element() {
	BinaryData data = Bytes({4, 1, 9, 3});
	auto it = std::max_element(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_max_element", *it == std::byte{9});
	RETURN_TEST("test_binary_data_algorithm_max_element", 0);
}

int test_binary_data_algorithm_min_element() {
	BinaryData data = Bytes({4, 1, 9, 3});
	auto it = std::min_element(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_min_element", *it == std::byte{1});
	RETURN_TEST("test_binary_data_algorithm_min_element", 0);
}

int test_binary_data_algorithm_minmax_element() {
	BinaryData data = Bytes({4, 1, 9, 3});
	const auto [lo, hi] = std::minmax_element(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_minmax_element", *lo == std::byte{1});
	ASSERT_TRUE("test_binary_data_algorithm_minmax_element", *hi == std::byte{9});
	RETURN_TEST("test_binary_data_algorithm_minmax_element", 0);
}

// -------------------
// Algorithms — modifying
// -------------------

int test_binary_data_algorithm_copy() {
	BinaryData source = Bytes({1, 2, 3, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	std::copy(source.begin(), source.end(), dest.begin());
	ASSERT_TRUE("test_binary_data_algorithm_copy", source == dest);
	RETURN_TEST("test_binary_data_algorithm_copy", 0);
}

int test_binary_data_algorithm_copy_backward() {
	BinaryData data = Bytes({1, 2, 3, 0, 0});
	std::copy_backward(data.begin(), data.begin() + 3, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_copy_backward", SameBytes(data, {1, 2, 1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_copy_backward", 0);
}

int test_binary_data_algorithm_copy_if() {
	BinaryData source = Bytes({1, 2, 3, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	auto last = std::copy_if(source.begin(), source.end(), dest.begin(),
		[](std::byte b) { return static_cast<unsigned>(b) % 2u == 0u; });
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_copy_if", SameBytes(dest, {2, 4}));
	RETURN_TEST("test_binary_data_algorithm_copy_if", 0);
}

int test_binary_data_algorithm_copy_n() {
	BinaryData source = Bytes({1, 2, 3, 4});
	BinaryData dest(ByteSize{2}, std::byte{0});
	std::copy_n(source.begin(), 2, dest.begin());
	ASSERT_TRUE("test_binary_data_algorithm_copy_n", SameBytes(dest, {1, 2}));
	RETURN_TEST("test_binary_data_algorithm_copy_n", 0);
}

int test_binary_data_algorithm_fill() {
	BinaryData data(ByteSize{4}, std::byte{0});
	std::fill(data.begin(), data.end(), std::byte{0xAB});
	ASSERT_TRUE("test_binary_data_algorithm_fill", SameBytes(data, {0xAB, 0xAB, 0xAB, 0xAB}));
	RETURN_TEST("test_binary_data_algorithm_fill", 0);
}

int test_binary_data_algorithm_fill_n() {
	BinaryData data(ByteSize{4}, std::byte{0});
	std::fill_n(data.begin(), 2, std::byte{9});
	ASSERT_TRUE("test_binary_data_algorithm_fill_n", SameBytes(data, {9, 9, 0, 0}));
	RETURN_TEST("test_binary_data_algorithm_fill_n", 0);
}

int test_binary_data_algorithm_generate() {
	BinaryData data(ByteSize{3}, std::byte{0});
	unsigned n = 1;
	std::generate(data.begin(), data.end(), [&n] {
		return static_cast<std::byte>(n++);
	});
	ASSERT_TRUE("test_binary_data_algorithm_generate", SameBytes(data, {1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_generate", 0);
}

int test_binary_data_algorithm_generate_n() {
	BinaryData data(ByteSize{4}, std::byte{0});
	unsigned n = 5;
	std::generate_n(data.begin(), 2, [&n] {
		return static_cast<std::byte>(n++);
	});
	ASSERT_TRUE("test_binary_data_algorithm_generate_n", SameBytes(data, {5, 6, 0, 0}));
	RETURN_TEST("test_binary_data_algorithm_generate_n", 0);
}

int test_binary_data_algorithm_iter_swap() {
	BinaryData data = Bytes({1, 2});
	std::iter_swap(data.begin(), data.begin() + 1);
	ASSERT_TRUE("test_binary_data_algorithm_iter_swap", SameBytes(data, {2, 1}));
	RETURN_TEST("test_binary_data_algorithm_iter_swap", 0);
}

int test_binary_data_algorithm_move() {
	BinaryData source = Bytes({1, 2, 3});
	BinaryData dest(ByteSize{3}, std::byte{0});
	std::move(source.begin(), source.end(), dest.begin());
	ASSERT_TRUE("test_binary_data_algorithm_move", SameBytes(dest, {1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_move", 0);
}

int test_binary_data_algorithm_move_backward() {
	BinaryData data = Bytes({1, 2, 3, 0});
	std::move_backward(data.begin(), data.begin() + 3, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_move_backward", SameBytes(data, {1, 1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_move_backward", 0);
}

int test_binary_data_algorithm_remove() {
	BinaryData data = Bytes({1, 2, 1, 3, 1});
	auto last = std::remove(data.begin(), data.end(), std::byte{1});
	data.erase(last, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_remove", SameBytes(data, {2, 3}));
	RETURN_TEST("test_binary_data_algorithm_remove", 0);
}

int test_binary_data_algorithm_remove_copy() {
	BinaryData source = Bytes({1, 2, 1, 3});
	BinaryData dest(ByteSize{4}, std::byte{0});
	auto last = std::remove_copy(source.begin(), source.end(), dest.begin(), std::byte{1});
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_remove_copy", SameBytes(dest, {2, 3}));
	RETURN_TEST("test_binary_data_algorithm_remove_copy", 0);
}

int test_binary_data_algorithm_remove_if() {
	BinaryData data = Bytes({1, 2, 3, 4});
	auto last = std::remove_if(data.begin(), data.end(),
		[](std::byte b) { return static_cast<unsigned>(b) % 2u == 0u; });
	data.erase(last, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_remove_if", SameBytes(data, {1, 3}));
	RETURN_TEST("test_binary_data_algorithm_remove_if", 0);
}

int test_binary_data_algorithm_replace() {
	BinaryData data = Bytes({1, 2, 1, 3});
	std::replace(data.begin(), data.end(), std::byte{1}, std::byte{9});
	ASSERT_TRUE("test_binary_data_algorithm_replace", SameBytes(data, {9, 2, 9, 3}));
	RETURN_TEST("test_binary_data_algorithm_replace", 0);
}

int test_binary_data_algorithm_replace_copy() {
	BinaryData source = Bytes({1, 2, 1});
	BinaryData dest(ByteSize{3}, std::byte{0});
	std::replace_copy(source.begin(), source.end(), dest.begin(), std::byte{1}, std::byte{9});
	ASSERT_TRUE("test_binary_data_algorithm_replace_copy", SameBytes(dest, {9, 2, 9}));
	RETURN_TEST("test_binary_data_algorithm_replace_copy", 0);
}

int test_binary_data_algorithm_replace_if() {
	BinaryData data = Bytes({1, 2, 3, 4});
	std::replace_if(data.begin(), data.end(),
		[](std::byte b) { return static_cast<unsigned>(b) > 2u; },
		std::byte{0});
	ASSERT_TRUE("test_binary_data_algorithm_replace_if", SameBytes(data, {1, 2, 0, 0}));
	RETURN_TEST("test_binary_data_algorithm_replace_if", 0);
}

int test_binary_data_algorithm_reverse() {
	BinaryData data = Bytes({1, 2, 3, 4});
	std::reverse(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_reverse", SameBytes(data, {4, 3, 2, 1}));
	RETURN_TEST("test_binary_data_algorithm_reverse", 0);
}

int test_binary_data_algorithm_reverse_copy() {
	BinaryData source = Bytes({1, 2, 3});
	BinaryData dest(ByteSize{3}, std::byte{0});
	std::reverse_copy(source.begin(), source.end(), dest.begin());
	ASSERT_TRUE("test_binary_data_algorithm_reverse_copy", SameBytes(dest, {3, 2, 1}));
	RETURN_TEST("test_binary_data_algorithm_reverse_copy", 0);
}

int test_binary_data_algorithm_rotate() {
	BinaryData data = Bytes({1, 2, 3, 4});
	std::rotate(data.begin(), data.begin() + 1, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_rotate", SameBytes(data, {2, 3, 4, 1}));
	RETURN_TEST("test_binary_data_algorithm_rotate", 0);
}

int test_binary_data_algorithm_rotate_copy() {
	BinaryData source = Bytes({1, 2, 3, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	std::rotate_copy(source.begin(), source.begin() + 2, source.end(), dest.begin());
	ASSERT_TRUE("test_binary_data_algorithm_rotate_copy", SameBytes(dest, {3, 4, 1, 2}));
	RETURN_TEST("test_binary_data_algorithm_rotate_copy", 0);
}

int test_binary_data_algorithm_shift() {
	BinaryData left = Bytes({1, 2, 3, 4});
	std::shift_left(left.begin(), left.end(), 1);
	left.pop_back();
	ASSERT_TRUE("test_binary_data_algorithm_shift", SameBytes(left, {2, 3, 4}));
	BinaryData right = Bytes({1, 2, 3, 4});
	std::shift_right(right.begin(), right.end(), 1);
	ASSERT_TRUE("test_binary_data_algorithm_shift", right[ByteSize{1}] == std::byte{1});
	ASSERT_TRUE("test_binary_data_algorithm_shift", right[ByteSize{2}] == std::byte{2});
	ASSERT_TRUE("test_binary_data_algorithm_shift", right[ByteSize{3}] == std::byte{3});
	RETURN_TEST("test_binary_data_algorithm_shift", 0);
}

int test_binary_data_algorithm_swap_ranges() {
	BinaryData a = Bytes({1, 2, 3});
	BinaryData b = Bytes({4, 5, 6});
	std::swap_ranges(a.begin(), a.end(), b.begin());
	ASSERT_TRUE("test_binary_data_algorithm_swap_ranges", SameBytes(a, {4, 5, 6}));
	ASSERT_TRUE("test_binary_data_algorithm_swap_ranges", SameBytes(b, {1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_swap_ranges", 0);
}

int test_binary_data_algorithm_transform() {
	BinaryData data = Bytes({1, 2, 3});
	std::transform(data.begin(), data.end(), data.begin(),
		[](std::byte b) { return static_cast<std::byte>(static_cast<unsigned>(b) + 1); });
	ASSERT_TRUE("test_binary_data_algorithm_transform", SameBytes(data, {2, 3, 4}));
	RETURN_TEST("test_binary_data_algorithm_transform", 0);
}

int test_binary_data_algorithm_unique() {
	BinaryData data = Bytes({1, 1, 2, 2, 2, 3});
	auto last = std::unique(data.begin(), data.end());
	data.erase(last, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_unique", SameBytes(data, {1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_unique", 0);
}

int test_binary_data_algorithm_unique_copy() {
	BinaryData source = Bytes({1, 1, 2, 2, 3});
	BinaryData dest(ByteSize{5}, std::byte{0});
	auto last = std::unique_copy(source.begin(), source.end(), dest.begin());
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_unique_copy", SameBytes(dest, {1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_unique_copy", 0);
}

// -------------------
// Algorithms — non-modifying
// -------------------

int test_binary_data_algorithm_adjacent_find() {
	BinaryData data = Bytes({1, 2, 2, 3});
	auto it = std::adjacent_find(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_adjacent_find", it == data.begin() + 1);
	RETURN_TEST("test_binary_data_algorithm_adjacent_find", 0);
}

int test_binary_data_algorithm_all_any_none() {
	BinaryData data = Bytes({1, 1, 1});
	ASSERT_TRUE("test_binary_data_algorithm_all_any_none",
		std::all_of(data.begin(), data.end(), [](std::byte b) { return b == std::byte{1}; }));
	ASSERT_TRUE("test_binary_data_algorithm_all_any_none",
		std::any_of(data.begin(), data.end(), [](std::byte b) { return b == std::byte{1}; }));
	ASSERT_TRUE("test_binary_data_algorithm_all_any_none",
		std::none_of(data.begin(), data.end(), [](std::byte b) { return b == std::byte{9}; }));
	RETURN_TEST("test_binary_data_algorithm_all_any_none", 0);
}

int test_binary_data_algorithm_count() {
	BinaryData data = Bytes({1, 2, 1, 1, 3});
	const auto found = std::count(data.begin(), data.end(), std::byte{1});
	ASSERT_EQUAL("test_binary_data_algorithm_count", 3, found);
	RETURN_TEST("test_binary_data_algorithm_count", 0);
}

int test_binary_data_algorithm_count_if() {
	BinaryData data = Bytes({1, 2, 3, 4});
	const auto found = std::count_if(data.begin(), data.end(),
		[](std::byte b) { return static_cast<unsigned>(b) % 2u == 0u; });
	ASSERT_EQUAL("test_binary_data_algorithm_count_if", 2, found);
	RETURN_TEST("test_binary_data_algorithm_count_if", 0);
}

int test_binary_data_algorithm_find() {
	BinaryData data = Bytes({10, 20, 30, 40});
	auto it = std::find(data.begin(), data.end(), std::byte{30});
	ASSERT_TRUE("test_binary_data_algorithm_find", it != data.end());
	ASSERT_TRUE("test_binary_data_algorithm_find", *it == std::byte{30});
	ASSERT_EQUAL("test_binary_data_algorithm_find", 2, it - data.begin());
	RETURN_TEST("test_binary_data_algorithm_find", 0);
}

int test_binary_data_algorithm_find_end() {
	BinaryData hay = Bytes({1, 2, 3, 2, 3, 4});
	BinaryData needle = Bytes({2, 3});
	auto it = std::find_end(hay.begin(), hay.end(), needle.begin(), needle.end());
	ASSERT_TRUE("test_binary_data_algorithm_find_end", it == hay.begin() + 3);
	RETURN_TEST("test_binary_data_algorithm_find_end", 0);
}

int test_binary_data_algorithm_find_first_of() {
	BinaryData hay = Bytes({1, 2, 3, 4});
	BinaryData keys = Bytes({9, 3});
	auto it = std::find_first_of(hay.begin(), hay.end(), keys.begin(), keys.end());
	ASSERT_TRUE("test_binary_data_algorithm_find_first_of", it == hay.begin() + 2);
	RETURN_TEST("test_binary_data_algorithm_find_first_of", 0);
}

int test_binary_data_algorithm_find_if() {
	BinaryData data = Bytes({1, 2, 3});
	auto it = std::find_if(data.begin(), data.end(),
		[](std::byte b) { return b == std::byte{2}; });
	ASSERT_TRUE("test_binary_data_algorithm_find_if", it == data.begin() + 1);
	RETURN_TEST("test_binary_data_algorithm_find_if", 0);
}

int test_binary_data_algorithm_find_if_not() {
	BinaryData data = Bytes({1, 1, 2});
	auto it = std::find_if_not(data.begin(), data.end(),
		[](std::byte b) { return b == std::byte{1}; });
	ASSERT_TRUE("test_binary_data_algorithm_find_if_not", it == data.begin() + 2);
	RETURN_TEST("test_binary_data_algorithm_find_if_not", 0);
}

int test_binary_data_algorithm_for_each() {
	BinaryData data = Bytes({1, 2, 3});
	unsigned sum = 0;
	std::for_each(data.begin(), data.end(), [&sum](std::byte b) {
		sum += static_cast<unsigned>(b);
	});
	ASSERT_EQUAL("test_binary_data_algorithm_for_each", 6u, sum);
	RETURN_TEST("test_binary_data_algorithm_for_each", 0);
}

int test_binary_data_algorithm_for_each_n() {
	BinaryData data = Bytes({1, 2, 3, 4});
	unsigned sum = 0;
	std::for_each_n(data.begin(), 2, [&sum](std::byte b) {
		sum += static_cast<unsigned>(b);
	});
	ASSERT_EQUAL("test_binary_data_algorithm_for_each_n", 3u, sum);
	RETURN_TEST("test_binary_data_algorithm_for_each_n", 0);
}

int test_binary_data_algorithm_search() {
	BinaryData hay = Bytes({1, 2, 3, 4, 5});
	BinaryData needle = Bytes({3, 4});
	auto it = std::search(hay.begin(), hay.end(), needle.begin(), needle.end());
	ASSERT_TRUE("test_binary_data_algorithm_search", it == hay.begin() + 2);
	RETURN_TEST("test_binary_data_algorithm_search", 0);
}

int test_binary_data_algorithm_search_n() {
	BinaryData data = Bytes({1, 2, 2, 2, 3});
	auto it = std::search_n(data.begin(), data.end(), 3, std::byte{2});
	ASSERT_TRUE("test_binary_data_algorithm_search_n", it == data.begin() + 1);
	RETURN_TEST("test_binary_data_algorithm_search_n", 0);
}

// -------------------
// Algorithms — numeric
// -------------------

int test_binary_data_algorithm_adjacent_difference() {
	BinaryData source = Bytes({1, 3, 6, 10});
	BinaryData dest(ByteSize{4}, std::byte{0});
	std::adjacent_difference(source.begin(), source.end(), dest.begin(),
		[](std::byte a, std::byte b) {
			return static_cast<std::byte>(static_cast<unsigned>(a) - static_cast<unsigned>(b));
		});
	ASSERT_TRUE("test_binary_data_algorithm_adjacent_difference", SameBytes(dest, {1, 2, 3, 4}));
	RETURN_TEST("test_binary_data_algorithm_adjacent_difference", 0);
}

int test_binary_data_algorithm_inner_product() {
	BinaryData a = Bytes({1, 2, 3});
	BinaryData b = Bytes({4, 5, 6});
	const unsigned value = std::inner_product(a.begin(), a.end(), b.begin(), 0u,
		std::plus<unsigned>{},
		[](std::byte x, std::byte y) {
			return static_cast<unsigned>(x) * static_cast<unsigned>(y);
		});
	ASSERT_EQUAL("test_binary_data_algorithm_inner_product", 32u, value);
	RETURN_TEST("test_binary_data_algorithm_inner_product", 0);
}

int test_binary_data_algorithm_iota() {
	BinaryData data(ByteSize{4}, std::byte{0});
	unsigned n = 1;
	for (auto it = data.begin(); it != data.end(); ++it)
		*it = static_cast<std::byte>(n++);
	ASSERT_TRUE("test_binary_data_algorithm_iota", SameBytes(data, {1, 2, 3, 4}));
	RETURN_TEST("test_binary_data_algorithm_iota", 0);
}

int test_binary_data_algorithm_numeric_accumulate() {
	BinaryData data = Bytes({1, 2, 3, 4});
	const unsigned sum = std::accumulate(data.begin(), data.end(), 0u,
		[](unsigned acc, std::byte b) { return acc + static_cast<unsigned>(b); });
	ASSERT_EQUAL("test_binary_data_algorithm_numeric_accumulate", 10u, sum);
	RETURN_TEST("test_binary_data_algorithm_numeric_accumulate", 0);
}

int test_binary_data_algorithm_partial_sum() {
	BinaryData source = Bytes({1, 2, 3, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	std::partial_sum(source.begin(), source.end(), dest.begin(),
		[](std::byte a, std::byte b) {
			return static_cast<std::byte>(static_cast<unsigned>(a) + static_cast<unsigned>(b));
		});
	ASSERT_TRUE("test_binary_data_algorithm_partial_sum", SameBytes(dest, {1, 3, 6, 10}));
	RETURN_TEST("test_binary_data_algorithm_partial_sum", 0);
}

// -------------------
// Algorithms — partitioning
// -------------------

int test_binary_data_algorithm_is_partitioned() {
	BinaryData data = Bytes({1, 3, 2, 4});
	const auto odd = [](std::byte b) { return static_cast<unsigned>(b) % 2u == 1u; };
	ASSERT_TRUE("test_binary_data_algorithm_is_partitioned",
		std::is_partitioned(data.begin(), data.end(), odd));
	RETURN_TEST("test_binary_data_algorithm_is_partitioned", 0);
}

int test_binary_data_algorithm_partition() {
	BinaryData data = Bytes({1, 2, 3, 4});
	const auto odd = [](std::byte b) { return static_cast<unsigned>(b) % 2u == 1u; };
	std::partition(data.begin(), data.end(), odd);
	ASSERT_TRUE("test_binary_data_algorithm_partition",
		std::is_partitioned(data.begin(), data.end(), odd));
	RETURN_TEST("test_binary_data_algorithm_partition", 0);
}

int test_binary_data_algorithm_partition_copy() {
	BinaryData source = Bytes({1, 2, 3, 4});
	BinaryData yes(ByteSize{4}, std::byte{0});
	BinaryData no(ByteSize{4}, std::byte{0});
	const auto odd = [](std::byte b) { return static_cast<unsigned>(b) % 2u == 1u; };
	const auto [yend, nend] = std::partition_copy(source.begin(), source.end(), yes.begin(), no.begin(), odd);
	yes.erase(yend, yes.end());
	no.erase(nend, no.end());
	ASSERT_TRUE("test_binary_data_algorithm_partition_copy", SameBytes(yes, {1, 3}));
	ASSERT_TRUE("test_binary_data_algorithm_partition_copy", SameBytes(no, {2, 4}));
	RETURN_TEST("test_binary_data_algorithm_partition_copy", 0);
}

int test_binary_data_algorithm_partition_point() {
	BinaryData data = Bytes({1, 3, 2, 4});
	const auto odd = [](std::byte b) { return static_cast<unsigned>(b) % 2u == 1u; };
	auto it = std::partition_point(data.begin(), data.end(), odd);
	ASSERT_TRUE("test_binary_data_algorithm_partition_point", it == data.begin() + 2);
	RETURN_TEST("test_binary_data_algorithm_partition_point", 0);
}

int test_binary_data_algorithm_stable_partition() {
	BinaryData data = Bytes({1, 2, 3, 4});
	const auto odd = [](std::byte b) { return static_cast<unsigned>(b) % 2u == 1u; };
	std::stable_partition(data.begin(), data.end(), odd);
	ASSERT_TRUE("test_binary_data_algorithm_stable_partition", SameBytes(data, {1, 3, 2, 4}));
	RETURN_TEST("test_binary_data_algorithm_stable_partition", 0);
}

// -------------------
// Algorithms — permutations
// -------------------

int test_binary_data_algorithm_is_permutation() {
	BinaryData a = Bytes({1, 2, 3});
	BinaryData b = Bytes({3, 1, 2});
	ASSERT_TRUE("test_binary_data_algorithm_is_permutation",
		std::is_permutation(a.begin(), a.end(), b.begin(), b.end()));
	RETURN_TEST("test_binary_data_algorithm_is_permutation", 0);
}

int test_binary_data_algorithm_next_prev_permutation() {
	BinaryData data = Bytes({1, 2, 3});
	ASSERT_TRUE("test_binary_data_algorithm_next_prev_permutation",
		std::next_permutation(data.begin(), data.end()));
	ASSERT_TRUE("test_binary_data_algorithm_next_prev_permutation", SameBytes(data, {1, 3, 2}));
	ASSERT_TRUE("test_binary_data_algorithm_next_prev_permutation",
		std::prev_permutation(data.begin(), data.end()));
	ASSERT_TRUE("test_binary_data_algorithm_next_prev_permutation", SameBytes(data, {1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_next_prev_permutation", 0);
}

// -------------------
// Algorithms — ranges
// -------------------

int test_binary_data_algorithm_ranges_sort() {
	BinaryData data = Bytes({4, 1, 3, 2});
	std::ranges::sort(data);
	ASSERT_TRUE("test_binary_data_algorithm_ranges_sort", SameBytes(data, {1, 2, 3, 4}));
	RETURN_TEST("test_binary_data_algorithm_ranges_sort", 0);
}

// -------------------
// Algorithms — set
// -------------------

int test_binary_data_algorithm_includes() {
	BinaryData a = Bytes({1, 2, 3, 4});
	BinaryData b = Bytes({2, 4});
	ASSERT_TRUE("test_binary_data_algorithm_includes",
		std::includes(a.begin(), a.end(), b.begin(), b.end()));
	RETURN_TEST("test_binary_data_algorithm_includes", 0);
}

int test_binary_data_algorithm_merge() {
	BinaryData a = Bytes({1, 3, 5});
	BinaryData b = Bytes({2, 4, 6});
	BinaryData dest(ByteSize{6}, std::byte{0});
	std::merge(a.begin(), a.end(), b.begin(), b.end(), dest.begin());
	ASSERT_TRUE("test_binary_data_algorithm_merge", SameBytes(dest, {1, 2, 3, 4, 5, 6}));
	RETURN_TEST("test_binary_data_algorithm_merge", 0);
}

int test_binary_data_algorithm_inplace_merge() {
	BinaryData data = Bytes({1, 3, 5, 2, 4, 6});
	std::inplace_merge(data.begin(), data.begin() + 3, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_inplace_merge", SameBytes(data, {1, 2, 3, 4, 5, 6}));
	RETURN_TEST("test_binary_data_algorithm_inplace_merge", 0);
}

int test_binary_data_algorithm_set_difference() {
	BinaryData a = Bytes({1, 2, 3, 4});
	BinaryData b = Bytes({2, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	auto last = std::set_difference(a.begin(), a.end(), b.begin(), b.end(), dest.begin());
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_set_difference", SameBytes(dest, {1, 3}));
	RETURN_TEST("test_binary_data_algorithm_set_difference", 0);
}

int test_binary_data_algorithm_set_intersection() {
	BinaryData a = Bytes({1, 2, 3, 4});
	BinaryData b = Bytes({2, 4, 6});
	BinaryData dest(ByteSize{4}, std::byte{0});
	auto last = std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), dest.begin());
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_set_intersection", SameBytes(dest, {2, 4}));
	RETURN_TEST("test_binary_data_algorithm_set_intersection", 0);
}

int test_binary_data_algorithm_set_symmetric_difference() {
	BinaryData a = Bytes({1, 2, 3});
	BinaryData b = Bytes({2, 3, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	auto last = std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(), dest.begin());
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_set_symmetric_difference", SameBytes(dest, {1, 4}));
	RETURN_TEST("test_binary_data_algorithm_set_symmetric_difference", 0);
}

int test_binary_data_algorithm_set_union() {
	BinaryData a = Bytes({1, 3});
	BinaryData b = Bytes({2, 3, 4});
	BinaryData dest(ByteSize{4}, std::byte{0});
	auto last = std::set_union(a.begin(), a.end(), b.begin(), b.end(), dest.begin());
	dest.erase(last, dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_set_union", SameBytes(dest, {1, 2, 3, 4}));
	RETURN_TEST("test_binary_data_algorithm_set_union", 0);
}

// -------------------
// Algorithms — sorting
// -------------------

int test_binary_data_algorithm_is_sorted() {
	BinaryData data = Bytes({1, 2, 3, 4});
	ASSERT_TRUE("test_binary_data_algorithm_is_sorted", std::is_sorted(data.begin(), data.end()));
	RETURN_TEST("test_binary_data_algorithm_is_sorted", 0);
}

int test_binary_data_algorithm_is_sorted_until() {
	BinaryData data = Bytes({1, 2, 4, 3});
	auto it = std::is_sorted_until(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_is_sorted_until", it == data.begin() + 3);
	RETURN_TEST("test_binary_data_algorithm_is_sorted_until", 0);
}

int test_binary_data_algorithm_nth_element() {
	BinaryData data = Bytes({4, 1, 3, 2});
	std::nth_element(data.begin(), data.begin() + 1, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_nth_element", data[ByteSize{1}] == std::byte{2});
	RETURN_TEST("test_binary_data_algorithm_nth_element", 0);
}

int test_binary_data_algorithm_partial_sort() {
	BinaryData data = Bytes({4, 1, 3, 2});
	std::partial_sort(data.begin(), data.begin() + 2, data.end());
	ASSERT_TRUE("test_binary_data_algorithm_partial_sort", data[ByteSize{0}] == std::byte{1});
	ASSERT_TRUE("test_binary_data_algorithm_partial_sort", data[ByteSize{1}] == std::byte{2});
	RETURN_TEST("test_binary_data_algorithm_partial_sort", 0);
}

int test_binary_data_algorithm_partial_sort_copy() {
	BinaryData source = Bytes({4, 1, 3, 2});
	BinaryData dest(ByteSize{2}, std::byte{0});
	std::partial_sort_copy(source.begin(), source.end(), dest.begin(), dest.end());
	ASSERT_TRUE("test_binary_data_algorithm_partial_sort_copy", SameBytes(dest, {1, 2}));
	RETURN_TEST("test_binary_data_algorithm_partial_sort_copy", 0);
}

int test_binary_data_algorithm_sort() {
	BinaryData data = Bytes({9, 1, 5, 3});
	std::sort(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_sort", SameBytes(data, {1, 3, 5, 9}));
	RETURN_TEST("test_binary_data_algorithm_sort", 0);
}

int test_binary_data_algorithm_stable_sort() {
	BinaryData data = Bytes({3, 1, 2, 1});
	std::stable_sort(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_algorithm_stable_sort", SameBytes(data, {1, 1, 2, 3}));
	RETURN_TEST("test_binary_data_algorithm_stable_sort", 0);
}

// -------------------
// Capacity
// -------------------

int test_binary_data_capacity_clear_keeps_capacity() {
	BinaryData data(ByteSize{8}, std::byte{1});
	data.reserve(ByteSize{32});
	const ByteSize cap = data.capacity();
	data.clear();
	ASSERT_TRUE("test_binary_data_capacity_clear_keeps_capacity", data.empty());
	ASSERT_TRUE("test_binary_data_capacity_clear_keeps_capacity", data.capacity() >= cap);
	RETURN_TEST("test_binary_data_capacity_clear_keeps_capacity", 0);
}

int test_binary_data_capacity_max_size() {
	BinaryData data;
	ASSERT_TRUE("test_binary_data_capacity_max_size", data.max_size() > ByteSize{0});
	RETURN_TEST("test_binary_data_capacity_max_size", 0);
}

int test_binary_data_capacity_reserve() {
	BinaryData data;
	data.reserve(ByteSize{64});
	ASSERT_TRUE("test_binary_data_capacity_reserve", data.empty());
	ASSERT_TRUE("test_binary_data_capacity_reserve", data.capacity() >= ByteSize{64});
	RETURN_TEST("test_binary_data_capacity_reserve", 0);
}

int test_binary_data_capacity_resize_grow_zero() {
	BinaryData data = Bytes({1, 2});
	data.resize(ByteSize{4});
	ASSERT_TRUE("test_binary_data_capacity_resize_grow_zero", SameBytes(data, {1, 2, 0, 0}));
	RETURN_TEST("test_binary_data_capacity_resize_grow_zero", 0);
}

int test_binary_data_capacity_resize_grow_value() {
	BinaryData data = Bytes({1});
	data.resize(ByteSize{3}, std::byte{9});
	ASSERT_TRUE("test_binary_data_capacity_resize_grow_value", SameBytes(data, {1, 9, 9}));
	RETURN_TEST("test_binary_data_capacity_resize_grow_value", 0);
}

int test_binary_data_capacity_resize_shrink() {
	BinaryData data = Bytes({1, 2, 3, 4});
	data.resize(ByteSize{2});
	ASSERT_TRUE("test_binary_data_capacity_resize_shrink", SameBytes(data, {1, 2}));
	RETURN_TEST("test_binary_data_capacity_resize_shrink", 0);
}

int test_binary_data_capacity_resize_zero() {
	BinaryData data = Bytes({1, 2, 3});
	data.resize(ByteSize{0});
	ASSERT_TRUE("test_binary_data_capacity_resize_zero", data.empty());
	RETURN_TEST("test_binary_data_capacity_resize_zero", 0);
}

int test_binary_data_capacity_shrink_to_fit() {
	BinaryData data;
	data.reserve(ByteSize{128});
	data.push_back(std::byte{1});
	data.shrink_to_fit();
	ASSERT_TRUE("test_binary_data_capacity_shrink_to_fit", data.size() == ByteSize{1});
	ASSERT_TRUE("test_binary_data_capacity_shrink_to_fit", data.capacity() >= data.size());
	RETURN_TEST("test_binary_data_capacity_shrink_to_fit", 0);
}

// -------------------
// Comparison
// -------------------

int test_binary_data_compare_empty() {
	ASSERT_TRUE("test_binary_data_compare_empty", BinaryData{} == BinaryData{});
	ASSERT_TRUE("test_binary_data_compare_empty", BinaryData{} < Bytes({0}));
	RETURN_TEST("test_binary_data_compare_empty", 0);
}

int test_binary_data_compare_equal() {
	ASSERT_TRUE("test_binary_data_compare_equal", Bytes({1, 2}) == Bytes({1, 2}));
	ASSERT_TRUE("test_binary_data_compare_equal", Bytes({1, 2}) != Bytes({1, 3}));
	RETURN_TEST("test_binary_data_compare_equal", 0);
}

int test_binary_data_compare_order() {
	ASSERT_TRUE("test_binary_data_compare_order", Bytes({1, 2}) < Bytes({1, 3}));
	ASSERT_TRUE("test_binary_data_compare_order", Bytes({1, 2}) < Bytes({1, 2, 0}));
	ASSERT_TRUE("test_binary_data_compare_order", (Bytes({1, 2}) <=> Bytes({1, 2})) == std::strong_ordering::equal);
	ASSERT_TRUE("test_binary_data_compare_order", Bytes({2}) > Bytes({1, 9}));
	RETURN_TEST("test_binary_data_compare_order", 0);
}

int test_binary_data_compare_span_equal() {
	BinaryData data = Bytes({1, 2, 3});
	const std::byte raw[] = { std::byte{1}, std::byte{2}, std::byte{3} };
	const std::span<const std::byte> view{raw, 3};
	ASSERT_TRUE("test_binary_data_compare_span_equal", data == view);
	ASSERT_TRUE("test_binary_data_compare_span_equal", view == data);
	ASSERT_TRUE("test_binary_data_compare_span_equal", !(data != view));
	RETURN_TEST("test_binary_data_compare_span_equal", 0);
}

int test_binary_data_compare_span_order() {
	BinaryData data = Bytes({1, 2, 3});
	const std::byte raw[] = { std::byte{1}, std::byte{2}, std::byte{4} };
	const std::span<const std::byte> view{raw, 3};
	ASSERT_TRUE("test_binary_data_compare_span_order", (data <=> view) == std::strong_ordering::less);
	ASSERT_TRUE("test_binary_data_compare_span_order", data != view);
	RETURN_TEST("test_binary_data_compare_span_order", 0);
}

// -------------------
// Concepts
// -------------------

int test_binary_data_concepts_container_shape() {
	static_assert(StormByte::Type::Container<BinaryData>);
	static_assert(StormByte::Type::Sized<BinaryData>);
	static_assert(StormByte::Type::HasPushBack<BinaryData>);
	static_assert(StormByte::Type::HasSubscript<BinaryData, ByteSize>);
	static_assert(StormByte::Type::HasSubscript<BinaryData, std::size_t>);
	static_assert(!StormByte::Type::HasPushFront<BinaryData>);
	static_assert(!StormByte::Type::HasInsert<BinaryData>);
	static_assert(!StormByte::Type::HasKeyType<BinaryData>);
	static_assert(!StormByte::Type::HasMappedType<BinaryData>);
	static_assert(!StormByte::Type::String<BinaryData>);
	RETURN_TEST("test_binary_data_concepts_container_shape", 0);
}

int test_binary_data_concepts_range() {
	static_assert(StormByte::Type::ByteInputRange<BinaryData>);
	BinaryData data = Bytes({1, 2, 3});
	ASSERT_TRUE("test_binary_data_concepts_range", std::ranges::contiguous_range<BinaryData>);
	ASSERT_TRUE("test_binary_data_concepts_range", std::ranges::sized_range<BinaryData>);
	ASSERT_TRUE("test_binary_data_concepts_range", std::ranges::size(data) == 3);
	RETURN_TEST("test_binary_data_concepts_range", 0);
}

// -------------------
// Construction
// -------------------

int test_binary_data_construct_char_pointer() {
	BinaryData data("AB");
	ASSERT_TRUE("test_binary_data_construct_char_pointer", SameBytes(data, {'A', 'B'}));
	BinaryData empty(static_cast<const char*>(nullptr));
	ASSERT_TRUE("test_binary_data_construct_char_pointer", empty.empty());
	RETURN_TEST("test_binary_data_construct_char_pointer", 0);
}

int test_binary_data_construct_copy_move() {
	BinaryData original = Bytes({1, 2, 3});
	BinaryData copied(original);
	ASSERT_TRUE("test_binary_data_construct_copy_move", copied == original);
	BinaryData moved(std::move(original));
	ASSERT_TRUE("test_binary_data_construct_copy_move", SameBytes(moved, {1, 2, 3}));
	ASSERT_TRUE("test_binary_data_construct_copy_move", original.empty());
	RETURN_TEST("test_binary_data_construct_copy_move", 0);
}

int test_binary_data_construct_count_fill() {
	BinaryData data(ByteSize{3}, std::byte{7});
	ASSERT_TRUE("test_binary_data_construct_count_fill", SameBytes(data, {7, 7, 7}));
	RETURN_TEST("test_binary_data_construct_count_fill", 0);
}

int test_binary_data_construct_count_zero() {
	BinaryData data(ByteSize{3});
	ASSERT_TRUE("test_binary_data_construct_count_zero", SameBytes(data, {0, 0, 0}));
	RETURN_TEST("test_binary_data_construct_count_zero", 0);
}

int test_binary_data_construct_empty() {
	BinaryData data;
	ASSERT_TRUE("test_binary_data_construct_empty", data.empty());
	ASSERT_TRUE("test_binary_data_construct_empty", data.size() == ByteSize{0});
	ASSERT_TRUE("test_binary_data_construct_empty", data.begin() == data.end());
	ASSERT_TRUE("test_binary_data_construct_empty", data.data() == nullptr);
	RETURN_TEST("test_binary_data_construct_empty", 0);
}

int test_binary_data_construct_from_self_lvalue() {
	BinaryData original = Bytes({1, 2, 3});
	BinaryData copied(original);
	ASSERT_TRUE("test_binary_data_construct_from_self_lvalue", copied == original);
	ASSERT_TRUE("test_binary_data_construct_from_self_lvalue", SameBytes(original, {1, 2, 3}));
	RETURN_TEST("test_binary_data_construct_from_self_lvalue", 0);
}

int test_binary_data_construct_from_self_rvalue() {
	BinaryData original = Bytes({1, 2, 3});
	BinaryData moved(std::move(original));
	ASSERT_TRUE("test_binary_data_construct_from_self_rvalue", SameBytes(moved, {1, 2, 3}));
	ASSERT_TRUE("test_binary_data_construct_from_self_rvalue", original.empty());
	RETURN_TEST("test_binary_data_construct_from_self_rvalue", 0);
}

int test_binary_data_construct_initializer_list() {
	BinaryData data{std::byte{1}, std::byte{2}, std::byte{3}};
	ASSERT_TRUE("test_binary_data_construct_initializer_list", SameBytes(data, {1, 2, 3}));
	RETURN_TEST("test_binary_data_construct_initializer_list", 0);
}

int test_binary_data_construct_pointer_count() {
	const std::byte raw[] = {std::byte{9}, std::byte{8}};
	BinaryData data(raw, ByteSize{2});
	ASSERT_TRUE("test_binary_data_construct_pointer_count", SameBytes(data, {9, 8}));
	BinaryData empty(static_cast<const std::byte*>(nullptr), ByteSize{0});
	ASSERT_TRUE("test_binary_data_construct_pointer_count", empty.empty());
	RETURN_TEST("test_binary_data_construct_pointer_count", 0);
}

int test_binary_data_construct_range() {
	const std::array<unsigned char, 3> raw{1, 2, 3};
	BinaryData data(raw);
	ASSERT_TRUE("test_binary_data_construct_range", SameBytes(data, {1, 2, 3}));
	RETURN_TEST("test_binary_data_construct_range", 0);
}

int test_binary_data_construct_span() {
	const std::byte raw[] = {std::byte{4}, std::byte{5}};
	BinaryData data(std::span<const std::byte>(raw, 2));
	ASSERT_TRUE("test_binary_data_construct_span", SameBytes(data, {4, 5}));
	RETURN_TEST("test_binary_data_construct_span", 0);
}

int test_binary_data_construct_string_view() {
	BinaryData data(std::string_view("Hi"));
	ASSERT_TRUE("test_binary_data_construct_string_view", SameBytes(data, {'H', 'i'}));
	RETURN_TEST("test_binary_data_construct_string_view", 0);
}

int test_binary_data_construct_vector() {
	const std::vector<std::byte> raw{std::byte{7}, std::byte{8}, std::byte{9}};
	BinaryData data(raw);
	ASSERT_TRUE("test_binary_data_construct_vector", SameBytes(data, {7, 8, 9}));
	RETURN_TEST("test_binary_data_construct_vector", 0);
}

// -------------------
// Element access
// -------------------

int test_binary_data_access_at_ok() {
	BinaryData data = Bytes({1, 2, 3});
	ASSERT_TRUE("test_binary_data_access_at_ok", data.at(ByteSize{0}) == std::byte{1});
	data.at(ByteSize{2}) = std::byte{9};
	ASSERT_TRUE("test_binary_data_access_at_ok", SameBytes(data, {1, 2, 9}));
	RETURN_TEST("test_binary_data_access_at_ok", 0);
}

int test_binary_data_access_at_throws() {
	BinaryData data = Bytes({1});
	bool threw = false;
	try {
		(void)data.at(ByteSize{1});
	} catch (const OutOfBoundsError&) {
		threw = true;
	}
	ASSERT_TRUE("test_binary_data_access_at_throws", threw);
	const BinaryData& cref = data;
	threw = false;
	try {
		(void)cref.at(ByteSize{2});
	} catch (const OutOfBoundsError&) {
		threw = true;
	}
	ASSERT_TRUE("test_binary_data_access_at_throws", threw);
	RETURN_TEST("test_binary_data_access_at_throws", 0);
}

int test_binary_data_access_front_back() {
	BinaryData data = Bytes({1, 2, 3});
	ASSERT_TRUE("test_binary_data_access_front_back", data.front() == std::byte{1});
	ASSERT_TRUE("test_binary_data_access_front_back", data.back() == std::byte{3});
	data.front() = std::byte{9};
	data.back() = std::byte{8};
	ASSERT_TRUE("test_binary_data_access_front_back", SameBytes(data, {9, 2, 8}));
	RETURN_TEST("test_binary_data_access_front_back", 0);
}

int test_binary_data_access_span() {
	BinaryData data = Bytes({1, 2, 3});
	std::span<std::byte> view = data;
	ASSERT_EQUAL("test_binary_data_access_span", std::size_t{3}, view.size());
	view[1] = std::byte{9};
	ASSERT_TRUE("test_binary_data_access_span", data[ByteSize{1}] == std::byte{9});
	const BinaryData& cref = data;
	std::span<const std::byte> cview = cref;
	ASSERT_EQUAL("test_binary_data_access_span", std::size_t{3}, cview.size());
	RETURN_TEST("test_binary_data_access_span", 0);
}

int test_binary_data_access_subscript() {
	BinaryData data = Bytes({1, 2, 3});
	ASSERT_TRUE("test_binary_data_access_subscript", data[ByteSize{0}] == std::byte{1});
	data[ByteSize{1}] = std::byte{9};
	ASSERT_TRUE("test_binary_data_access_subscript", data.at(ByteSize{1}) == std::byte{9});
	RETURN_TEST("test_binary_data_access_subscript", 0);
}

// -------------------
// Hex dump
// -------------------

int test_binary_data_hexdump_empty() {
	BinaryData data;
	const std::string text = static_cast<std::string>(data.HexDump());
	ASSERT_TRUE("test_binary_data_hexdump_empty", text.empty());
	ASSERT_TRUE("test_binary_data_hexdump_empty",
		static_cast<std::string>(data.HexDump(8)).empty());
	ASSERT_TRUE("test_binary_data_hexdump_empty",
		static_cast<std::string>(data.HexDump(0)).empty());
	RETURN_TEST("test_binary_data_hexdump_empty", 0);
}

int test_binary_data_hexdump_default_matches_sixteen() {
	BinaryData data{std::byte{'H'}, std::byte{'i'}};
	const std::string a = static_cast<std::string>(data.HexDump());
	const std::string b = static_cast<std::string>(data.HexDump(16));
	ASSERT_TRUE("test_binary_data_hexdump_default_matches_sixteen", a == b);
	ASSERT_TRUE("test_binary_data_hexdump_default_matches_sixteen",
		a.find("00000000") == 0);
	ASSERT_TRUE("test_binary_data_hexdump_default_matches_sixteen",
		a.find("48 69") != std::string::npos);
	ASSERT_TRUE("test_binary_data_hexdump_default_matches_sixteen",
		a.find("Hi") != std::string::npos);
	RETURN_TEST("test_binary_data_hexdump_default_matches_sixteen", 0);
}

int test_binary_data_hexdump_columns_eight() {
	BinaryData data;
	for (unsigned i = 0; i < 20; ++i)
		data.push_back(static_cast<std::byte>(i));
	const std::string text = static_cast<std::string>(data.HexDump(8));
	ASSERT_TRUE("test_binary_data_hexdump_columns_eight",
		text.find("00000000") == 0);
	ASSERT_TRUE("test_binary_data_hexdump_columns_eight",
		text.find("00000008") != std::string::npos);
	ASSERT_TRUE("test_binary_data_hexdump_columns_eight",
		text.find("00000010") != std::string::npos);
	ASSERT_TRUE("test_binary_data_hexdump_columns_eight",
		text.find('\n') != std::string::npos);
	RETURN_TEST("test_binary_data_hexdump_columns_eight", 0);
}

int test_binary_data_hexdump_columns_zero_single_line() {
	BinaryData data{std::byte{0x00}, std::byte{0x0A}, std::byte{'A'}};
	const std::string text = static_cast<std::string>(data.HexDump(0));
	ASSERT_TRUE("test_binary_data_hexdump_columns_zero_single_line",
		text.find('\n') == std::string::npos);
	ASSERT_TRUE("test_binary_data_hexdump_columns_zero_single_line",
		text.find("00 0a 41") != std::string::npos);
	ASSERT_TRUE("test_binary_data_hexdump_columns_zero_single_line",
		text.find(".A") != std::string::npos);
	RETURN_TEST("test_binary_data_hexdump_columns_zero_single_line", 0);
}

// -------------------
// Iterators
// -------------------

int test_binary_data_iterators_contiguous() {
	BinaryData data = Bytes({1, 2, 3, 4});
	ASSERT_TRUE("test_binary_data_iterators_contiguous", data.data() + 2 == &data[ByteSize{2}]);
	ASSERT_TRUE("test_binary_data_iterators_contiguous", data.end() == data.begin() + 4);
	ASSERT_TRUE("test_binary_data_iterators_contiguous", std::contiguous_iterator<BinaryData::iterator>);
	ASSERT_TRUE("test_binary_data_iterators_contiguous", std::contiguous_iterator<BinaryData::const_iterator>);
	RETURN_TEST("test_binary_data_iterators_contiguous", 0);
}

int test_binary_data_iterators_empty() {
	BinaryData data;
	ASSERT_TRUE("test_binary_data_iterators_empty", data.begin() == data.end());
	ASSERT_TRUE("test_binary_data_iterators_empty", data.cbegin() == data.cend());
	ASSERT_TRUE("test_binary_data_iterators_empty", data.rbegin() == data.rend());
	RETURN_TEST("test_binary_data_iterators_empty", 0);
}

int test_binary_data_iterators_forward() {
	BinaryData data = Bytes({1, 2, 3});
	unsigned sum = 0;
	for (auto it = data.begin(); it != data.end(); ++it)
		sum += static_cast<unsigned>(*it);
	ASSERT_EQUAL("test_binary_data_iterators_forward", 6u, sum);
	ASSERT_TRUE("test_binary_data_iterators_forward", data.cbegin() == data.begin());
	ASSERT_TRUE("test_binary_data_iterators_forward", data.cend() == data.end());
	RETURN_TEST("test_binary_data_iterators_forward", 0);
}

int test_binary_data_iterators_reverse() {
	BinaryData data = Bytes({1, 2, 3});
	std::vector<unsigned char> seen;
	for (auto it = data.rbegin(); it != data.rend(); ++it)
		seen.push_back(static_cast<unsigned char>(*it));
	ASSERT_TRUE("test_binary_data_iterators_reverse", seen.size() == 3 && seen[0] == 3 && seen[1] == 2 && seen[2] == 1);
	ASSERT_TRUE("test_binary_data_iterators_reverse", *data.crbegin() == std::byte{3});
	RETURN_TEST("test_binary_data_iterators_reverse", 0);
}

// -------------------
// Modifiers
// -------------------

int test_binary_data_modifiers_append() {
	BinaryData data = Bytes({1});
	const std::byte extra[] = {std::byte{2}, std::byte{3}};
	data.append(std::span<const std::byte>(extra, 2));
	data.append(Bytes({4}));
	ASSERT_TRUE("test_binary_data_modifiers_append", SameBytes(data, {1, 2, 3, 4}));
	BinaryData moved = Bytes({5});
	data.append(std::move(moved));
	ASSERT_TRUE("test_binary_data_modifiers_append", SameBytes(data, {1, 2, 3, 4, 5}));
	ASSERT_TRUE("test_binary_data_modifiers_append", moved.empty());
	RETURN_TEST("test_binary_data_modifiers_append", 0);
}

int test_binary_data_modifiers_append_empty() {
	BinaryData data = Bytes({1});
	data.append(static_cast<const std::byte*>(nullptr), ByteSize{0});
	data.append(BinaryData{});
	ASSERT_TRUE("test_binary_data_modifiers_append_empty", SameBytes(data, {1}));
	RETURN_TEST("test_binary_data_modifiers_append_empty", 0);
}

int test_binary_data_modifiers_append_into_empty() {
	BinaryData data;
	BinaryData moved = Bytes({1, 2});
	data.append(std::move(moved));
	ASSERT_TRUE("test_binary_data_modifiers_append_into_empty", SameBytes(data, {1, 2}));
	ASSERT_TRUE("test_binary_data_modifiers_append_into_empty", moved.empty());
	RETURN_TEST("test_binary_data_modifiers_append_into_empty", 0);
}

int test_binary_data_modifiers_assign() {
	BinaryData data = Bytes({9, 9});
	data.assign(ByteSize{3}, std::byte{1});
	ASSERT_TRUE("test_binary_data_modifiers_assign", SameBytes(data, {1, 1, 1}));
	data.assign({std::byte{2}, std::byte{3}});
	ASSERT_TRUE("test_binary_data_modifiers_assign", SameBytes(data, {2, 3}));
	const std::byte raw[] = {std::byte{4}};
	data.assign(std::span<const std::byte>(raw, 1));
	ASSERT_TRUE("test_binary_data_modifiers_assign", SameBytes(data, {4}));
	const unsigned char more[] = {5, 6};
	data.assign(more, more + 2);
	ASSERT_TRUE("test_binary_data_modifiers_assign", SameBytes(data, {5, 6}));
	data.assign(ByteSize{0}, std::byte{0});
	ASSERT_TRUE("test_binary_data_modifiers_assign", data.empty());
	RETURN_TEST("test_binary_data_modifiers_assign", 0);
}

int test_binary_data_modifiers_erase() {
	BinaryData data = Bytes({1, 2, 3, 4});
	auto it = data.erase(data.begin() + 1);
	ASSERT_TRUE("test_binary_data_modifiers_erase", *it == std::byte{3});
	ASSERT_TRUE("test_binary_data_modifiers_erase", SameBytes(data, {1, 3, 4}));
	data.erase(data.begin(), data.begin() + 2);
	ASSERT_TRUE("test_binary_data_modifiers_erase", SameBytes(data, {4}));
	data.erase(data.begin(), data.end());
	ASSERT_TRUE("test_binary_data_modifiers_erase", data.empty());
	RETURN_TEST("test_binary_data_modifiers_erase", 0);
}

int test_binary_data_modifiers_insert() {
	BinaryData data = Bytes({1, 4});
	auto it = data.insert(data.begin() + 1, std::byte{2});
	ASSERT_TRUE("test_binary_data_modifiers_insert", *it == std::byte{2});
	data.insert(data.begin() + 2, ByteSize{1}, std::byte{3});
	ASSERT_TRUE("test_binary_data_modifiers_insert", SameBytes(data, {1, 2, 3, 4}));
	data.insert(data.end(), {std::byte{5}});
	ASSERT_TRUE("test_binary_data_modifiers_insert", SameBytes(data, {1, 2, 3, 4, 5}));
	const unsigned char extra[] = {6};
	data.insert(data.end(), extra, extra + 1);
	ASSERT_TRUE("test_binary_data_modifiers_insert", SameBytes(data, {1, 2, 3, 4, 5, 6}));
	const std::byte mid[] = {std::byte{7}};
	data.insert(data.begin(), std::span<const std::byte>(mid, 1));
	ASSERT_TRUE("test_binary_data_modifiers_insert", SameBytes(data, {7, 1, 2, 3, 4, 5, 6}));
	data.insert(data.end(), ByteSize{0}, std::byte{0});
	ASSERT_TRUE("test_binary_data_modifiers_insert", data.size() == ByteSize{7});
	RETURN_TEST("test_binary_data_modifiers_insert", 0);
}

int test_binary_data_modifiers_plus_equal() {
	BinaryData data = Bytes({1});
	const std::byte extra[] = { std::byte{2}, std::byte{3} };
	data += std::span<const std::byte>{extra, 2};
	ASSERT_TRUE("test_binary_data_modifiers_plus_equal", SameBytes(data, {1, 2, 3}));
	BinaryData tail = Bytes({4});
	data += tail;
	ASSERT_TRUE("test_binary_data_modifiers_plus_equal", SameBytes(data, {1, 2, 3, 4}));
	ASSERT_TRUE("test_binary_data_modifiers_plus_equal", SameBytes(tail, {4}));
	BinaryData moved = Bytes({5});
	data += std::move(moved);
	ASSERT_TRUE("test_binary_data_modifiers_plus_equal", SameBytes(data, {1, 2, 3, 4, 5}));
	ASSERT_TRUE("test_binary_data_modifiers_plus_equal", moved.empty());
	RETURN_TEST("test_binary_data_modifiers_plus_equal", 0);
}

int test_binary_data_modifiers_push_pop() {
	BinaryData data;
	data.push_back(std::byte{1});
	data.emplace_back(2);
	ASSERT_TRUE("test_binary_data_modifiers_push_pop", SameBytes(data, {1, 2}));
	data.pop_back();
	ASSERT_TRUE("test_binary_data_modifiers_push_pop", SameBytes(data, {1}));
	data.pop_back();
	ASSERT_TRUE("test_binary_data_modifiers_push_pop", data.empty());
	RETURN_TEST("test_binary_data_modifiers_push_pop", 0);
}

int test_binary_data_modifiers_swap() {
	BinaryData a = Bytes({1, 2});
	BinaryData b = Bytes({3});
	a.swap(b);
	ASSERT_TRUE("test_binary_data_modifiers_swap", SameBytes(a, {3}));
	ASSERT_TRUE("test_binary_data_modifiers_swap", SameBytes(b, {1, 2}));
	swap(a, b);
	ASSERT_TRUE("test_binary_data_modifiers_swap", SameBytes(a, {1, 2}));
	ASSERT_TRUE("test_binary_data_modifiers_swap", SameBytes(b, {3}));
	BinaryData empty;
	swap(a, empty);
	ASSERT_TRUE("test_binary_data_modifiers_swap", a.empty());
	ASSERT_TRUE("test_binary_data_modifiers_swap", SameBytes(empty, {1, 2}));
	RETURN_TEST("test_binary_data_modifiers_swap", 0);
}

// -------------------
// Serializable (container path)
// -------------------

int test_binary_data_serializable_empty() {
	BinaryData empty;
	StormByte::Serializable<BinaryData> wrapped(empty);
	const auto blob = wrapped.Serialize();
	auto loaded = StormByte::Serializable<BinaryData>::Deserialize(blob);
	ASSERT_TRUE("test_binary_data_serializable_empty", static_cast<bool>(loaded));
	ASSERT_TRUE("test_binary_data_serializable_empty", loaded.value().empty());
	RETURN_TEST("test_binary_data_serializable_empty", 0);
}

int test_binary_data_serializable_roundtrip() {
	BinaryData original = Bytes({0, 1, 2, 255, 128});
	StormByte::Serializable<BinaryData> wrapped(original);
	const auto blob = wrapped.Serialize();
	auto loaded = StormByte::Serializable<BinaryData>::Deserialize(blob);
	ASSERT_TRUE("test_binary_data_serializable_roundtrip", static_cast<bool>(loaded));
	ASSERT_TRUE("test_binary_data_serializable_roundtrip", loaded.value() == original);
	RETURN_TEST("test_binary_data_serializable_roundtrip", 0);
}

// -------------------
// Vector boundary
// -------------------

int test_binary_data_construct_from_vector_copy() {
	std::vector<std::byte> src{std::byte{1}, std::byte{2}, std::byte{3}};
	const BinaryData data(src);
	ASSERT_EQUAL("test_binary_data_construct_from_vector_copy", ByteSize{3}, data.size());
	ASSERT_TRUE("test_binary_data_construct_from_vector_copy", src.size() == 3);
	ASSERT_TRUE("test_binary_data_construct_from_vector_copy", data[ByteSize{0}] == std::byte{1});
	ASSERT_TRUE("test_binary_data_construct_from_vector_copy", data[ByteSize{2}] == std::byte{3});
	RETURN_TEST("test_binary_data_construct_from_vector_copy", 0);
}

int test_binary_data_construct_from_vector_rvalue_empties_source() {
	std::vector<std::byte> src{std::byte{9}, std::byte{8}};
	const BinaryData data(std::move(src));
	ASSERT_EQUAL("test_binary_data_construct_from_vector_rvalue_empties_source", ByteSize{2}, data.size());
	ASSERT_TRUE("test_binary_data_construct_from_vector_rvalue_empties_source", src.empty());
	ASSERT_TRUE("test_binary_data_construct_from_vector_rvalue_empties_source", data[ByteSize{0}] == std::byte{9});
	RETURN_TEST("test_binary_data_construct_from_vector_rvalue_empties_source", 0);
}

int test_binary_data_convert_to_vector_copy_leaves_source() {
	BinaryData data{std::byte{4}, std::byte{5}};
	const std::vector<std::byte> out = static_cast<std::vector<std::byte>>(data);
	ASSERT_EQUAL("test_binary_data_convert_to_vector_copy_leaves_source", ByteSize{2}, data.size());
	ASSERT_TRUE("test_binary_data_convert_to_vector_copy_leaves_source", out.size() == 2);
	ASSERT_TRUE("test_binary_data_convert_to_vector_copy_leaves_source", out[0] == std::byte{4});
	ASSERT_TRUE("test_binary_data_convert_to_vector_copy_leaves_source", data[ByteSize{1}] == std::byte{5});
	RETURN_TEST("test_binary_data_convert_to_vector_copy_leaves_source", 0);
}

int test_binary_data_convert_to_vector_rvalue_empties_source() {
	BinaryData data{std::byte{6}, std::byte{7}, std::byte{8}};
	const std::vector<std::byte> out = static_cast<std::vector<std::byte>>(std::move(data));
	ASSERT_TRUE("test_binary_data_convert_to_vector_rvalue_empties_source", data.empty());
	ASSERT_TRUE("test_binary_data_convert_to_vector_rvalue_empties_source", out.size() == 3);
	ASSERT_TRUE("test_binary_data_convert_to_vector_rvalue_empties_source", out[2] == std::byte{8});
	RETURN_TEST("test_binary_data_convert_to_vector_rvalue_empties_source", 0);
}

int test_binary_data_span_roundtrip_vector() {
	const std::vector<std::byte> src{std::byte{0}, std::byte{255}};
	BinaryData data{std::span<const std::byte>(src)};
	const std::span<const std::byte> view = data;
	ASSERT_TRUE("test_binary_data_span_roundtrip_vector", view.size() == 2);
	ASSERT_TRUE("test_binary_data_span_roundtrip_vector", view[1] == std::byte{255});
	RETURN_TEST("test_binary_data_span_roundtrip_vector", 0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithms — binary search
	// -------------------
	result += test_binary_data_algorithm_binary_search();
	result += test_binary_data_algorithm_equal_range();
	result += test_binary_data_algorithm_lower_bound();
	result += test_binary_data_algorithm_upper_bound();

	// -------------------
	// Algorithms — comparison
	// -------------------
	result += test_binary_data_algorithm_equal();
	result += test_binary_data_algorithm_lexicographical_compare();
	result += test_binary_data_algorithm_lexicographical_compare_three_way();
	result += test_binary_data_algorithm_mismatch();

	// -------------------
	// Algorithms — heap
	// -------------------
	result += test_binary_data_algorithm_heap();
	result += test_binary_data_algorithm_is_heap_until();

	// -------------------
	// Algorithms — min / max
	// -------------------
	result += test_binary_data_algorithm_max_element();
	result += test_binary_data_algorithm_min_element();
	result += test_binary_data_algorithm_minmax_element();

	// -------------------
	// Algorithms — modifying
	// -------------------
	result += test_binary_data_algorithm_copy();
	result += test_binary_data_algorithm_copy_backward();
	result += test_binary_data_algorithm_copy_if();
	result += test_binary_data_algorithm_copy_n();
	result += test_binary_data_algorithm_fill();
	result += test_binary_data_algorithm_fill_n();
	result += test_binary_data_algorithm_generate();
	result += test_binary_data_algorithm_generate_n();
	result += test_binary_data_algorithm_iter_swap();
	result += test_binary_data_algorithm_move();
	result += test_binary_data_algorithm_move_backward();
	result += test_binary_data_algorithm_remove();
	result += test_binary_data_algorithm_remove_copy();
	result += test_binary_data_algorithm_remove_if();
	result += test_binary_data_algorithm_replace();
	result += test_binary_data_algorithm_replace_copy();
	result += test_binary_data_algorithm_replace_if();
	result += test_binary_data_algorithm_reverse();
	result += test_binary_data_algorithm_reverse_copy();
	result += test_binary_data_algorithm_rotate();
	result += test_binary_data_algorithm_rotate_copy();
	result += test_binary_data_algorithm_shift();
	result += test_binary_data_algorithm_swap_ranges();
	result += test_binary_data_algorithm_transform();
	result += test_binary_data_algorithm_unique();
	result += test_binary_data_algorithm_unique_copy();

	// -------------------
	// Algorithms — non-modifying
	// -------------------
	result += test_binary_data_algorithm_adjacent_find();
	result += test_binary_data_algorithm_all_any_none();
	result += test_binary_data_algorithm_count();
	result += test_binary_data_algorithm_count_if();
	result += test_binary_data_algorithm_find();
	result += test_binary_data_algorithm_find_end();
	result += test_binary_data_algorithm_find_first_of();
	result += test_binary_data_algorithm_find_if();
	result += test_binary_data_algorithm_find_if_not();
	result += test_binary_data_algorithm_for_each();
	result += test_binary_data_algorithm_for_each_n();
	result += test_binary_data_algorithm_search();
	result += test_binary_data_algorithm_search_n();

	// -------------------
	// Algorithms — numeric
	// -------------------
	result += test_binary_data_algorithm_adjacent_difference();
	result += test_binary_data_algorithm_inner_product();
	result += test_binary_data_algorithm_iota();
	result += test_binary_data_algorithm_numeric_accumulate();
	result += test_binary_data_algorithm_partial_sum();

	// -------------------
	// Algorithms — partitioning
	// -------------------
	result += test_binary_data_algorithm_is_partitioned();
	result += test_binary_data_algorithm_partition();
	result += test_binary_data_algorithm_partition_copy();
	result += test_binary_data_algorithm_partition_point();
	result += test_binary_data_algorithm_stable_partition();

	// -------------------
	// Algorithms — permutations
	// -------------------
	result += test_binary_data_algorithm_is_permutation();
	result += test_binary_data_algorithm_next_prev_permutation();

	// -------------------
	// Algorithms — ranges
	// -------------------
	result += test_binary_data_algorithm_ranges_sort();

	// -------------------
	// Algorithms — set
	// -------------------
	result += test_binary_data_algorithm_includes();
	result += test_binary_data_algorithm_merge();
	result += test_binary_data_algorithm_inplace_merge();
	result += test_binary_data_algorithm_set_difference();
	result += test_binary_data_algorithm_set_intersection();
	result += test_binary_data_algorithm_set_symmetric_difference();
	result += test_binary_data_algorithm_set_union();

	// -------------------
	// Algorithms — sorting
	// -------------------
	result += test_binary_data_algorithm_is_sorted();
	result += test_binary_data_algorithm_is_sorted_until();
	result += test_binary_data_algorithm_nth_element();
	result += test_binary_data_algorithm_partial_sort();
	result += test_binary_data_algorithm_partial_sort_copy();
	result += test_binary_data_algorithm_sort();
	result += test_binary_data_algorithm_stable_sort();

	// -------------------
	// Capacity
	// -------------------
	result += test_binary_data_capacity_clear_keeps_capacity();
	result += test_binary_data_capacity_max_size();
	result += test_binary_data_capacity_reserve();
	result += test_binary_data_capacity_resize_grow_zero();
	result += test_binary_data_capacity_resize_grow_value();
	result += test_binary_data_capacity_resize_shrink();
	result += test_binary_data_capacity_resize_zero();
	result += test_binary_data_capacity_shrink_to_fit();

	// -------------------
	// Comparison
	// -------------------
	result += test_binary_data_compare_empty();
	result += test_binary_data_compare_equal();
	result += test_binary_data_compare_order();
	result += test_binary_data_compare_span_equal();
	result += test_binary_data_compare_span_order();

	// -------------------
	// Concepts
	// -------------------
	result += test_binary_data_concepts_container_shape();
	result += test_binary_data_concepts_range();

	// -------------------
	// Construction
	// -------------------
	result += test_binary_data_construct_char_pointer();
	result += test_binary_data_construct_copy_move();
	result += test_binary_data_construct_count_fill();
	result += test_binary_data_construct_count_zero();
	result += test_binary_data_construct_empty();
	result += test_binary_data_construct_from_self_lvalue();
	result += test_binary_data_construct_from_self_rvalue();
	result += test_binary_data_construct_initializer_list();
	result += test_binary_data_construct_pointer_count();
	result += test_binary_data_construct_range();
	result += test_binary_data_construct_span();
	result += test_binary_data_construct_string_view();
	result += test_binary_data_construct_vector();

	// -------------------
	// Element access
	// -------------------
	result += test_binary_data_access_at_ok();
	result += test_binary_data_access_at_throws();
	result += test_binary_data_access_front_back();
	result += test_binary_data_access_span();
	result += test_binary_data_access_subscript();

	// -------------------
	// Hex dump
	// -------------------
	result += test_binary_data_hexdump_columns_eight();
	result += test_binary_data_hexdump_columns_zero_single_line();
	result += test_binary_data_hexdump_default_matches_sixteen();
	result += test_binary_data_hexdump_empty();

	// -------------------
	// Iterators
	// -------------------
	result += test_binary_data_iterators_contiguous();
	result += test_binary_data_iterators_empty();
	result += test_binary_data_iterators_forward();
	result += test_binary_data_iterators_reverse();

	// -------------------
	// Modifiers
	// -------------------
	result += test_binary_data_modifiers_append();
	result += test_binary_data_modifiers_append_empty();
	result += test_binary_data_modifiers_append_into_empty();
	result += test_binary_data_modifiers_assign();
	result += test_binary_data_modifiers_erase();
	result += test_binary_data_modifiers_insert();
	result += test_binary_data_modifiers_plus_equal();
	result += test_binary_data_modifiers_push_pop();
	result += test_binary_data_modifiers_swap();

	// -------------------
	// Serializable (container path)
	// -------------------
	result += test_binary_data_serializable_empty();
	result += test_binary_data_serializable_roundtrip();

	// -------------------
	// Vector boundary
	// -------------------
	result += test_binary_data_construct_from_vector_copy();
	result += test_binary_data_construct_from_vector_rvalue_empties_source();
	result += test_binary_data_convert_to_vector_copy_leaves_source();
	result += test_binary_data_convert_to_vector_rvalue_empties_source();
	result += test_binary_data_span_roundtrip_vector();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
