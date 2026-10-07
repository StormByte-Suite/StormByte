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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/text.hxx>
#include <StormByte/safe/utf8.hxx>
#include <StormByte/safe/wstring.hxx>

#include <cstring>
#include <limits>
#include <string>
#include <utility>

extern "C" {
	typedef struct WBuf WBuf;
	WBuf* wbuf_new(const wchar_t* text, size_t count);
	void wbuf_free(WBuf* buffer);
	wchar_t* wbuf_data(WBuf* buffer);
	size_t wbuf_size(const WBuf* buffer);
	size_t wbuf_avail(const WBuf* buffer);
	WBuf* wbuf_make_room(WBuf* buffer, size_t extra);
	WBuf* wbuf_append(WBuf* buffer, const wchar_t* text, size_t count);
	void wbuf_set_len(WBuf* buffer, size_t count);
}

using namespace StormByte;
using namespace StormByte::Safe;
namespace Text = StormByte::Safe::Text;

namespace {
	constexpr unsigned char kLongBit = 0x80;
	constexpr unsigned char kSizeMask = 0x7f;

	WBuf* Long(void* buffer) noexcept {
		return static_cast<WBuf*>(buffer);
	}

	const WBuf* Long(const void* buffer) noexcept {
		return static_cast<const WBuf*>(buffer);
	}
}

void WString::SetEmpty() noexcept {
	m_storage.data[0] = L'\0';
	m_storage.m_tag = 0;
}

void WString::SetShort(const wchar_t* text, std::size_t count) noexcept {
	if (count != 0)
		std::memcpy(m_storage.data, text, count * sizeof(wchar_t));
	m_storage.data[count] = L'\0';
	m_storage.m_tag = static_cast<std::uint8_t>(count);
}

WString::WString() noexcept {
	SetEmpty();
}

WString::WString(const wchar_t* str) noexcept {
	SetEmpty();
	if (str)
		assign(std::wstring_view{str});
}

WString::WString(std::wstring_view str) noexcept {
	SetEmpty();
	assign(str);
}

WString::WString(size_type count, wchar_t character) {
	SetEmpty();
	assign(count, character);
}

WString::WString(const WString& other, size_type pos, size_type count) {
	SetEmpty();
	if (static_cast<std::size_t>(pos) > static_cast<std::size_t>(other.size()))
		throw OutOfBoundsError("position is past size");
	assign(static_cast<std::wstring_view>(other.substr(pos, count)));
}

WString::WString(const String& other) noexcept {
	SetEmpty();
	assign(Utf8::ToWide(static_cast<std::string_view>(other)));
}

WString::WString(const WString& other) noexcept {
	SetEmpty();
	assign(static_cast<std::wstring_view>(other));
	if (other.IsLong())
		reserve(other.capacity());
}

WString::WString(WString&& other) noexcept {
	m_storage = other.m_storage;
	other.SetEmpty();
}

WString::~WString() noexcept {
	ReleaseLong();
}

WString& WString::operator=(const WString& other) noexcept {
	if (this != &other)
		assign(static_cast<std::wstring_view>(other));
	return *this;
}

WString& WString::operator=(WString&& other) noexcept {
	if (this != &other) {
		ReleaseLong();
		m_storage = other.m_storage;
		other.SetEmpty();
	}
	return *this;
}

WString& WString::operator=(std::wstring_view text) {
	return assign(text);
}

bool WString::IsLong() const noexcept {
	return (m_storage.m_tag & kLongBit) != 0;
}

wchar_t* WString::data() noexcept {
	return IsLong() ? wbuf_data(Long(m_storage.long_)) : m_storage.data;
}

const wchar_t* WString::data() const noexcept {
	return IsLong() ? wbuf_data(Long(m_storage.long_)) : m_storage.data;
}

Size WString::size() const noexcept {
	if (IsLong())
		return Size{wbuf_size(Long(m_storage.long_))};
	return Size{static_cast<std::size_t>(m_storage.m_tag & kSizeMask)};
}

Size WString::capacity() const noexcept {
	if (IsLong())
		return Size{wbuf_size(Long(m_storage.long_)) + wbuf_avail(Long(m_storage.long_))};
	return Size{SSO_CAPACITY};
}

Size WString::max_size() const noexcept {
	return Size{std::numeric_limits<std::size_t>::max() / sizeof(wchar_t) / 2};
}

