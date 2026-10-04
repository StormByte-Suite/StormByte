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

#include <StormByte/safe/cstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/safe/wcstring.hxx>

#include <algorithm>
#include <compare>
#include <cwchar>
#include <functional>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>

using namespace StormByte;

static_assert(sizeof(Safe::WCString) == sizeof(wchar_t*));

namespace {
	const wchar_t* View(const Safe::WCString& text) {
		return static_cast<const wchar_t*>(text);
	}
}

// -------------------
// Construct
// -------------------

int test_default_is_null() {
	int result = 0;
	Safe::WCString text;
	ASSERT_TRUE("test_default_is_null", View(text) == nullptr);
	ASSERT_EQUAL("test_default_is_null", Size{0}, text.Length());
	ASSERT_FALSE("test_default_is_null", static_cast<bool>(text));
	RETURN_TEST("test_default_is_null", result);
}

int test_construct_from_null() {
	int result = 0;
	Safe::WCString text(static_cast<const wchar_t*>(nullptr));
	ASSERT_TRUE("test_construct_from_null", View(text) == nullptr);
	ASSERT_FALSE("test_construct_from_null", static_cast<bool>(text));
	RETURN_TEST("test_construct_from_null", result);
}

int test_construct_from_empty() {
	int result = 0;
	Safe::WCString text(L"");
	ASSERT_TRUE("test_construct_from_empty", View(text) != nullptr);
	ASSERT_TRUE("test_construct_from_empty", static_cast<bool>(text));
	ASSERT_EQUAL("test_construct_from_empty", 0, std::wcscmp(View(text), L""));
	ASSERT_EQUAL("test_construct_from_empty", Size{0}, text.Length());
	RETURN_TEST("test_construct_from_empty", result);
}

int test_construct_copies_text() {
	int result = 0;
	const wchar_t raw[] = L"hello";
	Safe::WCString text(raw);
	ASSERT_TRUE("test_construct_copies_text", View(text) != raw);
	ASSERT_EQUAL("test_construct_copies_text", 0, std::wcscmp(View(text), L"hello"));
	ASSERT_EQUAL("test_construct_copies_text", Size{5}, text.Length());
	ASSERT_TRUE("test_construct_copies_text", static_cast<bool>(text));
	RETURN_TEST("test_construct_copies_text", result);
}

int test_construct_stops_at_embedded_nul() {
	int result = 0;
	const wchar_t raw[] = { L'a', L'b', L'\0', L'c', L'\0' };
	Safe::WCString text(raw);
	ASSERT_EQUAL("test_construct_stops_at_embedded_nul", Size{2}, text.Length());
	ASSERT_EQUAL("test_construct_stops_at_embedded_nul", 0, std::wcscmp(View(text), L"ab"));
	RETURN_TEST("test_construct_stops_at_embedded_nul", result);
}

int test_construct_unicode() {
	int result = 0;
	Safe::WCString text(L"cañón 日本語");
	ASSERT_TRUE("test_construct_unicode", View(text) != nullptr);
	ASSERT_EQUAL("test_construct_unicode", 0, std::wcscmp(View(text), L"cañón 日本語"));
	ASSERT_EQUAL("test_construct_unicode", Size{std::wcslen(L"cañón 日本語")}, text.Length());
	RETURN_TEST("test_construct_unicode", result);
}

int test_construct_long() {
	int result = 0;
	const std::wstring raw(4096, L'X');
	Safe::WCString text(raw.c_str());
	ASSERT_EQUAL("test_construct_long", Size{raw.size()}, text.Length());
	ASSERT_EQUAL("test_construct_long", 0, std::wcscmp(View(text), raw.c_str()));
	RETURN_TEST("test_construct_long", result);
}

// -------------------
// Copy / move
// -------------------

