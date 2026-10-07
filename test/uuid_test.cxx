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

#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/uuid.hxx>

#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace StormByte;

namespace {
	bool IsHex(char c) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
	}

	bool IsVariant(char c) {
		return c == '8' || c == '9' || c == 'a' || c == 'b';
	}

	bool IsRfc4122(std::string_view text) {
		if (text.size() != 36)
			return false;
		if (text[8] != '-' || text[13] != '-' || text[18] != '-' || text[23] != '-')
			return false;
		if (text[14] != '4' || !IsVariant(text[19]))
			return false;
		for (std::size_t i = 0; i < text.size(); ++i) {
			if (i == 8 || i == 13 || i == 18 || i == 23)
				continue;
			if (!IsHex(text[i]))
				return false;
		}
		return true;
	}
}

// -------------------
// Format
// -------------------

int test_format_hex_lowercase() {
	const auto uuid = GenerateUUIDv4();
	for (Size i{0}; i < uuid.size(); i = Size{static_cast<std::size_t>(i) + 1}) {
		const auto index = static_cast<std::size_t>(i);
		if (index == 8 || index == 13 || index == 18 || index == 23)
			continue;
		ASSERT_TRUE(IsHex(uuid[i]));
		ASSERT_TRUE(uuid[i] < 'A' || uuid[i] > 'Z');
	}
	RETURN_TEST(0);
}

int test_format_hyphens() {
	const auto uuid = GenerateUUIDv4();
	ASSERT_EQUAL('-', uuid[Size{8}]);
	ASSERT_EQUAL('-', uuid[Size{13}]);
	ASSERT_EQUAL('-', uuid[Size{18}]);
	ASSERT_EQUAL('-', uuid[Size{23}]);
	RETURN_TEST(0);
}

int test_format_variant_rfc4122() {
	const auto uuid = GenerateUUIDv4();
	ASSERT_TRUE(IsVariant(uuid[Size{19}]));
	RETURN_TEST(0);
}

int test_format_version_is_4() {
	const auto uuid = GenerateUUIDv4();
	ASSERT_EQUAL('4', uuid[Size{14}]);
	RETURN_TEST(0);
}

// -------------------
// Generate
// -------------------

int test_generate_explicit_std_string() {
	const std::string text = static_cast<std::string>(GenerateUUIDv4());
	ASSERT_EQUAL(std::size_t{36}, text.size());
	ASSERT_TRUE(IsRfc4122(text));
	RETURN_TEST(0);
}

int test_generate_implicit_string_view() {
	const auto uuid = GenerateUUIDv4();
	const std::string_view text = uuid;
	ASSERT_EQUAL(std::size_t{36}, text.size());
	ASSERT_TRUE(IsRfc4122(text));
	RETURN_TEST(0);
}

int test_generate_length_and_not_empty() {
	const auto uuid = GenerateUUIDv4();
	ASSERT_NOT_EMPTY(uuid);
	ASSERT_EQUAL(Size{36}, uuid.size());
	ASSERT_NOT_NULL(uuid.c_str());
	RETURN_TEST(0);
}

int test_generate_noexcept() {
	static_assert(noexcept(GenerateUUIDv4()));
	ASSERT_NO_THROW((void)GenerateUUIDv4());
	RETURN_TEST(0);
}

int test_generate_samples_match_contract() {
	for (int i = 0; i < 64; ++i)
		ASSERT_TRUE(IsRfc4122(GenerateUUIDv4()));
	RETURN_TEST(0);
}

// -------------------
// Uniqueness
// -------------------

int test_generate_distinct() {
	std::set<std::string> seen;
	for (int i = 0; i < 1000; ++i) {
		const std::string text = static_cast<std::string>(GenerateUUIDv4());
		ASSERT_FALSE(seen.contains(text));
		seen.insert(text);
	}
	ASSERT_EQUAL(std::size_t{1000}, seen.size());
	RETURN_TEST(0);
}

int test_generate_distinct_across_threads() {
	constexpr int thread_count = 4;
	constexpr int per_thread = 100;
	std::vector<std::vector<std::string>> bags(thread_count);
	std::vector<std::thread> threads;
	for (int t = 0; t < thread_count; ++t) {
		threads.emplace_back([t, &bags]() {
			bags[t].reserve(per_thread);
			for (int i = 0; i < per_thread; ++i)
				bags[t].push_back(static_cast<std::string>(GenerateUUIDv4()));
		});
	}
	for (auto& thread : threads)
		thread.join();
	std::set<std::string> seen;
	for (const auto& bag : bags) {
		for (const auto& text : bag) {
			ASSERT_TRUE(IsRfc4122(text));
			ASSERT_FALSE(seen.contains(text));
			seen.insert(text);
		}
	}
	ASSERT_EQUAL(std::size_t{thread_count * per_thread}, seen.size());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Format
	// -------------------
	result += test_format_hex_lowercase();
	result += test_format_hyphens();
	result += test_format_variant_rfc4122();
	result += test_format_version_is_4();

	// -------------------
	// Generate
	// -------------------
	result += test_generate_explicit_std_string();
	result += test_generate_implicit_string_view();
	result += test_generate_length_and_not_empty();
	result += test_generate_noexcept();
	result += test_generate_samples_match_contract();

	// -------------------
	// Uniqueness
	// -------------------
	result += test_generate_distinct();
	result += test_generate_distinct_across_threads();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
