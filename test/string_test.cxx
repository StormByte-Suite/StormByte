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
using StormByte::Safe::String;
using StormByte::Safe::WString;

static int TestEmbeddedNul() {
	int result = 0;
	const std::string_view source("a\0b", 3);
	String text(source);
	ASSERT_EQUAL("String preserves embedded NUL size", Size{3}, text.size());
	ASSERT_EQUAL("String preserves embedded NUL view", source, static_cast<std::string_view>(text));
	return result;
}

// -------------------
// Algorithm
// -------------------

int test_algorithm_equal_mismatch() {
	int result = 0;
	const String left("abc");
	const String right("abc");
	const String other("abd");
	ASSERT_TRUE("test_algorithm_equal_mismatch", std::equal(left.begin(), left.end(), right.begin(), right.end()));
	ASSERT_FALSE("test_algorithm_equal_mismatch", std::equal(left.begin(), left.end(), other.begin(), other.end()));
	const auto miss = std::mismatch(left.begin(), left.end(), other.begin());
	ASSERT_TRUE("test_algorithm_equal_mismatch", *miss.first == 'c');
	ASSERT_TRUE("test_algorithm_equal_mismatch", *miss.second == 'd');
	RETURN_TEST("test_algorithm_equal_mismatch", result);
}

int test_algorithm_find_count() {
	int result = 0;
	const String text("abracadabra");
	ASSERT_TRUE("test_algorithm_find_count", std::find(text.begin(), text.end(), 'c') == text.begin() + 4);
	ASSERT_TRUE("test_algorithm_find_count", std::find(text.begin(), text.end(), 'z') == text.end());
	ASSERT_EQUAL("test_algorithm_find_count", 5, std::count(text.begin(), text.end(), 'a'));
	ASSERT_EQUAL("test_algorithm_find_count", 2, std::count(text.begin(), text.end(), 'b'));
	RETURN_TEST("test_algorithm_find_count", result);
}

int test_algorithm_search_copy() {
	int result = 0;
	const String text("hello world");
	const String needle("wor");
	const auto found = std::search(text.begin(), text.end(), needle.begin(), needle.end());
	ASSERT_TRUE("test_algorithm_search_copy", found == text.begin() + 6);
	std::string copy;
	std::copy(text.begin(), text.end(), std::back_inserter(copy));
	ASSERT_EQUAL("test_algorithm_search_copy", std::string("hello world"), copy);
	RETURN_TEST("test_algorithm_search_copy", result);
}

int test_ordered_and_hashed() {
	int result = 0;
	std::set<String> ordered;
	ordered.insert(String("b"));
	ordered.insert(String("a"));
	ASSERT_TRUE("test_ordered_and_hashed", *ordered.begin() == "a");
	std::unordered_set<String> hashed;
	hashed.insert(String("k"));
	ASSERT_TRUE("test_ordered_and_hashed", hashed.contains(String("k")));
	RETURN_TEST("test_ordered_and_hashed", result);
}

int test_ranges_algorithms() {
	int result = 0;
	const String text("mississippi");
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::find(text, 'p') != text.end());
	ASSERT_EQUAL("test_ranges_algorithms", 4, std::ranges::count(text, 'i'));
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::equal(text, std::string_view("mississippi")));
#if defined(__cpp_lib_ranges_starts_ends_with)
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::starts_with(text, std::string_view("miss")));
	ASSERT_TRUE("test_ranges_algorithms", std::ranges::ends_with(text, std::string_view("ppi")));
#endif
	const String prefix("miss");
	const String suffix("ppi");
	ASSERT_TRUE("test_ranges_algorithms", std::search(text.begin(), text.end(), prefix.begin(), prefix.end()) == text.begin());
	ASSERT_TRUE("test_ranges_algorithms", std::equal(text.end() - 3, text.end(), suffix.begin(), suffix.end()));
	RETURN_TEST("test_ranges_algorithms", result);
}

int test_sort_vector() {
	int result = 0;
	std::vector<String> values;
	values.emplace_back("c");
	values.emplace_back("a");
	values.emplace_back("b");
	std::ranges::sort(values);
	ASSERT_TRUE("test_sort_vector", values[0] == "a");
	ASSERT_TRUE("test_sort_vector", values[1] == "b");
	ASSERT_TRUE("test_sort_vector", values[2] == "c");
	RETURN_TEST("test_sort_vector", result);
}

