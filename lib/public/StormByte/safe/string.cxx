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
	typedef char* sds;
	sds sdsnewlen(const void* init, size_t initlen);
	void sdsfree(sds s);
	sds sdsMakeRoomFor(sds s, size_t addlen);
	sds sdscatlen(sds s, const void* t, size_t len);
	void sdsIncrLen(sds s, ptrdiff_t incr);
	size_t stormbyte_sdslen(const sds s);
	size_t stormbyte_sdsavail(const sds s);
}

using namespace StormByte;
using namespace StormByte::Safe;
namespace Text = StormByte::Safe::Text;

namespace {
	constexpr unsigned char kLongBit = 0x80;
	constexpr unsigned char kSizeMask = 0x7f;
}

void String::SetEmpty() noexcept {
	m_storage.data[0] = '\0';
	m_storage.m_tag = 0;
}

void String::SetShort(const char* text, std::size_t count) noexcept {
	if (count != 0)
		std::memcpy(m_storage.data, text, count);
	m_storage.data[count] = '\0';
	m_storage.m_tag = static_cast<std::uint8_t>(count);
}

String::String() noexcept {
	SetEmpty();
}

String::String(const char* str) noexcept {
	if (!str) {
		SetEmpty();
		return;
	}
	SetEmpty();
	assign(std::string_view{str});
}

String::String(std::string_view str) noexcept {
	SetEmpty();
	assign(str);
}

String::String(size_type count, char character) {
	SetEmpty();
	assign(count, character);
}

String::String(const String& other, size_type pos, size_type count) {
	SetEmpty();
	if (static_cast<std::size_t>(pos) > static_cast<std::size_t>(other.size()))
		throw OutOfBoundsError("position is past size");
	assign(static_cast<std::string_view>(other.substr(pos, count)));
}

String::String(const WString& other) noexcept {
	SetEmpty();
	if (other)
		assign(Utf8::FromWide(static_cast<std::wstring_view>(other)));
}

String::String(const String& other) noexcept {
	SetEmpty();
	assign(static_cast<std::string_view>(other));
	if (other.IsLong())
		reserve(other.capacity());
}

String::String(String&& other) noexcept {
	m_storage = other.m_storage;
	other.SetEmpty();
}

String::~String() noexcept {
	ReleaseLong();
}

String& String::operator=(const String& other) noexcept {
	if (this != &other)
		assign(static_cast<std::string_view>(other));
	return *this;
}

String& String::operator=(String&& other) noexcept {
	if (this != &other) {
		ReleaseLong();
		m_storage = other.m_storage;
		other.SetEmpty();
	}
	return *this;
}

String& String::operator=(std::string_view text) {
	return assign(text);
}

bool String::IsLong() const noexcept {
	return (m_storage.m_tag & kLongBit) != 0;
}

char* String::data() noexcept {
	return IsLong() ? m_storage.long_ : m_storage.data;
}

const char* String::data() const noexcept {
	return IsLong() ? m_storage.long_ : m_storage.data;
}

Size String::size() const noexcept {
	if (IsLong())
		return Size{stormbyte_sdslen(m_storage.long_)};
	return Size{static_cast<std::size_t>(m_storage.m_tag & kSizeMask)};
}

Size String::capacity() const noexcept {
	if (IsLong())
		return Size{stormbyte_sdslen(m_storage.long_) + stormbyte_sdsavail(m_storage.long_)};
	return Size{SSO_CAPACITY};
}

Size String::max_size() const noexcept {
	return Size{std::numeric_limits<std::size_t>::max() / 2};
}

String& String::append(std::string_view text) {
	if (text.empty())
		return *this;
	reserve(size() + Size{text.size()});
	if (IsLong()) {
		m_storage.long_ = sdscatlen(m_storage.long_, text.data(), text.size());
		if (!m_storage.long_)
			throw AllocationError();
		return *this;
	}
	const std::size_t n = static_cast<std::size_t>(size());
	std::memcpy(m_storage.data + n, text.data(), text.size());
	SetShort(m_storage.data, n + text.size());
	return *this;
}

