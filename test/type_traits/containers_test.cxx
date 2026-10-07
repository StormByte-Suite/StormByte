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

#include <StormByte/byte_size.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/containers.hxx>

#include <array>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace StormByte;

// -------------------
// Mutation
// -------------------

int test_mutation_insert_and_subscript() {
	static_assert(Type::HasInsert<std::map<int, int>>);
	static_assert(Type::HasInsert<std::multimap<int, int>>);
	static_assert(Type::HasInsert<std::set<int>>);
	static_assert(Type::HasInsert<std::multiset<int>>);
	static_assert(Type::HasInsert<std::unordered_map<int, int>>);
	static_assert(Type::HasInsert<std::unordered_set<int>>);
	static_assert(Type::HasInsert<std::set<std::unique_ptr<int>>>);
	static_assert(Type::HasInsert<Safe::Map<int, int>>);
	static_assert(!Type::HasInsert<std::vector<int>>);
	static_assert(!Type::HasInsert<Safe::Vector<int>>);
	static_assert(!Type::HasInsert<Safe::Binary>);
	static_assert(!Type::HasInsert<Safe::Queue<int>>);
	static_assert(!Type::HasInsert<std::list<int>>);
	static_assert(!Type::HasInsert<std::string>);
	static_assert(!Type::HasInsert<Safe::String>);
	static_assert(Type::HasSubscript<std::vector<int>, std::size_t>);
	static_assert(Type::HasSubscript<std::vector<int>, Size>);
	static_assert(Type::HasSubscript<std::deque<int>, std::size_t>);
	static_assert(Type::HasSubscript<std::array<int, 2>, std::size_t>);
	static_assert(Type::HasSubscript<std::map<int, int>, int>);
	static_assert(Type::HasSubscript<std::unordered_map<int, int>, int>);
	static_assert(Type::HasSubscript<Safe::Vector<int>, std::size_t>);
	static_assert(Type::HasSubscript<Safe::Vector<int>, Size>);
	static_assert(Type::HasSubscript<Safe::Binary, ByteSize>);
	static_assert(Type::HasSubscript<Safe::Map<int, int>, int>);
	static_assert(Type::HasSubscript<const std::vector<int>, std::size_t>);
	static_assert(!Type::HasSubscript<std::vector<int>&, std::size_t>);
	static_assert(!Type::HasSubscript<const Safe::Vector<int>&, std::size_t>);
	static_assert(!Type::HasSubscript<const Safe::Binary&, ByteSize>);
	static_assert(!Type::HasSubscript<std::set<int>, int>);
	static_assert(!Type::HasSubscript<std::list<int>, std::size_t>);
	static_assert(!Type::HasSubscript<Safe::Queue<int>, std::size_t>);
	static_assert(!Type::HasSubscript<std::string, std::size_t>);
	static_assert(!Type::HasSubscript<Safe::String, std::size_t>);
	ASSERT_TRUE((Type::HasInsert<Safe::Map<int, int>>));
	ASSERT_TRUE((Type::HasSubscript<Safe::Binary, ByteSize>));
	ASSERT_FALSE((Type::HasInsert<Safe::Vector<int>>));
	ASSERT_FALSE((Type::HasSubscript<Safe::String, std::size_t>));
	RETURN_TEST(0);
}

