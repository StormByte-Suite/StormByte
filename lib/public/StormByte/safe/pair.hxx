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

#include <StormByte/safe/hash.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <functional>
#include <span>
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
	 * @brief Owned values that cross a DLL without the caller's CRT.
	 */
	namespace Safe {
		/**
		 * @class Pair
		 * @brief `std::pair` whose members keep their own Safe storage.
		 * @tparam First Safe value stored in @ref first.
		 * @tparam Second Safe value stored in @ref second.
		 *
		 * The pair allocates nothing. Copy, move and destruction of each member run through that member's Safe operations, so the pair may be created or destroyed in a consumer module.
		 */
		template<Type::SafeValue First, Type::SafeValue Second>
		class STORMBYTE_PUBLIC_TYPE Pair final {
			public:
				using first_type = First; ///< First member type.
				using second_type = Second; ///< Second member type.

				First first; ///< First value.
				Second second; ///< Second value.

				/**
				 * @brief Value-initialize both members.
				 */
				Pair(): first(), second() {}

				/**
				 * @brief Copy two values into the pair.
				 * @param firstValue Initial first value.
				 * @param secondValue Initial second value.
				 */
				Pair(const First& firstValue, const Second& secondValue);

				/**
				 * @brief Move two values into the pair.
				 * @param firstValue Initial first value.
				 * @param secondValue Initial second value.
				 */
				Pair(First&& firstValue, Second&& secondValue) noexcept(
					std::is_nothrow_move_constructible_v<First> &&
					std::is_nothrow_move_constructible_v<Second>);

				/**
				 * @brief Construct both members from convertible values.
				 * @tparam U First source type.
				 * @tparam V Second source type.
				 * @param firstValue Initial first value.
				 * @param secondValue Initial second value.
				 */
				template<class U, class V>
				requires Type::ConstructibleFrom<First, U> && Type::ConstructibleFrom<Second, V>
				explicit((!Type::ConvertibleTo<U, First> || !Type::ConvertibleTo<V, Second>))
				Pair(U&& firstValue, V&& secondValue);

				/**
				 * @brief Construct both members from argument tuples.
				 * @tparam FirstArgs First constructor argument types.
				 * @tparam SecondArgs Second constructor argument types.
				 * @param tag Piecewise construction tag.
				 * @param firstArgs Arguments for the first member.
				 * @param secondArgs Arguments for the second member.
				 */
				template<class... FirstArgs, class... SecondArgs>
				Pair(std::piecewise_construct_t tag, std::tuple<FirstArgs...> firstArgs, std::tuple<SecondArgs...> secondArgs);

				/**
				 * @brief Copy a caller-owned STL pair. Its state is unchanged.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::ConstructibleFrom<First, const OtherFirst&> &&
					Type::ConstructibleFrom<Second, const OtherSecond&>
				explicit((!Type::ConvertibleTo<const OtherFirst&, First> || !Type::ConvertibleTo<const OtherSecond&, Second>))
				Pair(const std::pair<OtherFirst, OtherSecond>& other);

				/**
				 * @brief Move a caller-owned STL pair. The source members are moved-from.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::ConstructibleFrom<First, OtherFirst> &&
					Type::ConstructibleFrom<Second, OtherSecond>
				explicit((!Type::ConvertibleTo<OtherFirst, First> || !Type::ConvertibleTo<OtherSecond, Second>))
				Pair(std::pair<OtherFirst, OtherSecond>&& other);

				/**
				 * @brief Copy a compatible Safe pair.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::ConstructibleFrom<First, const OtherFirst&> &&
					Type::ConstructibleFrom<Second, const OtherSecond&>
				explicit((!Type::ConvertibleTo<const OtherFirst&, First> || !Type::ConvertibleTo<const OtherSecond&, Second>))
				Pair(const Pair<OtherFirst, OtherSecond>& other);

				/**
				 * @brief Move a compatible Safe pair.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::ConstructibleFrom<First, OtherFirst> &&
					Type::ConstructibleFrom<Second, OtherSecond>
				explicit((!Type::ConvertibleTo<OtherFirst, First> || !Type::ConvertibleTo<OtherSecond, Second>))
				Pair(Pair<OtherFirst, OtherSecond>&& other);

				/**
				 * @brief Copy a pair.
				 * @param other Source pair.
				 */
				Pair(const Pair& other) = default;

				/**
				 * @brief Move a pair.
				 * @param other Source pair.
				 */
				Pair(Pair&& other) noexcept(std::is_nothrow_move_constructible_v<First> &&
					std::is_nothrow_move_constructible_v<Second>) = default;

				/**
				 * @brief Destroy both members through their own Safe operations.
				 */
				~Pair() = default;

				/**
				 * @brief Copy-assign a pair.
				 * @param other Source pair.
				 * @return This pair.
				 */
				Pair& operator=(const Pair& other) = default;

				/**
				 * @brief Move-assign a pair.
				 * @param other Source pair.
				 * @return This pair.
				 */
				Pair& operator=(Pair&& other) noexcept(std::is_nothrow_move_assignable_v<First> &&
					std::is_nothrow_move_assignable_v<Second>) = default;

				/**
				 * @brief Copy-assign a compatible Safe pair.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 * @return This pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::AssignableFrom<First&, const OtherFirst&> &&
					Type::AssignableFrom<Second&, const OtherSecond&>
				Pair& operator=(const Pair<OtherFirst, OtherSecond>& other);

				/**
				 * @brief Move-assign a compatible Safe pair.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 * @return This pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::AssignableFrom<First&, OtherFirst> &&
					Type::AssignableFrom<Second&, OtherSecond>
				Pair& operator=(Pair<OtherFirst, OtherSecond>&& other);

				/**
				 * @brief Copy-assign a caller-owned STL pair. Its state is unchanged.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 * @return This pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::AssignableFrom<First&, const OtherFirst&> &&
					Type::AssignableFrom<Second&, const OtherSecond&>
				Pair& operator=(const std::pair<OtherFirst, OtherSecond>& other);

				/**
				 * @brief Move-assign a caller-owned STL pair.
				 * @tparam OtherFirst Source first type.
				 * @tparam OtherSecond Source second type.
				 * @param other Source pair.
				 * @return This pair.
				 */
				template<class OtherFirst, class OtherSecond>
				requires Type::AssignableFrom<First&, OtherFirst> &&
					Type::AssignableFrom<Second&, OtherSecond>
				Pair& operator=(std::pair<OtherFirst, OtherSecond>&& other);

				/**
				 * @brief Exchange both members.
				 * @param other Pair to exchange with.
				 */
				void swap(Pair& other) noexcept(Type::Swappable<First> && Type::Swappable<Second>);

				/**
				 * @brief Exchange two pairs.
				 * @param left First pair.
				 * @param right Second pair.
				 */
				friend void swap(Pair& left, Pair& right) noexcept(noexcept(left.swap(right))) {
					left.swap(right);
				}

				/**
				 * @brief Compare both members.
				 * @param other Pair to compare.
				 * @return Whether both members compare equal.
				 */
				bool operator==(const Pair& other) const requires Type::EqualityComparable<First> && Type::EqualityComparable<Second>;

				/**
				 * @brief Order by the first member, then the second.
				 * @param other Pair to compare.
				 * @return Lexicographical order.
				 */
				auto operator<=>(const Pair& other) const requires Type::ThreeWayComparable<First> && Type::ThreeWayComparable<Second>;

				/**
				 * @brief Copy the members into caller-owned STL storage.
				 * @return A `std::pair` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::pair<First, Second>() const {
					return {first, second};
				}

			private:
				/**
				 * @brief Construct both members from unpacked tuples.
				 * @tparam FirstArgs First constructor argument types.
				 * @tparam SecondArgs Second constructor argument types.
				 * @tparam FirstIndexes Indexes of the first tuple.
				 * @tparam SecondIndexes Indexes of the second tuple.
				 * @param firstArgs Arguments for the first member.
				 * @param secondArgs Arguments for the second member.
				 */
				template<class... FirstArgs, class... SecondArgs, std::size_t... FirstIndexes, std::size_t... SecondIndexes>
				Pair(std::piecewise_construct_t tag, std::tuple<FirstArgs...>& firstArgs, std::tuple<SecondArgs...>& secondArgs,
					std::index_sequence<FirstIndexes...>, std::index_sequence<SecondIndexes...>);
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
				PairReference(const First& firstValue, const Second& secondValue, SecondReference secondReference);

				/**
				 * @brief Convert to a refreshed caller-module Safe pair snapshot.
				 * @return Pair snapshot.
				 */
				operator const value_type&() const;

				/**
				 * @brief Copy a pair-reference proxy.
				 * @param other Source proxy.
				 */
				PairReference(const PairReference& other) = default;

				/**
				 * @brief Move a pair-reference proxy.
				 * @param other Source proxy.
				 */
				PairReference(PairReference&& other) = default;

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
		constexpr First& get(Pair<First, Second>& pair) noexcept;

		/**
		 * @brief Access the second pair member.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Second member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr Second& get(Pair<First, Second>& pair) noexcept;

		/**
		 * @brief Access the first pair member through a const pair.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return Const first member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr const First& get(const Pair<First, Second>& pair) noexcept;

		/**
		 * @brief Access the second pair member through a const pair.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Const second member reference.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr const Second& get(const Pair<First, Second>& pair) noexcept;

		/**
		 * @brief Move-access the first pair member.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return Rvalue reference to the first member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr First&& get(Pair<First, Second>&& pair) noexcept;

		/**
		 * @brief Move-access the second pair member.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Rvalue reference to the second member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr Second&& get(Pair<First, Second>&& pair) noexcept;

		/**
		 * @brief Read the first member from a const rvalue pair.
		 * @tparam Index Member index, which must be zero.
		 * @param pair Pair to access.
		 * @return Const rvalue reference to the first member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr const First&& get(const Pair<First, Second>&& pair) noexcept;

		/**
		 * @brief Read the second member from a const rvalue pair.
		 * @tparam Index Member index, which must be one.
		 * @param pair Pair to access.
		 * @return Const rvalue reference to the second member.
		 */
		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr const Second&& get(const Pair<First, Second>&& pair) noexcept;
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes a pair of already safe values.
		 * @tparam First First Safe value.
		 * @tparam Second Second Safe value.
		 */
		template<SafeValue First, SafeValue Second>
		requires IsSafe<First>::value && IsSafe<Second>::value
		struct IsSafe<Safe::Pair<First, Second>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety when either member is conditionally safe.
		 * @tparam First First Safe value.
		 * @tparam Second Second Safe value.
		 */
		template<SafeValue First, SafeValue Second>
		requires (MaybeSafe<First> || MaybeSafe<Second>)
		struct IsMaybeSafe<Safe::Pair<First, Second>>: std::true_type {};

		/**
		 * @brief Admits a pair of safe values as a collection value.
		 * @tparam First First Safe value.
		 * @tparam Second Second Safe value.
		 */
		template<SafeValue First, SafeValue Second>
		struct IsSafeValue<Safe::Pair<First, Second>>: std::true_type {};
	}
}

