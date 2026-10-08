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

namespace StormByte::Safe {
	template<Type::SafeValue T>
	List<T>::List() noexcept { ResetSentinel(); }

	template<Type::SafeValue T>
	List<T>::List(size_type count): List() {
		for (size_type index = 0; index < count; ++index)
			emplace_back();
	}

	template<Type::SafeValue T>
	List<T>::List(size_type count, const T& value): List() {
		for (size_type index = 0; index < count; ++index)
			push_back(value);
	}

	template<Type::SafeValue T>
	List<T>::List(const List& other): List() {
		for (const T& value : other)
			push_back(value);
	}

	template<Type::SafeValue T>
	List<T>::List(List&& other) noexcept: m_size(other.m_size) {
		if (other.empty()) {
			ResetSentinel();
			return;
		}
		m_sentinel.Next = other.m_sentinel.Next;
		m_sentinel.Prev = other.m_sentinel.Prev;
		m_sentinel.Next->Prev = &m_sentinel;
		m_sentinel.Prev->Next = &m_sentinel;
		other.ResetSentinel();
		other.m_size = 0;
	}

	template<Type::SafeValue T>
	List<T>::List(std::initializer_list<T> values): List() {
		for (const T& value : values)
			push_back(value);
	}

	template<Type::SafeValue T>
	List<T>::~List() noexcept { Destroy(); }

	template<Type::SafeValue T>
	List<T>& List<T>::operator=(const List& other) {
		if (this != &other) {
			List replacement(other);
			swap(replacement);
		}
		return *this;
	}

	template<Type::SafeValue T>
	List<T>& List<T>::operator=(List&& other) noexcept {
		if (this != &other) {
			Destroy();
			if (other.empty()) {
				ResetSentinel();
				m_size = 0;
			} else {
				m_sentinel.Next = other.m_sentinel.Next;
				m_sentinel.Prev = other.m_sentinel.Prev;
				m_sentinel.Next->Prev = &m_sentinel;
				m_sentinel.Prev->Next = &m_sentinel;
				m_size = other.m_size;
				other.ResetSentinel();
				other.m_size = 0;
			}
		}
		return *this;
	}

	template<Type::SafeValue T>
	List<T>& List<T>::operator=(std::initializer_list<T> values) {
		List replacement(values);
		swap(replacement);
		return *this;
	}

	template<Type::SafeValue T>
	void List<T>::assign(size_type count, const T& value) {
		List replacement(count, value);
		swap(replacement);
	}

	template<Type::SafeValue T>
	void List<T>::assign(std::initializer_list<T> values) {
		*this = values;
	}

	template<Type::SafeValue T>
	void List<T>::clear() noexcept { Destroy(); }

	template<Type::SafeValue T>
	List<T>::iterator List<T>::insert(const_iterator position, const T& value) {
		return emplace(position, value);
	}

	template<Type::SafeValue T>
	List<T>::iterator List<T>::insert(const_iterator position, T&& value) {
		return emplace(position, std::move(value));
	}

	template<Type::SafeValue T>
	List<T>::iterator List<T>::insert(const_iterator position, size_type count, const T& value) {
		iterator first(const_cast<Link*>(position.m_link));
		for (size_type index = 0; index < count; ++index) {
			iterator inserted = insert(position, value);
			if (index == 0)
				first = inserted;
		}
		return first;
	}

	template<Type::SafeValue T>
	List<T>::iterator List<T>::insert(const_iterator position, std::initializer_list<T> values) {
		return insert_range(position, values);
	}

	template<Type::SafeValue T>
	template<class... Args>
	List<T>::iterator List<T>::emplace(const_iterator position, Args&&... args) {
		Node* node = MakeNode(std::forward<Args>(args)...);
		LinkBefore(const_cast<Link*>(position.m_link), node);
		++m_size;
		return iterator(node);
	}

	template<Type::SafeValue T>
	List<T>::iterator List<T>::erase(const_iterator position) {
		Node* node = static_cast<Node*>(const_cast<Link*>(position.m_link));
		Link* next = node->Next;
		Unlink(node);
		node->Value.~T();
		Heap::Free(node);
		--m_size;
		return iterator(next);
	}

