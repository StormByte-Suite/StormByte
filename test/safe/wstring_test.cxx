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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace StormByte;

// -------------------
// Algorithm
// -------------------

int test_algorithm_read_and_rewrite_keeps_size() {
	Safe::WString text(L"cbaab");
	const Size owned = text.size();
	std::sort(text.begin(), text.end());
	std::stable_sort(text.begin(), text.end());
	std::partial_sort(text.begin(), text.begin() + 3, text.end());
	std::nth_element(text.begin(), text.begin() + 2, text.end());
	std::ranges::sort(text);
	ASSERT_EQUAL(owned, text.size());
	ASSERT_TRUE(std::is_sorted(text.begin(), text.end()));
	ASSERT_TRUE(std::ranges::is_sorted(text));
	auto minmax = std::minmax_element(text.begin(), text.end());
	ASSERT_EQUAL(L'a', *minmax.first);
	ASSERT_EQUAL(L'c', *minmax.second);
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count(text.begin(), text.end(), L'a'));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count_if(text.begin(), text.end(), [](wchar_t ch) { return ch == L'b'; }));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::ranges::count(text, L'a'));
	ASSERT_TRUE(std::all_of(text.begin(), text.end(), [](wchar_t ch) { return ch >= L'a' && ch <= L'c'; }));
	ASSERT_TRUE(std::any_of(text.begin(), text.end(), [](wchar_t ch) { return ch == L'b'; }));
	ASSERT_TRUE(std::none_of(text.begin(), text.end(), [](wchar_t ch) { return ch == L'z'; }));
	ASSERT_TRUE(std::adjacent_find(text.begin(), text.end()) != text.end());
	ASSERT_TRUE(std::binary_search(text.begin(), text.end(), L'b'));
	ASSERT_TRUE(std::ranges::binary_search(text, L'c'));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::upper_bound(text.begin(), text.end(), L'b') - std::lower_bound(text.begin(), text.end(), L'b'));
	ASSERT_TRUE(std::equal_range(text.begin(), text.end(), L'a').first == text.begin());
	ASSERT_TRUE(std::find(text.begin(), text.end(), L'c') != text.end());
	ASSERT_TRUE(std::find_if(text.begin(), text.end(), [](wchar_t ch) { return ch == L'b'; }) != text.end());
	ASSERT_TRUE(std::find_if_not(text.begin(), text.end(), [](wchar_t ch) { return ch == L'a'; }) != text.begin());
	ASSERT_TRUE(std::ranges::find(text, L'a') == text.begin());
	const wchar_t needle[] = {L'a', L'a'};
	ASSERT_TRUE(std::search(text.begin(), text.end(), needle, needle + 2) == text.begin());
	ASSERT_TRUE(std::find_end(text.begin(), text.end(), needle, needle + 2) == text.begin());
	ASSERT_TRUE(std::search_n(text.begin(), text.end(), 2, L'a') == text.begin());
	ASSERT_TRUE(std::find_first_of(text.begin(), text.end(), needle, needle + 1) == text.begin());
	std::for_each(text.begin(), text.end(), [](wchar_t& ch) { ch = ch; });
	std::for_each_n(text.begin(), 2, [](wchar_t& ch) { ch = ch; });
	std::ranges::for_each(text, [](wchar_t& ch) { ch = ch; });
	std::reverse(text.begin(), text.end());
	std::ranges::reverse(text);
	std::rotate(text.begin(), text.begin() + 1, text.end());
	std::ranges::rotate(text, text.begin() + 1);
	std::replace(text.begin(), text.end(), L'a', L'x');
	std::replace_if(text.begin(), text.end(), [](wchar_t ch) { return ch == L'x'; }, L'a');
	std::ranges::replace(text, L'a', L'x');
	std::ranges::replace(text, L'x', L'a');
	std::fill(text.begin(), text.end(), L'q');
	std::fill_n(text.begin(), 2, L'r');
	std::ranges::fill(text, L'q');
	std::generate(text.begin(), text.end(), [n = 0]() mutable { return static_cast<wchar_t>(L'a' + (n++ % 3)); });
	std::generate_n(text.begin(), 1, [] { return L'a'; });
	std::transform(text.begin(), text.end(), text.begin(), [](wchar_t ch) { return ch; });
	std::ranges::transform(text, text.begin(), [](wchar_t ch) { return ch; });
	Safe::WString other(text);
	ASSERT_TRUE(std::equal(text.begin(), text.end(), other.begin()));
	ASSERT_TRUE(std::ranges::equal(text, other));
	ASSERT_TRUE(std::mismatch(text.begin(), text.end(), other.begin()).first == text.end());
	ASSERT_FALSE(std::lexicographical_compare(text.begin(), text.end(), other.begin(), other.end()));
	std::vector<wchar_t> copied(static_cast<std::size_t>(owned));
	std::copy(text.begin(), text.end(), copied.begin());
	std::copy_n(text.begin(), 2, copied.begin());
	std::copy_backward(text.begin(), text.end(), copied.end());
	std::reverse_copy(text.begin(), text.end(), copied.begin());
	std::replace_copy(text.begin(), text.end(), copied.begin(), L'a', L'z');
	std::ranges::copy(text, copied.begin());
	std::iter_swap(text.begin(), text.end() - 1);
	std::swap_ranges(text.begin(), text.begin() + 2, other.begin());
	std::partition(text.begin(), text.end(), [](wchar_t ch) { return ch < L'n'; });
	std::sort(text.begin(), text.end());
	ASSERT_TRUE(std::unique(text.begin(), text.end()) <= text.end());
	std::reverse(text.rbegin(), text.rend());
	ASSERT_EQUAL(owned, text.size());
	ASSERT_EQUAL(L'\0', text.c_str()[static_cast<std::size_t>(text.size())]);
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_copy_move_and_narrow() {
	Safe::WString empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_FALSE(static_cast<bool>(empty));
	ASSERT_NOT_NULL(empty.c_str());
	ASSERT_NOT_NULL(empty.Bytes());
	ASSERT_EQUAL(L'\0', empty.c_str()[0]);
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.crbegin() == empty.crend());
	Safe::WString null_pointer(static_cast<const wchar_t*>(nullptr));
	ASSERT_TRUE(null_pointer == static_cast<const wchar_t*>(nullptr));
	Safe::WString literal(L"ab");
	ASSERT_TRUE(static_cast<bool>(literal));
	ASSERT_EQUAL(L'b', literal.at(Size{1}));
	literal[Size{0}] = L'a';
	ASSERT_EQUAL(L'a', literal.front());
	ASSERT_EQUAL(L'b', literal.back());
	ASSERT_EQUAL(L'\0', literal.c_str()[2]);
	ASSERT_EQUAL(L'b', *literal.rbegin());
	const wchar_t embedded[] = {L'a', L'\0', L'b'};
	Safe::WString kept(std::wstring_view(embedded, 3));
	ASSERT_EQUAL(Size{3}, kept.size());
	Safe::WString cut(embedded);
	ASSERT_EQUAL(Size{1}, cut.size());
	Safe::WString repeated(Size{4}, L'z');
	Safe::WString from_iter(literal.begin(), literal.end());
	Safe::WString slice(literal, Size{1}, Size{1});
	Safe::WString tail(literal, Size{1});
	ASSERT_EQUAL(L'b', slice[Size{0}]);
	ASSERT_EQUAL(Size{1}, tail.size());
	ASSERT_THROWS(Safe::WString(literal, Size{4}), Safe::OutOfBoundsError);
	Safe::WString copy(literal);
	Safe::WString moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	const std::wstring exported(literal);
	const std::wstring_view borrowed = literal;
	const wchar_t* raw = static_cast<const wchar_t*>(literal);
	ASSERT_EQUAL(std::wstring(L"ab"), exported);
	ASSERT_EQUAL(std::size_t{2}, borrowed.size());
	ASSERT_EQUAL(L'a', raw[0]);
	Safe::String narrow("ok");
	Safe::WString widened(narrow);
	Safe::String round(static_cast<Safe::String>(widened));
	ASSERT_EQUAL(std::string_view("ok"), std::string_view(round));
	Safe::WString long_text(Size{Safe::WString::SSO_CAPACITY + 3}, L'L');
	ASSERT_TRUE(long_text.capacity() >= long_text.size());
	ASSERT_TRUE(long_text.max_size() >= long_text.size());
	Safe::WString long_moved(std::move(long_text));
	ASSERT_TRUE(long_text.empty());
	ASSERT_EQUAL(L'L', long_moved.front());
	RETURN_TEST(0);
}

