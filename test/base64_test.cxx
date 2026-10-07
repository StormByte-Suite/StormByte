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

#include <StormByte/base64.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace StormByte;

namespace {
	Safe::Binary ToBytes(std::string_view text) {
		return Safe::Binary(text);
	}

	std::string FromBytes(const Safe::Binary& bytes) {
		if (bytes.empty())
			return {};
		return std::string(reinterpret_cast<const char*>(bytes.data()), static_cast<std::size_t>(bytes.size()));
	}
}

// -------------------
// Decode
// -------------------

int test_decode_empty() {
	const auto decoded = Base64Decode("");
	ASSERT_EMPTY(decoded);
	ASSERT_EQUAL(ByteSize{0}, decoded.size());
	RETURN_TEST(0);
}

int test_decode_hello() {
	const auto decoded = Base64Decode("SGVsbG8=");
	ASSERT_EQUAL(std::string("Hello"), FromBytes(decoded));
	RETURN_TEST(0);
}

int test_decode_ignores_space_tab_lf_cr() {
	const auto decoded = Base64Decode("SG Vs\tbG8=\r\n");
	ASSERT_EQUAL(std::string("Hello"), FromBytes(decoded));
	RETURN_TEST(0);
}

int test_decode_whitespace_only_is_empty() {
	const auto decoded = Base64Decode(" \t\r\n");
	ASSERT_EMPTY(decoded);
	RETURN_TEST(0);
}

int test_decode_stops_at_first_padding() {
	const auto decoded = Base64Decode("SGVsbG8=ZZZ!");
	ASSERT_EQUAL(std::string("Hello"), FromBytes(decoded));
	RETURN_TEST(0);
}

int test_decode_padding_only_is_empty() {
	const auto decoded = Base64Decode("====");
	ASSERT_EMPTY(decoded);
	RETURN_TEST(0);
}

int test_decode_without_padding_is_accepted() {
	const auto decoded = Base64Decode("QQ");
	ASSERT_EQUAL(ByteSize{1}, decoded.size());
	ASSERT_EQUAL(static_cast<unsigned char>('A'), static_cast<unsigned char>(decoded[ByteSize{0}]));
	RETURN_TEST(0);
}

int test_decode_plus_and_slash() {
	const auto decoded = Base64Decode("+/+/");
	ASSERT_EQUAL(ByteSize{3}, decoded.size());
	ASSERT_EQUAL(std::byte{0xFB}, decoded[ByteSize{0}]);
	ASSERT_EQUAL(std::byte{0xFF}, decoded[ByteSize{1}]);
	ASSERT_EQUAL(std::byte{0xBF}, decoded[ByteSize{2}]);
	RETURN_TEST(0);
}

int test_decode_invalid_character() {
	ASSERT_THROWS(Base64Decode("SGVsbG8!"), Base64Error);
	ASSERT_THROWS(Base64Decode("@"), Base64Error);
	ASSERT_THROWS(Base64Decode("QQ*="), Base64Error);
	RETURN_TEST(0);
}

int test_decode_error_is_root_leaf() {
	try {
		(void)Base64Decode("!");
		ASSERT_FAIL("Base64Decode did not throw");
	} catch (const Base64Error& error) {
		ASSERT_CONTAINS(std::string_view{error.what()}, "StormByte:");
		ASSERT_CONTAINS(std::string_view{error.what()}, "Invalid character");
	} catch (const Exception&) {
		ASSERT_FAIL("Base64Error was sliced to Exception before the typed catch");
	}
	RETURN_TEST(0);
}

// -------------------
// Encode
// -------------------

int test_encode_empty() {
	const auto encoded = Base64Encode(Safe::Binary{});
	ASSERT_TRUE(encoded.empty());
	ASSERT_EQUAL(Size{0}, encoded.size());
	ASSERT_EQUAL(std::string_view{}, std::string_view{encoded});
	RETURN_TEST(0);
}

int test_encode_empty_span() {
	const auto encoded = Base64Encode(std::span<const std::byte>{});
	ASSERT_EMPTY(encoded);
	RETURN_TEST(0);
}

int test_encode_hello() {
	const auto encoded = Base64Encode(ToBytes("Hello"));
	ASSERT_EQUAL(std::string_view{"SGVsbG8="}, std::string_view{encoded});
	RETURN_TEST(0);
}

int test_encode_one_byte_padding() {
	const auto encoded = Base64Encode(ToBytes("A"));
	ASSERT_EQUAL(std::string_view{"QQ=="}, std::string_view{encoded});
	RETURN_TEST(0);
}

int test_encode_two_byte_padding() {
	const auto encoded = Base64Encode(ToBytes("AB"));
	ASSERT_EQUAL(std::string_view{"QUI="}, std::string_view{encoded});
	RETURN_TEST(0);
}

int test_encode_three_byte_no_padding() {
	const auto encoded = Base64Encode(ToBytes("ABC"));
	ASSERT_EQUAL(std::string_view{"QUJD"}, std::string_view{encoded});
	RETURN_TEST(0);
}

int test_encode_man_vectors() {
	ASSERT_EQUAL(std::string_view{"TQ=="}, std::string_view{Base64Encode(ToBytes("M"))});
	ASSERT_EQUAL(std::string_view{"TWE="}, std::string_view{Base64Encode(ToBytes("Ma"))});
	ASSERT_EQUAL(std::string_view{"TWFu"}, std::string_view{Base64Encode(ToBytes("Man"))});
	RETURN_TEST(0);
}

