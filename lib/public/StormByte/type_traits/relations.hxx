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
		 * @defgroup TypeRelations Type relation concepts
		 * @brief Relationships between two (or more) types: invocability, equivalence, convertibility.
		 * @{
		 */

		/**
		 * @name Relations
		 * @{
		 */

		/**
		 * @brief Invocable with argument types @p Args (`std::is_invocable`).
		 * @tparam F Callable type.
		 * @tparam Args Argument types passed to `F`.
		 *
		 * @code
		 * template<typename F, typename... Args>
		 * requires Type::Callable<F, Args...>
		 * auto invoke(F&& func, Args&&... args);
		 * @endcode
		 */
		template<typename F, typename... Args>
		concept Callable = std::is_invocable_v<F, Args...>;

		/**
		 * @brief Regularly invocable with argument types @p Args (`std::invocable`).
		 * @tparam F Callable type.
		 * @tparam Args Argument types passed to `F`.
		 *
		 * Not @ref StormByte::Type::Callable. That only checks `std::is_invocable`. This also requires the invocation to be equality-preserving.
		 *
		 * @code
		 * template<typename F, typename... Args>
		 * requires Type::Invocable<F, Args...>
		 * decltype(auto) call(F&& func, Args&&... args);
		 * @endcode
		 */
		template<typename F, typename... Args>
		concept Invocable = std::invocable<F, Args...>;

		/**
		 * @brief Invocable with argument types @p Args and a result convertible to @p R (`std::is_invocable_r`).
		 * @tparam R Expected result type.
		 * @tparam F Callable type.
		 * @tparam Args Argument types passed to `F`.
		 *
		 * @code
		 * template<typename F, typename... Args>
		 * requires Type::InvocableR<bool, F, Args...>
		 * bool test(F&& func, Args&&... args);
		 * @endcode
		 */
		template<typename R, typename F, typename... Args>
		concept InvocableR = std::is_invocable_r_v<R, F, Args...>;

		/**
		 * @brief Predicate over argument types @p Args (`std::predicate`).
		 * @tparam F Callable type.
		 * @tparam Args Argument types passed to `F`.
		 *
		 * @code
		 * template<typename F, typename T>
		 * requires Type::Predicate<F, const T&>
		 * bool keep(F&& func, const T& value);
		 * @endcode
		 */
		template<typename F, typename... Args>
		concept Predicate = std::predicate<F, Args...>;

		/**
		 * @brief Same type after stripping cv and references from both sides.
		 * @tparam T First type.
		 * @tparam U Second type.
		 *
		 * Not `std::same_as`: that does not strip. `int` and `const int&` match here.
		 *
		 * @code
		 * template<typename T, typename U>
		 * requires Type::SameAs<T, U>
		 * void ensure_same_type(T a, U b);
		 * @endcode
		 */
		template<typename T, typename U>
		concept SameAs =
			std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

		/**
		 * @brief @p Derived derives from @p Base (`std::is_base_of`).
		 * @tparam Derived Candidate derived type.
		 * @tparam Base Candidate base type.
		 *
		 * Not `std::derived_from`: that also requires an unambiguous, public base-to-derived conversion. This only checks the inheritance relationship itself.
		 *
		 * @code
		 * struct Base {};
		 * struct Derived : Base {};
		 * static_assert(Type::DerivedFrom<Derived, Base>);
		 * @endcode
		 */
		template<typename Derived, typename Base>
		concept DerivedFrom = std::is_base_of_v<Base, Derived>;

		/** @} */
		/** @} */
	}
}