// -------------------
// Mutate
// -------------------

int test_mutate_edit_reserve_and_concat() {
	Safe::WString text;
	text.append(L"ab");
	text.append(Size{2}, L'c');
	text.append(std::wstring_view(L"d").begin(), std::wstring_view(L"d").end());
	text.append_range(std::wstring_view(L"e"));
	text += L'f';
	text += std::wstring_view(L"g");
	text += Safe::WString(L"h");
	text.push_back(L'i');
	text.pop_back();
	text.insert(Size{2}, L"XY");
	text.insert(Size{0}, Size{1}, L'Q');
	text.insert(text.begin(), {L'P'});
	text.insert_range(text.end(), std::wstring_view(L"!"));
	text.erase(Size{0}, Size{1});
	text.erase(text.begin());
	text.erase(text.begin(), text.begin() + 1);
	text.replace(Size{0}, Size{1}, L"ZZ");
	text.replace(text.begin(), text.begin() + 1, L"N");
	text.resize(Size{3}, L'q');
	text.resize(Size{4});
	text.reserve(Size{40});
	ASSERT_TRUE(text.capacity() >= Size{40});
	text.reserve(Size{1});
	text.shrink_to_fit();
	text.clear();
	ASSERT_TRUE(text.empty());
	text.assign(Size{2}, L'k');
	text.assign(std::wstring_view(L"no"));
	const wchar_t raw[] = {L'o', L'k'};
	text.assign(raw, raw + 2);
	text.assign_range(std::wstring_view(L"ok"));
	Safe::WString sum = text + Safe::WString(L"x");
	Safe::WString sum_view = text + std::wstring_view(L"y");
	Safe::WString sum_left = std::wstring_view(L"a") + text;
	ASSERT_TRUE(sum.ends_with(L'x'));
	ASSERT_TRUE(sum_view.ends_with(L'y'));
	ASSERT_TRUE(sum_left.starts_with(L'a'));
	Safe::WString swapped(L"left");
	text.swap(swapped);
	swap(text, swapped);
	wchar_t buffer[8] = {};
	ASSERT_EQUAL(Size{1}, text.copy(buffer, Size{1}, Size{1}));
	const Safe::WString& read = text;
	ASSERT_EQUAL(L'k', read.at(Size{1}));
	ASSERT_EQUAL(L'k', read[Size{1}]);
	ASSERT_EQUAL(L'o', read.front());
	ASSERT_EQUAL(L'k', read.back());
	ASSERT_THROWS(text.at(text.size()), Safe::OutOfBoundsError);
	std::wostringstream stream;
	stream << text;
	ASSERT_EQUAL(std::wstring(L"ok"), stream.str());
	RETURN_TEST(0);
}

