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

#include <StormByte/safe/vector.hxx>
#include <StormByte/type_traits/comparison.hxx>

#include <compare>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <optional>
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
		 * @namespace StormByte::Safe::Detail
		 * @brief Private implementation helpers for Safe containers.
		 */
		namespace Detail {
			/**
			 * @brief Recognizes callback-safe optional results with an empty-state observer.
			 * @tparam R Candidate monadic result.
			 */
			template<class R>
			concept SafeOptionalResult = Type::SafeComponent<R> && requires(const R& result) {
				{ result.has_value() } -> std::convertible_to<bool>;
			};
		}

		/**
		 * @class Optional
		 * @brief Zero or one Safe-owned value, reusing opaque sequence ownership.
		 * @tparam T Safe value.
		 * @note No std::optional layout. Vector's ABI, ownership and failure rules apply.
		 * @note Mutable dereference is a callback-backed proxy that reads by value and
		 *       writes through the owner callback. Const dereference returns a value copy.
		 * @note Arrow access uses a read-only snapshot proxy valid for the full expression;
		 *       mutations of T are not written back and its pointer must not be retained.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Optional final {
			public:
				using value_type = T; ///< Contained type.
				using iterator = typename Vector<T>::iterator; ///< Mutable 0/1-element iterator.
				using const_iterator = typename Vector<T>::const_iterator; ///< Read-only 0/1-element iterator.

				/**
				 * @class Reference
				 * @brief Mutable Optional proxy with value-copy reads and callback writes.
				 * @note Valid only while the owning Optional remains alive and is not moved.
				 */
				class Reference final {
					public:
						/**
						 * @brief Bind to a callback-backed element proxy.
						 * @param value Callback-backed element proxy.
						 */
						explicit Reference(typename iterator::reference value): m_value(std::move(value)) {}

						/**
						 * @brief Copy the current value into caller-owned storage.
						 * @return Value copy.
						 */
						operator T() const { return static_cast<T>(m_value); }

						/**
						 * @brief Assign a value through the creator callback.
						 * @param value Replacement value.
						 * @return This proxy.
						 */
						Reference& operator=(const T& value) {
							m_value = value;
							return *this;
						}

						/**
						 * @brief Move-assign a value through the creator callback.
						 * @param value Replacement value.
						 * @return This proxy.
						 */
						Reference& operator=(T&& value) {
							m_value = value;
							return *this;
						}

						/**
						 * @brief Assign a value through a const proxy.
						 * @param value Replacement value.
						 * @return This proxy.
						 */
						const Reference& operator=(const T& value) const {
							m_value = value;
							return *this;
						}

						/**
						 * @brief Move-assign through a const proxy.
						 * @param value Replacement value.
						 * @return This proxy.
						 */
						const Reference& operator=(T&& value) const {
							m_value = value;
							return *this;
						}

						/**
						 * @brief Assign from another proxy.
						 * @param other Source proxy.
						 * @return This proxy.
						 */
						Reference& operator=(const Reference& other) { return *this = static_cast<T>(other); }

						/**
						 * @brief Compare the copied value with another T.
						 * @param left Proxy operand.
						 * @param right Value operand.
						 * @return Whether the values compare equal.
						 */
						friend bool operator==(const Reference& left, const T& right) requires Type::EqualityComparable<T> {
							return static_cast<T>(left) == right;
						}

						/**
						 * @brief Compare T with the copied value.
						 * @param left Value operand.
						 * @param right Proxy operand.
						 * @return Whether the values compare equal.
						 */
						friend bool operator==(const T& left, const Reference& right) requires Type::EqualityComparable<T> {
							return left == static_cast<T>(right);
						}

					private:
						typename iterator::reference m_value; ///< Callback-backed element proxy.
				};

				/**
				 * @class ArrowProxy
				 * @brief Owns a read-only value snapshot for one arrow-access expression.
				 * @note Pointers obtained from this proxy expire with the proxy at the end of
				 *       the full expression; the snapshot is never written back.
				 */
				class ArrowProxy final {
					public:
						/**
						 * @brief Take ownership of an access snapshot.
						 * @param value Snapshot to retain.
						 */
						explicit ArrowProxy(T value): m_value(std::move(value)) {}

						/**
						 * @brief Return a read-only pointer to this snapshot.
						 * @return Pointer valid only while this proxy lives.
						 */
						const T* operator->() const noexcept { return &m_value; }

					private:
						T m_value; ///< Caller-owned snapshot, never creator storage.
				};

				using reference = Reference; ///< Mutable callback-backed value proxy.
				using const_reference = typename const_iterator::reference; ///< Read-only value copy.

				/**
				 * @brief Construct an empty optional in the calling module.
				 */
				STORMBYTE_FORCE_INLINE Optional();

				/**
				 * @brief Construct an empty optional.
				 * @param value Empty-state tag.
				 */
				STORMBYTE_FORCE_INLINE Optional(std::nullopt_t value);

				/**
				 * @brief Construct the contained value in Safe-owned storage.
				 * @tparam Args Constructor argument types.
				 * @param tag In-place construction tag.
				 * @param args Arguments forwarded to T.
				 */
				template<class... Args>
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
				 */
				template<class U, class... Args>
				explicit Optional(std::in_place_t tag, std::initializer_list<U> values, Args&&... args): Optional() {
					(void)tag;
					emplace(values, std::forward<Args>(args)...);
				}

				/**
				 * @brief Copy a value into Safe-owned storage.
				 * @param value Value to store.
				 */
				Optional(const T& value);

				/**
				 * @brief Move a value into Safe-owned storage.
				 * @param value Value to store.
				 */
				Optional(T&& value);

				/**
				 * @brief Convert a caller-owned STL optional when U constructs T.
				 * @tparam U Source value type.
				 */
				template<class U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, const U&>
				explicit(!std::convertible_to<const U&, T>) Optional(const std::optional<U>& value): Optional() {
					if (value)
						emplace(T(*value));
				}

				/**
				 * @brief Convert a moved STL optional when U constructs T, resetting it after successful transfer.
				 * @tparam U Source value type.
				 */
				template<class U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, U&&>
				explicit(!std::convertible_to<U&&, T>) Optional(std::optional<U>&& value): Optional() {
					if (value) {
						emplace(T(std::move(*value)));
						value.reset();
					}
				}

				/**
				 * @brief Construct from a value that can construct T.
				 * @tparam U Source value type.
				 */
				template<class U>
				requires (!std::same_as<std::remove_cvref_t<U>, T>) && std::constructible_from<T, U>
				explicit(!std::convertible_to<U, T>) Optional(U&& value): Optional() {
					emplace(std::forward<U>(value));
				}

				/**
				 * @brief Convert another Safe optional when its value constructs T.
				 * @tparam U Source Safe value type.
				 */
				template<Type::SafeValue U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, const U&>
				explicit(!std::convertible_to<const U&, T>) Optional(const Optional<U>& other): Optional() {
					if (other.has_value())
					emplace(other.value());
				}

				/**
				 * @brief Convert a moved Safe optional when its value constructs T, resetting it after successful transfer.
				 * @tparam U Source Safe value type.
				 */
				template<Type::SafeValue U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, U&&>
				explicit(!std::convertible_to<U&&, T>) Optional(Optional<U>&& other): Optional() {
					if (other.has_value()) {
						emplace(std::move(other.value()));
						other.reset();
					}
				}

				/**
				 * @brief Copy a caller-owned STL optional.
				 * @param value Source value; its ownership remains with its owner.
				 */
				Optional(const std::optional<T>& value);

				/**
				 * @brief Move an STL rvalue's value into local Safe storage.
				 * @param value Source optional; it is reset after transfer.
				 */
				Optional(std::optional<T>&& value);

				/**
				 * @brief Deep copy in the original creator module.
				 * @param other Source.
				 */
				Optional(const Optional& other) = default;

				/**
				 * @brief Transfer state, leaving source empty.
				 * @param other Source.
				 */
				Optional(Optional&& other) noexcept = default;

				/**
				 * @brief Release through creator-module callbacks.
				 */
				~Optional() noexcept = default;

				/**
				 * @brief Deep copy with strong guarantee.
				 * @param other Source.
				 * @return This optional.
				 */
				Optional& operator=(const Optional& other) = default;

				/**
				 * @brief Release old state and transfer.
				 * @param other Source.
				 * @return This optional.
				 */
				Optional& operator=(Optional&& other) noexcept = default;

				/**
				 * @brief Copy-assign a value, preserving this optional if preparation fails.
				 * @param value Value to store.
				 * @return This optional.
				 */
				Optional& operator=(const T& value);

				/**
				 * @brief Move-assign a value, preserving this optional if preparation fails.
				 * @param value Value to store.
				 * @return This optional.
				 */
				Optional& operator=(T&& value);

				/**
				 * @brief Assign from a value that can construct T.
				 * @tparam U Source value type.
				 * @param value Source value.
				 * @return This optional.
				 */
				template<class U>
				requires (!std::same_as<std::remove_cvref_t<U>, T>) && std::constructible_from<T, U>
				Optional& operator=(U&& value) {
					T converted(std::forward<U>(value));
					return *this = std::move(converted);
				}

				/**
				 * @brief Reset this optional through the empty-state tag.
				 * @param value Empty-state tag.
				 * @return This optional.
				 */
				Optional& operator=(std::nullopt_t value) {
					(void)value;
					reset();
					return *this;
				}

				/**
				 * @brief Copy from caller-owned STL optional storage.
				 * @param value Source optional.
				 * @return This optional.
				 */
				Optional& operator=(const std::optional<T>& value) {
					if (!value) {
						reset();
						return *this;
					}
					return *this = *value;
				}

				/**
				 * @brief Move a value from caller-owned STL optional storage.
				 * @param value Source optional, reset after transfer.
				 * @return This optional.
				 */
				Optional& operator=(std::optional<T>&& value) {
					if (!value) {
						reset();
						return *this;
					}
					*this = std::move(*value);
					value.reset();
					return *this;
				}

				/**
				 * @brief Convert and copy from another Safe optional.
				 * @tparam U Source Safe value type.
				 * @param other Source optional.
				 * @return This optional.
				 */
				template<Type::SafeValue U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, const U&>
				Optional& operator=(const Optional<U>& other) {
					if (!other.has_value()) {
						reset();
						return *this;
					}
					Optional replacement(other.value());
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Convert and move from another Safe optional.
				 * @tparam U Source Safe value type.
				 * @param other Source optional, reset after successful conversion.
				 * @return This optional.
				 */
				template<Type::SafeValue U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, U&&>
				Optional& operator=(Optional<U>&& other) {
					if (!other.has_value()) {
						reset();
						return *this;
					}
					T converted(std::move(other.value()));
					*this = std::move(converted);
					other.reset();
					return *this;
				}

				/**
				 * @brief Convert and copy from a caller-owned STL optional.
				 * @tparam U Source value type.
				 * @param value Source optional.
				 * @return This optional.
				 */
				template<class U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, const U&>
				Optional& operator=(const std::optional<U>& value) {
					if (!value) {
						reset();
						return *this;
					}
					Optional replacement{T(*value)};
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Convert and move from a caller-owned STL optional.
				 * @tparam U Source value type.
				 * @param value Source optional, reset after successful conversion.
				 * @return This optional.
				 */
				template<class U>
				requires (!std::same_as<U, T>) && std::constructible_from<T, U&&>
				Optional& operator=(std::optional<U>&& value) {
					if (!value) {
						reset();
						return *this;
					}
					Optional replacement(T(std::move(*value)));
					*this = std::move(replacement);
					value.reset();
					return *this;
				}

				/**
				 * @brief Exchange Safe storage and its creator callbacks with another optional.
				 * @param other Optional to exchange with.
				 */
				void swap(Optional& other) noexcept {
					if (this == &other)
						return;
					Optional temporary(std::move(*this));
					*this = std::move(other);
					other = std::move(temporary);
				}

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
				bool has_value() const noexcept { return !m_value.empty(); }

				/**
				 * @brief Test whether a value is held.
				 * @return Whether the optional contains a value.
				 */
				explicit operator bool() const noexcept { return has_value(); }

				/**
				 * @brief Return the first mutable iterator, or end when empty.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept { return m_value.begin(); }

				/**
				 * @brief Return the end mutable iterator.
				 * @return Mutable iterator.
				 */
				iterator end() noexcept { return m_value.end(); }

				/**
				 * @brief Return the first read-only iterator, or end when empty.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept { return m_value.begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return Read-only iterator.
				 */
				const_iterator end() const noexcept { return m_value.end(); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Return a copy of the value.
				 * @return Contained value.
				 * @throws StormByte::Exception The optional is empty or copying failed.
				 */
				T value() const {
					if (!has_value())
						Detail::ThrowSafeConversionFailure("Safe optional value is missing");
					return m_value.front();
				}

				/**
				 * @brief Return the contained value or a fallback, by value.
				 * @param fallback Value returned when empty.
				 * @return A copy of the contained value or the moved fallback.
				 */
				template<class U>
				requires std::convertible_to<U&&, T>
				T value_or(U&& fallback) const & {
					return has_value() ? value() : T(std::forward<U>(fallback));
				}

				/**
				 * @brief Return the contained value or a fallback, by value, on an rvalue.
				 * @tparam U Fallback type implicitly convertible to T.
				 * @param fallback Value returned when empty.
				 * @return A copy of the contained value or the forwarded fallback.
				 */
				template<class U>
				requires std::convertible_to<U&&, T>
				T value_or(U&& fallback) && {
					return has_value() ? value() : T(std::forward<U>(fallback));
				}

				/**
				 * @brief Access the mutable value through a callback-backed proxy.
				 * @return Proxy; reads copy T and writes use the creator callback. The Optional
				 *         must outlive the proxy, which is invalidated by moving the Optional.
				 * @throws StormByte::Exception The optional is empty or access failed.
				 */
				reference operator*() & { return reference(m_value.front()); }

				/**
				 * @brief Copy the value from a read-only optional.
				 * @return Contained value copy.
				 * @throws StormByte::Exception The optional is empty or copying failed.
				 */
				const_reference operator*() const & { return m_value.front(); }

				/**
				 * @brief Read an rvalue optional by value to avoid returning a dangling proxy.
				 * @return Contained value copy.
				 * @throws StormByte::Exception The optional is empty or copying failed.
				 */
				T operator*() && { return value(); }

				/**
				 * @brief Read a const rvalue optional by value.
				 * @return Contained value copy.
				 * @throws StormByte::Exception The optional is empty or copying failed.
				 */
				T operator*() const && { return value(); }

				/**
				 * @brief Access const members through a caller-owned value snapshot.
				 * @return Arrow proxy whose pointer is valid through the full expression only.
				 * @throws StormByte::Exception The optional is empty or copying failed.
				 */
				ArrowProxy operator->() const & { return ArrowProxy(value()); }

				/**
				 * @brief Access const members of an rvalue through an owned snapshot.
				 * @return Arrow proxy valid through the full expression only.
				 * @throws StormByte::Exception The optional is empty or copying failed.
				 */
				ArrowProxy operator->() const && { return ArrowProxy(value()); }

				/**
				 * @brief Compare optional states and, when present, their values.
				 * @param other Optional to compare.
				 * @return Whether both are empty or contain equal values.
				 */
				bool operator==(const Optional& other) const requires Type::EqualityComparable<T> {
					return has_value() == other.has_value() && (!has_value() || value() == other.value());
				}

				/**
				 * @brief Compare optional states and, when present, their values.
				 * @param other Optional to compare.
				 * @return Whether the optionals differ.
				 */
				bool operator!=(const Optional& other) const requires Type::EqualityComparable<T> {
					return !(*this == other);
				}

				/**
				 * @brief Compare optionals with different Safe value types.
				 * @tparam U Other Safe value type.
				 * @param other Optional to compare.
				 * @return Whether both are empty or their values compare equal.
				 */
				template<Type::SafeValue U>
				requires (!std::same_as<T, U>) && requires(const T& left, const U& right) {
					{ left == right } -> std::convertible_to<bool>;
				}
				bool operator==(const Optional<U>& other) const {
					return has_value() == other.has_value() && (!has_value() || value() == other.value());
				}

				/**
				 * @brief Compare optionals with different Safe value types for ordering.
				 * @tparam U Other Safe value type.
				 * @param other Optional to compare.
				 * @return Three-way comparison result, with empty before engaged.
				 */
				template<Type::SafeValue U>
				requires (!std::same_as<T, U>) && requires(const T& left, const U& right) { left <=> right; } &&
					(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>, void>)
				auto operator<=>(const Optional<U>& other) const {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>;
					if (has_value() && other.has_value())
						return static_cast<Result>(value() <=> other.value());
					if (has_value())
						return Result::greater;
					if (other.has_value())
						return Result::less;
					return Result::equivalent;
				}

				/**
				 * @brief Compare this optional with a caller-owned STL optional.
				 * @tparam U Other value type.
				 * @param other Optional to compare.
				 * @return Whether both are empty or their values compare equal.
				 */
				template<class U>
				requires requires(const T& left, const U& right) { { left == right } -> std::convertible_to<bool>; }
				bool operator==(const std::optional<U>& other) const {
					return has_value() == other.has_value() && (!has_value() || value() == *other);
				}

				/**
				 * @brief Compare this optional with a caller-owned STL optional for ordering.
				 * @tparam U Other value type.
				 * @param other Optional to compare.
				 * @return Three-way comparison result, with empty before engaged.
				 */
				template<class U>
				requires requires(const T& left, const U& right) { left <=> right; } &&
					(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>, void>)
				auto operator<=>(const std::optional<U>& other) const {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>;
					if (has_value() && other.has_value())
						return static_cast<Result>(value() <=> *other);
					if (has_value())
						return Result::greater;
					if (other.has_value())
						return Result::less;
					return Result::equivalent;
				}

				/**
				 * @brief Compare a caller-owned STL optional with this Safe optional.
				 * @tparam U Other value type.
				 * @param left STL optional.
				 * @param right Safe optional.
				 * @return Whether both are empty or their values compare equal.
				 */
				template<class U>
				requires requires(const U& left, const T& right) { { left == right } -> std::convertible_to<bool>; }
				friend bool operator==(const std::optional<U>& left, const Optional& right) {
					return left.has_value() == right.has_value() && (!left.has_value() || *left == right.value());
				}

				/**
				 * @brief Order a caller-owned STL optional against this Safe optional.
				 * @tparam U Other value type.
				 * @param left STL optional.
				 * @param right Safe optional.
				 * @return Three-way comparison result, with empty before engaged.
				 */
				template<class U>
				requires requires(const U& left, const T& right) { left <=> right; } &&
					(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const U&>() <=> std::declval<const T&>())>, void>)
				friend auto operator<=>(const std::optional<U>& left, const Optional& right) {
					using Result = std::common_comparison_category_t<decltype(std::declval<const U&>() <=> std::declval<const T&>())>;
					if (left.has_value() && right.has_value())
						return static_cast<Result>(*left <=> right.value());
					if (left.has_value())
						return Result::greater;
					if (right.has_value())
						return Result::less;
					return Result::equivalent;
				}

				/**
				 * @brief Order two optionals, with an empty optional ordered before a value.
				 * @param other Optional to compare.
				 * @return Three-way comparison result.
				 */
				auto operator<=>(const Optional& other) const
					requires Type::ThreeWayComparable<T> &&
						(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const T&>())>, void>) {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const T&>())>;
					if (has_value() && other.has_value())
						return static_cast<Result>(value() <=> other.value());
					if (has_value())
						return Result::greater;
					if (other.has_value())
						return Result::less;
					return Result::equivalent;
				}

				/**
				 * @brief Compare the contained value with a value of type T.
				 * @param other Value to compare.
				 * @return Whether this optional contains an equal value.
				 */
				bool operator==(const T& other) const requires Type::EqualityComparable<T> {
					return has_value() && value() == other;
				}

				/**
				 * @brief Compare the contained value with a value of type T.
				 * @param other Value to compare.
				 * @return Whether this optional is empty or contains a different value.
				 */
				bool operator!=(const T& other) const requires Type::EqualityComparable<T> {
					return !(*this == other);
				}

				/**
				 * @brief Compare the contained value with a heterogeneous value.
				 * @tparam U Other operand type.
				 * @param other Value to compare.
				 * @return Whether this optional contains an equal value.
				 */
				template<class U>
				requires (!std::same_as<std::remove_cvref_t<U>, T>) && requires(const T& left, const U& right) {
					{ left == right } -> std::convertible_to<bool>;
				}
				bool operator==(const U& other) const {
					return has_value() && value() == other;
				}

				/**
				 * @brief Order the optional against a heterogeneous value.
				 * @tparam U Other operand type.
				 * @param other Value to compare.
				 * @return Three-way comparison result; an empty optional is less.
				 */
				template<class U>
				requires (!std::same_as<std::remove_cvref_t<U>, T>) && requires(const T& left, const U& right) { left <=> right; } &&
					(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>, void>)
				auto operator<=>(const U& other) const {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const U&>())>;
					if (!has_value())
						return Result::less;
					return static_cast<Result>(value() <=> other);
				}

				/**
				 * @brief Compare an optional with the empty-state tag.
				 * @param other Empty-state tag.
				 * @return Whether this optional is empty.
				 */
				bool operator==(std::nullopt_t other) const noexcept {
					(void)other;
					return !has_value();
				}

				/**
				 * @brief Compare an optional with the empty-state tag.
				 * @param other Empty-state tag.
				 * @return Whether this optional contains a value.
				 */
				bool operator!=(std::nullopt_t other) const noexcept { return !(*this == other); }

				/**
				 * @brief Order an optional relative to the empty-state tag.
				 * @param other Empty-state tag.
				 * @return Equivalent when empty; greater when engaged.
				 */
				std::strong_ordering operator<=>(std::nullopt_t other) const noexcept {
					(void)other;
					return has_value() ? std::strong_ordering::greater : std::strong_ordering::equal;
				}

				/**
				 * @brief Compare the empty-state tag with an optional.
				 * @param left Empty-state tag.
				 * @param right Optional to compare.
				 * @return Whether the optional is empty.
				 */
				friend bool operator==(std::nullopt_t left, const Optional& right) noexcept { return right == left; }

				/**
				 * @brief Order the empty-state tag relative to an optional.
				 * @param left Empty-state tag.
				 * @param right Optional to compare.
				 * @return Equivalent when empty; less when the optional contains a value.
				 */
				friend std::strong_ordering operator<=>(std::nullopt_t left, const Optional& right) noexcept {
					return 0 <=> (right == left ? 0 : 1);
				}

				/**
				 * @brief Order an optional and a value of type T.
				 * @param other Value to compare.
				 * @return Three-way comparison result.
				 */
				auto operator<=>(const T& other) const
					requires Type::ThreeWayComparable<T> &&
						(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const T&>())>, void>) {
					using Result = std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const T&>())>;
					if (!has_value())
						return Result::less;
					return static_cast<Result>(value() <=> other);
				}

				/**
				 * @brief Compare a value of type T with an optional.
				 * @param left Value to compare.
				 * @param right Optional to compare.
				 * @return Whether the optional contains an equal value.
				 */
				friend bool operator==(const T& left, const Optional& right) requires Type::EqualityComparable<T> {
					return right == left;
				}

				/**
				 * @brief Compare a value of type T with an optional.
				 * @param left Value to compare.
				 * @param right Optional to compare.
				 * @return Whether the optional is empty or contains a different value.
				 */
				friend bool operator!=(const T& left, const Optional& right) requires Type::EqualityComparable<T> {
					return !(right == left);
				}

				/**
				 * @brief Order a value of type T and an optional.
				 * @param left Value to compare.
				 * @param right Optional to compare.
				 * @return Three-way comparison result.
				 */
				friend auto operator<=>(const T& left, const Optional& right)
					requires Type::ThreeWayComparable<T> &&
						(!std::same_as<std::common_comparison_category_t<decltype(std::declval<const T&>() <=> std::declval<const T&>())>, void>) {
					using Result = std::common_comparison_category_t<decltype(left <=> left)>;
					if (!right.has_value())
						return Result::greater;
					return static_cast<Result>(left <=> right.value());
				}

				/**
				 * @brief Apply a callable to a mutable value snapshot when present.
				 * @tparam F Callable accepting T& and returning a Safe optional.
				 * @param function Callable to invoke.
				 * @return The callable's result, or an empty result. Snapshot mutations are
				 *         written back only after the callable returns successfully.
				 */
				template<class F>
				requires std::invocable<F, T&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, T&>>>
				auto and_then(F&& function) & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
					if (!has_value())
						return Result{};
					T snapshot = value();
					Result result = std::invoke(std::forward<F>(function), snapshot);
					*this = std::move(snapshot);
					return result;
				}

				/**
				 * @brief Apply a callable to a const value copy when present.
				 * @tparam F Callable accepting const T& and returning a Safe optional.
				 * @param function Callable to invoke.
				 * @return Callable result, or an empty result.
				 */
				template<class F>
				requires std::invocable<F, const T&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, const T&>>>
				auto and_then(F&& function) const & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
					if (!has_value())
						return Result{};
					const T snapshot = value();
					return std::invoke(std::forward<F>(function), snapshot);
				}

				/**
				 * @brief Apply a callable to a moved value copy when present.
				 * @tparam F Callable accepting T&& and returning a Safe optional.
				 * @param function Callable to invoke.
				 * @return Callable result, or an empty result.
				 */
				template<class F>
				requires std::invocable<F, T&&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, T&&>>>
				auto and_then(F&& function) && {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
					if (!has_value())
						return Result{};
					T snapshot = value();
					return std::invoke(std::forward<F>(function), std::move(snapshot));
				}

				/**
				 * @brief Apply a callable to a const value copy from a const rvalue.
				 * @tparam F Callable accepting const T&& and returning a Safe optional.
				 * @param function Callable to invoke.
				 * @return Callable result, or an empty result.
				 */
				template<class F>
				requires std::invocable<F, const T&&> && Detail::SafeOptionalResult<std::remove_cvref_t<std::invoke_result_t<F, const T&&>>>
				auto and_then(F&& function) const && {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
					if (!has_value())
						return Result{};
					const T snapshot = value();
					return std::invoke(std::forward<F>(function), std::move(snapshot));
				}

				/**
				 * @brief Apply a callable to a mutable value snapshot and wrap its Safe result.
				 * @tparam F Callable accepting T& and returning a SafeValue.
				 * @param function Callable to invoke.
				 * @return Optional containing the result, or empty. Snapshot mutations are
				 *         written back only after the callable returns successfully.
				 */
				template<class F>
				requires std::invocable<F, T&> && Type::SafeValue<std::remove_cvref_t<std::invoke_result_t<F, T&>>>
				auto transform(F&& function) & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
					if (!has_value())
						return Optional<Result>{};
					T snapshot = value();
					Result result = std::invoke(std::forward<F>(function), snapshot);
					*this = std::move(snapshot);
					return Optional<Result>(std::move(result));
				}

				/**
				 * @brief Apply a callable to a const value copy and wrap its Safe result.
				 * @tparam F Callable accepting const T& and returning a SafeValue.
				 * @param function Callable to invoke.
				 * @return Optional containing the result, or empty.
				 */
				template<class F>
				requires std::invocable<F, const T&> && Type::SafeValue<std::remove_cvref_t<std::invoke_result_t<F, const T&>>>
				auto transform(F&& function) const & {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
					static_assert(Type::SafeValue<Result>, "Safe::Optional::transform requires a SafeValue result");
					if (!has_value())
						return Optional<Result>{};
					const T snapshot = value();
					return Optional<Result>(std::invoke(std::forward<F>(function), snapshot));
				}

				/**
				 * @brief Apply a callable to a moved value copy and wrap its Safe result.
				 * @tparam F Callable accepting T&& and returning a SafeValue.
				 * @param function Callable to invoke.
				 * @return Optional containing the result, or empty.
				 */
				template<class F>
				requires std::invocable<F, T&&> && Type::SafeValue<std::remove_cvref_t<std::invoke_result_t<F, T&&>>>
				auto transform(F&& function) && {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
					if (!has_value())
						return Optional<Result>{};
					T snapshot = value();
					return Optional<Result>(std::invoke(std::forward<F>(function), std::move(snapshot)));
				}

				/**
				 * @brief Apply a callable to a const value copy from a const rvalue.
				 * @tparam F Callable accepting const T&& and returning a SafeValue.
				 * @param function Callable to invoke.
				 * @return Optional containing the result, or empty.
				 */
				template<class F>
				requires std::invocable<F, const T&&> && Type::SafeValue<std::remove_cvref_t<std::invoke_result_t<F, const T&&>>>
				auto transform(F&& function) const && {
					using Result = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
					if (!has_value())
						return Optional<Result>{};
					const T snapshot = value();
					return Optional<Result>(std::invoke(std::forward<F>(function), std::move(snapshot)));
				}

				/**
				 * @brief Return this const optional when engaged, otherwise invoke a fallback callable.
				 * @tparam F Callable returning a Safe optional.
				 * @param function Fallback callable.
				 * @return This optional's copy or the callable's result.
				 */
				template<class F>
				requires std::invocable<F> && std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, Optional>
				Optional or_else(F&& function) const & {
					if (has_value())
						return *this;
					return std::invoke(std::forward<F>(function));
				}

				/**
				 * @brief Return this rvalue optional or invoke a fallback callable.
				 * @tparam F Callable returning Optional.
				 * @param function Fallback callable.
				 * @return This optional moved by value or the fallback result.
				 */
				template<class F>
				requires std::invocable<F> && std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, Optional>
				Optional or_else(F&& function) && {
					if (has_value())
						return std::move(*this);
					return std::invoke(std::forward<F>(function));
				}

				/**
				 * @brief Return this const rvalue optional or invoke a fallback callable.
				 * @tparam F Callable returning Optional.
				 * @param function Fallback callable.
				 * @return A copy of this optional or the fallback result.
				 */
				template<class F>
				requires std::invocable<F> && std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, Optional>
				Optional or_else(F&& function) const && {
					if (has_value())
						return *this;
					return std::invoke(std::forward<F>(function));
				}

				/**
				 * @brief Construct or replace the contained value.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p T.
				 * @return Mutable callback-backed proxy to the stored value.
				 */
				template<class... Args>
				reference emplace(Args&&... args) {
					T value(std::forward<Args>(args)...);
					if (has_value())
						*begin() = value;
					else
						m_value.push_back(value);
					return reference(m_value.front());
				}

				/**
				 * @brief Construct or replace T from initializer-list elements.
				 * @tparam U Initializer-list element type.
				 * @tparam Args Trailing constructor argument types.
				 * @param values Initializer-list elements.
				 * @param args Trailing arguments forwarded to T.
				 * @return Mutable callback-backed proxy to the stored value.
				 */
				template<class U, class... Args>
				reference emplace(std::initializer_list<U> values, Args&&... args) {
					T value(values, std::forward<Args>(args)...);
					if (has_value())
						*begin() = value;
					else
						m_value.push_back(value);
					return reference(m_value.front());
				}

				/**
				 * @brief Remove the contained value.
				 */
				void reset() { m_value.clear(); }

				/**
				 * @brief Copy the value into caller-owned STL storage.
				 * @return A std::optional owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::optional<T>() const;

			private:
				Vector<T> m_value;	///< Opaque sequence holding at most one value.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes opaque Safe optional values.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		requires Type::IsSafe<T>::value
		struct IsSafe<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the contained value.
		 * @tparam T Safe value type.
		 */
		template<SafeValue T>
		requires Type::MaybeSafe<T>
		struct IsMaybeSafe<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Registers Safe optional values for optional-aware generic APIs.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeOptional<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Admits nested opaque optional values.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::Optional<T>>: std::true_type {};
	}
}

#include <StormByte/safe/optional.txx>
