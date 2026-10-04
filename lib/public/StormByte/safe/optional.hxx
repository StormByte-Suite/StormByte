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

#include <optional>
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
		 * @class Optional
		 * @brief Zero or one Safe-owned value, reusing opaque sequence ownership.
		 * @tparam T Safe value.
		 * @note No std::optional layout. Vector's ABI, ownership and failure rules apply.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Optional final {
			public:
				using value_type = T; ///< Contained type.
				using iterator = typename Vector<T>::iterator; ///< Mutable 0/1-element iterator.
				using const_iterator = typename Vector<T>::const_iterator; ///< Read-only 0/1-element iterator.

				/**
				 * @brief Construct an empty optional in the calling module.
				 */
				STORMBYTE_FORCE_INLINE Optional();

				/**
				 * @brief Copy a caller-owned STL optional.
				 * @param value Source value; its ownership remains with its owner.
				 */
				explicit Optional(const std::optional<T>& value);

				/**
				 * @brief Move an STL rvalue's value into local Safe storage.
				 * @param value Source optional; it is reset after transfer.
				 */
				explicit Optional(std::optional<T>&& value);

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
				 * @brief Construct or replace the contained value.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p T.
				 * @return Iterator proxy to the stored value.
				 */
				template<class... Args>
				iterator::reference emplace(Args&&... args) {
					T value(std::forward<Args>(args)...);
					if (has_value())
						*begin() = value;
					else
						m_value.push_back(value);
					return *begin();
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
		struct IsSafe<Safe::Optional<T>>: std::true_type {};

		/**
		 * @brief Admits nested opaque optional values.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::Optional<T>>: std::true_type {};
	}
}

#include <StormByte/safe/optional.txx>