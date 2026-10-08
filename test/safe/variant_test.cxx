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
#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/variant.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <utility>
#include <variant>
#include <vector>

using namespace StormByte;

using Mixed = Safe::Variant<Safe::Monostate, int, Safe::String>;

// -------------------
// Access
// -------------------

int test_access_get_visit_and_holds() {
	Mixed value(7);
	ASSERT_TRUE(holds_alternative<int>(value));
	ASSERT_FALSE(holds_alternative<Safe::String>(value));
	ASSERT_FALSE(holds_alternative<Safe::Monostate>(value));
	ASSERT_NO_THROW((void)get<int>(value));
	ASSERT_EQUAL(7, get<int>(value));
	ASSERT_EQUAL(7, get<1>(value));
	ASSERT_NOT_NULL(get_if<int>(&value));
	ASSERT_NULL(get_if<Safe::String>(&value));
	ASSERT_NULL(get_if<0>(static_cast<Mixed*>(nullptr)));
	ASSERT_THROWS(get<Safe::String>(value), Safe::BadVariantAccess);
	ASSERT_THROWS(get<2>(value), Safe::BadVariantAccess);
	int seen = 0;
	ASSERT_NO_THROW(visit([&seen](const auto& alternative) {
		using Alternative = std::remove_cvref_t<decltype(alternative)>;
		if constexpr (std::is_same_v<Alternative, int>)
			seen = alternative;
	}, value));
	ASSERT_EQUAL(7, seen);
	Mixed empty;
	ASSERT_EQUAL(0u, empty.index());
	ASSERT_FALSE(empty.valueless_by_exception());
	ASSERT_TRUE(holds_alternative<Safe::Monostate>(empty));
	ASSERT_NO_THROW(visit([](const auto&) {}, empty));
	ASSERT_NO_THROW((void)get<0>(empty));
	Mixed gone(std::move(empty));
	ASSERT_TRUE(empty.valueless_by_exception());
	ASSERT_EQUAL((Mixed::npos), empty.index());
	ASSERT_THROWS(visit([](const auto&) {}, empty), Safe::BadVariantAccess);
	ASSERT_THROWS(get<0>(empty), Safe::BadVariantAccess);
	ASSERT_TRUE(holds_alternative<Safe::Monostate>(gone));
	RETURN_TEST(0);
}

// -------------------
// Algorithm
// -------------------

int test_algorithm_sort_find_and_visit() {
	std::vector<Mixed> values;
	values.emplace_back(Safe::String("b"));
	values.emplace_back(1);
	values.emplace_back(Safe::String("a"));
	values.emplace_back(3);
	ASSERT_NO_THROW(std::sort(values.begin(), values.end()));
	ASSERT_TRUE(holds_alternative<int>(values[0]));
	ASSERT_EQUAL(1, get<int>(values[0]));
	ASSERT_EQUAL(3, get<int>(values[1]));
	ASSERT_TRUE(holds_alternative<Safe::String>(values[2]));
	ASSERT_EQUAL(Safe::String("a"), get<Safe::String>(values[2]));
	auto found = std::find(values.begin(), values.end(), Mixed(Safe::String("b")));
	ASSERT_TRUE(found != values.end());
	ASSERT_EQUAL(Safe::String("b"), get<Safe::String>(*found));
	int sum = 0;
	std::for_each(values.begin(), values.end(), [&sum](const Mixed& item) {
		if (!holds_alternative<int>(item))
			return;
		sum += get<int>(item);
	});
	ASSERT_EQUAL(4, sum);
	ASSERT_EQUAL(static_cast<std::size_t>(2), static_cast<std::size_t>(std::count_if(values.begin(), values.end(), [](const Mixed& item) {
		return holds_alternative<int>(item);
	})));
	RETURN_TEST(0);
}

// -------------------
// Assign
// -------------------

