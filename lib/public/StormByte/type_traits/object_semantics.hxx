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

#include <StormByte/type_traits/comparison.hxx>
#include <StormByte/type_traits/containers.hxx>
#include <StormByte/type_traits/ranges.hxx>

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
		 * @defgroup TypeObjectSemantics Object semantics concepts
		 * @brief Construction, destruction and copy/move semantics predicates.
		 * @{
		 */

		namespace {
			/**
			 * @brief `T{src}` is well-formed for a const lvalue @p src.
			 * @tparam T Type to test.
			 */
			template<typename T, typename = void>
			inline constexpr bool ReallyCopyConstructible =
				requires(const T& src) { T{src}; };

			/**
			 * @brief Owning containers also require a copyable `value_type`.
			 * @tparam T Container that is not a @ref StormByte::Type::View.
			 */
			template<typename T>
			inline constexpr bool ReallyCopyConstructible<
				T, std::enable_if_t<
					Container<std::remove_cvref_t<T>> &&
					!View<std::remove_cvref_t<T>>
				>
			> =
				requires(const T& src) { T{src}; } &&
				ReallyCopyConstructible<typename std::remove_cvref_t<T>::value_type>;

			/**
			 * @brief `dest = src` is well-formed for const lvalue @p src.
			 * @tparam T Type to test.
			 */
			template<typename T, typename = void>
			inline constexpr bool ReallyCopyAssignable =
				requires(T& dest, const T& src) { dest = src; };

			/**
			 * @brief Owning containers also require an assignable `value_type`.
			 * @tparam T Container that is not a @ref StormByte::Type::View.
			 */
			template<typename T>
			inline constexpr bool ReallyCopyAssignable<
				T, std::enable_if_t<
					Container<std::remove_cvref_t<T>> &&
					!View<std::remove_cvref_t<T>>
				>
			> =
				requires(T& dest, const T& src) { dest = src; } &&
				ReallyCopyAssignable<typename std::remove_cvref_t<T>::value_type>;
		}

		/**
		 * @name Trivial
		 * @{
		 */

		/**
		 * @brief Type that may be copied with `memcpy` / as-if `memcpy`.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyCopyable T>
		 * void fast_copy(T* dest, const T* src, size_t n);
		 * @endcode
		 */
		template<typename T>
		concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

		/**
		 * @brief Type with a trivial destructor.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyDestructible T>
		 * void pool_allocate(T* ptr);
		 * @endcode
		 */
		template<typename T>
		concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

		/** @} */

		/**
		 * @name Construction
		 * @{
		 */

		/**
		 * @brief Type constructible from an empty initializer (`T{}` / `T()`).
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::DefaultConstructible T>
		 * T create() { return T{}; }
		 * @endcode
		 */
		template<typename T>
		concept DefaultConstructible = std::is_default_constructible_v<T>;

		/**
		 * @brief @ref StormByte::Type::DefaultConstructible with a trivial default constructor.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyDefaultConstructible T>
		 * T* pool_new();
		 * @endcode
		 */
		template<typename T>
		concept TriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;

		/**
		 * @brief Type that can actually be copy-constructed.
		 * @tparam T Type to test.
		 *
		 * Not `std::is_copy_constructible_v`: that trait is true for
		 * `std::vector<std::unique_ptr<U>>` because `vector` declares a
		 * copy constructor even when instantiating it is ill-formed.
		 * Recurses into `value_type` only when @p T is a @ref StormByte::Type::Container
		 * and not a @ref StormByte::Type::View. `std::span<std::unique_ptr<U>>` is a
		 * container *and* a view, so it stays copyable; an owning
		 * `std::vector<std::unique_ptr<U>>` does not.
		 *
		 * @code
		 * static_assert(Type::CopyConstructible<std::vector<int>>);
		 * static_assert(!Type::CopyConstructible<std::vector<std::unique_ptr<int>>>);
		 * static_assert(Type::CopyConstructible<std::span<std::unique_ptr<int>>>);
		 * @endcode
		 */
		template<typename T>
		concept CopyConstructible = ReallyCopyConstructible<T>;

		/**
		 * @brief @ref StormByte::Type::CopyConstructible with a trivial copy constructor.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyCopyConstructible T>
		 * void fast_construct(T* dest, const T& src);
		 * @endcode
		 */
		template<typename T>
		concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

		/**
		 * @brief Type that can be move-constructed.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::MoveConstructible T>
		 * T transfer(T&& source) { return T{std::move(source)}; }
		 * @endcode
		 */
		template<typename T>
		concept MoveConstructible = std::is_move_constructible_v<T>;

		/**
		 * @brief @ref StormByte::Type::MoveConstructible with a trivial move constructor.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyMoveConstructible T>
		 * void fast_construct(T* dest, T&& src);
		 * @endcode
		 */
		template<typename T>
		concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

		/** @} */

		/**
		 * @name Assignment
		 * @{
		 */

		/**
		 * @brief Type that can actually be copy-assigned.
		 * @tparam T Type to test.
		 *
		 * Not `std::is_copy_assignable_v`: same caveat as
		 * @ref StormByte::Type::CopyConstructible for containers of move-only values.
		 * Recurses into `value_type` only when @p T is a @ref StormByte::Type::Container
		 * and not a @ref StormByte::Type::View.
		 *
		 * @code
		 * template<Type::CopyAssignable T>
		 * void overwrite(T& dest, const T& src) { dest = src; }
		 * @endcode
		 */
		template<typename T>
		concept CopyAssignable = ReallyCopyAssignable<T>;

		/**
		 * @brief @ref StormByte::Type::CopyAssignable with a trivial copy-assignment operator.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyCopyAssignable T>
		 * void fast_assign(T& dest, const T& src) { dest = src; }
		 * @endcode
		 */
		template<typename T>
		concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;

		/**
		 * @brief Type that can be move-assigned (`operator=(T&&)`).
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::MoveAssignable T>
		 * void take_over(T& dest, T&& src) { dest = std::move(src); }
		 * @endcode
		 */
		template<typename T>
		concept MoveAssignable = std::is_move_assignable_v<T>;

		/**
		 * @brief @ref StormByte::Type::MoveAssignable with a trivial move-assignment operator.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TriviallyMoveAssignable T>
		 * void fast_take_over(T& dest, T&& src) { dest = std::move(src); }
		 * @endcode
		 */
		template<typename T>
		concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;

		/** @} */

		/**
		 * @name Combined
		 * @{
		 */

		/**
		 * @brief Both @ref StormByte::Type::CopyConstructible and @ref StormByte::Type::CopyAssignable.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Copyable T>
		 * T duplicate(const T& original);
		 * @endcode
		 */
		template<typename T>
		concept Copyable = CopyConstructible<T> && CopyAssignable<T>;

		/**
		 * @brief Both @ref StormByte::Type::MoveConstructible and @ref StormByte::Type::MoveAssignable.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Movable T>
		 * T transfer(T&& source);
		 * @endcode
		 */
		template<typename T>
		concept Movable = MoveConstructible<T> && MoveAssignable<T>;

		/**
		 * @brief Type that can be swapped with `std::swap` (`std::is_swappable`).
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Swappable T>
		 * void exchange(T& a, T& b) { std::swap(a, b); }
		 * @endcode
		 */
		template<typename T>
		concept Swappable = std::is_swappable_v<T>;

		/**
		 * @brief Type that can be destroyed.
		 * @tparam T Type to test.
		 *
		 * Not @ref StormByte::Type::TriviallyDestructible. A user-provided destructor matches here.
		 *
		 * @code
		 * template<Type::Destructible T>
		 * void release(T* object) { object->~T(); }
		 * @endcode
		 */
		template<typename T>
		concept Destructible = std::is_destructible_v<T>;

		/**
		 * @brief Type constructible from @p Args (`std::constructible_from`).
		 * @tparam T Type to construct.
		 * @tparam Args Constructor argument types.
		 *
		 * @code
		 * template<typename T, typename... Args>
		 * requires Type::ConstructibleFrom<T, Args...>
		 * T make(Args&&... args);
		 * @endcode
		 */
		template<typename T, typename... Args>
		concept ConstructibleFrom = std::constructible_from<T, Args...>;

		/**
		 * @brief @p T can be assigned from @p U (`std::assignable_from`).
		 * @tparam T Destination type.
		 * @tparam U Source type.
		 *
		 * Not @ref StormByte::Type::CopyAssignable. That only covers assignment from the same type.
		 *
		 * @code
		 * template<typename T, typename U>
		 * requires Type::AssignableFrom<T&, U>
		 * T& store(T& dest, U&& value);
		 * @endcode
		 */
		template<typename T, typename U>
		concept AssignableFrom = std::assignable_from<T, U>;

		/**
		 * @brief Type with equality and a total order.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::TotallyOrdered T>
		 * bool before(const T& left, const T& right) { return left < right; }
		 * @endcode
		 */
		template<typename T>
		concept TotallyOrdered = EqualityComparable<T> &&
			requires(std::remove_cvref_t<T> const& a, std::remove_cvref_t<T> const& b) {
				{ a < b } -> std::convertible_to<bool>;
				{ a > b } -> std::convertible_to<bool>;
				{ a <= b } -> std::convertible_to<bool>;
				{ a >= b } -> std::convertible_to<bool>;
			};

		/**
		 * @brief Default-constructible, copyable and movable.
		 * @tparam T Type to test.
		 *
		 * Not `std::semiregular`: that also requires `std::swappable`. Swap stays a separate contract.
		 *
		 * @code
		 * template<Type::Semiregular T>
		 * T make();
		 * @endcode
		 */
		template<typename T>
		concept Semiregular = DefaultConstructible<T> && Copyable<T> && Movable<T>;

		/**
		 * @brief @ref StormByte::Type::Semiregular and equality-comparable.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Regular T>
		 * bool same(const T& left, const T& right) { return left == right; }
		 * @endcode
		 */
		template<typename T>
		concept Regular = Semiregular<T> && EqualityComparable<T>;

		/** @} */
		/** @} */
	}
}