int test_mutation_push() {
	static_assert(Type::HasPushBack<std::vector<int>>);
	static_assert(Type::HasPushBack<std::deque<int>>);
	static_assert(Type::HasPushBack<std::list<int>>);
	static_assert(Type::HasPushBack<std::vector<std::unique_ptr<int>>>);
	static_assert(Type::HasPushBack<std::list<std::unique_ptr<int>>>);
	static_assert(Type::HasPushBack<Safe::Vector<int>>);
	static_assert(!Type::HasPushBack<std::forward_list<int>>);
	static_assert(!Type::HasPushBack<std::set<int>>);
	static_assert(!Type::HasPushBack<std::map<int, int>>);
	static_assert(!Type::HasPushBack<Safe::Map<int, int>>);
	static_assert(!Type::HasPushBack<Safe::Queue<int>>);
	static_assert(!Type::HasPushBack<std::array<int, 2>>);
	static_assert(!Type::HasPushBack<std::string>);
	static_assert(!Type::HasPushBack<Safe::String>);
	static_assert(Type::HasPushFront<std::deque<int>>);
	static_assert(Type::HasPushFront<std::list<int>>);
	static_assert(Type::HasPushFront<std::forward_list<int>>);
	static_assert(Type::HasPushFront<std::list<std::unique_ptr<int>>>);
	static_assert(!Type::HasPushFront<std::vector<int>>);
	static_assert(!Type::HasPushFront<Safe::Vector<int>>);
	static_assert(!Type::HasPushFront<Safe::Map<int, int>>);
	static_assert(!Type::HasPushFront<Safe::Queue<int>>);
	static_assert(!Type::HasPushFront<Safe::Binary>);
	static_assert(!Type::HasPushFront<std::string>);
	ASSERT_TRUE(Type::HasPushBack<Safe::Vector<int>>);
	ASSERT_FALSE(Type::HasPushBack<Safe::Queue<int>>);
	ASSERT_FALSE(Type::HasPushFront<Safe::Vector<int>>);
	RETURN_TEST(0);
}

// -------------------
// Shape
// -------------------

int test_shape_array_and_sized() {
	static_assert(Type::Array<std::array<int, 4>>);
	static_assert(Type::Array<std::array<int, 0>>);
	static_assert(Type::Array<const std::array<char, 1>>);
	static_assert(!Type::Array<int[4]>);
	static_assert(!Type::Array<std::span<int, 4>>);
	static_assert(!Type::Array<std::vector<int>>);
	static_assert(!Type::Array<Safe::Vector<int>>);
	static_assert(!Type::Array<Safe::Binary>);
	static_assert(!Type::Array<Safe::Map<int, int>>);
	static_assert(!Type::Array<Safe::String>);
	static_assert(Type::Sized<std::vector<int>>);
	static_assert(Type::Sized<std::deque<int>>);
	static_assert(Type::Sized<std::list<int>>);
	static_assert(Type::Sized<std::array<int, 0>>);
	static_assert(Type::Sized<std::set<int>>);
	static_assert(Type::Sized<std::map<int, int>>);
	static_assert(Type::Sized<std::unordered_set<int>>);
	static_assert(Type::Sized<std::span<int>>);
	static_assert(Type::Sized<Safe::Vector<int>>);
	static_assert(Type::Sized<Safe::Map<int, int>>);
	static_assert(Type::Sized<Safe::Binary>);
	static_assert(!Type::Sized<std::forward_list<int>>);
	static_assert(!Type::Sized<std::string>);
	static_assert(!Type::Sized<Safe::String>);
	static_assert(!Type::Sized<Safe::WString>);
	ASSERT_TRUE(Type::Sized<Safe::Vector<int>>);
	ASSERT_TRUE(Type::Sized<Safe::Binary>);
	ASSERT_FALSE(Type::Array<Safe::Vector<int>>);
	ASSERT_FALSE(Type::Sized<Safe::String>);
	RETURN_TEST(0);
}