String& String::append(size_type count, char character) {
	if (count == 0)
		return *this;
	reserve(size() + count);
	if (IsLong()) {
		const std::string block(static_cast<std::size_t>(count), character);
		m_storage.long_ = sdscatlen(m_storage.long_, block.data(), block.size());
		if (!m_storage.long_)
			throw AllocationError();
		return *this;
	}
	const std::size_t n = static_cast<std::size_t>(size());
	std::memset(m_storage.data + n, static_cast<unsigned char>(character), static_cast<std::size_t>(count));
	SetShort(m_storage.data, n + static_cast<std::size_t>(count));
	return *this;
}

String& String::assign(std::string_view text) {
	ReleaseLong();
	if (text.size() <= SSO_CAPACITY) {
		SetShort(text.data(), text.size());
		return *this;
	}
	m_storage.long_ = sdsnewlen(text.data(), text.size());
	if (!m_storage.long_)
		throw AllocationError();
	m_storage.m_tag = kLongBit;
	return *this;
}

String& String::assign(size_type count, char character) {
	if (count <= Size{SSO_CAPACITY}) {
		ReleaseLong();
		if (static_cast<std::size_t>(count) != 0)
			std::memset(m_storage.data, static_cast<unsigned char>(character), static_cast<std::size_t>(count));
		SetShort(m_storage.data, static_cast<std::size_t>(count));
		return *this;
	}
	const std::string block(static_cast<std::size_t>(count), character);
	return assign(std::string_view{block});
}

String& String::operator+=(std::string_view text) {
	return append(text);
}

String& String::operator+=(char character) {
	return append(Size{1}, character);
}

void String::push_back(char character) {
	append(Size{1}, character);
}

void String::pop_back() {
	if (size() == 0)
		return;
	resize(size() - Size{1});
}

void String::clear() {
	if (!IsLong()) {
		SetEmpty();
		return;
	}
	const int drop = -static_cast<int>(stormbyte_sdslen(m_storage.long_));
	sdsIncrLen(m_storage.long_, drop);
}

void String::reserve(size_type new_capacity) {
	if (new_capacity <= capacity())
		return;
	const std::size_t requested = static_cast<std::size_t>(new_capacity);
	if (requested > std::numeric_limits<std::size_t>::max() - 1)
		throw OutOfBoundsError("reserve capacity is too large");
	if (!IsLong()) {
		const std::size_t n = static_cast<std::size_t>(size());
		sds created = sdsnewlen(m_storage.data, n);
		if (!created)
			throw AllocationError();
		sds grown = sdsMakeRoomFor(created, requested - n);
		if (!grown)
			throw AllocationError();
		m_storage.long_ = grown;
		m_storage.m_tag = kLongBit;
		return;
	}
	const std::size_t n = static_cast<std::size_t>(size());
	if (requested < n)
		throw OutOfBoundsError("reserve capacity is too large");
	sds grown = sdsMakeRoomFor(m_storage.long_, requested - n);
	if (!grown)
		throw AllocationError();
	m_storage.long_ = grown;
}

void String::shrink_to_fit() {
	if (!IsLong())
		return;
	assign(static_cast<std::string_view>(*this));
}

void String::resize(size_type count, char character) {
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
	const int delta = static_cast<int>(static_cast<std::size_t>(count)) - static_cast<int>(stormbyte_sdslen(m_storage.long_));
	sdsIncrLen(m_storage.long_, delta);
	m_storage.long_[static_cast<std::size_t>(count)] = '\0';
}

String& String::insert(size_type position, std::string_view text) {
	const std::string_view view = *this;
	if (static_cast<std::size_t>(position) > view.size())
		throw OutOfBoundsError("insert position is past size");
	std::string merged;
	merged.reserve(view.size() + text.size());
	merged.append(view.substr(0, static_cast<std::size_t>(position)));
	merged.append(text);
	merged.append(view.substr(static_cast<std::size_t>(position)));
	return assign(std::string_view{merged});
}

String& String::insert(size_type position, size_type count, char character) {
	String block(count, character);
	return insert(position, static_cast<std::string_view>(block));
}

String::iterator String::insert(const_iterator pos, std::initializer_list<char> values) {
	const Size index{static_cast<std::size_t>(pos - cbegin())};
	insert(index, std::string_view(values.begin(), values.size()));
	return begin() + static_cast<std::size_t>(index);
}

