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

namespace StormByte {
	namespace Safe {
		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet() noexcept = default;

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(size_type buckets, const Hash& hash, const KeyEqual& equal): m_values(buckets, hash, equal) {}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(const UnorderedSet& other): m_values(other.m_values) {}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(UnorderedSet&& other) noexcept: m_values(std::move(other.m_values)) {}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::~UnorderedSet() = default;

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>& UnorderedSet<K, Hash, KeyEqual>::operator=(const UnorderedSet& other) {
			m_values = other.m_values;
			return *this;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>& UnorderedSet<K, Hash, KeyEqual>::operator=(UnorderedSet&& other) noexcept {
			m_values = std::move(other.m_values);
			return *this;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>& UnorderedSet<K, Hash, KeyEqual>::operator=(std::initializer_list<K> values) {
			UnorderedSet replacement(values, bucket_count(), hash_function(), key_eq());
			swap(replacement);
			return *this;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		template<class InputIt>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(InputIt first, InputIt last) {
			insert(first, last);
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		template<class InputIt>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(InputIt first, InputIt last, size_type buckets, const Hash& hash, const KeyEqual& equal): m_values(buckets, hash, equal) {
			insert(first, last);
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(std::initializer_list<K> values) {
			insert(values);
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		UnorderedSet<K, Hash, KeyEqual>::UnorderedSet(std::initializer_list<K> values, size_type buckets, const Hash& hash, const KeyEqual& equal): m_values(buckets, hash, equal) {
			insert(values);
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::hasher UnorderedSet<K, Hash, KeyEqual>::hash_function() const { return m_values.hash_function(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::key_equal UnorderedSet<K, Hash, KeyEqual>::key_eq() const { return m_values.key_eq(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::begin() noexcept { return iterator(m_values.begin()); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::const_iterator UnorderedSet<K, Hash, KeyEqual>::begin() const noexcept { return const_iterator(m_values.begin()); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::const_iterator UnorderedSet<K, Hash, KeyEqual>::cbegin() const noexcept { return begin(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::end() noexcept { return iterator(m_values.end()); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::const_iterator UnorderedSet<K, Hash, KeyEqual>::end() const noexcept { return const_iterator(m_values.end()); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::const_iterator UnorderedSet<K, Hash, KeyEqual>::cend() const noexcept { return end(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		bool UnorderedSet<K, Hash, KeyEqual>::empty() const noexcept { return m_values.empty(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::size() const noexcept { return m_values.size(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::max_size() const noexcept { return m_values.max_size(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		void UnorderedSet<K, Hash, KeyEqual>::clear() noexcept { m_values.clear(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::insert_result UnorderedSet<K, Hash, KeyEqual>::insert(const K& value) {
			auto result = m_values.emplace(value, Monostate{});
			return insert_result{iterator(result.first), result.second};
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::insert_result UnorderedSet<K, Hash, KeyEqual>::insert(K&& value) {
			auto result = m_values.emplace(std::move(value), Monostate{});
			return insert_result{iterator(result.first), result.second};
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::insert(const_iterator, const K& value) {
			return insert(value).first;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::insert(const_iterator, K&& value) {
			return insert(std::move(value)).first;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		template<class InputIt>
		void UnorderedSet<K, Hash, KeyEqual>::insert(InputIt first, InputIt last) {
			for (; first != last; ++first)
				insert(value_type(*first));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		void UnorderedSet<K, Hash, KeyEqual>::insert(std::initializer_list<K> values) { insert(values.begin(), values.end()); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		template<class... Args>
		typename UnorderedSet<K, Hash, KeyEqual>::insert_result UnorderedSet<K, Hash, KeyEqual>::emplace(Args&&... args) {
			return insert(K(std::forward<Args>(args)...));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		template<class... Args>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::emplace_hint(const_iterator hint, Args&&... args) {
			return insert(hint, K(std::forward<Args>(args)...));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::insert_return_type UnorderedSet<K, Hash, KeyEqual>::insert(node_type&& handle) {
			auto result = m_values.insert(std::move(handle.m_node));
			return insert_return_type{iterator(result.position), result.inserted, node_type(std::move(result.node))};
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::insert(const_iterator, node_type&& handle) {
			return insert(std::move(handle)).position;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::node_type UnorderedSet<K, Hash, KeyEqual>::extract(const_iterator position) {
			return node_type(m_values.extract(position.m_cursor));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::node_type UnorderedSet<K, Hash, KeyEqual>::extract(const K& value) {
			return node_type(m_values.extract(value));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		template<class OtherHash, class OtherEqual>
		void UnorderedSet<K, Hash, KeyEqual>::merge(UnorderedSet<K, OtherHash, OtherEqual>& source) {
			auto cursor = source.begin();
			while (cursor != source.end()) {
				if (contains(*cursor)) {
					++cursor;
					continue;
				}
				auto next = std::next(cursor);
				if constexpr (std::same_as<Hash, OtherHash> && std::same_as<KeyEqual, OtherEqual>)
					insert(source.extract(cursor));
				else {
					K value = *cursor;
					source.erase(cursor);
					insert(std::move(value));
				}
				cursor = next;
			}
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::erase(iterator position) {
			return iterator(m_values.erase(position.m_cursor));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::erase(const_iterator position) {
			return iterator(m_values.erase(position.m_cursor));
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::erase(const_iterator first, const_iterator last) {
			if (first == last)
				return first == end() ? end() : find(*first);
			iterator result = end();
			while (first != last) {
				const_iterator current = first;
				++first;
				result = erase(current);
			}
			return result;
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::erase(const K& value) { return m_values.erase(value); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		void UnorderedSet<K, Hash, KeyEqual>::swap(UnorderedSet& other) noexcept { m_values.swap(other.m_values); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::iterator UnorderedSet<K, Hash, KeyEqual>::find(const K& value) { return iterator(m_values.find(value)); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::const_iterator UnorderedSet<K, Hash, KeyEqual>::find(const K& value) const { return const_iterator(m_values.find(value)); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::count(const K& value) const { return m_values.count(value); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		bool UnorderedSet<K, Hash, KeyEqual>::contains(const K& value) const { return m_values.contains(value); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		Pair<typename UnorderedSet<K, Hash, KeyEqual>::iterator, typename UnorderedSet<K, Hash, KeyEqual>::iterator> UnorderedSet<K, Hash, KeyEqual>::equal_range(const K& value) {
			iterator found = find(value);
			if (found == end())
				return Pair<iterator, iterator>{end(), end()};
			iterator next = found;
			++next;
			return Pair<iterator, iterator>{found, next};
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		Pair<typename UnorderedSet<K, Hash, KeyEqual>::const_iterator, typename UnorderedSet<K, Hash, KeyEqual>::const_iterator> UnorderedSet<K, Hash, KeyEqual>::equal_range(const K& value) const {
			const_iterator found = find(value);
			if (found == end())
				return Pair<const_iterator, const_iterator>{end(), end()};
			const_iterator next = found;
			++next;
			return Pair<const_iterator, const_iterator>{found, next};
		}

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::bucket_count() const noexcept { return m_values.bucket_count(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::bucket_size(size_type index) const { return m_values.bucket_size(index); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		typename UnorderedSet<K, Hash, KeyEqual>::size_type UnorderedSet<K, Hash, KeyEqual>::bucket(const K& value) const { return m_values.bucket(value); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		float UnorderedSet<K, Hash, KeyEqual>::load_factor() const noexcept { return m_values.load_factor(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		float UnorderedSet<K, Hash, KeyEqual>::max_load_factor() const noexcept { return m_values.max_load_factor(); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		void UnorderedSet<K, Hash, KeyEqual>::max_load_factor(float factor) noexcept { m_values.max_load_factor(factor); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		void UnorderedSet<K, Hash, KeyEqual>::rehash(size_type buckets) { m_values.rehash(buckets); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		void UnorderedSet<K, Hash, KeyEqual>::reserve(size_type count) { m_values.reserve(count); }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		bool UnorderedSet<K, Hash, KeyEqual>::operator==(const UnorderedSet& other) const { return m_values == other.m_values; }

		template<Type::SafeValue K, class Hash, class KeyEqual>
		bool UnorderedSet<K, Hash, KeyEqual>::operator!=(const UnorderedSet& other) const { return !(*this == other); }
	}
}
