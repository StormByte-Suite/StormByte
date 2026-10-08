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
		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::Set() noexcept = default;

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::Set(const Compare& compare) noexcept: m_values(compare) {}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::Set(const Set& other): m_values(other.m_values) {}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::Set(Set&& other) noexcept: m_values(std::move(other.m_values)) {}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::~Set() = default;

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>& Set<K, Compare>::operator=(const Set& other) {
			m_values = other.m_values;
			return *this;
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>& Set<K, Compare>::operator=(Set&& other) noexcept {
			m_values = std::move(other.m_values);
			return *this;
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>& Set<K, Compare>::operator=(std::initializer_list<K> values) {
			Set replacement(values, key_comp());
			swap(replacement);
			return *this;
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class InputIt>
		Set<K, Compare>::Set(InputIt first, InputIt last) {
			insert(first, last);
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class InputIt>
		Set<K, Compare>::Set(InputIt first, InputIt last, const Compare& compare): m_values(compare) {
			insert(first, last);
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::Set(std::initializer_list<K> values) {
			insert(values);
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Set<K, Compare>::Set(std::initializer_list<K> values, const Compare& compare): m_values(compare) {
			insert(values);
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::key_compare Set<K, Compare>::key_comp() const { return m_values.key_comp(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::value_compare Set<K, Compare>::value_comp() const { return key_comp(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::begin() noexcept { return iterator(m_values.begin()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::begin() const noexcept { return const_iterator(m_values.begin()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::cbegin() const noexcept { return begin(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::end() noexcept { return iterator(m_values.end()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::end() const noexcept { return const_iterator(m_values.end()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::cend() const noexcept { return end(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::reverse_iterator Set<K, Compare>::rbegin() noexcept { return reverse_iterator(end()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_reverse_iterator Set<K, Compare>::rbegin() const noexcept { return const_reverse_iterator(end()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_reverse_iterator Set<K, Compare>::crbegin() const noexcept { return rbegin(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::reverse_iterator Set<K, Compare>::rend() noexcept { return reverse_iterator(begin()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_reverse_iterator Set<K, Compare>::rend() const noexcept { return const_reverse_iterator(begin()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_reverse_iterator Set<K, Compare>::crend() const noexcept { return rend(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::empty() const noexcept { return m_values.empty(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::size_type Set<K, Compare>::size() const noexcept { return m_values.size(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::size_type Set<K, Compare>::max_size() const noexcept { return m_values.max_size(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Set<K, Compare>::clear() noexcept { m_values.clear(); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::insert_result Set<K, Compare>::insert(const K& value) {
			auto result = m_values.emplace(value, Monostate{});
			return insert_result{iterator(result.first), result.second};
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::insert_result Set<K, Compare>::insert(K&& value) {
			auto result = m_values.emplace(std::move(value), Monostate{});
			return insert_result{iterator(result.first), result.second};
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::insert(const_iterator hint, const K& value) {
			return iterator(m_values.emplace_hint(hint.m_cursor, value, Monostate{}));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::insert(const_iterator hint, K&& value) {
			return iterator(m_values.emplace_hint(hint.m_cursor, std::move(value), Monostate{}));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class InputIt>
		void Set<K, Compare>::insert(InputIt first, InputIt last) {
			for (; first != last; ++first)
				insert(value_type(*first));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Set<K, Compare>::insert(std::initializer_list<K> values) { insert(values.begin(), values.end()); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		typename Set<K, Compare>::insert_result Set<K, Compare>::emplace(Args&&... args) {
			return insert(K(std::forward<Args>(args)...));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class... Args>
		typename Set<K, Compare>::iterator Set<K, Compare>::emplace_hint(const_iterator hint, Args&&... args) {
			return insert(hint, K(std::forward<Args>(args)...));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::insert_return_type Set<K, Compare>::insert(node_type&& handle) {
			auto result = m_values.insert(std::move(handle.m_node));
			return insert_return_type{iterator(result.position), result.inserted, node_type(std::move(result.node))};
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::insert(const_iterator hint, node_type&& handle) {
			return iterator(m_values.insert(hint.m_cursor, std::move(handle.m_node)));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::node_type Set<K, Compare>::extract(const_iterator position) {
			return node_type(m_values.extract(position.m_cursor));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::node_type Set<K, Compare>::extract(const K& value) {
			return node_type(m_values.extract(value));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		template<class OtherCompare>
		void Set<K, Compare>::merge(Set<K, OtherCompare>& source) {
			auto cursor = source.begin();
			while (cursor != source.end()) {
				if (contains(*cursor)) {
					++cursor;
					continue;
				}
				auto next = std::next(cursor);
				if constexpr (std::same_as<Compare, OtherCompare>)
					insert(source.extract(cursor));
				else {
					K value = *cursor;
					source.erase(cursor);
					insert(std::move(value));
				}
				cursor = next;
			}
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::erase(iterator position) noexcept {
			return iterator(m_values.erase(position.m_cursor));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::erase(const_iterator position) noexcept {
			return iterator(m_values.erase(position.m_cursor));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::erase(const_iterator first, const_iterator last) noexcept {
			return iterator(m_values.erase(first.m_cursor, last.m_cursor));
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::size_type Set<K, Compare>::erase(const K& value) noexcept { return m_values.erase(value); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		void Set<K, Compare>::swap(Set& other) noexcept { m_values.swap(other.m_values); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::find(const K& value) { return iterator(m_values.find(value)); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::find(const K& value) const { return const_iterator(m_values.find(value)); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::size_type Set<K, Compare>::count(const K& value) const { return m_values.count(value); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::contains(const K& value) const { return m_values.contains(value); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::lower_bound(const K& value) { return iterator(m_values.lower_bound(value)); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::lower_bound(const K& value) const { return const_iterator(m_values.lower_bound(value)); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::iterator Set<K, Compare>::upper_bound(const K& value) { return iterator(m_values.upper_bound(value)); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		typename Set<K, Compare>::const_iterator Set<K, Compare>::upper_bound(const K& value) const { return const_iterator(m_values.upper_bound(value)); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Pair<typename Set<K, Compare>::iterator, typename Set<K, Compare>::iterator> Set<K, Compare>::equal_range(const K& value) {
			auto range = m_values.equal_range(value);
			return Pair<iterator, iterator>{iterator(range.first), iterator(range.second)};
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		Pair<typename Set<K, Compare>::const_iterator, typename Set<K, Compare>::const_iterator> Set<K, Compare>::equal_range(const K& value) const {
			auto range = m_values.equal_range(value);
			return Pair<const_iterator, const_iterator>{const_iterator(range.first), const_iterator(range.second)};
		}

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::operator==(const Set& other) const { return m_values == other.m_values; }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::operator!=(const Set& other) const { return !(*this == other); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::operator<(const Set& other) const { return m_values < other.m_values; }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::operator<=(const Set& other) const { return !(other < *this); }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::operator>(const Set& other) const { return other < *this; }

		template<Type::SafeValue K, class Compare>
		requires std::strict_weak_order<Compare, const K&, const K&>
		bool Set<K, Compare>::operator>=(const Set& other) const { return !(*this < other); }
	}
}