WString& WString::append(std::wstring_view text) {
	if (text.empty())
		return *this;
	reserve(size() + Size{text.size()});
	if (IsLong()) {
		WBuf* grown = wbuf_append(Long(m_storage.long_), text.data(), text.size());
		if (!grown)
			throw AllocationError();
		m_storage.long_ = grown;
		return *this;
	}
	const std::size_t n = static_cast<std::size_t>(size());
	std::memcpy(m_storage.data + n, text.data(), text.size() * sizeof(wchar_t));
	SetShort(m_storage.data, n + text.size());
	return *this;
}

WString& WString::append(size_type count, wchar_t character) {
	if (count == 0)
		return *this;
	reserve(size() + count);
	if (IsLong()) {
		const std::wstring block(static_cast<std::size_t>(count), character);
		WBuf* grown = wbuf_append(Long(m_storage.long_), block.data(), block.size());
		if (!grown)
			throw AllocationError();
		m_storage.long_ = grown;
		return *this;
	}
	const std::size_t n = static_cast<std::size_t>(size());
	for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i)
		m_storage.data[n + i] = character;
	SetShort(m_storage.data, n + static_cast<std::size_t>(count));
	return *this;
}

WString& WString::assign(std::wstring_view text) {
	ReleaseLong();
	if (text.size() <= SSO_CAPACITY) {
		SetShort(text.data(), text.size());
		return *this;
	}
	WBuf* created = wbuf_new(text.data(), text.size());
	if (!created)
		throw AllocationError();
	m_storage.long_ = created;
	m_storage.m_tag = kLongBit;
	return *this;
}

WString& WString::assign(size_type count, wchar_t character) {
	if (count <= Size{SSO_CAPACITY}) {
		ReleaseLong();
		for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i)
			m_storage.data[i] = character;
		SetShort(m_storage.data, static_cast<std::size_t>(count));
		return *this;
	}
	const std::wstring block(static_cast<std::size_t>(count), character);
	return assign(std::wstring_view{block});
}

WString& WString::operator+=(std::wstring_view text) {
	return append(text);
}

WString& WString::operator+=(wchar_t character) {
	return append(Size{1}, character);
}

void WString::push_back(wchar_t character) {
	append(Size{1}, character);
}

void WString::pop_back() {
	assert(!empty());
	resize(size() - Size{1});
}

void WString::clear() {
	if (!IsLong()) {
		SetEmpty();
		return;
	}
	wbuf_set_len(Long(m_storage.long_), 0);
}

void WString::reserve(size_type new_capacity) {
	if (new_capacity <= capacity())
		return;
	const std::size_t requested = static_cast<std::size_t>(new_capacity);
	if (requested > std::numeric_limits<std::size_t>::max() / sizeof(wchar_t) - 1)
		throw OutOfBoundsError("reserve capacity is too large");
	if (!IsLong()) {
		const std::size_t n = static_cast<std::size_t>(size());
		WBuf* created = wbuf_new(m_storage.data, n);
		if (!created)
			throw AllocationError();
		WBuf* grown = wbuf_make_room(created, requested - n);
		if (!grown)
			throw AllocationError();
		m_storage.long_ = grown;
		m_storage.m_tag = kLongBit;
		return;
	}
	WBuf* grown = wbuf_make_room(Long(m_storage.long_), requested - static_cast<std::size_t>(size()));
	if (!grown)
		throw AllocationError();
	m_storage.long_ = grown;
}

void WString::shrink_to_fit() {
	if (!IsLong())
		return;
	assign(static_cast<std::wstring_view>(*this));
}

void WString::resize(size_type count, wchar_t character) {
	if (count == size())
		return;
	if (count > size()) {
		append(count - size(), character);
		return;
	}
	if (!IsLong()) {
		SetShort(m_storage.data, static_cast<std::size_t>(count));
		return;
	}
	wbuf_set_len(Long(m_storage.long_), static_cast<std::size_t>(count));
}

WString& WString::insert(size_type position, std::wstring_view text) {
	const std::wstring_view view = *this;
	if (static_cast<std::size_t>(position) > view.size())
		throw OutOfBoundsError("insert position is past size");
	std::wstring merged;
	merged.reserve(view.size() + text.size());
	merged.append(view.substr(0, static_cast<std::size_t>(position)));
	merged.append(text);
	merged.append(view.substr(static_cast<std::size_t>(position)));
	return assign(std::wstring_view{merged});
}

