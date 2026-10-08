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
#include <StormByte/safe/unordered_map.hxx>
#include <StormByte/safe/variant.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <unordered_set>
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
		 * @class UnorderedSet
		 * @brief Unique values stored in a Base-heap hash table.
		 * @tparam K Safe value type. It must be hashable by @p Hash.
		 * @tparam Hash Cross-module hash. A stateless hash does not allocate. A stateful one is stored in the set and must itself be Safe.
		 * @tparam KeyEqual Value equality. A stateless predicate does not allocate. A stateful one is stored in the set and must itself be Safe.
		 *
		 * The table is the same Base-heap table as @ref UnorderedMap. Iterators yield the value, not a pair. Order is bucket order. Rehash invalidates every cursor. @ref node_type owns a Base node. It is not `std::unordered_set::node_type`. There is no allocator. The default hash is @ref Hash, not `std::hash`. A conversion to `std::unordered_set` copies the values and does not reuse this hasher.
		 */
		template<Type::SafeValue K, class Hash = Hash<K>, class KeyEqual = std::equal_to<K>>
		class STORMBYTE_PUBLIC_TYPE UnorderedSet final {
			using Storage = UnorderedMap<K, Monostate, Hash, KeyEqual>;

			public:
				using key_type = K; ///< Value and lookup type.
				using value_type = K; ///< Stored value.
				using hasher = Hash; ///< Value hash.
				using key_equal = KeyEqual; ///< Value equality.
				using size_type = std::size_t; ///< Value count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.
				using reference = const K&; ///< Read-only value reference.
				using const_reference = const K&; ///< Read-only value reference.

				/**
				 * @class iterator
				 * @brief Forward cursor. Dereference is the value.
				 */
				class iterator {
					public:
						using iterator_category = std::forward_iterator_tag; ///< Iterator category.
						using iterator_concept = std::forward_iterator_tag; ///< C++20 iterator concept.
						using value_type = K; ///< Stored value.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using pointer = const K*; ///< Pointer to the value.
						using reference = const K&; ///< Reference to the value.
						using StormByteSafeCursor = void; ///< Base cursor mark. Copying the cursor copies one node pointer.

						/**
						 * @brief Construct a singular iterator.
						 */
						iterator() noexcept = default;

						/**
						 * @brief Return the value.
						 * @return Value. Valid until the node is erased or the set is rehashed.
						 */
						reference operator*() const { return m_cursor->first; }

						/**
						 * @brief Return a pointer to the value.
						 * @return Value pointer.
						 */
						pointer operator->() const { return &m_cursor->first; }

						/**
						 * @brief Advance to the next occupied node.
						 * @return This iterator.
						 */
						iterator& operator++() { ++m_cursor; return *this; }

						/**
						 * @brief Advance to the next occupied node.
						 * @return The previous position.
						 */
						iterator operator++(int) { iterator previous = *this; ++*this; return previous; }

						/**
						 * @brief Compare node identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same node.
						 */
						bool operator==(const iterator& other) const noexcept { return m_cursor == other.m_cursor; }

					private:
						friend class UnorderedSet;
						explicit iterator(typename Storage::iterator cursor) noexcept: m_cursor(cursor) {}
						typename Storage::iterator m_cursor; ///< Map cursor. The mapped value is unused.
				};

				/**
				 * @class const_iterator
				 * @brief Read-only forward cursor.
				 */
				class const_iterator {
					public:
						using iterator_category = std::forward_iterator_tag; ///< Iterator category.
						using iterator_concept = std::forward_iterator_tag; ///< C++20 iterator concept.
						using value_type = K; ///< Stored value.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using pointer = const K*; ///< Pointer to the value.
						using reference = const K&; ///< Reference to the value.
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
						 * @brief Return the value.
						 * @return Value. Valid until the node is erased or the set is rehashed.
						 */
						reference operator*() const { return m_cursor->first; }

						/**
						 * @brief Return a pointer to the value.
						 * @return Value pointer.
						 */
						pointer operator->() const { return &m_cursor->first; }

						/**
						 * @brief Advance to the next occupied node.
						 * @return This iterator.
						 */
						const_iterator& operator++() { ++m_cursor; return *this; }

						/**
						 * @brief Advance to the next occupied node.
						 * @return The previous position.
						 */
						const_iterator operator++(int) { const_iterator previous = *this; ++*this; return previous; }

						/**
						 * @brief Compare node identity.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same node.
						 */
						bool operator==(const const_iterator& other) const noexcept { return m_cursor == other.m_cursor; }

					private:
						friend class UnorderedSet;
						explicit const_iterator(typename Storage::const_iterator cursor) noexcept: m_cursor(cursor) {}
						typename Storage::const_iterator m_cursor; ///< Map cursor.
				};

				using insert_result = Pair<iterator, bool>; ///< Iterator to the value and whether it was inserted.

				/**
				 * @class node_type
				 * @brief Owning handle to one extracted Base node.
				 *
				 * Not `std::unordered_set::node_type`. The node was allocated with @ref Heap::Allocate. A node belongs to one hash and equality. @ref merge across a different hash copies the value.
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
						friend class UnorderedSet;
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
				UnorderedSet() noexcept;

				/**
				 * @brief Construct an empty set with a bucket count and predicates.
				 * @param buckets Initial bucket count. Rounded up to a power of two. Zero selects the default.
				 * @param hash Value hash.
				 * @param equal Value equality.
				 * @throws AllocationError The bucket array could not be allocated.
				 */
				explicit UnorderedSet(size_type buckets, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual());

				/**
				 * @brief Copy the values.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				UnorderedSet(const UnorderedSet& other);

				/**
				 * @brief Take the table. @p other is left empty.
				 * @param other Source.
				 */
				UnorderedSet(UnorderedSet&& other) noexcept;

				/**
				 * @brief Destroy the values.
				 */
				~UnorderedSet();

				/**
				 * @brief Copy-assign the values.
				 * @param other Source.
				 * @return This set.
				 * @throws AllocationError A node could not be allocated.
				 */
				UnorderedSet& operator=(const UnorderedSet& other);

				/**
				 * @brief Move-assign the table. @p other is left empty.
				 * @param other Source.
				 * @return This set.
				 */
				UnorderedSet& operator=(UnorderedSet&& other) noexcept;

				/**
				 * @brief Replace the contents with an initializer list.
				 * @param values Values. Duplicates are dropped.
				 * @return This set.
				 * @throws AllocationError A node could not be allocated.
				 */
				UnorderedSet& operator=(std::initializer_list<K> values);

				/**
				 * @brief Construct from an iterator range.
				 * @tparam InputIt Input iterator of values.
				 * @param first Start.
				 * @param last End.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class InputIt>
				UnorderedSet(InputIt first, InputIt last);

				/**
				 * @brief Construct from an iterator range, a bucket count and predicates.
				 * @tparam InputIt Input iterator of values.
				 * @param first Start.
				 * @param last End.
				 * @param buckets Initial bucket count.
				 * @param hash Value hash.
				 * @param equal Value equality.
				 * @throws AllocationError A node could not be allocated.
				 */
				template<class InputIt>
				UnorderedSet(InputIt first, InputIt last, size_type buckets, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual());

				/**
				 * @brief Construct from an initializer list.
				 * @param values Values. Duplicates are dropped.
				 * @throws AllocationError A node could not be allocated.
				 */
				UnorderedSet(std::initializer_list<K> values);

				/**
				 * @brief Construct from an initializer list, a bucket count and predicates.
				 * @param values Values. Duplicates are dropped.
				 * @param buckets Initial bucket count.
				 * @param hash Value hash.
				 * @param equal Value equality.
				 * @throws AllocationError A node could not be allocated.
				 */
				UnorderedSet(std::initializer_list<K> values, size_type buckets, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual());

				/**
				 * @brief Copy a caller-owned STL set. Its hasher is not reused.
				 * @param other Source.
				 * @throws AllocationError A node could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit UnorderedSet(const std::unordered_set<K>& other): UnorderedSet(other.bucket_count()) {
					for (const K& value : other)
						insert(value);
				}

				/**
				 * @brief Move values from a caller-owned STL set and leave it empty. Its hasher is not reused.
				 * @param other Source. Cleared before this function returns.
				 * @throws AllocationError A node could not be allocated.
				 */
				STORMBYTE_FORCE_INLINE explicit UnorderedSet(std::unordered_set<K>&& other): UnorderedSet(other.bucket_count()) {
					for (const K& value : other)
						insert(value);
					other.clear();
				}

				/**
				 * @brief Return the stored hash.
				 * @return Value hash.
				 */
				hasher hash_function() const;

				/**
				 * @brief Return the stored equality predicate.
				 * @return Value equality.
				 */
				key_equal key_eq() const;

				/**
				 * @brief Return an iterator to the first occupied node.
				 * @return First value, or end when empty.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return an iterator to the first occupied node.
				 * @return First value, or end when empty.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return an iterator to the first occupied node.
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
				 * @brief Destroy every value. The bucket array stays.
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
				 * @brief Insert a value. @p hint is ignored.
				 * @param hint Unused.
				 * @param value Value to copy.
				 * @return Iterator to the value.
				 * @throws AllocationError The node could not be allocated.
				 */
				iterator insert(const_iterator hint, const K& value);

				/**
				 * @brief Insert a value. @p hint is ignored.
				 * @param hint Unused.
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
				 * @brief Construct a value. @p hint is ignored.
				 * @tparam Args Value constructor argument types.
				 * @param hint Unused.
				 * @param args Arguments forwarded to K.
				 * @return Iterator to the value.
				 * @throws AllocationError The node could not be allocated.
				 */
				template<class... Args>
				iterator emplace_hint(const_iterator hint, Args&&... args);

				/**
				 * @brief Insert an extracted node if its value is absent. The same node is linked.
				 * @param handle Node from a set with this hash and equality. Emptied when inserted.
				 * @return Position, whether it was inserted, and the rejected node.
				 * @throws AllocationError The bucket array could not grow.
				 */
				insert_return_type insert(node_type&& handle);

				/**
				 * @brief Insert an extracted node. @p hint is ignored.
				 * @param hint Unused.
				 * @param handle Node from a set with this hash and equality. Emptied when inserted.
				 * @return Iterator to the value, or end when @p handle was empty.
				 * @throws AllocationError The bucket array could not grow.
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
				 * @tparam OtherHash Source hash.
				 * @tparam OtherEqual Source equality.
				 * @param source Set to drain. The same hash and equality transfer the node. A different predicate copies the value. Values already present stay in @p source.
				 * @throws AllocationError A copied node could not be allocated.
				 */
				template<class OtherHash, class OtherEqual>
				void merge(UnorderedSet<K, OtherHash, OtherEqual>& source);

				/**
				 * @brief Erase the value at an iterator.
				 * @param position Value to erase.
				 * @return Iterator following the erased value.
				 */
				iterator erase(iterator position);

				/**
				 * @brief Erase the value at a read-only iterator.
				 * @param position Value to erase.
				 * @return Iterator following the erased value.
				 */
				iterator erase(const_iterator position);

				/**
				 * @brief Erase `[first, last)`.
				 * @param first First value to erase.
				 * @param last End of the range.
				 * @return Iterator following the last erased value.
				 */
				iterator erase(const_iterator first, const_iterator last);

				/**
				 * @brief Erase a value.
				 * @param value Lookup value.
				 * @return Number of erased values, zero or one.
				 */
				size_type erase(const K& value);

				/**
				 * @brief Exchange tables.
				 * @param other Other set.
				 */
				void swap(UnorderedSet& other) noexcept;

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
				 * @brief Return the range of values equal to @p value.
				 * @param value Lookup value.
				 * @return The value and the following cursor, or two ends when absent.
				 */
				Pair<iterator, iterator> equal_range(const K& value);

				/**
				 * @brief Return the range of values equal to @p value.
				 * @param value Lookup value.
				 * @return The value and the following cursor, or two ends when absent.
				 */
				Pair<const_iterator, const_iterator> equal_range(const K& value) const;

				/**
				 * @brief Return the bucket count.
				 * @return Bucket count. Zero before the first allocation.
				 */
				size_type bucket_count() const noexcept;

				/**
				 * @brief Return the number of values in @p index.
				 * @param index Bucket index. Not checked.
				 * @return Value count in that bucket.
				 */
				size_type bucket_size(size_type index) const;

				/**
				 * @brief Return the bucket of @p value.
				 * @param value Lookup value.
				 * @return Bucket index. Zero when the table has no buckets.
				 */
				size_type bucket(const K& value) const;

				/**
				 * @brief Return the current load factor.
				 * @return Value count divided by bucket count, or zero when there are no buckets.
				 */
				float load_factor() const noexcept;

				/**
				 * @brief Return the load factor that triggers a rehash.
				 * @return Maximum load factor.
				 */
				float max_load_factor() const noexcept;

				/**
				 * @brief Set the load factor that triggers a rehash.
				 * @param factor Maximum load factor. Not checked.
				 */
				void max_load_factor(float factor) noexcept;

				/**
				 * @brief Replace the bucket array. Values are relinked, not copied.
				 * @param buckets New bucket count. Rounded up to a power of two.
				 * @throws AllocationError The new bucket array could not be allocated.
				 */
				void rehash(size_type buckets);

				/**
				 * @brief Reserve room for @p count values without exceeding the load factor.
				 * @param count Value count to accommodate.
				 * @throws AllocationError The bucket array could not be allocated.
				 */
				void reserve(size_type count);

				/**
				 * @brief Compare values. Order does not matter.
				 * @param other Other set.
				 * @return Whether both sets hold the same values.
				 */
				bool operator==(const UnorderedSet& other) const;

				/**
				 * @brief Compare values. Order does not matter.
				 * @param other Other set.
				 * @return Whether the sets differ.
				 */
				bool operator!=(const UnorderedSet& other) const;

				/**
				 * @brief Copy the values into caller-owned STL storage. The STL hasher is `std::hash`.
				 * @return An `std::unordered_set` owned by the caller.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::unordered_set<K>() const {
					std::unordered_set<K> exported(bucket_count());
					for (const K& value : *this)
						exported.insert(value);
					return exported;
				}

			private:
				Storage m_values; ///< Unique keys. The mapped monostate is unused.
		};

		/**
		 * @brief Exchange two sets.
		 * @tparam K Value type.
		 * @tparam Hash Value hash.
		 * @tparam KeyEqual Value equality.
		 * @param left Left set.
		 * @param right Right set.
		 */
		template<Type::SafeValue K, class Hash, class KeyEqual>
		inline void swap(UnorderedSet<K, Hash, KeyEqual>& left, UnorderedSet<K, Hash, KeyEqual>& right) noexcept {
			left.swap(right);
		}
	}
}

/// @cond
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<bool>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<signed char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<unsigned char>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<wchar_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<char8_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<char16_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<char32_t>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<short>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<unsigned short>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<int>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<unsigned int>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<unsigned long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<long long>;
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<unsigned long long>;
/// @endcond

#include <StormByte/safe/unordered_set.txx>
#include <StormByte/safe/string.hxx>

/// @cond
extern template class STORMBYTE_PUBLIC StormByte::Safe::UnorderedSet<StormByte::Safe::String>;
/// @endcond
