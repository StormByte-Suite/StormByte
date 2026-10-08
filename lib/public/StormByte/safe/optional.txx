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

#include <new>
#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeComponent T>
		Optional<T>::Optional() noexcept: m_value(nullptr) {}

		template<Type::SafeComponent T>
		Optional<T>::Optional(std::nullopt_t) noexcept: Optional() {}

		template<Type::SafeComponent T>
		Optional<T>::Optional(const T& value): m_value(nullptr) {
			m_value = Make(value);
		}

		template<Type::SafeComponent T>
		Optional<T>::Optional(T&& value): m_value(nullptr) {
			m_value = Make(std::move(value));
		}

		template<Type::SafeComponent T>
		Optional<T>::Optional(const std::optional<T>& value): m_value(nullptr) {
			if (value)
				m_value = Make(*value);
		}

		template<Type::SafeComponent T>
		Optional<T>::Optional(std::optional<T>&& value): m_value(nullptr) {
			if (value)
				m_value = Make(std::move(*value));
		}

		template<Type::SafeComponent T>
		Optional<T>::Optional(const Optional& other): m_value(nullptr) {
			if (other.m_value != nullptr)
				m_value = Make(*other.m_value);
		}

		template<Type::SafeComponent T>
		Optional<T>::Optional(Optional&& other) noexcept: m_value(other.m_value) {
			other.m_value = nullptr;
		}

		template<Type::SafeComponent T>
		Optional<T>::~Optional() noexcept {
			Release();
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(const Optional& other) {
			if (this == &other)
				return *this;
			T* created = other.m_value == nullptr ? nullptr : Make(*other.m_value);
			Release();
			m_value = created;
			return *this;
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(Optional&& other) noexcept {
			if (this == &other)
				return *this;
			Release();
			m_value = other.m_value;
			other.m_value = nullptr;
			return *this;
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(const T& value) {
			T* created = Make(value);
			Release();
			m_value = created;
			return *this;
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(T&& value) {
			T* created = Make(std::move(value));
			Release();
			m_value = created;
			return *this;
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(std::nullopt_t) noexcept {
			reset();
			return *this;
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(const std::optional<T>& value) {
			if (!value) {
				reset();
				return *this;
			}
			return *this = *value;
		}

		template<Type::SafeComponent T>
		Optional<T>& Optional<T>::operator=(std::optional<T>&& value) {
			if (!value) {
				reset();
				return *this;
			}
			return *this = std::move(*value);
		}

		template<Type::SafeComponent T>
		void Optional<T>::swap(Optional& other) noexcept {
			T* temporary = m_value;
			m_value = other.m_value;
			other.m_value = temporary;
		}

		template<Type::SafeComponent T>
		T& Optional<T>::value() & {
			if (m_value == nullptr)
				throw BadOptionalAccess();
			return *m_value;
		}

		template<Type::SafeComponent T>
		const T& Optional<T>::value() const & {
			if (m_value == nullptr)
				throw BadOptionalAccess();
			return *m_value;
		}

		template<Type::SafeComponent T>
		T&& Optional<T>::value() && {
			if (m_value == nullptr)
				throw BadOptionalAccess();
			return std::move(*m_value);
		}

		template<Type::SafeComponent T>
		const T&& Optional<T>::value() const && {
			if (m_value == nullptr)
				throw BadOptionalAccess();
			return std::move(*m_value);
		}

		template<Type::SafeComponent T>
		T& Optional<T>::operator*() & {
			return *m_value;
		}

		template<Type::SafeComponent T>
		const T& Optional<T>::operator*() const & {
			return *m_value;
		}

		template<Type::SafeComponent T>
		T&& Optional<T>::operator*() && {
			return std::move(*m_value);
		}

		template<Type::SafeComponent T>
		const T&& Optional<T>::operator*() const && {
			return std::move(*m_value);
		}

		template<Type::SafeComponent T>
		T* Optional<T>::operator->() {
			return m_value;
		}

		template<Type::SafeComponent T>
		const T* Optional<T>::operator->() const {
			return m_value;
		}

		template<Type::SafeComponent T>
		void Optional<T>::reset() noexcept {
			Release();
		}

		template<Type::SafeComponent T>
		void Optional<T>::Release() noexcept {
			if (m_value == nullptr)
				return;
			m_value->~T();
			Heap::Free(m_value);
			m_value = nullptr;
		}
	}
}
