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

#include <StormByte/test_handlers.h>
#include <StormByte/uuid.hxx>

#include <set>
#include <string>

using namespace StormByte;

namespace {
	bool IsHex(char c) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
	}

	bool IsVariant(char c) {
		return c == '8' || c == '9' || c == 'a' || c == 'b';
	}
}

// -------------------
// Generate
// -------------------

int test_generate_not_null() {
	int result = 0;
	const auto uuid = GenerateUUIDv4();
	ASSERT_TRUE("test_generate_not_null", static_cast<bool>(uuid));
	ASSERT_EQUAL("test_generate_not_null", 36u, uuid.size());
	RETURN_TEST("test_generate_not_null", result);
}

int test_generate_implicit_string() {
	int result = 0;
	const std::string text = static_cast<std::string>(GenerateUUIDv4());
	ASSERT_EQUAL("test_generate_implicit_string", 36u, text.size());
	RETURN_TEST("test_generate_implicit_string", result);
}

// -------------------
// Format
// -------------------

int test_format_hyphens() {
	int result = 0;
	const auto uuid = GenerateUUIDv4();
	ASSERT_EQUAL("test_format_hyphens", '-', uuid[8]);
	ASSERT_EQUAL("test_format_hyphens", '-', uuid[13]);
	ASSERT_EQUAL("test_format_hyphens", '-', uuid[18]);
	ASSERT_EQUAL("test_format_hyphens", '-', uuid[23]);
	RETURN_TEST("test_format_hyphens", result);
}

int test_format_version_is_4() {
	int result = 0;
	const auto uuid = GenerateUUIDv4();
	ASSERT_EQUAL("test_format_version_is_4", '4', uuid[14]);
	RETURN_TEST("test_format_version_is_4", result);
}

int test_format_variant_rfc4122() {
	int result = 0;
	const auto uuid = GenerateUUIDv4();
	ASSERT_TRUE("test_format_variant_rfc4122", IsVariant(uuid[19]));
	RETURN_TEST("test_format_variant_rfc4122", result);
}

int test_format_hex_lowercase() {
	int result = 0;
	const auto uuid = GenerateUUIDv4();
	for (std::size_t i = 0; i < uuid.size(); ++i) {
		if (i == 8 || i == 13 || i == 18 || i == 23)
			continue;
		ASSERT_TRUE("test_format_hex_lowercase", IsHex(uuid[i]));
		ASSERT_TRUE("test_format_hex_lowercase", uuid[i] < 'A' || uuid[i] > 'F');
	}
	RETURN_TEST("test_format_hex_lowercase", result);
}

int test_format_rejects_wrong_length_shape() {
	int result = 0;
	const auto uuid = GenerateUUIDv4();
	ASSERT_FALSE("test_format_rejects_wrong_length_shape", uuid.size() != 36);
	ASSERT_FALSE("test_format_rejects_wrong_length_shape", uuid[8] != '-');
	ASSERT_FALSE("test_format_rejects_wrong_length_shape", uuid[14] != '4');
	ASSERT_FALSE("test_format_rejects_wrong_length_shape", !IsVariant(uuid[19]));
	RETURN_TEST("test_format_rejects_wrong_length_shape", result);
}

// -------------------
// Uniqueness
// -------------------

int test_generate_distinct() {
	int result = 0;
	std::set<std::string> seen;
	for (int i = 0; i < 1000; ++i) {
		const auto uuid = GenerateUUIDv4();
		const std::string text = static_cast<std::string>(uuid);
		ASSERT_TRUE("test_generate_distinct", !seen.contains(text));
		seen.insert(text);
	}
	ASSERT_EQUAL("test_generate_distinct", 1000u, seen.size());
	RETURN_TEST("test_generate_distinct", result);
}

int main() {
	int result = 0;

	// -------------------
	// Generate
	// -------------------
	result += test_generate_not_null();
	result += test_generate_implicit_string();

	// -------------------
	// Format
	// -------------------
	result += test_format_hyphens();
	result += test_format_version_is_4();
	result += test_format_variant_rfc4122();
	result += test_format_hex_lowercase();
	result += test_format_rejects_wrong_length_shape();

	// -------------------
	// Uniqueness
	// -------------------
	result += test_generate_distinct();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
