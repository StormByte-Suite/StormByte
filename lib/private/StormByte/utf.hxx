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

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace StormByte {
	constexpr char32_t kUtfReplacement = 0xFFFD;

		inline char32_t NextUtf8(const unsigned char*& cursor, const unsigned char* end) noexcept {
			if (cursor >= end)
				return kUtfReplacement;
			const unsigned char lead = *cursor++;
			if (lead < 0x80)
				return lead;

			int extra = 0;
			char32_t code = 0;
			char32_t minimum = 0;
			if ((lead & 0xE0) == 0xC0) {
				extra = 1;
				code = lead & 0x1F;
				minimum = 0x80;
			} else if ((lead & 0xF0) == 0xE0) {
				extra = 2;
				code = lead & 0x0F;
				minimum = 0x800;
			} else if ((lead & 0xF8) == 0xF0) {
				extra = 3;
				code = lead & 0x07;
				minimum = 0x10000;
			} else {
				return kUtfReplacement;
			}

			if (end - cursor < extra)
				return kUtfReplacement;
			for (int i = 0; i < extra; ++i) {
				if ((cursor[i] & 0xC0) != 0x80)
					return kUtfReplacement;
				code = (code << 6) | (cursor[i] & 0x3F);
			}
			cursor += extra;
			if (code < minimum || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF))
				return kUtfReplacement;
			return code;
		}

		inline void AppendUtf8(std::string& out, char32_t code) {
			if (code <= 0x7F) {
				out.push_back(static_cast<char>(code));
			} else if (code <= 0x7FF) {
				out.push_back(static_cast<char>(0xC0 | (code >> 6)));
				out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
			} else if (code <= 0xFFFF) {
				out.push_back(static_cast<char>(0xE0 | (code >> 12)));
				out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
			} else {
				out.push_back(static_cast<char>(0xF0 | (code >> 18)));
				out.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
			}
		}

		inline void AppendWide(std::wstring& out, char32_t code) {
			if constexpr (sizeof(wchar_t) == 2) {
				if (code <= 0xFFFF) {
					out.push_back(static_cast<wchar_t>(code));
					return;
				}
				code -= 0x10000;
				out.push_back(static_cast<wchar_t>(0xD800 + (code >> 10)));
				out.push_back(static_cast<wchar_t>(0xDC00 + (code & 0x3FF)));
			} else {
				out.push_back(static_cast<wchar_t>(code));
			}
		}

		inline char32_t NextWide(const wchar_t*& cursor) noexcept {
			const char32_t code = static_cast<char32_t>(*cursor++);
			if constexpr (sizeof(wchar_t) != 2)
				return code;
			if (code < 0xD800 || code > 0xDFFF)
				return code;
			if (code > 0xDBFF || *cursor == L'\0')
				return kUtfReplacement;
			const char32_t low = static_cast<char32_t>(*cursor);
			if (low < 0xDC00 || low > 0xDFFF)
				return kUtfReplacement;
			++cursor;
			return 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
		}

	inline std::wstring Utf8ToWide(const char* text) {
		std::wstring out;
		const auto* cursor = reinterpret_cast<const unsigned char*>(text);
		const auto* end = cursor;
		while (*end != '\0')
			++end;
		while (cursor < end)
			AppendWide(out, NextUtf8(cursor, end));
		return out;
	}

	inline std::string WideToUtf8(const wchar_t* text) {
		std::string out;
		while (*text != L'\0')
			AppendUtf8(out, NextWide(text));
		return out;
	}
}
