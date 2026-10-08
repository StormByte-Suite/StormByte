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
		 * @defgroup TypeEnums Enumeration concepts
		 * @brief Concepts and helpers for `enum` / `enum class` types.
		 * @{
		 */

		/**
		 * @name Concepts
		 * @{
		 */

		/**
		 * @brief Unscoped or scoped enumeration (`enum` / `enum class`).
		 * @tparam E Type to test (cv stripped).
		 *
		 * @code
		 * template<Type::Enum E>
		 * void process(E value);
		 * @endcode
		 */
		template<typename E>
		concept Enum = std::is_enum_v<std::remove_cv_t<E>>;

		/**
		 * @brief @ref StormByte::Type::Enum whose underlying type is unsigned.
		 * @tparam E Type to test.
		 *
		 * @code
		 * template<Type::UnsignedEnum E>
		 * void process(E value);
		 * @endcode
		 */
		template<typename E>
		concept UnsignedEnum =
			Enum<E> &&
			Unsigned<std::underlying_type_t<std::remove_cv_t<E>>>;

		/**
		 * @brief Scoped enumeration (`enum class` / `enum struct`).
		 * @tparam E Type to test.
		 *
		 * @code
		 * template<Type::ScopedEnum E>
		 * void process(E value);
		 * @endcode
		 */
		template<typename E>
		concept ScopedEnum = std::is_scoped_enum_v<E>;

		/** @} */

		/**
		 * @name Helpers
		 * @{
		 */

		/**
		 * @brief Underlying integer type of enumeration @p E.
		 * @tparam E Enumeration type satisfying @ref StormByte::Type::Enum.
		 *
		 * @code
		 * enum class Foo : uint16_t { A };
		 * Type::UnderlyingType<Foo> value = 0;
		 * @endcode
		 */
		template<typename E>
		requires Enum<E>
		using UnderlyingType = std::underlying_type_t<std::remove_cv_t<E>>;

		/**
		 * @brief Converts an enumeration value to its underlying integer.
		 * @tparam E Enumeration type satisfying @ref StormByte::Type::Enum.
		 * @param[in] e Value to convert.
		 * @return Underlying integer representation of @p e.
		 *
		 * @code
		 * enum class Foo : int { A = 3 };
		 * auto n = Type::ToUnderlying(Foo::A); // 3
		 * @endcode
		 */
		template<typename E>
		requires Enum<E>
		constexpr UnderlyingType<E> ToUnderlying(E e) noexcept {
			return static_cast<UnderlyingType<E>>(e);
		}

		/** @} */
		/** @} */
	}
}
