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

#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	class Size;

	/**
	 * @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
	 */
	namespace Safe {
		class WCString;

		/**
		 * @class CString
		 * @brief Owned NUL-terminated buffer, safe to use across a DLL boundary.
		 *
		 * This is not a replacement or reimplementation of `std::string`.
		 * It provides owned NUL-terminated storage, STL-shaped mutable contiguous
		 * ranges over existing characters, observers, comparisons and conversions.
		 *
		 * `operator const char*` is the analogue of `std::string::c_str()`.
		 * The pointer is valid only until this object is destroyed, moved
		 * from, assigned or @ref Reset. Using it afterwards is use-after-free.
		 *
		 * `operator std::string_view` is explicit and follows the same
		 * lifetime. A null buffer yields an empty view. The view covers
		 * `[0, Length())` and does not include the trailing NUL.
		 *
		 * `operator bool` is true when the pointer is not null. A buffer
		 * constructed from `""` is empty (`Length() == 0`) and valid.
		 * A default-constructed object is null.
		 *
		 * Construction from `std::string` / `std::string_view` *copies* onto
		 * Base's heap. It is not a heap steal. An empty source yields `""`,
		 * not a null buffer.
		 *
		 * Mutable `operator[]` and `data()` may change characters in `[0, Length())`.
		 * `Length()` is the trailing NUL; reading that position is valid, but writing
		 * any value except `char{}` there is undefined, as with `std::string`.
		 * A null buffer or an index past `Length()` is undefined and asserts when enabled.
		 *
		 * Equality and `<=>` compare text, not addresses. Two nulls are
		 * equal. Null is not equal to `""`. Null orders before any text.
		 *
		 * `operator std::string` and `operator<<` are `STORMBYTE_FORCE_INLINE`
		 * so the caller CRT owns the string and the stream buffer. `inline` on an
		 * exported class can still be a call into this DLL.
		 *
		 * If the text never leaves the module that created it, or the
		 * program is not built for Windows, use `std::string`.
		 */
		class STORMBYTE_PUBLIC CString final {
			public:
				using value_type = char; ///< Character type.
				using size_type = Size; ///< Character count type.
				using iterator = char*; ///< Mutable contiguous iterator.
				using const_iterator = const char*; ///< Read-only contiguous iterator.
				using reverse_iterator = std::reverse_iterator<iterator>; ///< Mutable reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.

				/**
				 * @name Life
				 * @{
				 */

				/**
				 * @brief Empty (null) buffer.
				 */
				CString() noexcept;

				/**
				 * @brief Copies @p str.
				 * @param str Source; may be null.
				 */
				explicit CString(const char* str) noexcept;

				/**
				 * @brief Copies @p sv onto Base's heap.
				 * @param sv Source view. Copied up to the first NUL, then terminated.
				 * @note Not a heap steal. Empty yields @c "".
				 */
				explicit CString(std::string_view sv) noexcept;

				/**
				 * @brief Copies @p str onto Base's heap.
				 * @param str Source. Remains valid and unchanged.
				 * @note Not a heap steal. Empty yields @c "".
				 */
				explicit CString(const std::string& str) noexcept;

				/**
				 * @brief Copies @p text as UTF-8.
				 * @param text Wide text. Null stays null.
				 */
				explicit CString(const WCString& text) noexcept;

				/**
				 * @brief Copy constructor.
				 * @param other Buffer to copy.
				 */
				CString(const CString& other) noexcept;

				/**
				 * @brief Move constructor.
				 * @param other Buffer to take. @p other becomes null.
				 */
				CString(CString&& other) noexcept;

				/**
				 * @brief Releases the buffer.
				 */
				~CString() noexcept;

				/**
				 * @brief Copy assignment.
				 * @param other Buffer to copy.
				 * @return *this.
				 */
				CString& operator=(const CString& other) noexcept;

				/**
				 * @brief Move assignment.
				 * @param other Buffer to take. @p other becomes null.
				 * @return *this.
				 */
				CString& operator=(CString&& other) noexcept;

				/** @} */

				/**
				 * @name Modifiers
				 * @{
				 */

				/**
				 * @brief Replaces the buffer with a copy of @p str.
				 * @param str Source; may be null.
				 */
				void Reset(const char* str = nullptr) noexcept;

				/**
				 * @brief Swaps buffers with @p other.
				 * @param other Other buffer.
				 */
				void swap(CString& other) noexcept;

				/** @} */

				/**
				 * @name Observers
				 * @{
				 */

				/**
				 * @brief Character count (`strlen`), or `0` when empty or null.
				 * @return Length as @ref StormByte::Size (units, not bytes).
				 */
				Size Length() const noexcept;

				/**
				 * @brief Return the character count.
				 * @return Number of characters.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Test whether the buffer is empty.
				 * @return Whether the character count is zero.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the first mutable character iterator, or null for a null buffer.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept { return data(); }

				/**
				 * @brief Return the end mutable iterator, or null for a null buffer.
				 * @return Mutable iterator.
				 */
				iterator end() noexcept {
					char* text = data();
				return text ? text + std::string_view(text).size() : nullptr;
				}

				/**
				 * @brief Return the first read-only character iterator, or null for a null buffer.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept { return static_cast<const char*>(*this); }

				/**
				 * @brief Return the end read-only iterator, or null for a null buffer.
				 * @return Read-only iterator.
				 */
				const_iterator end() const noexcept {
					const char* text = begin();
					return text ? text + std::string_view(text).size() : nullptr;
				}

				/**
				 * @brief Return the first read-only character iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Return the mutable reverse begin iterator.
				 * @return Reverse iterator to the last character.
				 */
				reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

				/**
				 * @brief Return the mutable reverse end iterator.
				 * @return Reverse end iterator.
				 */
				reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

				/**
				 * @brief Return the read-only reverse begin iterator.
				 * @return Reverse iterator to the last character.
				 */
				const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

				/**
				 * @brief Return the read-only reverse begin iterator.
				 * @return Reverse iterator to the last character.
				 */
				const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

				/**
				 * @brief Return the mutable buffer pointer, or null.
				 * @return Mutable pointer to the first character.
				 */
				char* data() noexcept { return m_data; }

				/**
				 * @brief Return the read-only buffer pointer, or null.
				 * @return Read-only pointer to the first character.
				 */
				const char* data() const noexcept { return m_data; }

				/**
				 * @brief Mutable character at @p index.
				 * @param index Position in `[0, Length()]`. `Length()` is the trailing NUL.
				 * @return Mutable character reference.
				 * @note At `Length()`, only assigning `char{}` is valid. Null or past-length access is undefined.
				 */
				char& operator[](const Size& index) noexcept;

				/**
				 * @brief Read-only character at @p index.
				 * @param index Position in `[0, Length()]`. `Length()` is the trailing NUL.
				 * @return Character copy.
				 * @note Null or `index > Length()` is undefined. Checked with `assert` when assertions are on.
				 */
				char operator[](const Size& index) const noexcept;

				/**
				 * @brief `true` when the buffer pointer is not null.
				 * @note `""` is valid and empty. A default object is null.
				 * @return Whether a buffer is held.
				 */
				inline explicit operator bool() const noexcept {
					return static_cast<const char*>(*this) != nullptr;
				}

				/** @} */

				/**
				 * @name Conversions
				 * @{
				 */

				/**
				 * @brief View of the owned buffer.
				 * @return Buffer, or null.
				 * @note Same lifetime rules as `std::string::c_str()`.
				 */
				explicit operator const char*() const noexcept;

				/**
				 * @brief Non-owning view of the text.
				 * @return Empty view when the buffer is null.
				 * @note Same lifetime rules as `std::string::c_str()`.
				 */
				inline explicit operator std::string_view() const noexcept {
					const char* text = static_cast<const char*>(*this);
					return text ? std::string_view(text) : std::string_view();
				}

				/**
				 * @brief Copy of the text in the caller’s heap.
				 * @return Empty string when the buffer is null.
				 */
				STORMBYTE_FORCE_INLINE operator std::string() const {
					const char* text = static_cast<const char*>(*this);
					return text ? std::string(text) : std::string();
				}

				/**
				 * @brief Writes the text to @p stream.
				 * @param stream Destination.
				 * @return @p stream.
				 */
				STORMBYTE_FORCE_INLINE std::ostream& operator<<(std::ostream& stream) const {
					const char* text = static_cast<const char*>(*this);
					if (text)
						stream << text;
					return stream;
				}

				/** @} */

				/**
				 * @name Comparison
				 * @{
				 */

				/**
				 * @brief Content equality.
				 * @param other Other buffer.
				 * @return Whether the texts are equal.
				 */
				bool operator==(const CString& other) const noexcept;

				/**
				 * @brief Content inequality.
				 * @param other Other buffer.
				 * @return Whether the texts differ.
				 */
				bool operator!=(const CString& other) const noexcept {
					return !(*this == other);
				}

				/**
				 * @brief Content equality with a C string.
				 * @param str May be null (treated as a null @ref CString).
				 * @return Whether the texts are equal.
				 */
				bool operator==(const char* str) const noexcept;

				/**
				 * @brief Content inequality with a C string.
				 * @param str May be null.
				 * @return Whether the texts differ.
				 */
				bool operator!=(const char* str) const noexcept {
					return !(*this == str);
				}

				/**
				 * @brief Content order. Null is less than any text.
				 * @param other Other buffer.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const CString& other) const noexcept;

				/**
				 * @brief Content order against a C string.
				 * @param str May be null.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const char* str) const noexcept;

				/** @} */

			private:
				char* m_data;	///< Owned buffer

				/**
				 * @brief Copies @p str into a new buffer.
				 * @param str Source; may be null.
				 * @return New buffer, or null.
				 */
				static char* Duplicate(const char* str) noexcept;

				/**
				 * @brief Copies @p sv into a new buffer and appends NUL.
				 * @param sv Source view.
				 * @return New buffer (`""` when @p sv is empty).
				 */
				static char* Duplicate(std::string_view sv) noexcept;
		};

		/**
		 * @brief Writes @p text to @p stream.
		 * @param stream Destination.
		 * @param text Source.
		 * @return @p stream.
		 */
		inline std::ostream& operator<<(std::ostream& stream, const CString& text) {
			return text.operator<<(stream);
		}

		/**
		 * @brief Content equality.
		 * @param str C string; may be null.
		 * @param text Buffer.
		 * @return Whether the texts are equal.
		 */
		inline bool operator==(const char* str, const CString& text) noexcept {
			return text == str;
		}

		/**
		 * @brief Content inequality.
		 * @param str C string; may be null.
		 * @param text Buffer.
		 * @return Whether the texts differ.
		 */
		inline bool operator!=(const char* str, const CString& text) noexcept {
			return text != str;
		}

		/**
		 * @brief Swaps two buffers.
		 * @param left First buffer.
		 * @param right Second buffer.
		 */
		inline void swap(CString& left, CString& right) noexcept {
			left.swap(right);
		}
	}
}

#include <StormByte/size.hxx>

/**
 * @brief Hash of the text (`0` when the buffer is null).
 */
template<>
struct std::hash<StormByte::Safe::CString> {
	/**
	 * @brief Hashes @p text.
	 * @param text Buffer.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::CString& text) const noexcept {
		const char* raw = static_cast<const char*>(text);
		if (!raw)
			return 0;
		return std::hash<std::string_view>{}(raw);
	}
};
