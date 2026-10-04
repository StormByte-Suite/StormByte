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
#include <cwchar>
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
		class CString;

		/**
		 * @class WCString
		 * @brief Owned NUL-terminated wide buffer, safe to use across a DLL boundary.
		 *
		 * This is not a replacement or reimplementation of `std::wstring`.
		 * It provides owned NUL-terminated storage, STL-shaped mutable contiguous
		 * ranges over existing code units, observers, comparisons and conversions.
		 *
		 * `operator const wchar_t*` is the analogue of `std::wstring::c_str()`.
		 * The pointer is valid only until this object is destroyed, moved
		 * from, assigned or @ref Reset. Using it afterwards is use-after-free.
		 *
		 * `operator std::wstring_view` is explicit and follows the same
		 * lifetime. A null buffer yields an empty view. The view covers
		 * `[0, Length())` and does not include the trailing NUL.
		 *
		 * `operator bool` is true when the pointer is not null. A buffer
		 * constructed from `L""` is empty (`Length() == 0`) and valid.
		 * A default-constructed object is null.
		 *
		 * Construction from `std::wstring` / `std::wstring_view` *copies* onto
		 * Base's heap. It is not a heap steal. An empty source yields `L""`,
		 * not a null buffer.
		 *
		 * `capacity()` counts allocated wchar_t code units excluding the trailing
		 * NUL. `reserve(n)` grows only when `n > capacity()` and never shrinks for
		 * a smaller request. `Reset` reuses reserved storage when the text fits.
		 *
		 * Mutable `operator[]` and `data()` may change code units in `[0, Length())`.
		 * `Length()` is the trailing NUL; reading that position is valid, but writing
		 * any value except `wchar_t{}` there is undefined, as with `std::wstring`.
		 * A null buffer or an index past `Length()` is undefined and asserts when enabled.
		 *
		 * Equality and `<=>` compare text, not addresses. Two nulls are
		 * equal. Null is not equal to `L""`. Null orders before any text.
		 *
		 * `operator std::wstring` and `operator<<` are `STORMBYTE_FORCE_INLINE`
		 * so the caller CRT owns the string and the stream buffer. `inline` on an
		 * exported class can still be a call into this DLL.
		 *
		 * If the text never leaves the module that created it, or the
		 * program is not built for Windows, use `std::wstring`.
		 */
		class STORMBYTE_PUBLIC WCString final {
			public:
				using value_type = wchar_t; ///< Wide character type.
				using size_type = Size; ///< Code-unit count type.
				using iterator = wchar_t*; ///< Mutable contiguous iterator.
				using const_iterator = const wchar_t*; ///< Read-only contiguous iterator.
				using reverse_iterator = std::reverse_iterator<iterator>; ///< Mutable reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.

				/**
				 * @name Life
				 * @{
				 */

				/**
				 * @brief Empty (null) buffer.
				 */
				WCString() noexcept;

				/**
				 * @brief Copies @p str.
				 * @param str Source; may be null.
				 */
				explicit WCString(const wchar_t* str) noexcept;

				/**
				 * @brief Copies @p sv onto Base's heap.
				 * @param sv Source view. Copied up to the first NUL, then terminated.
				 * @note Not a heap steal. Empty yields @c L"".
				 */
				explicit WCString(std::wstring_view sv) noexcept;

				/**
				 * @brief Copies @p str onto Base's heap.
				 * @param str Source. Remains valid and unchanged.
				 * @note Not a heap steal. Empty yields @c L"".
				 */
				explicit WCString(const std::wstring& str) noexcept;

				/**
				 * @brief Copies @p text as wide characters. Narrow text is UTF-8.
				 * @param text Narrow text. Null stays null.
				 */
				explicit WCString(const CString& text) noexcept;

				/**
				 * @brief Copy constructor.
				 * @param other Buffer to copy.
				 */
				WCString(const WCString& other) noexcept;

				/**
				 * @brief Move constructor.
				 * @param other Buffer to take. @p other becomes null.
				 */
				WCString(WCString&& other) noexcept;

				/**
				 * @brief Releases the buffer.
				 */
				~WCString() noexcept;

				/**
				 * @brief Copy assignment.
				 * @param other Buffer to copy.
				 * @return *this.
				 */
				WCString& operator=(const WCString& other) noexcept;

				/**
				 * @brief Move assignment.
				 * @param other Buffer to take. @p other becomes null.
				 * @return *this.
				 */
				WCString& operator=(WCString&& other) noexcept;

				/** @} */

				/**
				 * @name Modifiers
				 * @{
				 */

				/**
				 * @brief Replaces the buffer with a copy of @p str.
				 * @param str Source; may be null.
				 */
				void Reset(const wchar_t* str = nullptr) noexcept;

				/**
				 * @brief Ensure storage for at least @p new_capacity code units, excluding the NUL.
				 * @param new_capacity Requested code-unit capacity.
				 * @note Requests at or below the current capacity do nothing and never shrink.
				 * @throws StormByte::OutOfBoundsError If the requested allocation size overflows.
				 * @throws StormByte::AllocationError If storage cannot be allocated.
				 */
				void reserve(size_type new_capacity);

				/**
				 * @brief Swaps buffers with @p other.
				 * @param other Other buffer.
				 */
				void swap(WCString& other) noexcept;

				/** @} */

				/**
				 * @name Observers
				 * @{
				 */

				/**
				 * @brief Character count (`wcslen`), or `0` when empty or null.
				 * @return Length as @ref StormByte::Size (units, not bytes).
				 */
				Size Length() const noexcept;

				/**
				 * @brief Return allocated capacity in wchar_t code units, excluding the NUL.
				 * @return Capacity in code units.
				 */
				size_type capacity() const noexcept;

				/**
				 * @brief Return the code-unit count.
				 * @return Number of code units.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Test whether the buffer is empty.
				 * @return Whether the code-unit count is zero.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the first mutable code-unit iterator, or null for a null buffer.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept { return data(); }

				/**
				 * @brief Return the end mutable iterator, or null for a null buffer.
				 * @return Mutable iterator.
				 */
				iterator end() noexcept {
					wchar_t* text = data();
					return text ? text + std::wstring_view(text).size() : nullptr;
				}

				/**
				 * @brief Return the first read-only code-unit iterator, or null for a null buffer.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept { return static_cast<const wchar_t*>(*this); }

				/**
				 * @brief Return the end read-only iterator, or null for a null buffer.
				 * @return Read-only iterator.
				 */
				const_iterator end() const noexcept {
					const wchar_t* text = begin();
					return text ? text + std::wstring_view(text).size() : nullptr;
				}

				/**
				 * @brief Return the first read-only code-unit iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the end read-only code-unit iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Return the mutable reverse begin iterator.
				 * @return Reverse iterator to the last code unit.
				 */
				reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

				/**
				 * @brief Return the mutable reverse end iterator.
				 * @return Reverse end iterator.
				 */
				reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

				/**
				 * @brief Return the read-only reverse begin iterator.
				 * @return Reverse iterator to the last code unit.
				 */
				const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

				/**
				 * @brief Return the read-only reverse begin iterator.
				 * @return Reverse iterator to the last code unit.
				 */
				const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

				/**
				 * @brief Return the mutable buffer pointer, or null.
				 * @return Mutable pointer to the first code unit.
				 */
				wchar_t* data() noexcept { return m_data; }

				/**
				 * @brief Return the read-only buffer pointer, or null.
				 * @return Read-only pointer to the first code unit.
				 */
				const wchar_t* data() const noexcept { return m_data; }

				/**
				 * @brief Mutable code unit at @p index.
				 * @param index Position in `[0, Length()]`. `Length()` is the trailing NUL.
				 * @return Mutable code-unit reference.
				 * @note At `Length()`, only assigning `wchar_t{}` is valid. Null or past-length access is undefined.
				 */
				wchar_t& operator[](const Size& index) noexcept;

				/**
				 * @brief Read-only code unit at @p index.
				 * @param index Position in `[0, Length()]`. `Length()` is the trailing NUL.
				 * @return Code-unit copy.
				 * @note Null or `index > Length()` is undefined. Checked with `assert` when assertions are on.
				 */
				wchar_t operator[](const Size& index) const noexcept;

				/**
				 * @brief `true` when the buffer pointer is not null.
				 * @note `L""` is valid and empty. A default object is null.
				 * @return Whether a buffer is held.
				 */
				inline explicit operator bool() const noexcept {
					return static_cast<const wchar_t*>(*this) != nullptr;
				}

				/** @} */

				/**
				 * @name Conversions
				 * @{
				 */

				/**
				 * @brief View of the owned buffer.
				 * @return Buffer, or null.
				 * @note Same lifetime rules as `std::wstring::c_str()`.
				 */
				explicit operator const wchar_t*() const noexcept;

				/**
				 * @brief Non-owning view of the text.
				 * @return Empty view when the buffer is null.
				 * @note Same lifetime rules as `std::wstring::c_str()`.
				 */
				inline explicit operator std::wstring_view() const noexcept {
					const wchar_t* text = static_cast<const wchar_t*>(*this);
					return text ? std::wstring_view(text) : std::wstring_view();
				}

				/**
				 * @brief Copy of the text in the caller’s heap.
				 * @return Empty string when the buffer is null.
				 */
				STORMBYTE_FORCE_INLINE operator std::wstring() const {
					const wchar_t* text = static_cast<const wchar_t*>(*this);
					return text ? std::wstring(text) : std::wstring();
				}

				/**
				 * @brief Writes the text to @p stream.
				 * @param stream Destination.
				 * @return @p stream.
				 */
				STORMBYTE_FORCE_INLINE std::wostream& operator<<(std::wostream& stream) const {
					const wchar_t* text = static_cast<const wchar_t*>(*this);
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
				bool operator==(const WCString& other) const noexcept;

				/**
				 * @brief Content inequality.
				 * @param other Other buffer.
				 * @return Whether the texts differ.
				 */
				bool operator!=(const WCString& other) const noexcept {
					return !(*this == other);
				}

				/**
				 * @brief Content equality with a C wide string.
				 * @param str May be null (treated as a null @ref WCString).
				 * @return Whether the texts are equal.
				 */
				bool operator==(const wchar_t* str) const noexcept;

				/**
				 * @brief Content inequality with a C wide string.
				 * @param str May be null.
				 * @return Whether the texts differ.
				 */
				bool operator!=(const wchar_t* str) const noexcept {
					return !(*this == str);
				}

				/**
				 * @brief Content order. Null is less than any text.
				 * @param other Other buffer.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const WCString& other) const noexcept;

				/**
				 * @brief Content order against a C wide string.
				 * @param str May be null.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const wchar_t* str) const noexcept;

				/** @} */

			private:
				wchar_t* m_data;	///< Owned buffer

				/**
				 * @brief Allocate wide text plus an aligned private capacity header.
				 * @param capacity Usable code-unit count, excluding the NUL.
				 * @return Text pointer after its private header.
				 */
				static wchar_t* Allocate(std::size_t capacity);

				/**
				 * @brief Free a pointer returned by @ref Allocate.
				 * @param text Text pointer, or null.
				 */
				static void Release(wchar_t* text) noexcept;

				/**
				 * @brief Read capacity from a pointer returned by @ref Allocate.
				 * @param text Text pointer, or null.
				 * @return Usable capacity, excluding the NUL.
				 */
				static std::size_t CapacityOf(const wchar_t* text) noexcept;

				/**
				 * @brief Copies @p str into a new buffer.
				 * @param str Source; may be null.
				 * @return New buffer, or null.
				 */
				static wchar_t* Duplicate(const wchar_t* str) noexcept;

				/**
				 * @brief Copies @p sv into a new buffer and appends NUL.
				 * @param sv Source view.
				 * @return New buffer (`L""` when @p sv is empty).
				 */
				static wchar_t* Duplicate(std::wstring_view sv) noexcept;
		};

		/**
		 * @brief Writes @p text to @p stream.
		 * @param stream Destination.
		 * @param text Source.
		 * @return @p stream.
		 */
		inline std::wostream& operator<<(std::wostream& stream, const WCString& text) {
			return text.operator<<(stream);
		}

		/**
		 * @brief Content equality.
		 * @param str C wide string; may be null.
		 * @param text Buffer.
		 * @return Whether the texts are equal.
		 */
		inline bool operator==(const wchar_t* str, const WCString& text) noexcept {
			return text == str;
		}

		/**
		 * @brief Content inequality.
		 * @param str C wide string; may be null.
		 * @param text Buffer.
		 * @return Whether the texts differ.
		 */
		inline bool operator!=(const wchar_t* str, const WCString& text) noexcept {
			return text != str;
		}

		/**
		 * @brief Swaps two buffers.
		 * @param left First buffer.
		 * @param right Second buffer.
		 */
		inline void swap(WCString& left, WCString& right) noexcept {
			left.swap(right);
		}
	}
}

#include <StormByte/size.hxx>

/**
 * @brief Hash of the text (`0` when the buffer is null).
 */
template<>
struct std::hash<StormByte::Safe::WCString> {
	/**
	 * @brief Hashes @p text.
	 * @param text Buffer.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::WCString& text) const noexcept {
		const wchar_t* raw = static_cast<const wchar_t*>(text);
		if (!raw)
			return 0;
		return std::hash<std::wstring_view>{}(raw);
	}
};
