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
#include <functional>
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
	Safe::String text("cbaab");
	const Size owned = text.size();
	std::sort(text.begin(), text.end());
	std::stable_sort(text.begin(), text.end());
	std::partial_sort(text.begin(), text.begin() + 3, text.end());
	std::nth_element(text.begin(), text.begin() + 2, text.end());
	std::sort(text.begin(), text.end());
	std::ranges::sort(text);
	ASSERT_EQUAL(owned, text.size());
	ASSERT_TRUE(std::is_sorted(text.begin(), text.end()));
	ASSERT_TRUE(std::ranges::is_sorted(text));
	auto minmax = std::minmax_element(text.begin(), text.end());
	ASSERT_EQUAL('a', *minmax.first);
	ASSERT_EQUAL('c', *minmax.second);
	ASSERT_EQUAL('a', *std::ranges::min_element(text));
	ASSERT_EQUAL('c', *std::ranges::max_element(text));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count(text.begin(), text.end(), 'a'));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count_if(text.begin(), text.end(), [](char ch) { return ch == 'b'; }));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::ranges::count(text, 'a'));
	ASSERT_TRUE(std::all_of(text.begin(), text.end(), [](char ch) { return ch >= 'a' && ch <= 'c'; }));
	ASSERT_TRUE(std::any_of(text.begin(), text.end(), [](char ch) { return ch == 'b'; }));
	ASSERT_TRUE(std::none_of(text.begin(), text.end(), [](char ch) { return ch == 'z'; }));
	ASSERT_TRUE(std::ranges::all_of(text, [](char ch) { return ch != 'z'; }));
	ASSERT_TRUE(std::adjacent_find(text.begin(), text.end()) != text.end());
	ASSERT_TRUE(std::binary_search(text.begin(), text.end(), 'c'));
	ASSERT_FALSE(std::binary_search(text.begin(), text.end(), 'z'));
	ASSERT_TRUE(std::ranges::binary_search(text, 'b'));
	auto lower = std::lower_bound(text.begin(), text.end(), 'b');
	auto upper = std::upper_bound(text.begin(), text.end(), 'b');
	auto range = std::equal_range(text.begin(), text.end(), 'b');
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), upper - lower);
	ASSERT_TRUE(range.first == lower);
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::find(text.begin(), text.end(), 'b') - text.begin());
	ASSERT_TRUE(std::find_if(text.begin(), text.end(), [](char ch) { return ch == 'c'; }) != text.end());
	ASSERT_TRUE(std::find_if_not(text.begin(), text.end(), [](char ch) { return ch == 'a'; }) != text.begin());
	ASSERT_TRUE(std::ranges::find(text, 'c') != text.end());
	ASSERT_TRUE(std::ranges::find_if(text, [](char ch) { return ch == 'b'; }) != text.end());
	const char needle[] = {'a', 'a'};
	ASSERT_TRUE(std::search(text.begin(), text.end(), needle, needle + 2) == text.begin());
	ASSERT_TRUE(std::find_end(text.begin(), text.end(), needle, needle + 2) == text.begin());
	ASSERT_TRUE(std::search_n(text.begin(), text.end(), 2, 'a') == text.begin());
	ASSERT_TRUE(std::find_first_of(text.begin(), text.end(), needle, needle + 1) == text.begin());
	std::for_each(text.begin(), text.end(), [](char& ch) { ch = ch; });
	std::for_each_n(text.begin(), 2, [](char& ch) { ch = ch; });
	std::ranges::for_each(text, [](char& ch) { ch = ch; });
	std::reverse(text.begin(), text.end());
	std::ranges::reverse(text);
	std::rotate(text.begin(), text.begin() + 1, text.end());
	std::ranges::rotate(text, text.begin() + 1);
	std::replace(text.begin(), text.end(), 'a', 'x');
	std::replace_if(text.begin(), text.end(), [](char ch) { return ch == 'x'; }, 'a');
	std::ranges::replace(text, 'a', 'x');
	std::ranges::replace(text, 'x', 'a');
	std::fill(text.begin(), text.end(), 'q');
	std::fill_n(text.begin(), 2, 'r');
	std::ranges::fill(text, 'q');
	std::generate(text.begin(), text.end(), [n = 0]() mutable { return static_cast<char>('a' + (n++ % 3)); });
	std::generate_n(text.begin(), 1, [] { return 'a'; });
	std::transform(text.begin(), text.end(), text.begin(), [](char ch) { return ch; });
	std::ranges::transform(text, text.begin(), [](char ch) { return ch; });
	ASSERT_EQUAL(owned, text.size());
	Safe::String other(text);
	ASSERT_TRUE(std::equal(text.begin(), text.end(), other.begin()));
	ASSERT_TRUE(std::ranges::equal(text, other));
	ASSERT_TRUE(std::mismatch(text.begin(), text.end(), other.begin()).first == text.end());
	ASSERT_FALSE(std::lexicographical_compare(text.begin(), text.end(), other.begin(), other.end()));
	std::vector<char> copied(static_cast<std::size_t>(owned));
	std::copy(text.begin(), text.end(), copied.begin());
	std::copy_n(text.begin(), 2, copied.begin());
	std::copy_backward(text.begin(), text.end(), copied.end());
	std::copy_if(text.begin(), text.end(), copied.begin(), [](char) { return true; });
	std::reverse_copy(text.begin(), text.end(), copied.begin());
	std::replace_copy(text.begin(), text.end(), copied.begin(), 'a', 'z');
	std::replace_copy_if(text.begin(), text.end(), copied.begin(), [](char ch) { return ch == 'a'; }, 'z');
	std::ranges::copy(text, copied.begin());
	std::iter_swap(text.begin(), text.end() - 1);
	std::swap_ranges(text.begin(), text.begin() + 2, other.begin());
	std::partition(text.begin(), text.end(), [](char ch) { return ch < 'n'; });
	std::sort(text.begin(), text.end());
	ASSERT_TRUE(std::unique(text.begin(), text.end()) <= text.end());
	std::reverse(text.rbegin(), text.rend());
	ASSERT_EQUAL(owned, text.size());
	ASSERT_EQUAL('\0', text.c_str()[static_cast<std::size_t>(text.size())]);
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_copy_move_sso_and_long() {
	Safe::String empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_FALSE(static_cast<bool>(empty));
	ASSERT_EQUAL(Size{0}, empty.size());
	ASSERT_EQUAL(Size{0}, empty.length());
	ASSERT_NOT_NULL(empty.data());
	ASSERT_NOT_NULL(empty.c_str());
	ASSERT_NOT_NULL(empty.Bytes());
	ASSERT_EQUAL('\0', empty.c_str()[0]);
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.cbegin() == empty.cend());
	ASSERT_TRUE(empty.rbegin() == empty.rend());
	ASSERT_TRUE(empty.crbegin() == empty.crend());
	Safe::String null_pointer(static_cast<const char*>(nullptr));
	ASSERT_TRUE(null_pointer.empty());
	ASSERT_TRUE(null_pointer == static_cast<const char*>(nullptr));
	Safe::String literal("ab");
	ASSERT_TRUE(static_cast<bool>(literal));
	ASSERT_EQUAL(Size{2}, literal.size());
	ASSERT_EQUAL('a', literal[Size{0}]);
	ASSERT_EQUAL('b', literal.at(Size{1}));
	literal[Size{0}] = 'a';
	ASSERT_EQUAL('a', literal.front());
	ASSERT_EQUAL('b', literal.back());
	ASSERT_EQUAL('\0', literal.c_str()[2]);
	ASSERT_EQUAL('b', *literal.rbegin());
	ASSERT_EQUAL('b', *literal.crbegin());
	const char embedded[] = {'a', '\0', 'b'};
	Safe::String kept(std::string_view(embedded, 3));
	ASSERT_EQUAL(Size{3}, kept.size());
	ASSERT_EQUAL('\0', kept[Size{1}]);
	Safe::String cut(embedded);
	ASSERT_EQUAL(Size{1}, cut.size());
	Safe::String repeated(Size{4}, 'z');
	ASSERT_EQUAL('z', repeated[Size{3}]);
	Safe::String from_iter(literal.begin(), literal.end());
	ASSERT_EQUAL(std::string_view("ab"), std::string_view(from_iter));
	Safe::String slice(literal, Size{1}, Size{1});
	Safe::String tail(literal, Size{1});
	ASSERT_EQUAL('b', slice[Size{0}]);
	ASSERT_EQUAL(Size{1}, tail.size());
	ASSERT_THROWS(Safe::String(literal, Size{3}), Safe::OutOfBoundsError);
	Safe::String copy(literal);
	copy = literal;
	Safe::String moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	moved = std::move(literal);
	ASSERT_TRUE(literal.empty());
	moved = std::string_view("q");
	const std::string exported(moved);
	const std::string_view borrowed = moved;
	const char* raw = static_cast<const char*>(moved);
	ASSERT_EQUAL(std::string("q"), exported);
	ASSERT_EQUAL(std::size_t{1}, borrowed.size());
	ASSERT_EQUAL('q', raw[0]);
	Safe::WString wide(L"ok");
	Safe::String from_wide(wide);
	Safe::WString back(static_cast<Safe::WString>(from_wide));
	ASSERT_EQUAL(std::string_view("ok"), std::string_view(from_wide));
	ASSERT_EQUAL(Size{2}, back.size());
	Safe::String long_text(Size{Safe::String::SSO_CAPACITY + 4}, 'L');
	ASSERT_TRUE(long_text.capacity() >= long_text.size());
	ASSERT_TRUE(long_text.max_size() >= long_text.size());
	Safe::String long_moved(std::move(long_text));
	ASSERT_TRUE(long_text.empty());
	ASSERT_EQUAL('L', long_moved.back());
	RETURN_TEST(0);
}

