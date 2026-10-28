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
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <sstream>
#include <vector>

using namespace StormByte;

// -------------------
// Algorithm
// -------------------

int test_algorithm_read_and_rewrite_keeps_size() {
	Safe::Vector<int> values{3, 1, 2, 2, 0};
	const Size owned = values.size();
	std::sort(values.begin(), values.end());
	std::stable_sort(values.begin(), values.end());
	std::partial_sort(values.begin(), values.begin() + 3, values.end());
	std::nth_element(values.begin(), values.begin() + 2, values.end());
	std::ranges::sort(values);
	ASSERT_EQUAL(owned, values.size());
	ASSERT_TRUE(std::is_sorted(values.begin(), values.end()));
	ASSERT_TRUE(std::ranges::is_sorted(values));
	ASSERT_EQUAL(0, *std::min_element(values.begin(), values.end()));
	ASSERT_EQUAL(3, *std::max_element(values.begin(), values.end()));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count(values.begin(), values.end(), 2));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(1), std::count_if(values.begin(), values.end(), [](int n) { return n == 0; }));
	ASSERT_TRUE(std::all_of(values.begin(), values.end(), [](int n) { return n >= 0; }));
	ASSERT_TRUE(std::any_of(values.begin(), values.end(), [](int n) { return n == 3; }));
	ASSERT_TRUE(std::none_of(values.begin(), values.end(), [](int n) { return n < 0; }));
	ASSERT_TRUE(std::binary_search(values.begin(), values.end(), 1));
	ASSERT_TRUE(std::ranges::binary_search(values, 2));
	ASSERT_TRUE(std::lower_bound(values.begin(), values.end(), 2) != values.end());
	ASSERT_TRUE(std::upper_bound(values.begin(), values.end(), 2) != values.begin());
	ASSERT_TRUE(std::equal_range(values.begin(), values.end(), 2).first != values.end());
	ASSERT_TRUE(std::find(values.begin(), values.end(), 3) != values.end());
	ASSERT_TRUE(std::find_if(values.begin(), values.end(), [](int n) { return n == 1; }) != values.end());
	ASSERT_TRUE(std::ranges::find(values, 0) == values.begin());
	const int needle[] = {0, 1};
	ASSERT_TRUE(std::search(values.begin(), values.end(), needle, needle + 2) == values.begin());
	ASSERT_TRUE(std::search_n(values.begin(), values.end(), 2, 2) != values.end());
	std::for_each(values.begin(), values.end(), [](int& n) { n = n; });
	std::ranges::for_each(values, [](int& n) { n = n; });
	std::reverse(values.begin(), values.end());
	std::ranges::reverse(values);
	std::rotate(values.begin(), values.begin() + 1, values.end());
	std::replace(values.begin(), values.end(), 0, 9);
	std::replace_if(values.begin(), values.end(), [](int n) { return n == 9; }, 0);
	std::fill(values.begin(), values.begin() + 1, 4);
	std::generate(values.begin(), values.end(), [n = 0]() mutable { return n++; });
	std::transform(values.begin(), values.end(), values.begin(), [](int n) { return n; });
	ASSERT_EQUAL(owned, values.size());
	Safe::Vector<int> other(values);
	ASSERT_TRUE(std::equal(values.begin(), values.end(), other.begin()));
	ASSERT_TRUE(std::ranges::equal(values, other));
	ASSERT_TRUE(std::mismatch(values.begin(), values.end(), other.begin()).first == values.end());
	std::vector<int> copied(static_cast<std::size_t>(owned));
	std::copy(values.begin(), values.end(), copied.begin());
	std::copy_backward(values.begin(), values.end(), copied.end());
	std::reverse_copy(values.begin(), values.end(), copied.begin());
	std::ranges::copy(values, copied.begin());
	std::iter_swap(values.begin(), values.end() - 1);
	std::swap_ranges(values.begin(), values.begin() + 2, other.begin());
	std::partition(values.begin(), values.end(), [](int n) { return n < 3; });
	std::sort(values.begin(), values.end());
	ASSERT_TRUE(std::unique(values.begin(), values.end()) <= values.end());
	std::reverse(values.rbegin(), values.rend());
	ASSERT_EQUAL(owned, values.size());
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_copy_move_and_export() {
	Safe::Vector<int> empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(Size{0}, empty.size());
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.cbegin() == empty.cend());
	ASSERT_TRUE(empty.rbegin() == empty.rend());
	ASSERT_TRUE(empty.crbegin() == empty.crend());
	ASSERT_TRUE(empty.max_size() > Size{0});
	Safe::Vector<int> counted(Size{3});
	ASSERT_EQUAL(Size{3}, counted.size());
	ASSERT_EQUAL(0, counted[Size{0}]);
	Safe::Vector<int> filled(Size{2}, 7);
	ASSERT_EQUAL(7, filled.at(Size{1}));
	const int raw[] = {1, 2, 3};
	Safe::Vector<int> from_iter(raw, raw + 3);
	Safe::Vector<int> listed{4, 5};
	std::vector<int> caller{8, 9};
	Safe::Vector<int> from_std(caller);
	ASSERT_EQUAL(static_cast<std::size_t>(2), caller.size());
	Safe::Vector<int> from_moved(std::move(caller));
	ASSERT_TRUE(caller.empty());
	ASSERT_EQUAL(8, from_moved.front());
	Safe::Vector<int> copy(from_iter);
	copy = from_iter;
	Safe::Vector<int> moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	moved = std::move(from_iter);
	ASSERT_TRUE(from_iter.empty());
	moved = {1, 2};
	std::vector<int> assigned{3};
	moved = assigned;
	ASSERT_EQUAL(1, assigned.size());
	std::vector<int> stolen{6, 7};
	moved = std::move(stolen);
	ASSERT_TRUE(stolen.empty());
	const std::vector<int> exported(moved);
	ASSERT_EQUAL(static_cast<std::size_t>(2), exported.size());
	const std::span<const int> view = moved;
	ASSERT_EQUAL(static_cast<std::size_t>(2), view.size());
	ASSERT_NOT_NULL(moved.data());
	ASSERT_TRUE(moved.capacity() >= moved.size());
	RETURN_TEST(0);
}

