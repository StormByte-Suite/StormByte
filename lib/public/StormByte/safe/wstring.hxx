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

#pragma once

#include <StormByte/visibility.h>
#include <StormByte/safe/wcstring.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/safe/queue.hxx>

#include <compare>
#include <cstddef>
#include <cwctype>
#include <functional>
#include <iterator>
#include <ostream>
#include <queue>
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
	 * @brief Owned UTF-8 and wide text on top of @ref StormByte::Safe::CString / @ref StormByte::Safe::WCString.
	 */
	namespace Safe {
		/**
		 * @class String
		 * @brief UTF-8 counterpart of @ref WString. Defined in string.hxx.
		 */
		class String;

		/**
		 * @class WString
		 * @brief Owned wide text composed of @ref StormByte::Safe::WCString.
		 *
		 * Not a `std::wstring`. Iterators are constant and contiguous so
		 * algorithms that read a range of `wchar_t` work. In-place mutating
		 * algorithms do not: helpers return a new @ref WString.
		 *
		 * `operator std::wstring_view` is implicit and inline. `operator
		 * std::wstring` is explicit and `STORMBYTE_FORCE_INLINE` (caller heap).
		 *
		 * Conversion to @ref String is explicit and runs in the module
		 * (wide → UTF-8). Conversion from @ref String copies wide units
		 * in the module.
		 *
		 * `ToUpper` / `ToLower` map only ASCII `A–Z` / `a–z` (as `wchar_t`).
		 * Other code points are copied. On 16-bit `wchar_t`, a well-formed
		 * surrogate pair is copied together.
		 *
		 * Code units are contiguous and mutable. Classic and ranges algorithms
		 * can read or modify existing units; modifiers rebuild the Base-owned buffer
		 * from a caller-owned snapshot and do not adopt caller-CRT storage.
		 * Observers (`starts_with`, `ends_with`, `contains`, `find`, `substr`, …)
		 * follow `std::wstring_view`.
		 */
		class STORMBYTE_PUBLIC WString {
			public:
				using value_type = wchar_t;	///< Code unit type
				using size_type = Size;	///< Code-unit count and position type
				using difference_type = std::ptrdiff_t;	///< Iterator distance type
				using pointer = wchar_t*;	///< Mutable buffer pointer
				using const_pointer = const wchar_t*;	///< Read-only buffer pointer
				using reference = wchar_t&;	///< Mutable code-unit reference
				using const_reference = const wchar_t&;	///< Read-only code-unit reference
				using iterator = wchar_t*;	///< Mutable contiguous iterator
				using const_iterator = const wchar_t*;	///< Contiguous observer
				using reverse_iterator = std::reverse_iterator<iterator>;	///< Mutable reverse observer
				using const_reverse_iterator = std::reverse_iterator<const_iterator>;	///< Reverse observer

				/**
				 * @name Life
				 * @{
				 */

				/**
				 * @brief Null text.
				 */
				WString() noexcept;

				/**
				 * @brief Copies a wide C string.
				 * @param str Source; may be null.
				 */
				explicit WString(const wchar_t* str) noexcept;

				/**
				 * @brief Copies a view into an owned NUL-terminated buffer.
				 * @param str Source.
				 */
				explicit WString(std::wstring_view str) noexcept;

				/**
				 * @brief Takes an owned buffer.
				 * @param text Buffer.
				 */
				explicit WString(WCString text) noexcept;

				/**
				 * @brief Wide text from UTF-8.
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
				 * @param other Text to take. @p other becomes null.
				 */
				WString(WString&& other) noexcept;

				/**
				 * @brief Releases the buffer.
				 */
				~WString() noexcept = default;

				/**
				 * @brief Copy assignment.
				 * @param other Text to copy.
				 * @return *this.
				 */
				WString& operator=(const WString& other) noexcept;

				/**
				 * @brief Move assignment.
				 * @param other Text to take. @p other becomes null.
				 * @return *this.
				 */
				WString& operator=(WString&& other) noexcept;

				/**
				 * @brief Copy wide text from a caller-owned view into Base storage.
				 * @param text Source code units.
				 * @return This string.
				 */
				WString& operator=(std::wstring_view text) {
					return *this = WString(text);
				}

				/** @} */

				/**
				 * @name Range
				 * @{
				 */

				/**
				 * @brief First character, or null.
				 * @return Iterator.
				 */
				inline iterator begin() noexcept {
					return data();
				}

				/**
				 * @brief First character, or null.
				 * @return Read-only iterator.
				 */
				inline const_iterator begin() const noexcept {
					return data();
				}

				/**
				 * @brief One past the last character, or null.
				 * @return Iterator.
				 */
				inline iterator end() noexcept {
					wchar_t* text = data();
					return text ? text + static_cast<std::size_t>(size()) : nullptr;
				}

				/**
				 * @brief One past the last character, or null.
				 * @return Read-only iterator.
				 */
				inline const_iterator end() const noexcept {
					const wchar_t* text = data();
					return text ? text + size() : nullptr;
				}

				/**
				 * @brief First character, or null.
				 * @return Iterator.
				 */
				inline const_iterator cbegin() const noexcept {
					return begin();
				}

				/**
				 * @brief One past the last character, or null.
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
				 * @brief Contiguous pointer; null when the buffer is null.
				 * @return Pointer to the first code unit.
				 */
				inline wchar_t* data() noexcept {
					return m_text.data();
				}

				/**
				 * @brief Read-only contiguous pointer; null when the buffer is null.
				 * @return Pointer to the first code unit.
				 */
				inline const wchar_t* data() const noexcept {
					return static_cast<const wchar_t*>(m_text);
				}

				/**
				 * @brief Code-unit count; `0` when null or empty.
				 * @return Length as @ref StormByte::Size (code units, not bytes).
				 */
				inline Size size() const noexcept {
					return m_text.Length();
				}

				/**
				 * @brief Same as @ref size.
				 * @return Length as @ref StormByte::Size.
				 */
				inline Size length() const noexcept {
					return size();
				}

				/**
				 * @brief Whether @ref size is zero.
				 * @return Emptiness. A null buffer is empty.
				 */
				inline bool empty() const noexcept {
					return size() == 0;
				}

				 /**
				  * @brief Append wide code units.
				  * @param text Text to append.
				  * @return This string.
				  */
				 WString& append(std::wstring_view text) {
					 return Mutate([text](std::wstring& value) { value.append(text); });
				 }

				 /**
				  * @brief Append count copies of a wide code unit.
				  * @param count Number of copies.
				  * @param character Code unit to append.
				  * @return This string.
				  */
				 WString& append(size_type count, wchar_t character) {
					 return Mutate([count, character](std::wstring& value) { value.append(static_cast<std::size_t>(count), character); });
				 }

				/**
				 * @brief Replace the contents with a wide text view.
				 * @param text Source code units.
				 * @return This string.
				 */
				WString& assign(std::wstring_view text) { return *this = text; }

				/**
				 * @brief Replace the contents with count copies of a wide code unit.
				 * @param count Number of copies.
				 * @param character Code unit to assign.
				 * @return This string.
				 */
				WString& assign(size_type count, wchar_t character) {
					return Mutate([count, character](std::wstring& value) { value.assign(static_cast<std::size_t>(count), character); });
				}

				 /** @brief Append a wide view. @param text Text to append. @return This string. */
				 WString& operator+=(std::wstring_view text) { return append(text); }
				 /** @brief Append one wide code unit. @param character Code unit. @return This string. */
				 WString& operator+=(wchar_t character) { return append(1, character); }

				 /** @brief Append one wide code unit. @param character Code unit. */
				 void push_back(wchar_t character) { (void)append(1, character); }

				 /** @brief Remove the final code unit; empty-string use follows std::wstring preconditions. */
				 void pop_back() { Mutate([](std::wstring& value) { value.pop_back(); }); }

				 /** @brief Replace the contents with a valid empty wide string. */
				 void clear() { *this = WString(std::wstring_view{}); }

				 /**
				  * @brief Resize the code-unit sequence.
				  * @param count New code-unit count.
				  * @param character Fill code unit, default initialized to NUL.
				  */
				 void resize(size_type count, wchar_t character = wchar_t{}) {
					 Mutate([count, character](std::wstring& value) { value.resize(static_cast<std::size_t>(count), character); });
				 }

				 /**
				  * @brief Insert wide code units at a code-unit position.
				  * @param position Insertion position.
				  * @param text Text to insert.
				  * @return This string.
				  */
				 WString& insert(size_type position, std::wstring_view text) {
					 return Mutate([position, text](std::wstring& value) { value.insert(static_cast<std::size_t>(position), text); });
				 }

				 /**
				  * @brief Erase wide code units starting at position.
				  * @param position First code unit to erase.
				  * @param count Maximum code units to erase.
				  * @return This string.
				  */
				 WString& erase(size_type position = {}, size_type count = npos) {
					 return Mutate([position, count](std::wstring& value) {
						 value.erase(static_cast<std::size_t>(position), static_cast<std::size_t>(count));
					 });
				 }

				 /**
				  * @brief Replace a wide code-unit range with text.
				  * @param position First code unit to replace.
				  * @param count Maximum code units to erase.
				  * @param text Replacement text.
				  * @return This string.
				  */
				 WString& replace(size_type position, size_type count, std::wstring_view text) {
					 return Mutate([position, count, text](std::wstring& value) {
						 value.replace(static_cast<std::size_t>(position), static_cast<std::size_t>(count), text);
					 });
				 }

				/**
				 * @brief Code unit at @p index.
				 * @param index Position in `[0, size()]`. `size()` is the trailing NUL.
				 * @return Character.
				 * @note Null or `index > size()` is undefined and `assert`s when assertions are on.
				 */
				inline wchar_t& operator[](const Size& index) noexcept {
					return m_text[index];
				}

				/**
				 * @brief Read-only code unit at @p index.
				 * @param index Position in `[0, size()]`. `size()` is the trailing NUL.
				 * @return Character copy.
				 * @note Null or `index > size()` is undefined and `assert`s when assertions are on.
				 */
				inline wchar_t operator[](const Size& index) const noexcept {
					return m_text[index];
				}

				/**
				 * @brief Whether a buffer is held.
				 * @return `false` only for a null @ref WCString. `L""` is valid and empty.
				 */
				inline explicit operator bool() const noexcept {
					return static_cast<bool>(m_text);
				}

				/** @} */

				/**
				 * @name Conversions
				 * @{
				 */

				/**
				 * @brief Non-owning view of the text.
				 * @return Empty view when the buffer is null.
				 * @note Same lifetime as `std::wstring::c_str()`.
				 */
				inline operator std::wstring_view() const noexcept {
					return static_cast<std::wstring_view>(m_text);
				}

				/**
				 * @brief Copy of the text in the caller’s heap.
				 * @return Empty string when the buffer is null.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::wstring() const {
					return static_cast<std::wstring>(m_text);
				}

				/**
				 * @brief View of the owned buffer.
				 * @return Buffer, or null.
				 * @note Same lifetime as `std::wstring::c_str()`.
				 */
				inline explicit operator const wchar_t*() const noexcept {
					return static_cast<const wchar_t*>(m_text);
				}

				/**
				 * @brief UTF-8 text (wide encoded in the module).
				 * @return Owned @ref String.
				 */
				explicit operator String() const noexcept;

				/**
				 * @brief Non-owning view of the owned code units.
				 * @return Buffer, or null.
				 */
				inline const wchar_t* Bytes() const noexcept {
					return static_cast<const wchar_t*>(m_text);
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
				 * @param ch Prefix code unit.
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
				 * @param ch Suffix code unit.
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
				 * @param ch Code unit.
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
				 * @param ch Code unit.
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
				 * @param pos Highest start, in code units. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(std::wstring_view text, Size pos = npos) const noexcept {
					return RFindAt(static_cast<std::wstring_view>(*this), text, pos);
				}

				/**
				 * @brief Last occurrence of @p ch at or before @p pos.
				 * @param ch Code unit.
				 * @param pos Highest start, in code units. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(wchar_t ch, Size pos = npos) const noexcept {
					return RFindAt(static_cast<std::wstring_view>(*this), ch, pos);
				}

				/**
				 * @brief Last occurrence of @p count units of @p text.
				 * @param text Needle. May be null when @p count is zero.
				 * @param pos Highest start, in code units.
				 * @param count Units of @p text to use.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(const wchar_t* text, Size pos, Size count) const noexcept {
					return rfind(std::wstring_view(text, static_cast<std::size_t>(count)), pos);
				}

				/**
				 * @brief First unit that is in @p text, at or after @p pos.
				 * @param text Set of code units.
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
				 * @param ch Code unit.
				 * @param pos Start, in code units.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_of(wchar_t ch, Size pos = {}) const noexcept {
					return find(ch, pos);
				}

				/**
				 * @brief Last unit that is in @p text, at or before @p pos.
				 * @param text Set of code units.
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
				 * @param ch Code unit.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_of(wchar_t ch, Size pos = npos) const noexcept {
					return rfind(ch, pos);
				}

				/**
				 * @brief First unit that is not in @p text, at or after @p pos.
				 * @param text Set of code units.
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
				 * @param ch Code unit.
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
				 * @param text Set of code units.
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
				 * @param ch Code unit.
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
				 * @brief First code unit.
				 * @return Unit.
				 * @note Empty is undefined, same as `std::wstring::front`.
				 */
				inline wchar_t& front() noexcept {
					return (*this)[Size{0}];
				}

				/**
				 * @brief Read the first code unit.
				 * @return Code-unit copy.
				 * @note Empty is undefined, same as `std::wstring::front`.
				 */
				inline wchar_t front() const noexcept {
					return (*this)[Size{0}];
				}

				/**
				 * @brief Last code unit.
				 * @return Unit.
				 * @note Empty is undefined, same as `std::wstring::back`.
				 */
				inline wchar_t& back() noexcept {
					return (*this)[size() - Size{1}];
				}

				/**
				 * @brief Read the last code unit.
				 * @return Code-unit copy.
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
				 * @brief ASCII-letter lower case; other code points copied.
				 * @param str Source.
				 * @return New text.
				 */
				static WString ToLower(std::wstring_view str) noexcept;

				/**
				 * @brief ASCII-letter upper case; other code points copied.
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
				 * @brief Drops `iswspace` code units.
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
				 * @brief Whitespace-separated tokens. @p out is the caller’s container.
				 * @param str Source.
				 * @param[out] out Tokens.
				 * @note `STORMBYTE_FORCE_INLINE` so the container nodes are allocated in the caller.
				 */
				static STORMBYTE_FORCE_INLINE void Split(std::wstring_view str, std::vector<WString>& out) noexcept {
					out.clear();
					std::size_t i = 0;
					while (i < str.size()) {
						while (i < str.size() && std::iswspace(static_cast<wint_t>(str[i])) != 0)
							++i;
						if (i >= str.size())
							break;
						std::size_t j = i;
						while (j < str.size() && std::iswspace(static_cast<wint_t>(str[j])) == 0)
							++j;
						out.emplace_back(str.substr(i, j - i));
						i = j;
					}
				}

				/**
				 * @brief Whitespace-separated tokens into creator-owned Safe storage.
				 * @param str Source.
				 * @param[out] out Safe sequence, replaced only on success.
				 * @return Success or Failure; failure leaves @p out unchanged.
				 */
				static Status Split(std::wstring_view str, Vector<WString>& out) noexcept {
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

				/**
				 * @brief Tokens on @p delimiter. @p out is the caller’s container.
				 * @param str Source.
				 * @param delimiter Separator.
				 * @param[out] out Tokens, including empty ones.
				 * @note `STORMBYTE_FORCE_INLINE` so the container nodes are allocated in the caller.
				 */
				static STORMBYTE_FORCE_INLINE void Explode(std::wstring_view str, wchar_t delimiter, std::queue<WString>& out) noexcept {
					while (!out.empty())
						out.pop();
					std::size_t start = 0;
					for (std::size_t i = 0; i <= str.size(); ++i) {
						if (i == str.size() || str[i] == delimiter) {
							out.emplace(str.substr(start, i - start));
							start = i + 1;
						}
					}
				}

				/**
				 * @brief Tokens on @p delimiter into creator-owned Safe storage.
				 * @param str Source.
				 * @param delimiter Separator.
				 * @param[out] out Safe queue, replaced only on success.
				 * @return Success or Failure; failure leaves @p out unchanged.
				 */
				static Status Explode(std::wstring_view str, wchar_t delimiter, Queue<WString>& out) noexcept {
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
				 * @brief Drops `iswspace` code units from this text.
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
				 * @brief Whitespace-separated tokens. The vector is built in the caller.
				 * @return Tokens.
				 */
				STORMBYTE_FORCE_INLINE std::vector<WString> Split() const noexcept {
					std::vector<WString> out;
					Split(static_cast<std::wstring_view>(*this), out);
					return out;
				}

				/**
				 * @brief Whitespace-separated tokens into caller-provided Safe storage.
				 * @param[out] out Destination sequence.
				 * @return Success or Failure; failure leaves @p out unchanged.
				 */
				Status Split(Vector<WString>& out) const noexcept {
					return Split(static_cast<std::wstring_view>(*this), out);
				}

				/**
				 * @brief Tokens on @p delimiter. The queue is built in the caller.
				 * @param delimiter Separator.
				 * @return Tokens, including empty ones.
				 */
				STORMBYTE_FORCE_INLINE std::queue<WString> Explode(wchar_t delimiter) const noexcept {
					std::queue<WString> out;
					Explode(static_cast<std::wstring_view>(*this), delimiter, out);
					return out;
				}

				/**
				 * @brief Tokens on @p delimiter into caller-provided Safe storage.
				 * @param delimiter Separator.
				 * @param[out] out Destination queue.
				 * @return Success or Failure; failure leaves @p out unchanged.
				 */
				Status Explode(wchar_t delimiter, Queue<WString>& out) const noexcept {
					return Explode(static_cast<std::wstring_view>(*this), delimiter, out);
				}

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
					return m_text == other.m_text;
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
				 * @brief Content equality with a wide C string.
				 * @param str May be null.
				 * @return Whether the texts are equal.
				 */
				inline bool operator==(const wchar_t* str) const noexcept {
					return m_text == str;
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
				 * @brief Content order. Null is less than any text.
				 * @param other Other text.
				 * @return Ordering.
				 */
				inline std::strong_ordering operator<=>(const WString& other) const noexcept {
					return m_text <=> other.m_text;
				}

				/**
				 * @brief Content order against a wide C string.
				 * @param str May be null.
				 * @return Ordering.
				 */
				inline std::strong_ordering operator<=>(const wchar_t* str) const noexcept {
					return m_text <=> str;
				}

				/** @} */

				/**
				 * @brief Swaps buffers with @p other.
				 * @param other Other text.
				 */
				void swap(WString& other) noexcept;

			private:
					 /**
					  * @brief Mutate a caller-owned standard wide-string snapshot and copy it back to Base.
					  * @tparam Function Modifier accepting std::wstring&.
					  * @param function Modifier to invoke.
					  * @return This string.
					  */
					 template<class Function>
					 WString& Mutate(Function&& function) {
						 std::wstring value(static_cast<std::wstring_view>(*this));
						 std::forward<Function>(function)(value);
						 *this = WString(std::wstring_view(value));
						 return *this;
					 }

				static Size FromIndex(std::size_t index) noexcept {
					return index == std::wstring_view::npos ? npos : Size{index};
				}

				template<typename Needle>
				static Size FindAt(std::wstring_view self, Needle needle, Size pos) noexcept {
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find(needle, static_cast<std::size_t>(pos)));
				}

				template<typename Needle>
				static Size RFindAt(std::wstring_view self, Needle needle, Size pos) noexcept {
					const std::size_t start = pos == npos ? std::wstring_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.rfind(needle, start));
				}

				WCString m_text;	///< Owned code units
		};

		/**
		 * @brief Writes @p text to @p stream.
		 * @param stream Destination.
		 * @param text Source.
		 * @return @p stream.
		 */
		inline std::wostream& operator<<(std::wostream& stream, const WString& text) {
				const wchar_t* bytes = text.Bytes();
				return bytes ? stream << bytes : stream;
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
		 * @brief Swaps two texts.
		 * @param left First text.
		 * @param right Second text.
		 */
		inline void swap(WString& left, WString& right) noexcept {
			left.swap(right);
		}
	}
}

/**
 * @brief Hash of the text (`0` when the view is empty and the buffer is null).
 */
template<>
struct std::hash<StormByte::Safe::WString> {
	/**
	 * @brief Hashes @p text.
	 * @param text Text.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::WString& text) const noexcept {
		return std::hash<std::wstring_view>{}(static_cast<std::wstring_view>(text));
	}
};
