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

#pragma once

#include <StormByte/byte_size.hxx>
#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits.hxx>

#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Owned values that cross a DLL without the caller's CRT.
	 */
	namespace Safe {
		/**
		 * @class Binary
		 * @brief Owned contiguous sequence of @c std::byte, stored in a @ref Vector.
		 *
		 * There is no separate backend. Bytes live in @ref Vector of @c std::byte, allocated with @ref Heap::Allocate. Names match @c std::vector. Length, capacity and offsets are @ref ByteSize. A hex-dump column count is @ref Size.
		 *
		 * A conversion to @c std::vector copies into the caller. It does not steal the Base pointer. An rvalue overload empties this object after the copy.
		 */
		class STORMBYTE_PUBLIC Binary final {
			public:
				using value_type = std::byte; ///< Element type.
				using size_type = std::size_t; ///< STL size typedef. @ref size() returns @ref ByteSize.
				using difference_type = std::ptrdiff_t; ///< Iterator difference.
				using reference = std::byte&; ///< Mutable element reference.
				using const_reference = const std::byte&; ///< Read-only element reference.
				using pointer = std::byte*; ///< Mutable pointer.
				using const_pointer = const std::byte*; ///< Read-only pointer.
				using iterator = std::byte*; ///< Contiguous mutable iterator.
				using const_iterator = const std::byte*; ///< Contiguous read-only iterator.
				using reverse_iterator = std::reverse_iterator<iterator>; ///< Reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.

				/**
				 * @brief Construct an empty sequence.
				 */
				Binary() noexcept;

				/**
				 * @brief Construct @p count bytes filled with @p value.
				 * @param count Element count.
				 * @param value Fill byte.
				 */
				Binary(const ByteSize& count, std::byte value);

				/**
				 * @brief Construct @p count zeroed bytes.
				 * @param count Element count.
				 */
				explicit Binary(const ByteSize& count);

				/**
				 * @brief Copy bytes from a contiguous span.
				 * @param bytes Source view. Not owned.
				 */
				explicit Binary(std::span<const std::byte> bytes);

				/**
				 * @brief Copy @p count bytes starting at @p bytes.
				 * @param bytes Source pointer. May be null when @p count is zero.
				 * @param count Byte count.
				 * @throws OutOfBoundsError @p bytes is null and @p count is not zero.
				 */
				Binary(const std::byte* bytes, const ByteSize& count);

				/**
				 * @brief Copy from an initializer list.
				 * @param list Source bytes.
				 */
				Binary(std::initializer_list<std::byte> list);

				/**
				 * @brief Copy character bytes from a string view. No trailing NUL.
				 * @param text Source characters.
				 */
				explicit Binary(std::string_view text);

				/**
				 * @brief Copy character bytes from a C string. No trailing NUL.
				 * @param text Source. A null pointer yields an empty sequence.
				 */
				explicit Binary(const char* text);

				/**
				 * @brief Copy bytes from a caller-owned vector. The vector is unchanged.
				 * @param bytes Source.
				 */
				STORMBYTE_FORCE_INLINE explicit Binary(const std::vector<std::byte>& bytes): Binary(std::span<const std::byte>(bytes.data(), bytes.size())) {}

				/**
				 * @brief Copy bytes from a caller-owned vector, then empty it.
				 * @param bytes Source. Cleared after the copy. Not a heap steal.
				 */
				STORMBYTE_FORCE_INLINE explicit Binary(std::vector<std::byte>&& bytes): Binary(std::span<const std::byte>(bytes.data(), bytes.size())) {
					bytes.clear();
					bytes.shrink_to_fit();
				}

				/**
				 * @brief Copy from an input range of byte-convertible values.
				 * @tparam R Range type satisfying @ref Type::ByteInputRange.
				 * @param range Source range.
				 */
				template<Type::ByteInputRange R>
				explicit Binary(const R& range);

				/**
				 * @brief Consume an rvalue range. Moves when @p R is an rvalue @ref Binary.
				 * @tparam R Range type satisfying @ref Type::ByteInputRange.
				 * @param range Source range.
				 */
				template<Type::ByteInputRange R>
				explicit Binary(R&& range);

				/**
				 * @brief Copy construct.
				 * @param other Source sequence.
				 */
				Binary(const Binary& other);

				/**
				 * @brief Move construct. @p other is left empty.
				 * @param other Source sequence.
				 */
				Binary(Binary&& other) noexcept;

				/**
				 * @brief Destroy the sequence on Base's heap.
				 */
				~Binary() noexcept;

				/**
				 * @brief Copy assign.
				 * @param other Source sequence.
				 * @return This sequence.
				 */
				Binary& operator=(const Binary& other);

				/**
				 * @brief Move assign. @p other is left empty.
				 * @param other Source sequence.
				 * @return This sequence.
				 */
				Binary& operator=(Binary&& other) noexcept;

				/**
				 * @brief Replace contents with an initializer list.
				 * @param list Source bytes.
				 * @return This sequence.
				 */
				Binary& operator=(std::initializer_list<std::byte> list);

				/**
				 * @brief Equality of byte contents.
				 * @param other Other sequence.
				 * @return Whether sizes and bytes match.
				 */
				bool operator==(const Binary& other) const noexcept;

				/**
				 * @brief Inequality of byte contents.
				 * @param other Other sequence.
				 * @return Whether the sequences differ.
				 */
				bool operator!=(const Binary& other) const noexcept;

				/**
				 * @brief Three-way lexicographical comparison.
				 * @param other Other sequence.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const Binary& other) const noexcept;

				/**
				 * @brief Equality with a byte span.
				 * @param bytes View to compare.
				 * @return Whether sizes and bytes match.
				 */
				bool operator==(std::span<const std::byte> bytes) const noexcept;

				/**
				 * @brief Inequality with a byte span.
				 * @param bytes View to compare.
				 * @return Whether the sequence differs from the span.
				 */
				bool operator!=(std::span<const std::byte> bytes) const noexcept;

				/**
				 * @brief Three-way comparison with a byte span.
				 * @param bytes View to compare.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(std::span<const std::byte> bytes) const noexcept;

				/**
				 * @brief Mutable iterator to the first byte.
				 * @return First byte, or a valid empty iterator.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Read-only iterator to the first byte.
				 * @return First byte, or a valid empty iterator.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Mutable iterator one past the last byte.
				 * @return Past-the-end iterator.
				 */
				iterator end() noexcept;

				/**
				 * @brief Read-only iterator one past the last byte.
				 * @return Past-the-end iterator.
				 */
				const_iterator end() const noexcept;

				/**
				 * @brief Read-only iterator to the first byte.
				 * @return Same as const @ref begin().
				 */
				const_iterator cbegin() const noexcept;

				/**
				 * @brief Read-only iterator one past the last byte.
				 * @return Same as const @ref end().
				 */
				const_iterator cend() const noexcept;

				/**
				 * @brief Mutable reverse iterator to the last byte.
				 * @return Reverse begin.
				 */
				reverse_iterator rbegin() noexcept;

				/**
				 * @brief Mutable reverse iterator before the first byte.
				 * @return Reverse end.
				 */
				reverse_iterator rend() noexcept;

				/**
				 * @brief Read-only reverse iterator to the last byte.
				 * @return Reverse begin.
				 */
				const_reverse_iterator rbegin() const noexcept;

				/**
				 * @brief Read-only reverse iterator before the first byte.
				 * @return Reverse end.
				 */
				const_reverse_iterator rend() const noexcept;

				/**
				 * @brief Read-only reverse iterator to the last byte.
				 * @return Same as const @ref rbegin().
				 */
				const_reverse_iterator crbegin() const noexcept;

				/**
				 * @brief Read-only reverse iterator before the first byte.
				 * @return Same as const @ref rend().
				 */
				const_reverse_iterator crend() const noexcept;

				/**
				 * @brief Occupied length in bytes.
				 * @return Length.
				 */
				ByteSize size() const noexcept;

				/**
				 * @brief Implementation maximum size in bytes.
				 * @return Maximum size.
				 */
				ByteSize max_size() const noexcept;

				/**
				 * @brief Allocated capacity in bytes.
				 * @return Capacity.
				 */
				ByteSize capacity() const noexcept;

				/**
				 * @brief Whether the sequence holds no bytes.
				 * @return Whether @ref size() is zero.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Request capacity of at least @p new_cap bytes.
				 * @param new_cap Requested capacity.
				 */
				void reserve(const ByteSize& new_cap);

				/**
				 * @brief Resize to @p new_size bytes. Appended bytes are zero.
				 * @param new_size New size.
				 */
				void resize(const ByteSize& new_size);

				/**
				 * @brief Resize to @p new_size bytes. Appended bytes are @p value.
				 * @param new_size New size.
				 * @param value Fill for new bytes.
				 */
				void resize(const ByteSize& new_size, std::byte value);

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
				 * @return Byte at @p index.
				 */
				std::byte& operator[](const ByteSize& index) noexcept;

				/**
				 * @brief Unchecked read-only subscript.
				 * @param index Byte offset.
				 * @return Byte at @p index.
				 */
				const std::byte& operator[](const ByteSize& index) const noexcept;

				/**
				 * @brief Checked mutable subscript.
				 * @param index Byte offset.
				 * @return Byte at @p index.
				 * @throws OutOfBoundsError @p index is not less than @ref size().
				 */
				std::byte& at(const ByteSize& index);

				/**
				 * @brief Checked read-only subscript.
				 * @param index Byte offset.
				 * @return Byte at @p index.
				 * @throws OutOfBoundsError @p index is not less than @ref size().
				 */
				const std::byte& at(const ByteSize& index) const;

				/**
				 * @brief First byte.
				 * @return First byte.
				 */
				std::byte& front();

				/**
				 * @brief First byte.
				 * @return First byte.
				 */
				const std::byte& front() const;

				/**
				 * @brief Last byte.
				 * @return Last byte.
				 */
				std::byte& back();

				/**
				 * @brief Last byte.
				 * @return Last byte.
				 */
				const std::byte& back() const;

				/**
				 * @brief Mutable pointer to the first byte, or null when empty.
				 * @return Contiguous storage.
				 */
				std::byte* data() noexcept;

				/**
				 * @brief Read-only pointer to the first byte, or null when empty.
				 * @return Contiguous storage.
				 */
				const std::byte* data() const noexcept;

				/**
				 * @brief Mutable view of the occupied bytes.
				 * @return Span over the occupied bytes.
				 */
				std::span<std::byte> span() noexcept;

				/**
				 * @brief Read-only view of the occupied bytes.
				 * @return Span over the occupied bytes.
				 */
				std::span<const std::byte> span() const noexcept;

				/**
				 * @brief Implicit mutable span conversion.
				 * @return Same as @ref span().
				 */
				operator std::span<std::byte>() noexcept;

				/**
				 * @brief Implicit read-only span conversion.
				 * @return Same as const @ref span().
				 */
				operator std::span<const std::byte>() const noexcept;

				/**
				 * @brief Copy bytes onto the caller CRT as a vector. This object is unchanged.
				 * @return Vector owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::vector<std::byte>() const& {
					const std::span<const std::byte> bytes = span();
					return std::vector<std::byte>(bytes.begin(), bytes.end());
				}

				/**
				 * @brief Copy bytes onto the caller CRT, then release this object's storage.
				 * @return Vector owned by the caller. This object is empty afterwards.
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
				void assign(const ByteSize& count, std::byte value);

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
				 * @brief Replace contents with the range `[first, last)`.
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
				 * @param bytes Source pointer. May be null when @p count is zero.
				 * @param count Byte count.
				 * @throws OutOfBoundsError @p bytes is null and @p count is not zero.
				 */
				void append(const std::byte* bytes, const ByteSize& count);

				/**
				 * @brief Append a copy of @p other.
				 * @param other Source sequence.
				 */
				void append(const Binary& other);

				/**
				 * @brief Append @p other, then empty it.
				 * @param other Source sequence. Empty afterwards when it is not this object.
				 */
				void append(Binary&& other);

				/**
				 * @brief Append a copy of @p bytes.
				 * @param bytes Source view.
				 * @return This sequence.
				 */
				Binary& operator+=(std::span<const std::byte> bytes);

				/**
				 * @brief Append a copy of @p other.
				 * @param other Source sequence.
				 * @return This sequence.
				 */
				Binary& operator+=(const Binary& other);

				/**
				 * @brief Append @p other, then empty it.
				 * @param other Source sequence.
				 * @return This sequence.
				 */
				Binary& operator+=(Binary&& other);

				/**
				 * @brief Append one byte.
				 * @param value Byte to append.
				 */
				void push_back(std::byte value);

				/**
				 * @brief Append a byte constructed in place.
				 * @tparam T Value convertible to @c std::byte.
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
				iterator insert(const_iterator pos, const ByteSize& count, std::byte value);

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
				 * @brief Insert the range `[first, last)` before @p pos.
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
				 * @brief Erase `[first, last)`.
				 * @param first Start of the range.
				 * @param last End of the range.
				 * @return Iterator following the last erased byte.
				 */
				iterator erase(const_iterator first, const_iterator last);

				/**
				 * @brief Exchange storage with @p other.
				 * @param other Other sequence.
				 */
				void swap(Binary& other) noexcept;

				/**
				 * @brief Hexadecimal dump, sixteen bytes per row.
				 * @return Dump text owned by Base. Empty when this sequence is empty.
				 */
				String HexDump() const;

				/**
				 * @brief Hexadecimal dump of the occupied bytes.
				 * @param columns Bytes per row. Zero means a single row. This is a count, not a @ref ByteSize.
				 * @return Dump text owned by Base. Empty when this sequence is empty.
				 */
				String HexDump(Size columns) const;

			private:
				Vector<std::byte> m_bytes; ///< Owned bytes.
		};

		/**
		 * @brief Exchange two sequences.
		 * @param left First sequence.
		 * @param right Second sequence.
		 */
		inline void swap(Binary& left, Binary& right) noexcept {
			left.swap(right);
		}

		/**
		 * @brief Equality of a span against a sequence.
		 * @param bytes Left view.
		 * @param data Right sequence.
		 * @return Same as @c data == bytes.
		 */
		inline bool operator==(std::span<const std::byte> bytes, const Binary& data) noexcept {
			return data == bytes;
		}

		/**
		 * @brief Inequality of a span against a sequence.
		 * @param bytes Left view.
		 * @param data Right sequence.
		 * @return Same as @c data != bytes.
		 */
		inline bool operator!=(std::span<const std::byte> bytes, const Binary& data) noexcept {
			return data != bytes;
		}

		/**
		 * @brief Three-way comparison of a span against a sequence.
		 * @param bytes Left view.
		 * @param data Right sequence.
		 * @return Reverse of @c data <=> bytes.
		 */
		inline std::strong_ordering operator<=>(std::span<const std::byte> bytes, const Binary& data) noexcept {
			return 0 <=> (data <=> bytes);
		}

		template<Type::ByteInputRange R>
		Binary::Binary(const R& range): Binary() {
			if constexpr (requires { std::size(range); })
				reserve(ByteSize{std::size(range)});
			for (auto&& byte : range)
				push_back(static_cast<std::byte>(byte));
		}

		template<Type::ByteInputRange R>
		Binary::Binary(R&& range): Binary() {
			using U = std::remove_cvref_t<R>;
			if constexpr (Type::SameAs<U, Binary> && !Type::LvalueReference<R>) {
				*this = std::move(range);
			} else {
				if constexpr (requires { std::size(range); })
					reserve(ByteSize{std::size(range)});
				for (auto&& byte : range)
					push_back(static_cast<std::byte>(byte));
			}
		}

		template<typename InputIt>
		void Binary::assign(InputIt first, InputIt last) {
			clear();
			for (; first != last; ++first)
				push_back(static_cast<std::byte>(*first));
		}

		template<typename T>
		Binary::reference Binary::emplace_back(T&& value) {
			push_back(static_cast<std::byte>(std::forward<T>(value)));
			return back();
		}

		template<typename InputIt>
		Binary::iterator Binary::insert(const_iterator pos, InputIt first, InputIt last) {
			Vector<std::byte> copied;
			for (; first != last; ++first)
				copied.push_back(static_cast<std::byte>(*first));
			return insert(pos, std::span<const std::byte>(copied.data(), static_cast<std::size_t>(copied.size())));
		}
	}
}

/**
 * @brief Cross-module hash of the byte sequence. Not `std::hash`.
 */
template<>
struct StormByte::Safe::Hash<StormByte::Safe::Binary> {
	/**
	 * @brief Hashes @p value.
	 * @param value Sequence. Not copied. Embedded content counts.
	 * @return FNV-1a of the occupied bytes. The same value in every module.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::Safe::Binary& value) const noexcept {
		return Safe::HashBytes(value.span());
	}
};

/**
 * @brief Hash of the byte sequence. An empty sequence hashes as an empty view.
 */
template<>
struct std::hash<StormByte::Safe::Binary> {
	/**
	 * @brief Hashes @p value.
	 * @param value Sequence. Not copied.
	 * @return Hash of this module's `std::hash<std::string_view>` over the bytes.
	 */
	std::size_t operator()(const StormByte::Safe::Binary& value) const noexcept {
		return StormByte::Safe::Hash<StormByte::Safe::Binary>{}(value);
	}
};
