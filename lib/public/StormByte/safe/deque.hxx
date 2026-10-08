/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This is the base for StormByte Suite libraries.
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

#include <compare>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <limits>
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
		 * @brief Throw the deque out-of-bounds error from Base.
		 * @throws OutOfBoundsError Always.
		 */
		[[noreturn]] STORMBYTE_PUBLIC void ThrowDequeOutOfBounds();

		/**
		 * @class Deque
		 * @brief Double-ended sequence stored on Base's heap.
		 * @tparam T Safe value.
		 *
		 * Not an alias of `std::deque`. There is no allocator parameter. A move from `std::deque` copies the elements onto Base's heap and clears the source. It does not steal its buffer. Conversion back is explicit and born in the caller.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE Deque final {
			public:
				using value_type = T; ///< Element type.
				using size_type = std::size_t; ///< Element count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = T&; ///< Mutable element reference.
				using const_reference = const T&; ///< Read-only element reference.
				using pointer = T*; ///< Mutable element pointer.
				using const_pointer = const T*; ///< Read-only element pointer.
				class iterator;
				class const_iterator;
				using reverse_iterator = std::reverse_iterator<iterator>; ///< Mutable reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.

				/**
				 * @brief Construct an empty deque.
				 */
				Deque() noexcept;

				/**
				 * @brief Construct @p count copies of @p value.
				 * @param count Element count.
				 * @param value Value to copy.
				 * @throws AllocationError A block could not be allocated.
				 */
				Deque(size_type count, const T& value);

				/**
				 * @brief Construct @p count value-initialized elements.
				 * @param count Element count.
				 * @throws AllocationError A block could not be allocated.
				 */
				explicit Deque(size_type count);

				/**
				 * @brief Copy the range `[first, last)`.
				 * @tparam InputIt Input iterator whose value is convertible to T.
				 * @param first Start of the source range.
				 * @param last End of the source range.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::input_iterator InputIt>
				Deque(InputIt first, InputIt last);

				/**
				 * @brief Construct from an initializer list.
				 * @param values Initial elements.
				 * @throws AllocationError A block could not be allocated.
				 */
				Deque(std::initializer_list<T> values);

				/**
				 * @brief Copy a caller-owned STL deque. Its state is unchanged.
				 * @param values Source.
				 * @throws AllocationError A block could not be allocated.
				 */
				explicit Deque(const std::deque<T>& values);

				/**
				 * @brief Move elements from a caller-owned STL deque and leave it empty.
				 * @param values Source. Cleared before this function returns.
				 * @throws AllocationError A block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit Deque(std::deque<T>&& values): Deque() {
					for (T& value : values)
						push_back(std::move(value));
					values.clear();
				}

				/**
				 * @brief Copy every element into new Base blocks.
				 * @param other Source.
				 * @throws AllocationError A block could not be allocated.
				 */
				Deque(const Deque& other);

				/**
				 * @brief Take the blocks. @p other is left empty.
				 * @param other Source.
				 */
				Deque(Deque&& other) noexcept;

				/**
				 * @brief Destroy every element and release the blocks.
				 */
				~Deque() noexcept;

				/**
				 * @brief Copy-assign. The previous blocks are released only after the copy exists.
				 * @param other Source.
				 * @return This deque.
				 * @throws AllocationError A block could not be allocated.
				 */
				Deque& operator=(const Deque& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This deque.
				 */
				Deque& operator=(Deque&& other) noexcept;

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values New elements.
				 * @return This deque.
				 * @throws AllocationError A block could not be allocated.
				 */
				Deque& operator=(std::initializer_list<T> values);

				/**
				 * @brief Copy-assign a caller-owned STL deque. Its state is unchanged.
				 * @param values Source.
				 * @return This deque.
				 * @throws AllocationError A block could not be allocated.
				 */
				Deque& operator=(const std::deque<T>& values);

				/**
				 * @brief Move-assign elements from a caller-owned STL deque and leave it empty.
				 * @param values Source. Cleared before this function returns.
				 * @return This deque.
				 * @throws AllocationError A block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE Deque& operator=(std::deque<T>&& values) {
					Deque replacement(std::move(values));
					swap(replacement);
					return *this;
				}

				/**
				 * @brief Replace the contents with @p count copies of @p value.
				 * @param count Element count.
				 * @param value Value to copy.
				 * @throws AllocationError A block could not be allocated.
				 */
				void assign(size_type count, const T& value);

				/**
				 * @brief Replace the contents with the range `[first, last)`.
				 * @tparam InputIt Input iterator whose value is convertible to T.
				 * @param first Start of the source range.
				 * @param last End of the source range.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::input_iterator InputIt>
				void assign(InputIt first, InputIt last);

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values New elements.
				 * @throws AllocationError A block could not be allocated.
				 */
				void assign(std::initializer_list<T> values);

				/**
				 * @brief Replace the contents with a range.
				 * @tparam R Input range whose value is convertible to T.
				 * @param range Source range.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::ranges::input_range R>
				void assign_range(R&& range);

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until erasure of that element.
				 * @throws OutOfBoundsError @p index is outside the deque.
				 */
				reference at(size_type index);

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until erasure of that element.
				 * @throws OutOfBoundsError @p index is outside the deque.
				 */
				const_reference at(size_type index) const;

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until erasure of that element.
				 */
				reference operator[](size_type index) noexcept;

				/**
				 * @brief Access an element.
				 * @param index Element index.
				 * @return Element reference. Valid until erasure of that element.
				 */
				const_reference operator[](size_type index) const noexcept;

				/**
				 * @brief Return the first element.
				 * @return First element. Valid until erasure of that element.
				 * @throws OutOfBoundsError The deque is empty.
				 */
				reference front();

				/**
				 * @brief Return the first element.
				 * @return First element. Valid until erasure of that element.
				 * @throws OutOfBoundsError The deque is empty.
				 */
				const_reference front() const;

				/**
				 * @brief Return the last element.
				 * @return Last element. Valid until erasure of that element.
				 * @throws OutOfBoundsError The deque is empty.
				 */
				reference back();

				/**
				 * @brief Return the last element.
				 * @return Last element. Valid until erasure of that element.
				 * @throws OutOfBoundsError The deque is empty.
				 */
				const_reference back() const;

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
				 * @brief Return a mutable reverse iterator to the last element.
				 * @return Reverse iterator.
				 */
				STORMBYTE_FORCE_INLINE reverse_iterator rbegin() noexcept {
					return reverse_iterator(end());
				}

				/**
				 * @brief Return the mutable reverse end iterator.
				 * @return Reverse end iterator.
				 */
				STORMBYTE_FORCE_INLINE reverse_iterator rend() noexcept {
					return reverse_iterator(begin());
				}

				/**
				 * @brief Return a read-only reverse iterator to the last element.
				 * @return Reverse iterator.
				 */
				STORMBYTE_FORCE_INLINE const_reverse_iterator rbegin() const noexcept {
					return const_reverse_iterator(end());
				}

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				STORMBYTE_FORCE_INLINE const_reverse_iterator rend() const noexcept {
					return const_reverse_iterator(begin());
				}

				/**
				 * @brief Return a read-only reverse iterator to the last element.
				 * @return Reverse iterator.
				 */
				STORMBYTE_FORCE_INLINE const_reverse_iterator crbegin() const noexcept {
					return rbegin();
				}

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				STORMBYTE_FORCE_INLINE const_reverse_iterator crend() const noexcept {
					return rend();
				}

				/**
				 * @brief Test whether the deque is empty.
				 * @return Whether there are no elements.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the element count.
				 * @return Element count.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Return the largest representable element count.
				 * @return Maximum element count for this element size.
				 */
				STORMBYTE_FORCE_INLINE size_type max_size() const noexcept {
					return std::numeric_limits<size_type>::max() / sizeof(T);
				}

				/**
				 * @brief Release unused blocks.
				 */
				void shrink_to_fit();

				/**
				 * @brief Destroy every element. Capacity is kept.
				 */
				void clear() noexcept;

				/**
				 * @brief Insert a copy before @p position.
				 * @param position Insert position.
				 * @param value Value to copy.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError A block could not be allocated.
				 */
				iterator insert(const_iterator position, const T& value);

				/**
				 * @brief Insert a moved value before @p position.
				 * @param position Insert position.
				 * @param value Value to move.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError A block could not be allocated.
				 */
				iterator insert(const_iterator position, T&& value);

				/**
				 * @brief Insert @p count copies before @p position.
				 * @param position Insert position.
				 * @param count Copy count.
				 * @param value Value to copy.
				 * @return Iterator to the first inserted element, or @p position when @p count is zero.
				 * @throws AllocationError A block could not be allocated.
				 */
				iterator insert(const_iterator position, size_type count, const T& value);

				/**
				 * @brief Insert the range `[first, last)` before @p position.
				 * @tparam InputIt Input iterator whose value is convertible to T.
				 * @param position Insert position.
				 * @param first Start of the source range.
				 * @param last End of the source range.
				 * @return Iterator to the first inserted element.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::input_iterator InputIt>
				iterator insert(const_iterator position, InputIt first, InputIt last);

				/**
				 * @brief Insert an initializer list before @p position.
				 * @param position Insert position.
				 * @param values Source elements.
				 * @return Iterator to the first inserted element.
				 * @throws AllocationError A block could not be allocated.
				 */
				iterator insert(const_iterator position, std::initializer_list<T> values);

				/**
				 * @brief Insert a range before @p position.
				 * @tparam R Input range whose value is convertible to T.
				 * @param position Insert position.
				 * @param range Source range.
				 * @return Iterator to the first inserted element.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::ranges::input_range R>
				iterator insert_range(const_iterator position, R&& range);

				/**
				 * @brief Construct an element before @p position.
				 * @tparam Args Constructor argument types.
				 * @param position Insert position.
				 * @param args Constructor arguments.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<typename... Args>
				iterator emplace(const_iterator position, Args&&... args);

				/**
				 * @brief Destroy the element at @p position.
				 * @param position Element to destroy.
				 * @return Iterator following the destroyed element.
				 */
				iterator erase(const_iterator position);

				/**
				 * @brief Destroy the half-open range `[first, last)`.
				 * @param first First element to destroy.
				 * @param last One past the last element to destroy.
				 * @return Iterator following the destroyed range.
				 */
				iterator erase(const_iterator first, const_iterator last);

				/**
				 * @brief Append a copy.
				 * @param value Value to copy.
				 * @throws AllocationError A block could not be allocated.
				 */
				void push_back(const T& value);

				/**
				 * @brief Append a moved value.
				 * @param value Value to move.
				 * @throws AllocationError A block could not be allocated.
				 */
				void push_back(T&& value);

				/**
				 * @brief Construct an element at the back.
				 * @tparam Args Constructor argument types.
				 * @param args Constructor arguments.
				 * @return The new element.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<typename... Args>
				reference emplace_back(Args&&... args);

				/**
				 * @brief Destroy the last element.
				 */
				void pop_back();

				/**
				 * @brief Prepend a copy.
				 * @param value Value to copy.
				 * @throws AllocationError A block could not be allocated.
				 */
				void push_front(const T& value);

				/**
				 * @brief Prepend a moved value.
				 * @param value Value to move.
				 * @throws AllocationError A block could not be allocated.
				 */
				void push_front(T&& value);

				/**
				 * @brief Construct an element at the front.
				 * @tparam Args Constructor argument types.
				 * @param args Constructor arguments.
				 * @return The new element.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<typename... Args>
				reference emplace_front(Args&&... args);

				/**
				 * @brief Destroy the first element.
				 */
				void pop_front();

				/**
				 * @brief Append a range.
				 * @tparam R Input range whose value is convertible to T.
				 * @param range Source range.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::ranges::input_range R>
				void append_range(R&& range);

				/**
				 * @brief Prepend a range.
				 * @tparam R Input range whose value is convertible to T.
				 * @param range Source range.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::ranges::input_range R>
				void prepend_range(R&& range);

				/**
				 * @brief Change the element count. New elements are value-initialized.
				 * @param count New element count.
				 * @throws AllocationError A block could not be allocated.
				 */
				void resize(size_type count);

				/**
				 * @brief Change the element count. New elements are copies of @p value.
				 * @param count New element count.
				 * @param value Value copied into each new element.
				 * @throws AllocationError A block could not be allocated.
				 */
				void resize(size_type count, const T& value);

				/**
				 * @brief Exchange blocks. Does not allocate.
				 * @param other Deque to exchange with.
				 */
				void swap(Deque& other) noexcept;

				/**
				 * @brief Copy into a caller-owned `std::deque`.
				 * @return Caller-owned deque.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::deque<T>() const {
					std::deque<T> out;
					for (const T& value : *this)
						out.push_back(value);
					return out;
				}

			private:
				struct Block;

				/**
				 * @brief Element at a logical index.
				 * @param index Logical index.
				 * @return Element.
				 */
				reference Slot(size_type index) noexcept;

				/**
				 * @brief Element at a logical index.
				 * @param index Logical index.
				 * @return Element.
				 */
				const_reference Slot(size_type index) const noexcept;

				/**
				 * @brief Make room for one more element at the back.
				 * @throws AllocationError A block could not be allocated.
				 */
				void EnsureBack();

				/**
				 * @brief Make room for one more element at the front.
				 * @throws AllocationError A block could not be allocated.
				 */
				void EnsureFront();

				Block* m_map;			///< Base-owned map of blocks, or null.
				size_type m_offset;		///< Index of the first element inside the map.
				size_type m_size;		///< Constructed element count.
				size_type m_capacity;	///< Block slots in the map.
		};

		/**
		 * @brief Whether two deques contain equal elements.
		 * @tparam T Element.
		 * @param left Left deque.
		 * @param right Right deque.
		 * @return Whether the elements compare equal.
		 */
		template<Type::SafeValue T>
		bool operator==(const Deque<T>& left, const Deque<T>& right);

		/**
		 * @brief Order two deques lexicographically.
		 * @tparam T Element.
		 * @param left Left deque.
		 * @param right Right deque.
		 * @return Ordering of the elements.
		 */
		template<Type::SafeValue T>
		std::strong_ordering operator<=>(const Deque<T>& left, const Deque<T>& right);

		/**
		 * @brief Exchange two deques.
		 * @tparam T Element.
		 * @param left Left deque.
		 * @param right Right deque.
		 */
		template<Type::SafeValue T>
		void swap(Deque<T>& left, Deque<T>& right) noexcept;
	}
}

/// @cond
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<bool>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<signed char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<unsigned char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<wchar_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<char8_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<char16_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<char32_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<short>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<unsigned short>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<int>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<unsigned int>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<unsigned long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<long long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<unsigned long long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::Deque<StormByte::Safe::String>;
/// @endcond

#include <StormByte/safe/deque.txx>
