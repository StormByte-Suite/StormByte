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

#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/visibility.h>

#if __has_include(<fmt/xchar.h>)
#include <fmt/xchar.h>
#endif

#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cwctype>
#include <format>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ostream>
#include <queue>
#include <ranges>
#include <span>
#include <string>
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
	 * @brief Owned UTF-8 and wide text. Short wide text lives in the object; the long path is Base-owned.
	 */
	namespace Safe {
		/**
		 * @class WString
		 * @brief Owned wide text.
		 *
		 * Not a `std::wstring`. Short text lives in the object, NUL-terminated, with an explicit size.
		 * Longer text is a wide buffer freed in Base. There is no null state: a moved-from string is empty.
		 * `data()` and `c_str()` are never null.
		 *
		 * Classic and ranges algorithms can read or modify existing units; they cannot change the owned size.
		 * `operator std::wstring_view` is implicit and inline. `operator std::wstring` is explicit and
		 * `STORMBYTE_FORCE_INLINE` (caller heap).
		 *
		 * Conversion to @ref String is explicit and runs in the module. Conversion from @ref String copies
		 * UTF-8 into wide units in the module.
		 *
		 * Observers follow `std::wstring_view`. Size-changing modifiers follow `std::wstring` value semantics.
		 * `capacity()` and `reserve(Size)` count code units excluding NUL; reserve never shrinks.
		 * `size_type` is @ref Size. There is no allocator.
		 */
		class STORMBYTE_PUBLIC WString {
			public:
				using value_type = wchar_t;	///< Code unit type
				using size_type = Size;	///< Unit count and position type
				using difference_type = std::ptrdiff_t;	///< Iterator distance type
				using pointer = wchar_t*;	///< Mutable buffer pointer
				using const_pointer = const wchar_t*;	///< Read-only buffer pointer
				using reference = wchar_t&;	///< Mutable unit reference
				using const_reference = const wchar_t&;	///< Read-only unit reference
				using iterator = wchar_t*;	///< Mutable contiguous iterator
				using const_iterator = const wchar_t*;	///< Contiguous observer
				using reverse_iterator = std::reverse_iterator<iterator>;	///< Mutable reverse observer
				using const_reverse_iterator = std::reverse_iterator<const_iterator>;	///< Reverse observer

				static constexpr std::size_t SSO_CAPACITY = 24 / sizeof(wchar_t) - 1;	///< Short capacity, excluding the trailing NUL

				/**
				 * @name Life
				 * @{
				 */

				/**
				 * @brief Empty text. Not null.
				 */
				WString() noexcept;

				/**
				 * @brief Copies a wide C string. A null pointer becomes empty.
				 * @param str Source; may be null.
				 */
				WString(const wchar_t* str) noexcept;

				/**
				 * @brief Copies a view into owned storage.
				 * @param str Source. Embedded NUL counts.
				 */
				WString(std::wstring_view str) noexcept;

				/**
				 * @brief Construct @p count copies of @p character.
				 * @param count Unit count.
				 * @param character Fill unit.
				 * @throws AllocationError The long path could not be allocated.
				 */
				WString(size_type count, wchar_t character);

				/**
				 * @brief Copy a slice of @p other.
				 * @param other Source text.
				 * @param pos Start, in code units.
				 * @param count Length. @ref npos means through the end.
				 * @throws OutOfBoundsError @p pos is past @p other.size().
				 * @throws AllocationError The long path could not be allocated.
				 */
				WString(const WString& other, size_type pos, size_type count = npos);

				/**
				 * @brief Copy the unit range `[first, last)`.
				 * @tparam InputIt Input iterator of wide units.
				 * @param first Start.
				 * @param last End.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::input_iterator InputIt>
				WString(InputIt first, InputIt last): WString() {
					append(first, last);
				}

				/**
				 * @brief Wide text from UTF-8, including an empty source.
				 * @param other UTF-8 source.
				 */
				explicit WString(const String& other) noexcept;

				/**
				 * @brief Copy constructor.
				 * @param other Text to copy.
				 */
				WString(const WString& other) noexcept;

				/**
				 * @brief Move constructor.
				 * @param other Text to take. @p other becomes empty.
				 */
				WString(WString&& other) noexcept;

				/**
				 * @brief Releases a long buffer. A short string has nothing to free.
				 */
				~WString() noexcept;

				/**
				 * @brief Copy assignment.
				 * @param other Text to copy.
				 * @return *this.
				 */
				WString& operator=(const WString& other) noexcept;

				/**
				 * @brief Move assignment.
				 * @param other Text to take. @p other becomes empty.
				 * @return *this.
				 */
				WString& operator=(WString&& other) noexcept;

				/**
				 * @brief Copy text from a caller-owned view.
				 * @param text Source units.
				 * @return This string.
				 */
				WString& operator=(std::wstring_view text);

				/** @} */

				/**
				 * @name Range
				 * @{
				 */

				/**
				 * @brief First unit.
				 * @return Iterator. Never null.
				 */
				inline iterator begin() noexcept {
					return data();
				}

				/**
				 * @brief First unit.
				 * @return Read-only iterator. Never null.
				 */
				inline const_iterator begin() const noexcept {
					return data();
				}

				/**
				 * @brief One past the last unit.
				 * @return Iterator. Never null.
				 */
				inline iterator end() noexcept {
					return data() + static_cast<std::size_t>(size());
				}

				/**
				 * @brief One past the last unit.
				 * @return Read-only iterator. Never null.
				 */
				inline const_iterator end() const noexcept {
					return data() + static_cast<std::size_t>(size());
				}

				/**
				 * @brief First unit.
				 * @return Iterator.
				 */
				inline const_iterator cbegin() const noexcept {
					return begin();
				}

				/**
				 * @brief One past the last unit.
				 * @return Iterator.
				 */
				inline const_iterator cend() const noexcept {
					return end();
				}

				/**
				 * @brief Reverse begin.
				 * @return Reverse iterator.
				 */
				inline reverse_iterator rbegin() noexcept {
					return reverse_iterator(end());
				}

				/**
				 * @brief Reverse begin.
				 * @return Read-only reverse iterator.
				 */
				inline const_reverse_iterator rbegin() const noexcept {
					return const_reverse_iterator(end());
				}

				/**
				 * @brief Reverse end.
				 * @return Reverse iterator.
				 */
				inline reverse_iterator rend() noexcept {
					return reverse_iterator(begin());
				}

				/**
				 * @brief Reverse end.
				 * @return Read-only reverse iterator.
				 */
				inline const_reverse_iterator rend() const noexcept {
					return const_reverse_iterator(begin());
				}

				/**
				 * @brief Reverse begin.
				 * @return Reverse iterator.
				 */
				inline const_reverse_iterator crbegin() const noexcept {
					return rbegin();
				}

				/**
				 * @brief Reverse end.
				 * @return Reverse iterator.
				 */
				inline const_reverse_iterator crend() const noexcept {
					return rend();
				}

				/**
				 * @brief Contiguous pointer to the first unit, or to the trailing NUL when empty.
				 * @return Never null.
				 */
				wchar_t* data() noexcept;

				/**
				 * @brief Read-only contiguous pointer to the first unit, or to the trailing NUL when empty.
				 * @return Never null.
				 */
				const wchar_t* data() const noexcept;

				/**
				 * @brief Same pointer as @ref data. The unit at `size()` is NUL and is not part of the length.
				 * @return Never null.
				 */
				inline const wchar_t* c_str() const noexcept {
					return data();
				}

				/**
				 * @brief Unit count, excluding the trailing NUL.
				 * @return Length as @ref StormByte::Size.
				 */
				Size size() const noexcept;

				/**
				 * @brief Allocated unit capacity, excluding the trailing NUL.
				 * @return Short capacity, or the long-buffer capacity.
				 */
				Size capacity() const noexcept;

				/**
				 * @brief Largest representable unit count.
				 * @return Maximum size.
				 */
				Size max_size() const noexcept;

				/**
				 * @brief Same as @ref size.
				 * @return Length as @ref StormByte::Size.
				 */
				inline Size length() const noexcept {
					return size();
				}

				/**
				 * @brief Whether @ref size is zero.
				 * @return Emptiness. An empty string still has a pointer.
				 */
				inline bool empty() const noexcept {
					return size() == 0;
				}

				/**
				 * @brief Append wide code units.
				 * @param text Text to append.
				 * @return This string.
				 */
				WString& append(std::wstring_view text);

				/**
				 * @brief Append count copies of a wide code unit.
				 * @param count Number of copies.
				 * @param character Code unit to append.
				 * @return This string.
				 */
				WString& append(size_type count, wchar_t character);

				/**
				 * @brief Append the unit range `[first, last)`.
				 * @tparam InputIt Input iterator of wide units.
				 * @param first Start.
				 * @param last End.
				 * @return This string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::input_iterator InputIt>
				WString& append(InputIt first, InputIt last) {
					for (; first != last; ++first)
						push_back(static_cast<wchar_t>(*first));
					return *this;
				}

				/**
				 * @brief Append a copy of @p range.
				 * @tparam R Input range of wide units.
				 * @param range Source range.
				 * @return This string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::ranges::input_range R>
				WString& append_range(R&& range) {
					return append(std::ranges::begin(range), std::ranges::end(range));
				}

				/**
				 * @brief Replace the contents with a wide text view.
				 * @param text Source code units.
				 * @return This string.
				 */
				WString& assign(std::wstring_view text);

				/**
				 * @brief Replace the contents with count copies of a wide code unit.
				 * @param count Number of copies.
				 * @param character Code unit to assign.
				 * @return This string.
				 */
				WString& assign(size_type count, wchar_t character);

				/**
				 * @brief Replace the contents with `[first, last)`.
				 * @tparam InputIt Input iterator of wide units.
				 * @param first Start.
				 * @param last End.
				 * @return This string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::input_iterator InputIt>
				WString& assign(InputIt first, InputIt last) {
					clear();
					return append(first, last);
				}

				/**
				 * @brief Replace the contents with a copy of @p range.
				 * @tparam R Input range of wide units.
				 * @param range Source range.
				 * @return This string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::ranges::input_range R>
				WString& assign_range(R&& range) {
					return assign(std::ranges::begin(range), std::ranges::end(range));
				}

				/**
				 * @brief Append a wide view. A WString converts to this view.
				 * @param text Text to append.
				 * @return This string.
				 */
				WString& operator+=(std::wstring_view text);

				/**
				 * @brief Append one wide code unit.
				 * @param character Code unit.
				 * @return This string.
				 */
				WString& operator+=(wchar_t character);

				/**
				 * @brief Append one wide code unit.
				 * @param character Code unit.
				 */
				void push_back(wchar_t character);

				/**
				 * @brief Remove the final code unit. Empty use follows `std::wstring` preconditions.
				 */
				void pop_back();

				/**
				 * @brief Replace the contents with an empty string, retaining long-path capacity.
				 */
				void clear();

				/**
				 * @brief Reserve storage for at least @p new_capacity code units.
				 * @param new_capacity Requested unit capacity, excluding the NUL.
				 * @throw OutOfBoundsError The request cannot be represented, including a trailing NUL.
				 * @throw AllocationError The size is representable but the wide allocation failed.
				 * @note Requests at or below capacity do not shrink or reallocate.
				 */
				void reserve(size_type new_capacity);

				/**
				 * @brief Drop unused long-path capacity. A short string is unchanged.
				 */
				void shrink_to_fit();

				/**
				 * @brief Resize the unit sequence, filling new units with character.
				 * @param count New unit count.
				 * @param character Fill unit, default initialized to NUL.
				 */
				void resize(size_type count, wchar_t character = wchar_t{});

				/**
				 * @brief Insert wide code units at a code-unit position.
				 * @param position Insertion position.
				 * @param text Text to insert.
				 * @return This string.
				 */
				WString& insert(size_type position, std::wstring_view text);

				/**
				 * @brief Insert @p count copies of @p character at @p position.
				 * @param position Insertion position.
				 * @param count Copy count.
				 * @param character Fill unit.
				 * @return This string.
				 * @throws OutOfBoundsError @p position is past @ref size.
				 * @throws AllocationError The long path could not be allocated.
				 */
				WString& insert(size_type position, size_type count, wchar_t character);

				/**
				 * @brief Insert `[first, last)` before @p pos.
				 * @tparam InputIt Input iterator of wide units.
				 * @param pos Insertion point.
				 * @param first Start.
				 * @param last End.
				 * @return Iterator to the first inserted unit, or @p pos when the range is empty.
				 * @throws OutOfBoundsError @p pos is outside this string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::input_iterator InputIt>
				iterator insert(const_iterator pos, InputIt first, InputIt last) {
					const Size index{static_cast<std::size_t>(pos - cbegin())};
					WString copied;
					copied.append(first, last);
					insert(index, static_cast<std::wstring_view>(copied));
					return begin() + static_cast<std::size_t>(index);
				}

				/**
				 * @brief Insert an initializer list before @p pos.
				 * @param pos Insertion point.
				 * @param values Units to insert.
				 * @return Iterator to the first inserted unit.
				 * @throws OutOfBoundsError @p pos is outside this string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				iterator insert(const_iterator pos, std::initializer_list<wchar_t> values);

				/**
				 * @brief Insert a copy of @p range before @p pos.
				 * @tparam R Input range of wide units.
				 * @param pos Insertion point.
				 * @param range Source range.
				 * @return Iterator to the first inserted unit.
				 * @throws OutOfBoundsError @p pos is outside this string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				template<std::ranges::input_range R>
				iterator insert_range(const_iterator pos, R&& range) {
					return insert(pos, std::ranges::begin(range), std::ranges::end(range));
				}

				/**
				 * @brief Erase wide code units starting at position.
				 * @param position First code unit to erase.
				 * @param count Maximum code units to erase.
				 * @return This string.
				 */
				WString& erase(size_type position = {}, size_type count = npos);

				/**
				 * @brief Erase the unit at @p pos.
				 * @param pos Unit to erase.
				 * @return Iterator following the erased unit.
				 * @throws OutOfBoundsError @p pos is outside this string.
				 */
				iterator erase(const_iterator pos);

				/**
				 * @brief Erase `[first, last)`.
				 * @param first Start.
				 * @param last End.
				 * @return Iterator following the erased range.
				 * @throws OutOfBoundsError The range is outside this string.
				 */
				iterator erase(const_iterator first, const_iterator last);

				/**
				 * @brief Replace a wide code-unit range with text.
				 * @param position First code unit to replace.
				 * @param count Maximum code units to erase.
				 * @param text Replacement text.
				 * @return This string.
				 */
				WString& replace(size_type position, size_type count, std::wstring_view text);

				/**
				 * @brief Replace `[first, last)` with @p text.
				 * @param first Start.
				 * @param last End.
				 * @param text Replacement.
				 * @return This string.
				 * @throws OutOfBoundsError The range is outside this string.
				 * @throws AllocationError The long path could not be allocated.
				 */
				WString& replace(const_iterator first, const_iterator last, std::wstring_view text);

				/**
				 * @brief Code unit at @p index.
				 * @param index Position in `[0, size()]`. `size()` is the trailing NUL.
				 * @return Code unit.
				 * @note `index > size()` is undefined and `assert`s when assertions are on.
				 */
				inline wchar_t& operator[](const Size& index) noexcept {
					assert(index <= size());
					return data()[static_cast<std::size_t>(index)];
				}

				/**
				 * @brief Read-only code unit at @p index.
				 * @param index Position in `[0, size()]`. `size()` is the trailing NUL.
				 * @return Code unit copy.
				 * @note `index > size()` is undefined and `assert`s when assertions are on.
				 */
				inline wchar_t operator[](const Size& index) const noexcept {
					assert(index <= size());
					return data()[static_cast<std::size_t>(index)];
				}

				/**
				 * @brief Unit at @p index.
				 * @param index Unit offset.
				 * @return Unit.
				 * @throws OutOfBoundsError @p index is not less than @ref size.
				 */
				wchar_t& at(size_type index);

				/**
				 * @brief Read-only unit at @p index.
				 * @param index Unit offset.
				 * @return Unit.
				 * @throws OutOfBoundsError @p index is not less than @ref size.
				 */
				const wchar_t& at(size_type index) const;

				/**
				 * @brief Copy at most @p count units into @p dest, starting at @p pos.
				 * @param dest Caller buffer.
				 * @param count Buffer capacity, in units.
				 * @param pos Start, in code units.
				 * @return Units copied.
				 * @throws OutOfBoundsError @p pos is past @ref size.
				 */
				size_type copy(wchar_t* dest, size_type count, size_type pos = {}) const;

				/**
				 * @brief Whether the text is non-empty.
				 * @return `false` for empty text. A pointer is always held.
				 */
				inline explicit operator bool() const noexcept {
					return !empty();
				}

				/** @} */

				/**
				 * @name Conversions
				 * @{
				 */

				/**
				 * @brief Non-owning view of the text.
				 * @return View of `size()` units. Embedded NUL is preserved. Never a null buffer.
				 * @note Same lifetime as `std::wstring::c_str()`. Invalidated by a mutating operation that reallocates.
				 */
				inline operator std::wstring_view() const noexcept {
					return std::wstring_view(data(), static_cast<std::size_t>(size()));
				}

				/**
				 * @brief Copy of the text in the caller’s heap.
				 * @return Caller-owned `std::wstring`.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::wstring() const {
					return std::wstring(static_cast<std::wstring_view>(*this));
				}

				/**
				 * @brief View of the owned buffer.
				 * @return `c_str()`. Never null.
				 * @note Same lifetime as `std::wstring::c_str()`.
				 */
				inline explicit operator const wchar_t*() const noexcept {
					return c_str();
				}

				/**
				 * @brief UTF-8 text decoded in the module.
				 * @return Owned @ref String.
				 */
				explicit operator String() const noexcept;

				/**
				 * @brief Non-owning view of the owned units.
				 * @return `data()`. Never null.
				 */
				inline const wchar_t* Bytes() const noexcept {
					return data();
				}

				/** @} */

				/**
				 * @name Lookup
				 * @{
				 */

				static constexpr size_type npos{~0ull};	///< Not found.

				/**
				 * @brief Whether the text begins with @p text.
				 * @param text Prefix.
				 * @return Match. Empty prefix matches.
				 */
				inline bool starts_with(std::wstring_view text) const noexcept {
					return static_cast<std::wstring_view>(*this).starts_with(text);
				}

				/**
				 * @brief Whether the text begins with @p ch.
				 * @param ch Prefix unit.
				 * @return Match.
				 */
				inline bool starts_with(wchar_t ch) const noexcept {
					return static_cast<std::wstring_view>(*this).starts_with(ch);
				}

				/**
				 * @brief Whether the text ends with @p text.
				 * @param text Suffix.
				 * @return Match. Empty suffix matches.
				 */
				inline bool ends_with(std::wstring_view text) const noexcept {
					return static_cast<std::wstring_view>(*this).ends_with(text);
				}

				/**
				 * @brief Whether the text ends with @p ch.
				 * @param ch Suffix unit.
				 * @return Match.
				 */
				inline bool ends_with(wchar_t ch) const noexcept {
					return static_cast<std::wstring_view>(*this).ends_with(ch);
				}

				/**
				 * @brief Whether @p text occurs.
				 * @param text Needle.
				 * @return Match. Empty needle matches.
				 */
				inline bool contains(std::wstring_view text) const noexcept {
					return static_cast<std::wstring_view>(*this).contains(text);
				}

				/**
				 * @brief Whether @p ch occurs.
				 * @param ch Unit.
				 * @return Match.
				 */
				inline bool contains(wchar_t ch) const noexcept {
					return static_cast<std::wstring_view>(*this).contains(ch);
				}

				/**
				 * @brief First occurrence of @p text at or after @p pos.
				 * @param text Needle.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find(std::wstring_view text, Size pos = {}) const noexcept {
					return FindAt(static_cast<std::wstring_view>(*this), text, pos);
				}

				/**
				 * @brief First occurrence of @p ch at or after @p pos.
				 * @param ch Unit.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find(wchar_t ch, Size pos = {}) const noexcept {
					return FindAt(static_cast<std::wstring_view>(*this), ch, pos);
				}

				/**
				 * @brief First occurrence of @p count units of @p text.
				 * @param text Needle. May be null when @p count is zero.
				 * @param pos Start, in code units.
				 * @param count Units of @p text to use.
				 * @return Index, or @ref npos.
				 */
				inline Size find(const wchar_t* text, Size pos, Size count) const noexcept {
					return find(std::wstring_view(text, static_cast<std::size_t>(count)), pos);
				}

				/**
				 * @brief Last occurrence of @p text at or before @p pos.
				 * @param text Needle.
				 * @param pos Highest start. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(std::wstring_view text, Size pos = npos) const noexcept {
					return RFindAt(static_cast<std::wstring_view>(*this), text, pos);
				}

				/**
				 * @brief Last occurrence of @p ch at or before @p pos.
				 * @param ch Unit.
				 * @param pos Highest start. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(wchar_t ch, Size pos = npos) const noexcept {
					return RFindAt(static_cast<std::wstring_view>(*this), ch, pos);
				}

				/**
				 * @brief Last occurrence of @p count units of @p text.
				 * @param text Needle. May be null when @p count is zero.
				 * @param pos Highest start.
				 * @param count Units of @p text to use.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(const wchar_t* text, Size pos, Size count) const noexcept {
					return rfind(std::wstring_view(text, static_cast<std::size_t>(count)), pos);
				}

				/**
				 * @brief First unit that is in @p text, at or after @p pos.
				 * @param text Set of units.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_of(std::wstring_view text, Size pos = {}) const noexcept {
					const std::wstring_view self = *this;
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find_first_of(text, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief First @p ch at or after @p pos.
				 * @param ch Unit.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_of(wchar_t ch, Size pos = {}) const noexcept {
					return find(ch, pos);
				}

				/**
				 * @brief Last unit that is in @p text, at or before @p pos.
				 * @param text Set of units.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_of(std::wstring_view text, Size pos = npos) const noexcept {
					const std::wstring_view self = *this;
					const std::size_t start = pos == npos ? std::wstring_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.find_last_of(text, start));
				}

				/**
				 * @brief Last @p ch at or before @p pos.
				 * @param ch Unit.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_of(wchar_t ch, Size pos = npos) const noexcept {
					return rfind(ch, pos);
				}

				/**
				 * @brief First unit that is not in @p text, at or after @p pos.
				 * @param text Set of units.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_not_of(std::wstring_view text, Size pos = {}) const noexcept {
					const std::wstring_view self = *this;
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find_first_not_of(text, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief First unit other than @p ch, at or after @p pos.
				 * @param ch Unit.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_not_of(wchar_t ch, Size pos = {}) const noexcept {
					const std::wstring_view self = *this;
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find_first_not_of(ch, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief Last unit that is not in @p text, at or before @p pos.
				 * @param text Set of units.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_not_of(std::wstring_view text, Size pos = npos) const noexcept {
					const std::wstring_view self = *this;
					const std::size_t start = pos == npos ? std::wstring_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.find_last_not_of(text, start));
				}

				/**
				 * @brief Last unit other than @p ch, at or before @p pos.
				 * @param ch Unit.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_not_of(wchar_t ch, Size pos = npos) const noexcept {
					const std::wstring_view self = *this;
					const std::size_t start = pos == npos ? std::wstring_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.find_last_not_of(ch, start));
				}

				/**
				 * @brief Copy of a slice. Not a view.
				 * @param pos Start, in code units.
				 * @param count Length. @ref npos means through the end.
				 * @return Owned text. Empty when @p pos is past @ref size. Does not throw.
				 */
				inline WString substr(Size pos = {}, Size count = npos) const noexcept {
					const std::wstring_view text = *this;
					if (pos > Size{text.size()})
						return WString();
					const std::size_t n = count == npos ? std::wstring_view::npos : static_cast<std::size_t>(count);
					return WString(text.substr(static_cast<std::size_t>(pos), n));
				}

				/**
				 * @brief Same order as `std::wstring_view::compare`.
				 * @param text Other text.
				 * @return Negative, zero, or positive.
				 */
				inline int compare(std::wstring_view text) const noexcept {
					return static_cast<std::wstring_view>(*this).compare(text);
				}

				/**
				 * @brief Compare a slice with @p text.
				 * @param pos Start, in code units.
				 * @param count Slice length.
				 * @param text Other text.
				 * @return Negative, zero, or positive.
				 */
				inline int compare(size_type pos, size_type count, std::wstring_view text) const noexcept {
					return static_cast<std::wstring_view>(substr(pos, count)).compare(text);
				}

				/**
				 * @brief First unit.
				 * @return Unit.
				 * @note Empty is undefined, same as `std::wstring::front`.
				 */
				inline wchar_t& front() noexcept {
					return (*this)[Size{0}];
				}

				/**
				 * @brief Read the first unit.
				 * @return Unit copy.
				 * @note Empty is undefined, same as `std::wstring::front`.
				 */
				inline wchar_t front() const noexcept {
					return (*this)[Size{0}];
				}

				/**
				 * @brief Last unit.
				 * @return Unit.
				 * @note Empty is undefined, same as `std::wstring::back`.
				 */
				inline wchar_t& back() noexcept {
					return (*this)[size() - Size{1}];
				}

				/**
				 * @brief Read the last unit.
				 * @return Unit copy.
				 * @note Empty is undefined, same as `std::wstring::back`.
				 */
				inline wchar_t back() const noexcept {
					return (*this)[size() - Size{1}];
				}

				/** @} */

				/**
				 * @name Helpers
				 * @{
				 */

				/**
				 * @brief ASCII-letter lower case; other units copied.
				 * @param str Source.
				 * @return New text.
				 */
				static WString ToLower(std::wstring_view str) noexcept;

				/**
				 * @brief ASCII-letter upper case; other units copied.
				 * @param str Source.
				 * @return New text.
				 */
				static WString ToUpper(std::wstring_view str) noexcept;

				/**
				 * @brief Turns CR LF into LF.
				 * @param str Source.
				 * @return New text.
				 */
				static WString SanitizeNewlines(std::wstring_view str) noexcept;

				/**
				 * @brief Drops wide space units.
				 * @param str Source.
				 * @return New text.
				 */
				static WString RemoveWhitespace(std::wstring_view str) noexcept;

				/**
				 * @brief Optional sign plus ASCII digits.
				 * @param str Source.
				 * @return Whether @p str is an integer token.
				 */
				static bool IsInteger(std::wstring_view str) noexcept;

				/**
				 * @brief Whitespace-separated tokens. Nodes are allocated in Base.
				 * @param str Source.
				 * @return Owned tokens.
				 */
				static Vector<WString> Split(std::wstring_view str);

				/**
				 * @brief Copy caller-owned tokens into Base storage.
				 * @param parts Source. Not modified.
				 * @return Owned tokens.
				 */
				static inline Vector<WString> Split(const std::vector<WString>& parts) {
					Vector<WString> result;
					for (const WString& part : parts)
						result.push_back(part);
					return result;
				}

				/**
				 * @brief Tokens on @p delimiter, including empty ones. Nodes are allocated in Base.
				 * @param str Source.
				 * @param delimiter Separator.
				 * @return Owned tokens.
				 */
				static Queue<WString> Explode(std::wstring_view str, wchar_t delimiter);

				/**
				 * @brief Copy a caller-owned queue into Base storage.
				 * @param parts Source. Not modified.
				 * @return Owned tokens.
				 */
				static inline Queue<WString> Explode(const std::queue<WString>& parts) {
					Queue<WString> result;
					std::queue<WString> copy = parts;
					while (!copy.empty()) {
						result.push(copy.front());
						copy.pop();
					}
					return result;
				}

				/**
				 * @brief ASCII-letter lower case of this text.
				 * @return New text.
				 */
				inline WString ToLower() const noexcept {
					return ToLower(static_cast<std::wstring_view>(*this));
				}

				/**
				 * @brief ASCII-letter upper case of this text.
				 * @return New text.
				 */
				inline WString ToUpper() const noexcept {
					return ToUpper(static_cast<std::wstring_view>(*this));
				}

				/**
				 * @brief CR LF to LF on this text.
				 * @return New text.
				 */
				inline WString SanitizeNewlines() const noexcept {
					return SanitizeNewlines(static_cast<std::wstring_view>(*this));
				}

				/**
				 * @brief Drops wide space units from this text.
				 * @return New text.
				 */
				inline WString RemoveWhitespace() const noexcept {
					return RemoveWhitespace(static_cast<std::wstring_view>(*this));
				}

				/**
				 * @brief Whether this text is an integer token.
				 * @return Whether it matches @ref IsInteger(std::wstring_view).
				 */
				inline bool IsInteger() const noexcept {
					return IsInteger(static_cast<std::wstring_view>(*this));
				}

				/**
				 * @brief Whitespace-separated tokens of this text.
				 * @return Owned tokens.
				 */
				Vector<WString> Split() const;

				/**
				 * @brief Tokens of this text on @p delimiter, including empty ones.
				 * @param delimiter Separator.
				 * @return Owned tokens.
				 */
				Queue<WString> Explode(wchar_t delimiter) const;

				/** @} */

				/**
				 * @name Comparison
				 * @{
				 */

				/**
				 * @brief Content equality.
				 * @param other Other text.
				 * @return Whether the texts are equal.
				 */
				inline bool operator==(const WString& other) const noexcept {
					return static_cast<std::wstring_view>(*this) == static_cast<std::wstring_view>(other);
				}

				/**
				 * @brief Content inequality.
				 * @param other Other text.
				 * @return Whether the texts differ.
				 */
				inline bool operator!=(const WString& other) const noexcept {
					return !(*this == other);
				}

				/**
				 * @brief Content equality with a wide C string. A null pointer compares equal to empty.
				 * @param str May be null.
				 * @return Whether the texts are equal.
				 */
				inline bool operator==(const wchar_t* str) const noexcept {
					return str ? static_cast<std::wstring_view>(*this) == std::wstring_view(str) : empty();
				}

				/**
				 * @brief Content inequality with a wide C string.
				 * @param str May be null.
				 * @return Whether the texts differ.
				 */
				inline bool operator!=(const wchar_t* str) const noexcept {
					return !(*this == str);
				}

				/**
				 * @brief Content order.
				 * @param other Other text.
				 * @return Ordering.
				 */
				inline std::strong_ordering operator<=>(const WString& other) const noexcept {
					return static_cast<std::wstring_view>(*this) <=> static_cast<std::wstring_view>(other);
				}

				/**
				 * @brief Content order against a wide C string. A null pointer compares as empty.
				 * @param str May be null.
				 * @return Ordering.
				 */
				inline std::strong_ordering operator<=>(const wchar_t* str) const noexcept {
					return str ? static_cast<std::wstring_view>(*this) <=> std::wstring_view(str)
						: static_cast<std::wstring_view>(*this) <=> std::wstring_view{};
				}

				/** @} */

				/**
				 * @brief Swaps storage with @p other.
				 * @param other Other text.
				 */
				void swap(WString& other) noexcept;

			private:
				/**
				 * @brief In-object short buffer, or a wide buffer pointer when bit 7 of @ref m_tag is set.
				 *
				 * `data` holds @ref SSO_CAPACITY units plus a trailing NUL. `m_tag` bits 0–6 are the short size.
				 * `long_` is active only while the long bit is set, and is freed in Base.
				 */
				struct alignas(alignof(void*)) Storage {
					union {
						wchar_t data[SSO_CAPACITY + 1];	///< Short units, or overlaid by @ref long_
						void* long_;	///< Wide buffer. Active only when the long bit is set
					};
					std::uint8_t m_tag;	///< Bit 7 long; bits 0–6 short size
				};

				static_assert(sizeof(Storage) == 32 || sizeof(wchar_t) == 4);
				static_assert(alignof(Storage) == alignof(void*));

				/**
				 * @brief Whether the wide buffer pointer is the active member.
				 * @return Long-path flag.
				 */
				bool IsLong() const noexcept;

				/**
				 * @brief Free the wide buffer, if any, and leave an empty short string.
				 */
				void ReleaseLong() noexcept;

				/**
				 * @brief Leave an empty short string. Does not free.
				 */
				void SetEmpty() noexcept;

				/**
				 * @brief Store a short sequence and its explicit size. `count` must be at most @ref SSO_CAPACITY.
				 * @param text Source units. May contain embedded NUL.
				 * @param count Unit count, excluding the trailing NUL written at `data[count]`.
				 */
				void SetShort(const wchar_t* text, std::size_t count) noexcept;

				/**
				 * @brief Convert a standard view index into the public position type.
				 * @param index Standard position or not-found sentinel.
				 * @return Public position or @ref npos.
				 */
				static Size FromIndex(std::size_t index) noexcept {
					return index == std::wstring_view::npos ? npos : Size{index};
				}

				/**
				 * @brief Find a needle from a checked starting position.
				 * @tparam Needle Unit or text view.
				 * @param self Text to search.
				 * @param needle Units to find.
				 * @param pos Starting position.
				 * @return Match position or @ref npos.
				 */
				template<typename Needle>
				static Size FindAt(std::wstring_view self, Needle needle, Size pos) noexcept {
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find(needle, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief Find the last needle at or before a position.
				 * @tparam Needle Unit or text view.
				 * @param self Text to search.
				 * @param needle Units to find.
				 * @param pos Highest starting position.
				 * @return Match position or @ref npos.
				 */
				template<typename Needle>
				static Size RFindAt(std::wstring_view self, Needle needle, Size pos) noexcept {
					const std::size_t start = pos == npos ? std::wstring_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.rfind(needle, start));
				}

				Storage m_storage{};	///< Short buffer or wide buffer
		};

		/**
		 * @brief Writes @p text to @p stream. Instantiated in the caller.
		 * @param stream Destination. Caller-owned stream.
		 * @param text Source.
		 * @return @p stream.
		 */
		STORMBYTE_FORCE_INLINE std::wostream& operator<<(std::wostream& stream, const WString& text) {
			return stream << static_cast<std::wstring_view>(text);
		}

		/**
		 * @brief Content equality.
		 * @param str Wide C string; may be null.
		 * @param text Text.
		 * @return Whether the texts are equal.
		 */
		inline bool operator==(const wchar_t* str, const WString& text) noexcept {
			return text == str;
		}

		/**
		 * @brief Content inequality.
		 * @param str Wide C string; may be null.
		 * @param text Text.
		 * @return Whether the texts differ.
		 */
		inline bool operator!=(const wchar_t* str, const WString& text) noexcept {
			return text != str;
		}

		/**
		 * @brief Concatenate two texts. A literal or a view converts to WString.
		 * @param left Left text.
		 * @param right Right text.
		 * @return New text owned by Base.
		 * @throws AllocationError The long path could not be allocated.
		 */
		inline WString operator+(const WString& left, const WString& right) {
			WString result(left);
			result += right;
			return result;
		}

		/**
		 * @brief Swaps two texts.
		 * @param left First text.
		 * @param right Second text.
		 */
		inline void swap(WString& left, WString& right) noexcept {
			left.swap(right);
		}
	}
}

