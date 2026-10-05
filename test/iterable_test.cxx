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

#include <StormByte/exception.hxx>
#include <StormByte/safe/iterable.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <ranges>
#include <utility>
#include <vector>

using namespace StormByte;

using Text = Safe::String;
using Sequence = Safe::Iterable<std::vector<Text>>;
using Dictionary = Safe::Iterable<std::map<Text, Text>>;

static_assert(Type::SafeValue<Sequence>);
static_assert(Type::SafeValue<Dictionary>);
static_assert(Type::SafeValue<Safe::Pair<Text, Text>>);
static_assert(std::random_access_iterator<Sequence::iterator>);
static_assert(std::sortable<Sequence::iterator>);
static_assert(std::ranges::input_range<Dictionary>);
static_assert(std::bidirectional_iterator<Dictionary::iterator>);
static_assert(!Type::SafeValue<std::vector<Text>>);

// -------------------
// Vector algorithms
// -------------------

int test_safe_iterable_sequence_algorithms() {
	int result = 0;
	Sequence values(std::vector<Text>{Text("delta"), Text("alpha"), Text("charlie"), Text("bravo")});
	std::ranges::sort(values);
	ASSERT_TRUE("test_safe_iterable_sequence_algorithms", static_cast<Text>(values[0]) == "alpha");
	std::ranges::reverse(values);
	ASSERT_TRUE("test_safe_iterable_sequence_algorithms", static_cast<Text>(values.front()) == "delta");

	auto middle = std::ranges::find(values, Text("charlie"));
	ASSERT_TRUE("test_safe_iterable_sequence_algorithms", middle != values.end());
	values.erase(middle);
	ASSERT_EQUAL("test_safe_iterable_sequence_algorithms", 3u, values.size());

	const auto exported = static_cast<std::vector<Text>>(values);
	ASSERT_EQUAL("test_safe_iterable_sequence_algorithms", 3u, exported.size());
	ASSERT_TRUE("test_safe_iterable_sequence_algorithms", exported[0] == "delta");
	std::vector<Text> source{Text("moved")};
	Sequence imported(std::move(source));
	ASSERT_TRUE("test_safe_iterable_sequence_algorithms", source.empty());
	ASSERT_TRUE("test_safe_iterable_sequence_algorithms", static_cast<Text>(imported.front()) == "moved");
	RETURN_TEST("test_safe_iterable_sequence_algorithms", result);
}

// -------------------
// Map algorithms
// -------------------

int test_safe_iterable_map_algorithms() {
	int result = 0;
	Dictionary values(std::map<Text, Text>{{Text("alpha"), Text("one")}, {Text("beta"), Text("two")}});
	ASSERT_TRUE("test_safe_iterable_map_algorithms", values.contains(Text("alpha")));
	ASSERT_TRUE("test_safe_iterable_map_algorithms", values.at(Text("beta")) == "two");

	const auto found = std::ranges::find_if(values, [](const auto& entry) {
		return entry.first == "alpha";
	});
	ASSERT_TRUE("test_safe_iterable_map_algorithms", found != values.end());
	auto [key, mapped] = *found;
	ASSERT_TRUE("test_safe_iterable_map_algorithms", key == "alpha" && mapped == "one");
	mapped = Text("through proxy");
	ASSERT_TRUE("test_safe_iterable_map_algorithms", values.at(Text("alpha")) == "through proxy");

	std::ranges::for_each(values, [](auto entry) {
		entry.second = Text("updated");
	});
	ASSERT_TRUE("test_safe_iterable_map_algorithms", values.at(Text("alpha")) == "updated");
	ASSERT_EQUAL("test_safe_iterable_map_algorithms", 1u, values.erase(Text("beta")));

	const auto exported = static_cast<std::map<Text, Text>>(values);
	ASSERT_EQUAL("test_safe_iterable_map_algorithms", 1u, exported.size());
	ASSERT_TRUE("test_safe_iterable_map_algorithms", exported.at(Text("alpha")) == "updated");
	std::map<Text, Text> source{{Text("moved"), Text("entry")}};
	Dictionary imported(std::move(source));
	ASSERT_TRUE("test_safe_iterable_map_algorithms", source.empty());
	ASSERT_TRUE("test_safe_iterable_map_algorithms", imported.at(Text("moved")) == "entry");
	RETURN_TEST("test_safe_iterable_map_algorithms", result);
}

// -------------------
// Integral-key map operations
// -------------------

