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

#include <StormByte/type_traits/conversions.hxx>
#include <StormByte/type_traits/relations.hxx>

#include <type_traits>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	class Size;		///< Unit count. Defined in size.hxx. Forwarded so Type::Numeral can name it.
	class ByteSize;	///< Octet count. Defined in byte_size.hxx. Forwarded so Type::Numeral can name it.

	/**
	 * @namespace Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @defgroup TypeCategories Fundamental type category concepts
		 * @brief Thin wrappers around the basic `std::is_*` type category predicates.
		 * @{
		 */

		/**
		 * @name References
		 * @{
		 */

		/**
		 * @brief Lvalue or rvalue reference type.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Reference T>
		 * void process(T&& ref);
		 * @endcode
		 */
		template<typename T>
		concept Reference = std::is_reference_v<T>;

		/**
		 * @brief Lvalue reference type (`std::is_lvalue_reference`).
		 * @tparam T Type to test as written.
		 *
		 * `int&` and `const int&` match. `int`, `int&&` and
		 * `const int` do not.
		 *
		 * @code
		 * template<typename T>
		 * requires Type::LvalueReference<T>
		 * void bind_lvalue(T&& ref);
		 * @endcode
		 */
		template<typename T>
		concept LvalueReference = std::is_lvalue_reference_v<T>;

		/**
		 * @brief Rvalue reference type (`std::is_rvalue_reference`).
		 * @tparam T Type to test as written.
		 *
		 * `int&&` matches. `int`, `int&` and `const int&` do not.
		 *
		 * @code
		 * template<typename T>
		 * requires Type::RvalueReference<T>
		 * void bind_rvalue(T&& ref);
		 * @endcode
		 */
		template<typename T>
		concept RvalueReference = std::is_rvalue_reference_v<T>;

		/** @} */

		/**
		 * @name Pointers
		 * @{
		 */

		/**
		 * @brief Raw (possibly cv-qualified) pointer type. Not a smart pointer.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Pointer T>
		 * void process(T ptr);
		 * @endcode
		 */
		template<typename T>
		concept Pointer = std::is_pointer_v<T>;

		/**
		 * @brief Smart pointer-like type: dereferenceable and exposes `get()`.
		 * @tparam T Type to test (cv/ref ignored).
		 *
		 * Matches `std::unique_ptr`, `std::shared_ptr` and similar RAII
		 * wrappers without naming them directly. A raw @ref StormByte::Type::Pointer does
		 * not match: it has no `get()`.
		 *
		 * @code
		 * static_assert(Type::SmartPointer<std::shared_ptr<int>>);
		 * static_assert(!Type::SmartPointer<int*>);
		 * @endcode
		 */
		template<typename T>
		concept SmartPointer =
			!Pointer<std::remove_cvref_t<T>> &&
			requires(std::remove_cvref_t<T> const& p) {
				{ *p };
				{ p.operator->() };
				{ p.get() };
			};

		/**
		 * @brief Smart pointer-like type that can test whether it contains an object.
		 * @tparam T Type to test (cv/ref ignored).
		 *
		 * Requires dereference, member access, `get()`, and an implicit or explicit
		 * conversion to `bool`. Unlike @ref StormByte::Type::SmartPointer, this is suitable for APIs
		 * that evaluate `!pointer` before dereferencing it.
		 */
		template<typename T>
		concept NullablePointer =
			SmartPointer<T> &&
			ExplicitlyConvertibleTo<std::remove_cvref_t<T>, bool>;

		/** @} */

		/**
		 * @name Arithmetic
		 * @{
		 */

		/**
		 * @brief Integral type (`bool`, `char`, `int`, `long`, …), including cv.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Integral T>
		 * T add(T a, T b) { return a + b; }
		 * @endcode
		 */
		template<typename T>
		concept Integral = std::is_integral_v<T>;

		/**
		 * @brief Floating-point type (`float`, `double`, `long double`).
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::FloatingPoint T>
		 * T multiply(T a, T b) { return a * b; }
		 * @endcode
		 */
		template<typename T>
		concept FloatingPoint = std::is_floating_point_v<T>;

		/**
		 * @brief Integral or floating-point type.
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Arithmetic T>
		 * T compute(T x) { return x * 2; }
		 * @endcode
		 */
		template<typename T>
		concept Arithmetic = Integral<T> || FloatingPoint<T>;

		/**
		 * @brief Signed arithmetic type (`std::is_signed`).
		 * @tparam T Type to test.
		 *
		 * Floating-point types are signed. `bool` is not.
		 *
		 * @code
		 * template<Type::Signed T>
		 * T negate(T value) { return -value; }
		 * @endcode
		 */
		template<typename T>
		concept Signed = std::is_signed_v<T>;

		/**
		 * @brief Unsigned arithmetic type (`std::is_unsigned`).
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Unsigned T>
		 * bool non_negative(T value) { return value >= T{0}; }
		 * @endcode
		 */
		template<typename T>
		concept Unsigned = std::is_unsigned_v<T>;

		/**
		 * @brief Host integer or suite count type.
		 * @tparam T Type to test (cv/ref stripped).
		 *
		 * Matches every @ref StormByte::Type::Integral,
		 * @ref StormByte::Size (abstract units) and
		 * @ref StormByte::ByteSize (octets). Both suite types store a
		 * `uint64_t` and convert implicitly only to `std::size_t`.
		 *
		 * Does not match floating-point types.
		 *
		 * @code
		 * static_assert(Type::Numeral<int>);
		 * static_assert(Type::Numeral<std::size_t>);
		 * static_assert(Type::Numeral<Size>);
		 * static_assert(Type::Numeral<ByteSize>);
		 * static_assert(!Type::Numeral<double>);
		 * @endcode
		 */
		template<typename T>
		concept Numeral =
			Integral<std::remove_cvref_t<T>> ||
			SameAs<std::remove_cvref_t<T>, Size> ||
			SameAs<std::remove_cvref_t<T>, ByteSize>;

		/** @} */

		/**
		 * @name Qualifiers
		 * @{
		 */

		/**
		 * @brief Top-level `const` qualifier (`std::is_const_v`).
		 * @tparam T Type to test as written.
		 *
		 * `const int` matches. `int&` and `const int&` do not: a reference
		 * type is never `const` itself.
		 *
		 * @code
		 * template<Type::Const T>
		 * void read_only(T value);
		 * @endcode
		 */
		template<typename T>
		concept Const = std::is_const_v<T>;

		/**
		 * @brief Class or struct type (not union, not enum).
		 * @tparam T Type to test.
		 *
		 * @code
		 * template<Type::Class T>
		 * void inspect(T obj);
		 * @endcode
		 */
		template<typename T>
		concept Class = std::is_class_v<T>;

		/** @} */
		/** @} */
	}
}
