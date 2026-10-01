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

#include <StormByte/safe/string.hxx>
#include <StormByte/safe/text.hxx>
#include <StormByte/safe/utf8.hxx>
#include <StormByte/safe/wstring.hxx>

#include <string>
#include <utility>

using StormByte::Safe::WCString;
using StormByte::Safe::String;
using StormByte::Safe::WString;
namespace Text = StormByte::Safe::Text;

namespace {
	WCString Own(std::wstring_view str) noexcept {
		if (str.empty())
			return WCString(L"");

		std::wstring copy(str);
		return WCString(copy.c_str());
	}
}

WString::WString() noexcept = default;

WString::WString(const wchar_t* str) noexcept: m_text(str) {}

WString::WString(std::wstring_view str) noexcept: m_text(Own(str)) {}

WString::WString(WCString text) noexcept: m_text(std::move(text)) {}

WString::WString(const String& other) noexcept: m_text(StormByte::Safe::Utf8::ToWide(other)) {}

WString::WString(const WString& other) noexcept = default;

WString::WString(WString&& other) noexcept = default;

WString& WString::operator=(const WString& other) noexcept = default;

WString& WString::operator=(WString&& other) noexcept = default;

void WString::swap(WString& other) noexcept {
	m_text.swap(other.m_text);
}

WString::operator String() const noexcept {
	return String(*this);
}

WString WString::ToLower(std::wstring_view str) noexcept {
	return WString(StormByte::Safe::Utf8::ToLower(str));
}

WString WString::ToUpper(std::wstring_view str) noexcept {
	return WString(StormByte::Safe::Utf8::ToUpper(str));
}

WString WString::SanitizeNewlines(std::wstring_view str) noexcept {
	return WString(WCString(Text::SanitizeNewlines(str).c_str()));
}

WString WString::RemoveWhitespace(std::wstring_view str) noexcept {
	return WString(WCString(Text::RemoveWhitespace(str).c_str()));
}

bool WString::IsInteger(std::wstring_view str) noexcept {
	return Text::IsInteger(str);
}
