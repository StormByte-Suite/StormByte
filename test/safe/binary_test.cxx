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

#include <StormByte/byte_size.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

using namespace StormByte;

// -------------------
// Algorithm
// -------------------

int test_algorithm_read_and_rewrite_keeps_size() {
	Safe::Binary bytes{std::byte{3}, std::byte{1}, std::byte{2}, std::byte{2}, std::byte{0}};
	const ByteSize owned = bytes.size();
	std::sort(bytes.begin(), bytes.end());
	std::stable_sort(bytes.begin(), bytes.end());
	std::ranges::sort(bytes);
	ASSERT_EQUAL(owned, bytes.size());
	ASSERT_TRUE(std::is_sorted(bytes.begin(), bytes.end()));
	ASSERT_TRUE(std::ranges::is_sorted(bytes));
	ASSERT_EQUAL(std::byte{0}, *std::min_element(bytes.begin(), bytes.end()));
	ASSERT_EQUAL(std::byte{3}, *std::max_element(bytes.begin(), bytes.end()));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count(bytes.begin(), bytes.end(), std::byte{2}));
	ASSERT_TRUE(std::binary_search(bytes.begin(), bytes.end(), std::byte{1}));
	ASSERT_TRUE(std::ranges::binary_search(bytes, std::byte{2}));
	ASSERT_TRUE(std::find(bytes.begin(), bytes.end(), std::byte{3}) != bytes.end());
	ASSERT_TRUE(std::ranges::find(bytes, std::byte{0}) == bytes.begin());
	std::reverse(bytes.begin(), bytes.end());
	std::ranges::reverse(bytes);
	std::rotate(bytes.begin(), bytes.begin() + 1, bytes.end());
	std::replace(bytes.begin(), bytes.end(), std::byte{0}, std::byte{9});
	std::fill(bytes.begin(), bytes.end(), std::byte{4});
	std::generate(bytes.begin(), bytes.end(), [n = 0]() mutable { return std::byte{static_cast<unsigned char>(n++ % 3)}; });
	std::transform(bytes.begin(), bytes.end(), bytes.begin(), [](std::byte value) { return value; });
	ASSERT_EQUAL(owned, bytes.size());
	Safe::Binary other(bytes);
	ASSERT_TRUE(std::equal(bytes.begin(), bytes.end(), other.begin()));
	ASSERT_TRUE(std::ranges::equal(bytes, other));
	std::vector<std::byte> copied(static_cast<std::size_t>(owned));
	std::copy(bytes.begin(), bytes.end(), copied.begin());
	std::reverse_copy(bytes.begin(), bytes.end(), copied.begin());
	std::ranges::copy(bytes, copied.begin());
	std::iter_swap(bytes.begin(), bytes.end() - 1);
	std::partition(bytes.begin(), bytes.end(), [](std::byte value) { return value < std::byte{2}; });
	std::sort(bytes.begin(), bytes.end());
	ASSERT_TRUE(std::unique(bytes.begin(), bytes.end()) <= bytes.end());
	std::reverse(bytes.rbegin(), bytes.rend());
	ASSERT_EQUAL(owned, bytes.size());
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_copy_move_and_export() {
	Safe::Binary empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_EQUAL(ByteSize{0}, empty.size());
	ASSERT_NULL(empty.data());
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.rbegin() == empty.rend());
	ASSERT_TRUE(empty.crbegin() == empty.crend());
	ASSERT_TRUE(empty.max_size() > ByteSize{0});
	Safe::Binary filled(ByteSize{3}, std::byte{7});
	ASSERT_EQUAL(std::byte{7}, filled[ByteSize{2}]);
	Safe::Binary zeroes(ByteSize{2});
	ASSERT_EQUAL(std::byte{0}, zeroes.at(ByteSize{0}));
	const std::byte raw[] = {std::byte{1}, std::byte{2}};
	Safe::Binary from_span(std::span<const std::byte>(raw, 2));
	Safe::Binary from_pointer(raw, ByteSize{2});
	ASSERT_THROWS(Safe::Binary(static_cast<const std::byte*>(nullptr), ByteSize{1}), Safe::OutOfBoundsError);
	Safe::Binary listed{std::byte{4}, std::byte{5}};
	Safe::Binary from_text(std::string_view("ab"));
	ASSERT_EQUAL(ByteSize{2}, from_text.size());
	Safe::Binary from_cstr("xy");
	Safe::Binary from_null(static_cast<const char*>(nullptr));
	ASSERT_TRUE(from_null.empty());
	std::vector<std::byte> caller{std::byte{8}};
	Safe::Binary from_std(caller);
	ASSERT_EQUAL(static_cast<std::size_t>(1), caller.size());
	Safe::Binary from_moved(std::move(caller));
	ASSERT_TRUE(caller.empty());
	Safe::Binary copy(from_span);
	copy = from_span;
	Safe::Binary moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	moved = std::move(from_span);
	moved = {std::byte{1}};
	const std::vector<std::byte> exported(moved);
	ASSERT_EQUAL(static_cast<std::size_t>(1), exported.size());
	const std::span<const std::byte> view = moved;
	ASSERT_EQUAL(static_cast<std::size_t>(1), view.size());
	ASSERT_EQUAL(ByteSize{1}, ByteSize{moved.span().size()});
	ASSERT_TRUE(moved.capacity() >= moved.size());
	RETURN_TEST(0);
}

