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

#include <concepts>
#include <cstddef>
#include <functional>
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
		 * @defgroup TypeComparison Comparison and hashing concepts
		 * @brief Equality, ordering and `std::hash` support for a type.
		 * @{
		 */

		/**
		 * @name Comparison
		 * @{
		 */

		/**
		 * @brief Type whose `==` and `!=` both yield something convertible to `bool`.
		 * @tparam T Type to test.
		 *
		 * Not `std::equality_comparable`: this only checks same-type
		 * comparison, not the mixed-type / common-reference machinery.
		 *
		 * @code
		 * template<Type::EqualityComparable T>
		 * bool same(T const& a, T const& b) { return a == b; }
		 * @endcode
		 */
		template<typename T>
		concept EqualityComparable =
			requires(std::remove_cvref_t<T> const& a, std::remove_cvref_t<T> const& b) {
				{ a == b } -> std::convertible_to<bool>;
				{ a != b } -> std::convertible_to<bool>;
			};

		/**
		 * @brief Type whose `<=>` operator is well-formed.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::ThreeWayComparable T>
		 * auto order(T const& a, T const& b) { return a <=> b; }
		 * @endcode
		 */
		template<typename T>
		concept ThreeWayComparable =
			requires(std::remove_cvref_t<T> const& a, std::remove_cvref_t<T> const& b) {
				{ a <=> b };
			};

		/**
		 * @brief Type with a valid `std::hash` specialization.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Hashable T>
		 * std::size_t hash_of(T const& value) { return std::hash<std::remove_cvref_t<T>>{}(value); }
		 * @endcode
		 */
		template<typename T>
		concept Hashable =
			requires(std::remove_cvref_t<T> const& t) {
				{ std::hash<std::remove_cvref_t<T>>{}(t) } -> std::convertible_to<std::size_t>;
			};

		/** @} */
		/** @} */
	}
}