int test_mutable_text_algorithms() {
	int result = 0;
	String text("dcba");
	std::ranges::sort(text);
	ASSERT_TRUE("test_mutable_text_algorithms", text == "abcd");
	std::ranges::reverse(text);
	ASSERT_TRUE("test_mutable_text_algorithms", text == "dcba");
	text.data()[0] = 'a';
	ASSERT_TRUE("test_mutable_text_algorithms", text.front() == 'a');
	const String::size_type pos{1};
	ASSERT_TRUE("test_mutable_text_algorithms", text.find('c', pos) == Size{1});
	ASSERT_TRUE("test_mutable_text_algorithms", text.find('z') == String::npos);
	RETURN_TEST("test_mutable_text_algorithms", result);
}

int test_string_size_modifiers() {
	String text("ab");
	text.reserve(Size{128});
	const Size reservedCapacity = text.capacity();
	ASSERT_TRUE("test_string_size_modifiers", reservedCapacity >= Size{128});
	text.reserve(Size{1});
	ASSERT_TRUE("test_string_size_modifiers", text.capacity() == reservedCapacity);
	std::string imported("assigned from STL");
	text = imported;
	ASSERT_TRUE("test_string_size_modifiers", text == "assigned from STL");
	text.assign("ab");
	ASSERT_TRUE("test_string_size_modifiers", text.capacity() == Size{String::SSO_CAPACITY});
	text.append("cd").append(2, '!');
	text += "ef";
	text.insert(Size{1}, "XY");
	ASSERT_TRUE("test_string_size_modifiers", text == "aXYbcd!!ef");
	text.erase(Size{1}, Size{2});
	text.replace(Size{1}, Size{1}, "-");
	ASSERT_TRUE("test_string_size_modifiers", text == "a-cd!!ef");
	text.resize(Size{3}, '?');
	ASSERT_TRUE("test_string_size_modifiers", text == "a-c");
	text.push_back('!');
	text.pop_back();
	ASSERT_TRUE("test_string_size_modifiers", text == "a-c");
	text.clear();
	ASSERT_TRUE("test_string_size_modifiers", text.empty() && !static_cast<bool>(text));
	ASSERT_TRUE("test_string_size_modifiers", text.data() != nullptr);
	text.assign(3, 'x');
	ASSERT_TRUE("test_string_size_modifiers", text == "xxx");
	RETURN_TEST("test_string_size_modifiers", 0);
}

int test_view_feeds_algorithms() {
	int result = 0;
	const String text("contract");
	const std::string_view view = text;
	ASSERT_TRUE("test_view_feeds_algorithms", std::ranges::find(view, 'a') != view.end());
	ASSERT_TRUE("test_view_feeds_algorithms", std::ranges::equal(view, std::string_view("contract")));
	RETURN_TEST("test_view_feeds_algorithms", result);
}

// -------------------
// Compare
// -------------------

int test_equal_and_order() {
	int result = 0;
	String a("ab");
	String b("ab");
	String c("ac");
	String missing;
	ASSERT_TRUE("test_equal_and_order", a == b);
	ASSERT_TRUE("test_equal_and_order", a != c);
	ASSERT_TRUE("test_equal_and_order", a == "ab");
	ASSERT_TRUE("test_equal_and_order", "ab" == a);
	ASSERT_TRUE("test_equal_and_order", (a <=> b) == std::strong_ordering::equal);
	ASSERT_TRUE("test_equal_and_order", (a <=> c) == std::strong_ordering::less);
	ASSERT_TRUE("test_equal_and_order", (missing <=> a) == std::strong_ordering::less);
	ASSERT_TRUE("test_equal_and_order", missing == "");
	RETURN_TEST("test_equal_and_order", result);
}

int test_hash() {
	int result = 0;
	String a("key");
	String b("key");
	ASSERT_EQUAL("test_hash", std::hash<String>{}(a), std::hash<String>{}(b));
	RETURN_TEST("test_hash", result);
}

// -------------------
// Construct
// -------------------

int test_construct_from_owned_string() {
	int result = 0;
	String source("owned");
	String text(source);
	ASSERT_TRUE("test_construct_from_owned_string", text == "owned");
	RETURN_TEST("test_construct_from_owned_string", result);
}

