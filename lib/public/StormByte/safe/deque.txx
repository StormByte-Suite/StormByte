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

#include <algorithm>
#include <cassert>
#include <limits>
#include <new>
#include <ranges>
#include <utility>

namespace StormByte::Safe {
	namespace {
		constexpr std::size_t BlockCount = 16;

		template<typename T>
		T* AllocateBlock() {
			return static_cast<T*>(Heap::Allocate(sizeof(T) * BlockCount));
		}
	}

	template<Type::SafeValue T>
	struct Deque<T>::Block {
		T* elements;
	};

	template<Type::SafeValue T>
	class Deque<T>::iterator {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;

			iterator() noexcept = default;

			reference operator*() const { return (*m_deque)[static_cast<size_type>(m_index)]; }
			pointer operator->() const { return &**this; }
			iterator& operator++() noexcept { ++m_index; return *this; }
			iterator operator++(int) noexcept { iterator previous = *this; ++*this; return previous; }
			iterator& operator--() noexcept { --m_index; return *this; }
			iterator operator--(int) noexcept { iterator previous = *this; --*this; return previous; }
			iterator& operator+=(difference_type delta) noexcept { m_index += delta; return *this; }
			iterator& operator-=(difference_type delta) noexcept { m_index -= delta; return *this; }
			iterator operator+(difference_type delta) const noexcept { iterator copy = *this; copy += delta; return copy; }
			iterator operator-(difference_type delta) const noexcept { iterator copy = *this; copy -= delta; return copy; }
			difference_type operator-(const iterator& other) const noexcept { return m_index - other.m_index; }
			reference operator[](difference_type delta) const { return *(*this + delta); }
			bool operator==(const iterator& other) const noexcept { return m_index == other.m_index; }
			std::strong_ordering operator<=>(const iterator& other) const noexcept { return m_index <=> other.m_index; }

		private:
			friend class Deque;
			friend class const_iterator;
			iterator(Deque* deque, std::ptrdiff_t index) noexcept : m_deque{deque}, m_index{index} {}
			Deque* m_deque = nullptr;
			std::ptrdiff_t m_index = 0;
	};

	template<Type::SafeValue T>
	class Deque<T>::const_iterator {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = const T*;
			using reference = const T&;

			const_iterator() noexcept = default;
			const_iterator(iterator other) noexcept : m_deque{other.m_deque}, m_index{other.m_index} {}

			reference operator*() const { return (*m_deque)[static_cast<size_type>(m_index)]; }
			pointer operator->() const { return &**this; }
			const_iterator& operator++() noexcept { ++m_index; return *this; }
			const_iterator operator++(int) noexcept { const_iterator previous = *this; ++*this; return previous; }
			const_iterator& operator--() noexcept { --m_index; return *this; }
			const_iterator operator--(int) noexcept { const_iterator previous = *this; --*this; return previous; }
			const_iterator& operator+=(difference_type delta) noexcept { m_index += delta; return *this; }
			const_iterator& operator-=(difference_type delta) noexcept { m_index -= delta; return *this; }
			const_iterator operator+(difference_type delta) const noexcept { const_iterator copy = *this; copy += delta; return copy; }
			const_iterator operator-(difference_type delta) const noexcept { const_iterator copy = *this; copy -= delta; return copy; }
			difference_type operator-(const const_iterator& other) const noexcept { return m_index - other.m_index; }
			reference operator[](difference_type delta) const { return *(*this + delta); }
			bool operator==(const const_iterator& other) const noexcept { return m_index == other.m_index; }
			std::strong_ordering operator<=>(const const_iterator& other) const noexcept { return m_index <=> other.m_index; }

		private:
			friend class Deque;
			const_iterator(const Deque* deque, std::ptrdiff_t index) noexcept : m_deque{deque}, m_index{index} {}
			const Deque* m_deque = nullptr;
			std::ptrdiff_t m_index = 0;
	};

	template<Type::SafeValue T>
	T& Deque<T>::Slot(size_type index) noexcept {
		const size_type absolute = m_offset + index;
		return m_map[absolute / BlockCount].elements[absolute % BlockCount];
	}

	template<Type::SafeValue T>
	const T& Deque<T>::Slot(size_type index) const noexcept {
		return const_cast<Deque*>(this)->Slot(index);
	}

