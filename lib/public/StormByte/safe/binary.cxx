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

#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/exception.hxx>

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <sstream>

using StormByte::ByteSize;
using StormByte::Safe::Binary;
using StormByte::Safe::OutOfBoundsError;
using StormByte::Safe::String;
using StormByte::Safe::Vector;
using StormByte::Size;

static std::size_t AsIndex(const ByteSize& count) {
	return static_cast<std::size_t>(count);
}

static ByteSize AsByteSize(std::size_t count) {
	return ByteSize{count};
}

Binary::Binary() noexcept = default;

Binary::Binary(const ByteSize& count, std::byte value) {
	m_bytes.assign(AsIndex(count), value);
}

Binary::Binary(const ByteSize& count)
	: Binary(count, std::byte{0}) {}

Binary::Binary(std::span<const std::byte> bytes) {
	if (!bytes.empty())
		m_bytes.assign(bytes.begin(), bytes.end());
}

Binary::Binary(const std::byte* bytes, const ByteSize& count) {
	if (bytes == nullptr && count != ByteSize{0})
		throw OutOfBoundsError("null source with non-zero count");
	if (count == ByteSize{0})
		return;
	m_bytes.assign(bytes, bytes + AsIndex(count));
}

Binary::Binary(std::initializer_list<std::byte> list) {
	m_bytes.assign(list.begin(), list.end());
}

Binary::Binary(std::string_view text) {
	const auto* raw = reinterpret_cast<const std::byte*>(text.data());
	if (text.empty())
		return;
	m_bytes.assign(raw, raw + text.size());
}

Binary::Binary(const char* text)
	: Binary(text != nullptr ? std::string_view(text) : std::string_view{}) {}

Binary::Binary(const Binary& other) = default;

Binary::Binary(Binary&& other) noexcept = default;

Binary::~Binary() noexcept = default;

Binary& Binary::operator=(const Binary& other) = default;

Binary& Binary::operator=(Binary&& other) noexcept = default;

Binary& Binary::operator=(std::initializer_list<std::byte> list) {
	m_bytes.assign(list.begin(), list.end());
	return *this;
}

bool Binary::operator==(const Binary& other) const noexcept {
	return m_bytes == other.m_bytes;
}

bool Binary::operator!=(const Binary& other) const noexcept {
	return !(*this == other);
}

std::strong_ordering Binary::operator<=>(const Binary& other) const noexcept {
	return m_bytes <=> other.m_bytes;
}

bool Binary::operator==(std::span<const std::byte> bytes) const noexcept {
	return span().size() == bytes.size()
		&& std::equal(span().begin(), span().end(), bytes.begin());
}

bool Binary::operator!=(std::span<const std::byte> bytes) const noexcept {
	return !(*this == bytes);
}

std::strong_ordering Binary::operator<=>(std::span<const std::byte> bytes) const noexcept {
	return std::lexicographical_compare_three_way(
		span().begin(), span().end(), bytes.begin(), bytes.end());
}

Binary::iterator Binary::begin() noexcept {
	return data();
}

Binary::const_iterator Binary::begin() const noexcept {
	return data();
}

Binary::iterator Binary::end() noexcept {
	return data() + AsIndex(size());
}

Binary::const_iterator Binary::end() const noexcept {
	return data() + AsIndex(size());
}

Binary::const_iterator Binary::cbegin() const noexcept {
	return begin();
}

Binary::const_iterator Binary::cend() const noexcept {
	return end();
}

Binary::reverse_iterator Binary::rbegin() noexcept {
	return reverse_iterator(end());
}

Binary::reverse_iterator Binary::rend() noexcept {
	return reverse_iterator(begin());
}

Binary::const_reverse_iterator Binary::rbegin() const noexcept {
	return const_reverse_iterator(end());
}

Binary::const_reverse_iterator Binary::rend() const noexcept {
	return const_reverse_iterator(begin());
}

Binary::const_reverse_iterator Binary::crbegin() const noexcept {
	return rbegin();
}

Binary::const_reverse_iterator Binary::crend() const noexcept {
	return rend();
}

ByteSize Binary::size() const noexcept {
	return AsByteSize(static_cast<std::size_t>(m_bytes.size()));
}

ByteSize Binary::max_size() const noexcept {
	return AsByteSize(static_cast<std::size_t>(m_bytes.max_size()));
}

ByteSize Binary::capacity() const noexcept {
	return AsByteSize(static_cast<std::size_t>(m_bytes.capacity()));
}

bool Binary::empty() const noexcept {
	return m_bytes.empty();
}

void Binary::reserve(const ByteSize& new_cap) {
	m_bytes.reserve(AsIndex(new_cap));
}

void Binary::resize(const ByteSize& new_size) {
	m_bytes.resize(AsIndex(new_size));
}

void Binary::resize(const ByteSize& new_size, std::byte value) {
	m_bytes.resize(AsIndex(new_size), value);
}

void Binary::shrink_to_fit() {
	m_bytes.shrink_to_fit();
}

void Binary::clear() noexcept {
	m_bytes.clear();
}

std::byte& Binary::operator[](const ByteSize& index) noexcept {
	return m_bytes[AsIndex(index)];
}

const std::byte& Binary::operator[](const ByteSize& index) const noexcept {
	return m_bytes[AsIndex(index)];
}

std::byte& Binary::at(const ByteSize& index) {
	if (index >= size())
		throw OutOfBoundsError("index out of range");
	return m_bytes[AsIndex(index)];
}

const std::byte& Binary::at(const ByteSize& index) const {
	if (index >= size())
		throw OutOfBoundsError("index out of range");
	return m_bytes[AsIndex(index)];
}

std::byte& Binary::front() {
	return m_bytes.front();
}

