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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <optional>
#include <span>
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
		 * @namespace StormByte::Safe::Detail
		 * @brief Private implementation helpers for Safe containers.
		 */
		namespace Detail {
			/**
			 * @brief Recognizes a monadic result that can report an empty state.
			 * @tparam R Candidate result.
			 */
			template<class R>
			concept SafeOptionalResult = Type::SafeComponent<R> && requires(const R& result) {
				{ result.has_value() } -> Type::ConvertibleTo<bool>;
			};
		}

		/**
		 * @class Optional
		 * @brief `std::optional` stored on Base's heap.
		 * @tparam T Safe component.
		 *
		 * The observable contract matches `std::optional`. The contained value is constructed in a block from @ref Heap::Allocate and released with @ref Heap::Free. A move of this optional leaves the source empty. A move from `std::optional` does not change its `has_value()`. `value()` throws @ref BadOptionalAccess when empty. `operator*` and `operator->` require a contained value and do not check.
		 */
		template<Type::SafeComponent T>
		class STORMBYTE_PUBLIC_TYPE Optional final {
			public:
				using value_type = T; ///< Contained type.
				using iterator = T*; ///< Mutable iterator over the single value, or end when empty.
				using const_iterator = const T*; ///< Read-only iterator over the single value, or end when empty.
				using reference = T&; ///< Mutable contained value.
				using const_reference = const T&; ///< Read-only contained value.

				/**
				 * @brief Construct an empty optional.
				 */
				Optional() noexcept;

				/**
				 * @brief Construct an empty optional.
				 * @param value Empty-state tag.
				 */
				Optional(std::nullopt_t value) noexcept;

				/**
				 * @brief Construct the contained value in Base storage.
				 * @tparam Args Constructor argument types.
				 * @param tag In-place construction tag.
				 * @param args Arguments forwarded to T.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class... Args>
				requires Type::ConstructibleFrom<T, Args...>
				explicit Optional(std::in_place_t tag, Args&&... args): Optional() {
					(void)tag;
					emplace(std::forward<Args>(args)...);
				}

				/**
				 * @brief Construct T from an initializer list and trailing arguments.
				 * @tparam U Initializer-list element type.
				 * @tparam Args Trailing constructor argument types.
				 * @param tag In-place construction tag.
				 * @param values Initializer-list elements.
				 * @param args Trailing arguments forwarded to T.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class U, class... Args>
				requires Type::ConstructibleFrom<T, std::initializer_list<U>&, Args...>
				explicit Optional(std::in_place_t tag, std::initializer_list<U> values, Args&&... args): Optional() {
					(void)tag;
					emplace(values, std::forward<Args>(args)...);
				}

				/**
				 * @brief Copy a value into Base storage.
				 * @param value Value to store.
				 * @throws AllocationError The block could not be allocated.
				 */
				Optional(const T& value);

				/**
				 * @brief Move a value into Base storage.
				 * @param value Value to store.
				 * @throws AllocationError The block could not be allocated.
				 */
				Optional(T&& value);

				/**
				 * @brief Construct T from a value that can construct it.
				 * @tparam U Source type.
				 * @param value Source value.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class U>
				requires (!Type::SameAs<U, Optional>) &&
					(!Type::SameAs<U, std::nullopt_t>) &&
					(!Type::SameAs<U, std::in_place_t>) &&
					Type::ConstructibleFrom<T, U>
				explicit((!Type::ConvertibleTo<U, T>)) Optional(U&& value): Optional() {
					emplace(std::forward<U>(value));
				}

				/**
				 * @brief Copy a caller-owned STL optional into Base storage.
				 * @param value Source. Its state is unchanged.
				 * @throws AllocationError The block could not be allocated.
				 */
				Optional(const std::optional<T>& value);

				/**
				 * @brief Move a caller-owned STL optional into Base storage.
				 * @param value Source. `has_value()` is unchanged; a held value is moved-from.
				 * @throws AllocationError The block could not be allocated.
				 */
				Optional(std::optional<T>&& value);

				/**
				 * @brief Copy a caller-owned STL optional of a convertible value.
				 * @tparam U Source value type.
				 * @param value Source. Its state is unchanged.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class U>
				requires (!Type::SameAs<U, T>) && Type::ConstructibleFrom<T, const U&>
				explicit((!Type::ConvertibleTo<const U&, T>)) Optional(const std::optional<U>& value): Optional() {
					if (value)
						emplace(T(*value));
				}

				/**
				 * @brief Move a caller-owned STL optional of a convertible value.
				 * @tparam U Source value type.
				 * @param value Source. `has_value()` is unchanged; a held value is moved-from.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class U>
				requires (!Type::SameAs<U, T>) && Type::ConstructibleFrom<T, U>
				explicit((!Type::ConvertibleTo<U, T>)) Optional(std::optional<U>&& value): Optional() {
					if (value)
						emplace(T(std::move(*value)));
				}

				/**
				 * @brief Copy the contained value into a new Base block.
				 * @param other Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				Optional(const Optional& other);

				/**
				 * @brief Take the contained block. @p other is left empty.
				 * @param other Source.
				 */
				Optional(Optional&& other) noexcept;

				/**
				 * @brief Destroy a held value and release its Base block.
				 */
				~Optional() noexcept;

				/**
				 * @brief Copy-assign. The previous value is released only after the copy exists.
				 * @param other Source.
				 * @return This optional.
				 * @throws AllocationError The replacement block could not be allocated.
				 */
				Optional& operator=(const Optional& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This optional.
				 */
				Optional& operator=(Optional&& other) noexcept;

				/**
				 * @brief Copy-assign a value.
				 * @param value Value to store.
				 * @return This optional.
				 * @throws AllocationError The replacement block could not be allocated.
				 */
				Optional& operator=(const T& value);

				/**
				 * @brief Move-assign a value.
				 * @param value Value to store.
				 * @return This optional.
				 * @throws AllocationError The replacement block could not be allocated.
				 */
				Optional& operator=(T&& value);

				/**
				 * @brief Assign a value that can construct T.
				 * @tparam U Source type.
				 * @param value Source value.
				 * @return This optional.
				 * @throws AllocationError The replacement block could not be allocated.
				 */
				template<class U>
				requires (!Type::SameAs<U, Optional>) &&
					(!Type::SameAs<U, std::nullopt_t>) &&
					Type::ConstructibleFrom<T, U>
				Optional& operator=(U&& value) {
					emplace(std::forward<U>(value));
					return *this;
				}

				/**
				 * @brief Reset this optional.
				 * @param value Empty-state tag.
				 * @return This optional.
				 */
				Optional& operator=(std::nullopt_t value) noexcept;

				/**
				 * @brief Copy-assign a caller-owned STL optional. Its state is unchanged.
				 * @param value Source.
				 * @return This optional.
				 * @throws AllocationError The replacement block could not be allocated.
				 */
				Optional& operator=(const std::optional<T>& value);

				/**
				 * @brief Move-assign a caller-owned STL optional. `has_value()` is unchanged.
				 * @param value Source.
				 * @return This optional.
				 * @throws AllocationError The replacement block could not be allocated.
				 */
				Optional& operator=(std::optional<T>&& value);

				/**
				 * @brief Exchange contained blocks. Does not allocate.
				 * @param other Optional to exchange with.
				 */
				void swap(Optional& other) noexcept;

				/**
				 * @brief Exchange two optionals.
				 * @param left First optional.
				 * @param right Second optional.
				 */
				friend void swap(Optional& left, Optional& right) noexcept { left.swap(right); }

				/**
				 * @brief Test whether a value is held.
				 * @return Whether the optional contains a value.
				 */
				bool has_value() const noexcept { return m_value != nullptr; }

				/**
				 * @brief Test whether a value is held.
				 * @return Whether the optional contains a value.
				 */
				explicit operator bool() const noexcept { return has_value(); }

				/**
				 * @brief Return the mutable iterator, or end when empty.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept { return m_value; }

				/**
				 * @brief Return the end mutable iterator.
				 * @return One past the value, or equal to @ref begin when empty.
				 */
				iterator end() noexcept { return m_value == nullptr ? nullptr : m_value + 1; }

				/**
				 * @brief Return the read-only iterator, or end when empty.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept { return m_value; }

				/**
				 * @brief Return the end read-only iterator.
				 * @return One past the value, or equal to @ref begin when empty.
				 */
				const_iterator end() const noexcept { return m_value == nullptr ? nullptr : m_value + 1; }

				/**
				 * @brief Return the read-only iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Return the contained value.
				 * @return Contained value. Valid until `reset` or a move.
				 * @throws BadOptionalAccess The optional is empty.
				 */
				T& value() &;

				/**
				 * @brief Return the contained value.
				 * @return Contained value. Valid until `reset` or a move.
				 * @throws BadOptionalAccess The optional is empty.
				 */
				const T& value() const &;

				/**
				 * @brief Return the contained value from an rvalue.
				 * @return Contained value.
				 * @throws BadOptionalAccess The optional is empty.
				 */
				T&& value() &&;

				/**
				 * @brief Return the contained value from a const rvalue.
				 * @return Contained value.
				 * @throws BadOptionalAccess The optional is empty.
				 */
				const T&& value() const &&;

				/**
				 * @brief Return the contained value or a fallback.
				 * @tparam U Fallback type convertible to T.
				 * @param fallback Value returned when empty.
				 * @return A copy of the contained value, or the forwarded fallback.
				 */
				template<class U>
				requires Type::ConvertibleTo<U, T>
				T value_or(U&& fallback) const & {
					return has_value() ? *m_value : T(std::forward<U>(fallback));
				}

				/**
				 * @brief Return the contained value or a fallback from an rvalue.
				 * @tparam U Fallback type convertible to T.
				 * @param fallback Value returned when empty.
				 * @return The moved contained value, or the forwarded fallback.
				 */
				template<class U>
				requires Type::ConvertibleTo<U, T>
				T value_or(U&& fallback) && {
					return has_value() ? T(std::move(*m_value)) : T(std::forward<U>(fallback));
				}

				/**
				 * @brief Access the contained value.
				 * @return Contained value. Valid until `reset` or a move.
				 * @pre The optional contains a value.
				 */
				T& operator*() &;

				/**
				 * @brief Access the contained value.
				 * @return Contained value. Valid until `reset` or a move.
				 * @pre The optional contains a value.
				 */
				const T& operator*() const &;

				/**
				 * @brief Access the contained value from an rvalue.
				 * @return Contained value.
				 * @pre The optional contains a value.
				 */
				T&& operator*() &&;

				/**
				 * @brief Access the contained value from a const rvalue.
				 * @return Contained value.
				 * @pre The optional contains a value.
				 */
				const T&& operator*() const &&;

				/**
				 * @brief Access a member of the contained value.
				 * @return Pointer to the contained value.
				 * @pre The optional contains a value.
				 */
				T* operator->();

				/**
				 * @brief Access a member of the contained value.
				 * @return Pointer to the contained value.
				 * @pre The optional contains a value.
				 */
				const T* operator->() const;

				/**
				 * @brief Compare states and, when present, values.
				 * @param other Optional to compare.
				 * @return Whether both are empty or contain equal values.
				 */
				bool operator==(const Optional& other) const requires Type::EqualityComparable<T> {
					return has_value() == other.has_value() && (!has_value() || *m_value == *other.m_value);
				}

				/**
				 * @brief Compare states and, when present, values.
				 * @param other Optional to compare.
				 * @return Whether the optionals differ.
				 */
				bool operator!=(const Optional& other) const requires Type::EqualityComparable<T> {
					return !(*this == other);
				}

				/**
				 * @brief Compare with the empty tag.
				 * @return Whether this optional is empty.
				 */
				bool operator==(std::nullopt_t) const noexcept { return !has_value(); }

				/**
				 * @brief Compare the empty tag with this optional.
				 * @param self Optional to compare.
				 * @return Whether this optional is empty.
				 */
				friend bool operator==(std::nullopt_t, const Optional& self) noexcept { return !self.has_value(); }

				/**
				 * @brief Order empty before engaged.
				 * @return Equivalent when empty, greater when engaged.
				 */
				std::strong_ordering operator<=>(std::nullopt_t) const noexcept {
					return has_value() ? std::strong_ordering::greater : std::strong_ordering::equivalent;
				}

				/**
				 * @brief Compare the contained value with a Safe component. Empty is not equal.
				 * @tparam U Safe component comparable with T. An iterator is not a candidate: checking `U == T` would re-enter this operator.
				 * @param value Value to compare.
				 * @return Whether a value is held and compares equal.
				 */
				template<class U>
				requires Type::SafeComponent<std::remove_cvref_t<U>> &&
					(!Type::SameAs<std::remove_cvref_t<U>, Optional>) &&
					requires(const T& left, const U& right) { { left == right } -> Type::ConvertibleTo<bool>; }
				bool operator==(const U& value) const {
					return has_value() && *m_value == value;
				}

				/**
				 * @brief Compare a Safe component with this optional.
				 * @tparam U Safe component comparable with T. An iterator is not a candidate: checking `U == T` would re-enter this operator.
				 * @param value Value to compare.
				 * @param self Optional to compare.
				 * @return Whether a value is held and compares equal.
				 */
				template<class U>
				requires Type::SafeComponent<std::remove_cvref_t<U>> &&
					(!Type::SameAs<std::remove_cvref_t<U>, Optional>) &&
					requires(const U& left, const T& right) { { left == right } -> Type::ConvertibleTo<bool>; }
				friend bool operator==(const U& value, const Optional& self) {
					return self.has_value() && value == *self.m_value;
				}

				/**
				 * @brief Order against a Safe component. Empty is less than any value.
				 * @tparam U Safe component orderable against T. An iterator is not a candidate.
				 * @param value Value to compare.
				 * @return Three-way comparison result.
				 */
				template<class U>
				requires Type::SafeComponent<std::remove_cvref_t<U>> &&
					(!Type::SameAs<std::remove_cvref_t<U>, Optional>) &&
					requires(const T& left, const U& right) { left <=> right; }
				auto operator<=>(const U& value) const {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>;
					if (!has_value())
						return Result::less;
					return static_cast<Result>(*m_value <=> value);
				}

				/**
				 * @brief Order empty before engaged, then order the values.
				 * @param other Optional to compare.
				 * @return Three-way comparison result.
				 */
				auto operator<=>(const Optional& other) const requires Type::ThreeWayComparable<T> {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const T&>())>;
					if (has_value() && other.has_value())
						return static_cast<Result>(*m_value <=> *other.m_value);
					if (has_value())
						return Result::greater;
					if (other.has_value())
						return Result::less;
					return Result::equivalent;
				}

				/**
				 * @brief Compare with a caller-owned STL optional.
				 * @tparam U Other value type.
				 * @param other Optional to compare.
				 * @return Whether both are empty or their values compare equal.
				 */
				template<class U>
				requires requires(const T& left, const U& right) { { left == right } -> Type::ConvertibleTo<bool>; }
				bool operator==(const std::optional<U>& other) const {
					return has_value() == other.has_value() && (!has_value() || *m_value == *other);
				}

				/**
				 * @brief Compare a caller-owned STL optional with this optional.
				 * @tparam U Other value type.
				 * @param other STL optional.
				 * @param self Optional to compare.
				 * @return Whether both are empty or their values compare equal.
				 */
				template<class U>
				requires requires(const U& left, const T& right) { { left == right } -> Type::ConvertibleTo<bool>; }
				friend bool operator==(const std::optional<U>& other, const Optional& self) {
					return self == other;
				}

				/**
				 * @brief Order against a caller-owned STL optional. Empty is less than engaged.
				 * @tparam U Other value type.
				 * @param other Optional to compare.
				 * @return Three-way comparison result.
				 */
				template<class U>
				requires requires(const T& left, const U& right) { left <=> right; }
				auto operator<=>(const std::optional<U>& other) const {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>;
					if (has_value() && other.has_value())
						return static_cast<Result>(*m_value <=> *other);
					if (has_value())
						return Result::greater;
					if (other.has_value())
						return Result::less;
					return Result::equivalent;
				}

				/**
				 * @brief Invoke a function on the value and return its optional result.
				 * @tparam F Callable returning a Safe optional.
				 * @param function Function invoked with the contained value.
				 * @return An empty optional of the result type, or the function result.
				 */
				template<class F>
				requires Type::Invocable<F, T&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, T&>>>
				auto and_then(F&& function) & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
					if (!has_value())
						return Result{};
					return std::invoke(std::forward<F>(function), *m_value);
				}

				/**
				 * @brief Invoke a function on the value and return its optional result.
				 * @tparam F Callable returning a Safe optional.
				 * @param function Function invoked with the contained value.
				 * @return An empty optional of the result type, or the function result.
				 */
				template<class F>
				requires Type::Invocable<F, const T&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, const T&>>>
				auto and_then(F&& function) const & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
					if (!has_value())
						return Result{};
					return std::invoke(std::forward<F>(function), static_cast<const T&>(*m_value));
				}

				/**
				 * @brief Invoke a function on an rvalue value and return its optional result.
				 * @tparam F Callable returning a Safe optional.
				 * @param function Function invoked with the contained value.
				 * @return An empty optional of the result type, or the function result.
				 */
				template<class F>
				requires Type::Invocable<F, T&&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, T&&>>>
				auto and_then(F&& function) && {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
					if (!has_value())
						return Result{};
					return std::invoke(std::forward<F>(function), std::move(*m_value));
				}

				/**
				 * @brief Map the contained value to another Safe component.
				 * @tparam F Callable returning a Safe component.
				 * @param function Function invoked with the contained value.
				 * @return An empty optional, or an optional holding the mapped value.
				 */
				template<class F>
				requires Type::Invocable<F, T&> && Type::SafeComponent<std::remove_cvref_t<std::invoke_result_t<F, T&>>>
				auto transform(F&& function) & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
					if (!has_value())
						return Optional<Result>{};
					return Optional<Result>(std::invoke(std::forward<F>(function), *m_value));
				}

				/**
				 * @brief Map the contained value to another Safe component.
				 * @tparam F Callable returning a Safe component.
				 * @param function Function invoked with the contained value.
				 * @return An empty optional, or an optional holding the mapped value.
				 */
				template<class F>
				requires Type::Invocable<F, const T&> && Type::SafeComponent<std::remove_cvref_t<std::invoke_result_t<F, const T&>>>
				auto transform(F&& function) const & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
					if (!has_value())
						return Optional<Result>{};
					return Optional<Result>(std::invoke(std::forward<F>(function), static_cast<const T&>(*m_value)));
				}

				/**
				 * @brief Map an rvalue contained value to another Safe component.
				 * @tparam F Callable returning a Safe component.
				 * @param function Function invoked with the contained value.
				 * @return An empty optional, or an optional holding the mapped value.
				 */
				template<class F>
				requires Type::Invocable<F, T&&> && Type::SafeComponent<std::remove_cvref_t<std::invoke_result_t<F, T&&>>>
				auto transform(F&& function) && {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
					if (!has_value())
						return Optional<Result>{};
					return Optional<Result>(std::invoke(std::forward<F>(function), std::move(*m_value)));
				}

				/**
				 * @brief Return this optional when engaged, otherwise invoke a fallback.
				 * @tparam F Callable returning Optional.
				 * @param function Fallback callable.
				 * @return A copy of this optional, or the callable result.
				 */
				template<class F>
				requires Type::Invocable<F> && Type::SameAs<std::invoke_result_t<F>, Optional>
				Optional or_else(F&& function) const & {
					if (has_value())
						return *this;
					return std::invoke(std::forward<F>(function));
				}

				/**
				 * @brief Return this rvalue optional, or invoke a fallback.
				 * @tparam F Callable returning Optional.
				 * @param function Fallback callable.
				 * @return This optional moved, or the callable result.
				 */
				template<class F>
				requires Type::Invocable<F> && Type::SameAs<std::invoke_result_t<F>, Optional>
				Optional or_else(F&& function) && {
					if (has_value())
						return std::move(*this);
					return std::invoke(std::forward<F>(function));
				}

				/**
				 * @brief Construct or replace the contained value.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to T.
				 * @return Contained value. Valid until `reset` or a move.
				 * @throws AllocationError The block could not be allocated. The previous value is kept.
				 */
				template<class... Args>
				T& emplace(Args&&... args) {
					T* created = Make(std::forward<Args>(args)...);
					Release();
					m_value = created;
					return *m_value;
				}

				/**
				 * @brief Construct or replace T from an initializer list.
				 * @tparam U Initializer-list element type.
				 * @tparam Args Trailing constructor argument types.
				 * @param values Initializer-list elements.
				 * @param args Trailing arguments forwarded to T.
				 * @return Contained value. Valid until `reset` or a move.
				 * @throws AllocationError The block could not be allocated. The previous value is kept.
				 */
				template<class U, class... Args>
				T& emplace(std::initializer_list<U> values, Args&&... args) {
					T* created = Make(values, std::forward<Args>(args)...);
					Release();
					m_value = created;
					return *m_value;
				}

				/**
				 * @brief Destroy a held value and release its block.
				 */
				void reset() noexcept;

				/**
				 * @brief Copy the value into caller-owned STL storage.
				 * @return A `std::optional` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::optional<T>() const {
					if (!has_value())
						return std::nullopt;
					return std::optional<T>(*m_value);
				}

			private:
				/**
				 * @brief Allocate a Base block and construct T in it.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to T.
				 * @return Pointer to the constructed value.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class... Args>
				static T* Make(Args&&... args) {
					void* block = Heap::Allocate(sizeof(T));
					try {
						return new (block) T(std::forward<Args>(args)...);
					} catch (...) {
						Heap::Free(block);
						throw;
					}
				}

				/**
				 * @brief Destroy a held value and release its block. Empty is a no-op.
				 */
				void Release() noexcept;

				T* m_value;	///< Base-owned value, or null when empty.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes an optional of an already safe component.
		 * @tparam T Safe component.
		 */
		template<SafeComponent T>
		requires Type::IsSafe<T>::value
		struct IsSafe<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the contained value.
		 * @tparam T Conditionally safe component.
		 */
		template<SafeComponent T>
		requires Type::MaybeSafe<T>
		struct IsMaybeSafe<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Registers a Safe optional for optional-aware generic APIs.
		 * @tparam T Safe component.
		 */
		template<SafeComponent T>
		struct IsSafeOptional<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Admits a Safe optional as a collection value.
		 * @tparam T Safe component.
		 */
		template<SafeComponent T>
		struct IsSafeValue<Safe::Optional<T>>: std::true_type {};
	}
}

/**
 * @brief Cross-module hash of a @ref StormByte::Safe::Optional.
 * @tparam T Safe component.
 */
template<StormByte::Type::SafeComponent T>
struct StormByte::Safe::Hash<StormByte::Safe::Optional<T>> {
    /**
     * @brief Hash @p value. Empty and engaged differ.
     * @param value Optional.
     * @return Hash.
     */
    STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::Safe::Optional<T>& value) const noexcept {
        if (!value.has_value())
            return Hash<std::uint8_t>{}(0);
        return HashCombine(Hash<std::uint8_t>{}(1), Hash<T>{}(*value));
    }
};

/**
 * @brief Hash of a @ref StormByte::Safe::Optional for an STL unordered container in this module.
 * @tparam T Safe component.
 */
template<StormByte::Type::SafeComponent T>
struct std::hash<StormByte::Safe::Optional<T>> {
    /**
     * @brief Hash @p value. Empty and engaged differ.
     * @param value Optional.
     * @return Hash.
     */
    std::size_t operator()(const StormByte::Safe::Optional<T>& value) const noexcept {
        return StormByte::Safe::Hash<StormByte::Safe::Optional<T>>{}(value);
    }
};

#include <StormByte/safe/optional.txx>
