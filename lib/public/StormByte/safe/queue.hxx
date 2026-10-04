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

#include <queue>

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
		 * @class Queue
		 * @brief Opaque FIFO whose storage and operations remain in its creator module.
		 * @tparam T Unqualified Type::SafeValue.
		 *
		 * STL imports copy elements into creator-owned storage; they never adopt
		 * another module's container allocation. STL exports are force-inlined so
		 * the returned queue is allocated and destroyed in the caller. Front copies
		 * into caller-owned output and never lends an internal reference. Copy is
		 * deep; move leaves an empty readable object. Compatible C++ ABI, packing,
		 * calling convention and loaded creator/Base modules are required.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Queue final {
			public:
				/**
				 * @brief Construct an empty FIFO in the calling module.
				 */
				STORMBYTE_FORCE_INLINE Queue();

				/**
				 * @brief Copy elements from a caller-owned STL queue.
				 * @param values Source queue; its allocation remains with its owner.
				 */
				explicit Queue(const std::queue<T>& values);

				/**
				 * @brief Copy elements from an STL rvalue without adopting its allocation.
				 * @param values Source queue; it remains valid and unchanged.
				 */
				explicit Queue(std::queue<T>&& values);

				/**
				 * @brief Deep copy in the original owner module.
				 * @param other Source.
				 */
				Queue(const Queue& other) = default;

				/**
				 * @brief Transfer ownership; source becomes empty.
				 * @param other Source.
				 */
				Queue(Queue&& other) noexcept = default;

				/**
				 * @brief Release storage through the creator-module callback.
				 */
				~Queue() noexcept = default;

				/**
				 * @brief Deep copy with strong guarantee.
				 * @param other Source.
				 * @return This FIFO.
				 */
				Queue& operator=(const Queue& other) = default;

				/**
				 * @brief Release old state and transfer.
				 * @param other Source.
				 * @return This FIFO.
				 */
				Queue& operator=(Queue&& other) noexcept = default;

				/**
				 * @brief Number of queued elements.
				 * @return Element count, including zero after move.
				 */
				StormByte::Size Size() const noexcept;

				/**
				 * @brief Test whether the FIFO has no elements.
				 * @return True when empty.
				 */
				bool Empty() const noexcept;

				/**
				 * @brief Copy the front element to caller-owned output.
				 * @param output Destination; unchanged when empty or copying fails.
				 * @return Missing when empty; otherwise Success or Failure.
				 */
				Status Front(T& output) const noexcept;

				/**
				 * @brief Append a copy at the back.
				 * @param value Source value.
				 * @return Success or Failure; failure leaves the queue unchanged.
				 */
				Status Push(const T& value) noexcept;

				/**
				 * @brief Remove the front element.
				 * @return Missing when empty; otherwise Success.
				 */
				Status Pop() noexcept;

				/**
				 * @brief Remove all elements in the owner module.
				 * @return Operation status.
				 */
				Status Clear() noexcept;

				/**
				 * @brief Copy elements into caller-owned STL storage.
				 * @return A std::queue allocated and destroyed in the caller module.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::queue<T>() const;

			private:
				/**
				 * @brief Creator-local implementation.
				 */
				struct STORMBYTE_PRIVATE Store;

				/**
				 * @brief Internal operation identifier.
				 */
				enum class Action {
					Front, ///< Copy the first element.
					Push, ///< Append an element.
					Pop, ///< Remove the first element.
					Clear ///< Remove all elements.
				};

				/**
				 * @brief Creator-local operation callback.
				 */
				using Dispatch = Status (*)(void*, Action, const T*, T*) noexcept;

				/**
				 * @brief Creator-local count callback.
				 */
				using Count = StormByte::Size (*)(const void*) noexcept;

				Detail::Owner m_owner; ///< State with creator-module lifetime callbacks.
				Dispatch m_dispatch; ///< Operations in the creator module.
				Count m_count; ///< Count in the creator module.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes opaque Safe FIFOs.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafe<Safe::Queue<T>>: std::true_type {};

		/**
		 * @brief Admits nested opaque FIFOs.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::Queue<T>>: std::true_type {};
	}
}

#include <StormByte/safe/queue.txx>