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

namespace StormByte::Safe {
	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	template<bool IsConst>
	UnorderedMap<K, V, Hash, KeyEqual>::BasicIterator<IsConst>& UnorderedMap<K, V, Hash, KeyEqual>::BasicIterator<IsConst>::operator++() noexcept {
		if (m_node->Next != nullptr) {
			m_node = m_node->Next;
			return *this;
		}
		const size_type slot = m_node->Code & (m_map->m_bucket_count - 1);
		m_node = m_map->NextBucket(slot);
		return *this;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	template<bool IsConst>
	UnorderedMap<K, V, Hash, KeyEqual>::BasicIterator<IsConst> UnorderedMap<K, V, Hash, KeyEqual>::BasicIterator<IsConst>::operator++(int) noexcept {
		BasicIterator previous(*this);
		++(*this);
		return previous;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::UnorderedMap() noexcept = default;

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::UnorderedMap(size_type buckets, const Hash& hash, const KeyEqual& equal): m_hash(hash), m_equal(equal) {
		const size_type count = BucketCount(buckets);
		m_buckets = AllocateBuckets(count);
		m_bucket_count = count;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::UnorderedMap(const UnorderedMap& other): UnorderedMap(other.m_bucket_count, other.m_hash, other.m_equal) {
		for (const value_type& entry : other)
			insert(entry);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::UnorderedMap(UnorderedMap&& other) noexcept: m_buckets(other.m_buckets), m_bucket_count(other.m_bucket_count), m_size(other.m_size), m_max_load(other.m_max_load), m_hash(std::move(other.m_hash)), m_equal(std::move(other.m_equal)) {
		other.m_buckets = nullptr;
		other.m_bucket_count = 0;
		other.m_size = 0;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::~UnorderedMap() noexcept { Destroy(); }

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>& UnorderedMap<K, V, Hash, KeyEqual>::operator=(const UnorderedMap& other) {
		if (this != &other) {
			UnorderedMap replacement(other);
			swap(replacement);
		}
		return *this;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>& UnorderedMap<K, V, Hash, KeyEqual>::operator=(UnorderedMap&& other) noexcept {
		if (this != &other) {
			Destroy();
			m_buckets = other.m_buckets;
			m_bucket_count = other.m_bucket_count;
			m_size = other.m_size;
			m_max_load = other.m_max_load;
			m_hash = std::move(other.m_hash);
			m_equal = std::move(other.m_equal);
			other.m_buckets = nullptr;
			other.m_bucket_count = 0;
			other.m_size = 0;
		}
		return *this;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::UnorderedMap(std::initializer_list<value_type> values): UnorderedMap() {
		insert(values);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>& UnorderedMap<K, V, Hash, KeyEqual>::operator=(std::initializer_list<value_type> values) {
		UnorderedMap replacement(values);
		swap(replacement);
		return *this;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::iterator UnorderedMap<K, V, Hash, KeyEqual>::begin() noexcept {
		return iterator(NextBucket(static_cast<size_type>(-1)), this);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::const_iterator UnorderedMap<K, V, Hash, KeyEqual>::begin() const noexcept {
		return const_iterator(NextBucket(static_cast<size_type>(-1)), this);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::clear() noexcept {
		for (size_type index = 0; index < m_bucket_count; ++index) {
			Node* node = m_buckets[index];
			while (node != nullptr) {
				Node* next = node->Next;
				node->Entry.~value_type();
				Heap::Free(node);
				node = next;
			}
			m_buckets[index] = nullptr;
		}
		m_size = 0;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::insert_result UnorderedMap<K, V, Hash, KeyEqual>::insert(const value_type& value) {
		return emplace(value);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::insert_result UnorderedMap<K, V, Hash, KeyEqual>::insert(value_type&& value) {
		return emplace(std::move(value));
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::insert(std::initializer_list<value_type> values) {
		for (const value_type& value : values)
			insert(value);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	template<class... Args>
	UnorderedMap<K, V, Hash, KeyEqual>::insert_result UnorderedMap<K, V, Hash, KeyEqual>::emplace(Args&&... args) {
		Node* storage = static_cast<Node*>(Heap::Allocate(sizeof(Node)));
		if (storage == nullptr)
			throw AllocationError();
		Node* node = nullptr;
		try {
			node = new (storage) Node{nullptr, 0, value_type(std::forward<Args>(args)...)};
		} catch (...) {
			Heap::Free(storage);
			throw;
		}
		node->Code = m_hash(node->Entry.first);
		Node* existing = FindNode(node->Entry.first, node->Code);
		if (existing != nullptr) {
			node->Entry.~value_type();
			Heap::Free(node);
			return insert_result(iterator(existing, this), false);
		}
		try {
			Link(node);
		} catch (...) {
			node->Entry.~value_type();
			Heap::Free(node);
			throw;
		}
		return insert_result(iterator(node, this), true);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	template<class... Args>
	UnorderedMap<K, V, Hash, KeyEqual>::insert_result UnorderedMap<K, V, Hash, KeyEqual>::try_emplace(const K& key, Args&&... args) {
		const std::size_t code = m_hash(key);
		Node* existing = FindNode(key, code);
		if (existing != nullptr)
			return insert_result(iterator(existing, this), false);
		return emplace(key, V(std::forward<Args>(args)...));
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::insert_result UnorderedMap<K, V, Hash, KeyEqual>::insert_or_assign(const K& key, V mapped) {
		const std::size_t code = m_hash(key);
		Node* existing = FindNode(key, code);
		if (existing != nullptr) {
			existing->Entry.second = std::move(mapped);
			return insert_result(iterator(existing, this), false);
		}
		return emplace(key, std::move(mapped));
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::iterator UnorderedMap<K, V, Hash, KeyEqual>::erase(const_iterator position) {
		Node* node = const_cast<Node*>(position.m_node);
		Node* next = Unlink(node);
		const size_type slot = node->Code & (m_bucket_count - 1);
		node->Entry.~value_type();
		Heap::Free(node);
		if (next == nullptr)
			next = NextBucket(slot);
		return iterator(next, this);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::size_type UnorderedMap<K, V, Hash, KeyEqual>::erase(const K& key) {
		const_iterator found = find(key);
		if (found == end())
			return 0;
		erase(found);
		return 1;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::node_type UnorderedMap<K, V, Hash, KeyEqual>::extract(const_iterator position) {
		Unlink(const_cast<Node*>(position.m_node));
		return node_type(const_cast<Node*>(position.m_node));
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::node_type UnorderedMap<K, V, Hash, KeyEqual>::extract(const K& key) {
		const_iterator found = find(key);
		if (found == end())
			return node_type();
		return extract(found);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::insert_return_type UnorderedMap<K, V, Hash, KeyEqual>::insert(node_type&& node) {
		insert_return_type result{end(), false, node_type()};
		if (node.empty())
			return result;
		Node* existing = FindNode(node.m_node->Entry.first, node.m_node->Code);
		if (existing != nullptr) {
			result.position = iterator(existing, this);
			result.node = std::move(node);
			return result;
		}
		Node* taken = node.m_node;
		node.m_node = nullptr;
		taken->Next = nullptr;
		Link(taken);
		result.position = iterator(taken, this);
		result.inserted = true;
		return result;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::merge(UnorderedMap& other) {
		if (this == &other)
			return;
		for (size_type index = 0; index < other.m_bucket_count; ++index) {
			Node* node = other.m_buckets[index];
			while (node != nullptr) {
				Node* next = node->Next;
				if (FindNode(node->Entry.first, node->Code) == nullptr) {
					other.Unlink(node);
					node->Next = nullptr;
					Link(node);
				}
				node = next;
			}
		}
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::swap(UnorderedMap& other) noexcept {
		std::swap(m_buckets, other.m_buckets);
		std::swap(m_bucket_count, other.m_bucket_count);
		std::swap(m_size, other.m_size);
		std::swap(m_max_load, other.m_max_load);
		std::swap(m_hash, other.m_hash);
		std::swap(m_equal, other.m_equal);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	V& UnorderedMap<K, V, Hash, KeyEqual>::at(const K& key) {
		Node* node = FindNode(key, m_hash(key));
		if (node == nullptr)
			throw OutOfBoundsError(Safe::String("unordered map key"));
		return node->Entry.second;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	const V& UnorderedMap<K, V, Hash, KeyEqual>::at(const K& key) const {
		Node* node = FindNode(key, m_hash(key));
		if (node == nullptr)
			throw OutOfBoundsError(Safe::String("unordered map key"));
		return node->Entry.second;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	V& UnorderedMap<K, V, Hash, KeyEqual>::operator[](const K& key) {
		insert_result result = try_emplace(key);
		return result.first->second;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::iterator UnorderedMap<K, V, Hash, KeyEqual>::find(const K& key) {
		return iterator(FindNode(key, m_hash(key)), this);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::const_iterator UnorderedMap<K, V, Hash, KeyEqual>::find(const K& key) const {
		return const_iterator(FindNode(key, m_hash(key)), this);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::size_type UnorderedMap<K, V, Hash, KeyEqual>::count(const K& key) const {
		return contains(key) ? 1 : 0;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	bool UnorderedMap<K, V, Hash, KeyEqual>::contains(const K& key) const {
		return FindNode(key, m_hash(key)) != nullptr;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::size_type UnorderedMap<K, V, Hash, KeyEqual>::bucket_size(size_type index) const {
		size_type count = 0;
		for (Node* node = m_buckets[index]; node != nullptr; node = node->Next)
			++count;
		return count;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::size_type UnorderedMap<K, V, Hash, KeyEqual>::bucket(const K& key) const {
		if (m_bucket_count == 0)
			return 0;
		return m_hash(key) & (m_bucket_count - 1);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	float UnorderedMap<K, V, Hash, KeyEqual>::load_factor() const noexcept {
		if (m_bucket_count == 0)
			return 0.0f;
		return static_cast<float>(m_size) / static_cast<float>(m_bucket_count);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::rehash(size_type buckets) {
		const size_type count = BucketCount(buckets == 0 ? 1 : buckets);
		Node** fresh = AllocateBuckets(count);
		Node** previous = m_buckets;
		const size_type previous_count = m_bucket_count;
		m_buckets = fresh;
		m_bucket_count = count;
		for (size_type index = 0; index < previous_count; ++index) {
			Node* node = previous[index];
			while (node != nullptr) {
				Node* next = node->Next;
				const size_type slot = node->Code & (count - 1);
				node->Next = m_buckets[slot];
				m_buckets[slot] = node;
				node = next;
			}
		}
		if (previous != nullptr)
			Heap::Free(previous);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::reserve(size_type count) {
		const float needed = static_cast<float>(count) / m_max_load;
		if (needed > static_cast<float>(m_bucket_count))
			rehash(static_cast<size_type>(needed) + 1);
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	bool UnorderedMap<K, V, Hash, KeyEqual>::operator==(const UnorderedMap& other) const {
		if (m_size != other.m_size)
			return false;
		for (const value_type& entry : *this) {
			const_iterator found = other.find(entry.first);
			if (found == other.end() || !(found->second == entry.second))
				return false;
		}
		return true;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::size_type UnorderedMap<K, V, Hash, KeyEqual>::BucketCount(size_type count) noexcept {
		if (count == 0)
			return 0;
		size_type rounded = 8;
		while (rounded < count)
			rounded <<= 1;
		return rounded;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::Node** UnorderedMap<K, V, Hash, KeyEqual>::AllocateBuckets(size_type count) {
		if (count == 0)
			return nullptr;
		void* block = Heap::Allocate(sizeof(Node*) * count);
		if (block == nullptr)
			throw AllocationError();
		Node** buckets = static_cast<Node**>(block);
		for (size_type index = 0; index < count; ++index)
			buckets[index] = nullptr;
		return buckets;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::FreeBuckets() noexcept {
		if (m_buckets != nullptr)
			Heap::Free(m_buckets);
		m_buckets = nullptr;
		m_bucket_count = 0;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::Destroy() noexcept {
		clear();
		FreeBuckets();
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::Node* UnorderedMap<K, V, Hash, KeyEqual>::FindNode(const K& key, std::size_t code) const noexcept {
		if (m_bucket_count == 0)
			return nullptr;
		for (Node* node = m_buckets[code & (m_bucket_count - 1)]; node != nullptr; node = node->Next) {
			if (node->Code == code && m_equal(node->Entry.first, key))
				return node;
		}
		return nullptr;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::Node* UnorderedMap<K, V, Hash, KeyEqual>::NextBucket(size_type slot) const noexcept {
		for (size_type index = slot + 1; index < m_bucket_count; ++index) {
			if (m_buckets[index] != nullptr)
				return m_buckets[index];
		}
		return nullptr;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::Link(Node* node) {
		Ensure(1);
		const size_type slot = node->Code & (m_bucket_count - 1);
		node->Next = m_buckets[slot];
		m_buckets[slot] = node;
		++m_size;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	UnorderedMap<K, V, Hash, KeyEqual>::Node* UnorderedMap<K, V, Hash, KeyEqual>::Unlink(Node* node) noexcept {
		const size_type slot = node->Code & (m_bucket_count - 1);
		Node** link = &m_buckets[slot];
		while (*link != node)
			link = &(*link)->Next;
		*link = node->Next;
		--m_size;
		return node->Next;
	}

	template<Type::SafeValue K, Type::SafeValue V, class Hash, class KeyEqual>
	void UnorderedMap<K, V, Hash, KeyEqual>::Ensure(size_type extra) {
		if (m_bucket_count == 0) {
			rehash(8);
			return;
		}
		if (static_cast<float>(m_size + extra) > m_max_load * static_cast<float>(m_bucket_count))
			rehash(m_bucket_count << 1);
	}
}