int test_construct_from_empty() {
	int result = 0;
	String text("");
	ASSERT_FALSE("test_construct_from_empty", static_cast<bool>(text));
	ASSERT_EQUAL("test_construct_from_empty", Size{0}, text.size());
	ASSERT_TRUE("test_construct_from_empty", text.data() != nullptr);
	RETURN_TEST("test_construct_from_empty", result);
}

int test_construct_from_ptr() {
	int result = 0;
	String text("abc");
	ASSERT_EQUAL("test_construct_from_ptr", Size{3}, text.size());
	ASSERT_EQUAL("test_construct_from_ptr", 'a', text[0]);
	ASSERT_EQUAL("test_construct_from_ptr", 'c', text[2]);
	ASSERT_EQUAL("test_construct_from_ptr", '\0', text[3]);
	RETURN_TEST("test_construct_from_ptr", result);
}

int test_construct_from_view() {
	int result = 0;
	String text(std::string_view("hi"));
	ASSERT_EQUAL("test_construct_from_view", Size{2}, text.size());
	ASSERT_TRUE("test_construct_from_view", text == "hi");
	RETURN_TEST("test_construct_from_view", result);
}

int test_default_is_empty() {
	int result = 0;
	String text;
	ASSERT_FALSE("test_default_is_empty", static_cast<bool>(text));
	ASSERT_TRUE("test_default_is_empty", text.empty());
	ASSERT_EQUAL("test_default_is_empty", Size{0}, text.size());
	ASSERT_TRUE("test_default_is_empty", text.data() != nullptr);
	ASSERT_EQUAL("test_default_is_empty", '\0', text.data()[0]);
	RETURN_TEST("test_default_is_empty", result);
}

// -------------------
// Conversions / streams
// -------------------

int test_free_stream_operator() {
	int result = 0;
	String text("streamed");
	std::ostringstream out;
	out << text;
	ASSERT_EQUAL("test_free_stream_operator", std::string("streamed"), out.str());
	RETURN_TEST("test_free_stream_operator", result);
}

int test_stream_null_writes_nothing() {
	int result = 0;
	String text;
	std::ostringstream out;
	out << "pre" << text << "post";
	ASSERT_EQUAL("test_stream_null_writes_nothing", std::string("prepost"), out.str());
	RETURN_TEST("test_stream_null_writes_nothing", result);
}

int test_lookup() {
	int result = 0;
	const String text("before\nafter\n");
	ASSERT_TRUE("test_lookup", text.starts_with("before"));
	ASSERT_TRUE("test_lookup", text.starts_with('b'));
	ASSERT_TRUE("test_lookup", text.ends_with("after\n"));
	ASSERT_TRUE("test_lookup", text.ends_with('\n'));
	ASSERT_FALSE("test_lookup", text.ends_with("nope"));
	ASSERT_TRUE("test_lookup", text.contains("after"));
	ASSERT_TRUE("test_lookup", text.contains('\n'));
	ASSERT_TRUE("test_lookup", text.find("after") == Size{7});
	ASSERT_TRUE("test_lookup", text.find("missing") == String::npos);
	ASSERT_TRUE("test_lookup", text.rfind('\n') == Size{12});
	ASSERT_TRUE("test_lookup", text.find_first_of("axz") == Size{7});
	ASSERT_EQUAL("test_lookup", std::string("after\n"), std::string(text.substr(Size{7})));
	ASSERT_EQUAL("test_lookup", 'b', text.front());
	ASSERT_EQUAL("test_lookup", '\n', text.back());
	ASSERT_EQUAL("test_lookup", 0, text.compare("before\nafter\n"));
	const WString wide(L"before\nafter\n");
	ASSERT_TRUE("test_lookup", wide.ends_with(L"after\n"));
	ASSERT_TRUE("test_lookup", wide.starts_with(L"before"));
	ASSERT_TRUE("test_lookup", wide.contains(L"after"));
	ASSERT_TRUE("test_lookup", wide.find(L"after") == Size{7});
	ASSERT_EQUAL("test_lookup", std::wstring(L"after\n"), std::wstring(wide.substr(Size{7})));
	RETURN_TEST("test_lookup", result);
}

