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
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

using namespace StormByte;

// -------------------
// Construct
// -------------------

int test_construct_copy_move_and_assign() {
	Safe::Optional<int> empty;
	Safe::Optional<int> none(std::nullopt);
	ASSERT_FALSE(empty.has_value());
	ASSERT_FALSE(static_cast<bool>(none));
	ASSERT_TRUE(empty == std::nullopt);
	ASSERT_TRUE(std::nullopt == none);
	Safe::Optional<int> from_value(7);
	ASSERT_TRUE(from_value.has_value());
	ASSERT_EQUAL(7, *from_value);
	ASSERT_TRUE(from_value == 7);
	ASSERT_TRUE(7 == from_value);
	Safe::Optional<int> in_place(std::in_place, 3);
	ASSERT_EQUAL(3, in_place.value());
	std::optional<int> standard(5);
	Safe::Optional<int> from_std(standard);
	ASSERT_TRUE(standard.has_value());
	ASSERT_TRUE(from_std == standard);
	ASSERT_TRUE(standard == from_std);
	Safe::Optional<int> copy(from_value);
	ASSERT_EQUAL(7, copy.value());
	ASSERT_TRUE(copy == from_value);
	Safe::Optional<int> moved(std::move(copy));
	ASSERT_FALSE(copy.has_value());
	ASSERT_EQUAL(7, *moved);
	moved = 4;
	ASSERT_EQUAL(4, moved.value());
	moved = std::nullopt;
	ASSERT_FALSE(moved.has_value());
	moved = standard;
	ASSERT_EQUAL(5, *moved);
	moved.reset();
	ASSERT_FALSE(moved.has_value());
	RETURN_TEST(0);
}

// -------------------
// Value
// -------------------

int test_self_assignment_and_moved_from_reuse() {
	const char* payload = "owned payload long enough to require Base heap storage";
	Safe::Optional<Safe::String> text(std::in_place, payload);
	auto& alias = text;
	ASSERT_TRUE(&(text = alias) == &text);
	ASSERT_TRUE(&(text = std::move(alias)) == &text);
	ASSERT_TRUE(text.value() == payload);
	Safe::Optional<Safe::String> moved(std::move(text));
	ASSERT_FALSE(text.has_value());
	text.emplace("reused source");
	ASSERT_TRUE(text.value() == "reused source");
	text = std::move(moved);
	ASSERT_FALSE(moved.has_value());
	moved.emplace("reused again");
	ASSERT_TRUE(moved.value() == "reused again");
	ASSERT_TRUE(text.value() == payload);
	Safe::Optional<Safe::String> copy;
	copy = text;
	copy->clear();
	ASSERT_TRUE(text.value() == payload);
	ASSERT_TRUE(copy->empty());
	RETURN_TEST(0);
}

int test_aliased_assignment_and_emplace() {
	const char* payload = "aliased payload long enough to require Base heap storage";
	Safe::Optional<Safe::String> text(std::in_place, payload);
	ASSERT_TRUE(&(text = text.value()) == &text);
	ASSERT_TRUE(text.value() == payload);
	ASSERT_TRUE(&text.emplace(text.value()) == &text.value());
	ASSERT_TRUE(text.value() == payload);
	text = std::move(text.value());
	ASSERT_TRUE(text.value() == payload);
	text.emplace(std::move(text.value()));
	ASSERT_TRUE(text.value() == payload);
	text.swap(text);
	ASSERT_TRUE(text.value() == payload);
	RETURN_TEST(0);
}