template<typename Key>
int test_safe_iterable_integral_map_operations() {
	int result = 0;
	Safe::Map<Key, int> values;
	const auto& view = std::as_const(values);
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.lower_bound(Key(7)) == values.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.upper_bound(Key(7)) == values.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.lower_bound(Key(7)) == view.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.upper_bound(Key(7)) == view.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.erase(values.cend(), values.cend()) == values.end());
	ASSERT_EQUAL("test_safe_iterable_integral_map_operations", 0u, values.count(Key(7)));
	ASSERT_THROWS("test_safe_iterable_integral_map_operations", values.at(Key(7)), Exception);

	auto inserted = values.try_emplace(Key(7), 70);
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", inserted.second && inserted.first->first == Key(7));
	auto last = values.end();
	--last;
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", last == values.begin());
	auto constLast = view.cend();
	--constLast;
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", constLast == view.cbegin());
	ASSERT_THROWS("test_safe_iterable_integral_map_operations", --last, Exception);
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", !values.try_emplace(Key(7), 999).second);
	ASSERT_EQUAL("test_safe_iterable_integral_map_operations", 70, view.at(Key(7)));
	values.emplace(Key(3), 30);
	const std::pair<Key, int> entry{Key(11), 110};
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.insert(entry).second);
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", !values.insert(std::pair{Key(11), 999}).second);
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", !values.insert_or_assign(Key(7), 71).second);
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.insert_or_assign(Key(15), 150).second);
	values[Key(19)] = 190;
	values.at(Key(7)) = 72;
	ASSERT_EQUAL("test_safe_iterable_integral_map_operations", 72, view.at(Key(7)));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.find(Key(7)) == view.find(Key(7)));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.find(Key(99)) == values.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.find(Key(99)) == view.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.contains(Key(7)) && !view.contains(Key(99)));
	ASSERT_EQUAL("test_safe_iterable_integral_map_operations", 1u, view.count(Key(7)));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.lower_bound(Key(4))->first == Key(7));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.upper_bound(Key(7))->first == Key(11));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.lower_bound(Key(4))->first == Key(7));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.upper_bound(Key(7))->first == Key(11));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", view.upper_bound(Key(19)) == view.end());
	auto [equalFirst, equalLast] = values.equal_range(Key(7));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", equalFirst->first == Key(7) && equalLast->first == Key(11));
	auto [constFirst, constEnd] = view.equal_range(Key(7));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", constFirst == equalFirst && constEnd == equalLast);
	auto [missingFirst, missingLast] = view.equal_range(Key(8));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", missingFirst == missingLast && missingFirst->first == Key(11));

	auto stable = values.find(Key(7));
	typename Safe::Map<Key, int>::const_iterator constStable = stable;
	auto stableEnd = values.end();
	values.emplace(Key(1), 10);
	values.erase(Key(3));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", stable == values.find(Key(7)) && constStable == stable);
	stable->second = 73;
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", constStable->second == 73);
	++stable;
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", stable->first == Key(11));
	--stable;
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", stable == constStable);
	--stableEnd;
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", stableEnd->first == Key(19));
	std::ranges::for_each(values, [](auto current) { current.second = static_cast<int>(current.first); });
	ASSERT_EQUAL("test_safe_iterable_integral_map_operations", 7, view.at(Key(7)));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.erase(constStable)->first == Key(11));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", !values.contains(Key(7)));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.erase(values.find(Key(19))) == values.end());
	auto rangeEnd = values.find(Key(15));
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.erase(values.cbegin(), rangeEnd) == rangeEnd);
	ASSERT_EQUAL("test_safe_iterable_integral_map_operations", 1u, values.size());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.erase(values.cbegin(), values.cend()) == values.end());
	ASSERT_TRUE("test_safe_iterable_integral_map_operations", values.begin() == values.end());
	RETURN_TEST("test_safe_iterable_integral_map_operations", result);
}

// -------------------
// Pair bindings
// -------------------

int test_safe_pair_structured_bindings() {
	int result = 0;
	Safe::Pair<Text, Text> pair(Text("key"), Text("value"));
	auto [key, value] = pair;
	ASSERT_TRUE("test_safe_pair_structured_bindings", key == "key" && value == "value");
	pair.second = Text("changed");
	ASSERT_TRUE("test_safe_pair_structured_bindings", pair.second == "changed");
	const std::pair<Text, Text> standardPair{Text("standard"), Text("source")};
	Safe::Pair<Text, Text> imported(standardPair);
	const auto exported = static_cast<std::pair<Text, Text>>(imported);
	ASSERT_TRUE("test_safe_pair_structured_bindings", exported.first == "standard" && exported.second == "source");
	RETURN_TEST("test_safe_pair_structured_bindings", result);
}

int main() {
	int result = 0;

	// -------------------
	// Vector algorithms
	// -------------------
	result += test_safe_iterable_sequence_algorithms();

	// -------------------
	// Map algorithms
	// -------------------
	result += test_safe_iterable_map_algorithms();

	// -------------------
	// Integral-key map operations
	// -------------------
	result += test_safe_iterable_integral_map_operations<int>();
	result += test_safe_iterable_integral_map_operations<unsigned int>();
	result += test_safe_iterable_integral_map_operations<std::ptrdiff_t>();

	// -------------------
	// Pair bindings
	// -------------------
	result += test_safe_pair_structured_bindings();

	return result;
}
