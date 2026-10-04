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
#include <StormByte/exception.hxx>

#include <cassert>
#include <cwchar>
#include <cstring>
#include <limits>
#include <utility>

using namespace StormByte;
using namespace StormByte::Safe;

wchar_t* WCString::Allocate(const std::size_t capacity) {
	constexpr std::size_t header_units = (sizeof(std::size_t) + sizeof(wchar_t) - 1) / sizeof(wchar_t);
	if (capacity > std::numeric_limits<std::size_t>::max() / sizeof(wchar_t) - header_units - 1)
		throw OutOfBoundsError("Safe::WCString reserve capacity is too large");
	wchar_t* allocation;
	try {
		allocation = new wchar_t[header_units + capacity + 1];
	} catch (const std::bad_alloc&) {
		throw AllocationError();
	}
	std::memcpy(allocation, &capacity, sizeof(capacity));
	wchar_t* text = allocation + header_units;
	text[0] = L'\0';
	return text;
}

void WCString::Release(wchar_t* text) noexcept {
	if (text) {
		constexpr std::size_t header_units = (sizeof(std::size_t) + sizeof(wchar_t) - 1) / sizeof(wchar_t);
		delete[] (text - header_units);
	}
}

std::size_t WCString::CapacityOf(const wchar_t* text) noexcept {
	if (!text)
		return 0;
	constexpr std::size_t header_units = (sizeof(std::size_t) + sizeof(wchar_t) - 1) / sizeof(wchar_t);
	const char* header = reinterpret_cast<const char*>(text - header_units);
	std::size_t capacity = 0;
	std::memcpy(&capacity, header, sizeof(capacity));
	return capacity;
}

 wchar_t* WCString::Duplicate(const wchar_t* str) noexcept {
	if (!str)
		return nullptr;
	const std::size_t len = std::wcslen(str);
	wchar_t* out = Allocate(len);
	std::wmemcpy(out, str, len + 1);
	return out;
}

wchar_t* WCString::Duplicate(std::wstring_view sv) noexcept {
	const std::size_t len = sv.size();
	wchar_t* out = Allocate(len);
	if (len != 0)
		std::wmemcpy(out, sv.data(), len);
	out[len] = L'\0';
	return out;
}

WCString::WCString() noexcept
: m_data(nullptr) {}

WCString::WCString(const wchar_t* str) noexcept
: m_data(Duplicate(str)) {}

WCString::WCString(std::wstring_view sv) noexcept
: m_data(Duplicate(sv)) {}

WCString::WCString(const std::wstring& str) noexcept
: m_data(Duplicate(std::wstring_view(str))) {}

WCString::WCString(const CString& text) noexcept
: m_data(nullptr) {
	const char* raw = static_cast<const char*>(text);
	if (!raw)
		return;
	const std::wstring converted = Utf8ToWide(raw);
	m_data = Duplicate(converted.c_str());
}

WCString::WCString(const WCString& other) noexcept
: m_data(Duplicate(other.m_data)) {}

WCString::WCString(WCString&& other) noexcept
: m_data(other.m_data) {
	other.m_data = nullptr;
}

WCString::~WCString() noexcept {
	Release(m_data);
	m_data = nullptr;
}

WCString& WCString::operator=(const WCString& other) noexcept {
	if (this != &other)
		Reset(other.m_data);
	return *this;
}

WCString& WCString::operator=(WCString&& other) noexcept {
	if (this != &other) {
		Release(m_data);
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	return *this;
}

void WCString::Reset(const wchar_t* str) noexcept {
	if (!str) {
		Release(m_data);
		m_data = nullptr;
		return;
	}
	const std::size_t length = std::wcslen(str);
	if (m_data && length <= CapacityOf(m_data)) {
		std::wmemmove(m_data, str, length + 1);
		return;
	}
	wchar_t* replacement = Duplicate(str);
	Release(m_data);
	m_data = replacement;
}

void WCString::reserve(const size_type new_capacity) {
	const std::size_t requested = static_cast<std::size_t>(new_capacity);
	if (requested <= CapacityOf(m_data))
		return;
	wchar_t* replacement = Allocate(requested);
	if (m_data)
		std::wmemcpy(replacement, m_data, std::wcslen(m_data) + 1);
	else
		replacement[0] = L'\0';
	Release(m_data);
	m_data = replacement;
}

Size WCString::Length() const noexcept {
	return m_data ? Size{std::wcslen(m_data)} : Size{};
}

WCString::size_type WCString::capacity() const noexcept {
	return Size{CapacityOf(m_data)};
}

WCString::size_type WCString::size() const noexcept {
	return Length();
}

bool WCString::empty() const noexcept {
	return Length() == Size{};
}

wchar_t& WCString::operator[](const Size& index) noexcept {
	assert(m_data != nullptr);
	const Size length{std::wcslen(m_data)};
	assert(index <= length);
	return m_data[static_cast<std::size_t>(index)];
}

wchar_t WCString::operator[](const Size& index) const noexcept {
	assert(m_data != nullptr);
	const Size length{std::wcslen(m_data)};
	assert(index <= length);
	return m_data[static_cast<std::size_t>(index)];
}

void WCString::swap(WCString& other) noexcept {
	wchar_t* tmp = m_data;
	m_data = other.m_data;
	other.m_data = tmp;
}

WCString::operator const wchar_t*() const noexcept {
	return m_data;
}

bool WCString::operator==(const WCString& other) const noexcept {
	if (m_data == other.m_data)
		return true;
	if (!m_data || !other.m_data)
		return false;
	return std::wcscmp(m_data, other.m_data) == 0;
}

bool WCString::operator==(const wchar_t* str) const noexcept {
	if (m_data == str)
		return true;
	if (!m_data || !str)
		return false;
	return std::wcscmp(m_data, str) == 0;
}

std::strong_ordering WCString::operator<=>(const WCString& other) const noexcept {
	if (!m_data && !other.m_data)
		return std::strong_ordering::equal;
	if (!m_data)
		return std::strong_ordering::less;
	if (!other.m_data)
		return std::strong_ordering::greater;
	const int cmp = std::wcscmp(m_data, other.m_data);
	if (cmp < 0)
		return std::strong_ordering::less;
	if (cmp > 0)
		return std::strong_ordering::greater;
	return std::strong_ordering::equal;
}

std::strong_ordering WCString::operator<=>(const wchar_t* str) const noexcept {
	if (!m_data && !str)
		return std::strong_ordering::equal;
	if (!m_data)
		return std::strong_ordering::less;
	if (!str)
		return std::strong_ordering::greater;
	const int cmp = std::wcscmp(m_data, str);
	if (cmp < 0)
		return std::strong_ordering::less;
	if (cmp > 0)
		return std::strong_ordering::greater;
	return std::strong_ordering::equal;
}
