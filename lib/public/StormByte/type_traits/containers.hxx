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

#include <StormByte/type_traits/relations.hxx>

#include <array>
#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	class Size;	///< Unit count. Defined in size.hxx. Forwarded so Type::Sized can name it.

	/**
	 * @namespace StormByte::Safe
	 * @brief Suite text types owned by Base.
	 */
	namespace Safe {
		/**
		 * @brief Forward declaration of Base-owned UTF-8 text.
		 */
		class String;
		/**
		 * @brief Forward declaration of Base-owned wide text.
		 */
		class WString;
	}

	/**
	 * @namespace Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @defgroup TypeContainers Container concepts
		 * @brief Container shape and mutation capability predicates.
		 * @{
		 */

		/**
		 * @name Text
		 * @{
		 */

		/**
		 * @brief Text string types that are not generic containers.
		 * @tparam T Type to test.
		 *
		 * Includes `std::string`, `std::wstring`, `std::u16string`,
		 * `std::u32string`, `StormByte::Safe::String` and `StormByte::Safe::WString`. They stay out of
		 * @ref StormByte::Type::Container so @ref StormByte::Serializable routes them through
		 * @ref StormByte::Detail::Codec.
		 *
		 * The suite types are forward-declared; their definitions live in Base.
		 *
		 * @code
		 * template<Type::String T>
		 * void process(T str);
		 * @endcode
		 */
		template<typename T>
		concept String =
			SameAs<T, std::string> ||
			SameAs<T, std::wstring> ||
			SameAs<T, std::u16string> ||
			SameAs<T, std::u32string> ||
			SameAs<T, StormByte::Safe::String> ||
			SameAs<T, StormByte::Safe::WString>;

		/** @} */

		/**
		 * @name Shape
		 * @{
		 */

		namespace Detail {
			/**
			 * @brief Primary: @p T is not `std::array`.
			 * @tparam T Decayed type.
			 */
			template<typename T>
			inline constexpr bool ArrayV = false;

			/**
			 * @brief Partial: exact `std::array<U, N>`.
			 * @tparam U Element type.
			 * @tparam N Extent.
			 *
			 * Specialisation only. Do not use `tuple_size_v` in the
			 * concept: any type with `value_type` (e.g. @ref StormByte::Safe::Binary)
			 * would instantiate an incomplete `std::tuple_size`.
			 */
			template<typename U, std::size_t N>
			inline constexpr bool ArrayV<std::array<U, N>> = true;
		}

		/**
		 * @brief `std::array<U, N>` after stripping cv/ref.
		 * @tparam T Type to test.
		 *
		 * Same case @ref StormByte::Serializable uses when decoding a
		 * container: fixed extent, assign by index, no `insert(end)`.
		 * C arrays, `std::span` and other fixed buffers do not match.
		 *
		 * @code
		 * static_assert(Type::Array<std::array<int, 4>>);
		 * static_assert(Type::Array<const std::array<int, 4>&>);
		 * static_assert(!Type::Array<int[4]>);
		 * @endcode
		 */
		template<typename T>
		concept Array = Detail::ArrayV<std::remove_cvref_t<T>>;

		/**
		 * @brief Has `begin()`, `end()` and `value_type`, and is not a @ref StormByte::Type::String.
		 * @tparam T Type to test (cv/ref as written; strings use `std::decay_t`).
		 *
		 * Strings are excluded so serialization / pretty-print do not treat
		 * them as generic sequences of code units.
		 *
		 * @code
		 * template<Type::Container T>
		 * void process(T container);
		 * @endcode
		 */
		template<typename T>
		concept Container =
			requires(T t) {
				t.begin();
				t.end();
				typename T::value_type;
			} && !String<std::decay_t<T>>;

		/**
		 * @brief @ref StormByte::Type::Container that publishes a nested `key_type`.
		 * @tparam C Container type (cv/ref ignored for the nested lookup).
		 *
		 * Typical matches: `std::map`, `std::set`, `std::unordered_map`,
		 * `std::unordered_set`.
		 *
		 * @code
		 * template<Type::HasKeyType T>
		 * void process(T container);
		 * @endcode
		 */
		template<typename C>
		concept HasKeyType =
			Container<std::remove_cvref_t<C>> &&
			requires { typename std::remove_cvref_t<C>::key_type; };

		/**
		 * @brief @ref StormByte::Type::Container that publishes a nested `mapped_type`.
		 * @tparam C Container type (cv/ref ignored for the nested lookup).
		 *
		 * Typical matches: `std::map`, `std::unordered_map`.
		 *
		 * @code
		 * template<Type::HasMappedType T>
		 * void process(T container);
		 * @endcode
		 */
		template<typename C>
		concept HasMappedType =
			Container<std::remove_cvref_t<C>> &&
			requires { typename std::remove_cvref_t<C>::mapped_type; };

		/**
		 * @brief @ref StormByte::Type::Container that publishes `size()`.
		 * @tparam C Container type (cv/ref ignored).
		 *
		 * The return type of `size()` is implicitly convertible to
		 * `std::size_t` (STL containers and @ref StormByte::Size) or
		 * is exactly @ref StormByte::Size. `Size` converts to
		 * `std::size_t` implicitly (clamp on a 32-bit host).
		 *
		 * @code
		 * static_assert(Type::Sized<std::vector<int>>);
		 * @endcode
		 */
		template<typename C>
		concept Sized =
			Container<std::remove_cvref_t<C>> &&
			requires(std::remove_cvref_t<C> const& c) {
				c.size();
			} &&
			(
				std::convertible_to<
					decltype(std::declval<std::remove_cvref_t<C> const&>().size()),
					std::size_t
				> ||
				SameAs<
					decltype(std::declval<std::remove_cvref_t<C> const&>().size()),
					Size
				>
			);

		/** @} */

		/**
		 * @name Mutation
		 * @{
		 */

		/**
		 * @brief @ref StormByte::Type::Container that accepts `push_back` of a `value_type`.
		 * @tparam C Container type (cv/ref ignored).
		 *
		 * True if either `push_back(const value_type&)` or
		 * `push_back(value_type&&)` is valid. The const-ref-only check
		 * rejected `std::vector<std::unique_ptr<T>>`.
		 *
		 * Typical matches: `std::vector`, `std::deque`, `std::list`.
		 *
		 * @code
		 * static_assert(Type::HasPushBack<std::vector<int>>);
		 * static_assert(Type::HasPushBack<std::vector<std::unique_ptr<int>>>);
		 * @endcode
		 */
		template<typename C>
		concept HasPushBack =
			Container<std::remove_cvref_t<C>> &&
			(
				requires(std::remove_cvref_t<C>& c,
						typename std::remove_cvref_t<C>::value_type const& v) {
					c.push_back(v);
				} ||
				requires(std::remove_cvref_t<C>& c,
						typename std::remove_cvref_t<C>::value_type&& v) {
					c.push_back(std::move(v));
				}
			);

		/**
		 * @brief @ref StormByte::Type::Container that accepts `push_front` of a `value_type`.
		 * @tparam C Container type (cv/ref ignored).
		 *
		 * Same const-ref / rvalue split as @ref StormByte::Type::HasPushBack.
		 * Typical matches: `std::deque`, `std::list`. `std::vector` does
		 * **not** match.
		 *
		 * @code
		 * static_assert(Type::HasPushFront<std::deque<int>>);
		 * static_assert(!Type::HasPushFront<std::vector<int>>);
		 * @endcode
		 */
		template<typename C>
		concept HasPushFront =
			Container<std::remove_cvref_t<C>> &&
			(
				requires(std::remove_cvref_t<C>& c,
						typename std::remove_cvref_t<C>::value_type const& v) {
					c.push_front(v);
				} ||
				requires(std::remove_cvref_t<C>& c,
						typename std::remove_cvref_t<C>::value_type&& v) {
					c.push_front(std::move(v));
				}
			);

		/**
		 * @brief Associative @ref StormByte::Type::Container that accepts `insert` of a `value_type`.
		 * @tparam C Container type (cv/ref ignored).
		 *
		 * Requires @ref StormByte::Type::HasKeyType or @ref StormByte::Type::HasMappedType so positional
		 * `insert(iterator, value)` on `std::vector` does not satisfy this
		 * on any standard library. Accepts const-ref or rvalue `insert`.
		 *
		 * Typical matches: `std::map`, `std::set`, `std::unordered_map`,
		 * `std::unordered_set`.
		 *
		 * @code
		 * static_assert(Type::HasInsert<std::map<int, int>>);
		 * static_assert(Type::HasInsert<std::set<int>>);
		 * static_assert(!Type::HasInsert<std::vector<int>>);
		 * @endcode
		 */
		template<typename C>
		concept HasInsert =
			Container<std::remove_cvref_t<C>> &&
			(HasKeyType<C> || HasMappedType<C>) &&
			(
				requires(std::remove_cvref_t<C>& c,
						typename std::remove_cvref_t<C>::value_type const& v) {
					c.insert(v);
				} ||
				requires(std::remove_cvref_t<C>& c,
						typename std::remove_cvref_t<C>::value_type&& v) {
					c.insert(std::move(v));
				}
			);

		/**
		 * @brief @ref StormByte::Type::Container that supports `operator[]` with key/index @p U.
		 * @tparam C Container type as written (no cv/ref strip — historical).
		 * @tparam U Key or index type passed to `operator[]`.
		 *
		 * @p U is the caller's index or key (`std::size_t`, `int`,
		 * @ref StormByte::Size, …).
		 *
		 * @code
		 * static_assert(Type::HasSubscript<std::vector<int>, std::size_t>);
		 * static_assert(Type::HasSubscript<std::map<int, int>, int>);
		 * @endcode
		 */
		template<typename C, typename U>
		concept HasSubscript = Container<C> && requires(C& c, U const& u) {
			{ c[u] };
		};

		/** @} */
		/** @} */
	}
}
