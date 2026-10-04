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

#include <StormByte/safe/cstring.hxx>
#include <StormByte/utf.hxx>
#include <StormByte/safe/wcstring.hxx>

#include <cassert>
#include <cstring>

using namespace StormByte;
using namespace StormByte::Safe;

char* CString::Duplicate(const char* str) noexcept {
	if (!str)
		return nullptr;
	const std::size_t len = std::strlen(str) + 1;
	char* out = new char[len];
	std::memcpy(out, str, len);
	return out;
}

char* CString::Duplicate(std::string_view sv) noexcept {
	const std::size_t len = sv.size();
	char* out = new char[len + 1];
	if (len != 0)
		std::memcpy(out, sv.data(), len);
	out[len] = '\0';
	return out;
}

CString::CString() noexcept
: m_data(nullptr) {}

CString::CString(const char* str) noexcept
: m_data(Duplicate(str)) {}

CString::CString(std::string_view sv) noexcept
: m_data(Duplicate(sv)) {}

CString::CString(const std::string& str) noexcept
: m_data(Duplicate(std::string_view(str))) {}

CString::CString(const WCString& text) noexcept
: m_data(nullptr) {
	const wchar_t* raw = static_cast<const wchar_t*>(text);
	if (!raw)
		return;
	m_data = Duplicate(WideToUtf8(raw));
}

CString::CString(const CString& other) noexcept
: m_data(Duplicate(other.m_data)) {}

CString::CString(CString&& other) noexcept
: m_data(other.m_data) {
	other.m_data = nullptr;
}

CString::~CString() noexcept {
	delete[] m_data;
	m_data = nullptr;
}

CString& CString::operator=(const CString& other) noexcept {
	if (this != &other) {
		delete[] m_data;
		m_data = Duplicate(other.m_data);
	}
	return *this;
}

CString& CString::operator=(CString&& other) noexcept {
	if (this != &other) {
		delete[] m_data;
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	return *this;
}

void CString::Reset(const char* str) noexcept {
	delete[] m_data;
	m_data = Duplicate(str);
}

Size CString::Length() const noexcept {
	return m_data ? Size{std::strlen(m_data)} : Size{};
}

CString::size_type CString::size() const noexcept {
	return Length();
}

bool CString::empty() const noexcept {
	return Length() == Size{};
}

char& CString::operator[](const Size& index) noexcept {
	assert(m_data != nullptr);
	const Size length{std::strlen(m_data)};
	assert(index <= length);
	return m_data[static_cast<std::size_t>(index)];
}

char CString::operator[](const Size& index) const noexcept {
	assert(m_data != nullptr);
	const Size length{std::strlen(m_data)};
	assert(index <= length);
	return m_data[static_cast<std::size_t>(index)];
}

void CString::swap(CString& other) noexcept {
	char* tmp = m_data;
	m_data = other.m_data;
	other.m_data = tmp;
}

CString::operator const char*() const noexcept {
	return m_data;
}

bool CString::operator==(const CString& other) const noexcept {
	if (m_data == other.m_data)
		return true;
	if (!m_data || !other.m_data)
		return false;
	return std::strcmp(m_data, other.m_data) == 0;
}

bool CString::operator==(const char* str) const noexcept {
	if (m_data == str)
		return true;
	if (!m_data || !str)
		return false;
	return std::strcmp(m_data, str) == 0;
}

std::strong_ordering CString::operator<=>(const CString& other) const noexcept {
	if (!m_data && !other.m_data)
		return std::strong_ordering::equal;
	if (!m_data)
		return std::strong_ordering::less;
	if (!other.m_data)
		return std::strong_ordering::greater;
	const int cmp = std::strcmp(m_data, other.m_data);
	if (cmp < 0)
		return std::strong_ordering::less;
	if (cmp > 0)
		return std::strong_ordering::greater;
	return std::strong_ordering::equal;
}

std::strong_ordering CString::operator<=>(const char* str) const noexcept {
	if (!m_data && !str)
		return std::strong_ordering::equal;
	if (!m_data)
		return std::strong_ordering::less;
	if (!str)
		return std::strong_ordering::greater;
	const int cmp = std::strcmp(m_data, str);
	if (cmp < 0)
		return std::strong_ordering::less;
	if (cmp > 0)
		return std::strong_ordering::greater;
	return std::strong_ordering::equal;
}
