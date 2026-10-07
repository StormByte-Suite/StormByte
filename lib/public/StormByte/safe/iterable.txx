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

#include <iterator>
#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		template<bool OtherConst>
		requires IsConst && (!OtherConst)
		Iterable<Component>::BasicIterator<IsConst>::BasicIterator(const BasicIterator<OtherConst>& other) noexcept: m_current(other.m_current) {}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>::BasicIterator(Inner current) noexcept: m_current(current) {}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>::reference Iterable<Component>::BasicIterator<IsConst>::operator*() const {
			return *m_current;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>& Iterable<Component>::BasicIterator<IsConst>::operator++() {
			++m_current;
			return *this;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst> Iterable<Component>::BasicIterator<IsConst>::operator++(int) {
			BasicIterator previous = *this;
			++*this;
			return previous;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>& Iterable<Component>::BasicIterator<IsConst>::operator--() {
			--m_current;
			return *this;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst> Iterable<Component>::BasicIterator<IsConst>::operator--(int) {
			BasicIterator previous = *this;
			--*this;
			return previous;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>& Iterable<Component>::BasicIterator<IsConst>::operator+=(difference_type offset) requires random_access {
			m_current += offset;
			return *this;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>& Iterable<Component>::BasicIterator<IsConst>::operator-=(difference_type offset) requires random_access {
			m_current -= offset;
			return *this;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		Iterable<Component>::BasicIterator<IsConst>::reference Iterable<Component>::BasicIterator<IsConst>::operator[](difference_type offset) const requires random_access {
			return m_current[offset];
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		template<bool OtherConst>
		bool Iterable<Component>::BasicIterator<IsConst>::operator==(const BasicIterator<OtherConst>& other) const {
			return m_current == other.m_current;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		template<bool IsConst>
		template<bool OtherConst>
		bool Iterable<Component>::BasicIterator<IsConst>::operator<(const BasicIterator<OtherConst>& other) const requires random_access {
			return m_current < other.m_current;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::Iterable(container_type container) noexcept: m_container(std::move(container)) {}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::iterator Iterable<Component>::begin() noexcept {
			return iterator(m_container.begin());
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::iterator Iterable<Component>::end() noexcept {
			return iterator(m_container.end());
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::const_iterator Iterable<Component>::begin() const noexcept {
			return const_iterator(m_container.begin());
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::const_iterator Iterable<Component>::end() const noexcept {
			return const_iterator(m_container.end());
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::const_iterator Iterable<Component>::cbegin() const noexcept {
			return begin();
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::const_iterator Iterable<Component>::cend() const noexcept {
			return end();
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		bool Iterable<Component>::empty() const noexcept {
			return m_container.empty();
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::size_type Iterable<Component>::size() const noexcept {
			return static_cast<size_type>(m_container.size());
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		Iterable<Component>::container_type& Iterable<Component>::Container() noexcept {
			return m_container;
		}

		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		const Iterable<Component>::container_type& Iterable<Component>::Container() const noexcept {
			return m_container;
		}
	}
}
