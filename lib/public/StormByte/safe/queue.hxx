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
		 * Lvalue imports copy and rvalue imports move elements into creator-owned
		 * storage; they never adopt another module's container allocation. STL exports are force-inlined so
		 * the returned queue is allocated and destroyed in the caller. Front copies
		 * into caller-owned output and never lends an internal reference. Copy is
		 * deep; move leaves an empty readable object. Compatible C++ ABI, packing,
		 * calling convention and loaded creator/Base modules are required.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Queue final {
			public:
				using value_type = T; ///< Element type.
				using size_type = std::size_t; ///< Element count type.

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
				 * @brief Move elements from an STL rvalue into locally owned storage.
				 * @param values Source queue; it is left valid and empty.
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
				 * @brief Import an STL queue by copy.
				 * @param values Caller-owned source queue.
				 * @return This queue.
				 */
				Queue& operator=(const std::queue<T>& values) {
					Queue replacement(values);
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Import an STL queue by moving its elements.
				 * @param values Source queue, empty after successful transfer.
				 * @return This queue.
				 */
				Queue& operator=(std::queue<T>&& values) {
					Queue replacement(std::move(values));
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Return the number of queued elements.
				 * @return Element count, including zero after move.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Test whether the queue is empty.
				 * @return Whether no elements are queued.
				 */
				bool empty() const noexcept { return size() == 0; }

				/**
				 * @brief Return a copy of the front element.
				 * @return Front element.
				 * @throws StormByte::Exception The queue is empty or copying failed.
				 */
				T front() const {
					T output{};
					if (m_dispatch(m_owner.Get(), Action::Front, nullptr, &output) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe queue front failed");
					return output;
				}

				/**
				 * @brief Return a copy of the back element.
				 * @return Back element snapshot.
				 * @throws StormByte::Exception The queue is empty or copying failed.
				 */
				T back() const {
					T output{};
					if (m_dispatch(m_owner.Get(), Action::Back, nullptr, &output) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe queue back failed");
					return output;
				}

				/**
				 * @brief Append a value.
				 * @param value Value to copy into the queue.
				 * @throws StormByte::Exception Storage creation or copying failed.
				 */
				void push(const T& value) {
					EnsureOwner();
					if (m_dispatch(m_owner.Get(), Action::Push, &value, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe queue push failed");
				}

				/**
				 * @brief Append an rvalue. The Safe value is copied through the creator callback.
				 * @param value Value to append.
				 */
				void push(T&& value) { push(static_cast<const T&>(value)); }

				/**
				 * @brief Construct and append an element.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to T.
				 * @return Caller-owned snapshot of the inserted value; modifying it does not
				 *         modify the queued element.
				 */
				template<class... Args>
				T emplace(Args&&... args) {
					T value(std::forward<Args>(args)...);
					push(value);
					return value;
				}

				/**
				 * @brief Remove the front element.
				 * @throws StormByte::Exception The queue is empty or removal failed.
				 */
				void pop() {
					if (m_dispatch(m_owner.Get(), Action::Pop, nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe queue pop failed");
				}

				/**
				 * @brief Exchange queue storage and creator callbacks.
				 * @param other Queue to exchange with.
				 */
				void swap(Queue& other) noexcept {
					if (this == &other)
						return;
					Queue temporary(std::move(*this));
					*this = std::move(other);
					other = std::move(temporary);
				}

				/**
				 * @brief Exchange two queues.
				 * @param left First queue.
				 * @param right Second queue.
				 */
				friend void swap(Queue& left, Queue& right) noexcept { left.swap(right); }

				/**
				 * @brief Compare FIFO contents in order.
				 * @param left First queue.
				 * @param right Second queue.
				 * @return Whether the queues contain equal values in order.
				 */
				friend bool operator==(const Queue& left, const Queue& right)
					requires Type::EqualityComparable<T> {
					return static_cast<std::queue<T>>(left) == static_cast<std::queue<T>>(right);
				}

				/**
				 * @brief Order FIFO contents lexicographically.
				 * @param left First queue.
				 * @param right Second queue.
				 * @return Comparison category of the underlying standard queue.
				 */
				friend auto operator<=>(const Queue& left, const Queue& right)
					requires Type::ThreeWayComparable<T> {
					return static_cast<std::queue<T>>(left) <=> static_cast<std::queue<T>>(right);
				}

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
					Back, ///< Copy the last element.
					Push, ///< Append an element.
					Pop ///< Remove the first element.
				};

				/**
				 * @brief Creator-local operation callback.
				 */
				using Dispatch = Status (*)(void*, Action, const T*, T*) noexcept;

				/**
				 * @brief Creator-local count callback.
				 */
				using Count = StormByte::Size (*)(const void*) noexcept;
				using Create = void* (*)(); ///< Empty-store callback type in the creator module.
				using Clone = Detail::Owner::Clone; ///< Deep-clone callback type in the creator module.
				using Destroy = Detail::Owner::Destroy; ///< Release callback type in the creator module.

				/**
				 * @brief Recreate empty owner storage after move.
				 */
				void EnsureOwner() {
					if (!m_owner.Get())
						m_owner = Detail::Owner(m_create(), m_clone, m_destroy);
				}

				Create m_create; ///< Empty-store callback in the creator module.
				Clone m_clone; ///< Deep-clone callback in the creator module.
				Destroy m_destroy; ///< Release callback in the creator module.
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
		requires Type::IsSafe<T>::value
		struct IsSafe<Safe::Queue<T>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the FIFO value type.
		 * @tparam T Safe value type.
		 */
		template<SafeValue T>
		requires Type::MaybeSafe<T>
		struct IsMaybeSafe<Safe::Queue<T>>: std::true_type {};

		/**
		 * @brief Admits nested opaque FIFOs.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::Queue<T>>: std::true_type {};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Registers Safe FIFO wrappers for generic queue operations.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeQueue<Safe::Queue<T>>: std::true_type {};
	}
}

#include <StormByte/safe/queue.txx>
