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

#include <algorithm>
#include <new>
#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<bool IsConst>
		Map<K, V, Compare>::BasicIterator<IsConst>& Map<K, V, Compare>::BasicIterator<IsConst>::operator++() noexcept {
			if (m_node->Right != nullptr) {
				m_node = m_node->Right;
				while (m_node->Left != nullptr)
					m_node = m_node->Left;
				return *this;
			}
			NodeType* parent = m_node->Parent;
			while (m_node == parent->Right) {
				m_node = parent;
				parent = parent->Parent;
			}
			if (m_node->Right != parent)
				m_node = parent;
			return *this;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<bool IsConst>
		Map<K, V, Compare>::BasicIterator<IsConst> Map<K, V, Compare>::BasicIterator<IsConst>::operator++(int) noexcept {
			BasicIterator previous = *this;
			++*this;
			return previous;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<bool IsConst>
		Map<K, V, Compare>::BasicIterator<IsConst>& Map<K, V, Compare>::BasicIterator<IsConst>::operator--() noexcept {
			if (m_node->Red && m_node->Parent->Parent == m_node) {
				m_node = m_node->Right;
				return *this;
			}
			if (m_node->Left != nullptr) {
				m_node = m_node->Left;
				while (m_node->Right != nullptr)
					m_node = m_node->Right;
				return *this;
			}
			NodeType* parent = m_node->Parent;
			while (m_node == parent->Left) {
				m_node = parent;
				parent = parent->Parent;
			}
			m_node = parent;
			return *this;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<bool IsConst>
		Map<K, V, Compare>::BasicIterator<IsConst> Map<K, V, Compare>::BasicIterator<IsConst>::operator--(int) noexcept {
			BasicIterator previous = *this;
			--*this;
			return previous;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Map() noexcept: m_compare(), m_header(nullptr), m_size(0) {
			CreateHeader();
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Map(const Compare& compare) noexcept: m_compare(compare), m_header(nullptr), m_size(0) {
			CreateHeader();
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Map(const Map& other): m_compare(other.m_compare), m_header(nullptr), m_size(0) {
			CreateHeader();
			if (other.m_header->Parent != nullptr) {
				m_header->Parent = Clone(other.m_header->Parent, m_header);
				m_header->Left = Leftmost();
				Node* right = m_header->Parent;
				while (right->Right != nullptr)
					right = right->Right;
				m_header->Right = right;
				m_size = other.m_size;
			}
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Map(Map&& other) noexcept: m_compare(std::move(other.m_compare)), m_header(other.m_header), m_size(other.m_size) {
			other.m_header = nullptr;
			other.m_size = 0;
			other.CreateHeader();
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Map(std::initializer_list<value_type> values): Map() {
			insert(values);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::~Map() noexcept {
			clear();
			DestroyHeader();
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>& Map<K, V, Compare>::operator=(const Map& other) {
			if (this == &other)
				return *this;
			Map copy(other);
			swap(copy);
			return *this;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>& Map<K, V, Compare>::operator=(Map&& other) noexcept {
			if (this == &other)
				return *this;
			clear();
			DestroyHeader();
			m_compare = std::move(other.m_compare);
			m_header = other.m_header;
			m_size = other.m_size;
			other.m_header = nullptr;
			other.m_size = 0;
			other.CreateHeader();
			return *this;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>& Map<K, V, Compare>::operator=(std::initializer_list<value_type> values) {
			Map replacement(values);
			swap(replacement);
			return *this;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::begin() noexcept {
			return iterator(m_header->Left);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::end() noexcept {
			return iterator(m_header);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::begin() const noexcept {
			return const_iterator(m_header->Left);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::end() const noexcept {
			return const_iterator(m_header);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::cbegin() const noexcept {
			return begin();
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::cend() const noexcept {
			return end();
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::clear() noexcept {
			DestroyTree(m_header->Parent);
			m_header->Parent = nullptr;
			m_header->Left = m_header;
			m_header->Right = m_header;
			m_size = 0;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::swap(Map& other) noexcept {
			using std::swap;
			swap(m_compare, other.m_compare);
			swap(m_header, other.m_header);
			swap(m_size, other.m_size);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::HintMatches(const_iterator hint, const K& key) const noexcept {
			if (hint == cend())
				return empty() || m_compare(m_header->Right->Entry.first, key);
			if (m_compare(key, hint->first)) {
				if (hint == cbegin())
					return true;
				const_iterator previous = hint;
				--previous;
				return m_compare(previous->first, key);
			}
			return false;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert(const value_type& entry) {
			return try_emplace(entry.first, entry.second);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert(value_type&& entry) {
			return try_emplace(std::move(const_cast<K&>(entry.first)), std::move(entry.second));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert(const_iterator hint, const value_type& entry) {
			return try_emplace(hint, entry.first, entry.second);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert(const_iterator hint, value_type&& entry) {
			return try_emplace(hint, std::move(const_cast<K&>(entry.first)), std::move(entry.second));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class P>
		requires Type::ConstructibleFrom<typename Map<K, V, Compare>::value_type, P> && (!Type::SameAs<std::remove_cvref_t<P>, typename Map<K, V, Compare>::value_type>)
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert(P&& entry) {
			return insert(value_type(std::forward<P>(entry)));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::insert(std::initializer_list<value_type> values) {
			insert(values.begin(), values.end());
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::try_emplace(const K& key, Args&&... args) {
			Node* existing = FindNode(key);
			if (existing != nullptr)
				return insert_result(iterator(existing), false);
			return insert_result(try_emplace(cend(), key, std::forward<Args>(args)...), true);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::try_emplace(K&& key, Args&&... args) {
			Node* existing = FindNode(key);
			if (existing != nullptr)
				return insert_result(iterator(existing), false);
			return insert_result(try_emplace(cend(), std::move(key), std::forward<Args>(args)...), true);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::try_emplace(const_iterator hint, const K& key, Args&&... args) {
			Node* existing = FindNode(key);
			if (existing != nullptr)
				return iterator(existing);
			Node* parent = m_header;
			bool left = true;
			if (HintMatches(hint, key)) {
				if (hint == cend()) {
					parent = empty() ? m_header : m_header->Right;
					left = empty();
				} else {
					parent = const_cast<Node*>(hint.m_node);
					left = true;
				}
			} else {
				Node* current = m_header->Parent;
				while (current != nullptr) {
					parent = current;
					left = m_compare(key, current->Entry.first);
					current = left ? current->Left : current->Right;
				}
			}
			Node* created = nullptr;
			if constexpr (sizeof...(Args) == 0)
				created = MakeNode(key, V{});
			else
				created = MakeNode(key, std::forward<Args>(args)...);
			InsertNode(created, parent, left);
			return iterator(created);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::try_emplace(const_iterator hint, K&& key, Args&&... args) {
			const K& lookup = key;
			Node* existing = FindNode(lookup);
			if (existing != nullptr)
				return iterator(existing);
			Node* parent = m_header;
			bool left = true;
			if (HintMatches(hint, lookup)) {
				if (hint == cend()) {
					parent = empty() ? m_header : m_header->Right;
					left = empty();
				} else {
					parent = const_cast<Node*>(hint.m_node);
					left = true;
				}
			} else {
				Node* current = m_header->Parent;
				while (current != nullptr) {
					parent = current;
					left = m_compare(lookup, current->Entry.first);
					current = left ? current->Left : current->Right;
				}
			}
			Node* created = nullptr;
			if constexpr (sizeof...(Args) == 0)
				created = MakeNode(std::move(key), V{});
			else
				created = MakeNode(std::move(key), std::forward<Args>(args)...);
			InsertNode(created, parent, left);
			return iterator(created);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::emplace(Args&&... args) {
			return insert(value_type(std::forward<Args>(args)...));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::emplace_hint(const_iterator hint, Args&&... args) {
			return insert(hint, value_type(std::forward<Args>(args)...));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert_or_assign(const K& key, const V& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = value;
				return insert_result(iterator(existing), false);
			}
			return insert_result(try_emplace(cend(), key, value), true);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert_or_assign(const K& key, V&& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = std::move(value);
				return insert_result(iterator(existing), false);
			}
			return insert_result(try_emplace(cend(), key, std::move(value)), true);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert_or_assign(K&& key, const V& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = value;
				return insert_result(iterator(existing), false);
			}
			return insert_result(try_emplace(cend(), std::move(key), value), true);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_result Map<K, V, Compare>::insert_or_assign(K&& key, V&& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = std::move(value);
				return insert_result(iterator(existing), false);
			}
			return insert_result(try_emplace(cend(), std::move(key), std::move(value)), true);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert_or_assign(const_iterator hint, const K& key, const V& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = value;
				return iterator(existing);
			}
			return try_emplace(hint, key, value);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert_or_assign(const_iterator hint, const K& key, V&& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = std::move(value);
				return iterator(existing);
			}
			return try_emplace(hint, key, std::move(value));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert_or_assign(const_iterator hint, K&& key, const V& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = value;
				return iterator(existing);
			}
			return try_emplace(hint, std::move(key), value);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert_or_assign(const_iterator hint, K&& key, V&& value) {
			Node* existing = FindNode(key);
			if (existing != nullptr) {
				existing->Entry.second = std::move(value);
				return iterator(existing);
			}
			return try_emplace(hint, std::move(key), std::move(value));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::insert_return_type Map<K, V, Compare>::insert(node_type&& handle) {
			if (!handle)
				return insert_return_type{end(), false, node_type{}};
			Node* existing = FindNode(handle.key());
			if (existing != nullptr)
				return insert_return_type{iterator(existing), false, std::move(handle)};
			Node* node = handle.m_node;
			handle.m_node = nullptr;
			node->Left = nullptr;
			node->Right = nullptr;
			node->Red = true;
			const K& key = node->Entry.first;
			Node* parent = m_header;
			Node* current = m_header->Parent;
			bool left = true;
			while (current != nullptr) {
				parent = current;
				left = m_compare(key, current->Entry.first);
				current = left ? current->Left : current->Right;
			}
			InsertNode(node, parent, left);
			return insert_return_type{iterator(node), true, node_type{}};
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::insert(const_iterator hint, node_type&& handle) {
			if (!handle)
				return end();
			if (!HintMatches(hint, handle.key()))
				return insert(std::move(handle)).position;
			Node* existing = FindNode(handle.key());
			if (existing != nullptr)
				return iterator(existing);
			Node* node = handle.m_node;
			handle.m_node = nullptr;
			node->Left = nullptr;
			node->Right = nullptr;
			node->Red = true;
			Node* parent = m_header;
			bool left = true;
			if (hint == cend()) {
				parent = empty() ? m_header : m_header->Right;
				left = empty();
			} else {
				parent = const_cast<Node*>(hint.m_node);
				left = true;
			}
			InsertNode(node, parent, left);
			return iterator(node);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::node_type Map<K, V, Compare>::extract(const_iterator position) {
			if (position == cend())
				return node_type{};
			Node* node = const_cast<Node*>(position.m_node);
			Unlink(node);
			node->Parent = nullptr;
			node->Left = nullptr;
			node->Right = nullptr;
			return node_type(node);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::node_type Map<K, V, Compare>::extract(const K& key) {
			iterator found = find(key);
			return found == end() ? node_type{} : extract(found);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class OtherCompare>
		void Map<K, V, Compare>::merge(Map<K, V, OtherCompare>& source) {
			auto cursor = source.begin();
			while (cursor != source.end()) {
				auto next = cursor;
				++next;
				if (FindNode(cursor->first) == nullptr) {
					node_type handle = source.extract(cursor);
					insert(std::move(handle));
				}
				cursor = next;
			}
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::erase(iterator position) noexcept {
			iterator following = position;
			++following;
			Unlink(position.m_node);
			DestroyNode(position.m_node);
			return following;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::erase(const_iterator position) noexcept {
			return erase(iterator(const_cast<Node*>(position.m_node)));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::erase(const_iterator first, const_iterator last) noexcept {
			while (first != last) {
				iterator current(const_cast<Node*>(first.m_node));
				++first;
				erase(current);
			}
			return iterator(const_cast<Node*>(last.m_node));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::size_type Map<K, V, Compare>::erase(const K& key) noexcept {
			Node* node = FindNode(key);
			if (node == nullptr)
				return 0;
			Unlink(node);
			DestroyNode(node);
			return 1;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::find(const K& key) noexcept {
			Node* node = FindNode(key);
			return node == nullptr ? end() : iterator(node);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::find(const K& key) const noexcept {
			Node* node = FindNode(key);
			return node == nullptr ? end() : const_iterator(node);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::contains(const K& key) const noexcept {
			return FindNode(key) != nullptr;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::size_type Map<K, V, Compare>::count(const K& key) const noexcept {
			return contains(key) ? 1 : 0;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::lower_bound(const K& key) noexcept {
			Node* current = m_header->Parent;
			Node* bound = m_header;
			while (current != nullptr) {
				if (!m_compare(current->Entry.first, key)) {
					bound = current;
					current = current->Left;
				} else {
					current = current->Right;
				}
			}
			return iterator(bound);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::lower_bound(const K& key) const noexcept {
			Node* current = m_header->Parent;
			Node* bound = m_header;
			while (current != nullptr) {
				if (!m_compare(current->Entry.first, key)) {
					bound = current;
					current = current->Left;
				} else {
					current = current->Right;
				}
			}
			return const_iterator(bound);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::iterator Map<K, V, Compare>::upper_bound(const K& key) noexcept {
			Node* current = m_header->Parent;
			Node* bound = m_header;
			while (current != nullptr) {
				if (m_compare(key, current->Entry.first)) {
					bound = current;
					current = current->Left;
				} else {
					current = current->Right;
				}
			}
			return iterator(bound);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_iterator Map<K, V, Compare>::upper_bound(const K& key) const noexcept {
			Node* current = m_header->Parent;
			Node* bound = m_header;
			while (current != nullptr) {
				if (m_compare(key, current->Entry.first)) {
					bound = current;
					current = current->Left;
				} else {
					current = current->Right;
				}
			}
			return const_iterator(bound);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::equal_range_result Map<K, V, Compare>::equal_range(const K& key) noexcept {
			return equal_range_result(lower_bound(key), upper_bound(key));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::const_equal_range_result Map<K, V, Compare>::equal_range(const K& key) const noexcept {
			return const_equal_range_result(lower_bound(key), upper_bound(key));
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		V& Map<K, V, Compare>::operator[](const K& key) {
			return try_emplace(key).first->second;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		V& Map<K, V, Compare>::operator[](K&& key) {
			return try_emplace(std::move(key)).first->second;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		V& Map<K, V, Compare>::at(const K& key) {
			Node* node = FindNode(key);
			if (node == nullptr)
				throw OutOfBoundsError(AbsentKey(key));
			return node->Entry.second;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		const V& Map<K, V, Compare>::at(const K& key) const {
			Node* node = FindNode(key);
			if (node == nullptr)
				throw OutOfBoundsError(AbsentKey(key));
			return node->Entry.second;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::operator==(const Map& other) const requires Type::EqualityComparable<K> && Type::EqualityComparable<V> {
			if (m_size != other.m_size)
				return false;
			auto left = begin();
			auto right = other.begin();
			while (left != end()) {
				if (!(left->first == right->first) || !(left->second == right->second))
					return false;
				++left;
				++right;
			}
			return true;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		std::strong_ordering Map<K, V, Compare>::operator<=>(const Map& other) const requires std::three_way_comparable<K> && std::three_way_comparable<V> {
			return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end(), [](const value_type& left, const value_type& right) {
				if (auto key = left.first <=> right.first; key != 0)
					return key;
				return left.second <=> right.second;
			});
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::operator<(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; } {
			return std::lexicographical_compare(begin(), end(), other.begin(), other.end(), [](const value_type& left, const value_type& right) {
				return left.first < right.first || (!(right.first < left.first) && left.second < right.second);
			});
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::operator<=(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; } {
			return !(other < *this);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::operator>(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; } {
			return other < *this;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Map<K, V, Compare>::operator>=(const Map& other) const requires requires(const K& a, const K& b, const V& c, const V& d) { a < b; c < d; } {
			return !(*this < other);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::CreateHeader() {
			void* block = Heap::Allocate(sizeof(Node));
			m_header = static_cast<Node*>(block);
			m_header->Parent = nullptr;
			m_header->Left = m_header;
			m_header->Right = m_header;
			m_header->Red = true;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::DestroyHeader() noexcept {
			Heap::Free(m_header);
			m_header = nullptr;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		Map<K, V, Compare>::Node* Map<K, V, Compare>::MakeNode(Args&&... args) {
			void* block = Heap::Allocate(sizeof(Node));
			try {
				Node* node = static_cast<Node*>(block);
				new (&node->Entry) value_type(std::forward<Args>(args)...);
				node->Parent = nullptr;
				node->Left = nullptr;
				node->Right = nullptr;
				node->Red = true;
				return node;
			} catch (...) {
				Heap::Free(block);
				throw;
			}
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::DestroyNode(Node* node) noexcept {
			node->Entry.~value_type();
			Heap::Free(node);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::DestroyTree(Node* node) noexcept {
			while (node != nullptr) {
				DestroyTree(node->Right);
				Node* left = node->Left;
				DestroyNode(node);
				node = left;
			}
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Node* Map<K, V, Compare>::Clone(const Node* source, Node* parent) {
			Node* node = MakeNode(source->Entry.first, source->Entry.second);
			node->Parent = parent;
			node->Red = source->Red;
			if (source->Left != nullptr)
				node->Left = Clone(source->Left, node);
			if (source->Right != nullptr)
				node->Right = Clone(source->Right, node);
			return node;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Node* Map<K, V, Compare>::FindNode(const K& key) const noexcept {
			Node* current = m_header->Parent;
			while (current != nullptr) {
				if (m_compare(key, current->Entry.first))
					current = current->Left;
				else if (m_compare(current->Entry.first, key))
					current = current->Right;
				else
					return current;
			}
			return nullptr;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::RotateLeft(Node* node) noexcept {
			Node* next = node->Right;
			node->Right = next->Left;
			if (next->Left != nullptr)
				next->Left->Parent = node;
			next->Parent = node->Parent;
			if (node == m_header->Parent)
				m_header->Parent = next;
			else if (node == node->Parent->Left)
				node->Parent->Left = next;
			else
				node->Parent->Right = next;
			next->Left = node;
			node->Parent = next;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::RotateRight(Node* node) noexcept {
			Node* next = node->Left;
			node->Left = next->Right;
			if (next->Right != nullptr)
				next->Right->Parent = node;
			next->Parent = node->Parent;
			if (node == m_header->Parent)
				m_header->Parent = next;
			else if (node == node->Parent->Right)
				node->Parent->Right = next;
			else
				node->Parent->Left = next;
			next->Right = node;
			node->Parent = next;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::InsertNode(Node* node, Node* parent, bool left) noexcept {
			node->Parent = parent;
			if (parent == m_header) {
				m_header->Parent = node;
				m_header->Left = node;
				m_header->Right = node;
			} else if (left) {
				parent->Left = node;
				if (parent == m_header->Left)
					m_header->Left = node;
			} else {
				parent->Right = node;
				if (parent == m_header->Right)
					m_header->Right = node;
			}
			++m_size;
			while (node != m_header->Parent && node->Parent->Red) {
				Node* upper = node->Parent->Parent;
				if (node->Parent == upper->Left) {
					Node* uncle = upper->Right;
					if (uncle != nullptr && uncle->Red) {
						node->Parent->Red = false;
						uncle->Red = false;
						upper->Red = true;
						node = upper;
					} else {
						if (node == node->Parent->Right) {
							node = node->Parent;
							RotateLeft(node);
						}
						node->Parent->Red = false;
						upper->Red = true;
						RotateRight(upper);
					}
				} else {
					Node* uncle = upper->Left;
					if (uncle != nullptr && uncle->Red) {
						node->Parent->Red = false;
						uncle->Red = false;
						upper->Red = true;
						node = upper;
					} else {
						if (node == node->Parent->Left) {
							node = node->Parent;
							RotateRight(node);
						}
						node->Parent->Red = false;
						upper->Red = true;
						RotateLeft(upper);
					}
				}
			}
			m_header->Parent->Red = false;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Map<K, V, Compare>::Node* Map<K, V, Compare>::Leftmost() const noexcept {
			Node* node = m_header->Parent;
			if (node == nullptr)
				return m_header;
			while (node->Left != nullptr)
				node = node->Left;
			return node;
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::Unlink(Node* node) noexcept {
			EraseNode(node);
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Map<K, V, Compare>::EraseNode(Node* node) noexcept {
			Node* successor = node;
			Node* child = nullptr;
			Node* childParent = nullptr;
			if (successor->Left == nullptr)
				child = successor->Right;
			else if (successor->Right == nullptr)
				child = successor->Left;
			else {
				successor = successor->Right;
				while (successor->Left != nullptr)
					successor = successor->Left;
				child = successor->Right;
			}
			if (successor != node) {
				node->Left->Parent = successor;
				successor->Left = node->Left;
				if (successor != node->Right) {
					childParent = successor->Parent;
					if (child != nullptr)
						child->Parent = successor->Parent;
					successor->Parent->Left = child;
					successor->Right = node->Right;
					node->Right->Parent = successor;
				} else {
					childParent = successor;
				}
				if (m_header->Parent == node)
					m_header->Parent = successor;
				else if (node->Parent->Left == node)
					node->Parent->Left = successor;
				else
					node->Parent->Right = successor;
				successor->Parent = node->Parent;
				std::swap(successor->Red, node->Red);
				successor = node;
			} else {
				childParent = node->Parent;
				if (child != nullptr)
					child->Parent = node->Parent;
				if (m_header->Parent == node)
					m_header->Parent = child;
				else if (node->Parent->Left == node)
					node->Parent->Left = child;
				else
					node->Parent->Right = child;
				if (m_header->Left == node)
					m_header->Left = child == nullptr ? node->Parent : Leftmost();
				if (m_header->Right == node) {
					Node* right = child == nullptr ? node->Parent : child;
					if (child != nullptr)
						while (right->Right != nullptr)
							right = right->Right;
					m_header->Right = right;
				}
			}
			if (!successor->Red) {
				while (child != m_header->Parent && (child == nullptr || !child->Red)) {
					if (child == childParent->Left) {
						Node* sibling = childParent->Right;
						if (sibling != nullptr && sibling->Red) {
							sibling->Red = false;
							childParent->Red = true;
							RotateLeft(childParent);
							sibling = childParent->Right;
						}
						if (sibling == nullptr || ((sibling->Left == nullptr || !sibling->Left->Red) && (sibling->Right == nullptr || !sibling->Right->Red))) {
							if (sibling != nullptr)
								sibling->Red = true;
							child = childParent;
							childParent = childParent->Parent;
						} else {
							if (sibling->Right == nullptr || !sibling->Right->Red) {
								if (sibling->Left != nullptr)
									sibling->Left->Red = false;
								sibling->Red = true;
								RotateRight(sibling);
								sibling = childParent->Right;
							}
							sibling->Red = childParent->Red;
							childParent->Red = false;
							if (sibling->Right != nullptr)
								sibling->Right->Red = false;
							RotateLeft(childParent);
							break;
						}
					} else {
						Node* sibling = childParent->Left;
						if (sibling != nullptr && sibling->Red) {
							sibling->Red = false;
							childParent->Red = true;
							RotateRight(childParent);
							sibling = childParent->Left;
						}
						if (sibling == nullptr || ((sibling->Left == nullptr || !sibling->Left->Red) && (sibling->Right == nullptr || !sibling->Right->Red))) {
							if (sibling != nullptr)
								sibling->Red = true;
							child = childParent;
							childParent = childParent->Parent;
						} else {
							if (sibling->Left == nullptr || !sibling->Left->Red) {
								if (sibling->Right != nullptr)
									sibling->Right->Red = false;
								sibling->Red = true;
								RotateLeft(sibling);
								sibling = childParent->Left;
							}
							sibling->Red = childParent->Red;
							childParent->Red = false;
							if (sibling->Left != nullptr)
								sibling->Left->Red = false;
							RotateRight(childParent);
							break;
						}
					}
				}
				if (child != nullptr)
					child->Red = false;
			}
			--m_size;
			if (m_size == 0) {
				m_header->Parent = nullptr;
				m_header->Left = m_header;
				m_header->Right = m_header;
			}
		}

		template<Type::SafeValue K, Type::SafeValue V, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		String Map<K, V, Compare>::AbsentKey(const K& key) {
			String message("map key '");
			if constexpr (requires { String(key); })
				message += String(key);
			else if constexpr (requires { key.data(); key.size(); })
				message += String(key.data(), static_cast<std::size_t>(key.size()));
			else
				message += "?";
			message += "' is absent";
			return message;
		}
	}
}
