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
#include <StormByte/type_traits/object_semantics.hxx>
#include <StormByte/type_traits/relations.hxx>

#include <array>
#include <deque>
#include <expected>
#include <forward_list>
#include <functional>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <stack>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

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
	 * @brief Forward declaration of Base's DLL-safe exception root.
	 */
	class Exception;

	/**
	 * @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
	 */
	namespace Safe {
		/**
		 * @brief Forward declaration of Base-owned binary storage.
		 */
		class Binary;
		/**
		 * @brief Forward declaration of Base-owned UTF-8 text.
		 */
		class String;
		/**
		 * @brief Forward declaration of Base-owned wide text.
		 */
		class WString;
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
		 * @brief Arithmetic values cross a DLL boundary by value.
		 * @tparam T Arithmetic type.
		 */
		template<Type::Arithmetic T> struct IsSafe<T>: std::true_type {};
		/**
		 * @brief Enumeration values cross a DLL boundary by value.
		 * @tparam T Enumeration type.
		 */
		template<Type::Enum T> struct IsSafe<T>: std::true_type {};
		/**
		 * @brief Recognizes Base's exception root, whose destructor and message storage are DLL-safe.
		 */
		template<> struct IsSafe<StormByte::Exception>: std::true_type {};

		/**
		 * @brief Recognizes Base-owned binary storage.
		 */
		template<> struct IsSafe<Safe::Binary>: std::true_type {};
		/**
		 * @brief Recognizes Base-owned UTF-8 text.
		 */
		template<> struct IsSafe<Safe::String>: std::true_type {};
		/**
		 * @brief Recognizes Base-owned wide text.
		 */
		template<> struct IsSafe<Safe::WString>: std::true_type {};
		/**
		 * @brief Recognizes Base's fixed-width count.
		 */
		template<> struct IsSafe<Size>: std::true_type {};
		/**
		 * @brief Recognizes Base's fixed-width byte count.
		 */
		template<> struct IsSafe<ByteSize>: std::true_type {};
		/**
		 * @brief Recognizes shared ownership only when its pointee is IsSafe.
		 * @tparam T Pointee.
		 */
		template<typename T> struct IsSafe<Safe::Shared<T>>: std::bool_constant<IsSafe<std::remove_cvref_t<T>>::value> {};
		/**
		 * @brief Recognizes unique ownership only when its pointee is IsSafe.
		 * @tparam T Pointee.
		 */
		template<typename T> struct IsSafe<Safe::Unique<T>>: std::bool_constant<IsSafe<std::remove_cvref_t<T>>::value> {};
		/**
		 * @brief Recognizes weak ownership only when its pointee is IsSafe.
		 * @tparam T Pointee.
		 */
		template<typename T> struct IsSafe<Safe::Weak<T>>: std::bool_constant<IsSafe<std::remove_cvref_t<T>>::value> {};

		/**
		 * @namespace StormByte::Type::Detail
		 * @brief Private helpers for Safe classification.
		 */
		namespace Detail {
			/**
			 * @brief Identifies standard-library ownership and view wrappers that Base will not admit.
			 * @tparam T Candidate type.
			 */
			template<typename T>
			struct IsKnownNeverSafe: std::false_type {};

			template<typename T> struct IsKnownNeverSafe<T*>: std::true_type {};
			template<typename T> struct IsKnownNeverSafe<T[]>: std::true_type {};
			template<typename T, std::size_t N> struct IsKnownNeverSafe<T[N]>: std::true_type {};
			template<typename T, typename Traits, typename Allocator>
			struct IsKnownNeverSafe<std::basic_string<T, Traits, Allocator>>: std::true_type {};
			template<typename T, typename Traits>
			struct IsKnownNeverSafe<std::basic_string_view<T, Traits>>: std::true_type {};
			template<typename T, std::size_t N> struct IsKnownNeverSafe<std::array<T, N>>: std::true_type {};
			template<typename T, typename Allocator> struct IsKnownNeverSafe<std::vector<T, Allocator>>: std::true_type {};
			template<typename T, typename Allocator> struct IsKnownNeverSafe<std::deque<T, Allocator>>: std::true_type {};
			template<typename T, typename Allocator> struct IsKnownNeverSafe<std::list<T, Allocator>>: std::true_type {};
			template<typename T, typename Allocator> struct IsKnownNeverSafe<std::forward_list<T, Allocator>>: std::true_type {};
			template<typename K, typename V, typename Compare, typename Allocator>
			struct IsKnownNeverSafe<std::map<K, V, Compare, Allocator>>: std::true_type {};
			template<typename K, typename V, typename Compare, typename Allocator>
			struct IsKnownNeverSafe<std::multimap<K, V, Compare, Allocator>>: std::true_type {};
			template<typename K, typename Compare, typename Allocator>
			struct IsKnownNeverSafe<std::set<K, Compare, Allocator>>: std::true_type {};
			template<typename K, typename Compare, typename Allocator>
			struct IsKnownNeverSafe<std::multiset<K, Compare, Allocator>>: std::true_type {};
			template<typename K, typename V, typename Hash, typename Equal, typename Allocator>
			struct IsKnownNeverSafe<std::unordered_map<K, V, Hash, Equal, Allocator>>: std::true_type {};
			template<typename K, typename V, typename Hash, typename Equal, typename Allocator>
			struct IsKnownNeverSafe<std::unordered_multimap<K, V, Hash, Equal, Allocator>>: std::true_type {};
			template<typename K, typename Hash, typename Equal, typename Allocator>
			struct IsKnownNeverSafe<std::unordered_set<K, Hash, Equal, Allocator>>: std::true_type {};
			template<typename K, typename Hash, typename Equal, typename Allocator>
			struct IsKnownNeverSafe<std::unordered_multiset<K, Hash, Equal, Allocator>>: std::true_type {};
			template<typename T, typename Container>
			struct IsKnownNeverSafe<std::queue<T, Container>>: std::true_type {};
			template<typename T, typename Container, typename Compare>
			struct IsKnownNeverSafe<std::priority_queue<T, Container, Compare>>: std::true_type {};
			template<typename T, typename Container>
			struct IsKnownNeverSafe<std::stack<T, Container>>: std::true_type {};
			template<typename T>
			struct IsKnownNeverSafe<std::initializer_list<T>>: std::true_type {};
			template<typename T> struct IsKnownNeverSafe<std::optional<T>>: std::true_type {};
			template<typename... T> struct IsKnownNeverSafe<std::variant<T...>>: std::true_type {};
			template<typename... T> struct IsKnownNeverSafe<std::tuple<T...>>: std::true_type {};
			template<typename A, typename B> struct IsKnownNeverSafe<std::pair<A, B>>: std::true_type {};
			template<typename T, std::size_t Extent> struct IsKnownNeverSafe<std::span<T, Extent>>: std::true_type {};
			template<typename T> struct IsKnownNeverSafe<std::reference_wrapper<T>>: std::true_type {};
			template<typename Signature> struct IsKnownNeverSafe<std::function<Signature>>: std::true_type {};
			template<typename T> struct IsKnownNeverSafe<std::shared_ptr<T>>: std::true_type {};
			template<typename T, typename Deleter> struct IsKnownNeverSafe<std::unique_ptr<T, Deleter>>: std::true_type {};
			template<typename T> struct IsKnownNeverSafe<std::weak_ptr<T>>: std::true_type {};
		}

		/**
		 * @struct IsMaybeSafe
		 * @brief Recognizes types whose DLL-boundary safety depends on documented provider requirements.
		 * @tparam T Exact type.
		 * @note Consumers use STORMBYTE_DECLARE_MAYBE_SAFE rather than specializing this trait directly.
		 * @note Exception derivatives qualify when complete; their destructors must be defined out-of-line.
		 */
		template<typename T>
		struct IsMaybeSafe: std::false_type {};

		/**
		 * @brief Recognizes complete derivatives of StormByte::Exception as conditionally DLL-safe.
		 * @tparam T Derived exception type.
		 */
		template<typename T>
		requires requires { sizeof(T); } && DerivedFrom<T, StormByte::Exception> && (!SameAs<T, StormByte::Exception>)
		struct IsMaybeSafe<T>: std::true_type {};
		/**
		 * @brief Shared ownership inherits the pointee's conditional classification.
		 * @tparam T Pointee.
		 */
		template<typename T>
		struct IsMaybeSafe<Safe::Shared<T>>: std::bool_constant<
			IsMaybeSafe<std::remove_cvref_t<T>>::value && !Detail::IsKnownNeverSafe<std::remove_cvref_t<T>>::value
		> {};
		/**
		 * @brief Unique ownership inherits the pointee's conditional classification.
		 * @tparam T Pointee.
		 */
		template<typename T>
		struct IsMaybeSafe<Safe::Unique<T>>: std::bool_constant<
			IsMaybeSafe<std::remove_cvref_t<T>>::value && !Detail::IsKnownNeverSafe<std::remove_cvref_t<T>>::value
		> {};
		/**
		 * @brief Weak ownership inherits the pointee's conditional classification.
		 * @tparam T Pointee.
		 */
		template<typename T>
		struct IsMaybeSafe<Safe::Weak<T>>: std::bool_constant<
			IsMaybeSafe<std::remove_cvref_t<T>>::value && !Detail::IsKnownNeverSafe<std::remove_cvref_t<T>>::value
		> {};

		/**
		 * @brief Recognizes a conditional Safe component after stripping cv and references.
		 * @tparam T Candidate type.
		 */
		template<typename T>
		concept MaybeSafe = !IsSafe<std::remove_cvref_t<T>>::value &&
			IsMaybeSafe<std::remove_cvref_t<T>>::value &&
			!Detail::IsKnownNeverSafe<std::remove_cvref_t<T>>::value;

		/**
		 * @brief Recognized Safe component after stripping cv and references.
		 * @tparam T Candidate type.
		 * @note MaybeSafe components require their provider's documented guarantees.
		 */
		template<typename T>
		concept SafeComponent = IsSafe<std::remove_cvref_t<T>>::value || MaybeSafe<T>;

		/**
		 * @brief Standard expected is conditionally Safe when its alternatives are Safe components.
		 * @tparam T Value type, or void.
		 * @tparam E Error type.
		 * @note Its STL ABI remains subject to the compatible-toolchain requirement.
		 */
		template<typename T, typename E>
		requires (std::is_void_v<T> || SafeComponent<T>) && SafeComponent<E>
		struct IsMaybeSafe<std::expected<T, E>>: std::true_type {};

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
		 * @brief Recognizes collection values with Base-controlled lifetime.
		 * @tparam T Exact, unqualified value type.
		 * @note MaybeSafe values are additionally admitted by SafeValue when their value operations are available.
		 * @note Weak, Unique and arbitrary Clonable implementations are not collection values.
		 */
		template<typename T>
		struct IsSafeValue: std::false_type {};

		/**
		 * @brief Admits Base-owned binary storage.
		 */
		template<> struct IsSafeValue<Safe::Binary>: std::true_type {};
		/**
		 * @brief Admits Base-owned UTF-8 text.
		 */
		template<> struct IsSafeValue<Safe::String>: std::true_type {};
		/**
		 * @brief Admits Base-owned wide text.
		 */
		template<> struct IsSafeValue<Safe::WString>: std::true_type {};
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
		template<typename T>
		requires SafeComponent<T>
		struct IsSafeValue<Safe::Shared<T>>: std::true_type {};
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
		 * @brief Recognizes a cursor that stores only a Base node pointer.
		 * @tparam T Candidate cursor.
		 *
		 * The mark is an alias inside the cursor. Copying the cursor copies that pointer. It is not a Safe component and it owns no element.
		 */
		template<typename T>
		concept SafeCursor = requires { typename std::remove_cvref_t<T>::StormByteSafeCursor; };

		/**
		 * @brief Value permitted in opaque Safe collections.
		 * @tparam T Candidate value. A const value is the same value. References stay rejected.
		 * @note MaybeSafe values must support default construction, copying, assignment and movement.
		 * @note A cursor is admitted so a result pair can hold it. It is not a collection element.
		 */
		template<typename T>
		concept SafeValue = IsSafeValue<std::remove_const_t<T>>::value ||
			SafeCursor<T> ||
			(MaybeSafe<T> && DefaultConstructible<T> && Copyable<T> && MoveConstructible<T> && MoveAssignable<T>);
	}
}

/**
 * @brief Register a complete consumer type as conditionally DLL-safe.
 * @param SafeType Fully qualified consumer-defined type.
 *
 * This is an explicit provider responsibility declaration, not a certification
 * by Base. The type's fields, copy/move operations, assignment, destructor,
 * allocator ownership and ABI must satisfy the MaybeSafe contract. Invoke at
 * global namespace scope after the type is complete. Known incompatible STL
 * types remain rejected even if this macro is applied to them.
 */
#define STORMBYTE_DECLARE_MAYBE_SAFE(SafeType) \
	template<> struct StormByte::Type::IsMaybeSafe<SafeType>: std::true_type {}