int test_assign_copy_move_and_emplace() {
	Mixed value(4);
	Mixed other(Safe::String("kept"));
	value = other;
	ASSERT_EQUAL(Safe::String("kept"), get<Safe::String>(value));
	ASSERT_EQUAL(Safe::String("kept"), get<Safe::String>(other));
	value = std::move(other);
	ASSERT_TRUE(other.valueless_by_exception());
	ASSERT_EQUAL((Mixed::npos), other.index());
	ASSERT_EQUAL(Safe::String("kept"), get<Safe::String>(value));
	value = 9;
	ASSERT_EQUAL(9, get<int>(value));
	value = Safe::String("next");
	ASSERT_EQUAL(Safe::String("next"), get<Safe::String>(value));
	int& emplaced = value.emplace<1>(12);
	ASSERT_EQUAL(12, emplaced);
	ASSERT_EQUAL(1u, value.index());
	Safe::String& text = value.emplace<Safe::String>("again");
	ASSERT_EQUAL(Safe::String("again"), text);
	ASSERT_EQUAL(2u, value.index());
	value.emplace<0>();
	ASSERT_TRUE(holds_alternative<Safe::Monostate>(value));
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_alternatives_and_standard() {
	Mixed empty;
	ASSERT_EQUAL(0u, empty.index());
	ASSERT_FALSE(empty.valueless_by_exception());
	ASSERT_TRUE(holds_alternative<Safe::Monostate>(empty));
	ASSERT_NO_THROW((void)get<0>(empty));
	ASSERT_TRUE((get<0>(empty) == Safe::Monostate{}));
	Mixed from_int(6);
	ASSERT_EQUAL(1u, from_int.index());
	ASSERT_EQUAL(6, get<int>(from_int));
	Mixed from_text(Safe::String("ab"));
	ASSERT_EQUAL(2u, from_text.index());
	ASSERT_EQUAL(Safe::String("ab"), get<Safe::String>(from_text));
	Mixed in_index(std::in_place_index<1>, 8);
	ASSERT_EQUAL(8, get<1>(in_index));
	Mixed in_type(std::in_place_type<Safe::String>, "cd");
	ASSERT_EQUAL(Safe::String("cd"), get<Safe::String>(in_type));
	std::variant<Safe::Monostate, int, Safe::String> standard(5);
	Mixed from_std(standard);
	ASSERT_EQUAL(1u, standard.index());
	ASSERT_EQUAL(5, get<int>(from_std));
	std::variant<Safe::Monostate, int, Safe::String> copied = static_cast<std::variant<Safe::Monostate, int, Safe::String>>(from_std);
	ASSERT_EQUAL(5, std::get<int>(copied));
	Mixed copy(from_text);
	ASSERT_EQUAL(Safe::String("ab"), get<Safe::String>(copy));
	Mixed moved(std::move(copy));
	ASSERT_TRUE(copy.valueless_by_exception());
	ASSERT_EQUAL(Safe::String("ab"), get<Safe::String>(moved));
	ASSERT_THROWS((static_cast<std::variant<Safe::Monostate, int, Safe::String>>(copy)), Safe::BadVariantAccess);
	RETURN_TEST(0);
}

// -------------------
// Hash
// -------------------

int test_hash_monostate_and_alternative() {
	Mixed left(7);
	Mixed same(7);
	Mixed other(8);
	std::size_t left_hash = 0;
	std::size_t same_hash = 0;
	std::size_t other_hash = 0;
	ASSERT_NO_THROW(left_hash = Safe::Hash<Mixed>{}(left));
	ASSERT_NO_THROW(same_hash = Safe::Hash<Mixed>{}(same));
	ASSERT_NO_THROW(other_hash = Safe::Hash<Mixed>{}(other));
	ASSERT_EQUAL(left_hash, same_hash);
	ASSERT_NOT_EQUAL(left_hash, other_hash);
	ASSERT_EQUAL(std::hash<Mixed>{}(left), std::hash<Mixed>{}(same));
	ASSERT_EQUAL(Safe::Hash<Safe::Monostate>{}(Safe::Monostate{}), std::hash<Safe::Monostate>{}(Safe::Monostate{}));
	Mixed empty;
	Mixed empty_copy;
	ASSERT_NO_THROW((void)Safe::Hash<Mixed>{}(empty));
	ASSERT_EQUAL(Safe::Hash<Mixed>{}(empty), Safe::Hash<Mixed>{}(empty_copy));
	RETURN_TEST(0);
}

// -------------------
// Order
// -------------------

int test_order_index_then_alternative() {
	Mixed empty;
	Mixed number(1);
	Mixed later(2);
	Mixed text(Safe::String("a"));
	ASSERT_TRUE(empty == empty);
	ASSERT_TRUE(number == Mixed(1));
	ASSERT_FALSE(number == later);
	ASSERT_NO_THROW((void)(empty < number));
	ASSERT_TRUE((empty < number));
	ASSERT_TRUE((number < text));
	ASSERT_TRUE((number < later));
	ASSERT_TRUE((text > number));
	Safe::Monostate left;
	Safe::Monostate right;
	ASSERT_TRUE(left == right);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::equal);
	RETURN_TEST(0);
}

// -------------------
// Swap
// -------------------

int test_swap_member_and_free() {
	Mixed left(3);
	Mixed right(Safe::String("z"));
	left.swap(right);
	ASSERT_EQUAL(Safe::String("z"), get<Safe::String>(left));
	ASSERT_EQUAL(3, get<int>(right));
	swap(left, right);
	ASSERT_EQUAL(3, get<int>(left));
	ASSERT_EQUAL(Safe::String("z"), get<Safe::String>(right));
	Mixed gone(std::move(left));
	right.swap(left);
	ASSERT_TRUE(right.valueless_by_exception());
	ASSERT_EQUAL(Safe::String("z"), get<Safe::String>(left));
	ASSERT_EQUAL(3, get<int>(gone));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Access
	// -------------------
	result += test_access_get_visit_and_holds();

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_sort_find_and_visit();

	// -------------------
	// Assign
	// -------------------
	result += test_assign_copy_move_and_emplace();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_alternatives_and_standard();

	// -------------------
	// Hash
	// -------------------
	result += test_hash_monostate_and_alternative();

	// -------------------
	// Order
	// -------------------
	result += test_order_index_then_alternative();

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
