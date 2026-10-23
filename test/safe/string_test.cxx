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

#include <StormByte/byte_size.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
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
	std::for_each(text.begin(), text.end(), [](char& ch) { ch = static_cast<char>(ch - 'a' + 'A'); });
	ASSERT_TRUE(std::string_view(text) == "AABBC");
	std::for_each_n(text.begin(), 2, [](char& ch) { ch = static_cast<char>(ch - 'A' + 'a'); });
	ASSERT_TRUE(std::string_view(text) == "aaBBC");
	std::ranges::for_each(text, [](char& ch) {
		if (ch >= 'A' && ch <= 'Z')
			ch = static_cast<char>(ch - 'A' + 'a');
	});
	ASSERT_TRUE(std::string_view(text) == "aabbc");
	ASSERT_EQUAL(owned, text.size());
	ASSERT_EQUAL('\0', text.c_str()[static_cast<std::size_t>(owned)]);
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
	ASSERT_EQUAL(Size{1}, Safe::Queue<Safe::String>(caller_queue).size());
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
	ASSERT_EQUAL(Safe::Hash<Safe::String>{}(text), std::hash<Safe::String>{}(text));
	RETURN_TEST(0);
}

// -------------------
// Format
// -------------------

int test_format_as_text() {
	const Safe::String text("hola mundo");
	ASSERT_EQUAL(std::string("Texto: hola mundo"), std::format("Texto: {}", text));
	ASSERT_EQUAL(std::string("hola mundo"), std::format("{:s}", text));
	ASSERT_EQUAL(std::string("hola"), std::format("{:.4}", text));
	ASSERT_EQUAL(std::string("hola mundo__"), std::format("{:_<12}", text));
	ASSERT_EQUAL(std::string("__hola mundo"), std::format("{:_>12}", text));
	ASSERT_EQUAL(std::string("_hola mundo_"), std::format("{:_^12s}", text));
#if defined(CLANG) && defined(LIBSTDCXX) && _GLIBCXX_RELEASE == 15 && __cplusplus > 202302L
	// GCC PR127220: Clang cannot constant-evaluate libstdc++ 15's dynamic spec checks.
	int width = 6;
	int precision = 4;
	ASSERT_EQUAL(std::string("__hola"), std::vformat("{:_>{}.{}s}", std::make_format_args(text, width, precision)));
#else
	ASSERT_EQUAL(std::string("__hola"), std::format("{:_>{}.{}s}", text, 6, 4));
#endif
	ASSERT_EQUAL(std::string(), std::format("{}", Safe::String{}));
	const Safe::String long_text("This text is longer than the short string buffer.");
	ASSERT_EQUAL(std::string(std::string_view(long_text)), std::format("{}", long_text));
	const Safe::String embedded(std::string_view("a\0b", 3));
	ASSERT_EQUAL(std::string("a\0b", 3), std::format("{}", embedded));
	std::string output;
	std::format_to(std::back_inserter(output), "Texto: {:.4s}", text);
	ASSERT_EQUAL(std::string("Texto: hola"), output);
#if __has_include(<fmt/format.h>)
	ASSERT_EQUAL(std::string("Texto: hola mundo"), fmt::format("Texto: {}", text));
	ASSERT_EQUAL(std::string("__hola"), fmt::format("{:_>{}.{}s}", text, 6, 4));
	ASSERT_EQUAL(std::string("a\0b", 3), fmt::format("{}", embedded));
	output.clear();
	fmt::format_to(std::back_inserter(output), "Texto: {:.4s}", text);
	ASSERT_EQUAL(std::string("Texto: hola"), output);
#endif
	RETURN_TEST(0);
}


