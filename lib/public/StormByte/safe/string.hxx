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

#include <StormByte/safe/pointers.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/visibility.h>

#include <cctype>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
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
	 * @brief Owned UTF-8 and wide text. Short UTF-8 lives in the object; the long path is Base-owned SDS.
	 */
	namespace Safe {
		/**
		 * @class WString
		 * @brief Wide counterpart of @ref String. Defined in wstring.hxx.
		 */
		class WString;

		/**
		 * @class String
		 * @brief Owned UTF-8 text.
		 *
		 * Not a `std::string`. Up to 22 bytes live in the object, NUL-terminated, with an explicit size.
		 * Longer text is an SDS buffer freed in Base. There is no null state: a moved-from string is empty.
		 * `data()` and `c_str()` are never null.
		 *
		 * Classic and ranges algorithms can read or modify existing bytes; they cannot change the owned size.
		 * `operator std::string_view` is implicit and inline. `operator std::string` is explicit and
		 * `STORMBYTE_FORCE_INLINE` (caller heap).
		 *
		 * Conversion to @ref WString is explicit and runs in the module (UTF-8 to wide). Conversion from
		 * @ref WString copies UTF-8 in the module.
		 *
		 * `ToUpper` / `ToLower` map only ASCII `A–Z` / `a–z`. Other well-formed UTF-8 code points are copied.
		 * Ill-formed bytes are copied one-by-one so a sequence is never split in the middle of a valid character.
		 *
		 * Observers follow `std::string_view`. Size-changing modifiers follow `std::string` value semantics.
		 * `capacity()` and `reserve(Size)` count UTF-8 bytes excluding NUL; reserve never shrinks.
		 */
		class STORMBYTE_PUBLIC String {
			public:
				using value_type = char;	///< Byte type
				using size_type = Size;	///< Byte count and position type
				using difference_type = std::ptrdiff_t;	///< Iterator distance type
				using pointer = char*;	///< Mutable buffer pointer
				using const_pointer = const char*;	///< Read-only buffer pointer
				using reference = char&;	///< Mutable character reference
				using const_reference = const char&;	///< Read-only character reference
				using iterator = char*;	///< Mutable contiguous iterator
				using const_iterator = const char*;	///< Contiguous observer
				using reverse_iterator = std::reverse_iterator<iterator>;	///< Mutable reverse observer
				using const_reverse_iterator = std::reverse_iterator<const_iterator>;	///< Reverse observer

				static constexpr std::size_t SSO_CAPACITY = 22;	///< Short-text capacity, excluding the trailing NUL

				/**
				 * @name Life
				 * @{
				 */

				/**
				 * @brief Empty text. Not null.
				 */
				String() noexcept;

				/**
				 * @brief Copies a C string. A null pointer becomes empty.
				 * @param str Source; may be null.
				 */
				explicit String(const char* str) noexcept;

				/**
				 * @brief Copies a view into owned storage.
				 * @param str Source. Embedded NUL counts.
				 */
				explicit String(std::string_view str) noexcept;

				/**
				 * @brief UTF-8 from wide text.
				 * @param other Wide source.
				 */
				explicit String(const WString& other) noexcept;

				/**
				 * @brief Copy constructor.
				 * @param other Text to copy.
				 */
				String(const String& other) noexcept;

				/**
				 * @brief Move constructor.
				 * @param other Text to take. @p other becomes empty.
				 */
				String(String&& other) noexcept;

				/**
				 * @brief Releases a long SDS buffer. A short string has nothing to free.
				 */
				~String() noexcept;

				/**
				 * @brief Copy assignment.
				 * @param other Text to copy.
				 * @return *this.
				 */
				String& operator=(const String& other) noexcept;

				/**
				 * @brief Move assignment.
				 * @param other Text to take. @p other becomes empty.
				 * @return *this.
				 */
				String& operator=(String&& other) noexcept;

				/**
				 * @brief Copy text from a caller-owned view.
				 * @param text Source bytes.
				 * @return This string.
				 */
				String& operator=(std::string_view text);

				/** @} */

				/**
				 * @name Range
				 * @{
				 */

				/**
				 * @brief First character.
				 * @return Iterator. Never null.
				 */
				inline iterator begin() noexcept {
					return data();
				}

				/**
				 * @brief First character.
				 * @return Read-only iterator. Never null.
				 */
				inline const_iterator begin() const noexcept {
					return data();
				}

				/**
				 * @brief One past the last character.
				 * @return Iterator. Never null.
				 */
				inline iterator end() noexcept {
					return data() + static_cast<std::size_t>(size());
				}

				/**
				 * @brief One past the last character.
				 * @return Read-only iterator. Never null.
				 */
				inline const_iterator end() const noexcept {
					return data() + static_cast<std::size_t>(size());
				}

				/**
				 * @brief First character.
				 * @return Iterator.
				 */
				inline const_iterator cbegin() const noexcept {
					return begin();
				}

				/**
				 * @brief One past the last character.
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
				 * @brief Contiguous pointer to the first byte, or to the trailing NUL when empty.
				 * @return Never null.
				 */
				char* data() noexcept;

				/**
				 * @brief Read-only contiguous pointer to the first byte, or to the trailing NUL when empty.
				 * @return Never null.
				 */
				const char* data() const noexcept;

				/**
				 * @brief Same pointer as @ref data. The byte at `size()` is NUL and is not part of the length.
				 * @return Never null.
				 */
				inline const char* c_str() const noexcept {
					return data();
				}

				/**
				 * @brief Byte count, excluding the trailing NUL.
				 * @return Length as @ref StormByte::Size.
				 */
				Size size() const noexcept;

				/**
				 * @brief Allocated byte capacity, excluding the trailing NUL.
				 * @return 22 for a short string; SDS capacity for a long one.
				 */
				Size capacity() const noexcept;

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
				 * @brief Append UTF-8 bytes.
				 * @param text Text to append.
				 * @return This string.
				 */
				String& append(std::string_view text);

				/**
				 * @brief Append count copies of a byte.
				 * @param count Number of copies.
				 * @param character Byte to append.
				 * @return This string.
				 */
				String& append(size_type count, char character);

				/**
				 * @brief Replace the contents with a text view.
				 * @param text Source bytes.
				 * @return This string.
				 */
				String& assign(std::string_view text);

				/**
				 * @brief Replace the contents with count copies of a byte.
				 * @param count Number of copies.
				 * @param character Byte to assign.
				 * @return This string.
				 */
				String& assign(size_type count, char character);

				/**
				 * @brief Append a view.
				 * @param text Text to append.
				 * @return This string.
				 */
				String& operator+=(std::string_view text);

				/**
				 * @brief Append one byte.
				 * @param character Byte to append.
				 * @return This string.
				 */
				String& operator+=(char character);

				/**
				 * @brief Append one byte.
				 * @param character Byte to append.
				 */
				void push_back(char character);

				/**
				 * @brief Remove the final byte. Empty use follows `std::string` preconditions.
				 */
				void pop_back();

				/**
				 * @brief Replace the contents with an empty string, retaining long-path capacity.
				 */
				void clear();

				/**
				 * @brief Reserve storage for at least @p new_capacity UTF-8 bytes.
				 * @param new_capacity Requested byte capacity, excluding the NUL.
				 * @return Nothing.
				 * @throw OutOfBoundsError The request cannot be represented, including a trailing NUL.
				 * @throw AllocationError The size is representable but the SDS allocation failed.
				 * @note Requests at or below capacity do not shrink or reallocate.
				 */
				void reserve(size_type new_capacity);

				/**
				 * @brief Resize the byte sequence, filling new bytes with character.
				 * @param count New byte count.
				 * @param character Fill byte, default initialized to NUL.
				 */
				void resize(size_type count, char character = char{});

				/**
				 * @brief Insert bytes at a byte position.
				 * @param position Insertion position.
				 * @param text Bytes to insert.
				 * @return This string.
				 */
				String& insert(size_type position, std::string_view text);

				/**
				 * @brief Erase bytes starting at position.
				 * @param position First byte to erase.
				 * @param count Maximum bytes to erase.
				 * @return This string.
				 */
				String& erase(size_type position = {}, size_type count = npos);

				/**
				 * @brief Replace a byte range with text.
				 * @param position First byte to replace.
				 * @param count Maximum bytes to erase.
				 * @param text Replacement bytes.
				 * @return This string.
				 */
				String& replace(size_type position, size_type count, std::string_view text);

				/**
				 * @brief Character at @p index.
				 * @param index Position in `[0, size()]`. `size()` is the trailing NUL.
				 * @return Character.
				 * @note `index > size()` is undefined and `assert`s when assertions are on.
				 */
				inline char& operator[](const Size& index) noexcept {
					assert(index <= size());
					return data()[static_cast<std::size_t>(index)];
				}

				/**
				 * @brief Read-only character at @p index.
				 * @param index Position in `[0, size()]`. `size()` is the trailing NUL.
				 * @return Character copy.
				 * @note `index > size()` is undefined and `assert`s when assertions are on.
				 */
				inline char operator[](const Size& index) const noexcept {
					assert(index <= size());
					return data()[static_cast<std::size_t>(index)];
				}

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
				 * @return View of `size()` bytes. Embedded NUL is preserved. Never a null buffer.
				 * @note Same lifetime as `std::string::c_str()`. Invalidated by a mutating operation that reallocates.
				 */
				inline operator std::string_view() const noexcept {
					return std::string_view(data(), static_cast<std::size_t>(size()));
				}

				/**
				 * @brief Copy of the text in the caller’s heap.
				 * @return Caller-owned `std::string`.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::string() const {
					return std::string(static_cast<std::string_view>(*this));
				}

				/**
				 * @brief View of the owned buffer.
				 * @return `c_str()`. Never null.
				 * @note Same lifetime as `std::string::c_str()`.
				 */
				inline explicit operator const char*() const noexcept {
					return c_str();
				}

				/**
				 * @brief Wide text (UTF-8 decoded in the module).
				 * @return Owned @ref WString.
				 */
				explicit operator WString() const noexcept;

				/**
				 * @brief Non-owning view of the owned bytes.
				 * @return `data()`. Never null.
				 */
				inline const char* Bytes() const noexcept {
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
				inline bool starts_with(std::string_view text) const noexcept {
					return static_cast<std::string_view>(*this).starts_with(text);
				}

				/**
				 * @brief Whether the text begins with @p ch.
				 * @param ch Prefix byte.
				 * @return Match.
				 */
				inline bool starts_with(char ch) const noexcept {
					return static_cast<std::string_view>(*this).starts_with(ch);
				}

				/**
				 * @brief Whether the text ends with @p text.
				 * @param text Suffix.
				 * @return Match. Empty suffix matches.
				 */
				inline bool ends_with(std::string_view text) const noexcept {
					return static_cast<std::string_view>(*this).ends_with(text);
				}

				/**
				 * @brief Whether the text ends with @p ch.
				 * @param ch Suffix byte.
				 * @return Match.
				 */
				inline bool ends_with(char ch) const noexcept {
					return static_cast<std::string_view>(*this).ends_with(ch);
				}

				/**
				 * @brief Whether @p text occurs.
				 * @param text Needle.
				 * @return Match. Empty needle matches.
				 */
				inline bool contains(std::string_view text) const noexcept {
					return static_cast<std::string_view>(*this).contains(text);
				}

				/**
				 * @brief Whether @p ch occurs.
				 * @param ch Byte.
				 * @return Match.
				 */
				inline bool contains(char ch) const noexcept {
					return static_cast<std::string_view>(*this).contains(ch);
				}

				/**
				 * @brief First occurrence of @p text at or after @p pos.
				 * @param text Needle.
				 * @param pos Start, in bytes.
				 * @return Index, or @ref npos.
				 */
				inline Size find(std::string_view text, Size pos = {}) const noexcept {
					return FindAt(static_cast<std::string_view>(*this), text, pos);
				}

				/**
				 * @brief First occurrence of @p ch at or after @p pos.
				 * @param ch Byte.
				 * @param pos Start, in bytes.
				 * @return Index, or @ref npos.
				 */
				inline Size find(char ch, Size pos = {}) const noexcept {
					return FindAt(static_cast<std::string_view>(*this), ch, pos);
				}

				/**
				 * @brief First occurrence of @p count bytes of @p text.
				 * @param text Needle. May be null when @p count is zero.
				 * @param pos Start, in bytes.
				 * @param count Bytes of @p text to use.
				 * @return Index, or @ref npos.
				 */
				inline Size find(const char* text, Size pos, Size count) const noexcept {
					return find(std::string_view(text, static_cast<std::size_t>(count)), pos);
				}

				/**
				 * @brief Last occurrence of @p text at or before @p pos.
				 * @param text Needle.
				 * @param pos Highest start, in bytes. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(std::string_view text, Size pos = npos) const noexcept {
					return RFindAt(static_cast<std::string_view>(*this), text, pos);
				}

				/**
				 * @brief Last occurrence of @p ch at or before @p pos.
				 * @param ch Byte.
				 * @param pos Highest start, in bytes. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(char ch, Size pos = npos) const noexcept {
					return RFindAt(static_cast<std::string_view>(*this), ch, pos);
				}

				/**
				 * @brief Last occurrence of @p count bytes of @p text.
				 * @param text Needle. May be null when @p count is zero.
				 * @param pos Highest start, in bytes.
				 * @param count Bytes of @p text to use.
				 * @return Index, or @ref npos.
				 */
				inline Size rfind(const char* text, Size pos, Size count) const noexcept {
					return rfind(std::string_view(text, static_cast<std::size_t>(count)), pos);
				}

				/**
				 * @brief First byte that is in @p text, at or after @p pos.
				 * @param text Set of bytes.
				 * @param pos Start, in bytes.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_of(std::string_view text, Size pos = {}) const noexcept {
					const std::string_view self = *this;
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find_first_of(text, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief First @p ch at or after @p pos.
				 * @param ch Byte.
				 * @param pos Start, in bytes.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_of(char ch, Size pos = {}) const noexcept {
					return find(ch, pos);
				}

				/**
				 * @brief Last byte that is in @p text, at or before @p pos.
				 * @param text Set of bytes.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_of(std::string_view text, Size pos = npos) const noexcept {
					const std::string_view self = *this;
					const std::size_t start = pos == npos ? std::string_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.find_last_of(text, start));
				}

				/**
				 * @brief Last @p ch at or before @p pos.
				 * @param ch Byte.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_of(char ch, Size pos = npos) const noexcept {
					return rfind(ch, pos);
				}

				/**
				 * @brief First byte that is not in @p text, at or after @p pos.
				 * @param text Set of bytes.
				 * @param pos Start, in bytes.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_not_of(std::string_view text, Size pos = {}) const noexcept {
					const std::string_view self = *this;
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find_first_not_of(text, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief First byte other than @p ch, at or after @p pos.
				 * @param ch Byte.
				 * @param pos Start, in bytes.
				 * @return Index, or @ref npos.
				 */
				inline Size find_first_not_of(char ch, Size pos = {}) const noexcept {
					const std::string_view self = *this;
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find_first_not_of(ch, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief Last byte that is not in @p text, at or before @p pos.
				 * @param text Set of bytes.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_not_of(std::string_view text, Size pos = npos) const noexcept {
					const std::string_view self = *this;
					const std::size_t start = pos == npos ? std::string_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.find_last_not_of(text, start));
				}

				/**
				 * @brief Last byte other than @p ch, at or before @p pos.
				 * @param ch Byte.
				 * @param pos Highest index. Default is the end.
				 * @return Index, or @ref npos.
				 */
				inline Size find_last_not_of(char ch, Size pos = npos) const noexcept {
					const std::string_view self = *this;
					const std::size_t start = pos == npos ? std::string_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.find_last_not_of(ch, start));
				}

				/**
				 * @brief Copy of a slice. Not a view.
				 * @param pos Start, in bytes.
				 * @param count Length. @ref npos means through the end.
				 * @return Owned text. Empty when @p pos is past @ref size. Does not throw.
				 */
				inline String substr(Size pos = {}, Size count = npos) const noexcept {
					const std::string_view text = *this;
					if (pos > Size{text.size()})
						return String();
					const std::size_t n = count == npos ? std::string_view::npos : static_cast<std::size_t>(count);
					return String(text.substr(static_cast<std::size_t>(pos), n));
				}

				/**
				 * @brief Same order as `std::string_view::compare`.
				 * @param text Other text.
				 * @return Negative, zero, or positive.
				 */
				inline int compare(std::string_view text) const noexcept {
					return static_cast<std::string_view>(*this).compare(text);
				}

				/**
				 * @brief First byte.
				 * @return Byte.
				 * @note Empty is undefined, same as `std::string::front`.
				 */
				inline char& front() noexcept {
					return (*this)[Size{0}];
				}

				/**
				 * @brief Read the first byte.
				 * @return Byte copy.
				 * @note Empty is undefined, same as `std::string::front`.
				 */
				inline char front() const noexcept {
					return (*this)[Size{0}];
				}

				/**
				 * @brief Last byte.
				 * @return Byte.
				 * @note Empty is undefined, same as `std::string::back`.
				 */
				inline char& back() noexcept {
					return (*this)[size() - Size{1}];
				}

				/**
				 * @brief Read the last byte.
				 * @return Byte copy.
				 * @note Empty is undefined, same as `std::string::back`.
				 */
				inline char back() const noexcept {
					return (*this)[size() - Size{1}];
				}

				/** @} */

				/**
				 * @name Helpers
				 * @{
				 */

				/**
				 * @brief ASCII-letter lower case; UTF-8 otherwise copied.
				 * @param str Source.
				 * @return New text.
				 */
				static String ToLower(std::string_view str) noexcept;

				/**
				 * @brief ASCII-letter upper case; UTF-8 otherwise copied.
				 * @param str Source.
				 * @return New text.
				 */
				static String ToUpper(std::string_view str) noexcept;

				/**
				 * @brief Turns CR LF into LF.
				 * @param str Source.
				 * @return New text.
				 */
				static String SanitizeNewlines(std::string_view str) noexcept;

				/**
				 * @brief Drops `isspace` bytes.
				 * @param str Source.
				 * @return New text.
				 */
				static String RemoveWhitespace(std::string_view str) noexcept;

				/**
				 * @brief Optional sign plus ASCII digits.
				 * @param str Source.
				 * @return Whether @p str is an integer token.
				 */
				static bool IsInteger(std::string_view str) noexcept;

				/**
				 * @brief Whitespace-separated tokens. @p out is the caller’s container.
				 * @param str Source.
				 * @param[out] out Tokens.
				 * @note `STORMBYTE_FORCE_INLINE` so the container nodes are allocated in the caller.
				 */
				static STORMBYTE_FORCE_INLINE void Split(std::string_view str, std::vector<String>& out) noexcept {
					out.clear();
					std::size_t i = 0;
					while (i < str.size()) {
						while (i < str.size() && std::isspace(static_cast<unsigned char>(str[i])) != 0)
							++i;
						if (i >= str.size())
							break;
						std::size_t j = i;
						while (j < str.size() && std::isspace(static_cast<unsigned char>(str[j])) == 0)
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
				static Status Split(std::string_view str, Vector<String>& out) noexcept;

				/**
				 * @brief Tokens on @p delimiter. @p out is the caller’s container.
				 * @param str Source.
				 * @param delimiter Separator.
				 * @param[out] out Tokens, including empty ones.
				 * @note `STORMBYTE_FORCE_INLINE` so the container nodes are allocated in the caller.
				 */
				static STORMBYTE_FORCE_INLINE void Explode(std::string_view str, char delimiter, std::queue<String>& out) noexcept {
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
				static Status Explode(std::string_view str, char delimiter, Queue<String>& out) noexcept;

				/**
				 * @brief ASCII-letter lower case of this text.
				 * @return New text.
				 */
				inline String ToLower() const noexcept {
					return ToLower(static_cast<std::string_view>(*this));
				}

				/**
				 * @brief ASCII-letter upper case of this text.
				 * @return New text.
				 */
				inline String ToUpper() const noexcept {
					return ToUpper(static_cast<std::string_view>(*this));
				}

				/**
				 * @brief CR LF to LF on this text.
				 * @return New text.
				 */
				inline String SanitizeNewlines() const noexcept {
					return SanitizeNewlines(static_cast<std::string_view>(*this));
				}

				/**
				 * @brief Drops `isspace` bytes from this text.
				 * @return New text.
				 */
				inline String RemoveWhitespace() const noexcept {
					return RemoveWhitespace(static_cast<std::string_view>(*this));
				}

				/**
				 * @brief Whether this text is an integer token.
				 * @return Whether it matches @ref IsInteger(std::string_view).
				 */
				inline bool IsInteger() const noexcept {
					return IsInteger(static_cast<std::string_view>(*this));
				}

				/**
				 * @brief Whitespace-separated tokens. The vector is built in the caller.
				 * @return Tokens.
				 */
				STORMBYTE_FORCE_INLINE std::vector<String> Split() const noexcept {
					std::vector<String> out;
					Split(static_cast<std::string_view>(*this), out);
					return out;
				}

				/**
				 * @brief Whitespace-separated tokens into caller-provided Safe storage.
				 * @param[out] out Destination sequence.
				 * @return Success or Failure; failure leaves @p out unchanged.
				 */
				Status Split(Vector<String>& out) const noexcept;

				/**
				 * @brief Tokens on @p delimiter. The queue is built in the caller.
				 * @param delimiter Separator.
				 * @return Tokens, including empty ones.
				 */
				STORMBYTE_FORCE_INLINE std::queue<String> Explode(char delimiter) const noexcept {
					std::queue<String> out;
					Explode(static_cast<std::string_view>(*this), delimiter, out);
					return out;
				}

				/**
				 * @brief Tokens on @p delimiter into caller-provided Safe storage.
				 * @param delimiter Separator.
				 * @param[out] out Destination queue.
				 * @return Success or Failure; failure leaves @p out unchanged.
				 */
				Status Explode(char delimiter, Queue<String>& out) const noexcept;

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
				inline bool operator==(const String& other) const noexcept {
					return static_cast<std::string_view>(*this) == static_cast<std::string_view>(other);
				}

				/**
				 * @brief Content inequality.
				 * @param other Other text.
				 * @return Whether the texts differ.
				 */
				inline bool operator!=(const String& other) const noexcept {
					return !(*this == other);
				}

				/**
				 * @brief Content equality with a C string. A null pointer compares equal to empty.
				 * @param str May be null.
				 * @return Whether the texts are equal.
				 */
				inline bool operator==(const char* str) const noexcept {
					return str ? static_cast<std::string_view>(*this) == std::string_view(str) : empty();
				}

				/**
				 * @brief Content inequality with a C string.
				 * @param str May be null.
				 * @return Whether the texts differ.
				 */
				inline bool operator!=(const char* str) const noexcept {
					return !(*this == str);
				}

				/**
				 * @brief Content order.
				 * @param other Other text.
				 * @return Ordering.
				 */
				inline std::strong_ordering operator<=>(const String& other) const noexcept {
					return static_cast<std::string_view>(*this) <=> static_cast<std::string_view>(other);
				}

				/**
				 * @brief Content order against a C string. A null pointer compares as empty.
				 * @param str May be null.
				 * @return Ordering.
				 */
				inline std::strong_ordering operator<=>(const char* str) const noexcept {
					return str ? static_cast<std::string_view>(*this) <=> std::string_view(str)
						: static_cast<std::string_view>(*this) <=> std::string_view{};
				}

				/** @} */

				/**
				 * @brief Swaps storage with @p other.
				 * @param other Other text.
				 */
				void swap(String& other) noexcept;

			private:
				/**
				 * @brief In-object short buffer, or an SDS pointer when bit 7 of @ref m_tag is set.
				 *
				 * `data` holds 22 bytes plus a trailing NUL. `m_tag` bits 0–6 are the short size.
				 * `long_` is active only while the long bit is set, and is freed in Base.
				 */
				struct alignas(alignof(void*)) Storage {
					union {
						char data[SSO_CAPACITY + 1];	///< Short bytes, or overlaid by @ref long_
						char* long_;	///< SDS pointer. Active only when the long bit is set
					};
					std::uint8_t m_tag;	///< Bit 7 long; bits 0–6 short size
				};

				static_assert(sizeof(Storage) == 32);
				static_assert(alignof(Storage) == alignof(void*));

				/**
				 * @brief Whether the SDS pointer is the active member.
				 * @return Long-path flag.
				 */
				bool IsLong() const noexcept;

				/**
				 * @brief Free the SDS buffer, if any, and leave an empty short string.
				 */
				void ReleaseLong() noexcept;

				/**
				 * @brief Leave an empty short string. Does not free.
				 */
				void SetEmpty() noexcept;

				/**
				 * @brief Store a short sequence and its explicit size. `count` must be at most @ref SSO_CAPACITY.
				 * @param text Source bytes. May contain embedded NUL.
				 * @param count Byte count, excluding the trailing NUL written at `data[count]`.
				 */
				void SetShort(const char* text, std::size_t count) noexcept;

				/**
				 * @brief Convert a standard view index into the public position type.
				 * @param index Standard position or not-found sentinel.
				 * @return Public position or @ref npos.
				 */
				static Size FromIndex(std::size_t index) noexcept {
					return index == std::string_view::npos ? npos : Size{index};
				}

				/**
				 * @brief Find a needle from a checked starting position.
				 * @tparam Needle Byte or text view.
				 * @param self Text to search.
				 * @param needle Bytes to find.
				 * @param pos Starting position.
				 * @return Match position or @ref npos.
				 */
				template<typename Needle>
				static Size FindAt(std::string_view self, Needle needle, Size pos) noexcept {
					if (pos > Size{self.size()})
						return npos;
					return FromIndex(self.find(needle, static_cast<std::size_t>(pos)));
				}

				/**
				 * @brief Find the last needle at or before a position.
				 * @tparam Needle Byte or text view.
				 * @param self Text to search.
				 * @param needle Bytes to find.
				 * @param pos Highest starting position.
				 * @return Match position or @ref npos.
				 */
				template<typename Needle>
				static Size RFindAt(std::string_view self, Needle needle, Size pos) noexcept {
					const std::size_t start = pos == npos ? std::string_view::npos : static_cast<std::size_t>(pos);
					return FromIndex(self.rfind(needle, start));
				}

				Storage m_storage{};	///< Short buffer or SDS pointer
		};

		/**
		 * @brief Writes @p text to @p stream.
		 * @param stream Destination.
		 * @param text Source.
		 * @return @p stream.
		 */
		inline std::ostream& operator<<(std::ostream& stream, const String& text) {
			return stream << static_cast<std::string_view>(text);
		}

		/**
		 * @brief Content equality.
		 * @param str C string; may be null.
		 * @param text Text.
		 * @return Whether the texts are equal.
		 */
		inline bool operator==(const char* str, const String& text) noexcept {
			return text == str;
		}

		/**
		 * @brief Content inequality.
		 * @param str C string; may be null.
		 * @param text Text.
		 * @return Whether the texts differ.
		 */
		inline bool operator!=(const char* str, const String& text) noexcept {
			return text != str;
		}

		/**
		 * @brief Swaps two texts.
		 * @param left First text.
		 * @param right Second text.
		 */
		inline void swap(String& left, String& right) noexcept {
			left.swap(right);
		}
	}
}

/**
 * @brief Hash of the text. An empty string hashes as an empty view.
 */
template<>
struct std::hash<StormByte::Safe::String> {
	/**
	 * @brief Hashes @p text.
	 * @param text Text.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::String& text) const noexcept {
		return std::hash<std::string_view>{}(static_cast<std::string_view>(text));
	}
};