String& String::erase(size_type position, size_type count) {
	const std::string_view view = *this;
	if (static_cast<std::size_t>(position) > view.size())
		throw OutOfBoundsError("erase position is past size");
	const std::size_t start = static_cast<std::size_t>(position);
	const std::size_t n = count == npos ? view.size() - start : std::min(static_cast<std::size_t>(count), view.size() - start);
	std::string merged;
	merged.append(view.substr(0, start));
	merged.append(view.substr(start + n));
	return assign(std::string_view{merged});
}

String::iterator String::erase(const_iterator pos) {
	return erase(pos, pos + 1);
}

String::iterator String::erase(const_iterator first, const_iterator last) {
	const Size index{static_cast<std::size_t>(first - cbegin())};
	const Size count{static_cast<std::size_t>(last - first)};
	erase(index, count);
	return begin() + static_cast<std::size_t>(index);
}

String& String::replace(size_type position, size_type count, std::string_view text) {
	erase(position, count);
	return insert(position, text);
}

String& String::replace(const_iterator first, const_iterator last, std::string_view text) {
	const Size index{static_cast<std::size_t>(first - cbegin())};
	const Size count{static_cast<std::size_t>(last - first)};
	return replace(index, count, text);
}

char& String::at(size_type index) {
	if (index >= size())
		throw OutOfBoundsError("index is past size");
	return data()[static_cast<std::size_t>(index)];
}

const char& String::at(size_type index) const {
	if (index >= size())
		throw OutOfBoundsError("index is past size");
	return data()[static_cast<std::size_t>(index)];
}

String::size_type String::copy(char* dest, size_type count, size_type pos) const {
	if (static_cast<std::size_t>(pos) > static_cast<std::size_t>(size()))
		throw OutOfBoundsError("copy position is past size");
	const std::size_t available = static_cast<std::size_t>(size() - pos);
	const std::size_t n = std::min(available, static_cast<std::size_t>(count));
	if (n != 0)
		std::memcpy(dest, data() + static_cast<std::size_t>(pos), n);
	return Size{n};
}

void String::swap(String& other) noexcept {
	std::swap(m_storage, other.m_storage);
}

void String::ReleaseLong() noexcept {
	if (!IsLong())
		return;
	sdsfree(m_storage.long_);
	SetEmpty();
}

String::operator WString() const noexcept {
	return WString(*this);
}

String String::ToLower(std::string_view str) noexcept {
	String result;
	result.assign(Utf8::ToLower(str));
	return result;
}

String String::ToUpper(std::string_view str) noexcept {
	String result;
	result.assign(Utf8::ToUpper(str));
	return result;
}

String String::SanitizeNewlines(std::string_view str) noexcept {
	String result;
	result.assign(Text::SanitizeNewlines(str));
	return result;
}

String String::RemoveWhitespace(std::string_view str) noexcept {
	String result;
	result.assign(Text::RemoveWhitespace(str));
	return result;
}

bool String::IsInteger(std::string_view str) noexcept {
	return Text::IsInteger(str);
}

Vector<String> String::Split(std::string_view str) {
	Vector<String> result;
	std::size_t index = 0;
	while (index < str.size()) {
		while (index < str.size() && std::isspace(static_cast<unsigned char>(str[index])) != 0)
			++index;
		if (index >= str.size())
			break;
		std::size_t end = index;
		while (end < str.size() && std::isspace(static_cast<unsigned char>(str[end])) == 0)
			++end;
		result.push_back(String(str.substr(index, end - index)));
		index = end;
	}
	return result;
}

Vector<String> String::Split() const {
	return Split(static_cast<std::string_view>(*this));
}

Queue<String> String::Explode(std::string_view str, char delimiter) {
	Queue<String> result;
	std::size_t start = 0;
	for (std::size_t index = 0; index <= str.size(); ++index) {
		if (index == str.size() || str[index] == delimiter) {
			result.push(String(str.substr(start, index - start)));
			start = index + 1;
		}
	}
	return result;
}

Queue<String> String::Explode(char delimiter) const {
	return Explode(static_cast<std::string_view>(*this), delimiter);
}
