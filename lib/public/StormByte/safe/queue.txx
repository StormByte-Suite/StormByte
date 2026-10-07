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

#include <algorithm>
#include <compare>
#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeValue T>
		Queue<T>::Queue() noexcept: m_values() {}

		template<Type::SafeValue T>
		Queue<T>::Queue(const container_type& values): m_values(values) {}

		template<Type::SafeValue T>
		Queue<T>::Queue(container_type&& values) noexcept: m_values(std::move(values)) {}

		template<Type::SafeValue T>
		Queue<T>::Queue(const Queue& other): m_values(other.m_values) {}

		template<Type::SafeValue T>
		Queue<T>::Queue(Queue&& other) noexcept: m_values(std::move(other.m_values)) {}

		template<Type::SafeValue T>
		Queue<T>::~Queue() noexcept = default;

		template<Type::SafeValue T>
		Queue<T>& Queue<T>::operator=(const Queue& other) {
			if (this == &other)
				return *this;
			m_values = other.m_values;
			return *this;
		}

		template<Type::SafeValue T>
		Queue<T>& Queue<T>::operator=(Queue&& other) noexcept {
			if (this == &other)
				return *this;
			m_values = std::move(other.m_values);
			return *this;
		}

		template<Type::SafeValue T>
		Queue<T>::iterator Queue<T>::begin() noexcept {
			return m_values.begin();
		}

		template<Type::SafeValue T>
		Queue<T>::iterator Queue<T>::end() noexcept {
			return m_values.end();
		}

		template<Type::SafeValue T>
		Queue<T>::const_iterator Queue<T>::begin() const noexcept {
			return m_values.begin();
		}

		template<Type::SafeValue T>
		Queue<T>::const_iterator Queue<T>::end() const noexcept {
			return m_values.end();
		}

		template<Type::SafeValue T>
		Queue<T>::const_iterator Queue<T>::cbegin() const noexcept {
			return m_values.cbegin();
		}

		template<Type::SafeValue T>
		Queue<T>::const_iterator Queue<T>::cend() const noexcept {
			return m_values.cend();
		}

		template<Type::SafeValue T>
		bool Queue<T>::empty() const noexcept {
			return m_values.empty();
		}

		template<Type::SafeValue T>
		Queue<T>::size_type Queue<T>::size() const noexcept {
			return static_cast<size_type>(m_values.size());
		}

		template<Type::SafeValue T>
		Queue<T>::reference Queue<T>::front() {
			if (m_values.empty())
				ThrowQueueOutOfBounds();
			return m_values.front();
		}

		template<Type::SafeValue T>
		Queue<T>::const_reference Queue<T>::front() const {
			if (m_values.empty())
				ThrowQueueOutOfBounds();
			return m_values.front();
		}

		template<Type::SafeValue T>
		Queue<T>::reference Queue<T>::back() {
			if (m_values.empty())
				ThrowQueueOutOfBounds();
			return m_values.back();
		}

		template<Type::SafeValue T>
		Queue<T>::const_reference Queue<T>::back() const {
			if (m_values.empty())
				ThrowQueueOutOfBounds();
			return m_values.back();
		}

		template<Type::SafeValue T>
		void Queue<T>::push(const T& value) {
			m_values.push_back(value);
		}

		template<Type::SafeValue T>
		void Queue<T>::push(T&& value) {
			m_values.push_back(std::move(value));
		}

		template<Type::SafeValue T>
		template<class... Args>
		Queue<T>::reference Queue<T>::emplace(Args&&... args) {
			return m_values.emplace_back(std::forward<Args>(args)...);
		}

		template<Type::SafeValue T>
		void Queue<T>::pop() {
			if (m_values.empty())
				ThrowQueueOutOfBounds();
			m_values.erase(m_values.begin());
		}

		template<Type::SafeValue T>
		Queue<T>::iterator Queue<T>::erase(const_iterator position) {
			return m_values.erase(position);
		}

		template<Type::SafeValue T>
		Queue<T>::iterator Queue<T>::erase(const_iterator first, const_iterator last) {
			return m_values.erase(first, last);
		}

		template<Type::SafeValue T>
		void Queue<T>::swap(Queue& other) noexcept {
			m_values.swap(other.m_values);
		}

		template<Type::SafeValue T>
		bool Queue<T>::operator==(const Queue& other) const requires Type::EqualityComparable<T> {
			return m_values == other.m_values;
		}

		template<Type::SafeValue T>
		std::strong_ordering Queue<T>::operator<=>(const Queue& other) const requires std::three_way_comparable<T> {
			return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
		}

		template<Type::SafeValue T>
		bool Queue<T>::operator<(const Queue& other) const requires requires(const T& left, const T& right) { left < right; } {
			return m_values < other.m_values;
		}

		template<Type::SafeValue T>
		bool Queue<T>::operator<=(const Queue& other) const requires requires(const T& left, const T& right) { left < right; } {
			return m_values <= other.m_values;
		}

		template<Type::SafeValue T>
		bool Queue<T>::operator>(const Queue& other) const requires requires(const T& left, const T& right) { left < right; } {
			return m_values > other.m_values;
		}

		template<Type::SafeValue T>
		bool Queue<T>::operator>=(const Queue& other) const requires requires(const T& left, const T& right) { left < right; } {
			return m_values >= other.m_values;
		}
	}
}
