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

#include <StormByte/type_traits/categories.hxx>
#include <StormByte/type_traits/conversions.hxx>
#include <StormByte/type_traits/relations.hxx>

#include <charconv>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace StormByte::Safe {
	/**
	 * @brief One `ReplaceToken` field. Empty means the historical `{}` conversion.
	 *
	 * Supported text, in any combination: a leading `0`, a width, `.` plus a
	 * precision, and a type `d`, `f`, `g`, `G`, `x` or `X`. Anything else is
	 * not a field.
	 */
	struct ReplaceTokenSpec {
		bool zero = false;
		bool width_set = false;
		unsigned width = 0;
		bool precision_set = false;
		int precision = 0;
		char type = 0;

		constexpr bool IsDefault() const noexcept {
			return !zero && !width_set && !precision_set && type == 0;
		}
	};

	/**
	 * @brief Where a field sits in the pattern, and how to render its argument.
	 * @tparam Char `char` or `wchar_t`.
	 */
	template<class Char>
	struct ReplaceTokenPlace {
		std::size_t offset = 0;
		std::size_t length = 0;
		ReplaceTokenSpec spec{};
	};

	/**
	 * @brief Parse the inside of one `{...}`.
	 * @tparam Char `char` or `wchar_t`.
	 * @param data Spec text, without the braces.
	 * @param count Number of code units.
	 * @param out Parsed spec. Untouched when this returns false.
	 * @return Whether @p data is empty or a supported spec.
	 */
	template<class Char>
	constexpr bool ParseReplaceTokenSpec(const Char* data, std::size_t count, ReplaceTokenSpec& out) noexcept {
		ReplaceTokenSpec spec;
		std::size_t index = 0;
		const auto digit = [](Char unit) noexcept {
			return unit >= static_cast<Char>('0') && unit <= static_cast<Char>('9');
		};
		if (index < count && data[index] == static_cast<Char>(':'))
			++index;
		else if (count != 0)
			return false;
		if (index < count && data[index] == static_cast<Char>('0')) {
			spec.zero = true;
			++index;
		}
		if (index < count && digit(data[index])) {
			unsigned width = 0;
			spec.width_set = true;
			while (index < count && digit(data[index])) {
				const unsigned next = static_cast<unsigned>(data[index] - static_cast<Char>('0'));
				if (width > (100000u - next) / 10u)
					return false;
				width = width * 10u + next;
				++index;
			}
			spec.width = width;
		}
		if (index < count && data[index] == static_cast<Char>('.')) {
			++index;
			spec.precision_set = true;
			int precision = 0;
			while (index < count && digit(data[index])) {
				precision = precision * 10 + static_cast<int>(data[index] - static_cast<Char>('0'));
				if (precision > 100)
					return false;
				++index;
			}
			spec.precision = precision;
		}
		if (index < count) {
			const Char type = data[index++];
			if (type != static_cast<Char>('d') && type != static_cast<Char>('f') && type != static_cast<Char>('g') && type != static_cast<Char>('G') && type != static_cast<Char>('x') && type != static_cast<Char>('X'))
				return false;
			spec.type = static_cast<char>(type);
		}
		if (index != count)
			return false;
		out = spec;
		return true;
	}

	/**
	 * @brief Collect up to @p capacity fields, in order.
	 * @tparam Char `char` or `wchar_t`.
	 * @param text Pattern.
	 * @param places Output. Only the first returned count is written.
	 * @param capacity Slots available.
	 * @return How many fields were stored. A brace run that is not a supported spec stays literal.
	 */
	template<class Char>
	std::size_t FindReplaceTokens(std::basic_string_view<Char> text, ReplaceTokenPlace<Char>* places, std::size_t capacity) noexcept {
		std::size_t found = 0;
		for (std::size_t index = 0; index < text.size() && found < capacity; ++index) {
			if (text[index] != static_cast<Char>('{'))
				continue;
			const std::size_t close = text.find(static_cast<Char>('}'), index + 1);
			if (close == std::basic_string_view<Char>::npos)
				break;
			const auto inside = text.substr(index + 1, close - index - 1);
			if (inside.find(static_cast<Char>('{')) != std::basic_string_view<Char>::npos)
				continue;
			ReplaceTokenSpec spec;
			if (!ParseReplaceTokenSpec(inside.data(), inside.size(), spec))
				continue;
			places[found] = ReplaceTokenPlace<Char>{index, close - index + 1, spec};
			++found;
			index = close;
		}
		return found;
	}

	namespace {
		inline void AppendHexDigits(std::string& out, std::uintmax_t value, bool upper) {
			if (value == 0) {
				out.push_back('0');
				return;
			}
			char digits[sizeof(std::uintmax_t) * 2];
			int count = 0;
			const char* alphabet = upper ? "0123456789ABCDEF" : "0123456789abcdef";
			while (value != 0) {
				digits[count++] = alphabet[value & 0xF];
				value >>= 4;
			}
			while (count != 0)
				out.push_back(digits[--count]);
		}

		inline void PadAscii(std::string& text, const ReplaceTokenSpec& spec) {
			if (!spec.width_set || text.size() >= spec.width)
				return;
			const std::size_t pad = spec.width - text.size();
			if (spec.zero) {
				const std::size_t at = (!text.empty() && (text[0] == '-' || text[0] == '+')) ? 1 : 0;
				text.insert(at, pad, '0');
			} else
				text.insert(0, pad, ' ');
		}

		template<class Char>
		void AssignAscii(std::basic_string<Char>& out, std::string_view ascii) {
			out.assign(ascii.begin(), ascii.end());
		}

		template<class Int>
		std::string FormatIntegerAscii(Int value, const ReplaceTokenSpec& spec) {
			std::string ascii;
			if (spec.type == 'x' || spec.type == 'X') {
				std::uintmax_t bits = 0;
				if constexpr (Type::Signed<Int>)
					bits = static_cast<std::make_unsigned_t<Int>>(value);
				else
					bits = static_cast<std::uintmax_t>(value);
				AppendHexDigits(ascii, bits, spec.type == 'X');
			} else {
				char buffer[64];
				const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value);
				ascii.assign(buffer, result.ptr);
			}
			PadAscii(ascii, spec);
			return ascii;
		}

		template<class Float>
		std::string FormatFloatAscii(Float value, const ReplaceTokenSpec& spec) {
			const int precision = spec.precision_set ? spec.precision : 6;
			std::chars_format format = std::chars_format::general;
			if (spec.type == 'f')
				format = std::chars_format::fixed;
			char buffer[128];
			const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value, format, precision);
			std::string ascii(buffer, result.ptr == buffer ? buffer : result.ptr);
			if (result.ec != std::errc{})
				ascii = std::to_string(static_cast<double>(value));
			if (spec.type == 'G') {
				for (char& unit : ascii) {
					if (unit >= 'a' && unit <= 'z')
						unit = static_cast<char>(unit - 'a' + 'A');
				}
			}
			PadAscii(ascii, spec);
			return ascii;
		}
	}

	/**
	 * @brief True when @ref FormatReplaceToken always writes @p T and never asks for a string conversion.
	 * @tparam Char `char` or `wchar_t`.
	 * @tparam T Argument type, references included.
	 *
	 * A number, a bool, a pointer and a view are in this set. `Size` is not: a bare `{}` still uses its string conversion.
	 */
	template<class Char, class T>
	concept ReplaceTokenBuiltin =
		Type::SameAs<std::remove_cvref_t<T>, bool> ||
		Type::SameAs<std::remove_cvref_t<T>, std::nullptr_t> ||
		Type::Pointer<std::remove_cvref_t<T>> ||
		Type::SmartPointer<std::remove_cvref_t<T>> ||
		(Type::SameAs<std::remove_cvref_t<T>, char> && Type::SameAs<Char, char>) ||
		(Type::SameAs<std::remove_cvref_t<T>, wchar_t> && Type::SameAs<Char, wchar_t>) ||
		Type::ConvertibleTo<std::remove_cvref_t<T>, std::basic_string_view<Char>> ||
		Type::FloatingPoint<std::remove_cvref_t<T>> ||
		Type::Integral<std::remove_cvref_t<T>>;

	/**
	 * @brief Render one argument when it does not need an owned-string conversion.
	 * @tparam Char `char` or `wchar_t`.
	 * @tparam T Argument type, including references.
	 * @param out Caller scratch. Replaced when this returns true.
	 * @param value Argument. A view is copied into @p out. A pointer is not dereferenced.
	 * @param spec Field spec. A default spec keeps the historical conversion.
	 * @return False when the caller must convert @p value through `std::string` / `String` or the wide equivalents.
	 */
	template<class Char, class T>
	bool FormatReplaceToken(std::basic_string<Char>& out, T&& value, const ReplaceTokenSpec& spec) {
		using Bare = std::remove_cvref_t<T>;
		if constexpr (Type::SameAs<Bare, bool>) {
			AssignAscii(out, value ? std::string_view("true") : std::string_view("false"));
			return true;
		} else if constexpr (Type::Pointer<Bare> && Type::SameAs<std::remove_cv_t<std::remove_pointer_t<Bare>>, Char>) {
			if (value == nullptr)
				out.clear();
			else
				out.assign(std::basic_string_view<Char>(value));
			return true;
		} else if constexpr (Type::SameAs<Bare, std::nullptr_t> || Type::Pointer<Bare> || Type::SmartPointer<Bare>) {
			const void* address = nullptr;
			if constexpr (Type::Pointer<Bare>)
				address = reinterpret_cast<const void*>(value);
			else if constexpr (Type::SmartPointer<Bare>)
				address = static_cast<const void*>(value.get());
			if (spec.IsDefault()) {
				std::string ascii = "0x";
				AppendHexDigits(ascii, reinterpret_cast<std::uintptr_t>(address), false);
				AssignAscii(out, ascii);
			} else
				AssignAscii(out, FormatIntegerAscii(reinterpret_cast<std::uintptr_t>(address), spec));
			return true;
		} else if constexpr (Type::SameAs<Bare, char> && Type::SameAs<Char, char>) {
			if (spec.IsDefault())
				out.assign(1, static_cast<char>(value));
			else
				AssignAscii(out, FormatIntegerAscii(static_cast<int>(static_cast<unsigned char>(value)), spec));
			return true;
		} else if constexpr (Type::SameAs<Bare, wchar_t> && Type::SameAs<Char, wchar_t>) {
			if (spec.IsDefault())
				out.assign(1, static_cast<wchar_t>(value));
			else
				AssignAscii(out, FormatIntegerAscii(static_cast<std::uint32_t>(value), spec));
			return true;
		} else if constexpr (Type::ConvertibleTo<Bare, std::basic_string_view<Char>>) {
			out.assign(std::basic_string_view<Char>(std::forward<T>(value)));
			return true;
		} else if constexpr (Type::FloatingPoint<Bare>) {
			if (spec.IsDefault()) {
				if constexpr (Type::SameAs<Char, wchar_t>)
					out = std::to_wstring(value);
				else
					out = std::to_string(value);
			} else if (spec.type == 'x' || spec.type == 'X' || spec.type == 'd')
				AssignAscii(out, FormatIntegerAscii(static_cast<long long>(value), spec));
			else
				AssignAscii(out, FormatFloatAscii(value, spec));
			return true;
		} else if constexpr (Type::Integral<Bare>) {
			if (spec.IsDefault()) {
				if constexpr (Type::SameAs<Char, wchar_t>)
					out = std::to_wstring(value);
				else
					out = std::to_string(value);
			} else if (spec.type == 'f' || spec.type == 'g' || spec.type == 'G')
				AssignAscii(out, FormatFloatAscii(static_cast<double>(value), spec));
			else
				AssignAscii(out, FormatIntegerAscii(value, spec));
			return true;
		} else if constexpr (Type::ExplicitlyConvertibleTo<Bare, std::uint64_t>) {
			if (spec.IsDefault())
				return false;
			AssignAscii(out, FormatIntegerAscii(static_cast<std::uint64_t>(std::forward<T>(value)), spec));
			return true;
		} else
			return false;
	}
}
