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
#include <StormByte/safe/pair.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
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
		 * @class Map
		 * @brief Ordered unique-key map stored on Base's heap.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Strict weak ordering. A stateless comparator does not allocate. A stateful one is stored in the map object and must itself be Safe.
		 *
		 * Nodes are allocated with @ref Heap::Allocate. Iterators point at those nodes. Dereference returns a real @ref Pair reference, so the map is a bidirectional range. Public results are @ref Pair, never `std::pair`. @ref node_type owns a Base node. It is not `std::map::node_type`. @ref extract unlinks that node. It does not copy it. There is no allocator.
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
				 * @class value_compare
				 * @brief Orders entries by key, using the map comparator.
				 */
				class value_compare {
						friend class Map;

					public:
						/**
						 * @brief Compare two entries by key.
						 * @param left Left entry.
						 * @param right Right entry.
						 * @return Whether @p left precedes @p right.
						 */
						bool operator()(const value_type& left, const value_type& right) const {
							return m_compare(left.first, right.first);
						}

					private:
						/**
						 * @brief Store the key comparator.
						 * @param compare Key ordering.
						 */
						explicit value_compare(Compare compare): m_compare(compare) {}

						Compare m_compare; ///< Key ordering.
				};

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
				using reverse_iterator = std::reverse_iterator<iterator>; ///< Mutable reverse iterator.
				using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< Read-only reverse iterator.
				using insert_result = Pair<iterator, bool>; ///< Iterator to the entry and whether it was inserted.
				using equal_range_result = Pair<iterator, iterator>; ///< Lower and upper bound.
				using const_equal_range_result = Pair<const_iterator, const_iterator>; ///< Read-only lower and upper bound.

				/**
				 * @class node_type
				 * @brief Owning handle to one extracted Base node.
				 *
				 * Not `std::map::node_type`. The node was allocated with @ref Heap::Allocate and is released with @ref Heap::Free. @ref extract transfers this node. It does not allocate another.
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
						node_type(node_type&& other) noexcept: m_node(other.m_node) { other.m_node = nullptr; }

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
						~node_type() { reset(); }

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
						 * @brief Return the owned key.
						 * @return Key. Valid until the handle is emptied.
						 */
						const K& key() const { return m_node->Entry.first; }

						/**
						 * @brief Return the owned mapped value.
						 * @return Mapped value. Valid until the handle is emptied.
						 */
						V& mapped() { return m_node->Entry.second; }

						/**
						 * @brief Return the owned mapped value.
						 * @return Mapped value. Valid until the handle is emptied.
						 */
						const V& mapped() const { return m_node->Entry.second; }

						/**
						 * @brief Exchange owned nodes.
						 * @param other Other handle.
						 */
						void swap(node_type& other) noexcept { std::swap(m_node, other.m_node); }

					private:
						friend class Map;

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
							m_node->Entry.~value_type();
							Heap::Free(m_node);
							m_node = nullptr;
						}

						Node* m_node = nullptr; ///< Owned node, or null.
				};

				/**
				 * @brief Result of inserting an extracted node.
				 */
				struct insert_return_type {
					iterator position; ///< Entry, or end when the node was not inserted and is empty.
					bool inserted; ///< Whether the node was linked.
					node_type node; ///< Returned node when the key was already present.
				};

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
				STORMBYTE_FORCE_INLINE explicit Map(const std::map<K, V, Compare>& other): Map(other.key_comp()) {
					for (const auto& entry : other)
						insert(value_type(entry.first, entry.second));
				}

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
				 * @brief Copy the entries of @p range.
				 * @tparam R Input range of @ref value_type.
				 * @param range Source. Instantiated in the caller.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
				STORMBYTE_FORCE_INLINE explicit Map(std::from_range_t, R&& range): Map() {
					insert_range(std::forward<R>(range));
				}

				/**
				 * @brief Insert every entry of an initializer list.
				 * @param values Entries.
				 * @throws AllocationError A node could not be allocated.
				 */
				Map(std::initializer_list<value_type> values);

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
				STORMBYTE_FORCE_INLINE Map& operator=(const std::map<K, V, Compare>& other) {
					Map replacement(other);
					swap(replacement);
					return *this;
				}

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
				 * @brief Replace the contents with an initializer list.
				 * @param values Entries.
				 * @return This map.
				 * @throws AllocationError A node could not be allocated.
				 */
				Map& operator=(std::initializer_list<value_type> values);

				/**
				 * @brief Return the key ordering.
				 * @return Comparator.
				 */
				key_compare key_comp() const { return m_compare; }

				/**
				 * @brief Return an entry comparator that uses the key ordering.
				 * @return Entry comparator.
				 */
				value_compare value_comp() const { return value_compare(m_compare); }

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
				 * @brief Return a reverse iterator to the last entry.
				 * @return Reverse iterator.
				 */
				reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

				/**
				 * @brief Return the reverse end iterator.
				 * @return Reverse end iterator.
				 */
				reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

				/**
				 * @brief Return a read-only reverse iterator to the last entry.
				 * @return Reverse iterator.
				 */
				const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

				/**
				 * @brief Return a read-only reverse iterator to the last entry.
				 * @return Reverse iterator.
				 */
				const_reverse_iterator crbegin() const noexcept { return rbegin(); }

				/**
				 * @brief Return the read-only reverse end iterator.
				 * @return Reverse end iterator.
				 */
				const_reverse_iterator crend() const noexcept { return rend(); }

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
				 * @brief Return the largest representable entry count.
				 * @return Maximum size.
				 */
				size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(Node); }

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
				 * @brief Insert a copy of an entry, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param entry Entry to insert.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert(const_iterator hint, const value_type& entry);

				/**
				 * @brief Insert a moved entry, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param entry Entry to insert.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert(const_iterator hint, value_type&& entry);

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
				 * @brief Insert every entry in `[first, last)`.
				 * @tparam InputIt Input iterator of entries.
				 * @param first Start.
				 * @param last End.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class InputIt>
				STORMBYTE_FORCE_INLINE void insert(InputIt first, InputIt last) {
					for (; first != last; ++first)
						insert(value_type(*first));
				}

				/**
				 * @brief Insert every entry of an initializer list.
				 * @param values Entries.
				 * @throws AllocationError A node could not be allocated.
				 */
				void insert(std::initializer_list<value_type> values);

				/**
				 * @brief Insert a copy of @p range.
				 * @tparam R Input range of entries.
				 * @param range Source. Instantiated in the caller.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
				STORMBYTE_FORCE_INLINE void insert_range(R&& range) {
					for (auto&& entry : range)
						insert(value_type(std::forward<decltype(entry)>(entry)));
				}

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
				 * @brief Construct an entry in a new node if the key is absent.
				 * @tparam Args Mapped constructor argument types.
				 * @param key Lookup key, moved when inserted.
				 * @param args Arguments forwarded to V.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				insert_result try_emplace(K&& key, Args&&... args);

				/**
				 * @brief Construct an entry if the key is absent, using @p hint when the neighbouring keys confirm it.
				 * @tparam Args Mapped constructor argument types.
				 * @param hint Suggested position.
				 * @param key Lookup key.
				 * @param args Arguments forwarded to V.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				iterator try_emplace(const_iterator hint, const K& key, Args&&... args);

				/**
				 * @brief Construct an entry if the key is absent, using @p hint when the neighbouring keys confirm it.
				 * @tparam Args Mapped constructor argument types.
				 * @param hint Suggested position.
				 * @param key Lookup key, moved when inserted.
				 * @param args Arguments forwarded to V.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				iterator try_emplace(const_iterator hint, K&& key, Args&&... args);

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
				 * @brief Construct an entry from arguments, using @p hint when the neighbouring keys confirm it.
				 * @tparam Args Entry constructor argument types.
				 * @param hint Suggested position.
				 * @param args Arguments forwarded to @ref value_type.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				iterator emplace_hint(const_iterator hint, Args&&... args);

				/**
				 * @brief Insert or replace the mapped value.
				 * @param key Lookup key.
				 * @param value Replacement value.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert_or_assign(const K& key, const V& value);

				/**
				 * @brief Insert or replace the mapped value.
				 * @param key Lookup key.
				 * @param value Replacement value, moved when assigned.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert_or_assign(const K& key, V&& value);

				/**
				 * @brief Insert or replace the mapped value.
				 * @param key Lookup key, moved when inserted.
				 * @param value Replacement value.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert_or_assign(K&& key, const V& value);

				/**
				 * @brief Insert or replace the mapped value.
				 * @param key Lookup key, moved when inserted.
				 * @param value Replacement value, moved when assigned.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError The node could not be allocated.
				 */
				insert_result insert_or_assign(K&& key, V&& value);

				/**
				 * @brief Insert or replace the mapped value, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param key Lookup key.
				 * @param value Replacement value.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert_or_assign(const_iterator hint, const K& key, const V& value);

				/**
				 * @brief Insert or replace the mapped value, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param key Lookup key.
				 * @param value Replacement value, moved when assigned.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert_or_assign(const_iterator hint, const K& key, V&& value);

				/**
				 * @brief Insert or replace the mapped value, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param key Lookup key, moved when inserted.
				 * @param value Replacement value.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert_or_assign(const_iterator hint, K&& key, const V& value);

				/**
				 * @brief Insert or replace the mapped value, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param key Lookup key, moved when inserted.
				 * @param value Replacement value, moved when assigned.
				 * @return Iterator to the entry.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert_or_assign(const_iterator hint, K&& key, V&& value);

				/**
				 * @brief Insert an extracted node if its key is absent. The same node is linked.
				 * @param handle Node. Emptied when inserted.
				 * @return Position, whether it was inserted, and the rejected node.
				 */
				insert_return_type insert(node_type&& handle);

				/**
				 * @brief Insert an extracted node, using @p hint when the neighbouring keys confirm it.
				 * @param hint Suggested position.
				 * @param handle Node. Emptied when inserted.
				 * @return Iterator to the entry, or end when @p handle was empty.
				 */
				iterator insert(const_iterator hint, node_type&& handle);

				/**
				 * @brief Unlink the entry at @p position and return that node.
				 * @param position Entry to extract.
				 * @return Owning handle. Empty when @p position is end.
				 */
				node_type extract(const_iterator position);

				/**
				 * @brief Unlink the entry with @p key and return that node.
				 * @param key Lookup key.
				 * @return Owning handle. Empty when the key is absent.
				 */
				node_type extract(const K& key);

				/**
				 * @brief Move unique keys from @p source into this map. The node is transferred, not copied.
				 * @tparam OtherCompare Source ordering.
				 * @param source Map to drain. Entries whose key is already present stay in @p source.
				 */
				template<class OtherCompare>
				void merge(Map<K, V, OtherCompare>& source);

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
				 * @brief Return the mapped value, inserting a value-initialized one if absent.
				 * @param key Lookup key, moved when inserted.
				 * @return Mapped value. Valid until the entry is erased.
				 * @throws AllocationError The node could not be allocated.
				 */
				V& operator[](K&& key);

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
				 * @brief Order maps lexicographically by key, then mapped value.
				 * @param other Map to compare.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const Map& other) const requires std::three_way_comparable<K> && std::three_way_comparable<V>;

				/**
				 * @brief Order maps lexicographically.
				 * @param other Map to compare.
				 * @return Whether this map precedes @p other.
				 */
				bool operator<(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; };

				/**
				 * @brief Order maps lexicographically.
				 * @param other Map to compare.
				 * @return Whether this map precedes or equals @p other.
				 */
				bool operator<=(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; };

				/**
				 * @brief Order maps lexicographically.
				 * @param other Map to compare.
				 * @return Whether this map follows @p other.
				 */
				bool operator>(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; };

				/**
				 * @brief Order maps lexicographically.
				 * @param other Map to compare.
				 * @return Whether this map follows or equals @p other.
				 */
				bool operator>=(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; };

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
				 * @brief Test whether @p hint is the insertion point of @p key.
				 * @param hint Suggested position.
				 * @param key Lookup key.
				 * @return Whether the neighbouring keys confirm @p hint.
				 */
				bool HintMatches(const_iterator hint, const K& key) const noexcept;

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
				 * @brief Unlink a node and rebalance. The node is not destroyed.
				 * @param node Node to unlink.
				 */
				void Unlink(Node* node) noexcept;

				/**
				 * @brief Unlink a node and rebalance. The node is not destroyed.
				 * @param node Node to unlink.
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
				 * @return Error message. Owned by Base.
				 */
				static String AbsentKey(const K& key);

				key_compare m_compare; ///< Key ordering.
				Node* m_header; ///< Sentinel. Its parent is the root.
				size_type m_size; ///< Entry count.
		};

		/**
		 * @brief Exchange two maps. Does not allocate.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Key ordering.
		 * @param left First map.
		 * @param right Second map.
		 */
		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		inline void swap(Map<K, V, Compare>& left, Map<K, V, Compare>& right) noexcept {
			left.swap(right);
		}
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