int test_view_and_string() {
	int result = 0;
	String text("abc");
	const std::string_view view = text;
	ASSERT_TRUE("test_view_and_string", view == "abc");
	ASSERT_EQUAL("test_view_and_string", std::string("abc"), std::string(text));
	ASSERT_TRUE("test_view_and_string", std::string_view(text.Bytes()) == "abc");
	RETURN_TEST("test_view_and_string", result);
}

// -------------------
// Copy / move
// -------------------

int test_copy_assign() {
	int result = 0;
	String left("old");
	String right("new");
	left = right;
	ASSERT_TRUE("test_copy_assign", left == "new");
	ASSERT_TRUE("test_copy_assign", right == "new");
	RETURN_TEST("test_copy_assign", result);
}

int test_copy_is_independent() {
	int result = 0;
	String original("alpha");
	String copy(original);
	ASSERT_TRUE("test_copy_is_independent", copy == original);
	ASSERT_TRUE("test_copy_is_independent", copy.data() != original.data());
	RETURN_TEST("test_copy_is_independent", result);
}

int test_move_assign() {
	int result = 0;
	String left("old");
	String right("fresh");
	left = std::move(right);
	ASSERT_TRUE("test_move_assign", left == "fresh");
	ASSERT_FALSE("test_move_assign", static_cast<bool>(right));
	RETURN_TEST("test_move_assign", result);
}

int test_move_leaves_source_null() {
	int result = 0;
	String original("payload");
	String taken(std::move(original));
	ASSERT_TRUE("test_move_leaves_source_null", taken == "payload");
	ASSERT_FALSE("test_move_leaves_source_null", static_cast<bool>(original));
	RETURN_TEST("test_move_leaves_source_null", result);
}

int test_swap_exchanges() {
	int result = 0;
	String left("L");
	String right("R");
	left.swap(right);
	ASSERT_TRUE("test_swap_exchanges", left == "R");
	ASSERT_TRUE("test_swap_exchanges", right == "L");
	swap(left, right);
	ASSERT_TRUE("test_swap_exchanges", left == "L");
	ASSERT_TRUE("test_swap_exchanges", right == "R");
	RETURN_TEST("test_swap_exchanges", result);
}

// -------------------
// Helpers
// -------------------

int test_case_ascii() {
	int result = 0;
	ASSERT_TRUE("test_case_ascii", String::ToUpper("abC") == "ABC");
	ASSERT_TRUE("test_case_ascii", String::ToLower("AbC") == "abc");
	ASSERT_TRUE("test_case_ascii", String("Xy").ToUpper() == "XY");
	ASSERT_TRUE("test_case_ascii", String("Xy").ToLower() == "xy");
	RETURN_TEST("test_case_ascii", result);
}

int test_case_latin1() {
	int result = 0;
	ASSERT_TRUE("test_case_latin1",
		static_cast<std::string_view>(String::ToUpper(std::string_view("café"))) == std::string_view("CAFÉ"));
	ASSERT_TRUE("test_case_latin1",
		static_cast<std::string_view>(String::ToLower(std::string_view("CAFÉ"))) == std::string_view("café"));
	RETURN_TEST("test_case_latin1", result);
}

int test_explode() {
	int result = 0;
	std::queue<String> parts;
	String::Explode("a,,b", ',', parts);
	ASSERT_EQUAL("test_explode", 3u, parts.size());
	ASSERT_TRUE("test_explode", parts.front() == "a");
	parts.pop();
	ASSERT_TRUE("test_explode", parts.front() == "");
	parts.pop();
	ASSERT_TRUE("test_explode", parts.front() == "b");
	const auto from_obj = String("x|y").Explode('|');
	ASSERT_EQUAL("test_explode", 2u, from_obj.size());
	RETURN_TEST("test_explode", result);
}

int test_is_integer() {
	int result = 0;
	ASSERT_TRUE("test_is_integer", String::IsInteger("0"));
	ASSERT_TRUE("test_is_integer", String::IsInteger("-12"));
	ASSERT_TRUE("test_is_integer", String::IsInteger("+3"));
	ASSERT_FALSE("test_is_integer", String::IsInteger(""));
	ASSERT_FALSE("test_is_integer", String::IsInteger("-"));
	ASSERT_FALSE("test_is_integer", String::IsInteger("1a"));
	ASSERT_TRUE("test_is_integer", String("42").IsInteger());
	RETURN_TEST("test_is_integer", result);
}

