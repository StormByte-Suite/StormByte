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

#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/ranges.hxx>

#include <cstddef>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

using namespace StormByte;

// -------------------
// Aliases
// -------------------

int test_aliases_value_and_iterator() {
	static_assert(std::is_same_v<Type::RangeValue<std::vector<int>>, int>);
	static_assert(std::is_same_v<Type::RangeValue<Safe::Vector<int>>, int>);
	static_assert(std::is_same_v<Type::RangeValue<Safe::Binary>, std::byte>);
	static_assert(std::is_same_v<Type::RangeValue<std::string_view>, char>);
	static_assert(std::is_same_v<Type::RangeReference<std::vector<int>&>, int&>);
	static_assert(std::is_same_v<Type::RangeDifference<std::vector<int>>, std::ptrdiff_t>);
	static_assert(std::is_same_v<Type::IteratorValue<std::vector<int>::iterator>, int>);
	static_assert(std::is_same_v<Type::IteratorValue<std::byte*>, std::byte>);
	ASSERT_TRUE((std::is_same_v<Type::RangeValue<Safe::Binary>, std::byte>));
	ASSERT_TRUE((std::is_same_v<Type::RangeValue<Safe::Vector<char>>, char>));
	RETURN_TEST(0);
}

// -------------------
// Bytes
// -------------------

int test_bytes_scalar_not_class() {
	static_assert(Type::ByteInputRange<std::vector<std::byte>>);
	static_assert(Type::ByteInputRange<std::vector<unsigned char>>);
	static_assert(Type::ByteInputRange<std::span<const std::byte>>);
	static_assert(Type::ByteInputRange<Safe::Binary>);
	static_assert(Type::ByteInputRange<const Safe::Binary&>);
	static_assert(Type::ByteInputRange<Safe::Vector<std::byte>>);
	static_assert(Type::ByteInputRange<std::string>);
	static_assert(!Type::ByteInputRange<std::vector<std::string>>);
	static_assert(!Type::ByteInputRange<Safe::Vector<Safe::String>>);
	static_assert(!Type::ByteInputRange<int>);
	static_assert(Type::ByteInputIterator<std::byte*>);
	static_assert(Type::ByteInputIterator<const unsigned char*>);
	static_assert(Type::ByteInputIterator<Safe::Binary::const_iterator>);
	static_assert(Type::ByteInputIterator<std::string::iterator>);
	static_assert(!Type::ByteInputIterator<std::vector<std::string>::iterator>);
	static_assert(!Type::ByteInputIterator<int>);
	ASSERT_TRUE(Type::ByteInputRange<Safe::Binary>);
	ASSERT_FALSE((Type::ByteInputRange<Safe::Vector<Safe::String>>));
	ASSERT_TRUE(Type::ByteInputIterator<std::byte*>);
	RETURN_TEST(0);
}

// -------------------
// Iterators
// -------------------

int test_iterators_categories() {
	static_assert(Type::InputIterator<std::vector<int>::iterator>);
	static_assert(Type::InputIterator<Safe::Vector<int>::const_iterator>);
	static_assert(Type::InputIterator<int*>);
	static_assert(!Type::InputIterator<int>);
	static_assert(Type::OutputIterator<std::vector<int>::iterator, int>);
	static_assert(Type::OutputIterator<int*, int>);
	static_assert(!Type::OutputIterator<std::vector<int>::const_iterator, int>);
	static_assert(Type::ForwardIterator<std::forward_list<int>::iterator>);
	static_assert(Type::ForwardIterator<Safe::Vector<int>::iterator>);
	static_assert(Type::BidirectionalIterator<std::list<int>::iterator>);
	static_assert(Type::BidirectionalIterator<std::string::iterator>);
	static_assert(Type::RandomAccessIterator<std::vector<int>::iterator>);
	static_assert(Type::RandomAccessIterator<Safe::Vector<int>::iterator>);
	static_assert(!Type::RandomAccessIterator<std::list<int>::iterator>);
	static_assert(Type::ContiguousIterator<int*>);
	static_assert(Type::ContiguousIterator<std::vector<int>::iterator>);
	static_assert(Type::ContiguousIterator<Safe::Vector<int>::iterator>);
	static_assert(Type::ContiguousIterator<Safe::Binary::iterator>);
	static_assert(!Type::ContiguousIterator<std::list<int>::iterator>);
	static_assert(Type::SentinelFor<std::vector<int>::iterator, std::vector<int>::iterator>);
	static_assert(Type::SizedSentinelFor<int*, int*>);
	static_assert(Type::SizedSentinelFor<Safe::Vector<int>::iterator, Safe::Vector<int>::iterator>);
	ASSERT_TRUE(Type::ContiguousIterator<Safe::Binary::iterator>);
	ASSERT_FALSE(Type::RandomAccessIterator<std::list<int>::iterator>);
	RETURN_TEST(0);
}