int test_encode_span_matches_binary() {
	const auto bytes = ToBytes("SpanTest");
	const auto from_binary = Base64Encode(bytes);
	const auto from_span = Base64Encode(bytes.span());
	ASSERT_EQUAL(std::string_view{"U3BhblRlc3Q="}, std::string_view{from_binary});
	ASSERT_EQUAL(std::string_view{from_binary}, std::string_view{from_span});
	RETURN_TEST(0);
}

int test_encode_all_bytes_length() {
	Safe::Binary original;
	original.reserve(ByteSize{256});
	for (std::size_t i = 0; i < 256; ++i)
		original.push_back(static_cast<std::byte>(i));
	const auto encoded = Base64Encode(original);
	ASSERT_NOT_EMPTY(encoded);
	ASSERT_EQUAL(Size{344}, encoded.size());
	ASSERT_TRUE(encoded.ends_with('='));
	RETURN_TEST(0);
}

int test_encode_explicit_std_string() {
	const std::string text = static_cast<std::string>(Base64Encode(ToBytes("A")));
	ASSERT_EQUAL(std::string{"QQ=="}, text);
	RETURN_TEST(0);
}

// -------------------
// Roundtrip
// -------------------

int test_roundtrip_padding_sizes() {
	const auto one = Base64Decode(std::string_view{Base64Encode(ToBytes("A"))});
	ASSERT_EQUAL(ByteSize{1}, one.size());
	ASSERT_EQUAL(static_cast<unsigned char>('A'), static_cast<unsigned char>(one[ByteSize{0}]));
	const auto two = Base64Decode(std::string_view{Base64Encode(ToBytes("AB"))});
	ASSERT_EQUAL(ByteSize{2}, two.size());
	const auto three = Base64Decode(std::string_view{Base64Encode(ToBytes("ABC"))});
	ASSERT_EQUAL(ByteSize{3}, three.size());
	RETURN_TEST(0);
}

int test_roundtrip_all_bytes() {
	Safe::Binary original;
	original.reserve(ByteSize{256});
	for (std::size_t i = 0; i < 256; ++i)
		original.push_back(static_cast<std::byte>(i));
	const auto decoded = Base64Decode(std::string_view{Base64Encode(original)});
	ASSERT_EQUAL(original, decoded);
	RETURN_TEST(0);
}

int test_roundtrip_embedded_nul() {
	const std::byte raw[] = {std::byte{'A'}, std::byte{0}, std::byte{'B'}};
	const Safe::Binary original(std::span<const std::byte>{raw, 3});
	const auto decoded = Base64Decode(std::string_view{Base64Encode(original)});
	ASSERT_EQUAL(ByteSize{3}, decoded.size());
	ASSERT_EQUAL(original, decoded);
	RETURN_TEST(0);
}

int test_roundtrip_long() {
	const std::string original(1000, 'X');
	const auto decoded = Base64Decode(std::string_view{Base64Encode(ToBytes(original))});
	ASSERT_EQUAL(original, FromBytes(decoded));
	RETURN_TEST(0);
}

int test_roundtrip_sizes_0_to_16() {
	for (std::size_t len = 0; len <= 16; ++len) {
		const std::string original(len, static_cast<char>('A' + (len % 26)));
		const auto decoded = Base64Decode(std::string_view{Base64Encode(ToBytes(original))});
		ASSERT_EQUAL(original, FromBytes(decoded));
	}
	RETURN_TEST(0);
}

int test_roundtrip_caller_vector_span() {
	std::vector<std::byte> bytes{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
	const auto encoded = Base64Encode(std::span<const std::byte>{bytes});
	const auto decoded = Base64Decode(std::string_view{encoded});
	ASSERT_EQUAL(ByteSize{4}, decoded.size());
	ASSERT_EQUAL(std::byte{1}, decoded[ByteSize{0}]);
	ASSERT_EQUAL(std::byte{4}, decoded[ByteSize{3}]);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Decode
	// -------------------
	result += test_decode_empty();
	result += test_decode_error_is_root_leaf();
	result += test_decode_hello();
	result += test_decode_ignores_space_tab_lf_cr();
	result += test_decode_invalid_character();
	result += test_decode_padding_only_is_empty();
	result += test_decode_plus_and_slash();
	result += test_decode_stops_at_first_padding();
	result += test_decode_whitespace_only_is_empty();
	result += test_decode_without_padding_is_accepted();

	// -------------------
	// Encode
	// -------------------
	result += test_encode_all_bytes_length();
	result += test_encode_empty();
	result += test_encode_empty_span();
	result += test_encode_explicit_std_string();
	result += test_encode_hello();
	result += test_encode_man_vectors();
	result += test_encode_one_byte_padding();
	result += test_encode_span_matches_binary();
	result += test_encode_three_byte_no_padding();
	result += test_encode_two_byte_padding();

	// -------------------
	// Roundtrip
	// -------------------
	result += test_roundtrip_all_bytes();
	result += test_roundtrip_caller_vector_span();
	result += test_roundtrip_embedded_nul();
	result += test_roundtrip_long();
	result += test_roundtrip_padding_sizes();
	result += test_roundtrip_sizes_0_to_16();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