WString& WString::insert(size_type position, size_type count, wchar_t character) {
	WString block(count, character);
	return insert(position, static_cast<std::wstring_view>(block));
}

WString::iterator WString::insert(const_iterator pos, std::initializer_list<wchar_t> values) {
	const Size index{static_cast<std::size_t>(pos - cbegin())};
	insert(index, std::wstring_view(values.begin(), values.size()));
	return begin() + static_cast<std::size_t>(index);
}

WString& WString::erase(size_type position, size_type count) {
	const std::wstring_view view = *this;
	if (static_cast<std::size_t>(position) > view.size())
		throw OutOfBoundsError("erase position is past size");
	const std::size_t start = static_cast<std::size_t>(position);
	const std::size_t n = count == npos ? view.size() - start : std::min(static_cast<std::size_t>(count), view.size() - start);
	std::wstring merged;
	merged.append(view.substr(0, start));
	merged.append(view.substr(start + n));
	return assign(std::wstring_view{merged});
}

WString::iterator WString::erase(const_iterator pos) {
	return erase(pos, pos + 1);
}

WString::iterator WString::erase(const_iterator first, const_iterator last) {
	const Size index{static_cast<std::size_t>(first - cbegin())};
	const Size count{static_cast<std::size_t>(last - first)};
	erase(index, count);
	return begin() + static_cast<std::size_t>(index);
}

WString& WString::replace(size_type position, size_type count, std::wstring_view text) {
	erase(position, count);
	return insert(position, text);
}

WString& WString::replace(const_iterator first, const_iterator last, std::wstring_view text) {
	const Size index{static_cast<std::size_t>(first - cbegin())};
	const Size count{static_cast<std::size_t>(last - first)};
	return replace(index, count, text);
}

wchar_t& WString::at(size_type index) {
	if (index >= size())
		throw OutOfBoundsError("index is past size");
	return data()[static_cast<std::size_t>(index)];
}

const wchar_t& WString::at(size_type index) const {
	if (index >= size())
		throw OutOfBoundsError("index is past size");
	return data()[static_cast<std::size_t>(index)];
}

WString::size_type WString::copy(wchar_t* dest, size_type count, size_type pos) const {
	if (static_cast<std::size_t>(pos) > static_cast<std::size_t>(size()))
		throw OutOfBoundsError("copy position is past size");
	const std::size_t available = static_cast<std::size_t>(size() - pos);
	const std::size_t n = std::min(available, static_cast<std::size_t>(count));
	if (n != 0)
		std::memcpy(dest, data() + static_cast<std::size_t>(pos), n * sizeof(wchar_t));
	return Size{n};
}

void WString::swap(WString& other) noexcept {
	std::swap(m_storage, other.m_storage);
}

void WString::ReleaseLong() noexcept {
	if (!IsLong())
		return;
	wbuf_free(Long(m_storage.long_));
	SetEmpty();
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
	return WString(std::wstring_view(Text::SanitizeNewlines(str)));
}

WString WString::RemoveWhitespace(std::wstring_view str) noexcept {
	return WString(std::wstring_view(Text::RemoveWhitespace(str)));
}

bool WString::IsInteger(std::wstring_view str) noexcept {
	return Text::IsInteger(str);
}

Vector<WString> WString::Split(std::wstring_view str) {
	Vector<WString> result;
	std::size_t index = 0;
	while (index < str.size()) {
		while (index < str.size() && std::iswspace(static_cast<wint_t>(str[index])) != 0)
			++index;
		if (index >= str.size())
			break;
		std::size_t end = index;
		while (end < str.size() && std::iswspace(static_cast<wint_t>(str[end])) == 0)
			++end;
		result.push_back(WString(str.substr(index, end - index)));
		index = end;
	}
	return result;
}

Vector<WString> WString::Split() const {
	return Split(static_cast<std::wstring_view>(*this));
}

Queue<WString> WString::Explode(std::wstring_view str, wchar_t delimiter) {
	Queue<WString> result;
	std::size_t start = 0;
	for (std::size_t index = 0; index <= str.size(); ++index) {
		if (index == str.size() || str[index] == delimiter) {
			result.push(WString(str.substr(start, index - start)));
			start = index + 1;
		}
	}
	return result;
}

Queue<WString> WString::Explode(wchar_t delimiter) const {
	return Explode(static_cast<std::wstring_view>(*this), delimiter);
}
