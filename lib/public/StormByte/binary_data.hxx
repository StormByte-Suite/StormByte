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

#pragma once

#include <StormByte/byte_size.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits.hxx>

#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @file StormByte/binary_data.hxx
 * @brief DLL-boundary-safe owned byte sequence for the StormByte suite.
 */

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class BinaryData
	 * @brief DLL-boundary-safe owned contiguous sequence of @c std::byte.
	 *
	 * @par Why this type exists
	 * Suite modules must not put @c std::vector&lt;std::byte&gt; in a public
	 * signature that can cross a shared-library boundary (Windows DLL, or
	 * mismatched libc++ / libstdc++ on Unix).
	 *
	 * A @c std::vector constructed in the application and mutated inside a
	 * StormByte shared library — or returned and destroyed on the other
	 * side — allocates and frees with two heaps. That is undefined behaviour
	 * whenever the two sides do not share one CRT.
	 *
	 * @c BinaryData owns its bytes on Base's heap. Allocation, growth and
	 * destruction run in this library. The header exposes contiguous
	 * @c std::byte* iterators, @c std::span views and a vector-like API
	 * so @c &lt;algorithm&gt; and @c std::ranges keep working.
	 *
	 * Occupied length, capacity and byte offsets are @ref ByteSize.
	 * HexDump column width is @ref Size (a count of columns, not a payload).
	 *
	 * Member names are lowercase to match @c std::vector.
	 *
	 * @par Construction from / conversion to @c std::vector
	 * Bytes can be copied in from a caller-owned @c std::vector and copied
	 * out to a caller-owned @c std::vector. The rvalue overloads *look*
	 * like a move: the source is emptied after the copy. They are **not**
	 * a heap steal. @c BinaryData storage is allocated and freed by Base;
	 * @c std::vector storage is allocated and freed by the caller CRT.
	 * Donating the pointer across those heaps would reintroduce the
	 * boundary this type exists to close. Peak usage is two copies during
	 * the transfer; afterwards the emptied side can release its allocation.
	 *
	 * @par Thread safety
	 * Not thread-safe. Callers that share an instance must synchronise.
	 */
	class STORMBYTE_PUBLIC BinaryData final {
		public:
			/**
			 * @brief Element type.
			 */
			using value_type = std::byte;

			/**
			 * @brief STL size typedef; @ref size() returns @ref StormByte::ByteSize.
			 */
			using size_type = std::size_t;

			/**
			 * @brief Iterator difference.
			 */
			using difference_type = std::ptrdiff_t;

			/**
			 * @brief Mutable reference to an element.
			 */
			using reference = std::byte&;

			/**
			 * @brief Const reference to an element.
			 */
			using const_reference = const std::byte&;

			/**
			 * @brief Mutable pointer and iterator.
			 */
			using pointer = std::byte*;

			/**
			 * @brief Const pointer and iterator.
			 */
			using const_pointer = const std::byte*;

			/**
			 * @brief Contiguous mutable iterator.
			 */
			using iterator = std::byte*;

			/**
			 * @brief Contiguous const iterator.
			 */
			using const_iterator = const std::byte*;

			/**
			 * @brief Reverse iterator.
			 */
			using reverse_iterator = std::reverse_iterator<iterator>;

			/**
			 * @brief Const reverse iterator.
			 */
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;

			/**
			 * @brief Construct an empty sequence.
			 */
			BinaryData() noexcept;

			/**
			 * @brief Construct @p count bytes filled with @p value.
			 * @param count Element count.
			 * @param value Fill byte.
			 */
			BinaryData(const StormByte::ByteSize& count, std::byte value);

			/**
			 * @brief Construct @p count zeroed bytes.
			 * @param count Element count.
			 */
			explicit BinaryData(const StormByte::ByteSize& count);

			/**
			 * @brief Copy bytes from a contiguous span.
			 * @param bytes Source view (not owned).
			 */
			explicit BinaryData(std::span<const std::byte> bytes);

			/**
			 * @brief Copy @p count bytes starting at @p bytes.
			 * @param bytes Source pointer; may be null when @p count is zero.
			 * @param count Byte count.
			 */
			BinaryData(const std::byte* bytes, const StormByte::ByteSize& count);

			/**
			 * @brief Copy from an initializer list.
			 * @param list Source bytes.
			 */
			BinaryData(std::initializer_list<std::byte> list);

			/**
			 * @brief Copy character bytes from a string view (no trailing NUL).
			 * @param sv Source characters.
			 */
			explicit BinaryData(std::string_view sv);

			/**
			 * @brief Copy character bytes from a C string (no trailing NUL).
			 * @param s Source; a null pointer yields an empty sequence.
			 */
			explicit BinaryData(const char* s);

			/**
			 * @brief Copy bytes from a caller-owned vector onto Base's heap.
			 * @param bytes Source. Remains valid and unchanged.
			 * @note Not a heap steal. The vector stays on the caller CRT.
			 */
			explicit BinaryData(const std::vector<std::byte>& bytes);

			/**
			 * @brief Copy bytes from a caller-owned vector onto Base's heap, then empty @p bytes.
			 * @param bytes Source. Cleared and shrunk after the copy.
			 * @note Looks like a move. Not a heap steal: Base cannot adopt the
			 *       caller pointer. Peak usage is two copies during the transfer.
			 */
			explicit BinaryData(std::vector<std::byte>&& bytes);

			/**
			 * @brief Copy from an input range of byte-convertible values.
			 * @tparam R Range type satisfying @ref StormByte::Type::ByteInputRange.
			 * @param range Source range.
			 */
			template<Type::ByteInputRange R>
			explicit BinaryData(const R& range);

			/**
			 * @brief Consume an rvalue range. Moves when @p R is an rvalue @ref BinaryData.
			 * @tparam R Range type satisfying @ref StormByte::Type::ByteInputRange.
			 * @param range Source range.
			 * @note An lvalue @ref BinaryData is copied. @ref StormByte::Type::SameAs
			 *       ignores references, so the implementation uses
			 *       @ref StormByte::Type::LvalueReference before moving.
			 */
			template<Type::ByteInputRange R>
			explicit BinaryData(R&& range);

			/**
			 * @brief Copy construct. Allocation runs in Base.
			 * @param other Source sequence.
			 */
			BinaryData(const BinaryData& other);

			/**
			 * @brief Move construct. @p other is left empty.
			 * @param other Source sequence.
			 */
			BinaryData(BinaryData&& other) noexcept;

			/**
			 * @brief Destroy the sequence on Base's heap.
			 */
			~BinaryData() noexcept;

			/**
			 * @brief Copy assign. Allocation runs in Base.
			 * @param other Source sequence.
			 * @return @c *this.
			 */
			BinaryData& operator=(const BinaryData& other);

			/**
			 * @brief Move assign. @p other is left empty.
			 * @param other Source sequence.
			 * @return @c *this.
			 */
			BinaryData& operator=(BinaryData&& other) noexcept;

			/**
			 * @brief Replace contents with an initializer list.
			 * @param list Source bytes.
			 * @return @c *this.
			 */
			BinaryData& operator=(std::initializer_list<std::byte> list);

			/**
			 * @brief Equality of byte contents.
			 * @param other Other sequence.
			 * @return @c true when sizes and bytes match.
			 */
			bool operator==(const BinaryData& other) const noexcept;

			/**
			 * @brief Inequality of byte contents.
			 * @param other Other sequence.
			 * @return Negation of equality with another sequence.
			 */
			bool operator!=(const BinaryData& other) const noexcept;

			/**
			 * @brief Three-way lexicographical comparison (same order as @c std::vector).
			 * @param other Other sequence.
			 * @return @c std::strong_ordering.
			 */
			std::strong_ordering operator<=>(const BinaryData& other) const noexcept;

			/**
			 * @brief Equality with a byte span.
			 * @param bytes View to compare.
			 * @return @c true when sizes and bytes match.
			 */
			bool operator==(std::span<const std::byte> bytes) const noexcept;

			/**
			 * @brief Inequality with a byte span.
			 * @param bytes View to compare.
			 * @return Negation of equality with a byte span.
			 */
			bool operator!=(std::span<const std::byte> bytes) const noexcept;

			/**
			 * @brief Three-way comparison with a byte span.
			 * @param bytes View to compare.
			 * @return @c std::strong_ordering.
			 */
			std::strong_ordering operator<=>(std::span<const std::byte> bytes) const noexcept;

			/**
			 * @brief Mutable iterator to the first byte.
			 * @return @c data() when non-empty; a valid empty iterator otherwise.
			 */
			iterator begin() noexcept;

			/**
			 * @brief Const iterator to the first byte.
			 * @return @c data() when non-empty; a valid empty iterator otherwise.
			 */
			const_iterator begin() const noexcept;

			/**
			 * @brief Mutable iterator one past the last byte.
			 * @return @c data() + size.
			 */
			iterator end() noexcept;

			/**
			 * @brief Const iterator one past the last byte.
			 * @return @c data() + size.
			 */
			const_iterator end() const noexcept;

			/**
			 * @brief Const iterator to the first byte.
			 * @return Same as const @ref begin().
			 */
			const_iterator cbegin() const noexcept;

			/**
			 * @brief Const iterator one past the last byte.
			 * @return Same as const @ref end().
			 */
			const_iterator cend() const noexcept;

			/**
			 * @brief Mutable reverse iterator to the last byte.
			 * @return @c reverse_iterator(end()).
			 */
			reverse_iterator rbegin() noexcept;

			/**
			 * @brief Mutable reverse iterator to before the first byte.
			 * @return @c reverse_iterator(begin()).
			 */
			reverse_iterator rend() noexcept;

			/**
			 * @brief Const reverse iterator to the last byte.
			 * @return @c const_reverse_iterator(end()).
			 */
			const_reverse_iterator rbegin() const noexcept;

			/**
			 * @brief Const reverse iterator to before the first byte.
			 * @return @c const_reverse_iterator(begin()).
			 */
			const_reverse_iterator rend() const noexcept;

			/**
			 * @brief Const reverse iterator to the last byte.
			 * @return Same as const @ref rbegin().
			 */
			const_reverse_iterator crbegin() const noexcept;

			/**
			 * @brief Const reverse iterator to before the first byte.
			 * @return Same as const @ref rend().
			 */
			const_reverse_iterator crend() const noexcept;

			/**
			 * @brief Occupied length in bytes.
			 * @return @ref StormByte::ByteSize.
			 */
			StormByte::ByteSize size() const noexcept;

			/**
			 * @brief Implementation maximum size in bytes.
			 * @return @ref StormByte::ByteSize.
			 */
			StormByte::ByteSize max_size() const noexcept;

			/**
			 * @brief Allocated capacity in bytes.
			 * @return @ref StormByte::ByteSize.
			 */
			StormByte::ByteSize capacity() const noexcept;

			/**
			 * @brief Whether the sequence holds no bytes.
			 * @return @c true when @ref size() is zero.
			 */
			bool empty() const noexcept;

			/**
			 * @brief Request capacity of at least @p new_cap bytes.
			 * @param new_cap Requested capacity.
			 */
			void reserve(const StormByte::ByteSize& new_cap);

			/**
			 * @brief Resize to @p new_size bytes. Appended bytes are zero.
			 * @param new_size New size.
			 */
			void resize(const StormByte::ByteSize& new_size);

			/**
			 * @brief Resize to @p new_size bytes. Appended bytes are @p value.
			 * @param new_size New size.
			 * @param value Fill for new bytes.
			 */
			void resize(const StormByte::ByteSize& new_size, std::byte value);

			/**
			 * @brief Release unused capacity when the implementation allows it.
			 */
			void shrink_to_fit();

			/**
			 * @brief Drop every byte. Capacity may remain.
			 */
			void clear() noexcept;

			/**
			 * @brief Unchecked mutable subscript.
			 * @param index Byte offset.
			 * @return Reference to the byte at @p index.
			 */
			std::byte& operator[](const StormByte::ByteSize& index) noexcept;

			/**
			 * @brief Unchecked const subscript.
			 * @param index Byte offset.
			 * @return Const reference to the byte at @p index.
			 */
			const std::byte& operator[](const StormByte::ByteSize& index) const noexcept;

			/**
			 * @brief Checked mutable subscript.
			 * @param index Byte offset.
			 * @return Reference to the byte at @p index.
			 * @throws StormByte::OutOfBoundsError When @p index is not less than @ref size().
			 */
			std::byte& at(const StormByte::ByteSize& index);

			/**
			 * @brief Checked const subscript.
			 * @param index Byte offset.
			 * @return Const reference to the byte at @p index.
			 * @throws StormByte::OutOfBoundsError When @p index is not less than @ref size().
			 */
			const std::byte& at(const StormByte::ByteSize& index) const;

			/**
			 * @brief First byte.
			 * @return Reference to the first byte.
			 */
			std::byte& front();

			/**
			 * @brief First byte.
			 * @return Const reference to the first byte.
			 */
			const std::byte& front() const;

			/**
			 * @brief Last byte.
			 * @return Reference to the last byte.
			 */
			std::byte& back();

			/**
			 * @brief Last byte.
			 * @return Const reference to the last byte.
			 */
			const std::byte& back() const;

			/**
			 * @brief Mutable pointer to the first byte, or @c nullptr when empty.
			 * @return Contiguous storage.
			 */
			std::byte* data() noexcept;

			/**
			 * @brief Const pointer to the first byte, or @c nullptr when empty.
			 * @return Contiguous storage.
			 */
			const std::byte* data() const noexcept;

			/**
			 * @brief Mutable view of the occupied bytes.
			 * @return Span over @c [data(), data() + size()).
			 */
			std::span<std::byte> span() noexcept;

			/**
			 * @brief Const view of the occupied bytes.
			 * @return Span over @c [data(), data() + size()).
			 */
			std::span<const std::byte> span() const noexcept;

			/**
			 * @brief Implicit mutable span conversion.
			 * @return Same as @ref span().
			 */
			operator std::span<std::byte>() noexcept;

			/**
			 * @brief Implicit const span conversion.
			 * @return Same as const @ref span().
			 */
			operator std::span<const std::byte>() const noexcept;

			/**
			 * @brief Copy bytes onto the caller CRT as a @c std::vector.
			 * @return New vector owned by the caller. @c *this is unchanged.
			 * @note Not a heap steal. `STORMBYTE_FORCE_INLINE` so the vector
			 *       is not allocated inside this DLL.
			 */
			STORMBYTE_FORCE_INLINE explicit operator std::vector<std::byte>() const& {
				const std::span<const std::byte> bytes = span();
				return std::vector<std::byte>(bytes.begin(), bytes.end());
			}

			/**
			 * @brief Copy bytes onto the caller CRT, then release this object's storage.
			 * @return New vector owned by the caller.
			 * @note Looks like a move. Not a heap steal: Base cannot hand its
			 *       pointer to a foreign @c std::vector. Peak usage is two copies
			 *       during the transfer; @c *this is empty afterwards.
			 */
			STORMBYTE_FORCE_INLINE explicit operator std::vector<std::byte>() && {
				const std::span<const std::byte> bytes = span();
				std::vector<std::byte> out(bytes.begin(), bytes.end());
				clear();
				shrink_to_fit();
				return out;
			}

			/**
			 * @brief Replace contents with @p count copies of @p value.
			 * @param count New size.
			 * @param value Fill byte.
			 */
			void assign(const StormByte::ByteSize& count, std::byte value);

			/**
			 * @brief Replace contents with a copy of @p bytes.
			 * @param bytes Source view.
			 */
			void assign(std::span<const std::byte> bytes);

			/**
			 * @brief Replace contents with an initializer list.
			 * @param list Source bytes.
			 */
			void assign(std::initializer_list<std::byte> list);

			/**
			 * @brief Replace contents with the range @c [first, last).
			 * @tparam InputIt Input iterator whose value converts to @c std::byte.
			 * @param first Start of the source range.
			 * @param last End of the source range.
			 */
			template<typename InputIt>
			void assign(InputIt first, InputIt last);

			/**
			 * @brief Append a copy of @p bytes.
			 * @param bytes Source view.
			 */
			void append(std::span<const std::byte> bytes);

			/**
			 * @brief Append @p count bytes starting at @p bytes.
			 * @param bytes Source pointer; may be null when @p count is zero.
			 * @param count Byte count.
			 */
			void append(const std::byte* bytes, const StormByte::ByteSize& count);

			/**
			 * @brief Append a copy of @p other.
			 * @param other Source sequence.
			 */
			void append(const BinaryData& other);

			/**
			 * @brief Append @p other, then empty it.
			 * @param other Source sequence. Empty afterwards when it is not @c *this.
			 * @note Same-heap move when @c *this is empty.
			 */
			void append(BinaryData&& other);

			/**
			 * @brief Append a copy of @p bytes.
			 * @param bytes Source view.
			 * @return @c *this.
			 */
			BinaryData& operator+=(std::span<const std::byte> bytes);

			/**
			 * @brief Append a copy of @p other.
			 * @param other Source sequence.
			 * @return @c *this.
			 */
			BinaryData& operator+=(const BinaryData& other);

			/**
			 * @brief Append @p other, then empty it.
			 * @param other Source sequence.
			 * @return @c *this.
			 */
			BinaryData& operator+=(BinaryData&& other);

			/**
			 * @brief Append one byte.
			 * @param value Byte to append.
			 */
			void push_back(std::byte value);

			/**
			 * @brief Append a byte constructed in place.
			 * @tparam T Value convertible to the integer stored in @c std::byte.
			 * @param value Argument forwarded into @c std::byte.
			 * @return Reference to the appended byte.
			 */
			template<typename T>
			reference emplace_back(T&& value);

			/**
			 * @brief Remove the last byte.
			 */
			void pop_back();

			/**
			 * @brief Insert one byte before @p pos.
			 * @param pos Insertion point.
			 * @param value Byte to insert.
			 * @return Iterator to the inserted byte.
			 */
			iterator insert(const_iterator pos, std::byte value);

			/**
			 * @brief Insert @p count copies of @p value before @p pos.
			 * @param pos Insertion point.
			 * @param count Number of bytes.
			 * @param value Fill byte.
			 * @return Iterator to the first inserted byte, or @p pos when @p count is zero.
			 */
			iterator insert(const_iterator pos, const StormByte::ByteSize& count, std::byte value);

			/**
			 * @brief Insert an initializer list before @p pos.
			 * @param pos Insertion point.
			 * @param list Source bytes.
			 * @return Iterator to the first inserted byte, or @p pos when @p list is empty.
			 */
			iterator insert(const_iterator pos, std::initializer_list<std::byte> list);

			/**
			 * @brief Insert a span before @p pos.
			 * @param pos Insertion point.
			 * @param bytes Source view.
			 * @return Iterator to the first inserted byte, or @p pos when @p bytes is empty.
			 */
			iterator insert(const_iterator pos, std::span<const std::byte> bytes);

			/**
			 * @brief Insert the range @c [first, last) before @p pos.
			 * @tparam InputIt Input iterator whose value converts to @c std::byte.
			 * @param pos Insertion point.
			 * @param first Start of the source range.
			 * @param last End of the source range.
			 * @return Iterator to the first inserted byte, or @p pos when the range is empty.
			 */
			template<typename InputIt>
			iterator insert(const_iterator pos, InputIt first, InputIt last);

			/**
			 * @brief Erase the byte at @p pos.
			 * @param pos Byte to erase.
			 * @return Iterator following the erased byte.
			 */
			iterator erase(const_iterator pos);

			/**
			 * @brief Erase @c [first, last).
			 * @param first Start of the range.
			 * @param last End of the range.
			 * @return Iterator following the last erased byte.
			 */
			iterator erase(const_iterator first, const_iterator last);

			/**
			 * @brief Exchange storage with @p other.
			 * @param other Other sequence.
			 */
			void swap(BinaryData& other) noexcept;

			/**
			 * @brief Hexadecimal dump of the occupied bytes, sixteen bytes per row.
			 *
			 * Same as @ref HexDump() with 16 columns.
			 *
			 * @return Dump text owned by Base. Empty when this sequence is empty.
			 */
			Safe::String HexDump() const;

			/**
			 * @brief Hexadecimal dump of the occupied bytes.
			 *
			 * Each line is an 8-digit offset, @p columns bytes as hex, and the
			 * same bytes as ASCII (non-printable shown as @c '.').
			 * @p columns is a row width, not a byte length: it is @ref Size,
			 * not @ref ByteSize.
			 * @c 0 prints every byte on one line (still with offset and ASCII).
			 *
			 * @param columns Bytes per row; @c 0 means a single row.
			 * @return Dump text owned by Base. Empty when this sequence is empty.
			 */
			Safe::String HexDump(Size columns) const;

		private:
			/**
			 * @brief Heap block owned by Base. Hidden from Doxygen.
			 * @cond
			 */
			struct Storage;
			/** @endcond */

			/**
			 * @brief Opaque pointer to @ref Storage.
			 */
			std::unique_ptr<Storage> m_storage;
	};

	/**
	 * @brief Exchange two sequences.
	 * @param lhs First sequence.
	 * @param rhs Second sequence.
	 */
	STORMBYTE_PUBLIC void swap(BinaryData& lhs, BinaryData& rhs) noexcept;

	/**
	 * @brief Equality of a span against a sequence.
	 * @param bytes Left view.
	 * @param data Right sequence.
	 * @return Same as @c data == bytes.
	 */
	inline bool operator==(std::span<const std::byte> bytes, const BinaryData& data) noexcept {
		return data == bytes;
	}

	/**
	 * @brief Inequality of a span against a sequence.
	 * @param bytes Left view.
	 * @param data Right sequence.
	 * @return Same as @c data != bytes.
	 */
	inline bool operator!=(std::span<const std::byte> bytes, const BinaryData& data) noexcept {
		return data != bytes;
	}

	/**
	 * @brief Three-way comparison of a span against a sequence.
	 * @param bytes Left view.
	 * @param data Right sequence.
	 * @return Reverse of @c data <=> bytes.
	 */
	inline std::strong_ordering operator<=>(std::span<const std::byte> bytes, const BinaryData& data) noexcept {
		return 0 <=> (data <=> bytes);
	}

	// Out-of-line template members. Documentation lives on the declarations.
	template<Type::ByteInputRange R>
	BinaryData::BinaryData(const R& range)
		: BinaryData() {
		if constexpr (requires { std::size(range); })
			reserve(StormByte::ByteSize{std::size(range)});
		for (auto&& b : range)
			push_back(static_cast<std::byte>(b));
	}

	template<Type::ByteInputRange R>
	BinaryData::BinaryData(R&& range)
		: BinaryData() {
		using U = std::remove_cvref_t<R>;
		if constexpr (Type::SameAs<U, BinaryData> && !Type::LvalueReference<R>) {
			*this = std::move(range);
		} else {
			if constexpr (requires { std::size(range); })
				reserve(StormByte::ByteSize{std::size(range)});
			for (auto&& b : range)
				push_back(static_cast<std::byte>(b));
		}
	}

	template<typename InputIt>
	void BinaryData::assign(InputIt first, InputIt last) {
		clear();
		for (; first != last; ++first)
			push_back(static_cast<std::byte>(*first));
	}

	template<typename InputIt>
	BinaryData::iterator BinaryData::insert(const_iterator pos, InputIt first, InputIt last) {
		const auto start = static_cast<std::size_t>(pos - cbegin());
		std::size_t n = start;
		for (; first != last; ++first) {
			insert(cbegin() + static_cast<difference_type>(n), static_cast<std::byte>(*first));
			++n;
		}
		return begin() + static_cast<difference_type>(start);
	}

	template<typename T>
	BinaryData::reference BinaryData::emplace_back(T&& value) {
		push_back(std::byte{ static_cast<unsigned char>(std::forward<T>(value)) });
		return back();
	}
}