int test_failed_emplace_and_empty_value_categories() {
	struct FailingValue {
		operator int() const {
			throw Safe::BadOptionalAccess();
		}
	};
	Safe::Optional<int> value(71);
	ASSERT_THROWS(value.emplace(FailingValue{}), Safe::BadOptionalAccess);
	ASSERT_TRUE(value.has_value());
	ASSERT_EQUAL(71, value.value());
	value.reset();
	ASSERT_THROWS(value.emplace(FailingValue{}), Safe::BadOptionalAccess);
	ASSERT_FALSE(value.has_value());
	const auto& read = value;
	ASSERT_THROWS(value.value(), Safe::BadOptionalAccess);
	ASSERT_THROWS(read.value(), Safe::BadOptionalAccess);
	ASSERT_THROWS(std::move(value).value(), Safe::BadOptionalAccess);
	ASSERT_THROWS(std::move(read).value(), Safe::BadOptionalAccess);
	ASSERT_FALSE(value.has_value());
	value.emplace(72);
	ASSERT_EQUAL(72, value.value());
	RETURN_TEST(0);
}

int test_value_throw_fallback_and_monadic() {
	Safe::Optional<int> empty;
	ASSERT_THROWS(empty.value(), Safe::BadOptionalAccess);
	ASSERT_EQUAL(9, empty.value_or(9));
	empty.emplace(11);
	ASSERT_EQUAL(11, *empty);
	ASSERT_EQUAL(11, empty.value());
	Safe::Optional<Safe::String> text(Safe::String("ab"));
	ASSERT_EQUAL(std::size_t{2}, text->size());
	Safe::Optional<int> left(1);
	Safe::Optional<int> right;
	left.swap(right);
	ASSERT_FALSE(left.has_value());
	ASSERT_EQUAL(1, right.value());
	swap(left, right);
	ASSERT_EQUAL(1, left.value());
	Safe::Optional<int> doubled = left.transform([](int n) { return n * 2; });
	ASSERT_EQUAL(2, doubled.value());
	Safe::Optional<int> chained = left.and_then([](int n) { return Safe::Optional<int>(n + 1); });
	ASSERT_EQUAL(2, chained.value());
	Safe::Optional<int> missing;
	Safe::Optional<int> recovered = missing.or_else([] { return Safe::Optional<int>(8); });
	ASSERT_EQUAL(8, recovered.value());
	Safe::Optional<int> kept = left.or_else([] { return Safe::Optional<int>(0); });
	ASSERT_EQUAL(1, kept.value());
	Safe::Optional<int> engaged = empty.or_else([] { return Safe::Optional<int>(0); });
	ASSERT_EQUAL(11, engaged.value());
	Safe::Optional<int> cleared;
	Safe::Optional<int> empty_transform = cleared.transform([](int n) { return n; });
	ASSERT_FALSE(empty_transform.has_value());
	RETURN_TEST(0);
}

int test_value_algorithm_sort_and_find() {
	std::vector<Safe::Optional<int>> values;
	values.emplace_back(3);
	values.emplace_back();
	values.emplace_back(1);
	values.emplace_back(2);
	std::sort(values.begin(), values.end(), [](const Safe::Optional<int>& left, const Safe::Optional<int>& right) {
		if (!left.has_value())
			return false;
		if (!right.has_value())
			return true;
		return *left < *right;
	});
	ASSERT_EQUAL(1, values[0].value());
	ASSERT_EQUAL(2, values[1].value());
	ASSERT_EQUAL(3, values[2].value());
	ASSERT_FALSE(values[3].has_value());
	auto found = std::find(values.begin(), values.end(), Safe::Optional<int>(2));
	ASSERT_TRUE(found != values.end());
	ASSERT_EQUAL(2, found->value());
	std::vector<int> present;
	std::for_each(values.begin(), values.end(), [&present](const Safe::Optional<int>& item) {
		if (item.has_value())
			present.push_back(*item);
	});
	ASSERT_EQUAL(static_cast<std::size_t>(3), present.size());
	ASSERT_EQUAL(1, present.front());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Construct
	// -------------------
	result += test_construct_copy_move_and_assign();

	// -------------------
	// Value
	// -------------------
	result += test_self_assignment_and_moved_from_reuse();
	result += test_aliased_assignment_and_emplace();
	result += test_failed_emplace_and_empty_value_categories();
	result += test_value_throw_fallback_and_monadic();
	result += test_value_algorithm_sort_and_find();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