template<>
struct StormByte::Safe::Hash<StormByte::Safe::WString> {
	/**
	 * @brief Hash @p text. The wide units are hashed as bytes of this process.
	 * @param text Text.
	 * @return Hash.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::Safe::WString& text) const noexcept {
		const std::wstring_view view = text;
		return HashBytes(std::as_bytes(std::span<const wchar_t>(view.data(), view.size())));
	}
};

/**
 * @brief Formats owned wide text with standard string format specifications.
 */
template<>
struct std::formatter<StormByte::Safe::WString, wchar_t>: std::formatter<std::wstring_view, wchar_t> {
	/**
	 * @brief Formats the borrowed view without copying the owned text.
	 * @tparam FormatContext Formatting context type.
	 * @param text Source text.
	 * @param context Destination formatting context.
	 * @return Iterator past the formatted output.
	 * @note The body is emitted in the caller; no formatting context crosses into Base.
	 */
	template<typename FormatContext>
	STORMBYTE_FORCE_INLINE auto format(const StormByte::Safe::WString& text, FormatContext& context) const {
		const std::wstring_view view = text;
		return std::formatter<std::wstring_view, wchar_t>::format(view, context);
	}
};

#if __has_include(<fmt/xchar.h>)
/**
 * @brief Formats owned wide text with the optional fmt string formatter.
 */
template<>
struct fmt::formatter<StormByte::Safe::WString, wchar_t>: fmt::formatter<std::wstring_view, wchar_t> {
	/**
	 * @brief Formats the borrowed view without copying the owned text.
	 * @tparam FormatContext Formatting context type.
	 * @param text Source text.
	 * @param context Destination formatting context.
	 * @return Iterator past the formatted output.
	 * @note The body is emitted in the caller; no formatting context crosses into Base.
	 */
	template<typename FormatContext>
	STORMBYTE_FORCE_INLINE auto format(const StormByte::Safe::WString& text, FormatContext& context) const {
		const std::wstring_view view = text;
		return fmt::formatter<std::wstring_view, wchar_t>::format(view, context);
	}
};
#endif

template<>
struct std::hash<StormByte::Safe::WString> {
	/**
	 * @brief Hashes @p text.
	 * @param text Text.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::WString& text) const noexcept {
		return StormByte::Safe::Hash<StormByte::Safe::WString>{}(text);
	}
};
