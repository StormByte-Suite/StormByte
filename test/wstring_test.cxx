/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-String.
 *
 * StormByte-String original source is dual-licensed:
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
 * Both licenses apply only to original StormByte-String source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte Base tree),
 * which remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-String is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-String. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <compare>
#include <iostream>
#include <iterator>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

using StormByte::Size;
using StormByte::Safe::WCString;
using StormByte::Safe::String;
using StormByte::Safe::WString;

// -------------------
// Algorithm
// -------------------

int test_algorithm_equal_mismatch() {
	int result = 0;
	const WString left(L"abc");
	const WString right(L"abc");
	const WString other(L"abd");
	ASSERT_TRUE("test_algorithm_equal_mismatch", std::equal(left.begin(), left.end(), right.begin(), right.end()));
	ASSERT_FALSE("test_algorithm_equal_mismatch", std::equal(left.begin(), left.end(), other.begin(), other.end()));
	const auto miss = std::mismatch(left.begin(), left.end(), other.begin());
	ASSERT_TRUE("test_algorithm_equal_mismatch", *miss.first == L'c');
	ASSERT_TRUE("test_algorithm_equal_mismatch", *miss.second == L'd');
	RETURN_TEST("test_algorithm_equal_mismatch", result);
}

int test_algorithm_find_count() {
	int result = 0;
	const WString text(L"abracadabra");
	ASSERT_TRUE("test_algorithm_find_count", std::find(text.begin(), text.end(), L'c') == text.begin() + 4);
	ASSERT_TRUE("test_algorithm_find_count", std::find(text.begin(), text.end(), L'z') == text.end());
	ASSERT_EQUAL("test_algorithm_find_count", 5, std::count(text.begin(), text.end(), L'a'));
	ASSERT_EQUAL("test_algorithm_find_count", 2, std::count(text.begin(), text.end(), L'b'));
	RETURN_TEST("test_algorithm_find_count", result);
}

int test_algorithm_search_copy() {
	int result = 0;
	const WString text(L"hello world");
	const WString needle(L"wor");
	const auto found = std::search(text.begin(), text.end(), needle.begin(), needle.end());
	ASSERT_TRUE("test_algorithm_search_copy", found == text.begin() + 6);
	std::wstring copy;
	std::copy(text.begin(), text.end(), std::back_inserter(copy));
	ASSERT_TRUE("test_algorithm_search_copy", copy == L"hello world");
	RETURN_TEST("test_algorithm_search_copy", result);
}

int test_ordered_and_hashed() {
	int result = 0;
	std::set<WString> ordered;
	ordered.insert(WString(L"b"));
	ordered.insert(WString(L"a"));
	ASSERT_TRUE("test_ordered_and_hashed", *ordered.begin() == L"a");
	std::unordered_set<WString> hashed;
	hashed.insert(WString(L"k"));
	ASSERT_TRUE("test_ordered_and_hashed", hashed.contains(WString(L"k")));
	RETURN_TEST("test_ordered_and_hashed", result);
}

int test_ranges_algorithms() {
	int result = 0;
	const WString text(L"mississippi");
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::find(text, L'p') != text.end());
	ASSERT_EQUAL("test_ranges_algorithms", 4, std::ranges::count(text, L'i'));
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::equal(text, std::wstring_view(L"mississippi")));
#if defined(__cpp_lib_ranges_starts_ends_with)
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::starts_with(text, std::wstring_view(L"miss")));
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::ends_with(text, std::wstring_view(L"ppi")));
#endif
	const WString prefix(L"miss");
	const WString suffix(L"ppi");
	ASSERT_TRUE("test_ranges_algorithms", std::search(text.begin(), text.end(), prefix.begin(), prefix.end()) == text.begin());
	ASSERT_TRUE("test_ranges_algorithms", std::equal(text.end() - 3, text.end(), suffix.begin(), suffix.end()));
	RETURN_TEST("test_ranges_algorithms", result);
}

int test_sort_vector() {
	int result = 0;
	std::vector<WString> values;
	values.emplace_back(L"c");
	values.emplace_back(L"a");
	values.emplace_back(L"b");
	std::ranges::sort(values);
	ASSERT_TRUE("test_sort_vector", values[0] == L"a");
	ASSERT_TRUE("test_sort_vector", values[1] == L"b");
	ASSERT_TRUE("test_sort_vector", values[2] == L"c");
	RETURN_TEST("test_sort_vector", result);
}

