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
#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <ranges>
#include <unordered_map>
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
		 * @class UnorderedMap
		 * @brief Unique-key hash map stored on Base's heap.
		 * @tparam K Safe key type. It must be hashable by @p Hash.
		 * @tparam V Safe mapped type.
		 * @tparam Hash Cross-module hash. A stateless hash does not allocate. A stateful one is stored in the map and must itself be Safe.
		 * @tparam KeyEqual Key equality. A stateless predicate does not allocate. A stateful one is stored in the map and must itself be Safe.
		 *
		 * Nodes and the bucket array are allocated with @ref Heap::Allocate. There is no allocator. Iterators are forward and point at Base nodes. A cursor keeps the map so it can leave an empty bucket. Dereference returns a real @ref Pair reference. Public results are @ref Pair, never `std::pair`. @ref node_type owns a Base node. It is not `std::unordered_map::node_type`. @ref extract unlinks that node. It does not copy it. The default hash is @ref Hash, not `std::hash`. A conversion to or from `std::unordered_map<K, V>` copies the entries. It does not reuse the STL hasher.
		 */
		template<Type::SafeValue K, Type::SafeValue V, class Hash = Hash<K>, class KeyEqual = std::equal_to<K>>
		class STORMBYTE_PUBLIC_TYPE UnorderedMap final {
			private:
				struct Node;

			public:
				using key_type = K; ///< Key type.
				using mapped_type = V; ///< Mapped type.
				using value_type = Pair<const K, V>; ///< Entry type. The key is immutable.
				using hasher = Hash; ///< Key hash.
				using key_equal = KeyEqual; ///< Key equality.
				using size_type = std::size_t; ///< Entry count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = value_type&; ///< Mutable entry reference.
				using const_reference = const value_type&; ///< Read-only entry reference.

				/**
				 * @class BasicIterator
				 * @brief Forward cursor over Base-owned hash nodes.
				 * @tparam IsConst Whether dereference is read-only.
				 *
				 * The cursor stores one node pointer and the map. Copying it does not allocate and does not take ownership. Order is bucket order, not insertion order. Rehash invalidates every cursor.
				 */
				template<bool IsConst>
				class BasicIterator final {
						using NodeType = std::conditional_t<IsConst, const Node, Node>;
						using MapType = std::conditional_t<IsConst, const UnorderedMap, UnorderedMap>;

					public:
						using iterator_category = std::forward_iterator_tag; ///< Iterator category.
						using iterator_concept = std::forward_iterator_tag; ///< C++20 iterator concept.
						using value_type = UnorderedMap::value_type; ///< Entry type.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using reference = std::conditional_t<IsConst, const value_type&, value_type&>; ///< Entry reference.
						using pointer = std::conditional_t<IsConst, const value_type*, value_type*>; ///< Entry pointer.
						using StormByteSafeCursor = void; ///< Base cursor mark. Copying the cursor copies one node pointer.

						/**
						 * @brief Construct a singular iterator.
						 */
						BasicIterator() noexcept: m_node(nullptr), m_map(nullptr) {}

						/**
						 * @brief Convert a mutable iterator to a const iterator.
						 * @param other Mutable iterator.
						 */
						template<bool OtherConst>
						requires IsConst && (!OtherConst)
						BasicIterator(const BasicIterator<OtherConst>& other) noexcept: m_node(other.m_node), m_map(other.m_map) {}

						/**
						 * @brief Return the entry stored in the node.
						 * @return Entry reference. Valid until the node is erased or the map is rehashed.
						 */
						reference operator*() const noexcept { return m_node->Entry; }

						/**
						 * @brief Access the entry stored in the node.
						 * @return Entry pointer. Valid until the node is erased or the map is rehashed.
						 */
						pointer operator->() const noexcept { return &m_node->Entry; }

						/**
						 * @brief Advance to the next occupied node, skipping empty buckets.
						 * @return This iterator.
						 */
						BasicIterator& operator++() noexcept;

						/**
						 * @brief Advance to the next occupied node, skipping empty buckets.
						 * @return Previous iterator value.
						 */
						BasicIterator operator++(int) noexcept;

						/**
						 * @brief Compare node identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same node.
						 */
						template<bool OtherConst>
						bool operator==(const BasicIterator<OtherConst>& other) const noexcept { return m_node == other.m_node; }

					private:
						friend class UnorderedMap;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind the cursor to a node of @p map. A null node is end.
						 * @param node Node, or null.
						 * @param map Map that owns the node.
						 */
						explicit BasicIterator(NodeType* node, MapType* map) noexcept: m_node(node), m_map(map) {}

						NodeType* m_node; ///< Current node, or null at end.
						MapType* m_map; ///< Map used to cross buckets. Not owned.
				};

				using iterator = BasicIterator<false>; ///< Mutable iterator.
				using const_iterator = BasicIterator<true>; ///< Read-only iterator.
				using insert_result = Pair<iterator, bool>; ///< Iterator to the entry and whether it was inserted.

				/**
				 * @class node_type
				 * @brief Owning handle to one extracted Base node.
				 *
				 * Not `std::unordered_map::node_type`. The node was allocated with @ref Heap::Allocate and is released with @ref Heap::Free. @ref extract transfers this node. It does not allocate another.
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
						friend class UnorderedMap;

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
				UnorderedMap() noexcept;

				/**
				 * @brief Construct an empty map with a bucket count and predicates.
				 * @param buckets Initial bucket count. Rounded up to a power of two. Zero selects the default.
				 * @param hash Key hash.
				 * @param equal Key equality.
				 * @throws AllocationError The bucket array could not be allocated.
				 */
				explicit UnorderedMap(size_type buckets, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual());

				/**
				 * @brief Copy every node into new Base blocks.
				 * @param other Source.
				 * @throws AllocationError A block could not be allocated.
				 */
				UnorderedMap(const UnorderedMap& other);

				/**
				 * @brief Take the table. @p other is left empty.
				 * @param other Source.
				 */
				UnorderedMap(UnorderedMap&& other) noexcept;

				/**
				 * @brief Destroy every node and the bucket array through Base.
				 */
				~UnorderedMap() noexcept;

				/**
				 * @brief Copy-assign. The previous table is released only after the copy exists.
				 * @param other Source.
				 * @return This map.
				 * @throws AllocationError A block could not be allocated.
				 */
				UnorderedMap& operator=(const UnorderedMap& other);

				/**
				 * @brief Move-assign. @p other is left empty.
				 * @param other Source.
				 * @return This map.
				 */
				UnorderedMap& operator=(UnorderedMap&& other) noexcept;

				/**
				 * @brief Copy a caller-owned STL map. Its state is unchanged. Its hasher is not reused.
				 * @param other Source.
				 * @throws AllocationError A block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit UnorderedMap(const std::unordered_map<K, V>& other): UnorderedMap(other.bucket_count()) {
					for (const auto& entry : other)
						insert(value_type(entry.first, entry.second));
				}

				/**
				 * @brief Move elements from a caller-owned STL map and leave it empty. Its hasher is not reused.
				 * @param other Source. Cleared before this function returns.
				 * @throws AllocationError A block could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit UnorderedMap(std::unordered_map<K, V>&& other): UnorderedMap(other.bucket_count()) {
					for (auto& entry : other)
						insert(value_type(entry.first, std::move(entry.second)));
					other.clear();
				}

				/**
				 * @brief Copy the entries of @p range.
				 * @tparam R Input range of @ref value_type.
				 * @param range Source. Instantiated in the caller.
				 * @throws AllocationError A block could not be allocated.
				 */
				template<std::ranges::input_range R>
				requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
				STORMBYTE_FORCE_INLINE explicit UnorderedMap(std::from_range_t, R&& range): UnorderedMap() {
					insert_range(std::forward<R>(range));
				}

				/**
				 * @brief Insert every entry of an initializer list.
				 * @param values Entries.
				 * @throws AllocationError A block could not be allocated.
				 */
				UnorderedMap(std::initializer_list<value_type> values);

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values Entries.
				 * @return This map.
				 * @throws AllocationError A block could not be allocated.
				 */
				UnorderedMap& operator=(std::initializer_list<value_type> values);

				/**
				 * @brief Return the first occupied node.
				 * @return Mutable iterator, or @ref end when empty.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return the past-the-end iterator.
				 * @return Mutable iterator.
				 */
				iterator end() noexcept { return iterator(nullptr, this); }

				/**
				 * @brief Return the first occupied node.
				 * @return Read-only iterator, or @ref end when empty.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return the past-the-end iterator.
				 * @return Read-only iterator.
				 */
				const_iterator end() const noexcept { return const_iterator(nullptr, this); }

				/**
				 * @brief Return the first occupied node.
				 * @return Read-only iterator.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the past-the-end iterator.
				 * @return Read-only iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Test whether the map holds no entries.
				 * @return Whether the map is empty.
				 */
				bool empty() const noexcept { return m_size == 0; }

				/**
				 * @brief Return the entry count.
				 * @return Entry count.
				 */
				size_type size() const noexcept { return m_size; }

				/**
				 * @brief Return the maximum entry count this map can report.
				 * @return Maximum count.
				 */
				size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

				/**
				 * @brief Destroy every entry. The bucket array stays.
				 */
				void clear() noexcept;

				/**
				 * @brief Insert a copy of @p value when the key is absent.
				 * @param value Entry.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError A node could not be allocated.
				 */
				insert_result insert(const value_type& value);

				/**
				 * @brief Insert a moved @p value when the key is absent.
				 * @param value Entry.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError A node could not be allocated.
				 */
				insert_result insert(value_type&& value);

				/**
				 * @brief Insert every entry of an initializer list.
				 * @param values Entries.
				 * @throws AllocationError A node could not be allocated.
				 */
				void insert(std::initializer_list<value_type> values);

				/**
				 * @brief Insert every entry of @p range.
				 * @tparam R Input range of @ref value_type.
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
				 * @brief Construct an entry in place when the key is absent.
				 * @tparam Args Constructor argument types of @ref value_type.
				 * @param args Arguments forwarded to @ref value_type.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class... Args>
				insert_result emplace(Args&&... args);

				/**
				 * @brief Construct the mapped value in place when the key is absent.
				 * @tparam Args Mapped constructor argument types.
				 * @param key Key.
				 * @param args Arguments forwarded to @p V.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class... Args>
				insert_result try_emplace(const K& key, Args&&... args);

				/**
				 * @brief Insert or replace the mapped value.
				 * @param key Key.
				 * @param mapped Mapped value.
				 * @return Iterator to the entry and whether it was inserted.
				 * @throws AllocationError A node could not be allocated.
				 */
				insert_result insert_or_assign(const K& key, V mapped);

				/**
				 * @brief Erase the entry at @p position.
				 * @param position Iterator to an entry. Not @ref end.
				 * @return Iterator to the following entry, across buckets.
				 */
				iterator erase(const_iterator position);

				/**
				 * @brief Erase the entry with @p key.
				 * @param key Key.
				 * @return Number of erased entries, zero or one.
				 */
				size_type erase(const K& key);

				/**
				 * @brief Unlink the entry at @p position. The node is not copied.
				 * @param position Iterator to an entry. Not @ref end.
				 * @return Owning handle.
				 */
				node_type extract(const_iterator position);

				/**
				 * @brief Unlink the entry with @p key. The node is not copied.
				 * @param key Key.
				 * @return Owning handle, empty when the key is absent.
				 */
				node_type extract(const K& key);

				/**
				 * @brief Link an extracted node. A duplicate key is returned in the result.
				 * @param node Handle. Left empty when inserted.
				 * @return Insertion result.
				 * @throws AllocationError The bucket array could not grow.
				 */
				insert_return_type insert(node_type&& node);

				/**
				 * @brief Move entries whose keys are absent from @p other.
				 * @param other Source. Extracted nodes are unlinked, not copied.
				 * @throws AllocationError The bucket array could not grow.
				 */
				void merge(UnorderedMap& other);

				/**
				 * @brief Exchange tables. Does not allocate.
				 * @param other Map to exchange with.
				 */
				void swap(UnorderedMap& other) noexcept;

				/**
				 * @brief Return the mapped value.
				 * @param key Key.
				 * @return Mapped value. Valid until the entry is erased or the map is rehashed.
				 * @throws OutOfBoundsError The key is absent.
				 */
				V& at(const K& key);

				/**
				 * @brief Return the mapped value.
				 * @param key Key.
				 * @return Mapped value. Valid until the entry is erased or the map is rehashed.
				 * @throws OutOfBoundsError The key is absent.
				 */
				const V& at(const K& key) const;

				/**
				 * @brief Return the mapped value, inserting a value-initialized one when absent.
				 * @param key Key.
				 * @return Mapped value. Valid until the entry is erased or the map is rehashed.
				 * @throws AllocationError A node could not be allocated.
				 */
				V& operator[](const K& key);

				/**
				 * @brief Find an entry.
				 * @param key Key.
				 * @return Iterator to the entry, or @ref end.
				 */
				iterator find(const K& key);

				/**
				 * @brief Find an entry.
				 * @param key Key.
				 * @return Read-only iterator to the entry, or @ref end.
				 */
				const_iterator find(const K& key) const;

				/**
				 * @brief Count entries with @p key.
				 * @param key Key.
				 * @return Zero or one.
				 */
				size_type count(const K& key) const;

				/**
				 * @brief Test whether @p key is present.
				 * @param key Key.
				 * @return Whether the key is present.
				 */
				bool contains(const K& key) const;

				/**
				 * @brief Return the bucket count.
				 * @return Bucket count. Zero before the first allocation.
				 */
				size_type bucket_count() const noexcept { return m_bucket_count; }

				/**
				 * @brief Return the number of entries in @p index.
				 * @param index Bucket index. Not checked.
				 * @return Entry count in that bucket.
				 */
				size_type bucket_size(size_type index) const;

				/**
				 * @brief Return the bucket of @p key.
				 * @param key Key.
				 * @return Bucket index. Zero when the table has no buckets.
				 */
				size_type bucket(const K& key) const;

				/**
				 * @brief Return the current load factor.
				 * @return Entry count divided by bucket count, or zero when there are no buckets.
				 */
				float load_factor() const noexcept;

				/**
				 * @brief Return the load factor that triggers a rehash.
				 * @return Maximum load factor.
				 */
				float max_load_factor() const noexcept { return m_max_load; }

				/**
				 * @brief Set the load factor that triggers a rehash.
				 * @param factor Maximum load factor. Not checked.
				 */
				void max_load_factor(float factor) noexcept { m_max_load = factor; }

				/**
				 * @brief Replace the bucket array. Entries are relinked, not copied.
				 * @param buckets New bucket count. Rounded up to a power of two.
				 * @throws AllocationError The new bucket array could not be allocated.
				 */
				void rehash(size_type buckets);

				/**
				 * @brief Reserve room for @p count entries without exceeding the load factor.
				 * @param count Entry count to accommodate.
				 * @throws AllocationError The bucket array could not be allocated.
				 */
				void reserve(size_type count);

				/**
				 * @brief Return the stored hash.
				 * @return Key hash.
				 */
				Hash hash_function() const { return m_hash; }

				/**
				 * @brief Return the stored equality predicate.
				 * @return Key equality.
				 */
				KeyEqual key_eq() const { return m_equal; }

				/**
				 * @brief Compare entries by key and mapped value. Order does not matter.
				 * @param other Map to compare.
				 * @return Whether both maps hold the same entries.
				 */
				bool operator==(const UnorderedMap& other) const;

				/**
				 * @brief Copy the entries into caller-owned STL storage. The STL hasher is `std::hash`.
				 * @return An `std::unordered_map` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::unordered_map<K, V>() const {
					std::unordered_map<K, V> copied(m_bucket_count);
					for (const value_type& entry : *this)
						copied.emplace(entry.first, entry.second);
					return copied;
				}

			private:
				/**
				 * @brief One chained entry. The link is not part of the public pair.
				 */
				struct Node {
					Node* Next; ///< Next node in this bucket, or null.
					std::size_t Code; ///< Cached hash.
					value_type Entry; ///< Public entry.
				};

				/**
				 * @brief Round @p count up to a power of two. Zero stays zero.
				 * @param count Requested count.
				 * @return Power of two, at least eight when @p count is not zero.
				 */
				static size_type BucketCount(size_type count) noexcept;

				/**
				 * @brief Allocate a zeroed bucket array.
				 * @param count Bucket count.
				 * @return Bucket array. Null when @p count is zero.
				 * @throws AllocationError The block could not be allocated.
				 */
				Node** AllocateBuckets(size_type count);

				/**
				 * @brief Release the bucket array. Nodes are not released.
				 */
				void FreeBuckets() noexcept;

				/**
				 * @brief Destroy and release every node, then the bucket array.
				 */
				void Destroy() noexcept;

				/**
				 * @brief Find the node of @p key.
				 * @param key Key.
				 * @param code Hash of @p key.
				 * @return Node, or null.
				 */
				Node* FindNode(const K& key, std::size_t code) const noexcept;

				/**
				 * @brief Return the first node in a bucket after @p slot.
				 * @param slot Bucket just visited. The search starts at @p slot + 1.
				 * @return Node, or null.
				 */
				Node* NextBucket(size_type slot) const noexcept;

				/**
				 * @brief Link a node that is known to be absent.
				 * @param node Node. Not null.
				 * @throws AllocationError The bucket array could not grow.
				 */
				void Link(Node* node);

				/**
				 * @brief Unlink @p node. The node is not destroyed.
				 * @param node Node. Not null.
				 * @return Following node in the same bucket, or null.
				 */
				Node* Unlink(Node* node) noexcept;

				/**
				 * @brief Grow when the next insert would pass the load factor.
				 * @throws AllocationError The bucket array could not be allocated.
				 */
				void Ensure(size_type extra);

				Node** m_buckets = nullptr; ///< Bucket heads, or null.
				size_type m_bucket_count = 0; ///< Bucket count.
				size_type m_size = 0; ///< Entry count.
				float m_max_load = 1.0f; ///< Load factor that triggers a rehash.
				Hash m_hash{}; ///< Key hash.
				KeyEqual m_equal{}; ///< Key equality.
		};

		/**
		 * @brief Exchange two maps.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Hash Key hash.
		 * @tparam KeyEqual Key equality.
		 * @param left First map.
		 * @param right Second map.
		 */
		template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
		void swap(UnorderedMap<K, V, Hash, KeyEqual>& left, UnorderedMap<K, V, Hash, KeyEqual>& right) noexcept { left.swap(right); }
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Concepts and traits used to constrain Safe components.
	 */
	namespace Type {
		/**
		 * @brief A map of Safe entries is Safe.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Hash Key hash.
		 * @tparam KeyEqual Key equality.
		 */
		template<SafeValue K, SafeValue V, class Hash, class KeyEqual>
		requires IsSafe<K>::value && IsSafe<V>::value
		struct IsSafe<Safe::UnorderedMap<K, V, Hash, KeyEqual>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from a map entry.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Hash Key hash.
		 * @tparam KeyEqual Key equality.
		 */
		template<SafeValue K, SafeValue V, class Hash, class KeyEqual>
		requires MaybeSafe<K> || MaybeSafe<V>
		struct IsMaybeSafe<Safe::UnorderedMap<K, V, Hash, KeyEqual>>: std::true_type {};

		/**
		 * @brief Admits a Safe unordered map as a collection value.
		 * @tparam K Safe key.
		 * @tparam V Safe mapped type.
		 * @tparam Hash Key hash.
		 * @tparam KeyEqual Key equality.
		 */
		template<SafeValue K, SafeValue V, class Hash, class KeyEqual>
		struct IsSafeValue<Safe::UnorderedMap<K, V, Hash, KeyEqual>>: std::true_type {};
	}
}

#include <StormByte/safe/unordered_map.txx>
