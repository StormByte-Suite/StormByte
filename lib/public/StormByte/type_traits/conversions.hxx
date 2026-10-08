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

#include <type_traits>

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
		 * @defgroup TypeConversions Type conversion concepts
		 * @brief Implicit and explicit convertibility predicates.
		 * @{
		 */

		/**
		 * @name Conversion
		 * @{
		 */

		/**
		 * @brief @p From is implicitly convertible to @p To (`std::is_convertible`).
		 * @tparam From Source type.
		 * @tparam To Destination type.
		 *
		 * Not `std::convertible_to`: that also requires an explicit
		 * `To` construct from `From` and would tighten the contract.
		 */
		template<typename From, typename To>
		concept ConvertibleTo = std::is_convertible_v<From, To>;

		/**
		 * @brief @p From may be explicitly converted to @p To with `static_cast`.
		 * @tparam From Source type.
		 * @tparam To Destination type.
		 *
		 * Accepts both implicit and explicit conversions. Unlike
		 * @ref StormByte::Type::ConvertibleTo, this matches types with an explicit conversion
		 * operator or constructor.
		 */
		template<typename From, typename To>
		concept ExplicitlyConvertibleTo = requires(From value) {
			static_cast<To>(value);
		};

		/** @} */
		/** @} */
	}
}