int test_copy_is_independent() {
	int result = 0;
	Safe::WCString original(L"alpha");
	Safe::WCString copy(original);
	ASSERT_TRUE("test_copy_is_independent", View(copy) != View(original));
	copy.Reset(L"beta");
	ASSERT_EQUAL("test_copy_is_independent", 0, std::wcscmp(View(original), L"alpha"));
	ASSERT_EQUAL("test_copy_is_independent", 0, std::wcscmp(View(copy), L"beta"));
	RETURN_TEST("test_copy_is_independent", result);
}

int test_copy_null() {
	int result = 0;
	Safe::WCString original;
	Safe::WCString copy(original);
	ASSERT_FALSE("test_copy_null", static_cast<bool>(original));
	ASSERT_FALSE("test_copy_null", static_cast<bool>(copy));
	RETURN_TEST("test_copy_null", result);
}

int test_copy_assign_overwrites() {
	int result = 0;
	Safe::WCString left(L"old");
	Safe::WCString right(L"new");
	left = right;
	ASSERT_TRUE("test_copy_assign_overwrites", View(left) != View(right));
	ASSERT_EQUAL("test_copy_assign_overwrites", 0, std::wcscmp(View(left), L"new"));
	RETURN_TEST("test_copy_assign_overwrites", result);
}

int test_copy_assign_self() {
	int result = 0;
	Safe::WCString text(L"self");
	Safe::WCString& alias = text;
	text = alias;
	ASSERT_EQUAL("test_copy_assign_self", 0, std::wcscmp(View(text), L"self"));
	RETURN_TEST("test_copy_assign_self", result);
}

int test_move_leaves_source_null() {
	int result = 0;
	Safe::WCString original(L"payload");
	const wchar_t* raw = View(original);
	Safe::WCString taken(std::move(original));
	ASSERT_FALSE("test_move_leaves_source_null", static_cast<bool>(original));
	ASSERT_TRUE("test_move_leaves_source_null", View(taken) == raw);
	RETURN_TEST("test_move_leaves_source_null", result);
}

int test_move_assign_leaves_source_null() {
	int result = 0;
	Safe::WCString left(L"old");
	Safe::WCString right(L"fresh");
	const wchar_t* raw = View(right);
	left = std::move(right);
	ASSERT_FALSE("test_move_assign_leaves_source_null", static_cast<bool>(right));
	ASSERT_TRUE("test_move_assign_leaves_source_null", View(left) == raw);
	RETURN_TEST("test_move_assign_leaves_source_null", result);
}

int test_move_assign_self() {
	int result = 0;
	Safe::WCString text(L"self-move");
	Safe::WCString& alias = text;
	text = std::move(alias);
	ASSERT_EQUAL("test_move_assign_self", 0, std::wcscmp(View(text), L"self-move"));
	RETURN_TEST("test_move_assign_self", result);
}

int test_move_from_null() {
	int result = 0;
	Safe::WCString original;
	Safe::WCString taken(std::move(original));
	ASSERT_FALSE("test_move_from_null", static_cast<bool>(original));
	ASSERT_FALSE("test_move_from_null", static_cast<bool>(taken));
	RETURN_TEST("test_move_from_null", result);
}

// -------------------
// Reset / swap
// -------------------

int test_reset_replaces() {
	int result = 0;
	Safe::WCString text(L"first");
	text.Reset(L"second");
	ASSERT_EQUAL("test_reset_replaces", 0, std::wcscmp(View(text), L"second"));
	text.Reset();
	ASSERT_FALSE("test_reset_replaces", static_cast<bool>(text));
	text.Reset(L"third");
	ASSERT_TRUE("test_reset_replaces", static_cast<bool>(text));
	RETURN_TEST("test_reset_replaces", result);
}

