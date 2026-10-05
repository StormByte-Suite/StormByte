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

#include <StormByte/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/text.hxx>
#include <StormByte/safe/utf8.hxx>
#include <StormByte/safe/wstring.hxx>

#include <string>
#include <utility>

using namespace StormByte;
using namespace StormByte::Safe;
namespace Text = StormByte::Safe::Text;

struct String::TextStorage {
	std::string value;
};

String::String() noexcept = default;

String::String(const char* str) noexcept {
	if (str)
		EnsureStorage().value.assign(str);
}

String::String(std::string_view str) noexcept {
	EnsureStorage().value.assign(str);
}

String::String(const WString& other) noexcept {
	if (other)
		EnsureStorage().value = Utf8::FromWide(static_cast<std::wstring_view>(other));
}

String::String(const String& other) noexcept {
	if (other.m_text) {
		auto& value = EnsureStorage().value;
		value.reserve(other.m_text->value.capacity());
		value.assign(other.m_text->value);
	}
}

String::String(String&& other) noexcept: m_text(std::move(other.m_text)) {}

String::~String() noexcept = default;

String& String::operator=(const String& other) noexcept {
	if (this != &other) {
		if (other.m_text)
			EnsureStorage().value.assign(other.m_text->value);
		else
			m_text.reset();
	}
	return *this;
}

String& String::operator=(String&& other) noexcept {
	if (this != &other)
		m_text = std::move(other.m_text);
	return *this;
}

String& String::operator=(std::string_view text) {
	return assign(text);
}

String::TextStorage& String::EnsureStorage() {
	if (!m_text)
		m_text = Heap::MakeUnique<TextStorage>();
	return *m_text;
}

char* String::data() noexcept {
	return m_text ? m_text->value.data() : nullptr;
}

const char* String::data() const noexcept {
	return m_text ? m_text->value.data() : nullptr;
}

Size String::size() const noexcept {
	return m_text ? Size{m_text->value.size()} : Size{};
}

Size String::capacity() const noexcept {
	return m_text ? Size{m_text->value.capacity()} : Size{};
}

String& String::append(std::string_view text) {
	EnsureStorage().value.append(text);
	return *this;
}

String& String::append(size_type count, char character) {
	EnsureStorage().value.append(static_cast<std::size_t>(count), character);
	return *this;
}

String& String::assign(std::string_view text) {
	EnsureStorage().value.assign(text);
	return *this;
}

String& String::assign(size_type count, char character) {
	EnsureStorage().value.assign(static_cast<std::size_t>(count), character);
	return *this;
}

String& String::operator+=(std::string_view text) {
	return append(text);
}

String& String::operator+=(char character) {
	return append(Size{1}, character);
}

void String::push_back(char character) {
	EnsureStorage().value.push_back(character);
}

void String::pop_back() {
	EnsureStorage().value.pop_back();
}

void String::clear() {
	EnsureStorage().value.clear();
}

void String::reserve(size_type new_capacity) {
	if (new_capacity <= capacity())
		return;
	try {
		EnsureStorage().value.reserve(static_cast<std::size_t>(new_capacity));
	} catch (const std::length_error&) {
		throw OutOfBoundsError("Safe::String reserve capacity is too large");
	} catch (const std::bad_alloc&) {
		throw AllocationError();
	}
}

void String::resize(size_type count, char character) {
	EnsureStorage().value.resize(static_cast<std::size_t>(count), character);
}

String& String::insert(size_type position, std::string_view text) {
	EnsureStorage().value.insert(static_cast<std::size_t>(position), text);
	return *this;
}

String& String::erase(size_type position, size_type count) {
	EnsureStorage().value.erase(static_cast<std::size_t>(position), static_cast<std::size_t>(count));
	return *this;
}

String& String::replace(size_type position, size_type count, std::string_view text) {
	EnsureStorage().value.replace(static_cast<std::size_t>(position), static_cast<std::size_t>(count), text);
	return *this;
}

void String::swap(String& other) noexcept {
	m_text.swap(other.m_text);
}

String::operator WString() const noexcept {
	return WString(*this);
}

String String::ToLower(std::string_view str) noexcept {
	String result;
	result.EnsureStorage().value = Utf8::ToLower(str);
	return result;
}

String String::ToUpper(std::string_view str) noexcept {
	String result;
	result.EnsureStorage().value = Utf8::ToUpper(str);
	return result;
}

String String::SanitizeNewlines(std::string_view str) noexcept {
	String result;
	result.EnsureStorage().value = Text::SanitizeNewlines(str);
	return result;
}

String String::RemoveWhitespace(std::string_view str) noexcept {
	String result;
	result.EnsureStorage().value = Text::RemoveWhitespace(str);
	return result;
}

bool String::IsInteger(std::string_view str) noexcept {
	return Text::IsInteger(str);
}

Status String::Split(std::string_view str, Vector<String>& out) noexcept {
	try {
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
		out = std::move(result);
		return Status::Success;
	} catch (...) {
		return Status::Failure;
	}
}

Status String::Split(Vector<String>& out) const noexcept {
	return Split(static_cast<std::string_view>(*this), out);
}

Status String::Explode(std::string_view str, char delimiter, Queue<String>& out) noexcept {
	try {
		Queue<String> result;
		std::size_t start = 0;
		for (std::size_t index = 0; index <= str.size(); ++index) {
			if (index == str.size() || str[index] == delimiter) {
				result.push(String(str.substr(start, index - start)));
				start = index + 1;
			}
		}
		out = std::move(result);
		return Status::Success;
	} catch (...) {
		return Status::Failure;
	}
}

Status String::Explode(char delimiter, Queue<String>& out) const noexcept {
	return Explode(static_cast<std::string_view>(*this), delimiter, out);
}
