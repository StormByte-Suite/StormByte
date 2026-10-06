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

#include <StormByte/binary_data.hxx>
#include <StormByte/safe/exception.hxx>

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <vector>

using StormByte::BinaryData;
using StormByte::ByteSize;
using StormByte::Safe::OutOfBoundsError;
using StormByte::Safe::String;
using StormByte::Size;

struct BinaryData::Storage {
	std::vector<std::byte> bytes;
};

static std::size_t AsIndex(const ByteSize& n) {
	return static_cast<std::size_t>(n);
}

static ByteSize AsByteSize(std::size_t n) {
	return ByteSize{n};
}

BinaryData::BinaryData() noexcept
	: m_storage(std::make_unique<Storage>()) {}

BinaryData::BinaryData(const ByteSize& count, std::byte value)
	: m_storage(std::make_unique<Storage>()) {
	m_storage->bytes.assign(AsIndex(count), value);
}

BinaryData::BinaryData(const ByteSize& count)
	: BinaryData(count, std::byte{0}) {}

BinaryData::BinaryData(std::span<const std::byte> bytes)
	: m_storage(std::make_unique<Storage>()) {
	m_storage->bytes.assign(bytes.begin(), bytes.end());
}

BinaryData::BinaryData(const std::byte* bytes, const ByteSize& count)
	: m_storage(std::make_unique<Storage>()) {
	if (bytes == nullptr && count != ByteSize{0})
		throw OutOfBoundsError("null source with non-zero count");
	if (count == ByteSize{0})
		return;
	m_storage->bytes.assign(bytes, bytes + AsIndex(count));
}

BinaryData::BinaryData(std::initializer_list<std::byte> list)
	: m_storage(std::make_unique<Storage>()) {
	m_storage->bytes.assign(list);
}

BinaryData::BinaryData(std::string_view sv)
	: m_storage(std::make_unique<Storage>()) {
	const auto* raw = reinterpret_cast<const std::byte*>(sv.data());
	m_storage->bytes.assign(raw, raw + sv.size());
}

BinaryData::BinaryData(const char* s)
	: BinaryData(s ? std::string_view(s) : std::string_view{}) {}

BinaryData::BinaryData(const std::vector<std::byte>& bytes)
	: m_storage(std::make_unique<Storage>()) {
	m_storage->bytes.assign(bytes.begin(), bytes.end());
}

BinaryData::BinaryData(std::vector<std::byte>&& bytes)
	: m_storage(std::make_unique<Storage>()) {
	m_storage->bytes.assign(bytes.begin(), bytes.end());
	bytes.clear();
	bytes.shrink_to_fit();
}

BinaryData::BinaryData(const BinaryData& other)
	: m_storage(std::make_unique<Storage>()) {
	m_storage->bytes = other.m_storage->bytes;
}

BinaryData::BinaryData(BinaryData&& other) noexcept
	: m_storage(std::move(other.m_storage)) {
	other.m_storage = std::make_unique<Storage>();
}

BinaryData::~BinaryData() noexcept = default;

BinaryData& BinaryData::operator=(const BinaryData& other) {
	if (this != &other)
		m_storage->bytes = other.m_storage->bytes;
	return *this;
}

BinaryData& BinaryData::operator=(BinaryData&& other) noexcept {
	if (this != &other) {
		m_storage = std::move(other.m_storage);
		other.m_storage = std::make_unique<Storage>();
	}
	return *this;
}

BinaryData& BinaryData::operator=(std::initializer_list<std::byte> list) {
	m_storage->bytes.assign(list);
	return *this;
}

bool BinaryData::operator==(const BinaryData& other) const noexcept {
	return m_storage->bytes == other.m_storage->bytes;
}

bool BinaryData::operator!=(const BinaryData& other) const noexcept {
	return !(*this == other);
}

std::strong_ordering BinaryData::operator<=>(const BinaryData& other) const noexcept {
	return m_storage->bytes <=> other.m_storage->bytes;
}

bool BinaryData::operator==(std::span<const std::byte> bytes) const noexcept {
	return span().size() == bytes.size()
		&& std::equal(span().begin(), span().end(), bytes.begin());
}

bool BinaryData::operator!=(std::span<const std::byte> bytes) const noexcept {
	return !(*this == bytes);
}

