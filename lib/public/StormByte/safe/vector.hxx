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

#include <StormByte/safe/heap.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

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
		 * @brief Throw the vector out-of-bounds error from Base.
		 * @throws OutOfBoundsError Always.
		 */
		[[noreturn]] STORMBYTE_PUBLIC void ThrowVectorOutOfBounds();

		/**
		 * @class Vector
		 * @brief Contiguous sequence stored on Base's heap.
		 * @tparam T Safe value.
		 *
		 * The block is allocated with @ref Heap::Allocate. Iterators are pointers into that block. A move from `std::vector` moves the elements and clears the source in the caller, so its buffer is released by the caller's CRT.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Vector final {
			public:
				using value_type = T; ///< Element type.
				using size_type = std::size_t; ///< Element count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = T&; ///< Mutable element reference.
				using const_reference = const T&; ///< Read-only element reference.
				using pointer = T*; ///< Mutable element pointer.
				using const_pointer = const T*; ///< Read-only element pointer.
				using iterator = T*; ///< Mutable random-access iterator.
				using const_iterator = const T*; ///< Read-only random-access iterator.

				/**
				 * @brief Construct an empty sequence.
				 */
				Vector() noexcept;

				/**
				 * @brief Construct @p count value-initialized elements.
				 * @param count Element count.
				 * @throws AllocationError The block could not be allocated.
				 */
				explicit Vector(size_type count);

				/**
				 * @brief Construct @p count copies of @p value.
				 * @param count Element count.
				 * @param value Value to copy.
				 * @throws AllocationError The block could not be allocated.
				 */
				Vector(size_type count, const T& value);

				/**
				 * @brief Construct from an initializer list.
				 * @param values Initial elements.
				 * @throws AllocationError The block could not be allocated.
				 */
				Vector(std::initializer_list<T> values);

				/**
				 * @brief Copy a caller-owned STL vector. Its state is unchanged.
				 * @param values Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				explicit Vector(const std::vector<T>& values);

				/**
				 * @brief Move elements from a caller-owned STL vector and leave it empty.
				 * @param values Source. Cleared before this function returns.
				 * @throws AllocationError The block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit Vector(std::vector<T>&& values): Vector() {
					reserve(values.size());
					for (T& value : values)
						push_back(std::move(value));
					values.clear();
				}

				/**
				 * @brief Copy every element into a new Base block.
				 * @param other Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				Vector(const Vector& other);

				/**
				 * @brief Take the block. @p other is left empty.
				 * @param other Source.
				 */
				Vector(Vector&& other) noexcept;

				/**
				 * @brief Destroy every element and release the block.
				 */
				~Vector() noexcept;

				/**
				 * @brief Copy-assign. The previous block is released only after the copy exists.
				 * @param other Source.
				 * @return This sequence.
				 * @throws AllocationError The block could not be allocated.
				 */
				Vector& operator=(const Vector& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This sequence.
				 */
				Vector& operator=(Vector&& other) noexcept;

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values New elements.
				 * @return This sequence.
				 * @throws AllocationError The block could not be allocated.
				 */
				Vector& operator=(std::initializer_list<T> values);

				/**
				 * @brief Copy-assign a caller-owned STL vector. Its state is unchanged.
				 * @param values Source.
				 * @return This sequence.
				 * @throws AllocationError The block could not be allocated.
				 */
				Vector& operator=(const std::vector<T>& values);

				/**
				 * @brief Move-assign elements from a caller-owned STL vector and leave it empty.
				 * @param values Source. Cleared before this function returns.
				 * @return This sequence.
				 * @throws AllocationError The block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE Vector& operator=(std::vector<T>&& values) {
					Vector replacement(std::move(values));
					swap(replacement);
					return *this;
				}

				/**
				 * @brief Return a mutable iterator to the first element.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return the mutable end iterator.
				 * @return End iterator.
				 */
				iterator end() noexcept;

				/**
				 * @brief Return a read-only iterator to the first element.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return the read-only end iterator.
				 * @return End iterator.
				 */
				const_iterator end() const noexcept;

				/**
				 * @brief Return a read-only iterator to the first element.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept;

				/**
				 * @brief Return the read-only end iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const noexcept;

				/**
				 * @brief Test whether the sequence is empty.
				 * @return Whether there are no elements.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the element count.
				 * @return Element count.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Return the allocated capacity.
				 * @return Capacity in elements.
				 */
				size_type capacity() const noexcept;

				/**
				 * @brief Return a pointer to the block.
				 * @return Block address, or null when no capacity is reserved.
				 */
				pointer data() noexcept;

				/**
				 * @brief Return a pointer to the block.
				 * @return Block address, or null when no capacity is reserved.
				 */
				const_pointer data() const noexcept;

				/**
				 * @brief Reserve room for at least @p count elements. A smaller request does not shrink.
				 * @param count Minimum capacity.
				 * @throws AllocationError The block could not be allocated.
				 */
				void reserve(size_type count);

				/**
				 * @brief Release unused capacity.
				 */
				void shrink_to_fit();

				/**
				 * @brief Destroy every element. Capacity is kept.
				 */
				void clear() noexcept;

				/**
				 * @brief Exchange blocks. Does not allocate.
				 * @param other Sequence to exchange with.
				 */
				void swap(Vector& other) noexcept;

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until reallocation or erasure.
				 */
				reference operator[](size_type index) noexcept;

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until reallocation or erasure.
				 */
				const_reference operator[](size_type index) const noexcept;

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until reallocation or erasure.
				 * @throws OutOfBoundsError @p index is outside the sequence.
				 */
				reference at(size_type index);

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until reallocation or erasure.
				 * @throws OutOfBoundsError @p index is outside the sequence.
				 */
				const_reference at(size_type index) const;

				/**
				 * @brief Return the first element.
				 * @return First element. Valid until reallocation or erasure.
				 * @throws OutOfBoundsError The sequence is empty.
				 */
				reference front();

				/**
				 * @brief Return the first element.
				 * @return First element. Valid until reallocation or erasure.
				 * @throws OutOfBoundsError The sequence is empty.
				 */
				const_reference front() const;

				/**
				 * @brief Return the last element.
				 * @return Last element. Valid until reallocation or erasure.
				 * @throws OutOfBoundsError The sequence is empty.
				 */
				reference back();

				/**
				 * @brief Return the last element.
				 * @return Last element. Valid until reallocation or erasure.
				 * @throws OutOfBoundsError The sequence is empty.
				 */
				const_reference back() const;

				/**
				 * @brief Append a copy.
				 * @param value Value to copy.
				 * @throws AllocationError The block could not be allocated.
				 */
				void push_back(const T& value);

				/**
				 * @brief Append a moved value.
				 * @param value Value to move.
				 * @throws AllocationError The block could not be allocated.
				 */
				void push_back(T&& value);

				/**
				 * @brief Remove the last element.
				 * @throws OutOfBoundsError The sequence is empty.
				 */
				void pop_back();

				/**
				 * @brief Construct an element at the end.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to T.
				 * @return Reference to the new element. Valid until reallocation or erasure.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class... Args>
				reference emplace_back(Args&&... args);

				/**
				 * @brief Insert a copy before @p position.
				 * @param position Insertion point.
				 * @param value Value to copy.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError The block could not be allocated.
				 */
				iterator insert(const_iterator position, const T& value);

				/**
				 * @brief Insert a moved value before @p position.
				 * @param position Insertion point.
				 * @param value Value to move.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError The block could not be allocated.
				 */
				iterator insert(const_iterator position, T&& value);

				/**
				 * @brief Insert @p count copies before @p position.
				 * @param position Insertion point.
				 * @param count Copy count.
				 * @param value Value to copy.
				 * @return Iterator to the first inserted element, or @p position when @p count is zero.
				 * @throws AllocationError The block could not be allocated.
				 */
				iterator insert(const_iterator position, size_type count, const T& value);

				/**
				 * @brief Construct an element before @p position.
				 * @tparam Args Constructor argument types.
				 * @param position Insertion point.
				 * @param args Arguments forwarded to T.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class... Args>
				iterator emplace(const_iterator position, Args&&... args);

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
				 * @brief Replace the contents with @p count copies.
				 * @param count Element count.
				 * @param value Value to copy.
				 * @throws AllocationError The block could not be allocated.
				 */
				void assign(size_type count, const T& value);

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values New elements.
				 * @throws AllocationError The block could not be allocated.
				 */
				void assign(std::initializer_list<T> values);

				/**
				 * @brief Change the size, value-initializing new elements.
				 * @param count New size.
				 * @throws AllocationError The block could not be allocated.
				 */
				void resize(size_type count);

				/**
				 * @brief Change the size, copying @p value into new elements.
				 * @param count New size.
				 * @param value Value to copy.
				 * @throws AllocationError The block could not be allocated.
				 */
				void resize(size_type count, const T& value);

				/**
				 * @brief Compare elements in order.
				 * @param other Sequence to compare.
				 * @return Whether both sequences contain the same elements.
				 */
				bool operator==(const Vector& other) const requires Type::EqualityComparable<T>;

				/**
				 * @brief Copy the elements into caller-owned STL storage.
				 * @return A `std::vector` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::vector<T>() const {
					return std::vector<T>(begin(), end());
				}

			private:
				/**
				 * @brief Allocate a block able to hold @p capacity elements.
				 * @param capacity Element capacity.
				 * @return Block address, or null when @p capacity is zero.
				 * @throws AllocationError The block could not be allocated.
				 */
				static T* Allocate(size_type capacity);

				/**
				 * @brief Destroy the constructed elements and release the block.
				 * @param data Block address.
				 * @param count Constructed element count.
				 */
				static void Release(T* data, size_type count) noexcept;

				/**
				 * @brief Grow so that one more element fits.
				 * @throws AllocationError The block could not be allocated.
				 */
				void Grow();

				T* m_data; ///< Base-owned block, or null when no capacity is reserved.
				size_type m_size; ///< Constructed element count.
				size_type m_capacity; ///< Allocated element capacity.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes a vector of already safe values.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		requires IsSafe<T>::value
		struct IsSafe<Safe::Vector<T>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the element type.
		 * @tparam T Conditionally safe value.
		 */
		template<SafeValue T>
		requires MaybeSafe<T>
		struct IsMaybeSafe<Safe::Vector<T>>: std::true_type {};

		/**
		 * @brief Admits a Safe vector as a collection value.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::Vector<T>>: std::true_type {};
	}
}

#include <StormByte/safe/vector.txx>
