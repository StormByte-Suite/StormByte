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

#include <StormByte/type_traits/safe.hxx>

#include <concepts>
#include <cstddef>
#include <optional>
#include <queue>
#include <type_traits>
#include <utility>
#include <variant>

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
		 */
		namespace Detail {
			/**
			 * @brief `true` when @p T is exactly `std::variant` after stripping cv and references.
			 * @tparam T Type to test.
			 *
			 * Partial specialization of a variable template — not SFINAE.
			 * Needed because there is no standard `std::is_variant`.
			 */
			template<typename T>
			constexpr bool is_variant_v = false;

			/**
			 * @brief Specialization for every `std::variant` alternative list.
			 * @tparam Ts Variant alternatives.
			 */
			template<typename... Ts>
			constexpr bool is_variant_v<std::variant<Ts...>> = true;

			/**
			 * @brief Primary classification for standard FIFO wrappers.
			 * @tparam T Candidate type.
			 */
			template<typename T>
			struct is_std_queue: std::false_type {};

			/**
			 * @brief Specialization for std::queue with any underlying container.
			 * @tparam Value Queued value type.
			 * @tparam Container Underlying container type.
			 */
			template<typename Value, typename Container>
			struct is_std_queue<std::queue<Value, Container>>: std::true_type {};

			/**
			 * @brief Fold: is stripped @p U one of @p VariantT's alternatives?
			 * @tparam VariantT A `std::variant` (already stripped by the caller).
			 * @tparam U Candidate alternative.
			 * @tparam I Index pack over `std::variant_size_v<VariantT>`.
			 * @param[in] seq Index sequence; unused except to expand @p I.
			 * @return `true` if @p U matches any alternative.
			 */
			template<typename VariantT, typename U, std::size_t... I>
			constexpr bool variant_has_type_impl(std::index_sequence<I...> seq) noexcept {
				(void)seq;
				return ((std::is_same_v<
					std::remove_cvref_t<U>,
					std::remove_cvref_t<std::variant_alternative_t<I, VariantT>>
				>) || ...);
			}

			/**
			 * @brief Convenience wrapper around @ref StormByte::Type::Detail::variant_has_type_impl.
			 * @tparam VariantT A `std::variant`.
			 * @tparam U Candidate alternative.
			 */
			template<typename VariantT, typename U>
			constexpr bool variant_has_type_v =
				variant_has_type_impl<VariantT, U>(
					std::make_index_sequence<std::variant_size_v<VariantT>>()
				);
		}

		/**
		 * @defgroup TypeWrappers Standard wrapper and aggregate concepts
		 * @brief Predicates for standard wrappers and pair-like aggregate values.
		 * @{
		 */

		/**
		 * @name Wrappers
		 * @{
		 */

		/**
		 * @brief `std::optional<U>` or a Safe optional wrapper.
		 * @tparam T Type to test (no decay).
		 *
		 * A type that merely has `value_type` does not match. Safe optional types
		 * are registered explicitly and use the same optional wire framing.
		 *
		 * @code
		 * template<Type::Optional T>
		 * void process(T opt);
		 * @endcode
		 */
		template<typename T>
		concept Optional =
			(requires { typename T::value_type; } &&
				std::same_as<T, std::optional<typename T::value_type>>) || IsSafeOptional<T>::value;

		/**
		 * @brief FIFO wrappers supported by generic queue operations.
		 * @tparam T Type to test.
		 *
		 * Includes `std::queue` and registered Safe queue wrappers. This concept
		 * does not require iteration; consumers use `front` / `pop` / `push`.
		 */
		template<typename T>
		concept Queue =
			Detail::is_std_queue<T>::value || IsSafeQueue<T>::value;

		/**
		 * @brief Instantiation of `std::variant`, after stripping cv and references.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Variant T>
		 * void process(T var);
		 * @endcode
		 */
		template<typename T>
		concept Variant = Detail::is_variant_v<std::remove_cvref_t<T>>;

		/**
		 * @brief @ref StormByte::Type::Variant @p T that lists @p U among its alternatives.
		 * @tparam T Variant type (cv and references stripped before the lookup).
		 * @tparam U Alternative to look for (cv and references stripped).
		 *
		 * @code
		 * template<typename T, typename U>
		 * requires Type::VariantHasType<T, U>
		 * void handle_variant(T var);
		 * @endcode
		 */
		template<typename T, typename U>
		concept VariantHasType =
			Variant<T> &&
			Detail::variant_has_type_v<std::remove_cvref_t<T>, U>;

		/** @} */

		/**
		 * @name Aggregates
		 * @{
		 */

		/**
		 * @brief Type with accessible `first` and `second` members.
		 * @tparam T Type to test.
		 *
		 * Broader than `std::pair`: any struct with those members matches.
		 * That is intentional and must stay that way.
		 *
		 * @code
		 * template<Type::Pair T>
		 * void process(T pair);
		 * @endcode
		 */
		template<typename T>
		concept Pair = requires(T t) {
			t.first;
			t.second;
		};

		/** @} */
		/** @} */
	}
}