	template<Type::SafeValue T>
	void Deque<T>::EnsureBack() {
		const size_type needed = (m_offset + m_size) / BlockCount + 1;
		if (m_map != nullptr && needed <= m_capacity)
			return;
		const size_type capacity = m_capacity == 0 ? 1 : std::max(needed, m_capacity * 2);
		Block* map = static_cast<Block*>(Heap::Allocate(sizeof(Block) * capacity));
		for (size_type i = 0; i < m_capacity; ++i)
			map[i] = m_map[i];
		for (size_type i = m_capacity; i < capacity; ++i)
			map[i].elements = nullptr;
		Heap::Free(m_map);
		m_map = map;
		m_capacity = capacity;
	}

	template<Type::SafeValue T>
	void Deque<T>::EnsureFront() {
		if (m_offset > 0)
			return;
		const size_type capacity = m_capacity + 1;
		Block* map = static_cast<Block*>(Heap::Allocate(sizeof(Block) * capacity));
		map[0].elements = nullptr;
		for (size_type i = 0; i < m_capacity; ++i)
			map[i + 1] = m_map[i];
		Heap::Free(m_map);
		m_map = map;
		m_capacity = capacity;
		m_offset = BlockCount;
	}

	template<Type::SafeValue T>
	Deque<T>::Deque() noexcept
	:	m_map{nullptr}, m_offset{0}, m_size{0}, m_capacity{0} {}

	template<Type::SafeValue T>
	Deque<T>::Deque(size_type count, const T& value)
	:	Deque() {
		for (size_type i = 0; i < count; ++i)
			push_back(value);
	}

	template<Type::SafeValue T>
	Deque<T>::Deque(size_type count)
	:	Deque() {
		for (size_type i = 0; i < count; ++i)
			emplace_back();
	}

	template<Type::SafeValue T>
	template<std::input_iterator InputIt>
	Deque<T>::Deque(InputIt first, InputIt last)
	:	Deque() {
		for (; first != last; ++first)
			push_back(*first);
	}

	template<Type::SafeValue T>
	Deque<T>::Deque(std::initializer_list<T> values)
	:	Deque(values.begin(), values.end()) {}

	template<Type::SafeValue T>
	Deque<T>::Deque(const std::deque<T>& values)
	:	Deque(values.begin(), values.end()) {}

	template<Type::SafeValue T>
	Deque<T>::Deque(const Deque& other)
	:	Deque(other.begin(), other.end()) {}