int test_replace_token_sso_and_heap() {
	const Safe::String original("Hello {}");
	const Safe::String copy = Safe::String::ReplaceToken(original, "Ann");
	ASSERT_EQUAL("Hello {}", original);
	ASSERT_EQUAL("Hello Ann", copy);
	ASSERT_SIZE(copy, Size{9});
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, copy.capacity());

	const Safe::String many = Safe::String::ReplaceToken("id={} n={}", "7", "Ada");
	ASSERT_EQUAL("id=7 n=Ada", many);
	ASSERT_SIZE(many, Size{10});
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, many.capacity());

	const Safe::String extra = Safe::String::ReplaceToken("a {} b {}", "X", "Y", "Z");
	ASSERT_EQUAL("a X b Y", extra);

	const Safe::String once = Safe::String::ReplaceToken("{} and {}", "only");
	ASSERT_EQUAL("only and {}", once);

	const Safe::String none = Safe::String::ReplaceToken("plain", "nope");
	ASSERT_EQUAL("plain", none);
	const Safe::String none_copy = Safe::String::ReplaceToken("plain", "a", "b");
	ASSERT_EQUAL("plain", none_copy);

	const Safe::String empty_tok = Safe::String::ReplaceToken("x{}y", "");
	ASSERT_EQUAL("xy", empty_tok);
	ASSERT_SIZE(empty_tok, Size{2});

	const Safe::String self("Hello {}");
	const Safe::String self_replaced = Safe::String::ReplaceToken(self, std::string_view(self).substr(0, 5));
	ASSERT_EQUAL("Hello {}", self);
	ASSERT_EQUAL("Hello Hello", self_replaced);

	const std::string big(80, 'Q');
	const Safe::String long_text(std::string("{}-") + big);
	const Safe::String grown = Safe::String::ReplaceToken(long_text, "HEAD");
	ASSERT_EQUAL(Safe::String(std::string("HEAD-") + big), grown);
	ASSERT_SIZE(grown, Size{85});
	ASSERT_TRUE(grown.capacity() > Size{Safe::String::SSO_CAPACITY});

	const Safe::String holes(std::string(30, 'A') + "{}" + "BB");
	ASSERT_TRUE(holes.capacity() > Size{Safe::String::SSO_CAPACITY});
	ASSERT_SIZE(holes, Size{34});
	const Safe::String shrunk = Safe::String::ReplaceToken(holes, "");
	ASSERT_EQUAL(Safe::String(std::string(30, 'A') + "BB"), shrunk);
	ASSERT_SIZE(shrunk, Size{32});
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, shrunk.capacity());

	const Safe::String owned("Ada");
	const std::string stl("Bob");
	const std::string_view view("Cyd");
	const int id = 7;
	const Safe::String typed = Safe::String::ReplaceToken("{} / {} / {} / {}", owned, stl, view, std::to_string(id));
	ASSERT_EQUAL("Ada / Bob / Cyd / 7", typed);
	ASSERT_SIZE(typed, Size{19});

	const Safe::String mixed = Safe::String::ReplaceToken("n={} s={}", std::to_string(3), Safe::String("ok"));
	ASSERT_EQUAL("n=3 s=ok", mixed);
	ASSERT_SIZE(mixed, Size{8});

	const Safe::String from_static = Safe::String::ReplaceToken(std::string_view("Hello {}"), "Ann");
	ASSERT_EQUAL("Hello Ann", from_static);
	ASSERT_SIZE(from_static, Size{9});
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, from_static.capacity());

	const Safe::String from_many = Safe::String::ReplaceToken("a {} b {}", Safe::String("X"), std::string("Y"));
	ASSERT_EQUAL("a X b Y", from_many);
	ASSERT_SIZE(from_many, Size{7});

	const std::string long_pattern(40, 'Z');
	const Safe::String from_long = Safe::String::ReplaceToken(std::string_view(long_pattern + " {}"), "ok");
	ASSERT_EQUAL(Safe::String(long_pattern + " ok"), from_long);
	ASSERT_SIZE(from_long, Size{43});
	ASSERT_TRUE(from_long.capacity() > Size{Safe::String::SSO_CAPACITY});

	const std::string shrink_pattern = std::string(30, 'A') + "{}" + "BB";
	const Safe::String from_shrink = Safe::String::ReplaceToken(std::string_view(shrink_pattern), "");
	ASSERT_EQUAL(Safe::String(std::string(30, 'A') + "BB"), from_shrink);
	ASSERT_SIZE(from_shrink, Size{32});
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, from_shrink.capacity());

	ASSERT_EQUAL("plain", Safe::String::ReplaceToken("plain", "z"));
	ASSERT_EQUAL("n=3", Safe::String::ReplaceToken("n={}", 3));
	ASSERT_EQUAL("id=A", Safe::String::ReplaceToken("id={}", 'A'));
	const char* word = "Ann";
	ASSERT_EQUAL("Hello Ann", Safe::String::ReplaceToken("Hello {}", word));
	const char* missing = nullptr;
	ASSERT_EQUAL("Hello ", Safe::String::ReplaceToken("Hello {}", missing));
	ASSERT_EQUAL("n=12", Safe::String::ReplaceToken("n={}", Size{12}));
	struct Tickets {
		explicit operator std::string() const { return "4"; }
	};
	ASSERT_EQUAL("n=4", Safe::String::ReplaceToken("n={}", Tickets{}));
	RETURN_TEST(0);
}