int test_view_feeds_algorithms() {
	int result = 0;
	const WString text(L"contract");
	const std::wstring_view view = text;
	ASSERT_TRUE("test_view_feeds_algorithms", std::ranges::find(view, L'a') != view.end());
	ASSERT_TRUE("test_view_feeds_algorithms", std::ranges::equal(view, std::wstring_view(L"contract")));
	RETURN_TEST("test_view_feeds_algorithms", result);
}

// -------------------
// Compare
// -------------------

int test_equal_and_order() {
	int result = 0;
	WString a(L"ab");
	WString b(L"ab");
	WString c(L"ac");
	WString missing;
	ASSERT_TRUE("test_equal_and_order", a == b);
	ASSERT_TRUE("test_equal_and_order", a != c);
	ASSERT_TRUE("test_equal_and_order", a == L"ab");
	ASSERT_TRUE("test_equal_and_order", L"ab" == a);
	ASSERT_TRUE("test_equal_and_order", (a <=> b) == std::strong_ordering::equal);
	ASSERT_TRUE("test_equal_and_order", (a <=> c) == std::strong_ordering::less);
	ASSERT_TRUE("test_equal_and_order", (missing <=> a) == std::strong_ordering::less);
	ASSERT_TRUE("test_equal_and_order", missing != L"");
	RETURN_TEST("test_equal_and_order", result);
}

int test_hash() {
	int result = 0;
	WString a(L"key");
	WString b(L"key");
	ASSERT_EQUAL("test_hash", std::hash<WString>{}(a), std::hash<WString>{}(b));
	RETURN_TEST("test_hash", result);
}

// -------------------
// Construct
// -------------------

int test_construct_from_empty() {
	int result = 0;
	WString text(L"");
	ASSERT_TRUE("test_construct_from_empty", static_cast<bool>(text));
	ASSERT_EQUAL("test_construct_from_empty", Size{0}, text.size());
	ASSERT_TRUE("test_construct_from_empty", text.data() != nullptr);
	RETURN_TEST("test_construct_from_empty", result);
}

int test_construct_from_ptr() {
	int result = 0;
	WString text(L"abc");
	ASSERT_EQUAL("test_construct_from_ptr", Size{3}, text.size());
	ASSERT_EQUAL("test_construct_from_ptr", L'a', text[0]);
	ASSERT_EQUAL("test_construct_from_ptr", L'c', text[2]);
	ASSERT_EQUAL("test_construct_from_ptr", L'\0', text[3]);
	RETURN_TEST("test_construct_from_ptr", result);
}

int test_construct_from_view() {
	int result = 0;
	WString text(std::wstring_view(L"hi"));
	ASSERT_EQUAL("test_construct_from_view", Size{2}, text.size());
	ASSERT_TRUE("test_construct_from_view", text == L"hi");
	RETURN_TEST("test_construct_from_view", result);
}

int test_construct_from_wcstring() {
	int result = 0;
	WString text(WCString(L"owned"));
	ASSERT_TRUE("test_construct_from_wcstring", text == L"owned");
	RETURN_TEST("test_construct_from_wcstring", result);
}

int test_default_is_null() {
	int result = 0;
	WString text;
	ASSERT_FALSE("test_default_is_null", static_cast<bool>(text));
	ASSERT_TRUE("test_default_is_null", text.empty());
	ASSERT_EQUAL("test_default_is_null", Size{0}, text.size());
	ASSERT_TRUE("test_default_is_null", text.data() == nullptr);
	RETURN_TEST("test_default_is_null", result);
}

// -------------------
// Conversions / streams
// -------------------

int test_free_stream_operator() {
	int result = 0;
	WString text(L"streamed");
	std::wostringstream out;
	out << text;
	ASSERT_TRUE("test_free_stream_operator", out.str() == L"streamed");
	RETURN_TEST("test_free_stream_operator", result);
}

int test_stream_null_writes_nothing() {
	int result = 0;
	WString text;
	std::wostringstream out;
	out << L"pre" << text << L"post";
	ASSERT_TRUE("test_stream_null_writes_nothing", out.str() == L"prepost");
	RETURN_TEST("test_stream_null_writes_nothing", result);
}

int test_view_and_wstring() {
	int result = 0;
	WString text(L"abc");
	const std::wstring_view view = text;
	ASSERT_TRUE("test_view_and_wstring", view == L"abc");
	ASSERT_TRUE("test_view_and_wstring", std::wstring(text) == L"abc");
	ASSERT_TRUE("test_view_and_wstring", std::wstring_view(text.Bytes()) == L"abc");
	RETURN_TEST("test_view_and_wstring", result);
}

