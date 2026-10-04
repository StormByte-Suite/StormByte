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

#include <StormByte/safe/iterable.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <map>
#include <ranges>
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
	// Pair bindings
	// -------------------
	result += test_safe_pair_structured_bindings();

	return result;
}
