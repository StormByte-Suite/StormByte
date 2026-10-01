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

using StormByte::Safe::CString;
using StormByte::Safe::String;
using StormByte::Safe::WString;
namespace Text = StormByte::Safe::Text;

namespace {
	CString Own(std::string_view str) noexcept {
		if (str.empty())
			return CString("");

		std::string copy(str);
		return CString(copy.c_str());
	}
}

String::String() noexcept = default;

String::String(const char* str) noexcept: m_text(str) {}

String::String(std::string_view str) noexcept: m_text(Own(str)) {}

String::String(CString text) noexcept: m_text(std::move(text)) {}

String::String(const WString& other) noexcept: m_text(StormByte::Safe::Utf8::FromWide(other)) {}

String::String(const String& other) noexcept = default;

String::String(String&& other) noexcept = default;

String& String::operator=(const String& other) noexcept = default;

String& String::operator=(String&& other) noexcept = default;

void String::swap(String& other) noexcept {
	m_text.swap(other.m_text);
}

String::operator WString() const noexcept {
	return WString(*this);
}

String String::ToLower(std::string_view str) noexcept {
	return String(StormByte::Safe::Utf8::ToLower(str));
}

String String::ToUpper(std::string_view str) noexcept {
	return String(StormByte::Safe::Utf8::ToUpper(str));
}

String String::SanitizeNewlines(std::string_view str) noexcept {
	return String(CString(Text::SanitizeNewlines(str).c_str()));
}

String String::RemoveWhitespace(std::string_view str) noexcept {
	return String(CString(Text::RemoveWhitespace(str).c_str()));
}

bool String::IsInteger(std::string_view str) noexcept {
	return Text::IsInteger(str);
}
