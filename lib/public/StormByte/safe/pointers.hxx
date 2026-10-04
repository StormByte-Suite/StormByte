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

#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

/**
 * @file StormByte/safe/pointers.hxx
 * @brief DLL-safe shared and unique owners. Storage is on Base's heap.
 *
 * @ref StormByte::Safe::Heap is not a public header. `Allocate` / `Free` are
 * exported from the StormByte library and defined in a private translation
 * unit that is not installed.
 */

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Types that are safe to pass across a DLL boundary: owners and polymorphic clones on Base's heap.
	 */
	namespace Safe {
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Shared;

		template<class T>
		class STORMBYTE_PUBLIC_TYPE Unique;

		template<class T>
		class STORMBYTE_PUBLIC_TYPE Weak;

		/**
		 * @namespace StormByte::Safe::Heap
		 * @brief Allocate and free raw blocks on Base's heap.
		 *
		 * Declarations match the private `safe/heap.cxx` definitions. The private
		 * header is not installed; public templates need these names here.
		 */
		namespace Heap {
			/**
			 * @brief Allocate @p bytes on Base's heap.
			 * @param bytes Block size in octets. Zero is forwarded to `operator new`.
			 * @return Address of the block.
			 * @throws StormByte::AllocationError When the allocator cannot satisfy the request.
			 */
			STORMBYTE_PUBLIC void* Allocate(std::size_t bytes);

			/**
			 * @brief Release a block obtained from @ref Allocate.
			 * @param pointer Block address, or a null pointer.
			 */
			STORMBYTE_PUBLIC void Free(void* pointer) noexcept;

			/** @brief Throw Base's expired-observer exception. */
			[[noreturn]] STORMBYTE_PUBLIC void ThrowExpiredWeakPointer();

			/**
			 * @brief Preserve an active StormByte exception or translate a foreign exception.
			 * @pre Called from an active exception handler.
			 * @throws StormByte::AllocationError On allocation failure.
			 * @throws StormByte::OperationError For other foreign exceptions.
			 */
			[[noreturn]] STORMBYTE_PUBLIC void RethrowException();

			/**
			 * @struct ObjectDeleter
			 * @brief Destroy @p T and return its block to @ref Free.
			 */
			struct STORMBYTE_PUBLIC_TYPE ObjectDeleter {
				/**
				 * @brief Destroy @p pointer and free its block on Base's heap.
				 * @tparam U Pointee type.
				 * @param pointer Object constructed with placement `new` on @ref Allocate.
				 */
				template<class U>
				void operator()(U* pointer) const noexcept {
					pointer->~U();
					Free(pointer);
				}
			};

			/**
			 * @struct Allocator
			 * @brief `std::shared_ptr` allocator that uses @ref Allocate / @ref Free.
			 * @tparam T Control-block value type.
			 */
			template<typename T>
			struct STORMBYTE_PUBLIC_TYPE Allocator {
				using value_type = T;	///< Allocator value type.

				/**
				 * @brief Default constructor.
				 */
				constexpr Allocator() noexcept = default;

				/**
				 * @brief Rebind constructor.
				 * @tparam U Other allocator value type.
				 */
				template<typename U>
				constexpr Allocator(const Allocator<U>&) noexcept {}

				/**
				 * @brief Allocate @p count objects of @p T.
				 * @param count Object count.
				 * @return Storage for @p count objects.
				 */
				[[nodiscard]] T* allocate(std::size_t count) {
					return static_cast<T*>(Allocate(count * sizeof(T)));
				}

				/**
				 * @brief Release storage obtained from @ref allocate.
				 * @param pointer Storage to release.
				 */
				void deallocate(T* pointer, std::size_t) noexcept {
					Free(pointer);
				}

				/**
				 * @brief All @ref Allocator instances compare equal.
				 * @tparam U Other allocator value type.
				 * @return Always `true`.
				 */
				template<typename U>
				constexpr bool operator==(const Allocator<U>&) const noexcept {
					return true;
				}
			};

			/**
			 * @brief Construct @p T on Base's heap and wrap it in @ref Shared.
			 * @tparam T Object type.
			 * @tparam Args Constructor argument types.
			 * @param args Forwarded to @p T.
			 * @return Owning @ref Shared.
			 */
			template<class T, class... Args>
			Shared<T> MakeShared(Args&&... args);

			/**
			 * @brief Construct @p T on Base's heap and wrap it in @ref Unique.
			 * @tparam T Object type.
			 * @tparam Args Constructor argument types.
			 * @param args Forwarded to @p T.
			 * @return Owning @ref Unique.
			 */
			template<class T, class... Args>
			Unique<T> MakeUnique(Args&&... args);
		}

		/**
		 * @class Shared
		 * @brief Shared owner of a @p T allocated on Base's heap.
		 * @tparam T Pointee type.
		 *
		 * Complements `std::shared_ptr`. It does not replace it. Use `std::shared_ptr`
		 * when the object does not cross a DLL. Use @ref Shared when the object and
		 * its control block must be freed on Base's heap.
		 *
		 * Construct the exact type with @ref Heap::MakeShared. Construct a derived
		 * type with @ref Shared::MakePointer. The daily operations match
		 * `std::shared_ptr`. There is no constructor from a raw pointer or from
		 * `std::shared_ptr`, and no `release`. Implicit conversion to
		 * `std::shared_ptr<T>` keeps Base's deleter, so a `std::shared_ptr` parameter
		 * does not need a new signature. There is no conversion back.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Shared {
			public:
				using element_type = T;	///< Pointee type.
				using weak_type = Weak<T>;	///< Matching @ref Weak.

				/**
				 * @brief Empty owner.
				 */
				Shared() noexcept = default;

				/**
				 * @brief Empty owner from `nullptr`.
				 */
				Shared(std::nullptr_t) noexcept {}

				/**
				 * @brief Share ownership with @p other.
				 * @tparam U Pointee convertible to @p T.
				 * @param other Other owner.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Shared(const Shared<U>& other) noexcept: m_ptr(other.m_ptr) {}

				/**
				 * @brief Take ownership from @p other.
				 * @tparam U Pointee convertible to @p T.
				 * @param other Other owner.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Shared(Shared<U>&& other) noexcept: m_ptr(std::move(other.m_ptr)) {}

				/**
				 * @brief Copy constructor.
				 */
				Shared(const Shared&) noexcept = default;

				/**
				 * @brief Move constructor.
				 */
				Shared(Shared&&) noexcept = default;

				/**
				 * @brief Take ownership from @p weak.
				 * @param weak Observer of a @ref Shared control block.
				 * @throws StormByte::ExpiredWeakPointerError When @p weak is empty or expired.
				 */
				explicit Shared(const Weak<T>& weak);

				/**
				 * @brief Copy assignment.
				 * @return @c *this.
				 */
				Shared& operator=(const Shared&) noexcept = default;

				/**
				 * @brief Move assignment.
				 * @return @c *this.
				 */
				Shared& operator=(Shared&&) noexcept = default;

				/**
				 * @brief Destructor.
				 */
				~Shared() = default;

				/**
				 * @brief Raw pointer, or null.
				 * @return Pointee.
				 */
				T* get() const noexcept {
					return m_ptr.get();
				}

				/**
				 * @brief Dereference.
				 * @return Pointee.
				 */
				T& operator*() const {
					return *m_ptr;
				}

				/**
				 * @brief Member access.
				 * @return Pointee.
				 */
				T* operator->() const noexcept {
					return m_ptr.get();
				}

				/**
				 * @brief Whether this owner holds an object.
				 * @return @c true when non-empty.
				 */
				explicit operator bool() const noexcept {
					return static_cast<bool>(m_ptr);
				}

				/**
				 * @brief Same control block as `std::shared_ptr`. Deleter stays Base.
				 * @return `std::shared_ptr<T>` that still frees on Base's heap.
				 */
				operator std::shared_ptr<T>() const noexcept {
					return m_ptr;
				}

				/**
				 * @brief Number of @ref Shared sharing this object.
				 * @return Use count.
				 */
				long use_count() const noexcept {
					return m_ptr.use_count();
				}

				/**
				 * @brief Drop this owner's reference.
				 */
				void reset() noexcept {
					m_ptr.reset();
				}

				/**
				 * @brief Exchange owners with @p other.
				 * @param other Other owner.
				 */
				void swap(Shared& other) noexcept {
					m_ptr.swap(other.m_ptr);
				}

				/**
				 * @brief Ownership order, same as `std::shared_ptr::owner_before`.
				 * @tparam U Other pointee.
				 * @param other Other owner.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Shared<U>& other) const noexcept {
					return m_ptr.owner_before(other.m_ptr);
				}

				/**
				 * @brief Ownership order against a @ref Weak.
				 * @tparam U Other pointee.
				 * @param other Observer.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Weak<U>& other) const noexcept;

				/**
				 * @brief Allocate @p Target on Base's heap and own it as @p T.
				 * @tparam Target Concrete type (`T` or derived from @p T).
				 * @tparam Args Constructor argument types.
				 * @param args Forwarded to `Target`.
				 * @return Owner of the `Target` object. The deleter still destroys `Target`.
				 */
				template<class Target, class... Args>
				requires Type::SameAs<Target, T> || Type::DerivedFrom<Target, T>
				static Shared<T> MakePointer(Args&&... args) {
					return Heap::MakeShared<Target>(std::forward<Args>(args)...);
				}

				/**
				 * @brief Equality with null.
				 * @param lhs Owner.
				 * @return Whether @p lhs is empty.
				 */
				friend bool operator==(const Shared& lhs, std::nullptr_t) noexcept {
					return !lhs.m_ptr;
				}

				/**
				 * @brief Inequality with null.
				 * @param lhs Owner.
				 * @return Whether @p lhs is non-empty.
				 */
				friend bool operator!=(const Shared& lhs, std::nullptr_t) noexcept {
					return static_cast<bool>(lhs.m_ptr);
				}

				/**
				 * @brief Equality with null.
				 * @param rhs Owner.
				 * @return Whether @p rhs is empty.
				 */
				friend bool operator==(std::nullptr_t, const Shared& rhs) noexcept {
					return !rhs.m_ptr;
				}

				/**
				 * @brief Inequality with null.
				 * @param rhs Owner.
				 * @return Whether @p rhs is non-empty.
				 */
				friend bool operator!=(std::nullptr_t, const Shared& rhs) noexcept {
					return static_cast<bool>(rhs.m_ptr);
				}

				/**
				 * @brief Same stored pointer.
				 * @param lhs Owner.
				 * @param rhs Owner.
				 * @return Whether both hold the same address.
				 */
				friend bool operator==(const Shared& lhs, const Shared& rhs) noexcept {
					return lhs.get() == rhs.get();
				}

				/**
				 * @brief Order of the stored pointers.
				 * @param lhs Owner.
				 * @param rhs Owner.
				 * @return Three-way comparison of the addresses.
				 */
				friend auto operator<=>(const Shared& lhs, const Shared& rhs) noexcept {
					return std::compare_three_way{}(lhs.get(), rhs.get());
				}

				/**
				 * @brief Exchange @p left and @p right.
				 * @param left Owner.
				 * @param right Owner.
				 */
				friend void swap(Shared& left, Shared& right) noexcept {
					left.swap(right);
				}

			private:
				/**
				 * @brief Wrap a control block that already belongs to Base.
				 * @param pointer Block created by @ref Heap::MakeShared or by a cast of one.
				 */
				explicit Shared(std::shared_ptr<T> pointer) noexcept: m_ptr(std::move(pointer)) {}

				/**
				 * @brief Tag for @ref Heap::MakeShared.
				 */
				struct Adopt {
					explicit constexpr Adopt() noexcept = default;
				};

				/**
				 * @brief Take a block already constructed on Base's heap.
				 * @param object Placement-new address from @ref Heap::Allocate.
				 */
				explicit Shared(Adopt, T* object): m_ptr(object, Heap::ObjectDeleter{}, Heap::Allocator<T>{}) {}

				template<class U>
				friend class Shared;

				template<class U>
				friend class Weak;

				template<class U, class... Args>
				friend Shared<U> Heap::MakeShared(Args&&...);

				template<class X, class Y>
				friend Shared<X> StaticPointerCast(const Shared<Y>&) noexcept;

				template<class X, class Y>
				friend Shared<X> DynamicPointerCast(const Shared<Y>&) noexcept;

				template<class X, class Y>
				friend Shared<X> ConstPointerCast(const Shared<Y>&) noexcept;

				template<class X, class Y>
				friend Shared<X> ReinterpretPointerCast(const Shared<Y>&) noexcept;

				std::shared_ptr<T> m_ptr;	///< Control block and object on Base's heap
		};

		/**
		 * @class Unique
		 * @brief Unique owner of a @p T allocated on Base's heap.
		 * @tparam T Pointee type.
		 *
		 * Complements `std::unique_ptr`. It does not replace it. Use `std::unique_ptr`
		 * when the object does not cross a DLL. Use @ref Unique when the object must
		 * be freed on Base's heap.
		 *
		 * Construct the exact type with @ref Heap::MakeUnique. Construct a derived
		 * type with @ref Unique::MakePointer. `~T` must be virtual when @p Target
		 * is not @p T, because the deleter calls `~T`. There is no `release` and no
		 * constructor from a raw pointer. Converts on move to
		 * `std::unique_ptr<T, Heap::ObjectDeleter>`, not to `std::unique_ptr<T>`.
		 * A signature that takes `std::unique_ptr<T>` has to change.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Unique {
			public:
				using element_type = T;	///< Pointee type.
				using pointer = T*;	///< Stored pointer.
				using deleter_type = Heap::ObjectDeleter;	///< Deleter that calls @ref Heap::Free.

				/**
				 * @brief Empty owner.
				 */
				Unique() noexcept = default;

				/**
				 * @brief Empty owner from `nullptr`.
				 */
				Unique(std::nullptr_t) noexcept {}

				/**
				 * @brief Take ownership from @p other.
				 * @tparam U Pointee convertible to @p T.
				 * @param other Other owner.
				 */
				template<class U>
				requires Type::SameAs<U, T> || (Type::DerivedFrom<U, T> && std::has_virtual_destructor_v<T>)
				Unique(Unique<U>&& other) noexcept: m_ptr(std::move(other.m_ptr)) {}

				/**
				 * @brief Copy constructor. Deleted.
				 */
				Unique(const Unique&) = delete;

				/**
				 * @brief Move constructor.
				 */
				Unique(Unique&&) noexcept = default;

				/**
				 * @brief Copy assignment. Deleted.
				 * @return @c *this.
				 */
				Unique& operator=(const Unique&) = delete;

				/**
				 * @brief Move assignment.
				 * @return @c *this.
				 */
				Unique& operator=(Unique&&) noexcept = default;

				/**
				 * @brief Destructor.
				 */
				~Unique() = default;

				/**
				 * @brief Raw pointer, or null.
				 * @return Pointee.
				 */
				T* get() const noexcept {
					return m_ptr.get();
				}

				/**
				 * @brief Dereference.
				 * @return Pointee.
				 */
				T& operator*() const {
					return *m_ptr;
				}

				/**
				 * @brief Member access.
				 * @return Pointee.
				 */
				T* operator->() const noexcept {
					return m_ptr.get();
				}

				/**
				 * @brief Whether this owner holds an object.
				 * @return @c true when non-empty.
				 */
				explicit operator bool() const noexcept {
					return static_cast<bool>(m_ptr);
				}

				/**
				 * @brief Release as `std::unique_ptr` with Base's deleter.
				 * @return `std::unique_ptr<T, Heap::ObjectDeleter>`.
				 */
				operator std::unique_ptr<T, Heap::ObjectDeleter>() && noexcept {
					return std::move(m_ptr);
				}

				/**
				 * @brief Deleter stored in this owner.
				 * @return @ref Heap::ObjectDeleter.
				 */
				deleter_type& get_deleter() noexcept {
					return m_ptr.get_deleter();
				}

				/**
				 * @brief Deleter stored in this owner.
				 * @return @ref Heap::ObjectDeleter.
				 */
				const deleter_type& get_deleter() const noexcept {
					return m_ptr.get_deleter();
				}

				/**
				 * @brief Destroy the object and drop this owner.
				 */
				void reset() noexcept {
					m_ptr.reset();
				}

				/**
				 * @brief Exchange owners with @p other.
				 * @param other Other owner.
				 */
				void swap(Unique& other) noexcept {
					m_ptr.swap(other.m_ptr);
				}

				/**
				 * @brief Allocate @p Target on Base's heap and own it as @p T.
				 * @tparam Target Concrete type (`T` or derived from @p T).
				 * @tparam Args Constructor argument types.
				 * @param args Forwarded to `Target`.
				 * @return Owner of the `Target` object.
				 * @note When @p Target is not @p T, @p T must have a virtual destructor.
				 */
				template<class Target, class... Args>
				requires Type::SameAs<Target, T> || (Type::DerivedFrom<Target, T> && std::has_virtual_destructor_v<T>)
				static Unique<T> MakePointer(Args&&... args) {
					return Heap::MakeUnique<Target>(std::forward<Args>(args)...);
				}

				/**
				 * @brief Equality with null.
				 * @param lhs Owner.
				 * @return Whether @p lhs is empty.
				 */
				friend bool operator==(const Unique& lhs, std::nullptr_t) noexcept {
					return !lhs.m_ptr;
				}

				/**
				 * @brief Inequality with null.
				 * @param lhs Owner.
				 * @return Whether @p lhs is non-empty.
				 */
				friend bool operator!=(const Unique& lhs, std::nullptr_t) noexcept {
					return static_cast<bool>(lhs.m_ptr);
				}

				/**
				 * @brief Equality with null.
				 * @param rhs Owner.
				 * @return Whether @p rhs is empty.
				 */
				friend bool operator==(std::nullptr_t, const Unique& rhs) noexcept {
					return !rhs.m_ptr;
				}

				/**
				 * @brief Inequality with null.
				 * @param rhs Owner.
				 * @return Whether @p rhs is non-empty.
				 */
				friend bool operator!=(std::nullptr_t, const Unique& rhs) noexcept {
					return static_cast<bool>(rhs.m_ptr);
				}

				/**
				 * @brief Same stored pointer.
				 * @param lhs Owner.
				 * @param rhs Owner.
				 * @return Whether both hold the same address.
				 */
				friend bool operator==(const Unique& lhs, const Unique& rhs) noexcept {
					return lhs.get() == rhs.get();
				}

				/**
				 * @brief Order of the stored pointers.
				 * @param lhs Owner.
				 * @param rhs Owner.
				 * @return Three-way comparison of the addresses.
				 */
				friend auto operator<=>(const Unique& lhs, const Unique& rhs) noexcept {
					return std::compare_three_way{}(lhs.get(), rhs.get());
				}

				/**
				 * @brief Exchange @p left and @p right.
				 * @param left Owner.
				 * @param right Owner.
				 */
				friend void swap(Unique& left, Unique& right) noexcept {
					left.swap(right);
				}

			private:
				/**
				 * @brief Tag for @ref Heap::MakeUnique.
				 */
				struct Adopt {
					explicit constexpr Adopt() noexcept = default;
				};

				/**
				 * @brief Take a block already constructed on Base's heap.
				 * @param object Placement-new address from @ref Heap::Allocate.
				 */
				explicit Unique(Adopt, T* object): m_ptr(object) {}

				template<class U>
				friend class Unique;

				template<class U, class... Args>
				friend Unique<U> Heap::MakeUnique(Args&&...);

				std::unique_ptr<T, Heap::ObjectDeleter> m_ptr;	///< Object on Base's heap
		};

		/**
		 * @class Weak
		 * @brief Non-owning observer of a @ref Shared control block.
		 * @tparam T Pointee type.
		 *
		 * Complements `std::weak_ptr`. It does not replace it. Construct it only
		 * from a @ref Shared. `lock` returns a @ref Shared, or an empty owner when
		 * the object is gone. There is no constructor from `std::weak_ptr`.
		 */
		template<class T>
		class STORMBYTE_PUBLIC_TYPE Weak {
			public:
				using element_type = T;	///< Pointee type.

				/**
				 * @brief Empty observer.
				 */
				Weak() noexcept = default;

				/**
				 * @brief Empty observer from `nullptr`.
				 */
				Weak(std::nullptr_t) noexcept {}

				/**
				 * @brief Observe @p owner.
				 * @tparam U Pointee convertible to @p T.
				 * @param owner Owner to watch.
				 */
				template<class U>
				requires Type::SameAs<U, T> || Type::DerivedFrom<U, T>
				Weak(const Shared<U>& owner) noexcept: m_weak(owner.m_ptr) {}

				/**
				 * @brief Copy constructor.
				 */
				Weak(const Weak&) noexcept = default;

				/**
				 * @brief Move constructor.
				 */
				Weak(Weak&&) noexcept = default;

				/**
				 * @brief Copy assignment.
				 * @return @c *this.
				 */
				Weak& operator=(const Weak&) noexcept = default;

				/**
				 * @brief Move assignment.
				 * @return @c *this.
				 */
				Weak& operator=(Weak&&) noexcept = default;

				/**
				 * @brief Destructor.
				 */
				~Weak() = default;

				/**
				 * @brief Lock the control block.
				 * @return @ref Shared owning the object, or empty when expired.
				 */
				Shared<T> lock() const noexcept {
					return Shared<T>(m_weak.lock());
				}

				/**
				 * @brief Whether the object is already gone.
				 * @return @c true when @ref lock would return empty.
				 */
				bool expired() const noexcept {
					return m_weak.expired();
				}

				/**
				 * @brief Number of @ref Shared still holding the object.
				 * @return Use count, or `0` when expired.
				 */
				long use_count() const noexcept {
					return m_weak.use_count();
				}

				/**
				 * @brief Drop this observer.
				 */
				void reset() noexcept {
					m_weak.reset();
				}

				/**
				 * @brief Exchange observers with @p other.
				 * @param other Other observer.
				 */
				void swap(Weak& other) noexcept {
					m_weak.swap(other.m_weak);
				}

				/**
				 * @brief Ownership order, same as `std::weak_ptr::owner_before`.
				 * @tparam U Other pointee.
				 * @param other Other observer.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Weak<U>& other) const noexcept {
					return m_weak.owner_before(other.m_weak);
				}

				/**
				 * @brief Ownership order against a @ref Shared.
				 * @tparam U Other pointee.
				 * @param other Owner.
				 * @return Whether this control block precedes @p other.
				 */
				template<class U>
				bool owner_before(const Shared<U>& other) const noexcept {
					return m_weak.owner_before(other.m_ptr);
				}

				/**
				 * @brief Exchange @p left and @p right.
				 * @param left Observer.
				 * @param right Observer.
				 */
				friend void swap(Weak& left, Weak& right) noexcept {
					left.swap(right);
				}

			private:
				template<class U>
				friend class Weak;

				template<class U>
				friend class Shared;

				std::weak_ptr<T> m_weak;	///< Observer of a Base control block
		};

		/**
		 * @brief `static_cast` of the stored pointer. The control block stays.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer.
		 */
		template<class T, class U>
		Shared<T> StaticPointerCast(const Shared<U>& from) noexcept {
			return Shared<T>(std::static_pointer_cast<T>(std::shared_ptr<U>(from)));
		}

		/**
		 * @brief `dynamic_cast` of the stored pointer. Empty when the cast fails.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer, or empty.
		 */
		template<class T, class U>
		Shared<T> DynamicPointerCast(const Shared<U>& from) noexcept {
			return Shared<T>(std::dynamic_pointer_cast<T>(std::shared_ptr<U>(from)));
		}

		/**
		 * @brief `const_cast` of the stored pointer. The control block stays.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer.
		 */
		template<class T, class U>
		Shared<T> ConstPointerCast(const Shared<U>& from) noexcept {
			return Shared<T>(std::const_pointer_cast<T>(std::shared_ptr<U>(from)));
		}

		/**
		 * @brief `reinterpret_cast` of the stored pointer. The control block stays.
		 * @tparam T Target pointee.
		 * @tparam U Source pointee.
		 * @param from Owner.
		 * @return Owner of the cast pointer.
		 */
		template<class T, class U>
		Shared<T> ReinterpretPointerCast(const Shared<U>& from) noexcept {
			return Shared<T>(std::reinterpret_pointer_cast<T>(std::shared_ptr<U>(from)));
		}

		template<class T>
		Shared<T>::Shared(const Weak<T>& weak): m_ptr(weak.m_weak.lock()) {
			if (!m_ptr)
				Heap::ThrowExpiredWeakPointer();
		}

		template<class T>
		template<class U>
		bool Shared<T>::owner_before(const Weak<U>& other) const noexcept {
			return m_ptr.owner_before(other.m_weak);
		}
	}
}


template<class T>
struct std::hash<StormByte::Safe::Shared<T>> {
	/**
	 * @brief Hashes the stored address.
	 * @param value Owner.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::Shared<T>& value) const noexcept {
		return std::hash<T*>{}(value.get());
	}
};

template<class T>
struct std::hash<StormByte::Safe::Unique<T>> {
	/**
	 * @brief Hashes the stored address.
	 * @param value Owner.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::Unique<T>& value) const noexcept {
		return std::hash<T*>{}(value.get());
	}
};

#include <StormByte/safe/pointers.txx>
