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

#include <cctype>
#include <cwctype>
#include <string>
#include <string_view>
#include <type_traits>

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
		 * @namespace Text
		 * @brief Private algorithms shared by @ref String and @ref WString.
		 */
		namespace Text {
			/**
			 * @brief Whether @p c is whitespace for @p Char.
			 * @tparam Char `char` or `wchar_t`.
			 * @param c Code unit.
			 * @return Whether it is space.
			 */
			template<typename Char>
			inline bool IsSpace(Char c) noexcept {
				if constexpr (std::is_same_v<Char, char>)
					return std::isspace(static_cast<unsigned char>(c)) != 0;
				else
					return std::iswspace(static_cast<wint_t>(c)) != 0;
			}

			/**
			 * @brief Whether @p c is an ASCII digit for @p Char.
			 * @tparam Char `char` or `wchar_t`.
			 * @param c Code unit.
			 * @return Whether it is a digit.
			 */
			template<typename Char>
			inline bool IsDigit(Char c) noexcept {
				if constexpr (std::is_same_v<Char, char>)
					return std::isdigit(static_cast<unsigned char>(c)) != 0;
				else
					return std::iswdigit(static_cast<wint_t>(c)) != 0;
			}

			/**
			 * @brief CR LF → LF.
			 * @tparam Char `char` or `wchar_t`.
			 * @param str Source.
			 * @return New owned characters.
			 */
			template<typename Char>
			inline std::basic_string<Char> SanitizeNewlines(std::basic_string_view<Char> str) noexcept {
				std::basic_string<Char> out;
				out.reserve(str.size());
				const Char cr = static_cast<Char>('\r');
				const Char lf = static_cast<Char>('\n');
				for (std::size_t i = 0; i < str.size(); ++i) {
					if (str[i] == cr && i + 1 < str.size() && str[i + 1] == lf) {
						out.push_back(lf);
						++i;
					} else
						out.push_back(str[i]);
				}
				return out;
			}

			/**
			 * @brief Drops whitespace code units.
			 * @tparam Char `char` or `wchar_t`.
			 * @param str Source.
			 * @return New owned characters.
			 */
			template<typename Char>
			inline std::basic_string<Char> RemoveWhitespace(std::basic_string_view<Char> str) noexcept {
				std::basic_string<Char> out;
				out.reserve(str.size());
				for (Char c : str) {
					if (!IsSpace(c))
						out.push_back(c);
				}
				return out;
			}

			/**
			 * @brief Optional sign plus digits.
			 * @tparam Char `char` or `wchar_t`.
			 * @param str Source.
			 * @return Whether @p str is an integer token.
			 */
			template<typename Char>
			inline bool IsInteger(std::basic_string_view<Char> str) noexcept {
				if (str.empty())
					return false;

				std::size_t start = 0;
				if (str[0] == static_cast<Char>('-') || str[0] == static_cast<Char>('+')) {
					if (str.size() == 1)
						return false;
					start = 1;
				}

				for (std::size_t i = start; i < str.size(); ++i) {
					if (!IsDigit(str[i]))
						return false;
				}
				return true;
			}
		}
	}
}
