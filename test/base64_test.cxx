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
#include <StormByte/binary_data.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <string>
#include <string_view>

using namespace StormByte;

namespace {
	BinaryData ToBytes(const std::string& str) {
		return BinaryData(str);
	}

	std::string FromBytes(const BinaryData& bytes) {
		if (bytes.empty())
			return {};
		return std::string(reinterpret_cast<const char*>(bytes.data()),
			static_cast<std::size_t>(bytes.size()));
	}
}

// -------------------
// Encode
// -------------------

int test_encode_empty() {
	int result = 0;
	const auto encoded = Base64Encode(BinaryData{});
	ASSERT_TRUE("test_encode_empty", encoded == "");
	ASSERT_EQUAL("test_encode_empty", Size{0}, encoded.size());
	RETURN_TEST("test_encode_empty", result);
}

int test_encode_hello() {
	int result = 0;
	const auto encoded = Base64Encode(ToBytes("Hello"));
	ASSERT_TRUE("test_encode_hello", encoded == "SGVsbG8=");
	RETURN_TEST("test_encode_hello", result);
}

int test_encode_one_byte_padding() {
	int result = 0;
	const auto encoded = Base64Encode(ToBytes("A"));
	ASSERT_TRUE("test_encode_one_byte_padding", encoded == "QQ==");
	RETURN_TEST("test_encode_one_byte_padding", result);
}

int test_encode_two_byte_padding() {
	int result = 0;
	const auto encoded = Base64Encode(ToBytes("AB"));
	ASSERT_TRUE("test_encode_two_byte_padding", encoded == "QUI=");
	RETURN_TEST("test_encode_two_byte_padding", result);
}

int test_encode_three_byte_no_padding() {
	int result = 0;
	const auto encoded = Base64Encode(ToBytes("ABC"));
	ASSERT_TRUE("test_encode_three_byte_no_padding", encoded == "QUJD");
	RETURN_TEST("test_encode_three_byte_no_padding", result);
}

int test_encode_span_overload() {
	int result = 0;
	const auto bytes = ToBytes("SpanTest");
	const auto encoded = Base64Encode(bytes.span());
	ASSERT_TRUE("test_encode_span_overload", encoded == "U3BhblRlc3Q=");
	RETURN_TEST("test_encode_span_overload", result);
}

int test_encode_all_bytes() {
	int result = 0;
	BinaryData original;
	original.reserve(ByteSize{256});
	for (std::size_t i = 0; i < 256; ++i)
		original.push_back(static_cast<std::byte>(i));
	const auto encoded = Base64Encode(original);
	ASSERT_TRUE("test_encode_all_bytes", static_cast<bool>(encoded));
	ASSERT_TRUE("test_encode_all_bytes", encoded.size() > Size{0});
	RETURN_TEST("test_encode_all_bytes", result);
}

// -------------------
// Decode
// -------------------

int test_decode_empty() {
	int result = 0;
	const auto decoded = Base64Decode("");
	ASSERT_EQUAL("test_decode_empty", ByteSize{0}, decoded.size());
	RETURN_TEST("test_decode_empty", result);
}

int test_decode_hello() {
	int result = 0;
	const auto decoded = Base64Decode("SGVsbG8=");
	ASSERT_EQUAL("test_decode_hello", std::string("Hello"), FromBytes(decoded));
	RETURN_TEST("test_decode_hello", result);
}

int test_decode_whitespace() {
	int result = 0;
	const auto decoded = Base64Decode("SGVs\nbG8=\t");
	ASSERT_EQUAL("test_decode_whitespace", std::string("Hello"), FromBytes(decoded));
	RETURN_TEST("test_decode_whitespace", result);
}

int test_decode_stops_at_padding() {
	int result = 0;
	const auto decoded = Base64Decode("SGVsbG8=ZZZ");
	ASSERT_EQUAL("test_decode_stops_at_padding", std::string("Hello"), FromBytes(decoded));
	RETURN_TEST("test_decode_stops_at_padding", result);
}

int test_decode_invalid_character() {
	int result = 0;
	ASSERT_THROWS("test_decode_invalid_character", Base64Decode("SGVsbG8!"), Base64Error);
	RETURN_TEST("test_decode_invalid_character", result);
}

// -------------------
// Roundtrip
// -------------------

int test_roundtrip_padding_sizes() {
	int result = 0;
	const auto one = Base64Decode(std::string_view{Base64Encode(ToBytes("A"))});
	ASSERT_EQUAL("test_roundtrip_padding_sizes", ByteSize{1}, one.size());
	ASSERT_EQUAL("test_roundtrip_padding_sizes", static_cast<unsigned char>('A'), static_cast<unsigned char>(one[ByteSize{0}]));
	const auto two = Base64Decode(std::string_view{Base64Encode(ToBytes("AB"))});
	ASSERT_EQUAL("test_roundtrip_padding_sizes", ByteSize{2}, two.size());
	const auto three = Base64Decode(std::string_view{Base64Encode(ToBytes("ABC"))});
	ASSERT_EQUAL("test_roundtrip_padding_sizes", ByteSize{3}, three.size());
	RETURN_TEST("test_roundtrip_padding_sizes", result);
}

int test_roundtrip_all_bytes() {
	int result = 0;
	BinaryData original;
	original.reserve(ByteSize{256});
	for (std::size_t i = 0; i < 256; ++i)
		original.push_back(static_cast<std::byte>(i));
	const auto decoded = Base64Decode(std::string_view{Base64Encode(original)});
	ASSERT_TRUE("test_roundtrip_all_bytes", original == decoded);
	RETURN_TEST("test_roundtrip_all_bytes", result);
}

int test_roundtrip_long() {
	int result = 0;
	const std::string original(1000, 'X');
	const auto decoded = Base64Decode(std::string_view{Base64Encode(ToBytes(original))});
	ASSERT_EQUAL("test_roundtrip_long", original, FromBytes(decoded));
	RETURN_TEST("test_roundtrip_long", result);
}

int test_roundtrip_sizes_0_to_16() {
	int result = 0;
	for (std::size_t len = 0; len <= 16; ++len) {
		const std::string original(len, static_cast<char>('A' + (len % 26)));
		const auto decoded = Base64Decode(std::string_view{Base64Encode(ToBytes(original))});
		ASSERT_EQUAL("test_roundtrip_sizes_0_to_16", original, FromBytes(decoded));
	}
	RETURN_TEST("test_roundtrip_sizes_0_to_16", result);
}

int main() {
	int result = 0;

	// -------------------
	// Encode
	// -------------------
	result += test_encode_empty();
	result += test_encode_hello();
	result += test_encode_one_byte_padding();
	result += test_encode_two_byte_padding();
	result += test_encode_three_byte_no_padding();
	result += test_encode_span_overload();
	result += test_encode_all_bytes();

	// -------------------
	// Decode
	// -------------------
	result += test_decode_empty();
	result += test_decode_hello();
	result += test_decode_whitespace();
	result += test_decode_stops_at_padding();
	result += test_decode_invalid_character();

	// -------------------
	// Roundtrip
	// -------------------
	result += test_roundtrip_padding_sizes();
	result += test_roundtrip_all_bytes();
	result += test_roundtrip_long();
	result += test_roundtrip_sizes_0_to_16();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