	template<Type::SafeValue T>
	List<T>::iterator List<T>::erase(const_iterator first, const_iterator last) {
		while (first != last)
			first = erase(first);
		return iterator(const_cast<Link*>(last.m_link));
	}

	template<Type::SafeValue T>
	void List<T>::push_front(const T& value) { emplace(begin(), value); }

	template<Type::SafeValue T>
	void List<T>::push_front(T&& value) { emplace(begin(), std::move(value)); }

	template<Type::SafeValue T>
	template<class... Args>
	T& List<T>::emplace_front(Args&&... args) { return *emplace(begin(), std::forward<Args>(args)...); }

	template<Type::SafeValue T>
	void List<T>::pop_front() noexcept { assert(!empty()); erase(begin()); }

	template<Type::SafeValue T>
	void List<T>::push_back(const T& value) { emplace(end(), value); }

	template<Type::SafeValue T>
	void List<T>::push_back(T&& value) { emplace(end(), std::move(value)); }

	template<Type::SafeValue T>
	template<class... Args>
	T& List<T>::emplace_back(Args&&... args) { return *emplace(end(), std::forward<Args>(args)...); }

	template<Type::SafeValue T>
	void List<T>::pop_back() noexcept { assert(!empty()); erase(std::prev(end())); }

	template<Type::SafeValue T>
	void List<T>::resize(size_type count) {
		while (m_size > count)
			pop_back();
		while (m_size < count)
			emplace_back();
	}

	template<Type::SafeValue T>
	void List<T>::resize(size_type count, const T& value) {
		while (m_size > count)
			pop_back();
		while (m_size < count)
			push_back(value);
	}

	template<Type::SafeValue T>
	void List<T>::swap(List& other) noexcept {
		if (this == &other)
			return;
		List temporary(std::move(*this));
		*this = std::move(other);
		other = std::move(temporary);
	}

	template<Type::SafeValue T>
	void List<T>::splice(const_iterator position, List& other) noexcept {
		splice(position, other, other.begin(), other.end());
	}

	template<Type::SafeValue T>
	void List<T>::splice(const_iterator position, List& other, const_iterator source) noexcept {
		const_iterator next = source;
		++next;
		splice(position, other, source, next);
	}

	template<Type::SafeValue T>
	void List<T>::splice(const_iterator position, List& other, const_iterator first, const_iterator last) noexcept {
		if (first == last)
			return;
		size_type count = 0;
		for (const_iterator cursor = first; cursor != last; ++cursor)
			++count;
		Link* before = const_cast<Link*>(first.m_link);
		Link* after = const_cast<Link*>(last.m_link)->Prev;
		before->Prev->Next = after->Next;
		after->Next->Prev = before->Prev;
		other.m_size -= count;
		Link* at = const_cast<Link*>(position.m_link);
		before->Prev = at->Prev;
		after->Next = at;
		at->Prev->Next = before;
		at->Prev = after;
		m_size += count;
	}

	template<Type::SafeValue T>
	List<T>::node_type List<T>::extract(const_iterator position) {
		Node* node = static_cast<Node*>(const_cast<Link*>(position.m_link));
		Unlink(node);
		--m_size;
		return node_type(node);
	}

	template<Type::SafeValue T>
	List<T>::iterator List<T>::insert(const_iterator position, node_type&& node) {
		if (node.empty())
			return iterator(const_cast<Link*>(position.m_link));
		Node* taken = node.m_node;
		node.m_node = nullptr;
		LinkBefore(const_cast<Link*>(position.m_link), taken);
		++m_size;
		return iterator(taken);
	}

	template<Type::SafeValue T>
	void List<T>::merge(List& other) { merge(other, std::less<T>{}); }

	template<Type::SafeValue T>
	template<class Compare>
	void List<T>::merge(List& other, Compare compare) {
		if (this == &other)
			return;
		iterator mine = begin();
		while (!other.empty()) {
			if (mine == end() || compare(other.front(), *mine))
				splice(mine, other, other.begin());
			else
				++mine;
		}
	}

