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

#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace StormByte {
	namespace Safe {
		namespace Detail {
			/**
			 * @brief Allocate a control block and a @p T in one Base block.
			 * @tparam T Object type.
			 * @tparam Args Constructor argument types.
			 * @param args Arguments forwarded to @p T.
			 * @return Control block. The object address is @ref Control::Object.
			 * @throws AllocationError The block could not be allocated.
			 */
			template<class T, class... Args>
			Control* Create(Args&&... args) {
				constexpr std::size_t align = alignof(T) > alignof(Control) ? alignof(T) : alignof(Control);
				const std::size_t controlSize = (sizeof(Control) + align - 1) & ~(align - 1);
				void* block = Heap::Allocate(controlSize + sizeof(T));
				Control* control = static_cast<Control*>(block);
				T* object = reinterpret_cast<T*>(static_cast<unsigned char*>(block) + controlSize);
				control->Strong.store(1, std::memory_order_relaxed);
				control->Weak.store(1, std::memory_order_relaxed);
				control->Object = object;
				control->Destroy = [](Control* dying) noexcept {
					static_cast<T*>(dying->Object)->~T();
				};
				try {
					new (object) T(std::forward<Args>(args)...);
				} catch (...) {
					Heap::Free(block);
					throw;
				}
				return control;
			}

			/**
			 * @brief Drop one strong reference and destroy the object when it was the last.
			 * @param control Control block.
			 */
			inline void ReleaseStrong(Control* control) noexcept {
				if (control == nullptr)
					return;
				if (control->Strong.fetch_sub(1, std::memory_order_acq_rel) == 1) {
					control->Destroy(control);
					if (control->Weak.fetch_sub(1, std::memory_order_acq_rel) == 1)
						Heap::Free(control);
				}
			}

			/**
			 * @brief Drop one weak reference and free the block when it was the last.
			 * @param control Control block.
			 */
			inline void ReleaseWeak(Control* control) noexcept {
				if (control == nullptr)
					return;
				if (control->Weak.fetch_sub(1, std::memory_order_acq_rel) == 1)
					Heap::Free(control);
			}
		}

		template<class T>
		Shared<T>::Shared() noexcept: m_control(nullptr), m_object(nullptr) {}

		template<class T>
		Shared<T>::Shared(std::nullptr_t) noexcept: m_control(nullptr), m_object(nullptr) {}

		template<class T>
		template<class U>
		requires Type::ConvertibleTo<U*, T*>
		Shared<T>::Shared(const Shared<U>& other) noexcept: m_control(other.m_control), m_object(other.m_object) {
			if (m_control != nullptr)
				m_control->Strong.fetch_add(1, std::memory_order_relaxed);
		}

		template<class T>
		template<class U>
		requires Type::ConvertibleTo<U*, T*>
		Shared<T>::Shared(Shared<U>&& other) noexcept: m_control(other.m_control), m_object(other.m_object) {
			other.m_control = nullptr;
			other.m_object = nullptr;
		}

		template<class T>
		Shared<T>::Shared(const Shared& other) noexcept: m_control(other.m_control), m_object(other.m_object) {
			if (m_control != nullptr)
				m_control->Strong.fetch_add(1, std::memory_order_relaxed);
		}

		template<class T>
		Shared<T>::Shared(Shared&& other) noexcept: m_control(other.m_control), m_object(other.m_object) {
			other.m_control = nullptr;
			other.m_object = nullptr;
		}

		template<class T>
		Shared<T>::Shared(const Weak<T>& weak): m_control(nullptr), m_object(nullptr) {
			Shared locked = weak.lock();
			if (!locked)
				Heap::ThrowExpiredWeakPointer();
			m_control = locked.m_control;
			m_object = locked.m_object;
			locked.m_control = nullptr;
			locked.m_object = nullptr;
		}

		template<class T>
		Shared<T>& Shared<T>::operator=(const Shared& other) noexcept {
			if (this == &other)
				return *this;
			Shared copy(other);
			swap(copy);
			return *this;
		}

		template<class T>
		Shared<T>& Shared<T>::operator=(Shared&& other) noexcept {
			if (this == &other)
				return *this;
			Shared taken(std::move(other));
			swap(taken);
			return *this;
		}

		template<class T>
		Shared<T>::~Shared() {
			Release();
		}

		template<class T>
		T* Shared<T>::get() const noexcept {
			return m_object;
		}

		template<class T>
		std::add_lvalue_reference_t<T> Shared<T>::operator*() const noexcept {
			if constexpr (!std::is_void_v<T>)
				return *m_object;
		}

		template<class T>
		T* Shared<T>::operator->() const noexcept {
			return m_object;
		}

		template<class T>
		Shared<T>::operator bool() const noexcept {
			return m_object != nullptr;
		}

		template<class T>
		long Shared<T>::use_count() const noexcept {
			return m_control == nullptr ? 0 : m_control->Strong.load(std::memory_order_relaxed);
		}

		template<class T>
		void Shared<T>::reset() noexcept {
			Release();
			m_control = nullptr;
			m_object = nullptr;
		}

		template<class T>
		void Shared<T>::swap(Shared& other) noexcept {
			using std::swap;
			swap(m_control, other.m_control);
			swap(m_object, other.m_object);
		}

		template<class T>
		template<class U>
		bool Shared<T>::owner_before(const Shared<U>& other) const noexcept {
			return m_control < other.m_control;
		}

		template<class T>
		template<class U>
		bool Shared<T>::owner_before(const Weak<U>& other) const noexcept {
			return m_control < other.m_control;
		}

		template<class T>
		template<class Target, class... Args>
		requires Type::SameAs<Target, T> || Type::DerivedFrom<Target, T>
		Shared<T> Shared<T>::MakePointer(Args&&... args) {
			return MakeShared<Target>(std::forward<Args>(args)...);
		}

		template<class T>
		Shared<T>::Shared(Detail::Control* control, T* object) noexcept: m_control(control), m_object(object) {}

		template<class T>
		void Shared<T>::Release() noexcept {
			Detail::ReleaseStrong(m_control);
		}

		template<class T>
		Unique<T>::Unique() noexcept: m_object(nullptr), m_destroy(nullptr) {}

		template<class T>
		Unique<T>::Unique(std::nullptr_t) noexcept: m_object(nullptr), m_destroy(nullptr) {}

		template<class T>
		template<class U>
		requires Type::ConvertibleTo<U*, T*>
		Unique<T>::Unique(Unique<U>&& other) noexcept: m_object(other.m_object), m_destroy(other.m_destroy) {
			other.m_object = nullptr;
			other.m_destroy = nullptr;
		}

		template<class T>
		Unique<T>::Unique(Unique&& other) noexcept: m_object(other.m_object), m_destroy(other.m_destroy) {
			other.m_object = nullptr;
			other.m_destroy = nullptr;
		}

		template<class T>
		Unique<T>& Unique<T>::operator=(Unique&& other) noexcept {
			if (this == &other)
				return *this;
			reset();
			m_object = other.m_object;
			m_destroy = other.m_destroy;
			other.m_object = nullptr;
			other.m_destroy = nullptr;
			return *this;
		}

		template<class T>
		Unique<T>::~Unique() {
			reset();
		}

		template<class T>
		T* Unique<T>::get() const noexcept {
			return m_object;
		}

		template<class T>
		std::add_lvalue_reference_t<T> Unique<T>::operator*() const noexcept {
			if constexpr (!std::is_void_v<T>)
				return *m_object;
		}

		template<class T>
		T* Unique<T>::operator->() const noexcept {
			return m_object;
		}

		template<class T>
		Unique<T>::operator bool() const noexcept {
			return m_object != nullptr;
		}

		template<class T>
		void Unique<T>::reset() noexcept {
			if (m_destroy != nullptr)
				m_destroy(m_object);
			m_object = nullptr;
			m_destroy = nullptr;
		}

		template<class T>
		void Unique<T>::swap(Unique& other) noexcept {
			using std::swap;
			swap(m_object, other.m_object);
			swap(m_destroy, other.m_destroy);
		}

		template<class T>
		template<class Target, class... Args>
		requires Type::SameAs<Target, T> || Type::DerivedFrom<Target, T>
		Unique<T> Unique<T>::MakePointer(Args&&... args) {
			return MakeUnique<Target>(std::forward<Args>(args)...);
		}

		template<class T>
		Unique<T>::Unique(T* object, void (*destroy)(void*) noexcept) noexcept: m_object(object), m_destroy(destroy) {}

		template<class T>
		Weak<T>::Weak() noexcept: m_control(nullptr), m_object(nullptr) {}

		template<class T>
		Weak<T>::Weak(std::nullptr_t) noexcept: m_control(nullptr), m_object(nullptr) {}

		template<class T>
		template<class U>
		requires Type::ConvertibleTo<U*, T*>
		Weak<T>::Weak(const Shared<U>& owner) noexcept: m_control(owner.m_control), m_object(owner.m_object) {
			if (m_control != nullptr)
				m_control->Weak.fetch_add(1, std::memory_order_relaxed);
		}

		template<class T>
		Weak<T>::Weak(const Weak& other) noexcept: m_control(other.m_control), m_object(other.m_object) {
			if (m_control != nullptr)
				m_control->Weak.fetch_add(1, std::memory_order_relaxed);
		}

		template<class T>
		Weak<T>::Weak(Weak&& other) noexcept: m_control(other.m_control), m_object(other.m_object) {
			other.m_control = nullptr;
			other.m_object = nullptr;
		}

		template<class T>
		Weak<T>& Weak<T>::operator=(const Weak& other) noexcept {
			if (this == &other)
				return *this;
			Weak copy(other);
			swap(copy);
			return *this;
		}

		template<class T>
		Weak<T>& Weak<T>::operator=(Weak&& other) noexcept {
			if (this == &other)
				return *this;
			Weak taken(std::move(other));
			swap(taken);
			return *this;
		}

		template<class T>
		Weak<T>::~Weak() {
			Release();
		}

		template<class T>
		Shared<T> Weak<T>::lock() const noexcept {
			if (m_control == nullptr)
				return {};
			long count = m_control->Strong.load(std::memory_order_relaxed);
			while (count != 0) {
				if (m_control->Strong.compare_exchange_weak(count, count + 1, std::memory_order_acquire, std::memory_order_relaxed))
					return Shared<T>(m_control, m_object);
			}
			return {};
		}

		template<class T>
		bool Weak<T>::expired() const noexcept {
			return m_control == nullptr || m_control->Strong.load(std::memory_order_relaxed) == 0;
		}

		template<class T>
		long Weak<T>::use_count() const noexcept {
			return m_control == nullptr ? 0 : m_control->Strong.load(std::memory_order_relaxed);
		}

		template<class T>
		void Weak<T>::reset() noexcept {
			Release();
			m_control = nullptr;
			m_object = nullptr;
		}

		template<class T>
		void Weak<T>::swap(Weak& other) noexcept {
			using std::swap;
			swap(m_control, other.m_control);
			swap(m_object, other.m_object);
		}

		template<class T>
		template<class U>
		bool Weak<T>::owner_before(const Weak<U>& other) const noexcept {
			return m_control < other.m_control;
		}

		template<class T>
		template<class U>
		bool Weak<T>::owner_before(const Shared<U>& other) const noexcept {
			return m_control < other.m_control;
		}

		template<class T>
		void Weak<T>::Release() noexcept {
			Detail::ReleaseWeak(m_control);
		}

		template<class T>
		EnableSharedFromThis<T>::EnableSharedFromThis(const EnableSharedFromThis&) noexcept {}

		template<class T>
		EnableSharedFromThis<T>::EnableSharedFromThis(EnableSharedFromThis&&) noexcept {}

		template<class T>
		EnableSharedFromThis<T>& EnableSharedFromThis<T>::operator=(const EnableSharedFromThis&) noexcept {
			return *this;
		}

		template<class T>
		EnableSharedFromThis<T>& EnableSharedFromThis<T>::operator=(EnableSharedFromThis&&) noexcept {
			return *this;
		}

		template<class T>
		Shared<T> EnableSharedFromThis<T>::SharedFromThis() const {
			return Shared<T>(m_weak);
		}

		template<class T>
		AtomicShared<T>::AtomicShared() noexcept: m_lock(), m_control(nullptr), m_object(nullptr) {}

		template<class T>
		AtomicShared<T>::AtomicShared(Shared<T> owner) noexcept: m_lock(), m_control(owner.m_control), m_object(owner.m_object) {
			owner.m_control = nullptr;
			owner.m_object = nullptr;
		}

		template<class T>
		AtomicShared<T>::~AtomicShared() {
			Detail::ReleaseStrong(m_control);
		}

		template<class T>
		AtomicShared<T>& AtomicShared<T>::operator=(Shared<T> owner) noexcept {
			store(std::move(owner));
			return *this;
		}

		template<class T>
		AtomicShared<T>::operator bool() const noexcept {
			return static_cast<bool>(load());
		}

		template<class T>
		Shared<T> AtomicShared<T>::load() const noexcept {
			Lock();
			Shared<T> loaded(m_control, m_object);
			if (m_control != nullptr)
				m_control->Strong.fetch_add(1, std::memory_order_relaxed);
			Unlock();
			return loaded;
		}

		template<class T>
		void AtomicShared<T>::store(Shared<T> owner) noexcept {
			exchange(std::move(owner));
		}

		template<class T>
		Shared<T> AtomicShared<T>::exchange(Shared<T> owner) noexcept {
			Lock();
			Shared<T> previous(m_control, m_object);
			m_control = owner.m_control;
			m_object = owner.m_object;
			owner.m_control = nullptr;
			owner.m_object = nullptr;
			Unlock();
			return previous;
		}

		template<class T>
		bool AtomicShared<T>::compare_exchange_strong(Shared<T>& expected, Shared<T> desired) noexcept {
			Lock();
			if (m_object == expected.get()) {
				Detail::ReleaseStrong(expected.m_control);
				expected.m_control = nullptr;
				expected.m_object = nullptr;
				expected = Shared<T>(m_control, m_object);
				if (m_control != nullptr)
					m_control->Strong.fetch_add(1, std::memory_order_relaxed);
				Detail::ReleaseStrong(m_control);
				m_control = desired.m_control;
				m_object = desired.m_object;
				desired.m_control = nullptr;
				desired.m_object = nullptr;
				Unlock();
				return true;
			}
			Detail::ReleaseStrong(expected.m_control);
			expected.m_control = m_control;
			expected.m_object = m_object;
			if (m_control != nullptr)
				m_control->Strong.fetch_add(1, std::memory_order_relaxed);
			Unlock();
			return false;
		}

		template<class T>
		bool AtomicShared<T>::compare_exchange_weak(Shared<T>& expected, Shared<T> desired) noexcept {
			return compare_exchange_strong(expected, std::move(desired));
		}

		template<class T>
		void AtomicShared<T>::Lock() const noexcept {
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
		}

		template<class T>
		void AtomicShared<T>::Unlock() const noexcept {
			m_lock.clear(std::memory_order_release);
		}

		template<class T, class... Args>
		Shared<T> MakeShared(Args&&... args) {
			try {
				Detail::Control* control = Detail::Create<T>(std::forward<Args>(args)...);
				Shared<T> owner(control, static_cast<T*>(control->Object));
				if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>)
					static_cast<EnableSharedFromThis<T>*>(owner.get())->m_weak = owner;
				return owner;
			} catch (...) {
				Heap::RethrowException();
			}
		}

		template<class T, class... Args>
		Unique<T> MakeUnique(Args&&... args) {
			void* block = Heap::Allocate(sizeof(T));
			try {
				T* object = new (block) T(std::forward<Args>(args)...);
				return Unique<T>(object, [](void* pointer) noexcept {
					static_cast<T*>(pointer)->~T();
					Heap::Free(pointer);
				});
			} catch (...) {
				Heap::Free(block);
				Heap::RethrowException();
			}
		}

		template<class T, class U>
		Shared<T> StaticPointerCast(const Shared<U>& from) noexcept {
			if (!from)
				return {};
			Shared<T> casted(from.m_control, static_cast<T*>(from.get()));
			casted.m_control->Strong.fetch_add(1, std::memory_order_relaxed);
			return casted;
		}

		template<class T, class U>
		Shared<T> DynamicPointerCast(const Shared<U>& from) noexcept {
			if (!from)
				return {};
			T* object = dynamic_cast<T*>(from.get());
			if (object == nullptr)
				return {};
			Shared<T> casted(from.m_control, object);
			casted.m_control->Strong.fetch_add(1, std::memory_order_relaxed);
			return casted;
		}

		template<class T, class U>
		Shared<T> ConstPointerCast(const Shared<U>& from) noexcept {
			if (!from)
				return {};
			Shared<T> casted(from.m_control, const_cast<T*>(from.get()));
			casted.m_control->Strong.fetch_add(1, std::memory_order_relaxed);
			return casted;
		}

		template<class T, class U>
		Shared<T> ReinterpretPointerCast(const Shared<U>& from) noexcept {
			if (!from)
				return {};
			Shared<T> casted(from.m_control, reinterpret_cast<T*>(from.get()));
			casted.m_control->Strong.fetch_add(1, std::memory_order_relaxed);
			return casted;
		}

		template<class T>
		void swap(Shared<T>& left, Shared<T>& right) noexcept {
			left.swap(right);
		}

		template<class T>
		void swap(Unique<T>& left, Unique<T>& right) noexcept {
			left.swap(right);
		}

		template<class T>
		void swap(Weak<T>& left, Weak<T>& right) noexcept {
			left.swap(right);
		}
	}
}
