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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <list>
#include <ranges>
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
		 * @class List
		 * @brief Doubly linked list stored on Base's heap.
		 * @tparam T Safe value type.
		 *
		 * Nodes are allocated with @ref Heap::Allocate. There is no allocator. The sentinel lives in the list object, so an empty list does not construct a @p T. Iterators are bidirectional and point at Base nodes. @ref node_type owns a Base node. It is not `std::list::node_type`. @ref extract, @ref splice and @ref merge relink that node. They do not copy it. A conversion to or from `std::list` copies the elements.
		 */
		template<Type::SafeValue T>
		class STORMBYTE_PUBLIC_TYPE List final {
			private:
				struct Link;
				struct Node;

			public:
				using value_type = T; ///< Element type.
				using size_type = std::size_t; ///< Element count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = T&; ///< Mutable element reference.
				using const_reference = const T&; ///< Read-only element reference.

				/**
				 * @class BasicIterator
				 * @brief Bidirectional cursor over Base-owned list nodes.
				 * @tparam IsConst Whether dereference is read-only.
				 *
				 * The cursor stores one link pointer. End designates the sentinel. Copying it does not allocate and does not take ownership.
				 */
				template<bool IsConst>
				class BasicIterator final {
						using LinkType = std::conditional_t<IsConst, const Link, Link>;

					public:
						using iterator_category = std::bidirectional_iterator_tag; ///< Iterator category.
						using iterator_concept = std::bidirectional_iterator_tag; ///< C++20 iterator concept.
						using value_type = T; ///< Element type.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using reference = std::conditional_t<IsConst, const T&, T&>; ///< Element reference.
						using pointer = std::conditional_t<IsConst, const T*, T*>; ///< Element pointer.
						using StormByteSafeCursor = void; ///< Base cursor mark. Copying the cursor copies one link pointer.

						/**
						 * @brief Construct a singular iterator.
						 */
						BasicIterator() noexcept: m_link(nullptr) {}

						/**
						 * @brief Convert a mutable iterator to a const iterator.
						 * @param other Mutable iterator.
						 */
						template<bool OtherConst>
						requires IsConst && (!OtherConst)
						BasicIterator(const BasicIterator<OtherConst>& other) noexcept: m_link(other.m_link) {}

						/**
						 * @brief Return the element stored in the node.
						 * @return Element reference. Valid until the node is erased.
						 */
						reference operator*() const noexcept { return static_cast<Node*>(const_cast<Link*>(m_link))->Value; }

						/**
						 * @brief Access the element stored in the node.
						 * @return Element pointer. Valid until the node is erased.
						 */
						pointer operator->() const noexcept { return &static_cast<Node*>(const_cast<Link*>(m_link))->Value; }

						/**
						 * @brief Advance to the next element.
						 * @return This iterator.
						 */
						BasicIterator& operator++() noexcept { m_link = m_link->Next; return *this; }

						/**
						 * @brief Advance to the next element.
						 * @return Previous iterator value.
						 */
						BasicIterator operator++(int) noexcept { BasicIterator previous(*this); ++(*this); return previous; }

						/**
						 * @brief Move to the previous element.
						 * @return This iterator.
						 */
						BasicIterator& operator--() noexcept { m_link = m_link->Prev; return *this; }

						/**
						 * @brief Move to the previous element.
						 * @return Previous iterator value.
						 */
						BasicIterator operator--(int) noexcept { BasicIterator previous(*this); --(*this); return previous; }

						/**
						 * @brief Compare link identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same link.
						 */
						template<bool OtherConst>
						bool operator==(const BasicIterator<OtherConst>& other) const noexcept { return m_link == other.m_link; }

					private:
						friend class List;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind the cursor to a link. The sentinel is end.
						 * @param link Link. Not null for a list iterator.
						 */
						explicit BasicIterator(LinkType* link) noexcept: m_link(link) {}

						LinkType* m_link; ///< Current link, or the sentinel at end.
				};

				using iterator = BasicIterator<false>; ///< Mutable iterator.
				using const_iterator = BasicIterator<true>; ///< Read-only iterator.
				using reverse_iterator = std::reverse_iterator<iterator>; ///< Mutable reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.

				/**
				 * @class node_type
				 * @brief Owning handle to one extracted Base node.
				 *
				 * Not `std::list::node_type`. The node was allocated with @ref Heap::Allocate and is released with @ref Heap::Free. @ref extract transfers this node. It does not allocate another.
				 */
				class node_type {
					public:
						/**
						 * @brief Construct an empty handle.
						 */
						node_type() noexcept = default;

						/**
						 * @brief Copying a node handle is not supported.
						 */
						node_type(const node_type&) = delete;

						/**
						 * @brief Take the node. @p other becomes empty.
						 * @param other Source handle.
						 */
						node_type(node_type&& other) noexcept: m_node(other.m_node) { other.m_node = nullptr; }

						/**
						 * @brief Destroy the owned node, if any.
						 */
						~node_type() { reset(); }

						/**
						 * @brief Copying a node handle is not supported.
						 * @return This handle.
						 */
						node_type& operator=(const node_type&) = delete;

						/**
						 * @brief Take the node. The previous node, if any, is destroyed.
						 * @param other Source handle.
						 * @return This handle.
						 */
						node_type& operator=(node_type&& other) noexcept {
							if (this != &other) {
								reset();
								m_node = other.m_node;
								other.m_node = nullptr;
							}
							return *this;
						}

						/**
						 * @brief Test whether the handle owns a node.
						 * @return Whether no node is owned.
						 */
						bool empty() const noexcept { return m_node == nullptr; }

						/**
						 * @brief Test whether the handle owns a node.
						 * @return Whether a node is owned.
						 */
						explicit operator bool() const noexcept { return !empty(); }

						/**
						 * @brief Return the owned element.
						 * @return Element. Valid until the handle is emptied.
						 */
						T& value() { return m_node->Value; }

						/**
						 * @brief Return the owned element.
						 * @return Element. Valid until the handle is emptied.
						 */
						const T& value() const { return m_node->Value; }

						/**
						 * @brief Exchange owned nodes.
						 * @param other Other handle.
						 */
						void swap(node_type& other) noexcept { std::swap(m_node, other.m_node); }

					private:
						friend class List;

						/**
						 * @brief Take ownership of an unlinked node.
						 * @param node Node. Not null.
						 */
						explicit node_type(Node* node) noexcept: m_node(node) {}

						/**
						 * @brief Destroy and release the owned node.
						 */
						void reset() noexcept {
							if (m_node == nullptr)
								return;
							m_node->Value.~T();
							Heap::Free(m_node);
							m_node = nullptr;
						}

						Node* m_node = nullptr; ///< Owned node, or null.
				};

				/**
				 * @brief Construct an empty list.
				 */
				List() noexcept;

				/**
				 * @brief Construct @p count value-initialized elements.
				 * @param count Element count.
				 * @throws AllocationError A node could not be allocated.
				 */
				explicit List(size_type count);

				/**
				 * @brief Construct @p count copies of @p value.
				 * @param count Element count.
				 * @param value Element to copy.
				 * @throws AllocationError A node could not be allocated.
				 */
				List(size_type count, const T& value);

				/**
				 * @brief Copy every node into new Base blocks.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				List(const List& other);

				/**
				 * @brief Take the chain. @p other is left empty.
				 * @param other Source.
				 */
				List(List&& other) noexcept;

				/**
				 * @brief Copy a caller-owned STL list. Its state is unchanged.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit List(const std::list<T>& other): List() {
					for (const T& value : other)
						push_back(value);
				}

				/**
				 * @brief Move elements from a caller-owned STL list and leave it empty.
				 * @param other Source. Cleared before this function returns.
				 * @throws AllocationError A node could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit List(std::list<T>&& other): List() {
					for (T& value : other)
						push_back(std::move(value));
					other.clear();
				}

				/**
				 * @brief Copy the elements of @p range.
				 * @tparam R Input range of @p T.
				 * @param range Source. Instantiated in the caller.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, T>
				STORMBYTE_FORCE_INLINE explicit List(std::from_range_t, R&& range): List() {
					insert_range(end(), std::forward<R>(range));
				}

				/**
				 * @brief Insert every element of an initializer list.
				 * @param values Elements.
				 * @throws AllocationError A node could not be allocated.
				 */
				List(std::initializer_list<T> values);

				/**
				 * @brief Destroy every node through Base.
				 */
				~List() noexcept;

				/**
				 * @brief Copy-assign. The previous chain is released only after the copy exists.
				 * @param other Source.
				 * @return This list.
				 * @throws AllocationError A node could not be allocated.
				 */
				List& operator=(const List& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This list.
				 */
				List& operator=(List&& other) noexcept;

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values Elements.
				 * @return This list.
				 * @throws AllocationError A node could not be allocated.
				 */
				List& operator=(std::initializer_list<T> values);

				/**
				 * @brief Replace the contents with @p count copies of @p value.
				 * @param count Element count.
				 * @param value Element to copy.
				 * @throws AllocationError A node could not be allocated.
				 */
				void assign(size_type count, const T& value);

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values Elements.
				 * @throws AllocationError A node could not be allocated.
				 */
				void assign(std::initializer_list<T> values);

				/**
				 * @brief Replace the contents with @p range.
				 * @tparam R Input range of @p T.
				 * @param range Source. Instantiated in the caller.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, T>
				STORMBYTE_FORCE_INLINE void assign_range(R&& range) {
					List replacement(std::from_range, std::forward<R>(range));
					swap(replacement);
				}

				/**
				 * @brief Return the first element.
				 * @return First element. The list is not empty.
				 */
				T& front() noexcept { assert(!empty()); return *begin(); }

				/**
				 * @brief Return the first element.
				 * @return First element. The list is not empty.
				 */
				const T& front() const noexcept { assert(!empty()); return *begin(); }

				/**
				 * @brief Return the last element.
				 * @return Last element. The list is not empty.
				 */
				T& back() noexcept { assert(!empty()); return *std::prev(end()); }

				/**
				 * @brief Return the last element.
				 * @return Last element. The list is not empty.
				 */
				const T& back() const noexcept { assert(!empty()); return *std::prev(end()); }

				/**
				 * @brief Return the first element.
				 * @return Mutable iterator, or @ref end when empty.
				 */
				iterator begin() noexcept { return iterator(m_sentinel.Next); }

				/**
				 * @brief Return the past-the-end iterator.
				 * @return Mutable iterator.
				 */
				iterator end() noexcept { return iterator(&m_sentinel); }

				/**
				 * @brief Return the first element.
				 * @return Read-only iterator, or @ref end when empty.
				 */
				const_iterator begin() const noexcept { return const_iterator(m_sentinel.Next); }

				/**
				 * @brief Return the past-the-end iterator.
				 * @return Read-only iterator.
				 */
				const_iterator end() const noexcept { return const_iterator(&m_sentinel); }

				/**
				 * @brief Return the first element.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the past-the-end iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Return a reverse iterator to the last element.
				 * @return Mutable reverse iterator.
				 */
				reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

				/**
				 * @brief Return the reverse past-the-end iterator.
				 * @return Mutable reverse iterator.
				 */
				reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

				/**
				 * @brief Return a reverse iterator to the last element.
				 * @return Read-only reverse iterator.
				 */
				const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

				/**
				 * @brief Return the reverse past-the-end iterator.
				 * @return Read-only reverse iterator.
				 */
				const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

				/**
				 * @brief Return a reverse iterator to the last element.
				 * @return Read-only reverse iterator.
				 */
				const_reverse_iterator crbegin() const noexcept { return rbegin(); }

				/**
				 * @brief Return the reverse past-the-end iterator.
				 * @return Read-only reverse iterator.
				 */
				const_reverse_iterator crend() const noexcept { return rend(); }

				/**
				 * @brief Test whether the list holds no elements.
				 * @return Whether the list is empty.
				 */
				bool empty() const noexcept { return m_size == 0; }

				/**
				 * @brief Return the element count.
				 * @return Element count.
				 */
				size_type size() const noexcept { return m_size; }

				/**
				 * @brief Return the maximum element count this list can report.
				 * @return Maximum count.
				 */
				size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

				/**
				 * @brief Destroy every element. The sentinel stays.
				 */
				void clear() noexcept;

				/**
				 * @brief Insert a copy of @p value before @p position.
				 * @param position Insertion point.
				 * @param value Element.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError A node could not be allocated.
				 */
				iterator insert(const_iterator position, const T& value);

				/**
				 * @brief Insert a moved @p value before @p position.
				 * @param position Insertion point.
				 * @param value Element.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError A node could not be allocated.
				 */
				iterator insert(const_iterator position, T&& value);

				/**
				 * @brief Insert @p count copies of @p value before @p position.
				 * @param position Insertion point.
				 * @param count Copy count.
				 * @param value Element.
				 * @return Iterator to the first inserted element, or @p position when @p count is zero.
				 * @throws AllocationError A node could not be allocated.
				 */
				iterator insert(const_iterator position, size_type count, const T& value);

				/**
				 * @brief Insert every element of an initializer list before @p position.
				 * @param position Insertion point.
				 * @param values Elements.
				 * @return Iterator to the first inserted element, or @p position when @p values is empty.
				 * @throws AllocationError A node could not be allocated.
				 */
				iterator insert(const_iterator position, std::initializer_list<T> values);

				/**
				 * @brief Insert every element of @p range before @p position.
				 * @tparam R Input range of @p T.
				 * @param position Insertion point.
				 * @param range Source. Instantiated in the caller.
				 * @return Iterator to the first inserted element, or @p position when @p range is empty.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, T>
				STORMBYTE_FORCE_INLINE iterator insert_range(const_iterator position, R&& range) {
					iterator first(const_cast<Link*>(position.m_link));
					bool started = false;
					for (auto&& value : range) {
						iterator inserted = insert(position, T(std::forward<decltype(value)>(value)));
						if (!started) {
							first = inserted;
							started = true;
						}
					}
					return first;
				}

				/**
				 * @brief Construct an element in place before @p position.
				 * @tparam Args Constructor argument types.
				 * @param position Insertion point.
				 * @param args Arguments forwarded to @p T.
				 * @return Iterator to the inserted element.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class... Args>
				iterator emplace(const_iterator position, Args&&... args);

				/**
				 * @brief Erase the element at @p position.
				 * @param position Iterator to an element. Not @ref end.
				 * @return Iterator to the following element.
				 */
				iterator erase(const_iterator position);

				/**
				 * @brief Erase the half-open range [@p first, @p last).
				 * @param first First element to erase.
				 * @param last One past the last element to erase.
				 * @return Iterator to @p last.
				 */
				iterator erase(const_iterator first, const_iterator last);

				/**
				 * @brief Copy @p value to the front.
				 * @param value Element.
				 * @throws AllocationError A node could not be allocated.
				 */
				void push_front(const T& value);

				/**
				 * @brief Move @p value to the front.
				 * @param value Element.
				 * @throws AllocationError A node could not be allocated.
				 */
				void push_front(T&& value);

				/**
				 * @brief Construct an element at the front.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p T.
				 * @return Inserted element.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class... Args>
				T& emplace_front(Args&&... args);

				/**
				 * @brief Destroy the first element. The list is not empty.
				 */
				void pop_front() noexcept;

				/**
				 * @brief Copy @p value to the back.
				 * @param value Element.
				 * @throws AllocationError A node could not be allocated.
				 */
				void push_back(const T& value);

				/**
				 * @brief Move @p value to the back.
				 * @param value Element.
				 * @throws AllocationError A node could not be allocated.
				 */
				void push_back(T&& value);

				/**
				 * @brief Construct an element at the back.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p T.
				 * @return Inserted element.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class... Args>
				T& emplace_back(Args&&... args);

				/**
				 * @brief Destroy the last element. The list is not empty.
				 */
				void pop_back() noexcept;

				/**
				 * @brief Grow or shrink to @p count. New elements are value-initialized.
				 * @param count New element count.
				 * @throws AllocationError A node could not be allocated.
				 */
				void resize(size_type count);

				/**
				 * @brief Grow or shrink to @p count. New elements are copies of @p value.
				 * @param count New element count.
				 * @param value Element to copy.
				 * @throws AllocationError A node could not be allocated.
				 */
				void resize(size_type count, const T& value);

				/**
				 * @brief Exchange chains. Does not allocate.
				 * @param other List to exchange with.
				 */
				void swap(List& other) noexcept;

				/**
				 * @brief Relink every node of @p other before @p position. @p other is left empty.
				 * @param position Insertion point.
				 * @param other Source. Not this list.
				 */
				void splice(const_iterator position, List& other) noexcept;

				/**
				 * @brief Relink the node at @p source before @p position.
				 * @param position Insertion point.
				 * @param other List that owns @p source.
				 * @param source Element to relink. Not @ref end.
				 */
				void splice(const_iterator position, List& other, const_iterator source) noexcept;

				/**
				 * @brief Relink [@p first, @p last) before @p position.
				 * @param position Insertion point.
				 * @param other List that owns the range.
				 * @param first First element to relink.
				 * @param last One past the last element to relink.
				 */
				void splice(const_iterator position, List& other, const_iterator first, const_iterator last) noexcept;

				/**
				 * @brief Unlink the element at @p position. The node is not copied.
				 * @param position Iterator to an element. Not @ref end.
				 * @return Owning handle.
				 */
				node_type extract(const_iterator position);

				/**
				 * @brief Link an extracted node before @p position.
				 * @param position Insertion point.
				 * @param node Handle. Left empty.
				 * @return Iterator to the linked element, or @p position when @p node is empty.
				 */
				iterator insert(const_iterator position, node_type&& node);

				/**
				 * @brief Move the ordered nodes of @p other into this ordered list.
				 * @param other Source. Left with the nodes that were not taken. Both lists are ordered by `operator<`.
				 */
				void merge(List& other);

				/**
				 * @brief Move the ordered nodes of @p other into this ordered list.
				 * @tparam Compare Strict weak ordering.
				 * @param other Source.
				 * @param compare Element ordering.
				 */
				template<class Compare>
				void merge(List& other, Compare compare);

				/**
				 * @brief Erase every element equal to @p value.
				 * @param value Element to erase.
				 * @return Number of erased elements.
				 */
				size_type remove(const T& value);

				/**
				 * @brief Erase every element for which @p predicate is true.
				 * @tparam Predicate Unary predicate.
				 * @param predicate Erase condition.
				 * @return Number of erased elements.
				 */
				template<class Predicate>
				size_type remove_if(Predicate predicate);

				/**
				 * @brief Reverse the chain. Does not allocate.
				 */
				void reverse() noexcept;

				/**
				 * @brief Erase consecutive duplicates. Equality is `operator==`.
				 * @return Number of erased elements.
				 */
				size_type unique();

				/**
				 * @brief Erase consecutive elements for which @p predicate is true.
				 * @tparam Predicate Binary predicate.
				 * @param predicate Duplicate condition.
				 * @return Number of erased elements.
				 */
				template<class Predicate>
				size_type unique(Predicate predicate);

				/**
				 * @brief Sort the chain. Ordering is `operator<`. Nodes are relinked, not copied.
				 */
				void sort();

				/**
				 * @brief Sort the chain. Nodes are relinked, not copied.
				 * @tparam Compare Strict weak ordering.
				 * @param compare Element ordering.
				 */
				template<class Compare>
				void sort(Compare compare);

				/**
				 * @brief Compare elements in order.
				 * @param other List to compare.
				 * @return Whether both lists hold the same elements in the same order.
				 */
				bool operator==(const List& other) const;

				/**
				 * @brief Order the lists lexicographically.
				 * @param other List to compare.
				 * @return Three-way comparison of the elements.
				 */
				std::strong_ordering operator<=>(const List& other) const;

				/**
				 * @brief Copy the elements into caller-owned STL storage.
				 * @return A `std::list` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::list<T>() const {
					std::list<T> copied;
					for (const T& value : *this)
						copied.push_back(value);
					return copied;
				}

			private:
				/**
				 * @brief Link shared by the sentinel and every node.
				 */
				struct Link {
					Link* Next = nullptr; ///< Following link.
					Link* Prev = nullptr; ///< Preceding link.
				};

				/**
				 * @brief One element. The links are not part of the public value.
				 */
				struct Node: Link {
					T Value; ///< Public element.

					/**
					 * @brief Construct the element.
					 * @tparam Args Constructor argument types.
					 * @param args Arguments forwarded to @p T.
					 */
					template<class... Args>
					explicit Node(Args&&... args): Value(std::forward<Args>(args)...) {}
				};

				/**
				 * @brief Point the sentinel at itself.
				 */
				void ResetSentinel() noexcept;

				/**
				 * @brief Allocate a node and construct @p T.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p T.
				 * @return Node. Not null.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class... Args>
				Node* MakeNode(Args&&... args);

				/**
				 * @brief Link @p node before @p position. Does not allocate.
				 * @param position Link that will follow @p node.
				 * @param node Node. Not null.
				 */
				void LinkBefore(Link* position, Node* node) noexcept;

				/**
				 * @brief Unlink @p node. The node is not destroyed.
				 * @param node Node. Not the sentinel.
				 */
				void Unlink(Node* node) noexcept;

				/**
				 * @brief Destroy and release every node.
				 */
				void Destroy() noexcept;

				Link m_sentinel{}; ///< End link. Not a node and not allocated.
				size_type m_size = 0; ///< Element count.
		};

		/**
		 * @brief Exchange two lists.
		 * @tparam T Safe value.
		 * @param left First list.
		 * @param right Second list.
		 */
		template<Type::SafeValue T>
		void swap(List<T>& left, List<T>& right) noexcept { left.swap(right); }
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Concepts and traits used to constrain Safe components.
	 */
	namespace Type {
		/**
		 * @brief A list of a Safe value is Safe.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		requires IsSafe<T>::value
		struct IsSafe<Safe::List<T>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the element.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		requires MaybeSafe<T>
		struct IsMaybeSafe<Safe::List<T>>: std::true_type {};

		/**
		 * @brief Admits a Safe list as a collection value.
		 * @tparam T Safe value.
		 */
		template<SafeValue T>
		struct IsSafeValue<Safe::List<T>>: std::true_type {};
	}
}

/// @cond
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<bool>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<signed char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<unsigned char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<wchar_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<char8_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<char16_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<char32_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<short>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<unsigned short>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<int>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<unsigned int>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<unsigned long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<long long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<unsigned long long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::List<StormByte::Safe::String>;
/// @endcond

#include <StormByte/safe/list.txx>