std::strong_ordering BinaryData::operator<=>(std::span<const std::byte> bytes) const noexcept {
	return std::lexicographical_compare_three_way(
		span().begin(), span().end(), bytes.begin(), bytes.end());
}

BinaryData::iterator BinaryData::begin() noexcept {
	return m_storage->bytes.data();
}

BinaryData::const_iterator BinaryData::begin() const noexcept {
	return m_storage->bytes.data();
}

BinaryData::iterator BinaryData::end() noexcept {
	return m_storage->bytes.data() + m_storage->bytes.size();
}

BinaryData::const_iterator BinaryData::end() const noexcept {
	return m_storage->bytes.data() + m_storage->bytes.size();
}

BinaryData::const_iterator BinaryData::cbegin() const noexcept {
	return begin();
}

BinaryData::const_iterator BinaryData::cend() const noexcept {
	return end();
}

BinaryData::reverse_iterator BinaryData::rbegin() noexcept {
	return reverse_iterator(end());
}

BinaryData::reverse_iterator BinaryData::rend() noexcept {
	return reverse_iterator(begin());
}

BinaryData::const_reverse_iterator BinaryData::rbegin() const noexcept {
	return const_reverse_iterator(end());
}

BinaryData::const_reverse_iterator BinaryData::rend() const noexcept {
	return const_reverse_iterator(begin());
}

BinaryData::const_reverse_iterator BinaryData::crbegin() const noexcept {
	return rbegin();
}

BinaryData::const_reverse_iterator BinaryData::crend() const noexcept {
	return rend();
}

ByteSize BinaryData::size() const noexcept {
	return AsByteSize(m_storage->bytes.size());
}

ByteSize BinaryData::max_size() const noexcept {
	return AsByteSize(m_storage->bytes.max_size());
}

ByteSize BinaryData::capacity() const noexcept {
	return AsByteSize(m_storage->bytes.capacity());
}

bool BinaryData::empty() const noexcept {
	return m_storage->bytes.empty();
}

void BinaryData::reserve(const ByteSize& new_cap) {
	m_storage->bytes.reserve(AsIndex(new_cap));
}

void BinaryData::resize(const ByteSize& new_size) {
	m_storage->bytes.resize(AsIndex(new_size));
}

void BinaryData::resize(const ByteSize& new_size, std::byte value) {
	m_storage->bytes.resize(AsIndex(new_size), value);
}

void BinaryData::shrink_to_fit() {
	m_storage->bytes.shrink_to_fit();
}

void BinaryData::clear() noexcept {
	m_storage->bytes.clear();
}

std::byte& BinaryData::operator[](const ByteSize& index) noexcept {
	return m_storage->bytes[AsIndex(index)];
}

const std::byte& BinaryData::operator[](const ByteSize& index) const noexcept {
	return m_storage->bytes[AsIndex(index)];
}

std::byte& BinaryData::at(const ByteSize& index) {
	if (index >= size())
		throw OutOfBoundsError("index out of range");
	return m_storage->bytes[AsIndex(index)];
}

const std::byte& BinaryData::at(const ByteSize& index) const {
	if (index >= size())
		throw OutOfBoundsError("index out of range");
	return m_storage->bytes[AsIndex(index)];
}

std::byte& BinaryData::front() {
	return m_storage->bytes.front();
}

const std::byte& BinaryData::front() const {
	return m_storage->bytes.front();
}

std::byte& BinaryData::back() {
	return m_storage->bytes.back();
}

const std::byte& BinaryData::back() const {
	return m_storage->bytes.back();
}

std::byte* BinaryData::data() noexcept {
	return m_storage->bytes.empty() ? nullptr : m_storage->bytes.data();
}

const std::byte* BinaryData::data() const noexcept {
	return m_storage->bytes.empty() ? nullptr : m_storage->bytes.data();
}

std::span<std::byte> BinaryData::span() noexcept {
	return std::span<std::byte>(data(), AsIndex(size()));
}

std::span<const std::byte> BinaryData::span() const noexcept {
	return std::span<const std::byte>(data(), AsIndex(size()));
}

BinaryData::operator std::span<std::byte>() noexcept {
	return span();
}

BinaryData::operator std::span<const std::byte>() const noexcept {
	return span();
}

void BinaryData::assign(const ByteSize& count, std::byte value) {
	m_storage->bytes.assign(AsIndex(count), value);
}

