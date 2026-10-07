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

#include <string>
#include <string_view>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Creator-owned storage and DLL-safe resource wrappers.
	 */
	namespace Safe {
		/**
		 * @namespace Utf8
		 * @brief Private UTF-8 ↔ wide conversion and Latin-1 case mapping.
		 */
		namespace Utf8 {
			/**
			 * @brief UTF-8 from wide code units. Ill-formed input becomes U+FFFD.
			 * @param str Wide text.
			 * @return Owned UTF-8.
			 */
			std::string FromWide(std::wstring_view str) noexcept;

			/**
			 * @brief Wide code units from UTF-8. Ill-formed input becomes U+FFFD.
			 * @param str UTF-8 text.
			 * @return Owned wide text.
			 */
			std::wstring ToWide(std::string_view str) noexcept;

			/**
			 * @brief ASCII + Latin-1 upper case of UTF-8 text.
			 * @param str Source.
			 * @return Owned UTF-8.
			 */
			std::string ToUpper(std::string_view str) noexcept;

			/**
			 * @brief ASCII + Latin-1 lower case of UTF-8 text.
			 * @param str Source.
			 * @return Owned UTF-8.
			 */
			std::string ToLower(std::string_view str) noexcept;

			/**
			 * @brief ASCII + Latin-1 upper case of wide text.
			 * @param str Source.
			 * @return Owned wide text.
			 */
			std::wstring ToUpper(std::wstring_view str) noexcept;

			/**
			 * @brief ASCII + Latin-1 lower case of wide text.
			 * @param str Source.
			 * @return Owned wide text.
			 */
			std::wstring ToLower(std::wstring_view str) noexcept;
		}
	}
}