// -------------------
// Mutate
// -------------------

int test_mutate_edit_reserve_and_concat() {
	Safe::String text;
	text.append("ab");
	text.append(Size{2}, 'c');
	text.append(std::string_view("d").begin(), std::string_view("d").end());
	text.append_range(std::string_view("e"));
	text += 'f';
	text += std::string_view("g");
	text += Safe::String("h");
	text.push_back('i');
	ASSERT_EQUAL(std::string_view("abccdefghi"), std::string_view(text));
	text.pop_back();
	text.insert(Size{2}, "XY");
	text.insert(Size{0}, Size{1}, 'Q');
	auto inserted = text.insert(text.begin() + 1, std::string_view("Z").begin(), std::string_view("Z").end());
	ASSERT_EQUAL('Z', *inserted);
	text.insert(text.begin(), {'P'});
	text.insert_range(text.end(), std::string_view("!"));
	text.erase(Size{0}, Size{1});
	text.erase(text.begin());
	text.erase(text.begin(), text.begin() + 1);
	text.replace(Size{0}, Size{1}, "MM");
	text.replace(text.begin(), text.begin() + 1, "N");
	text.resize(Size{4}, 'q');
	text.resize(Size{6});
	ASSERT_EQUAL('\0', text[Size{4}]);
	text.reserve(Size{80});
	ASSERT_TRUE(text.capacity() >= Size{80});
	text.reserve(Size{1});
	ASSERT_TRUE(text.capacity() >= text.size());
	text.shrink_to_fit();
	text.clear();
	ASSERT_TRUE(text.empty());
	text.assign(Size{3}, 'k');
	text.assign(std::string_view("no"));
	const char raw[] = {'o', 'k'};
	text.assign(raw, raw + 2);
	text.assign_range(std::string_view("ok"));
	Safe::String sum = text + Safe::String("x");
	Safe::String sum_view = text + std::string_view("y");
	Safe::String sum_left = std::string_view("a") + text;
	ASSERT_EQUAL(std::string_view("okx"), std::string_view(sum));
	ASSERT_EQUAL(std::string_view("oky"), std::string_view(sum_view));
	ASSERT_EQUAL(std::string_view("aok"), std::string_view(sum_left));
	Safe::String swapped("left");
	text.swap(swapped);
	swap(text, swapped);
	ASSERT_EQUAL(std::string_view("ok"), std::string_view(text));
	char buffer[8] = {};
	ASSERT_EQUAL(Size{1}, text.copy(buffer, Size{1}, Size{1}));
	ASSERT_EQUAL('k', buffer[0]);
	const Safe::String& read = text;
	ASSERT_EQUAL('k', read.at(Size{1}));
	ASSERT_EQUAL('k', read[Size{1}]);
	ASSERT_EQUAL('o', read.front());
	ASSERT_EQUAL('k', read.back());
	ASSERT_THROWS(text.at(text.size()), Safe::OutOfBoundsError);
	ASSERT_THROWS(text.insert(text.size() + Size{1}, "z"), Safe::OutOfBoundsError);
	ASSERT_THROWS(text.copy(buffer, Size{1}, text.size() + Size{1}), Safe::OutOfBoundsError);
	std::ostringstream stream;
	stream << text;
	ASSERT_EQUAL(std::string("ok"), stream.str());
	RETURN_TEST(0);
}

