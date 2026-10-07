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

#include <StormByte/safe/heap.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <atomic>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Owned values that cross a DLL without the caller's CRT.
	 */
	namespace Safe {
		template<class T>
		class Shared;

		template<class T>
		class Unique;

		template<class T>
		class Weak;

		template<class T>
		class AtomicShared;

		template<class T>
		class EnableSharedFromThis;

		/**
		 * @namespace StormByte::Safe::Heap
		 * @brief Deleter for a caller-owned `std::unique_ptr` that still frees Base's block.
		 *
		 * Allocation stays in the installed heap header. This name exists so a moved @ref Unique keeps its published deleter type.
		 */
		namespace Heap {
			/**
			 * @brief Destroy the concrete object and return its block to @ref Free.
			 */
			struct STORMBYTE_PUBLIC_TYPE ObjectDeleter {
				void (*destroy)(void*) noexcept = nullptr; ///< Concrete destructor stored by @ref Unique.

				/**
				 * @brief Destroy @p pointer with the concrete destructor.
				 * @tparam U Static pointee. The stored destructor is the real one.
				 * @param pointer Object to destroy, or null.
				 */
				template<class U>
				void operator()(U* pointer) const noexcept {
					if (destroy != nullptr)
						destroy(pointer);
				}
			};
		}

		/**
		 * @namespace StormByte::Safe::Detail
		 * @brief Control block shared by @ref Shared and @ref Weak.
		 */
		namespace Detail {
			/**
			 * @brief Base-owned reference counts and the real destructor.
			 *
			 * The object lives in the same @ref Heap::Allocate block, after this header. @ref Destroy is the destructor of the concrete type, stored when the object is created.
			 */
			struct Control {
				std::atomic<long> Strong; ///< Owners. Zero destroys the object.
				std::atomic<long> Weak; ///< Owners plus observers. Zero frees the block.
				void (*Destroy)(Control*) noexcept; ///< Destructor of the concrete object.
				void* Object; ///< Address of the concrete object inside this block.
			};
		}

		/**
		 * @class Shared
		 * @brief Shared owner of an object allocated on Base's heap.
		 * @tparam T Pointee type.
		 *
		 * The control block is a @ref Detail::Control allocated with @ref Heap::Allocate. Copying a @ref Shared copies that pointer. There is no `std::shared_ptr` inside. There is no constructor from a raw pointer, from `std::shared_ptr` or from `std::unique_ptr`.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Shared {
			public:
				using element_type = T; ///< Pointee type.
				using weak_type = Weak<T>; ///< Matching observer.

				/**
				 * @brief Construct an empty owner.
				 */
				Shared() noexcept;

				/**
				 * @brief Construct an empty owner from null.
				 * @param null Null pointer constant.
				 */
				Shared(std::nullptr_t null) noexcept;

				/**
				 * @brief Share ownership with another owner.
				 * @tparam U Pointee convertible to @p T.
				 * @param other Owner to share.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Shared(const Shared<U>& other) noexcept;

				/**
				 * @brief Take ownership from another owner.
				 * @tparam U Pointee convertible to @p T.
				 * @param other Owner to take.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Shared(Shared<U>&& other) noexcept;

				/**
				 * @brief Copy an owner. The control block stays.
				 * @param other Owner to copy.
				 */
				Shared(const Shared& other) noexcept;

				/**
				 * @brief Take an owner. @p other is left empty.
				 * @param other Owner to take.
				 */
				Shared(Shared&& other) noexcept;

				/**
				 * @brief Lock an observer.
				 * @param weak Observer of a live control block.
				 * @throws ExpiredWeakPointerError @p weak is empty or expired.
				 */
				explicit Shared(const Weak<T>& weak);

				/**
				 * @brief Share ownership.
				 * @param other Owner to copy.
				 * @return This owner.
				 */
				Shared& operator=(const Shared& other) noexcept;

				/**
				 * @brief Take ownership. @p other is left empty.
				 * @param other Owner to take.
				 * @return This owner.
				 */
				Shared& operator=(Shared&& other) noexcept;

				/**
				 * @brief Drop this owner. The last owner destroys the object.
				 */
				~Shared();

				/**
				 * @brief Return the stored pointer.
				 * @return Pointee, or null.
				 */
				T* get() const noexcept;

				/**
				 * @brief Dereference the stored pointer.
				 * @return Pointee.
				 */
				T& operator*() const noexcept;

				/**
				 * @brief Access a member of the pointee.
				 * @return Stored pointer.
				 */
				T* operator->() const noexcept;

				/**
				 * @brief Test whether this owner holds an object.
				 * @return Whether the stored pointer is non-null.
				 */
				explicit operator bool() const noexcept;

				/**
				 * @brief Return the number of owners.
				 * @return Strong count, or zero when empty.
				 */
				long use_count() const noexcept;

				/**
				 * @brief Drop this owner.
				 */
				void reset() noexcept;

				/**
				 * @brief Exchange owners.
				 * @param other Owner to exchange with.
				 */
				void swap(Shared& other) noexcept;

				/**
				 * @brief Order control blocks by identity.
				 * @tparam U Other pointee.
				 * @param other Other owner.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Shared<U>& other) const noexcept;

				/**
				 * @brief Order this control block against an observer.
				 * @tparam U Other pointee.
				 * @param other Observer.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Weak<U>& other) const noexcept;

				/**
				 * @brief Allocate @p Target on Base's heap and own it as @p T.
				 * @tparam Target Concrete type. It is @p T or derived from @p T.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p Target.
				 * @return Owner of the concrete object. The stored destructor is @p Target's.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class Target, class... Args>
				requires Type::SameAs<Target, T> || Type::DerivedFrom<Target, T>
				static Shared<T> MakePointer(Args&&... args);

				/**
				 * @brief Copy the owner into caller-owned STL storage.
				 * @return A `std::shared_ptr` whose deleter still releases Base's block. There is no conversion back.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::shared_ptr<T>() const {
					if (m_control == nullptr)
						return {};
					m_control->Strong.fetch_add(1, std::memory_order_relaxed);
					Detail::Control* control = m_control;
					return std::shared_ptr<T>(m_object, [control](T*) noexcept {
						if (control->Strong.fetch_sub(1, std::memory_order_acq_rel) == 1) {
							control->Destroy(control);
							if (control->Weak.fetch_sub(1, std::memory_order_acq_rel) == 1)
								Heap::Free(control);
						}
					});
				}

			private:
				/**
				 * @brief Adopt a control block already created on Base's heap.
				 * @param control Control block.
				 * @param object Typed address inside that block.
				 */
				Shared(Detail::Control* control, T* object) noexcept;

				/**
				 * @brief Drop one strong reference.
				 */
				void Release() noexcept;

				template<class U>
				friend class Shared;

				template<class U>
				friend class Weak;

				template<class U>
				friend class AtomicShared;

				template<class U, class... Args>
				friend Shared<U> MakeShared(Args&&...);

				template<class X, class Y>
				friend Shared<X> StaticPointerCast(const Shared<Y>&) noexcept;

				template<class X, class Y>
				friend Shared<X> DynamicPointerCast(const Shared<Y>&) noexcept;

				template<class X, class Y>
				friend Shared<X> ConstPointerCast(const Shared<Y>&) noexcept;

				template<class X, class Y>
				friend Shared<X> ReinterpretPointerCast(const Shared<Y>&) noexcept;

				Detail::Control* m_control; ///< Base control block, or null.
				T* m_object; ///< Typed address, or null. A cast may differ from the concrete object.
		};

		/**
		 * @class Unique
		 * @brief Unique owner of an object allocated on Base's heap.
		 * @tparam T Pointee type.
		 *
		 * The object and its concrete destructor live on Base's heap. There is no `release` and no constructor from a raw pointer.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Unique {
			public:
				using element_type = T; ///< Pointee type.
				using pointer = T*; ///< Stored pointer.
				using deleter_type = Heap::ObjectDeleter; ///< Deleter used by the STL conversion.

				/**
				 * @brief Construct an empty owner.
				 */
				Unique() noexcept;

				/**
				 * @brief Construct an empty owner from null.
				 * @param null Null pointer constant.
				 */
				Unique(std::nullptr_t null) noexcept;

				/**
				 * @brief Take ownership from another owner.
				 * @tparam U Pointee convertible to @p T.
				 * @param other Owner to take. Left empty.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Unique(Unique<U>&& other) noexcept;

				/**
				 * @brief Copy constructor. Deleted.
				 * @param other Ignored.
				 */
				Unique(const Unique& other) = delete;

				/**
				 * @brief Take ownership. @p other is left empty.
				 * @param other Owner to take.
				 */
				Unique(Unique&& other) noexcept;

				/**
				 * @brief Copy assignment. Deleted.
				 * @param other Ignored.
				 * @return This owner.
				 */
				Unique& operator=(const Unique& other) = delete;

				/**
				 * @brief Take ownership. @p other is left empty.
				 * @param other Owner to take.
				 * @return This owner.
				 */
				Unique& operator=(Unique&& other) noexcept;

				/**
				 * @brief Destroy the object through its concrete destructor and free the block.
				 */
				~Unique();

				/**
				 * @brief Return the stored pointer.
				 * @return Pointee, or null.
				 */
				T* get() const noexcept;

				/**
				 * @brief Dereference the stored pointer.
				 * @return Pointee.
				 */
				T& operator*() const noexcept;

				/**
				 * @brief Access a member of the pointee.
				 * @return Stored pointer.
				 */
				T* operator->() const noexcept;

				/**
				 * @brief Test whether this owner holds an object.
				 * @return Whether the stored pointer is non-null.
				 */
				explicit operator bool() const noexcept;

				/**
				 * @brief Destroy the object and leave this owner empty.
				 */
				void reset() noexcept;

				/**
				 * @brief Exchange owners.
				 * @param other Owner to exchange with.
				 */
				void swap(Unique& other) noexcept;

				/**
				 * @brief Allocate @p Target on Base's heap and own it as @p T.
				 * @tparam Target Concrete type. It is @p T or derived from @p T.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p Target.
				 * @return Owner. The stored destructor is @p Target's, so @p T does not need a virtual destructor.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class Target, class... Args>
				requires Type::SameAs<Target, T> || Type::DerivedFrom<Target, T>
				static Unique<T> MakePointer(Args&&... args);

				/**
				 * @brief Move the owner into caller-owned STL storage.
				 * @return A `std::unique_ptr` whose deleter still frees Base's block. There is no conversion back.
				 */
				operator std::unique_ptr<T, Heap::ObjectDeleter>() && noexcept {
					Heap::ObjectDeleter deleter{m_destroy};
					T* object = m_object;
					m_object = nullptr;
					m_destroy = nullptr;
					return std::unique_ptr<T, Heap::ObjectDeleter>(object, deleter);
				}

			private:
				/**
				 * @brief Adopt an object already constructed on Base's heap.
				 * @param object Object address.
				 * @param destroy Concrete destructor. It also frees the block.
				 */
				Unique(T* object, void (*destroy)(void*) noexcept) noexcept;

				template<class U>
				friend class Unique;

				template<class U, class... Args>
				friend Unique<U> MakeUnique(Args&&...);

				T* m_object; ///< Object, or null.
				void (*m_destroy)(void*) noexcept; ///< Concrete destructor, or null.
		};

		/**
		 * @class Weak
		 * @brief Non-owning observer of a @ref Shared control block.
		 * @tparam T Pointee type.
		 *
		 * Construct it only from a @ref Shared. @ref lock returns a @ref Shared, or an empty owner when the object is gone.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Weak {
			public:
				using element_type = T; ///< Pointee type.

				/**
				 * @brief Construct an empty observer.
				 */
				Weak() noexcept;

				/**
				 * @brief Construct an empty observer from null.
				 * @param null Null pointer constant.
				 */
				Weak(std::nullptr_t null) noexcept;

				/**
				 * @brief Observe an owner.
				 * @tparam U Pointee convertible to @p T.
				 * @param owner Owner to watch.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Weak(const Shared<U>& owner) noexcept;

				/**
				 * @brief Copy an observer.
				 * @param other Observer to copy.
				 */
				Weak(const Weak& other) noexcept;

				/**
				 * @brief Take an observer. @p other is left empty.
				 * @param other Observer to take.
				 */
				Weak(Weak&& other) noexcept;

				/**
				 * @brief Copy an observer.
				 * @param other Observer to copy.
				 * @return This observer.
				 */
				Weak& operator=(const Weak& other) noexcept;

				/**
				 * @brief Take an observer. @p other is left empty.
				 * @param other Observer to take.
				 * @return This observer.
				 */
				Weak& operator=(Weak&& other) noexcept;

				/**
				 * @brief Drop this observer. The last observer frees an expired block.
				 */
				~Weak();

				/**
				 * @brief Lock the control block.
				 * @return Owner of the object, or empty when expired.
				 */
				Shared<T> lock() const noexcept;

				/**
				 * @brief Test whether the object is already gone.
				 * @return Whether @ref lock would return empty.
				 */
				bool expired() const noexcept;

				/**
				 * @brief Return the number of owners.
				 * @return Strong count, or zero when expired.
				 */
				long use_count() const noexcept;

				/**
				 * @brief Drop this observer.
				 */
				void reset() noexcept;

				/**
				 * @brief Exchange observers.
				 * @param other Observer to exchange with.
				 */
				void swap(Weak& other) noexcept;

				/**
				 * @brief Order control blocks by identity.
				 * @tparam U Other pointee.
				 * @param other Other observer.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Weak<U>& other) const noexcept;

				/**
				 * @brief Order this control block against an owner.
				 * @tparam U Other pointee.
				 * @param other Owner.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Shared<U>& other) const noexcept;

			private:
				/**
				 * @brief Drop one weak reference.
				 */
				void Release() noexcept;

				template<class U>
				friend class Weak;

				template<class U>
				friend class Shared;

				template<class U>
				friend class EnableSharedFromThis;

				Detail::Control* m_control; ///< Base control block, or null.
				T* m_object; ///< Typed address observed, or null.
		};

		/**
		 * @class EnableSharedFromThis
		 * @brief Base that can recover a @ref Shared from an object owned by one.
		 * @tparam T Most derived type used with @ref MakeShared.
		 *
		 * @ref MakeShared links the embedded observer after construction. Calling @ref SharedFromThis before that link throws @ref ExpiredWeakPointerError.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE EnableSharedFromThis {
			public:
				/**
				 * @brief Construct an unlinked base.
				 */
				EnableSharedFromThis() noexcept = default;

				/**
				 * @brief Copy does not copy the observer.
				 * @param other Ignored.
				 */
				EnableSharedFromThis(const EnableSharedFromThis& other) noexcept;

				/**
				 * @brief Move does not move the observer.
				 * @param other Ignored.
				 */
				EnableSharedFromThis(EnableSharedFromThis&& other) noexcept;

				/**
				 * @brief Assignment does not replace the observer.
				 * @param other Ignored.
				 * @return This base.
				 */
				EnableSharedFromThis& operator=(const EnableSharedFromThis& other) noexcept;

				/**
				 * @brief Move assignment does not replace the observer.
				 * @param other Ignored.
				 * @return This base.
				 */
				EnableSharedFromThis& operator=(EnableSharedFromThis&& other) noexcept;

				/**
				 * @brief Destroy the embedded observer.
				 */
				~EnableSharedFromThis() = default;

			protected:
				/**
				 * @brief Recover an owner of this object.
				 * @return Owner sharing the control block created by @ref MakeShared.
				 * @throws ExpiredWeakPointerError This object is not currently owned by a @ref Shared.
				 */
				Shared<T> SharedFromThis() const;

			private:
				template<class U, class... Args>
				friend Shared<U> MakeShared(Args&&...);

				mutable Weak<T> m_weak; ///< Observer linked by @ref MakeShared.
		};

		/**
		 * @class AtomicShared
		 * @brief Atomic publication of a @ref Shared.
		 * @tparam T Pointee type.
		 *
		 * Load, store, exchange and compare-exchange adjust the same Base control block. The lock lives in this object, so two modules can publish a @ref Shared without a `libc++` control block.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE AtomicShared {
			public:
				/**
				 * @brief Construct an empty atomic owner.
				 */
				AtomicShared() noexcept;

				/**
				 * @brief Construct from an owner.
				 * @param owner Owner to publish.
				 */
				explicit AtomicShared(Shared<T> owner) noexcept;

				/**
				 * @brief Copy constructor. Deleted.
				 * @param other Ignored.
				 */
				AtomicShared(const AtomicShared& other) = delete;

				/**
				 * @brief Destroy the published owner.
				 */
				~AtomicShared();

				/**
				 * @brief Copy assignment. Deleted.
				 * @param other Ignored.
				 * @return This atomic owner.
				 */
				AtomicShared& operator=(const AtomicShared& other) = delete;

				/**
				 * @brief Publish an owner.
				 * @param owner Owner to publish.
				 * @return This atomic owner.
				 */
				AtomicShared& operator=(Shared<T> owner) noexcept;

				/**
				 * @brief Test whether an owner is published.
				 * @return Whether the published pointer is non-null.
				 */
				explicit operator bool() const noexcept;

				/**
				 * @brief Copy the published owner.
				 * @return Owner sharing the published control block.
				 */
				Shared<T> load() const noexcept;

				/**
				 * @brief Replace the published owner.
				 * @param owner Owner to publish.
				 */
				void store(Shared<T> owner) noexcept;

				/**
				 * @brief Replace the published owner and return the previous one.
				 * @param owner Owner to publish.
				 * @return Previous owner.
				 */
				Shared<T> exchange(Shared<T> owner) noexcept;

				/**
				 * @brief Replace the published owner when it compares equal by address.
				 * @param expected Expected owner. Replaced with the current owner when the exchange fails.
				 * @param desired Owner to publish.
				 * @return Whether the exchange happened.
				 */
				bool compare_exchange_strong(Shared<T>& expected, Shared<T> desired) noexcept;

				/**
				 * @brief Replace the published owner when it compares equal by address.
				 * @param expected Expected owner. Replaced with the current owner when the exchange fails.
				 * @param desired Owner to publish.
				 * @return Whether the exchange happened. May fail spuriously.
				 */
				bool compare_exchange_weak(Shared<T>& expected, Shared<T> desired) noexcept;

			private:
				/**
				 * @brief Lock the publication.
				 */
				void Lock() const noexcept;

				/**
				 * @brief Unlock the publication.
				 */
				void Unlock() const noexcept;

				mutable std::atomic_flag m_lock; ///< Publication lock.
				Detail::Control* m_control; ///< Published control block, or null.
				T* m_object; ///< Published typed address, or null.
		};

		/**
		 * @brief Construct @p T on Base's heap and share it.
		 * @tparam T Object type.
		 * @tparam Args Constructor argument types.
		 * @param args Arguments forwarded to @p T.
		 * @return Owning @ref Shared.
		 * @throws AllocationError The block could not be allocated.
		 */
		template<class T, class... Args>
		Shared<T> MakeShared(Args&&... args);

		/**
		 * @brief Construct @p T on Base's heap and own it.
		 * @tparam T Object type.
		 * @tparam Args Constructor argument types.
		 * @param args Arguments forwarded to @p T.
		 * @return Owning @ref Unique.
		 * @throws AllocationError The block could not be allocated.
		 */
		template<class T, class... Args>
		Unique<T> MakeUnique(Args&&... args);

		/**
		 * @brief `static_cast` of the stored pointer. The control block stays.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer.
		 */
		template<class T, class U>
		Shared<T> StaticPointerCast(const Shared<U>& from) noexcept;

		/**
		 * @brief `dynamic_cast` of the stored pointer. Empty when the cast fails.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer, or empty.
		 */
		template<class T, class U>
		Shared<T> DynamicPointerCast(const Shared<U>& from) noexcept;

		/**
		 * @brief `const_cast` of the stored pointer. The control block stays.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer.
		 */
		template<class T, class U>
		Shared<T> ConstPointerCast(const Shared<U>& from) noexcept;

		/**
		 * @brief `reinterpret_cast` of the stored pointer. The control block stays.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer.
		 */
		template<class T, class U>
		Shared<T> ReinterpretPointerCast(const Shared<U>& from) noexcept;

		/**
		 * @brief Same stored pointer.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param right Owner.
		 * @return Whether both hold the same address.
		 */
		template<class T>
		bool operator==(const Shared<T>& left, const Shared<T>& right) noexcept;

		/**
		 * @brief Order of the stored pointers.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param right Owner.
		 * @return Three-way comparison of the addresses.
		 */
		template<class T>
		std::strong_ordering operator<=>(const Shared<T>& left, const Shared<T>& right) noexcept;

		/**
		 * @brief Compare an owner with null.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param null Null pointer constant.
		 * @return Whether @p left is empty.
		 */
		template<class T>
		bool operator==(const Shared<T>& left, std::nullptr_t null) noexcept;

		/**
		 * @brief Same stored pointer.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param right Owner.
		 * @return Whether both hold the same address.
		 */
		template<class T>
		bool operator==(const Unique<T>& left, const Unique<T>& right) noexcept;

		/**
		 * @brief Order of the stored pointers.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param right Owner.
		 * @return Three-way comparison of the addresses.
		 */
		template<class T>
		std::strong_ordering operator<=>(const Unique<T>& left, const Unique<T>& right) noexcept;

		/**
		 * @brief Compare an owner with null.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param null Null pointer constant.
		 * @return Whether @p left is empty.
		 */
		template<class T>
		bool operator==(const Unique<T>& left, std::nullptr_t null) noexcept;

		/**
		 * @brief Exchange two owners.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param right Owner.
		 */
		template<class T>
		void swap(Shared<T>& left, Shared<T>& right) noexcept;

		/**
		 * @brief Exchange two owners.
		 * @tparam T Pointee.
		 * @param left Owner.
		 * @param right Owner.
		 */
		template<class T>
		void swap(Unique<T>& left, Unique<T>& right) noexcept;

		/**
		 * @brief Exchange two observers.
		 * @tparam T Pointee.
		 * @param left Observer.
		 * @param right Observer.
		 */
		template<class T>
		void swap(Weak<T>& left, Weak<T>& right) noexcept;
	}
}

template<class T>
struct std::hash<StormByte::Safe::Shared<T>> {
	/**
	 * @brief Hash the stored address.
	 * @param value Owner.
	 * @return Hash of the stored pointer.
	 */
	std::size_t operator()(const StormByte::Safe::Shared<T>& value) const noexcept {
		return std::hash<T*>{}(value.get());
	}
};

template<class T>
struct std::hash<StormByte::Safe::Unique<T>> {
	/**
	 * @brief Hash the stored address.
	 * @param value Owner.
	 * @return Hash of the stored pointer.
	 */
	std::size_t operator()(const StormByte::Safe::Unique<T>& value) const noexcept {
		return std::hash<T*>{}(value.get());
	}
};

#include <StormByte/safe/pointers.txx>