// -------------------
// Copy / move
// -------------------

int test_copy_assign() {
	int result = 0;
	WString left(L"old");
	WString right(L"new");
	left = right;
	ASSERT_TRUE("test_copy_assign", left == L"new");
	ASSERT_TRUE("test_copy_assign", right == L"new");
	RETURN_TEST("test_copy_assign", result);
}

int test_copy_is_independent() {
	int result = 0;
	WString original(L"alpha");
	WString copy(original);
	ASSERT_TRUE("test_copy_is_independent", copy == original);
	ASSERT_TRUE("test_copy_is_independent", copy.data() != original.data());
	RETURN_TEST("test_copy_is_independent", result);
}

int test_move_assign() {
	int result = 0;
	WString left(L"old");
	WString right(L"fresh");
	left = std::move(right);
	ASSERT_TRUE("test_move_assign", left == L"fresh");
	ASSERT_FALSE("test_move_assign", static_cast<bool>(right));
	RETURN_TEST("test_move_assign", result);
}

int test_move_leaves_source_null() {
	int result = 0;
	WString original(L"payload");
	WString taken(std::move(original));
	ASSERT_TRUE("test_move_leaves_source_null", taken == L"payload");
	ASSERT_FALSE("test_move_leaves_source_null", static_cast<bool>(original));
	RETURN_TEST("test_move_leaves_source_null", result);
}

int test_swap_exchanges() {
	int result = 0;
	WString left(L"L");
	WString right(L"R");
	left.swap(right);
	ASSERT_TRUE("test_swap_exchanges", left == L"R");
	ASSERT_TRUE("test_swap_exchanges", right == L"L");
	swap(left, right);
	ASSERT_TRUE("test_swap_exchanges", left == L"L");
	ASSERT_TRUE("test_swap_exchanges", right == L"R");
	RETURN_TEST("test_swap_exchanges", result);
}

// -------------------
// Helpers
// -------------------

int test_case_ascii() {
	int result = 0;
	ASSERT_TRUE("test_case_ascii", WString::ToUpper(L"abC") == L"ABC");
	ASSERT_TRUE("test_case_ascii", WString::ToLower(L"AbC") == L"abc");
	ASSERT_TRUE("test_case_ascii", WString(L"Xy").ToUpper() == L"XY");
	ASSERT_TRUE("test_case_ascii", WString(L"Xy").ToLower() == L"xy");
	RETURN_TEST("test_case_ascii", result);
}

int test_case_latin1() {
	int result = 0;
	ASSERT_TRUE("test_case_latin1", WString::ToUpper(L"café") == L"CAFÉ");
	ASSERT_TRUE("test_case_latin1", WString::ToLower(L"CAFÉ") == L"café");
	RETURN_TEST("test_case_latin1", result);
}

int test_explode() {
	int result = 0;
	std::queue<WString> parts;
	WString::Explode(L"a,,b", L',', parts);
	ASSERT_EQUAL("test_explode", 3u, parts.size());
	ASSERT_TRUE("test_explode", parts.front() == L"a");
	parts.pop();
	ASSERT_TRUE("test_explode", parts.front() == L"");
	parts.pop();
	ASSERT_TRUE("test_explode", parts.front() == L"b");
	const auto from_obj = WString(L"x|y").Explode(L'|');
	ASSERT_EQUAL("test_explode", 2u, from_obj.size());
	RETURN_TEST("test_explode", result);
}

int test_is_integer() {
	int result = 0;
	ASSERT_TRUE("test_is_integer", WString::IsInteger(L"0"));
	ASSERT_TRUE("test_is_integer", WString::IsInteger(L"-12"));
	ASSERT_TRUE("test_is_integer", WString::IsInteger(L"+3"));
	ASSERT_FALSE("test_is_integer", WString::IsInteger(L""));
	ASSERT_FALSE("test_is_integer", WString::IsInteger(L"-"));
	ASSERT_FALSE("test_is_integer", WString::IsInteger(L"1a"));
	ASSERT_TRUE("test_is_integer", WString(L"42").IsInteger());
	RETURN_TEST("test_is_integer", result);
}

int test_newlines_and_space() {
	int result = 0;
	ASSERT_TRUE("test_newlines_and_space", WString::SanitizeNewlines(L"a\r\nb\n") == L"a\nb\n");
	ASSERT_TRUE("test_newlines_and_space", WString(L"a\r\n").SanitizeNewlines() == L"a\n");
	ASSERT_TRUE("test_newlines_and_space", WString::RemoveWhitespace(L" a\tb\n") == L"ab");
	ASSERT_TRUE("test_newlines_and_space", WString(L" x ").RemoveWhitespace() == L"x");
	RETURN_TEST("test_newlines_and_space", result);
}