int test_reserve_grows_and_never_shrinks() {
	int result = 0;
	Safe::WCString text(L"alpha");
	text.reserve(Size{64});
	const wchar_t* reserved = View(text);
	ASSERT_TRUE("test_reserve_grows_and_never_shrinks", text.capacity() >= Size{64});
	ASSERT_EQUAL("test_reserve_grows_and_never_shrinks", Size{5}, text.Length());
	ASSERT_EQUAL("test_reserve_grows_and_never_shrinks", 0, std::wcscmp(reserved, L"alpha"));
	text.reserve(Size{2});
	ASSERT_TRUE("test_reserve_grows_and_never_shrinks", View(text) == reserved);
	ASSERT_TRUE("test_reserve_grows_and_never_shrinks", text.capacity() >= Size{64});
	text.Reset(L"beta");
	ASSERT_TRUE("test_reserve_grows_and_never_shrinks", View(text) == reserved);
	ASSERT_EQUAL("test_reserve_grows_and_never_shrinks", 0, std::wcscmp(View(text), L"beta"));
	Safe::WCString moved(std::move(text));
	ASSERT_TRUE("test_reserve_grows_and_never_shrinks", text.capacity() == Size{});
	ASSERT_TRUE("test_reserve_grows_and_never_shrinks", moved.capacity() >= Size{64});
	RETURN_TEST("test_reserve_grows_and_never_shrinks", result);
}

int test_reserve_materializes_null_as_empty() {
	int result = 0;
	Safe::WCString text;
	text.reserve(Size{16});
	ASSERT_TRUE("test_reserve_materializes_null_as_empty", static_cast<bool>(text));
	ASSERT_TRUE("test_reserve_materializes_null_as_empty", text.capacity() >= Size{16});
	ASSERT_EQUAL("test_reserve_materializes_null_as_empty", Size{}, text.Length());
	ASSERT_EQUAL("test_reserve_materializes_null_as_empty", L'\0', text[Size{}]);
	RETURN_TEST("test_reserve_materializes_null_as_empty", result);
}

int test_swap_exchanges() {
	int result = 0;
	Safe::WCString left(L"L");
	Safe::WCString right(L"R");
	left.swap(right);
	ASSERT_TRUE("test_swap_exchanges", left == L"R");
	ASSERT_TRUE("test_swap_exchanges", right == L"L");
	swap(left, right);
	ASSERT_TRUE("test_swap_exchanges", left == L"L");
	ASSERT_TRUE("test_swap_exchanges", right == L"R");
	RETURN_TEST("test_swap_exchanges", result);
}

// -------------------
// Observers
// -------------------

int test_subscript_characters() {
	int result = 0;
	Safe::WCString text(L"ab");
	ASSERT_EQUAL("test_subscript_characters", L'a', text[Size{0}]);
	ASSERT_EQUAL("test_subscript_characters", L'b', text[Size{1}]);
	RETURN_TEST("test_subscript_characters", result);
}

int test_subscript_nul_at_length() {
	int result = 0;
	Safe::WCString text(L"ab");
	ASSERT_EQUAL("test_subscript_nul_at_length", L'\0', text[text.Length()]);
	RETURN_TEST("test_subscript_nul_at_length", result);
}

int test_subscript_empty() {
	int result = 0;
	Safe::WCString text(L"");
	ASSERT_EQUAL("test_subscript_empty", L'\0', text[Size{0}]);
	ASSERT_EQUAL("test_subscript_empty", L'\0', text[text.Length()]);
	RETURN_TEST("test_subscript_empty", result);
}

int test_algorithm_ranges() {
	int result = 0;
	const Safe::WCString text(L"algorithm");
	ASSERT_TRUE("test_algorithm_ranges", std::find(text.begin(), text.end(), L'r') == text.begin() + 4);
	ASSERT_EQUAL("test_algorithm_ranges", 1, std::count(text.cbegin(), text.cend(), L'a'));
	std::wstring copied;
	std::ranges::copy(text, std::back_inserter(copied));
	ASSERT_TRUE("test_algorithm_ranges", copied == L"algorithm");
	const Safe::WCString null;
	ASSERT_TRUE("test_algorithm_ranges", null.begin() == null.end());
	Safe::WCString mutableText(L"dcba");
	std::ranges::sort(mutableText);
	ASSERT_TRUE("test_algorithm_ranges", mutableText == L"abcd");
	std::ranges::reverse(mutableText);
	ASSERT_TRUE("test_algorithm_ranges", mutableText == L"dcba");
	RETURN_TEST("test_algorithm_ranges", result);
}