	template<Type::SafeValue T>
	Deque<T>::Deque(Deque&& other) noexcept
	:	m_map{other.m_map}, m_offset{other.m_offset}, m_size{other.m_size}, m_capacity{other.m_capacity} {
		other.m_map = nullptr;
		other.m_offset = 0;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	template<Type::SafeValue T>
	Deque<T>::~Deque() noexcept {
		clear();
		if (m_map != nullptr) {
			for (size_type i = 0; i < m_capacity; ++i)
				Heap::Free(m_map[i].elements);
			Heap::Free(m_map);
		}
	}

	template<Type::SafeValue T>
	Deque<T>& Deque<T>::operator=(const Deque& other) {
		if (this != &other) {
			Deque copy(other);
			swap(copy);
		}
		return *this;
	}

	template<Type::SafeValue T>
	Deque<T>& Deque<T>::operator=(Deque&& other) noexcept {
		if (this != &other) {
			Deque taken(std::move(other));
			swap(taken);
		}
		return *this;
	}

	template<Type::SafeValue T>
	Deque<T>& Deque<T>::operator=(std::initializer_list<T> values) {
		Deque copy(values);
		swap(copy);
		return *this;
	}

	template<Type::SafeValue T>
	Deque<T>& Deque<T>::operator=(const std::deque<T>& values) {
		Deque copy(values);
		swap(copy);
		return *this;
	}

	template<Type::SafeValue T>
	void Deque<T>::assign(size_type count, const T& value) {
		Deque copy(count, value);
		swap(copy);
	}

	template<Type::SafeValue T>
	template<std::input_iterator InputIt>
	void Deque<T>::assign(InputIt first, InputIt last) {
		Deque copy(first, last);
		swap(copy);
	}

	template<Type::SafeValue T>
	void Deque<T>::assign(std::initializer_list<T> values) {
		assign(values.begin(), values.end());
	}

	template<Type::SafeValue T>
	template<std::ranges::input_range R>
	void Deque<T>::assign_range(R&& range) {
		assign(std::ranges::begin(range), std::ranges::end(range));
	}

	template<Type::SafeValue T>
	T& Deque<T>::at(size_type index) {
		if (index >= m_size)
			ThrowDequeOutOfBounds();
		return Slot(index);
	}

	template<Type::SafeValue T>
	const T& Deque<T>::at(size_type index) const {
		return const_cast<Deque*>(this)->at(index);
	}

	template<Type::SafeValue T>
	T& Deque<T>::operator[](size_type index) noexcept {
		assert(index < m_size);
		return Slot(index);
	}

	template<Type::SafeValue T>
	const T& Deque<T>::operator[](size_type index) const noexcept {
		return const_cast<Deque*>(this)->operator[](index);
	}

	template<Type::SafeValue T>
	T& Deque<T>::front() { return at(0); }

	template<Type::SafeValue T>
	const T& Deque<T>::front() const { return at(0); }

	template<Type::SafeValue T>
	T& Deque<T>::back() { return at(m_size - 1); }

	template<Type::SafeValue T>
	const T& Deque<T>::back() const { return at(m_size - 1); }

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::begin() noexcept { return iterator(this, 0); }

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::end() noexcept { return iterator(this, static_cast<std::ptrdiff_t>(m_size)); }

	template<Type::SafeValue T>
	typename Deque<T>::const_iterator Deque<T>::begin() const noexcept { return const_iterator(this, 0); }

	template<Type::SafeValue T>
	typename Deque<T>::const_iterator Deque<T>::end() const noexcept { return const_iterator(this, static_cast<std::ptrdiff_t>(m_size)); }

	template<Type::SafeValue T>
	typename Deque<T>::const_iterator Deque<T>::cbegin() const noexcept { return begin(); }

	template<Type::SafeValue T>
	typename Deque<T>::const_iterator Deque<T>::cend() const noexcept { return end(); }

	template<Type::SafeValue T>
	bool Deque<T>::empty() const noexcept { return m_size == 0; }

	template<Type::SafeValue T>
	typename Deque<T>::size_type Deque<T>::size() const noexcept { return m_size; }

	template<Type::SafeValue T>
	void Deque<T>::shrink_to_fit() {}

	template<Type::SafeValue T>
	void Deque<T>::clear() noexcept {
		while (!empty())
			pop_back();
	}

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::insert(const_iterator position, const T& value) {
		return emplace(position, value);
	}

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::insert(const_iterator position, T&& value) {
		return emplace(position, std::move(value));
	}

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::insert(const_iterator position, size_type count, const T& value) {
		const std::ptrdiff_t index = position.m_index;
		for (size_type i = 0; i < count; ++i)
			insert(const_iterator(this, index + static_cast<std::ptrdiff_t>(i)), value);
		return iterator(this, index);
	}

	template<Type::SafeValue T>
	template<std::input_iterator InputIt>
	typename Deque<T>::iterator Deque<T>::insert(const_iterator position, InputIt first, InputIt last) {
		const std::ptrdiff_t index = position.m_index;
		std::ptrdiff_t written = 0;
		for (; first != last; ++first, ++written)
			insert(const_iterator(this, index + written), *first);
		return iterator(this, index);
	}

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::insert(const_iterator position, std::initializer_list<T> values) {
		return insert(position, values.begin(), values.end());
	}

	template<Type::SafeValue T>
	template<std::ranges::input_range R>
	typename Deque<T>::iterator Deque<T>::insert_range(const_iterator position, R&& range) {
		return insert(position, std::ranges::begin(range), std::ranges::end(range));
	}

	template<Type::SafeValue T>
	template<typename... Args>
	typename Deque<T>::iterator Deque<T>::emplace(const_iterator position, Args&&... args) {
		const std::ptrdiff_t index = position.m_index;
		emplace_back();
		for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(m_size) - 1; i > index; --i)
			Slot(static_cast<size_type>(i)) = std::move(Slot(static_cast<size_type>(i - 1)));
		Slot(static_cast<size_type>(index)) = T(std::forward<Args>(args)...);
		return iterator(this, index);
	}

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::erase(const_iterator position) {
		return erase(position, const_iterator(this, position.m_index + 1));
	}