const std::byte& Binary::front() const {
	return m_bytes.front();
}

std::byte& Binary::back() {
	return m_bytes.back();
}

const std::byte& Binary::back() const {
	return m_bytes.back();
}

std::byte* Binary::data() noexcept {
	return m_bytes.empty() ? nullptr : m_bytes.data();
}

const std::byte* Binary::data() const noexcept {
	return m_bytes.empty() ? nullptr : m_bytes.data();
}

std::span<std::byte> Binary::span() noexcept {
	return std::span<std::byte>(data(), AsIndex(size()));
}

std::span<const std::byte> Binary::span() const noexcept {
	return std::span<const std::byte>(data(), AsIndex(size()));
}

Binary::operator std::span<std::byte>() noexcept {
	return span();
}

Binary::operator std::span<const std::byte>() const noexcept {
	return span();
}

void Binary::assign(const ByteSize& count, std::byte value) {
	m_bytes.assign(AsIndex(count), value);
}

void Binary::assign(std::span<const std::byte> bytes) {
	m_bytes.assign(bytes.begin(), bytes.end());
}

void Binary::assign(std::initializer_list<std::byte> list) {
	m_bytes.assign(list.begin(), list.end());
}

void Binary::append(std::span<const std::byte> bytes) {
	m_bytes.insert(m_bytes.end(), bytes.begin(), bytes.end());
}

void Binary::append(const std::byte* bytes, const ByteSize& count) {
	if (bytes == nullptr && count != ByteSize{0})
		throw OutOfBoundsError("null source with non-zero count");
	if (count == ByteSize{0})
		return;
	append(std::span<const std::byte>(bytes, AsIndex(count)));
}

void Binary::append(const Binary& other) {
	if (this == &other) {
		const Vector<std::byte> copy = m_bytes;
		m_bytes.insert(m_bytes.end(), copy.begin(), copy.end());
		return;
	}
	append(other.span());
}

void Binary::append(Binary&& other) {
	if (this == &other)
		return;
	if (empty()) {
		*this = std::move(other);
		return;
	}
	append(other.span());
	other.clear();
	other.shrink_to_fit();
}

Binary& Binary::operator+=(std::span<const std::byte> bytes) {
	append(bytes);
	return *this;
}

Binary& Binary::operator+=(const Binary& other) {
	append(other);
	return *this;
}

Binary& Binary::operator+=(Binary&& other) {
	append(std::move(other));
	return *this;
}

void Binary::push_back(std::byte value) {
	m_bytes.push_back(value);
}

void Binary::pop_back() {
	m_bytes.pop_back();
}

Binary::iterator Binary::insert(const_iterator pos, std::byte value) {
	const auto offset = static_cast<std::size_t>(pos - cbegin());
	m_bytes.insert(m_bytes.begin() + static_cast<std::ptrdiff_t>(offset), value);
	return data() + offset;
}

Binary::iterator Binary::insert(const_iterator pos, const ByteSize& count, std::byte value) {
	const auto offset = static_cast<std::size_t>(pos - cbegin());
	m_bytes.insert(m_bytes.begin() + static_cast<std::ptrdiff_t>(offset), AsIndex(count), value);
	return data() + offset;
}

Binary::iterator Binary::insert(const_iterator pos, std::initializer_list<std::byte> list) {
	return insert(pos, std::span<const std::byte>(list.begin(), list.size()));
}

Binary::iterator Binary::insert(const_iterator pos, std::span<const std::byte> bytes) {
	const auto offset = static_cast<std::size_t>(pos - cbegin());
	m_bytes.insert(m_bytes.begin() + static_cast<std::ptrdiff_t>(offset), bytes.begin(), bytes.end());
	return data() + offset;
}

Binary::iterator Binary::erase(const_iterator pos) {
	const auto offset = static_cast<std::size_t>(pos - cbegin());
	m_bytes.erase(m_bytes.begin() + static_cast<std::ptrdiff_t>(offset));
	return data() + offset;
}

Binary::iterator Binary::erase(const_iterator first, const_iterator last) {
	const auto offset = static_cast<std::size_t>(first - cbegin());
	const auto endOffset = static_cast<std::size_t>(last - cbegin());
	m_bytes.erase(
		m_bytes.begin() + static_cast<std::ptrdiff_t>(offset),
		m_bytes.begin() + static_cast<std::ptrdiff_t>(endOffset));
	return data() + offset;
}

void Binary::swap(Binary& other) noexcept {
	m_bytes.swap(other.m_bytes);
}

String Binary::HexDump() const {
	return HexDump(Size{16});
}

String Binary::HexDump(Size columns) const {
	if (empty())
		return String();

	const std::byte* raw = data();
	const std::size_t total = AsIndex(size());
	const std::size_t width = (columns == Size{0}) ? total : static_cast<std::size_t>(columns);

	std::ostringstream out;
	out << std::hex << std::setfill('0');
	for (std::size_t offset = 0; offset < total; offset += width) {
		if (offset != 0)
			out << '\n';
		out << std::setw(8) << offset << "  ";
		const std::size_t row = (total - offset < width) ? (total - offset) : width;
		for (std::size_t i = 0; i < width; ++i) {
			if (i != 0)
				out << ' ';
			if (i < row) {
				const unsigned value = static_cast<unsigned>(std::to_integer<unsigned char>(raw[offset + i]));
				out << std::setw(2) << value;
			} else {
				out << "  ";
			}
		}
		out << "  ";
		for (std::size_t i = 0; i < row; ++i) {
			const unsigned char ch = std::to_integer<unsigned char>(raw[offset + i]);
			out << ((ch >= 0x20 && ch < 0x7f) ? static_cast<char>(ch) : '.');
		}
	}
	return String(out.str());
}
