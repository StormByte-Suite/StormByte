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

#include <StormByte/type_traits/safe.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
	 */
	namespace Safe {
		/**
		 * @class Pair
		 * @brief Two Safe values with an STL-shaped public interface.
		 * @tparam First Safe type stored in @ref first.
		 * @tparam Second Safe type stored in @ref second.
		 *
		 * The pair owns no allocation. Its members retain their own Safe
		 * ownership rules, so the pair object may be copied or destroyed in a
		 * consumer module while creator-owned member storage remains opaque.
		 */
		template<Type::SafeValue First, Type::SafeValue Second>
		class STORMBYTE_PUBLIC_TYPE Pair final {
			public:
				using first_type = First; ///< First member type.
				using second_type = Second; ///< Second member type.

				First first; ///< First value.
				Second second; ///< Second value.

				/**
				 * @brief Construct a pair from default-initialized values.
				 */
				Pair() = default;

				/**
				 * @brief Copy two values into the pair.
				 * @param firstValue Initial first value.
				 * @param secondValue Initial second value.
				 */
				Pair(const First& firstValue, const Second& secondValue):
					first(firstValue), second(secondValue) {}

				/**
				 * @brief Copy values from a caller-owned std::pair.
				 * @tparam OtherFirst Source first component type.
				 * @tparam OtherSecond Source second component type.
				 * @param other Source pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires std::constructible_from<First, const OtherFirst&> &&
					std::constructible_from<Second, const OtherSecond&>
				Pair(const std::pair<OtherFirst, OtherSecond>& other):
					first(other.first), second(other.second) {}

				/**
				 * @brief Move two values into the pair.
				 * @param firstValue Initial first value.
				 * @param secondValue Initial second value.
				 */
				Pair(First&& firstValue, Second&& secondValue) noexcept(
					std::is_nothrow_move_constructible_v<First> &&
					std::is_nothrow_move_constructible_v<Second>):
					first(std::move(firstValue)), second(std::move(secondValue)) {}

				/**
				 * @brief Copy a pair.
				 * @param other Source pair.
				 */
				Pair(const Pair&) = default;

				/**
				 * @brief Move a pair.
				 * @param other Source pair.
				 */
				Pair(Pair&&) noexcept(std::is_nothrow_move_constructible_v<First> &&
					std::is_nothrow_move_constructible_v<Second>) = default;

				/**
				 * @brief Copy-assign a pair.
				 * @param other Source pair.
				 * @return This pair.
				 */
				Pair& operator=(const Pair&) = default;

				/**
				 * @brief Move-assign a pair.
				 * @param other Source pair.
				 * @return This pair.
				 */
				Pair& operator=(Pair&&) noexcept(std::is_nothrow_move_assignable_v<First> &&
					std::is_nothrow_move_assignable_v<Second>) = default;

				/**
				 * @brief Copy the components into caller-owned std::pair storage.
				 * @return std::pair value copy.
				 */
				explicit operator std::pair<First, Second>() const {
					return {first, second};
				}

				/**
				 * @brief Construct from a pair of compatible values.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires std::constructible_from<First, const OtherFirst&> &&
					std::constructible_from<Second, const OtherSecond&>
				explicit(!std::convertible_to<const OtherFirst&, First> ||
					!std::convertible_to<const OtherSecond&, Second>)
				Pair(const Pair<OtherFirst, OtherSecond>& other):
					first(other.first), second(other.second) {}

				/**
				 * @brief Compare two pairs lexicographically.
				 * @param other Pair to compare.
				 * @return Lexicographical ordering.
				 */
				auto operator<=>(const Pair&) const = default;
		};

			/**
			 * @class PairReference
			 * @brief Tuple-like proxy for an immutable first value and callback-backed second value.
			 * @tparam First First value type.
			 * @tparam Second Second value type.
			 * @tparam SecondReference Mutable reference proxy for the second value.
			 */
			template<Type::SafeValue First, Type::SafeValue Second, class SecondReference>
			class PairReference final {
				public:
					using value_type = Pair<First, Second>; ///< Copied entry value type.

					const First first; ///< Immutable first value copy.
					SecondReference second; ///< Callback-backed second value proxy.

					/**
					 * @brief Bind the proxy to copied values and a mutable second-value proxy.
					 * @param firstValue First value copy.
					 * @param secondValue Initial second value copy.
					 * @param secondReference Callback-backed second-value proxy.
					 */
					PairReference(const First& firstValue, const Second& secondValue, SecondReference secondReference):
						first(firstValue), second(std::move(secondReference)), m_snapshot(firstValue, secondValue) {}

					/**
					 * @brief Convert to a refreshed caller-module Safe pair snapshot.
					 * @return Pair snapshot.
					 */
					operator const value_type&() const {
						m_snapshot.second = static_cast<Second>(second);
						return m_snapshot;
					}

					/**
					 * @brief Copy a pair-reference proxy.
					 * @param other Source proxy.
					 */
					PairReference(const PairReference&) = default;

					/**
					 * @brief Move a pair-reference proxy.
					 * @param other Source proxy.
					 */
					PairReference(PairReference&&) = default;

					/**
					 * @brief Access the first component for structured bindings.
					 * @tparam Index Component index, which must be zero.
					 * @param reference Pair-reference proxy.
					 * @return First component.
					 */
					template<std::size_t Index>
					requires (Index == 0)
					friend const First& get(const PairReference& reference) noexcept {
						return reference.first;
					}

					/**
					 * @brief Access the second component for structured bindings.
					 * @tparam Index Component index, which must be one.
					 * @param reference Pair-reference proxy.
					 * @return Second-value proxy.
					 */
					template<std::size_t Index>
					requires (Index == 1)
					friend SecondReference& get(PairReference& reference) noexcept {
						return reference.second;
					}

					/**
					 * @brief Move-access the first component of a temporary proxy.
					 * @tparam Index Component index, which must be zero.
					 * @param reference Pair-reference proxy.
					 * @return Rvalue reference to the immutable first value.
					 */
					template<std::size_t Index>
					requires (Index == 0)
					friend const First&& get(PairReference&& reference) noexcept {
						return std::move(reference.first);
					}

					/**
					 * @brief Move-access the second component of a temporary proxy.
					 * @tparam Index Component index, which must be one.
					 * @param reference Pair-reference proxy.
					 * @return Mapped-value proxy.
					 */
					template<std::size_t Index>
					requires (Index == 1)
					friend SecondReference get(PairReference&& reference) noexcept {
						return std::move(reference.second);
					}

					/**
					 * @brief Read the first component of a const temporary proxy.
					 * @tparam Index Component index, which must be zero.
					 * @param reference Pair-reference proxy.
					 * @return Const rvalue reference to the first value.
					 */
					template<std::size_t Index>
					requires (Index == 0)
					friend const First&& get(const PairReference&& reference) noexcept {
						return std::move(reference.first);
					}

					/**
					 * @brief Read the second component of a const temporary proxy.
					 * @tparam Index Component index, which must be one.
					 * @param reference Pair-reference proxy.
					 * @return Mapped-value proxy copy.
					 */
					template<std::size_t Index>
					requires (Index == 1)
					friend SecondReference get(const PairReference&& reference) {
						return reference.second;
					}

				private:
					mutable value_type m_snapshot; ///< Caller-module pair snapshot.
			};

		/**
		 * @brief Access the first pair member.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return First member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 0)
		constexpr First& get(Pair<First, Second>& pair) noexcept {
			return pair.first;
		}

		/**
		 * @brief Access the second pair member.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Second member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 1)
		constexpr Second& get(Pair<First, Second>& pair) noexcept {
			return pair.second;
		}

		/**
		 * @brief Access the first pair member through a const pair.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return Const first member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 0)
		constexpr const First& get(const Pair<First, Second>& pair) noexcept {
			return pair.first;
		}

		/**
		 * @brief Access the second pair member through a const pair.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Const second member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 1)
		constexpr const Second& get(const Pair<First, Second>& pair) noexcept {
			return pair.second;
		}

		/**
		 * @brief Move-access the first pair member.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return Rvalue reference to the first member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 0)
		constexpr First&& get(Pair<First, Second>&& pair) noexcept {
			return std::move(pair.first);
		}

		/**
		 * @brief Move-access the second pair member.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Rvalue reference to the second member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 1)
		constexpr Second&& get(Pair<First, Second>&& pair) noexcept {
			return std::move(pair.second);
		}

		/**
		 * @brief Read the first member from a const rvalue pair.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return Const rvalue reference to the first member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 0)
		constexpr const First&& get(const Pair<First, Second>&& pair) noexcept {
			return std::move(pair.first);
		}

		/**
		 * @brief Read the second member from a const rvalue pair.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Const rvalue reference to the second member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
			requires (Index == 1)
		constexpr const Second&& get(const Pair<First, Second>&& pair) noexcept {
			return std::move(pair.second);
		}
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes Safe pairs.
		 * @tparam First First Safe type.
		 * @tparam Second Second Safe type.
		 */
		template<SafeValue First, SafeValue Second>
		struct IsSafe<Safe::Pair<First, Second>>: std::true_type {};

		/**
		 * @brief Admits Safe pairs as collection values.
		 * @tparam First First Safe type.
		 * @tparam Second Second Safe type.
		 */
		template<SafeValue First, SafeValue Second>
		struct IsSafeValue<Safe::Pair<First, Second>>: std::true_type {};
	}
}