// -------------------
// Ranges
// -------------------

int test_ranges_categories_and_views() {
	static_assert(Type::Range<std::vector<int>>);
	static_assert(Type::Range<const std::vector<int>&>);
	static_assert(Type::Range<std::string>);
	static_assert(Type::Range<std::string_view>);
	static_assert(Type::Range<Safe::Vector<int>>);
	static_assert(Type::Range<const Safe::Vector<int>&>);
	static_assert(Type::Range<Safe::Binary>);
	static_assert(Type::Range<const Safe::Binary&>);
	static_assert(Type::Range<Safe::String>);
	static_assert(!Type::Range<int>);
	static_assert(!Type::Range<int*>);
	static_assert(Type::InputRange<std::vector<int>>);
	static_assert(Type::InputRange<Safe::Vector<int>>);
	static_assert(Type::InputRange<Safe::Binary>);
	static_assert(Type::ForwardRange<std::forward_list<int>>);
	static_assert(Type::ForwardRange<Safe::String>);
	static_assert(Type::BidirectionalRange<std::list<int>>);
	static_assert(Type::BidirectionalRange<std::string>);
	static_assert(Type::RandomAccessRange<std::vector<int>>);
	static_assert(Type::RandomAccessRange<Safe::Vector<int>>);
	static_assert(Type::RandomAccessRange<Safe::Binary>);
	static_assert(!Type::RandomAccessRange<std::list<int>>);
	static_assert(Type::ContiguousRange<std::vector<int>>);
	static_assert(Type::ContiguousRange<std::string>);
	static_assert(Type::ContiguousRange<Safe::Vector<int>>);
	static_assert(Type::ContiguousRange<Safe::Binary>);
	static_assert(Type::ContiguousRange<Safe::String>);
	static_assert(!Type::ContiguousRange<std::list<int>>);
	static_assert(Type::SizedRange<std::vector<int>>);
	static_assert(Type::SizedRange<Safe::Vector<int>>);
	static_assert(Type::SizedRange<Safe::Binary>);
	static_assert(!Type::SizedRange<std::forward_list<int>>);
	static_assert(Type::CommonRange<std::vector<int>>);
	static_assert(Type::CommonRange<Safe::Vector<int>>);
	static_assert(Type::View<std::string_view>);
	static_assert(Type::View<std::span<int>>);
	static_assert(!Type::View<std::vector<int>>);
	static_assert(!Type::View<Safe::Vector<int>>);
	static_assert(!Type::View<Safe::Binary>);
	static_assert(!Type::View<Safe::String>);
	static_assert(Type::BorrowedRange<std::string_view>);
	static_assert(Type::BorrowedRange<std::span<const std::byte>>);
	static_assert(!Type::BorrowedRange<std::vector<int>>);
	static_assert(!Type::BorrowedRange<Safe::Vector<int>>);
	ASSERT_TRUE(Type::ContiguousRange<Safe::Binary>);
	ASSERT_TRUE(Type::Range<Safe::String>);
	ASSERT_FALSE(Type::View<Safe::Vector<int>>);
	ASSERT_FALSE(Type::BorrowedRange<Safe::Binary>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Aliases
	// -------------------
	result += test_aliases_value_and_iterator();

	// -------------------
	// Bytes
	// -------------------
	result += test_bytes_scalar_not_class();

	// -------------------
	// Iterators
	// -------------------
	result += test_iterators_categories();

	// -------------------
	// Ranges
	// -------------------
	result += test_ranges_categories_and_views();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
