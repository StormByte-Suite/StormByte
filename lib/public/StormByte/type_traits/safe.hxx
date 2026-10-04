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

#include <StormByte/type_traits/enums.hxx>

#include <type_traits>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @brief Forward declaration of Base's fixed-width count.
	 */
	class Size;
	/**
	 * @brief Forward declaration of Base's fixed-width byte count.
	 */
	class ByteSize;
	/**
	 * @brief Forward declaration of Base-owned binary storage.
	 */
	class BinaryData;

	/**
	 * @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
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
		/**
		 * @brief Forward declaration of Base-owned C text.
		 */
		class CString;
		/**
		 * @brief Forward declaration of Base-owned wide C text.
		 */
		class WCString;
		/**
		 * @brief Forward declaration of shared ownership.
		 * @tparam T Pointee.
		 */
		template<class T> class Shared;
		/**
		 * @brief Forward declaration of unique ownership.
		 * @tparam T Pointee.
		 */
		template<class T> class Unique;
		/**
		 * @brief Forward declaration of weak ownership.
		 * @tparam T Pointee.
		 */
		template<class T> class Weak;
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @struct IsSafe
		 * @brief Recognizes Base Safe components, not universal ABI compatibility.
		 * @tparam T Exact component type. Use SafeComponent to strip cv/references.
		 * @note User specializations are unsupported and cannot certify foreign types.
		 */
		template<typename T>
		struct IsSafe: std::false_type {};

		/**
		 * @brief Recognizes Base-owned UTF-8 text.
		 */
		template<> struct IsSafe<Safe::String>: std::true_type {};
		/**
		 * @brief Recognizes Base-owned wide text.
		 */
		template<> struct IsSafe<Safe::WString>: std::true_type {};
		/**
		 * @brief Recognizes Base-owned C text.
		 */
		template<> struct IsSafe<Safe::CString>: std::true_type {};
		/**
		 * @brief Recognizes Base-owned wide C text.
		 */
		template<> struct IsSafe<Safe::WCString>: std::true_type {};
		/**
		 * @brief Recognizes Base-owned binary storage.
		 */
		template<> struct IsSafe<BinaryData>: std::true_type {};
		/**
		 * @brief Recognizes Base's fixed-width count.
		 */
		template<> struct IsSafe<Size>: std::true_type {};
		/**
		 * @brief Recognizes Base's fixed-width byte count.
		 */
		template<> struct IsSafe<ByteSize>: std::true_type {};
		/**
		 * @brief Recognizes shared ownership, without certifying the pointee or STL ABI.
		 * @tparam T Pointee.
		 */
		template<typename T> struct IsSafe<Safe::Shared<T>>: std::true_type {};
		/**
		 * @brief Recognizes unique ownership, without certifying the pointee.
		 * @tparam T Pointee.
		 */
		template<typename T> struct IsSafe<Safe::Unique<T>>: std::true_type {};
		/**
		 * @brief Recognizes weak ownership, without certifying the pointee or STL ABI.
		 * @tparam T Pointee.
		 */
		template<typename T> struct IsSafe<Safe::Weak<T>>: std::true_type {};

		/**
		 * @brief Recognized Safe component after stripping cv and references.
		 * @tparam T Candidate type.
		 * @note This is classification, not an ABI or pointee-lifetime guarantee.
		 */
		template<typename T>
		concept SafeComponent = IsSafe<std::remove_cvref_t<T>>::value;

		/**
		 * @struct IsSafeOptional
		 * @brief Identifies Safe optional wrappers.
		 * @tparam T Candidate type.
		 */
		template<typename T>
		struct IsSafeOptional: std::false_type {};

		/**
		 * @struct IsSafeQueue
		 * @brief Identifies Safe FIFO wrappers.
		 * @tparam T Candidate type.
		 */
		template<typename T>
		struct IsSafeQueue: std::false_type {};

		/**
		 * @struct IsSafeValue
		 * @brief Closed admission list for collection values with Base-controlled lifetime.
		 * @tparam T Exact, unqualified value type.
		 * @note Shared owners are admitted, but this does not certify their pointee or module lifetime.
		 * @note Weak, Unique and arbitrary Clonable implementations are not admitted.
		 */
		template<typename T>
		struct IsSafeValue: std::false_type {};

		/**
		 * @brief Admits Base-owned UTF-8 text.
		 */
		template<> struct IsSafeValue<Safe::String>: std::true_type {};
		/**
		 * @brief Admits Base-owned wide text.
		 */
		template<> struct IsSafeValue<Safe::WString>: std::true_type {};
		/**
		 * @brief Admits Base-owned C text.
		 */
		template<> struct IsSafeValue<Safe::CString>: std::true_type {};
		/**
		 * @brief Admits Base-owned wide C text.
		 */
		template<> struct IsSafeValue<Safe::WCString>: std::true_type {};
		/**
		 * @brief Admits Base-owned binary storage.
		 */
		template<> struct IsSafeValue<BinaryData>: std::true_type {};
		/**
		 * @brief Admits Base's fixed-width count.
		 */
		template<> struct IsSafeValue<Size>: std::true_type {};
		/**
		 * @brief Admits Base's fixed-width byte count.
		 */
		template<> struct IsSafeValue<ByteSize>: std::true_type {};
		/**
		 * @brief Admits copyable Base-heap shared ownership as a collection value.
		 * @tparam T Pointee type; its ABI and lifetime are not certified.
		 */
		template<typename T> struct IsSafeValue<Safe::Shared<T>>: std::true_type {};
		/**
		 * @brief Admits enumeration values, which cross module boundaries by value.
		 * @tparam T Enumeration type.
		 */
		template<Type::Enum T>
		struct IsSafeValue<T>: std::true_type {};
		/**
		 * @brief Admits arithmetic values, which cross module boundaries by value.
		 * @tparam T Arithmetic type.
		 */
		template<Type::Arithmetic T>
		struct IsSafeValue<T>: std::true_type {};

		/**
		 * @brief Unqualified value permitted in the opaque Safe collections.
		 * @tparam T Candidate value; cv/ref forms and user specializations are unsupported.
		 */
		template<typename T>
		concept SafeValue = IsSafeValue<T>::value;
	}
}