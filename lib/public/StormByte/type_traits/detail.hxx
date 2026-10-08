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
#include <StormByte/type_traits/object_semantics.hxx>

#include <array>
#include <bit>
#include <cstddef>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @namespace Detail
		 * @brief Private helpers. Not a supported API.
		 *
		 * Other modules may call @ref StormByte::Type::Detail::swap_endian. Do not depend on
		 * any other name in this namespace.
		 */
		namespace Detail {
			/**
			 * @name Endian
			 * @{
			 */

			/**
			 * @brief Reverses the object-representation byte order of @p val.
			 * @tparam U Trivially copyable value type.
			 * @param[in] val Value whose bytes are reversed.
			 * @return @p val with endianness swapped.
			 *
			 * This **always** reverses bytes. Whether the caller should
			 * invoke it (host ≠ little-endian) is Serializable's job.
			 * Integrals go through `std::byteswap`; every other trivial
			 * type goes through `std::bit_cast` + reverse so `float`,
			 * `enum` and small POD structs stay defined.
			 *
			 * @warning Not a format detector. No BOM, no `std::endian` test.
			 */
			template<typename U>
			requires TriviallyCopyable<U> && (sizeof(U) > 0)
			constexpr U swap_endian(U val) noexcept {
				if constexpr (sizeof(U) == 1) {
					return val;
				} else if constexpr (Integral<U>) {
					return std::byteswap(val);
				} else {
					auto bytes = std::bit_cast<std::array<std::byte, sizeof(U)>>(val);
					for (std::size_t i = 0, j = sizeof(U); i < j; ++i) {
						--j;
						const std::byte tmp = bytes[i];
						bytes[i] = bytes[j];
						bytes[j] = tmp;
					}
					return std::bit_cast<U>(bytes);
				}
			}

			/** @} */
		}
	}
}
