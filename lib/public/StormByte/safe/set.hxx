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
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/variant.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <set>
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
		 * @class Set
		 * @brief Ordered unique values stored on Base's heap.
		 * @tparam K Safe value type.
		 * @tparam Compare Strict weak ordering. A stateless comparator does not allocate. A stateful one is stored in the set and must itself be Safe.
		 *
		 * The tree is the same Base-heap tree as @ref Map. Iterators yield the key, not a pair. @ref node_type owns a Base node. It is not `std::set::node_type`. There is no allocator.
		 */
		template<Type::SafeValue K, class Compare = std::less<K>>
		requires std::strict_weak_order<Compare, const K&, const K&>
		class STORMBYTE_PUBLIC_TYPE Set final {
			using Storage = Map<K, Monostate, Compare>;

			public:
				using key_type = K; ///< Value and lookup type.
				using value_type = K; ///< Stored value.
				using key_compare = Compare; ///< Ordering.
				using value_compare = Compare; ///< Ordering of values. Same as the key ordering.
				using size_type = std::size_t; ///< Value count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = const K&; ///< Read-only value reference.
				using const_reference = const K&; ///< Read-only value reference.

				/**
				 * @class iterator
				 * @brief Bidirectional cursor. Dereference is the key.
				 */
				class iterator {
					public:
						using iterator_category = std::bidirectional_iterator_tag; ///< Iterator category.
						using iterator_concept = std::bidirectional_iterator_tag; ///< C++20 iterator concept.
						using value_type = K; ///< Stored value.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using pointer = const K*; ///< Pointer to the key.
						using reference = const K&; ///< Reference to the key.
						using StormByteSafeCursor = void; ///< Base cursor mark. Copying the cursor copies one node pointer.

						/**
						 * @brief Construct a singular iterator.
						 */
						iterator() noexcept = default;

						/**
						 * @brief Return the key.
						 * @return Key. Valid while the node stays linked.
						 */
						reference operator*() const { return m_cursor->first; }

						/**
						 * @brief Return a pointer to the key.
						 * @return Key pointer.
						 */
						pointer operator->() const { return &m_cursor->first; }

						/**
						 * @brief Advance to the next key.
						 * @return This iterator.
						 */
						iterator& operator++() { ++m_cursor; return *this; }

						/**
						 * @brief Advance to the next key.
						 * @return The previous position.
						 */
						iterator operator++(int) { iterator previous = *this; ++*this; return previous; }

						/**
						 * @brief Move to the previous key.
						 * @return This iterator.
						 */
						iterator& operator--() { --m_cursor; return *this; }

						/**
						 * @brief Move to the previous key.
						 * @return The previous position.
						 */
						iterator operator--(int) { iterator previous = *this; --*this; return previous; }

						/**
						 * @brief Compare node identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same node.
						 */
						bool operator==(const iterator& other) const noexcept { return m_cursor == other.m_cursor; }

					private:
						friend class Set;
						explicit iterator(typename Storage::iterator cursor) noexcept: m_cursor(cursor) {}
						typename Storage::iterator m_cursor; ///< Map cursor. The mapped value is unused.
				};

				/**
				 * @class const_iterator
				 * @brief Read-only bidirectional cursor.
				 */
				class const_iterator {
					public:
						using iterator_category = std::bidirectional_iterator_tag; ///< Iterator category.
						using iterator_concept = std::bidirectional_iterator_tag; ///< C++20 iterator concept.
						using value_type = K; ///< Stored value.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using pointer = const K*; ///< Pointer to the key.
						using reference = const K&; ///< Reference to the key.
						using StormByteSafeCursor = void; ///< Base cursor mark. Copying the cursor copies one node pointer.

						/**
						 * @brief Construct a singular iterator.
						 */
						const_iterator() noexcept = default;

						/**
						 * @brief Construct from a mutable iterator.
						 * @param other Mutable cursor.
						 */
						const_iterator(iterator other) noexcept: m_cursor(other.m_cursor) {}

						/**
						 * @brief Return the key.
						 * @return Key.
						 */
						reference operator*() const { return m_cursor->first; }

						/**
						 * @brief Return a pointer to the key.
						 * @return Key pointer.
						 */
						pointer operator->() const { return &m_cursor->first; }

						/**
						 * @brief Advance to the next key.
						 * @return This iterator.
						 */
						const_iterator& operator++() { ++m_cursor; return *this; }

						/**
						 * @brief Advance to the next key.
						 * @return The previous position.
						 */
						const_iterator operator++(int) { const_iterator previous = *this; ++*this; return previous; }

						/**
						 * @brief Move to the previous key.
						 * @return This iterator.
						 */
						const_iterator& operator--() { --m_cursor; return *this; }

						/**
						 * @brief Move to the previous key.
						 * @return The previous position.
						 */
						const_iterator operator--(int) { const_iterator previous = *this; --*this; return previous; }

						/**
						 * @brief Compare node identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same node.
						 */
						bool operator==(const const_iterator& other) const noexcept { return m_cursor == other.m_cursor; }

					private:
						friend class Set;
						explicit const_iterator(typename Storage::const_iterator cursor) noexcept: m_cursor(cursor) {}
						typename Storage::const_iterator m_cursor; ///< Map cursor.
				};

				using reverse_iterator = std::reverse_iterator<iterator>; ///< Mutable reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.
				using insert_result = Pair<iterator, bool>; ///< Iterator to the value and whether it was inserted.

				/**
				 * @class node_type
				 * @brief Owning handle to one extracted Base node.
				 *
				 * Not `std::set::node_type`. The node was allocated with @ref Heap::Allocate. A node belongs to one comparator. @ref merge across a different ordering copies the key.
				 */
				class node_type {
					public:
						/**
						 * @brief Construct an empty handle.
						 */
						node_type() noexcept = default;

						/**
						 * @brief Take the node. @p other becomes empty.
						 * @param other Source handle.
						 */
						node_type(node_type&& other) noexcept = default;

						/**
						 * @brief Take the node. The previous node, if any, is destroyed.
						 * @param other Source handle.
						 * @return This handle.
						 */
						node_type& operator=(node_type&& other) noexcept = default;

						/**
						 * @brief Copying a node handle is not supported.
						 */
						node_type(const node_type&) = delete;

						/**
						 * @brief Copying a node handle is not supported.
						 * @return This handle.
						 */
						node_type& operator=(const node_type&) = delete;

						/**
						 * @brief Destroy the owned node, if any.
						 */
						~node_type() = default;

						/**
						 * @brief Test whether the handle owns a node.
						 * @return Whether no node is owned.
						 */
						bool empty() const noexcept { return m_node.empty(); }

						/**
						 * @brief Test whether the handle owns a node.
						 * @return Whether a node is owned.
						 */
						explicit operator bool() const noexcept { return !empty(); }

						/**
						 * @brief Return the owned value.
						 * @return Value. Valid until the handle is emptied.
						 */
						const K& value() const { return m_node.key(); }

						/**
						 * @brief Exchange owned nodes.
						 * @param other Other handle.
						 */
						void swap(node_type& other) noexcept { m_node.swap(other.m_node); }

					private:
						friend class Set;
						explicit node_type(typename Storage::node_type node) noexcept: m_node(std::move(node)) {}
						typename Storage::node_type m_node; ///< Owned map node.
				};

				/**
				 * @brief Result of inserting an extracted node.
				 */
				struct insert_return_type {
					iterator position; ///< Value, or end when the node was not inserted and is empty.
					bool inserted; ///< Whether the node was linked.
					node_type node; ///< Returned node when the value was already present.
				};

				/**
				 * @brief Construct an empty set.
				 */
				Set() noexcept;

				/**
				 * @brief Construct an empty set with a comparator.
				 * @param compare Value ordering.
				 */
				explicit Set(const Compare& compare) noexcept;

				/**
				 * @brief Copy the values.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				Set(const Set& other);

				/**
				 * @brief Take the values. @p other is left empty.
				 * @param other Source.
				 */
				Set(Set&& other) noexcept;

				/**
				 * @brief Destroy the values.
				 */
				~Set();

				/**
				 * @brief Copy-assign the values.
				 * @param other Source.
				 * @return This set.
				 * @throws AllocationError A node could not be allocated.
				 */
				Set& operator=(const Set& other);

				/**
				 * @brief Move-assign the values. @p other is left empty.
				 * @param other Source.
				 * @return This set.
				 */
				Set& operator=(Set&& other) noexcept;

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values Values. Duplicates are dropped.
				 * @return This set.
				 * @throws AllocationError A node could not be allocated.
				 */
				Set& operator=(std::initializer_list<K> values);

				/**
				 * @brief Construct from an iterator range.
				 * @tparam InputIt Input iterator of values.
				 * @param first Start.
				 * @param last End.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class InputIt>
				Set(InputIt first, InputIt last);

				/**
				 * @brief Construct from an iterator range and a comparator.
				 * @tparam InputIt Input iterator of values.
				 * @param first Start.
				 * @param last End.
				 * @param compare Value ordering.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class InputIt>
				Set(InputIt first, InputIt last, const Compare& compare);

				/**
				 * @brief Construct from an initializer list.
				 * @param values Values. Duplicates are dropped.
				 * @throws AllocationError A node could not be allocated.
				 */
				Set(std::initializer_list<K> values);

				/**
				 * @brief Construct from an initializer list and a comparator.
				 * @param values Values. Duplicates are dropped.
				 * @param compare Value ordering.
				 * @throws AllocationError A node could not be allocated.
				 */
				Set(std::initializer_list<K> values, const Compare& compare);

				/**
				 * @brief Return the ordering.
				 * @return Comparator.
				 */
				key_compare key_comp() const;

				/**
				 * @brief Return the ordering.
				 * @return Comparator.
				 */
				value_compare value_comp() const;

				/**
				 * @brief Return an iterator to the first value.
				 * @return First value, or end when empty.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return an iterator to the first value.
				 * @return First value, or end when empty.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return an iterator to the first value.
				 * @return First value, or end when empty.
				 */
				const_iterator cbegin() const noexcept;

				/**
				 * @brief Return the past-the-end iterator.
				 * @return End iterator.
				 */
				iterator end() noexcept;

				/**
				 * @brief Return the past-the-end iterator.
				 * @return End iterator.
				 */
				const_iterator end() const noexcept;

				/**
				 * @brief Return the past-the-end iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const noexcept;

				/**
				 * @brief Return a reverse iterator to the last value.
				 * @return Last value, or rend when empty.
				 */
				reverse_iterator rbegin() noexcept;

				/**
				 * @brief Return a reverse iterator to the last value.
				 * @return Last value, or rend when empty.
				 */
				const_reverse_iterator rbegin() const noexcept;

				/**
				 * @brief Return a reverse iterator to the last value.
				 * @return Last value, or rend when empty.
				 */
				const_reverse_iterator crbegin() const noexcept;

				/**
				 * @brief Return the past-the-start reverse iterator.
				 * @return Reverse end.
				 */
				reverse_iterator rend() noexcept;

				/**
				 * @brief Return the past-the-start reverse iterator.
				 * @return Reverse end.
				 */
				const_reverse_iterator rend() const noexcept;

				/**
				 * @brief Return the past-the-start reverse iterator.
				 * @return Reverse end.
				 */
				const_reverse_iterator crend() const noexcept;

				/**
				 * @brief Test whether the set has no values.
				 * @return Whether the set is empty.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the number of values.
				 * @return Value count.
				 */
				size_type size() const noexcept;

				/**
				 * @brief Return the maximum number of values.
				 * @return Implementation limit.
				 */
				size_type max_size() const noexcept;

				/**
				 * @brief Destroy every value.
				 */
				void clear() noexcept;

				/**
				 * @brief Insert a value if it is absent.
				 * @param value Value to copy.
				 * @return Iterator to the value and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert(const K& value);

				/**
				 * @brief Insert a value if it is absent.
				 * @param value Value to move.
				 * @return Iterator to the value and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert(K&& value);

				/**
				 * @brief Insert a value, using @p hint when the neighbours confirm it.
				 * @param hint Suggested position.
				 * @param value Value to copy.
				 * @return Iterator to the value.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert(const_iterator hint, const K& value);

				/**
				 * @brief Insert a value, using @p hint when the neighbours confirm it.
				 * @param hint Suggested position.
				 * @param value Value to move.
				 * @return Iterator to the value.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert(const_iterator hint, K&& value);

				/**
				 * @brief Insert every value in `[first, last)`.
				 * @tparam InputIt Input iterator of values.
				 * @param first Start.
				 * @param last End.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class InputIt>
				void insert(InputIt first, InputIt last);

				/**
				 * @brief Insert every value of an initializer list.
				 * @param values Values.
				 * @throws AllocationError A node could not be allocated.
				 */
				void insert(std::initializer_list<K> values);

				/**
				 * @brief Insert a copy of @p range.
				 * @tparam R Input range of values.
				 * @param range Source. Instantiated in the caller.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
				STORMBYTE_FORCE_INLINE void insert_range(R&& range) {
					for (auto&& value : range)
						insert(value_type(std::forward<decltype(value)>(value)));
				}

				/**
				 * @brief Construct a value in a new node if it is absent.
				 * @tparam Args Value constructor argument types.
				 * @param args Arguments forwarded to K.
				 * @return Iterator to the value and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				insert_result emplace(Args&&... args);

				/**
				 * @brief Construct a value, using @p hint when the neighbours confirm it.
				 * @tparam Args Value constructor argument types.
				 * @param hint Suggested position.
				 * @param args Arguments forwarded to K.
				 * @return Iterator to the value.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				iterator emplace_hint(const_iterator hint, Args&&... args);

				/**
				 * @brief Insert an extracted node if its value is absent. The same node is linked.
				 * @param handle Node from a set with this ordering. Emptied when inserted.
				 * @return Position, whether it was inserted, and the rejected node.
				 */
				insert_return_type insert(node_type&& handle);

				/**
				 * @brief Insert an extracted node, using @p hint when the neighbours confirm it.
				 * @param hint Suggested position.
				 * @param handle Node from a set with this ordering. Emptied when inserted.
				 * @return Iterator to the value, or end when @p handle was empty.
				 */
				iterator insert(const_iterator hint, node_type&& handle);

				/**
				 * @brief Unlink the value at @p position and return that node.
				 * @param position Value to extract.
				 * @return Owning handle. Empty when @p position is end.
				 */
				node_type extract(const_iterator position);

				/**
				 * @brief Unlink @p value and return that node.
				 * @param value Lookup value.
				 * @return Owning handle. Empty when the value is absent.
				 */
				node_type extract(const K& value);

				/**
				 * @brief Take unique values from @p source.
				 * @tparam OtherCompare Source ordering.
				 * @param source Set to drain. The same ordering transfers the node. A different ordering copies the key. Values already present stay in @p source.
				 * @throws AllocationError A copied node could not be allocated.
				 */
				template<class OtherCompare>
				void merge(Set<K, OtherCompare>& source);

				/**
				 * @brief Erase the value at an iterator.
				 * @param position Value to erase.
				 * @return Iterator following the erased value.
				 */
				iterator erase(iterator position) noexcept;

				/**
				 * @brief Erase the value at a read-only iterator.
				 * @param position Value to erase.
				 * @return Iterator following the erased value.
				 */
				iterator erase(const_iterator position) noexcept;

				/**
				 * @brief Erase `[first, last)`.
				 * @param first First value to erase.
				 * @param last End of the range.
				 * @return Iterator following the last erased value.
				 */
				iterator erase(const_iterator first, const_iterator last) noexcept;

				/**
				 * @brief Erase a value.
				 * @param value Lookup value.
				 * @return Number of erased values, zero or one.
				 */
				size_type erase(const K& value) noexcept;

				/**
				 * @brief Exchange values and comparators.
				 * @param other Other set.
				 */
				void swap(Set& other) noexcept;

				/**
				 * @brief Find a value.
				 * @param value Lookup value.
				 * @return Iterator to the value, or end.
				 */
				iterator find(const K& value);

				/**
				 * @brief Find a value.
				 * @param value Lookup value.
				 * @return Iterator to the value, or end.
				 */
				const_iterator find(const K& value) const;

				/**
				 * @brief Count matching values.
				 * @param value Lookup value.
				 * @return Zero or one.
				 */
				size_type count(const K& value) const;

				/**
				 * @brief Test whether a value is present.
				 * @param value Lookup value.
				 * @return Whether the value is present.
				 */
				bool contains(const K& value) const;

				/**
				 * @brief Return the first value not less than @p value.
				 * @param value Lookup value.
				 * @return Lower bound, or end.
				 */
				iterator lower_bound(const K& value);

				/**
				 * @brief Return the first value not less than @p value.
				 * @param value Lookup value.
				 * @return Lower bound, or end.
				 */
				const_iterator lower_bound(const K& value) const;

				/**
				 * @brief Return the first value greater than @p value.
				 * @param value Lookup value.
				 * @return Upper bound, or end.
				 */
				iterator upper_bound(const K& value);

				/**
				 * @brief Return the first value greater than @p value.
				 * @param value Lookup value.
				 * @return Upper bound, or end.
				 */
				const_iterator upper_bound(const K& value) const;

				/**
				 * @brief Return the range of values equal to @p value.
				 * @param value Lookup value.
				 * @return Lower and upper bound.
				 */
				Pair<iterator, iterator> equal_range(const K& value);

				/**
				 * @brief Return the range of values equal to @p value.
				 * @param value Lookup value.
				 * @return Lower and upper bound.
				 */
				Pair<const_iterator, const_iterator> equal_range(const K& value) const;

				/**
				 * @brief Compare lexicographically.
				 * @param other Other set.
				 * @return Whether this set is equal to @p other.
				 */
				bool operator==(const Set& other) const;

				/**
				 * @brief Compare lexicographically.
				 * @param other Other set.
				 * @return Whether the sets differ.
				 */
				bool operator!=(const Set& other) const;

				/**
				 * @brief Compare lexicographically.
				 * @param other Other set.
				 * @return Whether this set precedes @p other.
				 */
				bool operator<(const Set& other) const;

				/**
				 * @brief Compare lexicographically.
				 * @param other Other set.
				 * @return Whether this set precedes or equals @p other.
				 */
				bool operator<=(const Set& other) const;

				/**
				 * @brief Compare lexicographically.
				 * @param other Other set.
				 * @return Whether this set follows @p other.
				 */
				bool operator>(const Set& other) const;

				/**
				 * @brief Compare lexicographically.
				 * @param other Other set.
				 * @return Whether this set follows or equals @p other.
				 */
				bool operator>=(const Set& other) const;

				/**
				 * @brief Copy the values into caller-owned STL storage.
				 * @return A `std::set` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::set<K, Compare>() const {
					std::set<K, Compare> exported(key_comp());
					for (const K& value : *this)
						exported.insert(value);
					return exported;
				}

			private:
				Storage m_values; ///< Ordered unique keys. The mapped monostate is unused.
		};

		/**
		 * @brief Exchange two sets.
		 * @tparam K Value type.
		 * @tparam Compare Ordering.
		 * @param left Left set.
		 * @param right Right set.
		 */
		template<Type::SafeValue K, class Compare>
		inline void swap(Set<K, Compare>& left, Set<K, Compare>& right) noexcept {
			left.swap(right);
		}
	}
}

#include <StormByte/safe/set.txx>