	template<Type::SafeValue T>
	List<T>::size_type List<T>::remove(const T& value) {
		return remove_if([&value](const T& current) { return current == value; });
	}

	template<Type::SafeValue T>
	template<class Predicate>
	List<T>::size_type List<T>::remove_if(Predicate predicate) {
		size_type erased = 0;
		iterator cursor = begin();
		while (cursor != end()) {
			if (predicate(*cursor)) {
				cursor = erase(cursor);
				++erased;
			} else
				++cursor;
		}
		return erased;
	}

	template<Type::SafeValue T>
	void List<T>::reverse() noexcept {
		Link* cursor = &m_sentinel;
		do {
			std::swap(cursor->Next, cursor->Prev);
			cursor = cursor->Prev;
		} while (cursor != &m_sentinel);
	}

	template<Type::SafeValue T>
	List<T>::size_type List<T>::unique() {
		return unique(std::equal_to<T>{});
	}

	template<Type::SafeValue T>
	template<class Predicate>
	List<T>::size_type List<T>::unique(Predicate predicate) {
		if (size() < 2)
			return 0;
		size_type erased = 0;
		iterator cursor = begin();
		iterator next = std::next(cursor);
		while (next != end()) {
			if (predicate(*cursor, *next)) {
				next = erase(next);
				++erased;
			} else {
				cursor = next;
				++next;
			}
		}
		return erased;
	}

	template<Type::SafeValue T>
	void List<T>::sort() { sort(std::less<T>{}); }

	template<Type::SafeValue T>
	template<class Compare>
	void List<T>::sort(Compare compare) {
		if (size() < 2)
			return;
		List left;
		List right;
		size_type half = m_size / 2;
		const_iterator middle = begin();
		for (size_type index = 0; index < half; ++index)
			++middle;
		right.splice(right.end(), *this, middle, end());
		left.splice(left.end(), *this);
		left.sort(compare);
		right.sort(compare);
		left.merge(right, compare);
		splice(end(), left);
	}

	template<Type::SafeValue T>
	bool List<T>::operator==(const List& other) const {
		return operator<=>(other) == 0;
	}

	template<Type::SafeValue T>
	std::strong_ordering List<T>::operator<=>(const List& other) const {
		const_iterator left = begin();
		const_iterator right = other.begin();
		while (left != end() && right != other.end()) {
			if (*left < *right)
				return std::strong_ordering::less;
			if (*right < *left)
				return std::strong_ordering::greater;
			++left;
			++right;
		}
		if (left == end() && right == other.end())
			return std::strong_ordering::equal;
		return left == end() ? std::strong_ordering::less : std::strong_ordering::greater;
	}

	template<Type::SafeValue T>
	void List<T>::ResetSentinel() noexcept {
		m_sentinel.Next = &m_sentinel;
		m_sentinel.Prev = &m_sentinel;
	}

	template<Type::SafeValue T>
	template<class... Args>
	List<T>::Node* List<T>::MakeNode(Args&&... args) {
		void* storage = Heap::Allocate(sizeof(Node));
		if (storage == nullptr)
			throw AllocationError();
		try {
			return new (storage) Node(std::forward<Args>(args)...);
		} catch (...) {
			Heap::Free(storage);
			throw;
		}
	}

	template<Type::SafeValue T>
	void List<T>::LinkBefore(Link* position, Node* node) noexcept {
		node->Next = position;
		node->Prev = position->Prev;
		position->Prev->Next = node;
		position->Prev = node;
	}

	template<Type::SafeValue T>
	void List<T>::Unlink(Node* node) noexcept {
		node->Prev->Next = node->Next;
		node->Next->Prev = node->Prev;
	}

	template<Type::SafeValue T>
	void List<T>::Destroy() noexcept {
		Link* cursor = m_sentinel.Next;
		while (cursor != &m_sentinel) {
			Node* node = static_cast<Node*>(cursor);
			cursor = cursor->Next;
			node->Value.~T();
			Heap::Free(node);
		}
		ResetSentinel();
		m_size = 0;
	}
}