/**
 * @brief Tuple arity of a Safe pair.
 * @tparam First First Safe value.
 * @tparam Second Second Safe value.
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
 * @tparam First First Safe value.
 * @tparam Second Second Safe value.
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

/**
 * @brief Cross-module hash of a @ref StormByte::Safe::Pair.
 * @tparam First First Safe value.
 * @tparam Second Second Safe value.
 */
template<StormByte::Type::SafeValue First, StormByte::Type::SafeValue Second>
struct StormByte::Safe::Hash<StormByte::Safe::Pair<First, Second>> {
	/**
	 * @brief Hash @p value. Order matters.
	 * @param value Pair.
	 * @return Hash.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::Safe::Pair<First, Second>& value) const noexcept {
		return HashCombine(Hash<First>{}(value.first), Hash<Second>{}(value.second));
	}
};

/**
 * @brief Hash of a @ref StormByte::Safe::Pair for an STL unordered container in this module.
 * @tparam First First Safe value.
 * @tparam Second Second Safe value.
 */
template<StormByte::Type::SafeValue First, StormByte::Type::SafeValue Second>
struct std::hash<StormByte::Safe::Pair<First, Second>> {
	/**
	 * @brief Hash @p value. Order matters.
	 * @param value Pair.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::Pair<First, Second>& value) const noexcept {
		return StormByte::Safe::Hash<StormByte::Safe::Pair<First, Second>>{}(value);
	}
};

#include <StormByte/safe/pair.txx>