// -------------------
// Mutate
// -------------------

int test_mutate_append_insert_and_order() {
	Safe::Binary bytes;
	const std::byte raw[] = {std::byte{1}, std::byte{2}};
	bytes.append(std::span<const std::byte>(raw, 2));
	bytes.append(raw, ByteSize{1});
	Safe::Binary extra{std::byte{9}};
	bytes.append(extra);
	Safe::Binary taken{std::byte{8}};
	bytes.append(std::move(taken));
	ASSERT_TRUE(taken.empty());
	bytes += std::span<const std::byte>(raw, 1);
	bytes += Safe::Binary{std::byte{3}};
	Safe::Binary moved_add{std::byte{4}};
	bytes += std::move(moved_add);
	bytes.push_back(std::byte{5});
	bytes.emplace_back(6);
	ASSERT_EQUAL(std::byte{6}, bytes.back());
	ASSERT_EQUAL(std::byte{1}, bytes.front());
	bytes.pop_back();
	auto inserted = bytes.insert(bytes.begin(), std::byte{0});
	ASSERT_EQUAL(std::byte{0}, *inserted);
	bytes.erase(bytes.begin());
	bytes.erase(bytes.begin(), bytes.begin() + 1);
	bytes.assign(ByteSize{2}, std::byte{7});
	bytes.assign(std::span<const std::byte>(raw, 2));
	bytes.assign({std::byte{3}, std::byte{1}});
	bytes.assign(raw, raw + 2);
	bytes.resize(ByteSize{4});
	ASSERT_EQUAL(std::byte{0}, bytes[ByteSize{3}]);
	bytes.resize(ByteSize{3}, std::byte{9});
	bytes.reserve(ByteSize{32});
	ASSERT_TRUE(bytes.capacity() >= ByteSize{32});
	bytes.shrink_to_fit();
	const Safe::Binary& read = bytes;
	ASSERT_EQUAL(bytes.front(), read.front());
	ASSERT_EQUAL(bytes.back(), read.back());
	ASSERT_EQUAL(bytes[ByteSize{0}], read[ByteSize{0}]);
	ASSERT_EQUAL(bytes.at(ByteSize{0}), read.at(ByteSize{0}));
	ASSERT_THROWS(bytes.at(bytes.size()), Safe::OutOfBoundsError);
	ASSERT_THROWS(bytes.append(static_cast<const std::byte*>(nullptr), ByteSize{1}), Safe::OutOfBoundsError);
	Safe::Binary other{std::byte{1}};
	bytes.swap(other);
	swap(bytes, other);
	Safe::String dump = bytes.HexDump();
	ASSERT_FALSE(dump.empty());
	bytes.clear();
	ASSERT_TRUE(bytes.empty());
	ASSERT_TRUE(bytes.HexDump().empty());
	Safe::Binary left{std::byte{1}, std::byte{2}};
	Safe::Binary right{std::byte{1}, std::byte{3}};
	ASSERT_TRUE(left == left);
	ASSERT_TRUE(left != right);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::less);
	ASSERT_TRUE(left == std::span<const std::byte>(left.data(), static_cast<std::size_t>(left.size())));
	ASSERT_TRUE(std::span<const std::byte>(left.data(), static_cast<std::size_t>(left.size())) == left);
	ASSERT_TRUE(left != std::span<const std::byte>(right.data(), static_cast<std::size_t>(right.size())));
	ASSERT_TRUE((left <=> std::span<const std::byte>(right.data(), static_cast<std::size_t>(right.size()))) == std::strong_ordering::less);
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
	result += test_mutate_append_insert_and_order();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