int test_replace_token_specs() {
	const double pi = 3.141592653589793;
	ASSERT_EQUAL(std::format("{}", true), std::string(Safe::String::ReplaceToken("{}", true)));
	ASSERT_EQUAL(std::format("{}", false), std::string(Safe::String::ReplaceToken("{}", false)));
	ASSERT_EQUAL("true", Safe::String::ReplaceToken("{:02}", true));
	ASSERT_EQUAL(std::format("{:.3f}", pi), std::string(Safe::String::ReplaceToken("{:.3f}", pi)));
	ASSERT_EQUAL(std::format("{:.6f}", 1.23), std::string(Safe::String::ReplaceToken("{:.6f}", 1.23)));
	ASSERT_EQUAL(std::format("{:.17g}", pi), std::string(Safe::String::ReplaceToken("{:.17g}", pi)));
	ASSERT_EQUAL(std::format("{:6.2f}", 3.14159), std::string(Safe::String::ReplaceToken("{:6.2f}", 3.14159)));
	ASSERT_EQUAL(std::format("{:6.2f}", 100.0), std::string(Safe::String::ReplaceToken("{:6.2f}", 100.0)));
	ASSERT_EQUAL(std::format("{:02}", 3), std::string(Safe::String::ReplaceToken("{:02}", 3)));
	ASSERT_EQUAL(std::format("{:02}", 12), std::string(Safe::String::ReplaceToken("{:02}", 12)));
	ASSERT_EQUAL(std::format("{:02}", -3), std::string(Safe::String::ReplaceToken("{:02}", -3)));
	ASSERT_EQUAL(std::format("{:03}", 7), std::string(Safe::String::ReplaceToken("{:03}", 7)));
	ASSERT_EQUAL(std::format("{:03}", -3), std::string(Safe::String::ReplaceToken("{:03}", -3)));
	ASSERT_EQUAL(std::format("{:016x}", 0x2au), std::string(Safe::String::ReplaceToken("{:016x}", 0x2au)));
	ASSERT_EQUAL(std::format("{}:{:02}:{:02}.{:03}", 1, 2, 3, 4), std::string(Safe::String::ReplaceToken("{}:{:02}:{:02}.{:03}", 1, 2, 3, 4)));
	ASSERT_EQUAL("keep {:.2s}", Safe::String::ReplaceToken("keep {:.2s}", 1));
	ASSERT_EQUAL("n=12", Safe::String::ReplaceToken("n={}", Size{12}));
	ASSERT_EQUAL("n=12 B", Safe::String::ReplaceToken("n={}", ByteSize{12}));
	ASSERT_EQUAL("n=1.50 KiB", Safe::String::ReplaceToken("n={}", ByteSize{1536}));
	ASSERT_EQUAL(std::format("{:016x}", std::uint64_t{0x2a}), std::string(Safe::String::ReplaceToken("{:016x}", Size{0x2a})));
	ASSERT_EQUAL(std::format("{:02}", 3), std::string(Safe::String::ReplaceToken("{:02}", ByteSize{3})));
	ASSERT_EQUAL(std::format("{:.0f}", 12.0), std::string(Safe::String::ReplaceToken("{:.0f}", Size{12})));

	int object = 0;
	int* raw = &object;
	auto address = [](const void* pointer) {
		auto value = reinterpret_cast<std::uintptr_t>(pointer);
		std::string text = "0x";
		if (value == 0)
			return std::string("0x0");
		char digits[2 * sizeof(value)];
		int count = 0;
		while (value != 0) {
			digits[count++] = "0123456789abcdef"[value & 0xF];
			value >>= 4;
		}
		while (count != 0)
			text.push_back(digits[--count]);
		return text;
	};
	ASSERT_EQUAL(std::string("p=") + address(raw), std::string(Safe::String::ReplaceToken("p={}", raw)));
	ASSERT_EQUAL("0x0", Safe::String::ReplaceToken("{}", static_cast<int*>(nullptr)));

	const Safe::Unique<int> empty;
	ASSERT_EQUAL("0x0", Safe::String::ReplaceToken("{}", empty));
	const Safe::Unique<int> owned = Safe::Unique<int>::MakePointer<int>(7);
	ASSERT_EQUAL(address(owned.get()), std::string(Safe::String::ReplaceToken("{}", owned)));
	const Safe::Shared<int> shared = Safe::Shared<int>::MakePointer<int>(8);
	ASSERT_EQUAL(address(shared.get()), std::string(Safe::String::ReplaceToken("{}", shared)));
	const Safe::Shared<int> shared_empty;
	ASSERT_EQUAL("0x0", Safe::String::ReplaceToken("{}", shared_empty));
	RETURN_TEST(0);
}