	template<Type::SafeValue T>
	typename Deque<T>::iterator Deque<T>::erase(const_iterator first, const_iterator last) {
		const std::ptrdiff_t index = first.m_index;
		const std::ptrdiff_t count = last.m_index - first.m_index;
		for (std::ptrdiff_t i = index; i + count < static_cast<std::ptrdiff_t>(m_size); ++i)
			Slot(static_cast<size_type>(i)) = std::move(Slot(static_cast<size_type>(i + count)));
		for (std::ptrdiff_t i = 0; i < count; ++i)
			pop_back();
		return iterator(this, index);
	}

	template<Type::SafeValue T>
	void Deque<T>::push_back(const T& value) { emplace_back(value); }

	template<Type::SafeValue T>
	void Deque<T>::push_back(T&& value) { emplace_back(std::move(value)); }

	template<Type::SafeValue T>
	template<typename... Args>
	T& Deque<T>::emplace_back(Args&&... args) {
		EnsureBack();
		const size_type absolute = m_offset + m_size;
		Block& block = m_map[absolute / BlockCount];
		if (block.elements == nullptr)
			block.elements = AllocateBlock<T>();
		T* slot = block.elements + absolute % BlockCount;
		new (slot) T(std::forward<Args>(args)...);
		++m_size;
		return *slot;
	}

	template<Type::SafeValue T>
	void Deque<T>::pop_back() {
		assert(!empty());
		Slot(m_size - 1).~T();
		--m_size;
	}

	template<Type::SafeValue T>
	void Deque<T>::push_front(const T& value) { emplace_front(value); }

	template<Type::SafeValue T>
	void Deque<T>::push_front(T&& value) { emplace_front(std::move(value)); }

	template<Type::SafeValue T>
	template<typename... Args>
	T& Deque<T>::emplace_front(Args&&... args) {
		EnsureFront();
		--m_offset;
		const size_type absolute = m_offset;
		Block& block = m_map[absolute / BlockCount];
		if (block.elements == nullptr)
			block.elements = AllocateBlock<T>();
		T* slot = block.elements + absolute % BlockCount;
		new (slot) T(std::forward<Args>(args)...);
		++m_size;
		return *slot;
	}

	template<Type::SafeValue T>
	void Deque<T>::pop_front() {
		assert(!empty());
		Slot(0).~T();
		++m_offset;
		--m_size;
	}

	template<Type::SafeValue T>
	template<std::ranges::input_range R>
	void Deque<T>::append_range(R&& range) {
		for (auto&& value : range)
			emplace_back(std::forward<decltype(value)>(value));
	}

	template<Type::SafeValue T>
	template<std::ranges::input_range R>
	void Deque<T>::prepend_range(R&& range) {
		std::ptrdiff_t index = 0;
		for (auto&& value : range) {
			emplace(const_iterator(this, index), std::forward<decltype(value)>(value));
			++index;
		}
	}

	template<Type::SafeValue T>
	void Deque<T>::resize(size_type count) {
		while (m_size > count)
			pop_back();
		while (m_size < count)
			emplace_back();
	}

	template<Type::SafeValue T>
	void Deque<T>::resize(size_type count, const T& value) {
		while (m_size > count)
			pop_back();
		while (m_size < count)
			push_back(value);
	}

	template<Type::SafeValue T>
	void Deque<T>::swap(Deque& other) noexcept {
		std::swap(m_map, other.m_map);
		std::swap(m_offset, other.m_offset);
		std::swap(m_size, other.m_size);
		std::swap(m_capacity, other.m_capacity);
	}

	template<Type::SafeValue T>
	bool operator==(const Deque<T>& left, const Deque<T>& right) {
		return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin());
	}

	template<Type::SafeValue T>
	std::strong_ordering operator<=>(const Deque<T>& left, const Deque<T>& right) {
		return std::lexicographical_compare_three_way(left.begin(), left.end(), right.begin(), right.end());
	}

	template<Type::SafeValue T>
	void swap(Deque<T>& left, Deque<T>& right) noexcept {
		left.swap(right);
	}
}