// -------------------
// Search
// -------------------

int test_search_helpers_and_order() {
	Safe::WString text(L"abXab");
	ASSERT_TRUE(text.starts_with(L"ab"));
	ASSERT_TRUE(text.starts_with(L'a'));
	ASSERT_TRUE(text.ends_with(L"ab"));
	ASSERT_TRUE(text.ends_with(L'b'));
	ASSERT_TRUE(text.contains(L"bX"));
	ASSERT_TRUE(text.contains(L'X'));
	ASSERT_FALSE(text.contains(L'z'));
	ASSERT_EQUAL(Size{0}, text.find(L"ab"));
	ASSERT_EQUAL(Size{3}, text.find(L"ab", Size{1}));
	ASSERT_EQUAL(Size{2}, text.find(L'X'));
	ASSERT_EQUAL(Size{0}, text.find(L"ab", Size{0}, Size{2}));
	ASSERT_EQUAL(Safe::WString::npos, text.find(L"no"));
	ASSERT_EQUAL(Size{3}, text.rfind(L"ab"));
	ASSERT_EQUAL(Size{4}, text.rfind(L'b'));
	ASSERT_EQUAL(Size{3}, text.rfind(L"ab", Safe::WString::npos, Size{2}));
	ASSERT_EQUAL(Size{2}, text.find_first_of(L"XZ"));
	ASSERT_EQUAL(Size{0}, text.find_first_of(L'a'));
	ASSERT_EQUAL(Size{4}, text.find_last_of(L"ab"));
	ASSERT_EQUAL(Size{4}, text.find_last_of(L'b'));
	ASSERT_EQUAL(Size{2}, text.find_first_not_of(L"ab"));
	ASSERT_EQUAL(Size{1}, text.find_first_not_of(L'a'));
	ASSERT_EQUAL(Size{2}, text.find_last_not_of(L"ab"));
	ASSERT_EQUAL(Size{3}, text.find_last_not_of(L'b'));
	ASSERT_EQUAL(std::wstring_view(L"Xab"), std::wstring_view(text.substr(Size{2})));
	ASSERT_TRUE(text.substr(Size{9}).empty());
	ASSERT_EQUAL(0, text.compare(L"abXab"));
	ASSERT_TRUE(text.compare(Size{2}, Size{1}, L"X") == 0);
	ASSERT_TRUE(text.ToLower().contains(L'x'));
	ASSERT_TRUE(Safe::WString::ToUpper(L"abXab").contains(L'X'));
	ASSERT_TRUE(Safe::WString::ToLower(L"Ab").contains(L'a'));
	ASSERT_EQUAL(std::wstring_view(L"a\nb"), std::wstring_view(Safe::WString::SanitizeNewlines(L"a\r\nb")));
	ASSERT_EQUAL(std::wstring_view(L"ab"), std::wstring_view(Safe::WString(L" a b").RemoveWhitespace()));
	ASSERT_TRUE(Safe::WString::IsInteger(L"-12"));
	ASSERT_TRUE(Safe::WString(L"42").IsInteger());
	ASSERT_FALSE(Safe::WString(L"4a").IsInteger());
	ASSERT_EQUAL(Size{2}, Safe::WString::Split(L"a b").size());
	std::vector<Safe::WString> caller{Safe::WString(L"p")};
	ASSERT_EQUAL(Size{1}, Safe::WString::Split(caller).size());
	ASSERT_EQUAL(Size{3}, Safe::WString(L"a b c").Split().size());
	ASSERT_EQUAL(Size{3}, Safe::WString::Explode(L"a,,b", L',').size());
	std::queue<Safe::WString> caller_queue;
	caller_queue.push(Safe::WString(L"p"));
	ASSERT_EQUAL(Size{1}, Safe::WString::Explode(caller_queue).size());
	ASSERT_EQUAL(Size{2}, Safe::WString(L"a|b").Explode(L'|').size());
	ASSERT_TRUE(text == Safe::WString(L"abXab"));
	ASSERT_TRUE(text != Safe::WString(L"ab"));
	ASSERT_TRUE(text == L"abXab");
	ASSERT_TRUE(L"abXab" == text);
	ASSERT_TRUE(text != L"no");
	ASSERT_TRUE(L"no" != text);
	ASSERT_TRUE(Safe::WString(L"a") < Safe::WString(L"b"));
	ASSERT_TRUE(Safe::WString(L"ab") <= Safe::WString(L"ab"));
	ASSERT_TRUE((text <=> Safe::WString(L"abXab")) == std::strong_ordering::equal);
	ASSERT_TRUE((text <=> L"abXab") == std::strong_ordering::equal);
	ASSERT_EQUAL(std::hash<std::wstring_view>{}(L"abXab"), std::hash<Safe::WString>{}(text));
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
	result += test_construct_copy_move_and_narrow();

	// -------------------
	// Mutate
	// -------------------
	result += test_mutate_edit_reserve_and_concat();

	// -------------------
	// Search
	// -------------------
	result += test_search_helpers_and_order();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
