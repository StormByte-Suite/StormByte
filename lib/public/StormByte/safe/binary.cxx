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

// memset_s is only declared when requested before the first <string.h>.
#if defined(STORMBYTE_HAVE_MEMSET_S) && !defined(__STDC_WANT_LIB_EXT1__)
	#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/exception.hxx>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iomanip>
#include <sstream>

#if defined(STORMBYTE_HAVE_EXPLICIT_BZERO) || defined(STORMBYTE_HAVE_EXPLICIT_MEMSET) || defined(STORMBYTE_HAVE_MEMSET_S)
	#include <string.h>
#endif
#ifdef STORMBYTE_HAVE_EXPLICIT_BZERO
	#include <strings.h>	// FreeBSD declares explicit_bzero here
#endif

#ifdef WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
#endif

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

static bool Overlaps(const std::byte* storage, std::size_t size, std::span<const std::byte> bytes) noexcept {
	if (storage == nullptr || bytes.empty())
		return false;
	return !std::less<const std::byte*>{}(bytes.data(), storage) && std::less<const std::byte*>{}(bytes.data(), storage + size);
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
		throw OutOfBoundsError(StormByte::Size{static_cast<std::size_t>(count)});
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

Binary::Binary(Binary&& other) noexcept {
	const std::span<std::byte> block = other.Storage();
	m_bytes.swap(other.m_bytes);
	if (!block.empty())
		other.Release(block);
}

Binary::~Binary() noexcept = default;

Binary& Binary::operator=(const Binary& other) {
	if (this != &other)
		assign(other.span());
	return *this;
}

Binary& Binary::operator=(Binary&& other) noexcept {
	if (this == &other)
		return *this;
	// Acquire before other releases, so pages both sides lock stay locked throughout.
	const std::span<std::byte> incoming = other.Storage();
	if (!incoming.empty())
		Acquire(incoming);
	if (m_bytes.capacity() != 0) {
		Discard(Storage());
		Release(Storage());
	}
	m_bytes = std::move(other.m_bytes);
	if (!incoming.empty())
		other.Release(incoming);
	return *this;
}

Binary& Binary::operator=(std::initializer_list<std::byte> list) {
	assign(list);
	return *this;
}

static bool ConstantTimeEqual(std::span<const std::byte> left, std::span<const std::byte> right) noexcept {
	if (left.size() != right.size())
		return false;
	volatile unsigned char diff = 0;
	for (std::size_t index = 0; index < left.size(); ++index)
		diff = diff | std::to_integer<unsigned char>(left[index] ^ right[index]);
	return diff == 0;
}

bool Binary::operator==(const Binary& other) const noexcept {
	if (ConstantTimeCompare() || other.ConstantTimeCompare())
		return ConstantTimeEqual(span(), other.span());
	return m_bytes == other.m_bytes;
}

bool Binary::operator!=(const Binary& other) const noexcept {
	return !(*this == other);
}

bool Binary::operator==(std::span<const std::byte> bytes) const noexcept {
	if (ConstantTimeCompare())
		return ConstantTimeEqual(span(), bytes);
	return span().size() == bytes.size()
		&& std::equal(span().begin(), span().end(), bytes.begin());
}

bool Binary::operator!=(std::span<const std::byte> bytes) const noexcept {
	return !(*this == bytes);
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
	if (AsIndex(new_cap) > m_bytes.capacity())
		Reallocate(AsIndex(new_cap));
}

void Binary::resize(const ByteSize& new_size) {
	SetSize(AsIndex(new_size));
}

void Binary::resize(const ByteSize& new_size, std::byte value) {
	const std::size_t old = m_bytes.size();
	SetSize(AsIndex(new_size));
	if (AsIndex(new_size) > old)
		std::fill(m_bytes.data() + old, m_bytes.data() + AsIndex(new_size), value);
}

void Binary::shrink_to_fit() {
	if (m_bytes.size() != m_bytes.capacity())
		Reallocate(m_bytes.size());
}

void Binary::clear() noexcept {
	if (!m_bytes.empty())
		Discard(span());
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
		throw OutOfBoundsError(StormByte::Size{static_cast<std::size_t>(index)});
	return m_bytes[AsIndex(index)];
}

const std::byte& Binary::at(const ByteSize& index) const {
	if (index >= size())
		throw OutOfBoundsError(StormByte::Size{static_cast<std::size_t>(index)});
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

void Binary::assign(const ByteSize& count, std::byte value) {
	clear();
	resize(count, value);
}

void Binary::assign(std::span<const std::byte> bytes) {
	if (Overlaps(m_bytes.data(), m_bytes.size(), bytes)) {
		const std::size_t count = bytes.size();
		std::memmove(m_bytes.data(), bytes.data(), count);
		SetSize(count);
		return;
	}
	clear();
	if (bytes.empty())
		return;
	SetSize(bytes.size());
	std::memcpy(m_bytes.data(), bytes.data(), bytes.size());
}

void Binary::assign(std::initializer_list<std::byte> list) {
	assign(std::span<const std::byte>(list.begin(), list.size()));
}

void Binary::append(std::span<const std::byte> bytes) {
	if (bytes.empty())
		return;
	const std::size_t old = m_bytes.size();
	const bool aliased = Overlaps(m_bytes.data(), old, bytes);
	const std::size_t source = aliased ? static_cast<std::size_t>(bytes.data() - m_bytes.data()) : 0;
	SetSize(old + bytes.size());
	const std::byte* from = aliased ? m_bytes.data() + source : bytes.data();
	std::memcpy(m_bytes.data() + old, from, bytes.size());
}

void Binary::append(const std::byte* bytes, const ByteSize& count) {
	if (bytes == nullptr && count != ByteSize{0})
		throw OutOfBoundsError(StormByte::Size{static_cast<std::size_t>(count)});
	if (count == ByteSize{0})
		return;
	append(std::span<const std::byte>(bytes, AsIndex(count)));
}

void Binary::append(const Binary& other) {
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
	Grow(m_bytes.size() + 1);
	m_bytes.push_back(value);
}

void Binary::pop_back() {
	if (!m_bytes.empty())
		Discard(std::span<std::byte>(m_bytes.data() + m_bytes.size() - 1, 1));
	m_bytes.pop_back();
}

Binary::iterator Binary::insert(const_iterator pos, std::byte value) {
	std::byte* gap = OpenGap(static_cast<std::size_t>(pos - cbegin()), 1);
	*gap = value;
	return gap;
}

Binary::iterator Binary::insert(const_iterator pos, const ByteSize& count, std::byte value) {
	const auto offset = static_cast<std::size_t>(pos - cbegin());
	if (count == ByteSize{0})
		return begin() + offset;
	std::byte* gap = OpenGap(offset, AsIndex(count));
	std::fill(gap, gap + AsIndex(count), value);
	return gap;
}

Binary::iterator Binary::insert(const_iterator pos, std::initializer_list<std::byte> list) {
	return insert(pos, std::span<const std::byte>(list.begin(), list.size()));
}

Binary::iterator Binary::insert(const_iterator pos, std::span<const std::byte> bytes) {
	const auto offset = static_cast<std::size_t>(pos - cbegin());
	if (bytes.empty())
		return begin() + offset;
	const std::size_t count = bytes.size();
	if (!Overlaps(m_bytes.data(), m_bytes.size(), bytes)) {
		std::byte* gap = OpenGap(offset, count);
		std::memcpy(gap, bytes.data(), count);
		return gap;
	}
	// The source moves when the gap opens, so remap it from its old offset.
	const auto source = static_cast<std::size_t>(bytes.data() - m_bytes.data());
	std::byte* gap = OpenGap(offset, count);
	std::byte* storage = m_bytes.data();
	if (source + count <= offset) {
		std::memcpy(gap, storage + source, count);
	} else if (source >= offset) {
		std::memcpy(gap, storage + source + count, count);
	} else {
		const std::size_t head = offset - source;
		std::memcpy(gap, storage + source, head);
		std::memcpy(gap + head, storage + offset + count, count - head);
	}
	return gap;
}

Binary::iterator Binary::erase(const_iterator pos) {
	return erase(pos, pos + 1);
}

Binary::iterator Binary::erase(const_iterator first, const_iterator last) {
	const auto offset = static_cast<std::size_t>(first - cbegin());
	const auto end_offset = static_cast<std::size_t>(last - cbegin());
	const std::size_t count = end_offset - offset;
	if (count == 0)
		return begin() + offset;
	const std::size_t old = m_bytes.size();
	std::memmove(m_bytes.data() + offset, m_bytes.data() + end_offset, old - end_offset);
	SetSize(old - count);
	return begin() + offset;
}

void Binary::swap(Binary& other) noexcept {
	const std::span<std::byte> mine = Storage();
	const std::span<std::byte> theirs = other.Storage();
	if (!theirs.empty())
		Acquire(theirs);
	if (!mine.empty())
		other.Acquire(mine);
	m_bytes.swap(other.m_bytes);
	if (!mine.empty())
		Release(mine);
	if (!theirs.empty())
		other.Release(theirs);
}

void Binary::Discard(std::span<std::byte>) noexcept {}

void Binary::Acquire(std::span<std::byte>) noexcept {}

void Binary::Release(std::span<std::byte>) noexcept {}

bool Binary::ConstantTimeCompare() const noexcept {
	return false;
}

void Binary::Wipe(std::span<std::byte> bytes) noexcept {
	if (bytes.empty())
		return;
#if defined(WINDOWS)
	SecureZeroMemory(bytes.data(), bytes.size());
#elif defined(STORMBYTE_HAVE_EXPLICIT_BZERO)
	// glibc >= 2.25, musl >= 1.1.20, FreeBSD, OpenBSD, DragonFly.
	explicit_bzero(bytes.data(), bytes.size());
#elif defined(STORMBYTE_HAVE_EXPLICIT_MEMSET)
	// NetBSD.
	explicit_memset(bytes.data(), 0, bytes.size());
#elif defined(STORMBYTE_HAVE_MEMSET_S)
	// macOS and other C11 Annex K libraries.
	memset_s(bytes.data(), bytes.size(), 0, bytes.size());
#else
	volatile unsigned char* cursor = reinterpret_cast<volatile unsigned char*>(bytes.data());
	for (std::size_t index = 0; index < bytes.size(); ++index)
		cursor[index] = 0;
	// Keep the stores ordered before the block can be reused.
	__asm__ __volatile__("" : : "r"(bytes.data()) : "memory");
#endif
}

std::span<std::byte> Binary::Storage() noexcept {
	return std::span<std::byte>(m_bytes.data(), m_bytes.capacity());
}

void Binary::Reallocate(std::size_t new_cap) {
	const std::size_t count = m_bytes.size();
	Vector<std::byte> block;
	if (new_cap != 0) {
		block.reserve(new_cap);
		Acquire(std::span<std::byte>(block.data(), block.capacity()));
		block.resize(count);
		if (count != 0)
			std::memcpy(block.data(), m_bytes.data(), count);
	}
	if (m_bytes.capacity() != 0) {
		Discard(Storage());
		Release(Storage());
	}
	m_bytes.swap(block);
}

void Binary::Grow(std::size_t required) {
	const std::size_t capacity = m_bytes.capacity();
	if (required <= capacity)
		return;
	Reallocate(std::max(required, capacity * 2));
}

void Binary::SetSize(std::size_t new_size) {
	const std::size_t old = m_bytes.size();
	if (new_size < old) {
		Discard(std::span<std::byte>(m_bytes.data() + new_size, old - new_size));
		m_bytes.resize(new_size);
		return;
	}
	Grow(new_size);
	m_bytes.resize(new_size);
}

std::byte* Binary::OpenGap(std::size_t offset, std::size_t count) {
	const std::size_t old = m_bytes.size();
	SetSize(old + count);
	std::byte* storage = m_bytes.data();
	std::memmove(storage + offset + count, storage + offset, old - offset);
	std::fill(storage + offset, storage + offset + count, std::byte{0});
	return storage + offset;
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