void BinaryData::assign(std::span<const std::byte> bytes) {
	m_storage->bytes.assign(bytes.begin(), bytes.end());
}

void BinaryData::assign(std::initializer_list<std::byte> list) {
	m_storage->bytes.assign(list);
}

void BinaryData::append(std::span<const std::byte> bytes) {
	m_storage->bytes.insert(m_storage->bytes.end(), bytes.begin(), bytes.end());
}

void BinaryData::append(const std::byte* bytes, const ByteSize& count) {
	if (bytes == nullptr && count != ByteSize{0})
		throw OutOfBoundsError("null source with non-zero count");
	if (count == ByteSize{0})
		return;
	append(std::span<const std::byte>(bytes, AsIndex(count)));
}

void BinaryData::append(const BinaryData& other) {
	if (this == &other) {
		const auto copy = other.m_storage->bytes;
		m_storage->bytes.insert(m_storage->bytes.end(), copy.begin(), copy.end());
		return;
	}
	append(other.span());
}

void BinaryData::append(BinaryData&& other) {
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

BinaryData& BinaryData::operator+=(std::span<const std::byte> bytes) {
	append(bytes);
	return *this;
}

BinaryData& BinaryData::operator+=(const BinaryData& other) {
	append(other);
	return *this;
}

BinaryData& BinaryData::operator+=(BinaryData&& other) {
	append(std::move(other));
	return *this;
}

void BinaryData::push_back(std::byte value) {
	m_storage->bytes.push_back(value);
}

void BinaryData::pop_back() {
	m_storage->bytes.pop_back();
}

BinaryData::iterator BinaryData::insert(const_iterator pos, std::byte value) {
	const auto off = static_cast<std::size_t>(pos - cbegin());
	auto it = m_storage->bytes.insert(m_storage->bytes.begin() + static_cast<std::ptrdiff_t>(off), value);
	return m_storage->bytes.data() + (it - m_storage->bytes.begin());
}

BinaryData::iterator BinaryData::insert(const_iterator pos, const ByteSize& count, std::byte value) {
	const auto off = static_cast<std::size_t>(pos - cbegin());
	auto it = m_storage->bytes.insert(m_storage->bytes.begin() + static_cast<std::ptrdiff_t>(off), AsIndex(count), value);
	return m_storage->bytes.data() + (it - m_storage->bytes.begin());
}

BinaryData::iterator BinaryData::insert(const_iterator pos, std::initializer_list<std::byte> list) {
	return insert(pos, std::span<const std::byte>(list.begin(), list.size()));
}

BinaryData::iterator BinaryData::insert(const_iterator pos, std::span<const std::byte> bytes) {
	const auto off = static_cast<std::size_t>(pos - cbegin());
	auto it = m_storage->bytes.insert(m_storage->bytes.begin() + static_cast<std::ptrdiff_t>(off), bytes.begin(), bytes.end());
	return m_storage->bytes.data() + (it - m_storage->bytes.begin());
}

BinaryData::iterator BinaryData::erase(const_iterator pos) {
	const auto off = static_cast<std::size_t>(pos - cbegin());
	auto it = m_storage->bytes.erase(m_storage->bytes.begin() + static_cast<std::ptrdiff_t>(off));
	return m_storage->bytes.data() + (it - m_storage->bytes.begin());
}

BinaryData::iterator BinaryData::erase(const_iterator first, const_iterator last) {
	const auto off_a = static_cast<std::size_t>(first - cbegin());
	const auto off_b = static_cast<std::size_t>(last - cbegin());
	auto it = m_storage->bytes.erase(
		m_storage->bytes.begin() + static_cast<std::ptrdiff_t>(off_a),
		m_storage->bytes.begin() + static_cast<std::ptrdiff_t>(off_b));
	return m_storage->bytes.data() + (it - m_storage->bytes.begin());
}

void BinaryData::swap(BinaryData& other) noexcept {
	m_storage.swap(other.m_storage);
}

void StormByte::swap(BinaryData& lhs, BinaryData& rhs) noexcept {
	lhs.swap(rhs);
}

String BinaryData::HexDump() const {
	return HexDump(Size{16});
}

String BinaryData::HexDump(Size columns) const {
	if (empty())
		return Safe::String();

	const auto* raw = m_storage->bytes.data();
	const std::size_t total = m_storage->bytes.size();
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
	return Safe::String(out.str());
}