// -------------------
// Mutate
// -------------------

int test_mutate_insert_erase_and_order() {
	Safe::Vector<int> values;
	values.push_back(1);
	values.push_back(2);
	values.emplace_back(3);
	values.append_range(std::vector<int>{4});
	ASSERT_EQUAL(Size{4}, values.size());
	ASSERT_EQUAL(4, values.back());
	values.pop_back();
	ASSERT_THROWS(Safe::Vector<int>().pop_back(), Safe::OutOfBoundsError);
	ASSERT_THROWS(Safe::Vector<int>().front(), Safe::OutOfBoundsError);
	ASSERT_THROWS(Safe::Vector<int>().back(), Safe::OutOfBoundsError);
	ASSERT_THROWS(values.at(values.size()), Safe::OutOfBoundsError);
	auto inserted = values.insert(values.begin(), 0);
	ASSERT_EQUAL(0, *inserted);
	values.insert(values.end(), 9);
	values.insert(values.begin(), Size{2}, 8);
	const int extra[] = {7};
	values.insert(values.end(), extra, extra + 1);
	values.insert(values.end(), {6});
	values.insert_range(values.begin(), std::vector<int>{5});
	values.emplace(values.begin(), 4);
	values.erase(values.begin());
	values.erase(values.begin(), values.begin() + 1);
	values.assign(Size{2}, 1);
	values.assign(extra, extra + 1);
	values.assign({3, 1, 2});
	values.assign_range(std::vector<int>{2, 1});
	values.resize(Size{4});
	ASSERT_EQUAL(0, values[Size{3}]);
	values.resize(Size{3}, 9);
	values.reserve(Size{32});
	ASSERT_TRUE(values.capacity() >= Size{32});
	values.reserve(Size{1});
	ASSERT_TRUE(values.capacity() >= values.size());
	values.shrink_to_fit();
	const Safe::Vector<int>& read = values;
	ASSERT_EQUAL(values.front(), read.front());
	ASSERT_EQUAL(values.back(), read.back());
	ASSERT_EQUAL(values[Size{0}], read[Size{0}]);
	ASSERT_EQUAL(values.at(Size{0}), read.at(Size{0}));
	Safe::Vector<int> other{9};
	values.swap(other);
	swap(values, other);
	values.clear();
	ASSERT_TRUE(values.empty());
	ASSERT_TRUE(values.capacity() >= Size{0});
	Safe::Vector<int> left{1, 2};
	Safe::Vector<int> right{1, 3};
	ASSERT_TRUE(left == left);
	ASSERT_TRUE(left != right);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE(left <= left);
	ASSERT_TRUE(right > left);
	ASSERT_TRUE(right >= left);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::less);
	RETURN_TEST(0);
}