// -------------------
// Conversions / streams
// -------------------

int test_wstring_conversion_copies() {
	int result = 0;
	Safe::WCString text(L"bridge");
	const std::wstring copy = text;
	ASSERT_TRUE("test_wstring_conversion_copies", copy == L"bridge");
	text.Reset(L"changed");
	ASSERT_TRUE("test_wstring_conversion_copies", copy == L"bridge");
	RETURN_TEST("test_wstring_conversion_copies", result);
}

int test_wstring_conversion_from_null() {
	int result = 0;
	Safe::WCString text;
	const std::wstring copy = text;
	ASSERT_TRUE("test_wstring_conversion_from_null", copy.empty());
	RETURN_TEST("test_wstring_conversion_from_null", result);
}

int test_free_stream_operator() {
	int result = 0;
	Safe::WCString text(L"streamed");
	std::wostringstream out;
	out << text;
	ASSERT_TRUE("test_free_stream_operator", out.str() == L"streamed");
	RETURN_TEST("test_free_stream_operator", result);
}

int test_stream_null_writes_nothing() {
	int result = 0;
	Safe::WCString text;
	std::wostringstream out;
	out << L"pre" << text << L"post";
	ASSERT_TRUE("test_stream_null_writes_nothing", out.str() == L"prepost");
	RETURN_TEST("test_stream_null_writes_nothing", result);
}

// -------------------
// Bool / equality / order
// -------------------

int test_bool_empty_is_valid() {
	int result = 0;
	Safe::WCString empty(L"");
	Safe::WCString missing;
	ASSERT_TRUE("test_bool_empty_is_valid", static_cast<bool>(empty));
	ASSERT_FALSE("test_bool_empty_is_valid", static_cast<bool>(missing));
	ASSERT_TRUE("test_bool_empty_is_valid", empty != missing);
	RETURN_TEST("test_bool_empty_is_valid", result);
}

int test_equal_content_not_pointer() {
	int result = 0;
	Safe::WCString a(L"same");
	Safe::WCString b(L"same");
	ASSERT_TRUE("test_equal_content_not_pointer", View(a) != View(b));
	ASSERT_TRUE("test_equal_content_not_pointer", a == b);
	ASSERT_FALSE("test_equal_content_not_pointer", a != b);
	ASSERT_TRUE("test_equal_content_not_pointer", a == L"same");
	ASSERT_TRUE("test_equal_content_not_pointer", L"same" == a);
	ASSERT_TRUE("test_equal_content_not_pointer", a != L"other");
	ASSERT_TRUE("test_equal_content_not_pointer", L"other" != a);
	RETURN_TEST("test_equal_content_not_pointer", result);
}

int test_null_equals_null_not_empty() {
	int result = 0;
	Safe::WCString a;
	Safe::WCString b;
	Safe::WCString empty(L"");
	ASSERT_TRUE("test_null_equals_null_not_empty", a == b);
	ASSERT_FALSE("test_null_equals_null_not_empty", a == empty);
	ASSERT_TRUE("test_null_equals_null_not_empty", a != empty);
	ASSERT_TRUE("test_null_equals_null_not_empty", a == static_cast<const wchar_t*>(nullptr));
	ASSERT_TRUE("test_null_equals_null_not_empty", empty != static_cast<const wchar_t*>(nullptr));
	RETURN_TEST("test_null_equals_null_not_empty", result);
}

