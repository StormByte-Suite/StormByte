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

#include <new>
#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeValue T>
		Vector<T>::Vector() noexcept: m_data(nullptr), m_size(0), m_capacity(0) {}

		template<Type::SafeValue T>
		Vector<T>::Vector(size_type count): m_data(nullptr), m_size(0), m_capacity(0) {
			resize(count);
		}

		template<Type::SafeValue T>
		Vector<T>::Vector(size_type count, const T& value): m_data(nullptr), m_size(0), m_capacity(0) {
			assign(count, value);
		}

		template<Type::SafeValue T>
		Vector<T>::Vector(std::initializer_list<T> values): m_data(nullptr), m_size(0), m_capacity(0) {
			assign(values);
		}

		template<Type::SafeValue T>
		Vector<T>::Vector(const std::vector<T>& values): m_data(nullptr), m_size(0), m_capacity(0) {
			reserve(values.size());
			for (const T& value : values)
				push_back(value);
		}

		template<Type::SafeValue T>
		Vector<T>::Vector(const Vector& other): m_data(nullptr), m_size(0), m_capacity(0) {
			reserve(other.m_size);
			for (const T& value : other)
				push_back(value);
		}

		template<Type::SafeValue T>
		Vector<T>::Vector(Vector&& other) noexcept: m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
			other.m_data = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;
		}

		template<Type::SafeValue T>
		Vector<T>::~Vector() noexcept {
			Release(m_data, m_size);
		}

		template<Type::SafeValue T>
		Vector<T>& Vector<T>::operator=(const Vector& other) {
			if (this == &other)
				return *this;
			Vector copy(other);
			swap(copy);
			return *this;
		}

		template<Type::SafeValue T>
		Vector<T>& Vector<T>::operator=(Vector&& other) noexcept {
			if (this == &other)
				return *this;
			Release(m_data, m_size);
			m_data = other.m_data;
			m_size = other.m_size;
			m_capacity = other.m_capacity;
			other.m_data = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;
			return *this;
		}

		template<Type::SafeValue T>
		Vector<T>& Vector<T>::operator=(std::initializer_list<T> values) {
			assign(values);
			return *this;
		}

		template<Type::SafeValue T>
		Vector<T>& Vector<T>::operator=(const std::vector<T>& values) {
			Vector copy(values);
			swap(copy);
			return *this;
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::begin() noexcept {
			return m_data;
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::end() noexcept {
			return m_data + m_size;
		}

		template<Type::SafeValue T>
		Vector<T>::const_iterator Vector<T>::begin() const noexcept {
			return m_data;
		}

		template<Type::SafeValue T>
		Vector<T>::const_iterator Vector<T>::end() const noexcept {
			return m_data + m_size;
		}

		template<Type::SafeValue T>
		Vector<T>::const_iterator Vector<T>::cbegin() const noexcept {
			return begin();
		}

		template<Type::SafeValue T>
		Vector<T>::const_iterator Vector<T>::cend() const noexcept {
			return end();
		}

		template<Type::SafeValue T>
		bool Vector<T>::empty() const noexcept {
			return m_size == 0;
		}

		template<Type::SafeValue T>
		Vector<T>::size_type Vector<T>::size() const noexcept {
			return m_size;
		}

		template<Type::SafeValue T>
		Vector<T>::size_type Vector<T>::capacity() const noexcept {
			return m_capacity;
		}

		template<Type::SafeValue T>
		Vector<T>::pointer Vector<T>::data() noexcept {
			return m_data;
		}

		template<Type::SafeValue T>
		Vector<T>::const_pointer Vector<T>::data() const noexcept {
			return m_data;
		}

		template<Type::SafeValue T>
		void Vector<T>::reserve(size_type count) {
			if (count <= m_capacity)
				return;
			T* block = Allocate(count);
			for (size_type index = 0; index < m_size; ++index) {
				new (block + index) T(std::move(m_data[index]));
				m_data[index].~T();
			}
			Heap::Free(m_data);
			m_data = block;
			m_capacity = count;
		}

		template<Type::SafeValue T>
		void Vector<T>::shrink_to_fit() {
			if (m_size == m_capacity)
				return;
			Vector copy(*this);
			swap(copy);
		}

		template<Type::SafeValue T>
		void Vector<T>::clear() noexcept {
			for (size_type index = 0; index < m_size; ++index)
				m_data[index].~T();
			m_size = 0;
		}

		template<Type::SafeValue T>
		void Vector<T>::swap(Vector& other) noexcept {
			using std::swap;
			swap(m_data, other.m_data);
			swap(m_size, other.m_size);
			swap(m_capacity, other.m_capacity);
		}

		template<Type::SafeValue T>
		Vector<T>::reference Vector<T>::operator[](size_type index) noexcept {
			return m_data[index];
		}

		template<Type::SafeValue T>
		Vector<T>::const_reference Vector<T>::operator[](size_type index) const noexcept {
			return m_data[index];
		}

		template<Type::SafeValue T>
		Vector<T>::reference Vector<T>::at(size_type index) {
			if (index >= m_size)
				ThrowVectorOutOfBounds();
			return m_data[index];
		}

		template<Type::SafeValue T>
		Vector<T>::const_reference Vector<T>::at(size_type index) const {
			if (index >= m_size)
				ThrowVectorOutOfBounds();
			return m_data[index];
		}

		template<Type::SafeValue T>
		Vector<T>::reference Vector<T>::front() {
			if (m_size == 0)
				ThrowVectorOutOfBounds();
			return m_data[0];
		}

		template<Type::SafeValue T>
		Vector<T>::const_reference Vector<T>::front() const {
			if (m_size == 0)
				ThrowVectorOutOfBounds();
			return m_data[0];
		}

		template<Type::SafeValue T>
		Vector<T>::reference Vector<T>::back() {
			if (m_size == 0)
				ThrowVectorOutOfBounds();
			return m_data[m_size - 1];
		}

		template<Type::SafeValue T>
		Vector<T>::const_reference Vector<T>::back() const {
			if (m_size == 0)
				ThrowVectorOutOfBounds();
			return m_data[m_size - 1];
		}

		template<Type::SafeValue T>
		void Vector<T>::push_back(const T& value) {
			if (m_size == m_capacity)
				Grow();
			new (m_data + m_size) T(value);
			++m_size;
		}

		template<Type::SafeValue T>
		void Vector<T>::push_back(T&& value) {
			if (m_size == m_capacity)
				Grow();
			new (m_data + m_size) T(std::move(value));
			++m_size;
		}

		template<Type::SafeValue T>
		void Vector<T>::pop_back() {
			if (m_size == 0)
				ThrowVectorOutOfBounds();
			--m_size;
			m_data[m_size].~T();
		}

		template<Type::SafeValue T>
		template<class... Args>
		Vector<T>::reference Vector<T>::emplace_back(Args&&... args) {
			if (m_size == m_capacity)
				Grow();
			new (m_data + m_size) T(std::forward<Args>(args)...);
			++m_size;
			return m_data[m_size - 1];
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::insert(const_iterator position, const T& value) {
			return insert(position, 1, value);
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::insert(const_iterator position, T&& value) {
			const size_type index = static_cast<size_type>(position - m_data);
			if (m_size == m_capacity)
				Grow();
			for (size_type cursor = m_size; cursor > index; --cursor) {
				new (m_data + cursor) T(std::move(m_data[cursor - 1]));
				m_data[cursor - 1].~T();
			}
			new (m_data + index) T(std::move(value));
			++m_size;
			return m_data + index;
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::insert(const_iterator position, size_type count, const T& value) {
			const size_type index = static_cast<size_type>(position - m_data);
			if (count == 0)
				return m_data + index;
			if (m_size + count > m_capacity)
				reserve(m_capacity * 2 > m_size + count ? m_capacity * 2 : m_size + count);
			for (size_type cursor = m_size; cursor > index; --cursor) {
				new (m_data + cursor + count - 1) T(std::move(m_data[cursor - 1]));
				m_data[cursor - 1].~T();
			}
			for (size_type cursor = 0; cursor < count; ++cursor)
				new (m_data + index + cursor) T(value);
			m_size += count;
			return m_data + index;
		}

		template<Type::SafeValue T>
		template<class... Args>
		Vector<T>::iterator Vector<T>::emplace(const_iterator position, Args&&... args) {
			const size_type index = static_cast<size_type>(position - m_data);
			if (m_size == m_capacity)
				Grow();
			for (size_type cursor = m_size; cursor > index; --cursor) {
				new (m_data + cursor) T(std::move(m_data[cursor - 1]));
				m_data[cursor - 1].~T();
			}
			new (m_data + index) T(std::forward<Args>(args)...);
			++m_size;
			return m_data + index;
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::erase(const_iterator position) {
			return erase(position, position + 1);
		}

		template<Type::SafeValue T>
		Vector<T>::iterator Vector<T>::erase(const_iterator first, const_iterator last) {
			const size_type index = static_cast<size_type>(first - m_data);
			const size_type count = static_cast<size_type>(last - first);
			for (size_type cursor = 0; cursor < count; ++cursor)
				m_data[index + cursor].~T();
			for (size_type cursor = index + count; cursor < m_size; ++cursor) {
				new (m_data + cursor - count) T(std::move(m_data[cursor]));
				m_data[cursor].~T();
			}
			m_size -= count;
			return m_data + index;
		}

		template<Type::SafeValue T>
		void Vector<T>::assign(size_type count, const T& value) {
			clear();
			reserve(count);
			for (size_type index = 0; index < count; ++index)
				push_back(value);
		}

		template<Type::SafeValue T>
		void Vector<T>::assign(std::initializer_list<T> values) {
			clear();
			reserve(values.size());
			for (const T& value : values)
				push_back(value);
		}

		template<Type::SafeValue T>
		void Vector<T>::resize(size_type count) {
			if (count < m_size) {
				erase(m_data + count, end());
				return;
			}
			reserve(count);
			while (m_size < count)
				emplace_back();
		}

		template<Type::SafeValue T>
		void Vector<T>::resize(size_type count, const T& value) {
			if (count < m_size) {
				erase(m_data + count, end());
				return;
			}
			reserve(count);
			while (m_size < count)
				push_back(value);
		}

		template<Type::SafeValue T>
		bool Vector<T>::operator==(const Vector& other) const requires Type::EqualityComparable<T> {
			if (m_size != other.m_size)
				return false;
			for (size_type index = 0; index < m_size; ++index) {
				if (!(m_data[index] == other.m_data[index]))
					return false;
			}
			return true;
		}

		template<Type::SafeValue T>
		T* Vector<T>::Allocate(size_type capacity) {
			if (capacity == 0)
				return nullptr;
			return static_cast<T*>(Heap::Allocate(capacity * sizeof(T)));
		}

		template<Type::SafeValue T>
		void Vector<T>::Release(T* data, size_type count) noexcept {
			if (data == nullptr)
				return;
			for (size_type index = 0; index < count; ++index)
				data[index].~T();
			Heap::Free(data);
		}

		template<Type::SafeValue T>
		void Vector<T>::Grow() {
			reserve(m_capacity == 0 ? 1 : m_capacity * 2);
		}
	}
}