int test_newlines_and_space() {
	int result = 0;
	ASSERT_TRUE("test_newlines_and_space", String::SanitizeNewlines("a\r\nb\n") == "a\nb\n");
	ASSERT_TRUE("test_newlines_and_space", String("a\r\n").SanitizeNewlines() == "a\n");
	ASSERT_TRUE("test_newlines_and_space", String::RemoveWhitespace(" a\tb\n") == "ab");
	ASSERT_TRUE("test_newlines_and_space", String(" x ").RemoveWhitespace() == "x");
	RETURN_TEST("test_newlines_and_space", result);
}

int test_split() {
	int result = 0;
	std::vector<String> tokens;
	String::Split("  a  bb\tc ", tokens);
	ASSERT_EQUAL("test_split", 3u, tokens.size());
	ASSERT_TRUE("test_split", tokens[0] == "a");
	ASSERT_TRUE("test_split", tokens[2] == "c");
	const auto from_obj = String("x y").Split();
	ASSERT_EQUAL("test_split", 2u, from_obj.size());
	RETURN_TEST("test_split", result);
}

// -------------------
// Range
// -------------------

int test_range_iterators() {
	int result = 0;
	String text("xyz");
	ASSERT_TRUE("test_range_iterators", text.begin() == text.data());
	ASSERT_TRUE("test_range_iterators", text.end() == text.data() + 3);
	ASSERT_TRUE("test_range_iterators", text.cbegin() == text.begin());
	ASSERT_TRUE("test_range_iterators", text.cend() == text.end());
	char acc[4] {};
	std::size_t n = 0;
	for (char c : text)
		acc[n++] = c;
	ASSERT_EQUAL("test_range_iterators", 3u, n);
	ASSERT_EQUAL("test_range_iterators", 'x', acc[0]);
	ASSERT_EQUAL("test_range_iterators", 'z', acc[2]);
	RETURN_TEST("test_range_iterators", result);
}

int test_range_empty() {
	int result = 0;
	String text;
	ASSERT_TRUE("test_range_empty", text.begin() != nullptr);
	ASSERT_TRUE("test_range_empty", text.begin() == text.end());
	RETURN_TEST("test_range_empty", result);
}

// -------------------
// Wide
// -------------------

int test_wide_ascii_and_empty() {
	int result = 0;
	String ascii("Hi");
	ASSERT_TRUE("test_wide_ascii_and_empty", WString(ascii) == L"Hi");
	ASSERT_TRUE("test_wide_ascii_and_empty", String(WString(ascii)) == "Hi");
	String empty("");
	ASSERT_TRUE("test_wide_ascii_and_empty", WString(empty) == L"");
	ASSERT_TRUE("test_wide_ascii_and_empty", String(WString(empty)) == "");
	RETURN_TEST("test_wide_ascii_and_empty", result);
}

int test_wide_roundtrip() {
	int result = 0;
	String utf8(std::string_view("café"));
	WString from_ctor(utf8);
	ASSERT_TRUE("test_wide_roundtrip", std::wstring(from_ctor) == L"café");
	WString from_op = static_cast<WString>(utf8);
	ASSERT_TRUE("test_wide_roundtrip", from_op == from_ctor);
	String back_ctor(from_ctor);
	ASSERT_TRUE("test_wide_roundtrip", back_ctor == utf8);
	String back_op = static_cast<String>(from_ctor);
	ASSERT_TRUE("test_wide_roundtrip", back_op == utf8);
	RETURN_TEST("test_wide_roundtrip", result);
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
	result += test_mutable_text_algorithms();
	result += test_string_size_modifiers();
	result += test_view_feeds_algorithms();

	// -------------------
	// Compare
	// -------------------
	result += test_equal_and_order();
	result += test_hash();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_from_owned_string();
	result += TestEmbeddedNul();
	result += test_construct_from_empty();
	result += test_construct_from_ptr();
	result += test_construct_from_view();
	result += test_default_is_empty();

	// -------------------
	// Conversions / streams
	// -------------------
	result += test_free_stream_operator();
	result += test_stream_null_writes_nothing();
	result += test_view_and_string();
	result += test_lookup();

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
	result += test_range_empty();

	// -------------------
	// Wide
	// -------------------
	result += test_wide_ascii_and_empty();
	result += test_wide_roundtrip();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