int test_implode_vector_and_queue() {
	const Safe::Vector<Safe::String> none;
	const Safe::Queue<Safe::String> none_q;
	ASSERT_TRUE(Safe::String::Implode(none, ",").empty());
	ASSERT_TRUE(Safe::String::Implode(none_q, ",").empty());

	const Safe::Vector<Safe::String> one{Safe::String("only")};
	const Safe::String single = Safe::String::Implode(one, ",");
	ASSERT_EQUAL(std::string_view("only"), std::string_view(single));
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, single.capacity());

	const Safe::Vector<Safe::String> words{Safe::String("a"), Safe::String(""), Safe::String("b")};
	const Safe::String joined = Safe::String::Implode(words, ",");
	ASSERT_EQUAL(std::string_view("a,,b"), std::string_view(joined));
	ASSERT_EQUAL(Size{Safe::String::SSO_CAPACITY}, joined.capacity());
	ASSERT_EQUAL(std::string_view("a,,b"), std::string_view(Safe::String::Implode(Safe::String::Explode("a,,b", ','), ",")));

	Safe::Queue<Safe::String> fifo;
	fifo.push(Safe::String("x"));
	fifo.push(Safe::String("y"));
	const Safe::Queue<Safe::String>& fifo_ref = fifo;
	ASSERT_EQUAL(std::string_view("x | y"), std::string_view(Safe::String::Implode(fifo_ref, " | ")));
	ASSERT_EQUAL(std::string_view("xy"), std::string_view(Safe::String::Implode(fifo_ref, "")));

	Safe::Vector<Safe::String> long_parts;
	long_parts.push_back(Safe::String("0123456789abcdef"));
	long_parts.push_back(Safe::String("0123456789abcdef"));
	long_parts.push_back(Safe::String("0123456789abcdef"));
	const Safe::String heap = Safe::String::Implode(long_parts, ",");
	ASSERT_EQUAL(std::string_view("0123456789abcdef,0123456789abcdef,0123456789abcdef"), std::string_view(heap));
	ASSERT_TRUE(heap.size() > Size{Safe::String::SSO_CAPACITY});
	ASSERT_TRUE(heap.capacity() > Size{Safe::String::SSO_CAPACITY});
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

	// -------------------
	// Format
	// -------------------
	result += test_format_as_text();

	result += test_replace_token_sso_and_heap();
	result += test_replace_token_specs();
	result += test_implode_vector_and_queue();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