int test_split() {
	int result = 0;
	std::vector<WString> tokens;
	WString::Split(L"  a  bb\tc ", tokens);
	ASSERT_EQUAL("test_split", 3u, tokens.size());
	ASSERT_TRUE("test_split", tokens[0] == L"a");
	ASSERT_TRUE("test_split", tokens[2] == L"c");
	const auto from_obj = WString(L"x y").Split();
	ASSERT_EQUAL("test_split", 2u, from_obj.size());
	RETURN_TEST("test_split", result);
}

// -------------------
// Range
// -------------------

int test_range_iterators() {
	int result = 0;
	WString text(L"xyz");
	ASSERT_TRUE("test_range_iterators", text.begin() == text.data());
	ASSERT_TRUE("test_range_iterators", text.end() == text.data() + 3);
	wchar_t acc[4] {};
	std::size_t n = 0;
	for (wchar_t c : text)
		acc[n++] = c;
	ASSERT_EQUAL("test_range_iterators", 3u, n);
	ASSERT_EQUAL("test_range_iterators", L'x', acc[0]);
	ASSERT_EQUAL("test_range_iterators", L'z', acc[2]);
	RETURN_TEST("test_range_iterators", result);
}

int test_range_null() {
	int result = 0;
	WString text;
	ASSERT_TRUE("test_range_null", text.begin() == nullptr);
	ASSERT_TRUE("test_range_null", text.end() == nullptr);
	RETURN_TEST("test_range_null", result);
}

// -------------------
// UTF-8
// -------------------

int test_utf8_ascii_and_empty() {
	int result = 0;
	WString ascii(L"Hi");
	ASSERT_TRUE("test_utf8_ascii_and_empty", String(ascii) == "Hi");
	ASSERT_TRUE("test_utf8_ascii_and_empty", WString(String(ascii)) == L"Hi");
	WString empty(L"");
	ASSERT_TRUE("test_utf8_ascii_and_empty", String(empty) == "");
	ASSERT_TRUE("test_utf8_ascii_and_empty", WString(String(empty)) == L"");
	RETURN_TEST("test_utf8_ascii_and_empty", result);
}

int test_utf8_roundtrip() {
	int result = 0;
	WString wide(L"café");
	String from_ctor(wide);
	ASSERT_TRUE("test_utf8_roundtrip",
		static_cast<std::string_view>(from_ctor) == std::string_view("café"));
	String from_op = static_cast<String>(wide);
	ASSERT_TRUE("test_utf8_roundtrip", from_op == from_ctor);
	WString back_ctor(from_ctor);
	ASSERT_TRUE("test_utf8_roundtrip", back_ctor == wide);
	WString back_op = static_cast<WString>(from_ctor);
	ASSERT_TRUE("test_utf8_roundtrip", back_op == wide);
	RETURN_TEST("test_utf8_roundtrip", result);
}

int main() {
	int result = 0;

	// -------------------
	// Algorithm
	// -------------------
	result += test_algorithm_equal_mismatch();
	result += test_algorithm_find_count();
	result += test_algorithm_search_copy();
	result += test_ordered_and_hashed();
	result += test_ranges_algorithms();
	result += test_sort_vector();
	result += test_view_feeds_algorithms();

	// -------------------
	// Compare
	// -------------------
	result += test_equal_and_order();
	result += test_hash();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_from_empty();
	result += test_construct_from_ptr();
	result += test_construct_from_view();
	result += test_construct_from_wcstring();
	result += test_default_is_null();

	// -------------------
	// Conversions / streams
	// -------------------
	result += test_free_stream_operator();
	result += test_stream_null_writes_nothing();
	result += test_view_and_wstring();

	// -------------------
	// Copy / move
	// -------------------
	result += test_copy_assign();
	result += test_copy_is_independent();
	result += test_move_assign();
	result += test_move_leaves_source_null();
	result += test_swap_exchanges();

	// -------------------
	// Helpers
	// -------------------
	result += test_case_ascii();
	result += test_case_latin1();
	result += test_explode();
	result += test_is_integer();
	result += test_newlines_and_space();
	result += test_split();

	// -------------------
	// Range
	// -------------------
	result += test_range_iterators();
	result += test_range_null();

	// -------------------
	// UTF-8
	// -------------------
	result += test_utf8_ascii_and_empty();
	result += test_utf8_roundtrip();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