int test_spaceship_order() {
	int result = 0;
	Safe::WCString missing;
	Safe::WCString empty(L"");
	Safe::WCString alpha(L"alpha");
	Safe::WCString beta(L"beta");
	ASSERT_TRUE("test_spaceship_order", (missing <=> missing) == std::strong_ordering::equal);
	ASSERT_TRUE("test_spaceship_order", (missing <=> empty) == std::strong_ordering::less);
	ASSERT_TRUE("test_spaceship_order", (empty <=> missing) == std::strong_ordering::greater);
	ASSERT_TRUE("test_spaceship_order", (alpha <=> beta) == std::strong_ordering::less);
	ASSERT_TRUE("test_spaceship_order", (beta <=> alpha) == std::strong_ordering::greater);
	ASSERT_TRUE("test_spaceship_order", (alpha <=> L"alpha") == std::strong_ordering::equal);
	ASSERT_TRUE("test_spaceship_order", alpha < beta);
	ASSERT_TRUE("test_spaceship_order", missing < alpha);
	RETURN_TEST("test_spaceship_order", result);
}

int test_roundtrip_with_cstring() {
	int result = 0;
	const Safe::WCString original(L"cañón 日本語");
	const Safe::CString narrow(original);
	const Safe::WCString back(narrow);
	ASSERT_TRUE("test_roundtrip_with_cstring", back == original);
	const Safe::CString ascii("1.00 KiB");
	ASSERT_TRUE("test_roundtrip_with_cstring", Safe::WCString(ascii) == L"1.00 KiB");
	ASSERT_TRUE("test_roundtrip_with_cstring", !Safe::WCString(Safe::CString()));
	ASSERT_TRUE("test_roundtrip_with_cstring", !Safe::CString(Safe::WCString()));
	RETURN_TEST("test_roundtrip_with_cstring", result);
}

int test_hash_and_containers() {
	int result = 0;
	Safe::WCString a(L"key");
	Safe::WCString b(L"key");
	ASSERT_EQUAL("test_hash_and_containers", std::hash<Safe::WCString>{}(a), std::hash<Safe::WCString>{}(b));
	ASSERT_EQUAL("test_hash_and_containers", 0u, std::hash<Safe::WCString>{}(Safe::WCString()));
	std::set<Safe::WCString> ordered;
	ordered.insert(Safe::WCString(L"b"));
	ordered.insert(Safe::WCString(L"a"));
	ASSERT_TRUE("test_hash_and_containers", *ordered.begin() == L"a");
	std::unordered_set<Safe::WCString> hashed;
	hashed.insert(Safe::WCString(L"k"));
	ASSERT_TRUE("test_hash_and_containers", hashed.contains(Safe::WCString(L"k")));
	RETURN_TEST("test_hash_and_containers", result);
}

int main() {
	int result = 0;

	// -------------------
	// Construct
	// -------------------
	result += test_default_is_null();
	result += test_construct_from_null();
	result += test_construct_from_empty();
	result += test_construct_copies_text();
	result += test_construct_stops_at_embedded_nul();
	result += test_construct_unicode();
	result += test_construct_long();

	// -------------------
	// Copy / move
	// -------------------
	result += test_copy_is_independent();
	result += test_copy_null();
	result += test_copy_assign_overwrites();
	result += test_copy_assign_self();
	result += test_move_leaves_source_null();
	result += test_move_assign_leaves_source_null();
	result += test_move_assign_self();
	result += test_move_from_null();

	// -------------------
	// Reset / swap
	// -------------------
	result += test_reset_replaces();
	result += test_reserve_grows_and_never_shrinks();
	result += test_reserve_materializes_null_as_empty();
	result += test_swap_exchanges();

	// -------------------
	// Observers
	// -------------------
	result += test_subscript_characters();
	result += test_subscript_nul_at_length();
	result += test_subscript_empty();
	result += test_algorithm_ranges();

	// -------------------
	// Conversions / streams
	// -------------------
	result += test_wstring_conversion_copies();
	result += test_wstring_conversion_from_null();
	result += test_free_stream_operator();
	result += test_stream_null_writes_nothing();

	// -------------------
	// Bool / equality / order
	// -------------------
	result += test_bool_empty_is_valid();
	result += test_equal_content_not_pointer();
	result += test_null_equals_null_not_empty();
	result += test_spaceship_order();
	result += test_hash_and_containers();
	result += test_roundtrip_with_cstring();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