int test_shape_container_excludes_string() {
	static_assert(Type::Container<std::vector<int>>);
	static_assert(Type::Container<const std::vector<int>>);
	static_assert(!Type::Container<const std::vector<int>&>);
	static_assert(Type::Container<std::deque<int>>);
	static_assert(Type::Container<std::list<int>>);
	static_assert(Type::Container<std::forward_list<int>>);
	static_assert(Type::Container<std::array<int, 2>>);
	static_assert(Type::Container<std::set<int>>);
	static_assert(Type::Container<std::map<int, int>>);
	static_assert(Type::Container<std::unordered_map<int, int>>);
	static_assert(Type::Container<std::span<int>>);
	static_assert(Type::Container<std::string_view>);
	static_assert(Type::Container<Safe::Vector<int>>);
	static_assert(Type::Container<const Safe::Vector<int>>);
	static_assert(!Type::Container<const Safe::Vector<int>&>);
	static_assert(Type::Container<Safe::Map<int, int>>);
	static_assert(!Type::Container<const Safe::Map<int, int>&>);
	static_assert(Type::Container<Safe::Binary>);
	static_assert(!Type::Container<const Safe::Binary&>);
	static_assert(Type::Container<Safe::Queue<int>>);
	static_assert(!Type::Container<const Safe::Queue<int>&>);
	static_assert(!Type::Container<std::string>);
	static_assert(!Type::Container<std::wstring>);
	static_assert(!Type::Container<std::u16string>);
	static_assert(!Type::Container<std::u32string>);
	static_assert(!Type::Container<Safe::String>);
	static_assert(!Type::Container<Safe::WString>);
	static_assert(!Type::Container<int>);
	static_assert(!Type::Container<int*>);
	static_assert(Type::HasKeyType<std::map<int, int>>);
	static_assert(Type::HasKeyType<std::set<int>>);
	static_assert(Type::HasKeyType<std::unordered_map<int, int>>);
	static_assert(Type::HasKeyType<std::unordered_set<int>>);
	static_assert(Type::HasKeyType<Safe::Map<int, int>>);
	static_assert(!Type::HasKeyType<std::vector<int>>);
	static_assert(!Type::HasKeyType<Safe::Vector<int>>);
	static_assert(!Type::HasKeyType<Safe::Binary>);
	static_assert(!Type::HasKeyType<Safe::Queue<int>>);
	static_assert(!Type::HasKeyType<Safe::String>);
	static_assert(Type::HasMappedType<std::map<int, int>>);
	static_assert(Type::HasMappedType<std::multimap<int, int>>);
	static_assert(Type::HasMappedType<std::unordered_map<int, int>>);
	static_assert(Type::HasMappedType<Safe::Map<int, int>>);
	static_assert(!Type::HasMappedType<std::set<int>>);
	static_assert(!Type::HasMappedType<Safe::Vector<int>>);
	static_assert(!Type::HasMappedType<Safe::Binary>);
	static_assert(!Type::HasMappedType<Safe::Queue<int>>);
	ASSERT_TRUE(Type::Container<Safe::Queue<int>>);
	ASSERT_TRUE((Type::HasKeyType<Safe::Map<int, int>>));
	ASSERT_TRUE((Type::HasMappedType<Safe::Map<int, int>>));
	ASSERT_FALSE(Type::Container<Safe::String>);
	ASSERT_FALSE((Type::HasKeyType<Safe::Vector<int>>));
	RETURN_TEST(0);
}

// -------------------
// Text
// -------------------

int test_text_string_strips_cvref() {
	static_assert(Type::String<std::string>);
	static_assert(Type::String<const std::string&>);
	static_assert(Type::String<std::string&&>);
	static_assert(Type::String<std::wstring>);
	static_assert(Type::String<const std::wstring&>);
	static_assert(Type::String<std::u16string>);
	static_assert(Type::String<std::u32string>);
	static_assert(Type::String<Safe::String>);
	static_assert(Type::String<const Safe::String&>);
	static_assert(Type::String<Safe::String&&>);
	static_assert(Type::String<Safe::WString>);
	static_assert(Type::String<const Safe::WString&>);
	static_assert(Type::String<const Safe::WString&&>);
	static_assert(!Type::String<std::string_view>);
	static_assert(!Type::String<std::wstring_view>);
	static_assert(!Type::String<const char*>);
	static_assert(!Type::String<std::vector<char>>);
	static_assert(!Type::String<Safe::Vector<char>>);
	static_assert(!Type::String<Safe::Binary>);
	static_assert(!Type::String<int>);
	ASSERT_TRUE(Type::String<const Safe::WString&>);
	ASSERT_FALSE(Type::String<Safe::Binary>);
	ASSERT_FALSE(Type::String<std::string_view>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Mutation
	// -------------------
	result += test_mutation_insert_and_subscript();
	result += test_mutation_push();

	// -------------------
	// Shape
	// -------------------
	result += test_shape_array_and_sized();
	result += test_shape_container_excludes_string();

	// -------------------
	// Text
	// -------------------
	result += test_text_string_strips_cvref();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