int test_bulk_bytes_copy_assign_and_initialize() {
	const Size count{2 * 1024 * 1024};
	Safe::Vector<std::byte> bytes(count, std::byte{0xa5});
	Safe::Vector<std::byte> copy(bytes);
	ASSERT_TRUE(copy == bytes);
	copy[Size{0}] = std::byte{0x42};
	ASSERT_EQUAL(std::byte{0xa5}, bytes[Size{0}]);
	copy.assign(bytes.begin() + 17, bytes.end() - 13);
	ASSERT_EQUAL(count - 30, copy.size());
	ASSERT_TRUE(std::all_of(copy.begin(), copy.end(), [](std::byte value) { return value == std::byte{0xa5}; }));
	copy.assign(copy.begin() + 5, copy.end() - 7);
	ASSERT_EQUAL(count - 42, copy.size());
	copy.resize(count);
	ASSERT_TRUE(std::all_of(copy.end() - 42, copy.end(), [](std::byte value) { return value == std::byte{0}; }));
	copy.resize(count + 9, std::byte{0x7e});
	ASSERT_TRUE(std::all_of(copy.end() - 9, copy.end(), [](std::byte value) { return value == std::byte{0x7e}; }));
	copy.assign(Size{513}, std::byte{0x3c});
	ASSERT_EQUAL(Size{513}, copy.size());
	ASSERT_TRUE(std::all_of(copy.begin(), copy.end(), [](std::byte value) { return value == std::byte{0x3c}; }));
	copy.assign(bytes.begin(), bytes.begin());
	ASSERT_TRUE(copy.empty());
	const std::byte* empty = nullptr;
	copy.assign(empty, empty);
	ASSERT_TRUE(copy.empty());
	copy.resize(Size{0});
	copy.resize(Size{0}, std::byte{1});
	Safe::Vector<std::byte> empty_copy(copy);
	ASSERT_TRUE(empty_copy.empty());
	RETURN_TEST(0);
}

int test_bulk_paths_preserve_values_and_nontrivial_elements() {
	Safe::Vector<int> integers(Size{1024}, 0x12345678);
	Safe::Vector<int> copy(integers.begin(), integers.end());
	ASSERT_TRUE(copy == integers);
	copy.resize(Size{2048});
	ASSERT_TRUE(std::all_of(copy.begin() + 1024, copy.end(), [](int value) { return value == 0; }));
	Safe::Vector<bool> flags(std::vector<bool>{true, false, true});
	ASSERT_EQUAL(Size{3}, flags.size());
	ASSERT_TRUE(flags[Size{0}]);
	ASSERT_FALSE(flags[Size{1}]);
	Safe::Vector<Safe::String> text(Size{2}, Safe::String{"keep"});
	Safe::Vector<Safe::String> text_copy(text);
	text_copy.resize(Size{4});
	ASSERT_TRUE(text_copy[Size{2}].empty());
	text_copy.resize(Size{5}, Safe::String{"tail"});
	ASSERT_EQUAL(Safe::String{"tail"}, text_copy.back());
	ASSERT_EQUAL(Safe::String{"keep"}, text.front());
	std::istringstream input("4 5 6");
	Safe::Vector<int> streamed(std::istream_iterator<int>{input}, std::istream_iterator<int>{});
	ASSERT_EQUAL(Size{3}, streamed.size());
	ASSERT_EQUAL(6, streamed.back());
	volatile int observed[] = {7, 8};
	Safe::Vector<int> from_volatile(observed, observed + 2);
	ASSERT_EQUAL(7, from_volatile.front());
	ASSERT_EQUAL(8, from_volatile.back());
	copy.assign(Size{17}, copy.front());
	ASSERT_TRUE(std::all_of(copy.begin(), copy.end(), [](int value) { return value == 0x12345678; }));
	copy.resize(copy.capacity() + 1, copy.front());
	ASSERT_EQUAL(0x12345678, copy.back());
	text.assign(Size{3}, text.front());
	ASSERT_TRUE(std::all_of(text.begin(), text.end(), [](const Safe::String& value) { return value == Safe::String{"keep"}; }));
	text.resize(text.capacity() + 1, text.front());
	ASSERT_EQUAL(Safe::String{"keep"}, text.back());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_read_and_rewrite_keeps_size();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_copy_move_and_export();

	// -------------------
	// Mutate
	// -------------------
	result += test_mutate_insert_erase_and_order();
	result += test_bulk_bytes_copy_assign_and_initialize();
	result += test_bulk_paths_preserve_values_and_nontrivial_elements();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
