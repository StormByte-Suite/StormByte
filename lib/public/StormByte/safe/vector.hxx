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

#include <StormByte/safe/owner.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits/safe.hxx>

#include <vector>

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
		 * @class Vector
		 * @brief Opaque sequence whose storage and operations remain in its creator module.
		 * @tparam T Unqualified Type::SafeValue. No STL owners or arbitrary user types.
		 *
		 * Copy is deep; move leaves an empty readable object. Assign a fresh Vector
		 * before modifying a moved-from object. Get copies into caller-owned output;
		 * no references, iterators or STL containers escape. Modifications are transactional.
		 * Failure leaves the sequence unchanged. Missing leaves output unchanged.
		 * Concurrent modification requires external synchronization. All creator modules
		 * and Base must remain loaded. Compatible C++ ABI, packing and calling convention
		 * are required; this is not a facade for incompatible toolchains.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Vector final {
			public:
				/**
				 * @brief Construct an empty sequence in the calling module.
				 */
				STORMBYTE_FORCE_INLINE Vector();

				/**
				 * @brief Copy elements from a caller-owned STL vector.
				 * @param values Source vector; its allocation remains with its owner.
				 */
				explicit Vector(const std::vector<T>& values);

				/**
				 * @brief Copy elements from an STL rvalue without adopting its allocation.
				 * @param values Source vector; it remains valid and unchanged.
				 */
				explicit Vector(std::vector<T>&& values);
				/**
				 * @brief Deep copy in the original owner module.
				 * @param other Source.
				 */
				Vector(const Vector& other) = default;
				/**
				 * @brief Transfer ownership; source becomes empty.
				 * @param other Source.
				 */
				Vector(Vector&& other) noexcept = default;
				/**
				 * @brief Release storage through the creator-module callback.
				 */
				~Vector() noexcept = default;
				/**
				 * @brief Deep copy with strong guarantee.
				 * @param other Source.
				 * @return This sequence.
				 */
				Vector& operator=(const Vector& other) = default;
				/**
				 * @brief Release old state and transfer.
				 * @param other Source.
				 * @return This sequence.
				 */
				Vector& operator=(Vector&& other) noexcept = default;
				/**
				 * @brief Number of elements, including zero after move.
				 * @return Count.
				 */
				StormByte::Size Size() const noexcept;
				/**
				 * @brief Copy element into output.
				 * @param index Position.
				 * @param output Destination.
				 * @return Operation status.
				 */
				Status Get(const StormByte::Size& index, T& output) const noexcept;
				/**
				 * @brief Append a copy.
				 * @param value Source.
				 * @return Operation status.
				 */
				Status PushBack(const T& value) noexcept;
				/**
				 * @brief Replace an element with a copy.
				 * @param index Position.
				 * @param value Source.
				 * @return Operation status.
				 */
				Status Set(const StormByte::Size& index, const T& value) noexcept;
				/**
				 * @brief Remove an element; later indices shift left.
				 * @param index Position.
				 * @return Operation status.
				 */
				Status Erase(const StormByte::Size& index) noexcept;
				/**
				 * @brief Remove all elements in the owner module.
				 * @return Operation status.
				 */
				Status Clear() noexcept;

				/**
				 * @brief Copy elements into caller-owned STL storage.
				 * @return A std::vector allocated and destroyed in the caller module.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::vector<T>() const;
			private:
				/**
				 * @brief Creator-local implementation; never exported by value.
				 */
				struct STORMBYTE_PRIVATE Store;
				/**
				 * @brief Internal operation identifier.
				 */
				enum class Action {
					Get, ///< Copy an element to the caller.
					PushBack, ///< Append an element.
					Set, ///< Replace an element.
					Erase, ///< Remove an element.
					Clear ///< Remove all elements.
				};
				/**
				 * @brief Creator-local operation callback.
				 */
				using Dispatch = Status (*)(void*, Action, const StormByte::Size&, const T*, T*) noexcept;
				/**
				 * @brief Creator-local count callback.
				 */
				using Count = StormByte::Size (*)(const void*) noexcept;
				Detail::Owner m_owner;	///< Opaque owner with creator callbacks.
				Dispatch m_dispatch;		///< Operations in the creator module.
				Count m_count;			///< Count in the creator module.
		};
	}
	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes opaque Safe sequences.
		 * @tparam T Safe value.
		 */
		template<SafeValue T> struct IsSafe<Safe::Vector<T>>: std::true_type {};
		/**
		 * @brief Admits nested Safe sequences.
		 * @tparam T Safe value.
		 */
		template<SafeValue T> struct IsSafeValue<Safe::Vector<T>>: std::true_type {};
	}
}

#include <StormByte/safe/vector.txx>