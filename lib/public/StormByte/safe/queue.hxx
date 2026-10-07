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
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <queue>
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
		 * @brief Throw the queue out-of-bounds error from Base.
		 * @throws OutOfBoundsError Always.
		 */
		[[noreturn]] STORMBYTE_PUBLIC void ThrowQueueOutOfBounds();

		/**
		 * @class Queue
		 * @brief FIFO sequence stored on Base's heap.
		 * @tparam T Safe value.
		 *
		 * Elements live in a @ref Vector. A move from `std::queue` moves each element and pops the source in the caller, so the deque buffer is released by the caller's CRT. Iterators walk the queue in FIFO order.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Queue final {
			public:
				using value_type = T; ///< Element type.
				using size_type = std::size_t; ///< Element count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = T&; ///< Mutable element reference.
				using const_reference = const T&; ///< Read-only element reference.
				using iterator = typename Vector<T>::iterator; ///< Mutable iterator, front to back.
				using const_iterator = typename Vector<T>::const_iterator; ///< Read-only iterator, front to back.

				/**
				 * @brief Construct an empty queue.
				 */
				Queue() noexcept;

				/**
				 * @brief Copy a caller-owned STL queue. Its state is unchanged.
				 * @param values Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit Queue(const std::queue<T>& values): Queue() {
					std::queue<T> copy(values);
					while (!copy.empty()) {
						push(std::move(copy.front()));
						copy.pop();
					}
				}

				/**
				 * @brief Move elements from a caller-owned STL queue and leave it empty.
				 * @param values Source. Empty before this function returns.
				 * @throws AllocationError The block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit Queue(std::queue<T>&& values): Queue() {
					while (!values.empty()) {
						push(std::move(values.front()));
						values.pop();
					}
				}

				/**
				 * @brief Copy every element into a new Base block.
				 * @param other Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				Queue(const Queue& other);

				/**
				 * @brief Take the block. @p other is left empty.
				 * @param other Source.
				 */
				Queue(Queue&& other) noexcept;

				/**
				 * @brief Destroy every element and release the block.
				 */
				~Queue() noexcept;

				/**
				 * @brief Copy-assign.
				 * @param other Source.
				 * @return This queue.
				 * @throws AllocationError The block could not be allocated.
				 */
				Queue& operator=(const Queue& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This queue.
				 */
				Queue& operator=(Queue&& other) noexcept;

				/**
				 * @brief Copy-assign a caller-owned STL queue. Its state is unchanged.
				 * @param values Source.
				 * @return This queue.
				 * @throws AllocationError The block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE Queue& operator=(const std::queue<T>& values) {
					Queue replacement(values);
					swap(replacement);
					return *this;
				}

				/**
				 * @brief Move-assign a caller-owned STL queue and leave it empty.
				 * @param values Source. Empty before this function returns.
				 * @return This queue.
				 * @throws AllocationError The block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE Queue& operator=(std::queue<T>&& values) {
					Queue replacement(std::move(values));
					swap(replacement);
					return *this;
				}

				/**
				 * @brief Return a mutable iterator to the front.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return the mutable end iterator.
				 * @return End iterator.
				 */
				iterator end() noexcept;

				/**
				 * @brief Return a read-only iterator to the front.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return the read-only end iterator.
				 * @return End iterator.
				 */
				const_iterator end() const noexcept;

				/**
				 * @brief Return a read-only iterator to the front.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept;

				/**
				 * @brief Return the read-only end iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const noexcept;

				/**
				 * @brief Test whether the queue is empty.
				 * @return Whether no elements are queued.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the element count.
				 * @return Element count.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Return the front element.
				 * @return Front element. Valid until it is popped or the queue reallocates.
				 * @throws OutOfBoundsError The queue is empty.
				 */
				reference front();

				/**
				 * @brief Return the front element.
				 * @return Front element. Valid until it is popped or the queue reallocates.
				 * @throws OutOfBoundsError The queue is empty.
				 */
				const_reference front() const;

				/**
				 * @brief Return the back element.
				 * @return Back element. Valid until it is erased or the queue reallocates.
				 * @throws OutOfBoundsError The queue is empty.
				 */
				reference back();

				/**
				 * @brief Return the back element.
				 * @return Back element. Valid until it is erased or the queue reallocates.
				 * @throws OutOfBoundsError The queue is empty.
				 */
				const_reference back() const;

				/**
				 * @brief Append a copy.
				 * @param value Value to copy.
				 * @throws AllocationError The block could not be allocated.
				 */
				void push(const T& value);

				/**
				 * @brief Append a moved value.
				 * @param value Value to move.
				 * @throws AllocationError The block could not be allocated.
				 */
				void push(T&& value);

				/**
				 * @brief Construct an element at the back.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to T.
				 * @return Reference to the new element.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class... Args>
				reference emplace(Args&&... args);

				/**
				 * @brief Remove the front element.
				 * @throws OutOfBoundsError The queue is empty.
				 */
				void pop();

				/**
				 * @brief Erase one element.
				 * @param position Element to erase.
				 * @return Iterator following the erased element.
				 */
				iterator erase(const_iterator position);

				/**
				 * @brief Erase a half-open range.
				 * @param first First element to erase.
				 * @param last Past-the-end element.
				 * @return Iterator following the erased range.
				 */
				iterator erase(const_iterator first, const_iterator last);

				/**
				 * @brief Exchange blocks. Does not allocate.
				 * @param other Queue to exchange with.
				 */
				void swap(Queue& other) noexcept;

				/**
				 * @brief Compare elements in FIFO order.
				 * @param other Queue to compare.
				 * @return Whether both queues contain the same elements.
				 */
				bool operator==(const Queue& other) const requires Type::EqualityComparable<T>;

				/**
				 * @brief Order queues lexicographically.
				 * @param other Queue to compare.
				 * @return Whether this queue precedes @p other.
				 */
				bool operator<(const Queue& other) const requires requires(const T& left, const T& right) { left < right; };

				/**
				 * @brief Order queues lexicographically.
				 * @param other Queue to compare.
				 * @return Whether this queue precedes or equals @p other.
				 */
				bool operator<=(const Queue& other) const requires requires(const T& left, const T& right) { left < right; };

				/**
				 * @brief Order queues lexicographically.
				 * @param other Queue to compare.
				 * @return Whether this queue follows @p other.
				 */
				bool operator>(const Queue& other) const requires requires(const T& left, const T& right) { left < right; };

				/**
				 * @brief Order queues lexicographically.
				 * @param other Queue to compare.
				 * @return Whether this queue follows or equals @p other.
				 */
				bool operator>=(const Queue& other) const requires requires(const T& left, const T& right) { left < right; };

				/**
				 * @brief Copy the elements into caller-owned STL storage.
				 * @return A `std::queue` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::queue<T>() const {
					std::queue<T> exported;
					for (const T& value : *this)
						exported.push(value);
					return exported;
				}

			private:
				Vector<T> m_values; ///< FIFO storage. Front is the first element.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes a queue of already safe values.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		requires IsSafe<T>::value
		struct IsSafe<Safe::Queue<T>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the element type.
		 * @tparam T Conditionally safe value.
		 */
		template<SafeValue T>
		requires MaybeSafe<T>
		struct IsMaybeSafe<Safe::Queue<T>>: std::true_type {};

		/**
		 * @brief Admits a Safe queue as a collection value.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::Queue<T>>: std::true_type {};

		/**
		 * @brief Registers a Safe queue for generic queue operations.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeQueue<Safe::Queue<T>>: std::true_type {};
	}
}

#include <StormByte/safe/queue.txx>
