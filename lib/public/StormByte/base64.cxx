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

#include <array>
#include <cstdint>
#include <string>

using namespace StormByte;

namespace {
	constexpr char EncodeTable[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	constexpr std::array<std::uint8_t, 256> MakeDecodeTable() {
		std::array<std::uint8_t, 256> table{};
		table.fill(255);
		for (std::uint8_t i = 0; i < 26; ++i) {
			table['A' + i] = i;
			table['a' + i] = 26 + i;
		}

		for (std::uint8_t i = 0; i < 10; ++i) {
			table['0' + i] = 52 + i;
		}

		table['+'] = 62;
		table['/'] = 63;
		return table;
	}

	constexpr auto DecodeTable = MakeDecodeTable();

	Safe::String EncodeImpl(std::span<const std::byte> input) {
		const std::size_t size = input.size();
		std::string output;
		output.resize(((size + 2) / 3) * 4);
		char* out = output.data();
		std::size_t i = 0;
		while (i + 2 < size) {
			std::uint32_t n = (static_cast<std::uint32_t>(input[i])     << 16) |
							(static_cast<std::uint32_t>(input[i + 1]) <<  8) |
							static_cast<std::uint32_t>(input[i + 2]);
			*out++ = EncodeTable[(n >> 18) & 0x3F];
			*out++ = EncodeTable[(n >> 12) & 0x3F];
			*out++ = EncodeTable[(n >>  6) & 0x3F];
			*out++ = EncodeTable[ n        & 0x3F];
			i += 3;
		}

		if (i < size) {
			std::uint32_t n = static_cast<std::uint32_t>(input[i]) << 16;
			*out++ = EncodeTable[(n >> 18) & 0x3F];
			if (i + 1 < size) {
				n |= static_cast<std::uint32_t>(input[i + 1]) << 8;
				*out++ = EncodeTable[(n >> 12) & 0x3F];
				*out++ = EncodeTable[(n >>  6) & 0x3F];
				*out++ = '=';
			} else {
				*out++ = EncodeTable[(n >> 12) & 0x3F];
				*out++ = '=';
				*out++ = '=';
			}
		}

		return Safe::String(output.c_str());
	}
}

BinaryData StormByte::Base64Decode(std::string_view input) {
	BinaryData output;
	output.reserve(ByteSize{(input.size() / 4) * 3});
	std::uint32_t buffer = 0;
	int bits_collected = 0;
	for (char c : input) {
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
			continue;
		if (c == '=')
			break;
		std::uint8_t value = DecodeTable[static_cast<unsigned char>(c)];
		if (value == 255)
			throw Base64Error("Invalid character '{}' in input", c);

		buffer = (buffer << 6) | value;
		bits_collected += 6;
		if (bits_collected >= 8) {
			bits_collected -= 8;
			output.push_back(static_cast<std::byte>((buffer >> bits_collected) & 0xFF));
		}
	}

	return output;
}

Safe::String StormByte::Base64Encode(const BinaryData& input) {
	return EncodeImpl(input.span());
}

Safe::String StormByte::Base64Encode(std::span<const std::byte> input) {
	return EncodeImpl(input);
}
