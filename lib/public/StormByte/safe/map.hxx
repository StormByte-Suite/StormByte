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
#include <StormByte/safe/heap.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <string>
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
		 * @class Map
		 * @brief Ordered unique-key map stored on Base's heap.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Strict weak ordering. A stateless comparator does not allocate. A stateful one is stored in the map object and must itself be Safe.
		 *
		 * Nodes are allocated with @ref Heap::Allocate. Iterators point at those nodes. Dereference returns a real @ref Pair reference, so the map is a bidirectional range. Public results are @ref Pair, never `std::pair`.
		 */
		template<Type::SafeValue K, Type::SafeValue V, class Compare = std::less<K>>
		requires std::strict_weak_order<Compare, const K&, const K&>
		class STORMBYTE_PUBLIC_TYPE Map final {
			private:
				struct Node;

			public:
				using key_type = K; ///< Key type.
				using mapped_type = V; ///< Mapped type.
				using value_type = Pair<const K, V>; ///< Entry type. The key is immutable.
				using key_compare = Compare; ///< Key ordering.
				using size_type = std::size_t; ///< Entry count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = value_type&; ///< Mutable entry reference.
				using const_reference = const value_type&; ///< Read-only entry reference.

				/**
				 * @class BasicIterator
				 * @brief Bidirectional cursor over Base-owned map nodes.
				 * @tparam IsConst Whether dereference is read-only.
				 *
				 * The cursor stores one node pointer. Copying it does not allocate and does not take ownership.
				 */
				template<bool IsConst>
				class BasicIterator final {
						using NodeType = std::conditional_t<IsConst, const Node, Node>;

					public:
						using iterator_category = std::bidirectional_iterator_tag; ///< Iterator category.
						using iterator_concept = std::bidirectional_iterator_tag; ///< C++20 iterator concept.
						using value_type = Map::value_type; ///< Entry type.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using reference = std::conditional_t<IsConst, const value_type&, value_type&>; ///< Entry reference.
						using pointer = std::conditional_t<IsConst, const value_type*, value_type*>; ///< Entry pointer.
						using StormByteSafeCursor = void; ///< Base cursor mark. Copying the cursor copies one node pointer.

						/**
						 * @brief Construct a singular iterator.
						 */
						BasicIterator() noexcept: m_node(nullptr) {}

						/**
						 * @brief Convert a mutable iterator to a const iterator.
						 * @param other Mutable iterator.
						 */
						template<bool OtherConst>
						requires IsConst && (!OtherConst)
						BasicIterator(const BasicIterator<OtherConst>& other) noexcept: m_node(other.m_node) {}

						/**
						 * @brief Return the entry stored in the node.
						 * @return Entry reference. Valid until the node is erased.
						 */
						reference operator*() const noexcept { return m_node->Entry; }

						/**
						 * @brief Access the entry stored in the node.
						 * @return Entry pointer. Valid until the node is erased.
						 */
						pointer operator->() const noexcept { return &m_node->Entry; }

						/**
						 * @brief Advance to the next ordered entry.
						 * @return This iterator.
						 */
						BasicIterator& operator++() noexcept;

						/**
						 * @brief Advance to the next ordered entry.
						 * @return Previous iterator value.
						 */
						BasicIterator operator++(int) noexcept;

						/**
						 * @brief Move to the previous ordered entry.
						 * @return This iterator.
						 */
						BasicIterator& operator--() noexcept;

						/**
						 * @brief Move to the previous ordered entry.
						 * @return Previous iterator value.
						 */
						BasicIterator operator--(int) noexcept;

						/**
						 * @brief Compare node identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same node.
						 */
						template<bool OtherConst>
						bool operator==(const BasicIterator<OtherConst>& other) const noexcept { return m_node == other.m_node; }

					private:
						friend class Map;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind the cursor to a node. Null is the singular iterator.
						 * @param node Node, or null.
						 */
						explicit BasicIterator(NodeType* node) noexcept: m_node(node) {}

						NodeType* m_node; ///< Current node, or the header when at end.
				};

				using iterator = BasicIterator<false>; ///< Mutable iterator.
				using const_iterator = BasicIterator<true>; ///< Read-only iterator.
				using insert_result = Pair<iterator, bool>; ///< Iterator to the entry and whether it was inserted.
				using equal_range_result = Pair<iterator, iterator>; ///< Lower and upper bound.
				using const_equal_range_result = Pair<const_iterator, const_iterator>; ///< Read-only lower and upper bound.

				/**
				 * @brief Construct an empty map.
				 */
				Map() noexcept;

				/**
				 * @brief Construct an empty map with a comparator.
				 * @param compare Key ordering.
				 */
				explicit Map(const Compare& compare) noexcept;

				/**
				 * @brief Copy every node into new Base blocks.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				Map(const Map& other);

				/**
				 * @brief Take the tree. @p other is left empty.
				 * @param other Source.
				 */
				Map(Map&& other) noexcept;

				/**
				 * @brief Copy a caller-owned STL map. Its state is unchanged.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				explicit Map(const std::map<K, V, Compare>& other);

				/**
				 * @brief Move elements from a caller-owned STL map and leave it empty.
				 * @param other Source. Cleared before this function returns.
				 * @throws AllocationError A node could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit Map(std::map<K, V, Compare>&& other): Map(other.key_comp()) {
					for (auto& entry : other)
						insert(value_type(std::move(entry.first), std::move(entry.second)));
					other.clear();
				}

				/**
				 * @brief Destroy every node through Base.
				 */
				~Map() noexcept;

				/**
				 * @brief Copy-assign. The previous tree is released only after the copy exists.
				 * @param other Source.
				 * @return This map.
				 * @throws AllocationError A node could not be allocated.
				 */
				Map& operator=(const Map& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This map.
				 */
				Map& operator=(Map&& other) noexcept;

				/**
				 * @brief Copy-assign a caller-owned STL map. Its state is unchanged.
				 * @param other Source.
				 * @return This map.
				 * @throws AllocationError A node could not be allocated.
				 */
				Map& operator=(const std::map<K, V, Compare>& other);

				/**
				 * @brief Move-assign elements from a caller-owned STL map and leave it empty.
				 * @param other Source. Cleared before this function returns.
				 * @return This map.
				 * @throws AllocationError A node could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE Map& operator=(std::map<K, V, Compare>&& other) {
					Map replacement(std::move(other));
					swap(replacement);
					return *this;
				}

				/**
				 * @brief Return the key ordering.
				 * @return Comparator.
				 */
				key_compare key_comp() const { return m_compare; }

				/**
				 * @brief Return a mutable iterator to the first entry.
				 * @return Mutable iterator.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return the mutable end iterator.
				 * @return End iterator.
				 */
				iterator end() noexcept;

				/**
				 * @brief Return a read-only iterator to the first entry.
				 * @return Read-only iterator.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return the read-only end iterator.
				 * @return End iterator.
				 */
				const_iterator end() const noexcept;

				/**
				 * @brief Return a read-only iterator to the first entry.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept;

				/**
				 * @brief Return the read-only end iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const noexcept;

				/**
				 * @brief Test whether the map has no entries.
				 * @return Whether the map is empty.
				 */
				bool empty() const noexcept { return m_size == 0; }

				/**
				 * @brief Return the entry count.
				 * @return Entry count.
				 */
				size_type size() const noexcept { return m_size; }

				/**
				 * @brief Destroy every node.
				 */
				void clear() noexcept;

				/**
				 * @brief Exchange trees. Does not allocate.
				 * @param other Map to exchange with.
				 */
				void swap(Map& other) noexcept;

				/**
				 * @brief Insert a copy of an entry.
				 * @param entry Entry to insert.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert(const value_type& entry);

				/**
				 * @brief Insert a moved entry.
				 * @param entry Entry to insert.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert(value_type&& entry);

				/**
				 * @brief Insert an entry constructed from a compatible value.
				 * @tparam P Source type. @ref value_type must be constructible from it.
				 * @param entry Source entry.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class P>
				requires Type::ConstructibleFrom<value_type, P> && (!Type::SameAs<std::remove_cvref_t<P>, value_type>)
				insert_result insert(P&& entry);

				/**
				 * @brief Construct an entry in a new node if the key is absent.
				 * @tparam Args Mapped constructor argument types.
				 * @param key Lookup key.
				 * @param args Arguments forwarded to V. None value-initializes V.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				insert_result try_emplace(const K& key, Args&&... args);

				/**
				 * @brief Construct an entry from arguments forwarded to @ref value_type.
				 * @tparam Args Entry constructor argument types.
				 * @param args Arguments forwarded to @ref value_type.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				insert_result emplace(Args&&... args);

				/**
				 * @brief Insert or replace the mapped value.
				 * @param key Lookup key.
				 * @param value Replacement value.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert_or_assign(const K& key, const V& value);

				/**
				 * @brief Erase the entry at an iterator.
				 * @param position Entry to erase.
				 * @return Iterator following the erased entry.
				 */
				iterator erase(iterator position) noexcept;

				/**
				 * @brief Erase the entry at a read-only iterator.
				 * @param position Entry to erase.
				 * @return Iterator following the erased entry.
				 */
				iterator erase(const_iterator position) noexcept;

				/**
				 * @brief Erase a half-open range.
				 * @param first First entry to erase.
				 * @param last Past-the-end entry.
				 * @return Iterator following the erased range.
				 */
				iterator erase(const_iterator first, const_iterator last) noexcept;

				/**
				 * @brief Erase the entry with a key.
				 * @param key Lookup key.
				 * @return Number of erased entries, zero or one.
				 */
				size_type erase(const K& key) noexcept;

				/**
				 * @brief Find an entry.
				 * @param key Lookup key.
				 * @return Iterator to the entry, or end.
				 */
				iterator find(const K& key) noexcept;

				/**
				 * @brief Find an entry.
				 * @param key Lookup key.
				 * @return Iterator to the entry, or end.
				 */
				const_iterator find(const K& key) const noexcept;

				/**
				 * @brief Test whether a key is present.
				 * @param key Lookup key.
				 * @return Whether the key is present.
				 */
				bool contains(const K& key) const noexcept;

				/**
				 * @brief Return the number of entries with a key.
				 * @param key Lookup key.
				 * @return Zero or one.
				 */
				size_type count(const K& key) const noexcept;

				/**
				 * @brief Return the first entry not less than a key.
				 * @param key Lookup key.
				 * @return Iterator to the lower bound, or end.
				 */
				iterator lower_bound(const K& key) noexcept;

				/**
				 * @brief Return the first entry not less than a key.
				 * @param key Lookup key.
				 * @return Iterator to the lower bound, or end.
				 */
				const_iterator lower_bound(const K& key) const noexcept;

				/**
				 * @brief Return the first entry greater than a key.
				 * @param key Lookup key.
				 * @return Iterator to the upper bound, or end.
				 */
				iterator upper_bound(const K& key) noexcept;

				/**
				 * @brief Return the first entry greater than a key.
				 * @param key Lookup key.
				 * @return Iterator to the upper bound, or end.
				 */
				const_iterator upper_bound(const K& key) const noexcept;

				/**
				 * @brief Return the equal range of a key.
				 * @param key Lookup key.
				 * @return Lower and upper bound. They are equal when the key is absent.
				 */
				equal_range_result equal_range(const K& key) noexcept;

				/**
				 * @brief Return the equal range of a key.
				 * @param key Lookup key.
				 * @return Lower and upper bound. They are equal when the key is absent.
				 */
				const_equal_range_result equal_range(const K& key) const noexcept;

				/**
				 * @brief Return the mapped value, inserting a value-initialized one if absent.
				 * @param key Lookup key.
				 * @return Mapped value. Valid until the entry is erased.
				 * @throws AllocationError The node could not be allocated.
				 */
				V& operator[](const K& key);

				/**
				 * @brief Return the mapped value.
				 * @param key Lookup key.
				 * @return Mapped value. Valid until the entry is erased.
				 * @throws OutOfBoundsError The key is absent. The message includes the key when it can be printed.
				 */
				V& at(const K& key);

				/**
				 * @brief Return the mapped value.
				 * @param key Lookup key.
				 * @return Mapped value. Valid until the entry is erased.
				 * @throws OutOfBoundsError The key is absent. The message includes the key when it can be printed.
				 */
				const V& at(const K& key) const;

				/**
				 * @brief Compare entries in order.
				 * @param other Map to compare.
				 * @return Whether both maps contain the same entries.
				 */
				bool operator==(const Map& other) const requires Type::EqualityComparable<K> && Type::EqualityComparable<V>;

				/**
				 * @brief Copy the entries into caller-owned STL storage.
				 * @return A `std::map` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::map<K, V, Compare>() const {
					std::map<K, V, Compare> exported(m_compare);
					for (const value_type& entry : *this)
						exported.emplace(entry.first, entry.second);
					return exported;
				}

			private:
				/**
				 * @brief One ordered entry and its tree links.
				 */
				struct Node {
					value_type Entry; ///< Stored entry.
					Node* Parent; ///< Parent, or the header for the root.
					Node* Left; ///< Left child, or null.
					Node* Right; ///< Right child, or null.
					bool Red; ///< Whether the node is red.
				};

				/**
				 * @brief Allocate the header sentinel.
				 * @throws AllocationError The header could not be allocated.
				 */
				void CreateHeader();

				/**
				 * @brief Release the header sentinel.
				 */
				void DestroyHeader() noexcept;

				/**
				 * @brief Allocate a node and construct its entry.
				 * @tparam Args Entry constructor argument types.
				 * @param args Arguments forwarded to the entry.
				 * @return New node.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				Node* MakeNode(Args&&... args);

				/**
				 * @brief Destroy one node. Children are not visited.
				 * @param node Node to release.
				 */
				void DestroyNode(Node* node) noexcept;

				/**
				 * @brief Destroy a subtree.
				 * @param node Subtree root.
				 */
				void DestroyTree(Node* node) noexcept;

				/**
				 * @brief Clone a subtree.
				 * @param source Subtree root.
				 * @param parent Parent of the clone.
				 * @return Clone root.
				 * @throws AllocationError A node could not be allocated.
				 */
				Node* Clone(const Node* source, Node* parent);

				/**
				 * @brief Find the node for a key.
				 * @param key Lookup key.
				 * @return Node, or null.
				 */
				Node* FindNode(const K& key) const noexcept;

				/**
				 * @brief Link a new node and rebalance.
				 * @param node Node already constructed.
				 * @param parent Parent, or the header when the tree is empty.
				 * @param left Whether the node is the left child.
				 */
				void InsertNode(Node* node, Node* parent, bool left) noexcept;

				/**
				 * @brief Unlink a node and rebalance.
				 * @param node Node to erase.
				 */
				void EraseNode(Node* node) noexcept;

				/**
				 * @brief Return the leftmost node, or the header when empty.
				 * @return Leftmost node.
				 */
				Node* Leftmost() const noexcept;

				/**
				 * @brief Rotate a node left.
				 * @param node Pivot.
				 */
				void RotateLeft(Node* node) noexcept;

				/**
				 * @brief Rotate a node right.
				 * @param node Pivot.
				 */
				void RotateRight(Node* node) noexcept;

				/**
				 * @brief Format the missing-key error.
				 * @param key Absent key.
				 * @return Error message.
				 */
				static std::string AbsentKey(const K& key);

				key_compare m_compare; ///< Key ordering.
				Node* m_header; ///< Sentinel. Its parent is the root.
				size_type m_size; ///< Entry count.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes a map of already safe entries.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Key ordering.
		 */
		template<SafeValue K, SafeValue V, class Compare>
		requires IsSafe<K>::value && IsSafe<V>::value
		struct IsSafe<Safe::Map<K, V, Compare>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from a map entry.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Key ordering.
		 */
		template<SafeValue K, SafeValue V, class Compare>
		requires MaybeSafe<K> || MaybeSafe<V>
		struct IsMaybeSafe<Safe::Map<K, V, Compare>>: std::true_type {};

		/**
		 * @brief Admits a Safe map as a collection value.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Key ordering.
		 */
		template<SafeValue K, SafeValue V, class Compare>
		struct IsSafeValue<Safe::Map<K, V, Compare>>: std::true_type {};
	}
}

#include <StormByte/safe/map.txx>
