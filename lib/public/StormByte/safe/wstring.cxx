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

/**
 * @struct WString::TextStorage
 * @brief Wide code units allocated and destroyed inside Base.
 */
struct WString::TextStorage {
	/**
	 * @brief Owned code units and their retained allocation.
	 */
	std::wstring Text;
};

WString::WString() noexcept = default;

WString::WString(const wchar_t* str) noexcept {
	if (str)
		EnsureText().assign(str);
}

WString::WString(std::wstring_view str) noexcept {
	EnsureText().assign(str);
}

WString::WString(const String& other) noexcept {
	if (other)
		EnsureText() = Utf8::ToWide(static_cast<std::string_view>(other));
}

WString::WString(const WString& other) noexcept {
	if (other)
		EnsureText().assign(static_cast<std::wstring_view>(other));
}

WString::WString(WString&& other) noexcept: m_text(std::move(other.m_text)) {}

WString::~WString() noexcept = default;

WString& WString::operator=(const WString& other) noexcept {
	if (this != &other) {
		if (other)
			EnsureText().assign(static_cast<std::wstring_view>(other));
		else
			m_text.reset();
	}
	return *this;
}

WString& WString::operator=(WString&& other) noexcept {
	if (this != &other)
		m_text = std::move(other.m_text);
	return *this;
}

WString& WString::operator=(std::wstring_view text) {
	return assign(text);
}

std::wstring& WString::EnsureText() {
	if (!m_text)
		m_text = Heap::MakeUnique<TextStorage>();
	return m_text->Text;
}

wchar_t* WString::data() noexcept {
	return m_text ? m_text->Text.data() : nullptr;
}

const wchar_t* WString::data() const noexcept {
	return m_text ? m_text->Text.data() : nullptr;
}

Size WString::size() const noexcept {
	return m_text ? Size{m_text->Text.size()} : Size{};
}

Size WString::capacity() const noexcept {
	return m_text ? Size{m_text->Text.capacity()} : Size{};
}

WString& WString::append(std::wstring_view text) {
	EnsureText().append(text);
	return *this;
}

WString& WString::append(size_type count, wchar_t character) {
	EnsureText().append(static_cast<std::size_t>(count), character);
	return *this;
}

WString& WString::assign(std::wstring_view text) {
	EnsureText().assign(text);
	return *this;
}

WString& WString::assign(size_type count, wchar_t character) {
	EnsureText().assign(static_cast<std::size_t>(count), character);
	return *this;
}

WString& WString::operator+=(std::wstring_view text) {
	return append(text);
}

WString& WString::operator+=(wchar_t character) {
	return append(1, character);
}

void WString::push_back(wchar_t character) {
	EnsureText().push_back(character);
}

void WString::pop_back() {
	assert(!empty());
	EnsureText().pop_back();
}

void WString::clear() {
	EnsureText().clear();
}

void WString::reserve(size_type new_capacity) {
	if (new_capacity <= capacity())
		return;
	try {
		EnsureText().reserve(static_cast<std::size_t>(new_capacity));
	} catch (const std::length_error&) {
		throw OutOfBoundsError("Safe::WString reserve capacity is too large");
	} catch (const std::bad_alloc&) {
		throw AllocationError();
	}
}

void WString::resize(size_type count, wchar_t character) {
	EnsureText().resize(static_cast<std::size_t>(count), character);
}

WString& WString::insert(size_type position, std::wstring_view text) {
	EnsureText().insert(static_cast<std::size_t>(position), text);
	return *this;
}

WString& WString::erase(size_type position, size_type count) {
	EnsureText().erase(static_cast<std::size_t>(position), static_cast<std::size_t>(count));
	return *this;
}

WString& WString::replace(size_type position, size_type count, std::wstring_view text) {
	EnsureText().replace(static_cast<std::size_t>(position), static_cast<std::size_t>(count), text);
	return *this;
}

void WString::swap(WString& other) noexcept {
	m_text.swap(other.m_text);
}

WString::operator String() const noexcept {
	return *this ? String(*this) : String();
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

Status WString::Split(std::wstring_view str, Vector<WString>& out) noexcept {
	try {
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
		out = std::move(result);
		return Status::Success;
	} catch (...) {
		return Status::Failure;
	}
}

Status WString::Explode(std::wstring_view str, wchar_t delimiter, Queue<WString>& out) noexcept {
	try {
		Queue<WString> result;
		std::size_t start = 0;
		for (std::size_t index = 0; index <= str.size(); ++index) {
			if (index == str.size() || str[index] == delimiter) {
				result.push(WString(str.substr(start, index - start)));
				start = index + 1;
			}
		}
		out = std::move(result);
		return Status::Success;
	} catch (...) {
		return Status::Failure;
	}
}

Status WString::Split(Vector<WString>& out) const noexcept {
	return Split(static_cast<std::wstring_view>(*this), out);
}

Status WString::Explode(wchar_t delimiter, Queue<WString>& out) const noexcept {
	return Explode(static_cast<std::wstring_view>(*this), delimiter, out);
}