/**
 * @brief Tuple arity of a Safe pair.
 * @tparam First First Safe type.
 * @tparam Second Second Safe type.
 */
template<StormByte::Type::SafeValue First, StormByte::Type::SafeValue Second>
struct std::tuple_size<StormByte::Safe::Pair<First, Second>>: std::integral_constant<std::size_t, 2> {};

/**
 * @brief Tuple arity of a Safe pair-reference proxy.
 * @tparam First First value type.
 * @tparam Second Second value type.
 * @tparam SecondReference Mapped-value proxy type.
 */
template<StormByte::Type::SafeValue First, StormByte::Type::SafeValue Second, class SecondReference>
struct std::tuple_size<StormByte::Safe::PairReference<First, Second, SecondReference>>:
	std::integral_constant<std::size_t, 2> {};

/**
 * @brief Tuple element type of a Safe pair.
 * @tparam Index Member index.
 * @tparam First First Safe type.
 * @tparam Second Second Safe type.
 */
template<std::size_t Index, StormByte::Type::SafeValue First, StormByte::Type::SafeValue Second>
requires (Index < 2)
struct std::tuple_element<Index, StormByte::Safe::Pair<First, Second>> {
	using type = std::conditional_t<Index == 0, First, Second>;
};

/**
 * @brief Tuple element type of a Safe pair-reference proxy.
 * @tparam Index Member index.
 * @tparam First First value type.
 * @tparam Second Second value type.
 * @tparam SecondReference Mapped-value proxy type.
 */
template<std::size_t Index, StormByte::Type::SafeValue First, StormByte::Type::SafeValue Second, class SecondReference>
requires (Index < 2)
struct std::tuple_element<Index, StormByte::Safe::PairReference<First, Second, SecondReference>> {
	using type = std::conditional_t<Index == 0, const First, SecondReference>;
};
