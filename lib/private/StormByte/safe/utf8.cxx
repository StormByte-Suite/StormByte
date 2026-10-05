/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-String.
 *
 * StormByte-String original source is dual-licensed:
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
 * Both licenses apply only to original StormByte-String source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte Base tree),
 * which remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-String is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-String. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/safe/utf8.hxx>

#include <cstdint>
#include <string>

namespace {
	constexpr std::uint32_t LatinUpper(std::uint32_t codepoint) noexcept {
		if (codepoint >= U'a' && codepoint <= U'z')
			return codepoint - 32u;
		if (codepoint >= 0xE0u && codepoint <= 0xF6u)
			return codepoint - 0x20u;
		if (codepoint >= 0xF8u && codepoint <= 0xFEu)
			return codepoint - 0x20u;
		return codepoint;
	}

	constexpr std::uint32_t LatinLower(std::uint32_t codepoint) noexcept {
		if (codepoint >= U'A' && codepoint <= U'Z')
			return codepoint + 32u;
		if (codepoint >= 0xC0u && codepoint <= 0xD6u)
			return codepoint + 0x20u;
		if (codepoint >= 0xD8u && codepoint <= 0xDEu)
			return codepoint + 0x20u;
		return codepoint;
	}

	void AppendUtf8(std::string& out, std::uint32_t codepoint) {
		if (codepoint <= 0x7F)
			out.push_back(static_cast<char>(codepoint));
		else if (codepoint <= 0x7FF) {
			out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
			out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
		} else if (codepoint <= 0xFFFF) {
			out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
			out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
			out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
		} else {
			out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
			out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
			out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
			out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
		}
	}

	std::uint32_t NextUtf8(std::string_view input, std::size_t& index) {
		const auto first = static_cast<unsigned char>(input[index]);
		std::size_t width = 0;
		std::uint32_t codepoint = 0;
		std::uint32_t minimum = 0;
		if (first <= 0x7F) {
			++index;
			return first;
		} else if (first >= 0xC2 && first <= 0xDF) {
			width = 2;
			codepoint = first & 0x1F;
			minimum = 0x80;
		} else if (first >= 0xE0 && first <= 0xEF) {
			width = 3;
			codepoint = first & 0x0F;
			minimum = 0x800;
		} else if (first >= 0xF0 && first <= 0xF4) {
			width = 4;
			codepoint = first & 0x07;
			minimum = 0x10000;
		} else {
			++index;
			return 0xFFFD;
		}

		if (index + width > input.size()) {
			++index;
			return 0xFFFD;
		}

		for (std::size_t n = 1; n < width; ++n) {
			const auto cont = static_cast<unsigned char>(input[index + n]);
			if ((cont & 0xC0) != 0x80) {
				++index;
				return 0xFFFD;
			}
			codepoint = (codepoint << 6) | (cont & 0x3F);
		}

		if (codepoint < minimum || codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
			++index;
			return 0xFFFD;
		}

		index += width;
		return codepoint;
	}

	std::uint32_t NextWide(std::wstring_view str, std::size_t& index) {
		const std::uint32_t unit = static_cast<std::uint32_t>(str[index]);
		++index;
		if constexpr (sizeof(wchar_t) == 2) {
			if (unit >= 0xD800 && unit <= 0xDBFF) {
				if (index >= str.size())
					return 0xFFFD;
				const std::uint32_t low = static_cast<std::uint32_t>(str[index]);
				if (low < 0xDC00 || low > 0xDFFF)
					return 0xFFFD;
				++index;
				return 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
			}
			if (unit >= 0xDC00 && unit <= 0xDFFF)
				return 0xFFFD;
		} else if (unit > 0x10FFFF || (unit >= 0xD800 && unit <= 0xDFFF))
			return 0xFFFD;
		return unit;
	}

	void AppendWide(std::wstring& out, std::uint32_t codepoint) {
		if constexpr (sizeof(wchar_t) == 2) {
			if (codepoint <= 0xFFFF)
				out.push_back(static_cast<wchar_t>(codepoint));
			else {
				out.push_back(static_cast<wchar_t>(0xD800 + ((codepoint - 0x10000) >> 10)));
				out.push_back(static_cast<wchar_t>(0xDC00 + ((codepoint - 0x10000) & 0x3FF)));
			}
		} else
			out.push_back(static_cast<wchar_t>(codepoint));
	}
}

std::string StormByte::Safe::Utf8::FromWide(std::wstring_view str) noexcept {
	std::string out;
	out.reserve(str.size());
	for (std::size_t i = 0; i < str.size(); )
		AppendUtf8(out, NextWide(str, i));
	return out;
}

std::wstring StormByte::Safe::Utf8::ToWide(std::string_view str) noexcept {
	std::wstring out;
	out.reserve(str.size());
	for (std::size_t i = 0; i < str.size(); )
		AppendWide(out, NextUtf8(str, i));
	return out;
}

std::string StormByte::Safe::Utf8::ToUpper(std::string_view str) noexcept {
	std::string out;
	out.reserve(str.size());
	for (std::size_t i = 0; i < str.size(); )
		AppendUtf8(out, LatinUpper(NextUtf8(str, i)));
	return out;
}

std::string StormByte::Safe::Utf8::ToLower(std::string_view str) noexcept {
	std::string out;
	out.reserve(str.size());
	for (std::size_t i = 0; i < str.size(); )
		AppendUtf8(out, LatinLower(NextUtf8(str, i)));
	return out;
}

std::wstring StormByte::Safe::Utf8::ToUpper(std::wstring_view str) noexcept {
	std::wstring out;
	out.reserve(str.size());
	for (std::size_t i = 0; i < str.size(); )
		AppendWide(out, LatinUpper(NextWide(str, i)));
	return out;
}

std::wstring StormByte::Safe::Utf8::ToLower(std::wstring_view str) noexcept {
	std::wstring out;
	out.reserve(str.size());
	for (std::size_t i = 0; i < str.size(); )
		AppendWide(out, LatinLower(NextWide(str, i)));
	return out;
}