// -------------------
// Search
// -------------------

int test_search_helpers_and_order() {
	Safe::String text("abXab");
	ASSERT_TRUE(text.starts_with("ab"));
	ASSERT_TRUE(text.starts_with('a'));
	ASSERT_TRUE(text.starts_with(""));
	ASSERT_FALSE(text.starts_with("X"));
	ASSERT_TRUE(text.ends_with("ab"));
	ASSERT_TRUE(text.ends_with('b'));
	ASSERT_TRUE(text.ends_with(""));
	ASSERT_TRUE(text.contains("bX"));
	ASSERT_TRUE(text.contains('X'));
	ASSERT_TRUE(text.contains(""));
	ASSERT_FALSE(text.contains('z'));
	ASSERT_EQUAL(Size{0}, text.find("ab"));
	ASSERT_EQUAL(Size{3}, text.find("ab", Size{1}));
	ASSERT_EQUAL(Size{2}, text.find('X'));
	ASSERT_EQUAL(Size{0}, text.find("ab", Size{0}, Size{2}));
	ASSERT_EQUAL(Safe::String::npos, text.find("no"));
	ASSERT_EQUAL(Size{3}, text.rfind("ab"));
	ASSERT_EQUAL(Size{4}, text.rfind('b'));
	ASSERT_EQUAL(Size{3}, text.rfind("ab", Safe::String::npos, Size{2}));
	ASSERT_EQUAL(Size{2}, text.find_first_of("XZ"));
	ASSERT_EQUAL(Size{0}, text.find_first_of('a'));
	ASSERT_EQUAL(Size{4}, text.find_last_of("ab"));
	ASSERT_EQUAL(Size{4}, text.find_last_of('b'));
	ASSERT_EQUAL(Size{2}, text.find_first_not_of("ab"));
	ASSERT_EQUAL(Size{1}, text.find_first_not_of('a'));
	ASSERT_EQUAL(Size{2}, text.find_last_not_of("ab"));
	ASSERT_EQUAL(Size{3}, text.find_last_not_of('b'));
	ASSERT_EQUAL(std::string_view("Xab"), std::string_view(text.substr(Size{2})));
	ASSERT_EQUAL(std::string_view("bX"), std::string_view(text.substr(Size{1}, Size{2})));
	ASSERT_TRUE(text.substr(Size{9}).empty());
	ASSERT_EQUAL(0, text.compare("abXab"));
	ASSERT_TRUE(text.compare(Size{2}, Size{1}, "X") == 0);
	ASSERT_EQUAL(std::string_view("abxab"), std::string_view(text.ToLower()));
	ASSERT_EQUAL(std::string_view("ABXAB"), std::string_view(Safe::String::ToUpper("abXab")));
	ASSERT_EQUAL(std::string_view("abxab"), std::string_view(Safe::String::ToLower("abXab")));
	ASSERT_EQUAL(std::string_view("a\nb"), std::string_view(Safe::String::SanitizeNewlines("a\r\nb")));
	ASSERT_EQUAL(std::string_view("a\nb"), std::string_view(Safe::String("a\r\nb").SanitizeNewlines()));
	ASSERT_EQUAL(std::string_view("ab"), std::string_view(Safe::String::RemoveWhitespace(" a\tb ")));
	ASSERT_EQUAL(std::string_view("ab"), std::string_view(Safe::String(" a b").RemoveWhitespace()));
	ASSERT_TRUE(Safe::String::IsInteger("-12"));
	ASSERT_TRUE(Safe::String("42").IsInteger());
	ASSERT_FALSE(Safe::String("4a").IsInteger());
	Safe::Vector<Safe::String> words = Safe::String::Split("a b");
	ASSERT_EQUAL(Size{2}, words.size());
	ASSERT_EQUAL(std::string_view("b"), std::string_view(words[Size{1}]));
	std::vector<Safe::String> caller{Safe::String("p"), Safe::String("q")};
	ASSERT_EQUAL(Size{2}, Safe::String::Split(caller).size());
	Safe::Vector<Safe::String> member = Safe::String("a b c").Split();
	ASSERT_EQUAL(Size{3}, member.size());
	Safe::Queue<Safe::String> parts = Safe::String::Explode("a,,b", ',');
	ASSERT_EQUAL(Size{3}, parts.size());
	std::queue<Safe::String> caller_queue;
	caller_queue.push(Safe::String("p"));
	ASSERT_EQUAL(Size{1}, Safe::String::Explode(caller_queue).size());
	ASSERT_EQUAL(Size{2}, Safe::String("a|b").Explode('|').size());
	ASSERT_TRUE(text == Safe::String("abXab"));
	ASSERT_TRUE(text != Safe::String("ab"));
	ASSERT_TRUE(text == "abXab");
	ASSERT_TRUE("abXab" == text);
	ASSERT_TRUE(text != "no");
	ASSERT_TRUE("no" != text);
	ASSERT_TRUE(Safe::String("a") < Safe::String("b"));
	ASSERT_TRUE(Safe::String("b") > Safe::String("a"));
	ASSERT_TRUE(Safe::String("ab") <= Safe::String("ab"));
	ASSERT_TRUE(Safe::String("ab") >= "a");
	ASSERT_TRUE((text <=> Safe::String("abXab")) == std::strong_ordering::equal);
	ASSERT_TRUE((text <=> "abXab") == std::strong_ordering::equal);
	ASSERT_TRUE((Safe::String("a") <=> "b") == std::strong_ordering::less);
	ASSERT_EQUAL(std::hash<std::string_view>{}("abXab"), std::hash<Safe::String>{}(text));
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
	result += test_construct_copy_move_sso_and_long();

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
