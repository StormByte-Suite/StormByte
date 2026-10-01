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

#pragma once

#include <StormByte/binary_data.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <span>
#include <string_view>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @brief Decodes Base64 text into bytes.
	 *
	 * Alphabet `A–Z a–z 0–9 + /` and padding `=`.
	 * Space, tab, newline and CR are ignored.
	 * Decoding stops at the first `=`.
	 * Padding is accepted but not strictly checked.
	 *
	 * @param input Base64 text (`std::string_view`, `std::string` or a string literal).
	 * @return Decoded bytes owned by Base.
	 * @throws StormByte::Base64Error If a character is outside the alphabet.
	 */
	STORMBYTE_PUBLIC BinaryData Base64Decode(std::string_view input);

	/**
	 * @brief Encodes bytes as Base64 with `=` padding.
	 * @param input Bytes to encode.
	 * @return Base64 text as a `Safe::String`.
	 */
	STORMBYTE_PUBLIC Safe::String Base64Encode(const BinaryData& input);

	/**
	 * @brief Encodes a contiguous byte span as Base64 with `=` padding.
	 * @param input Bytes to encode.
	 * @return Base64 text as a `Safe::String`.
	 */
	STORMBYTE_PUBLIC Safe::String Base64Encode(std::span<const std::byte> input);
}
