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

#include <StormByte/safe/pointers.hxx>

/**
 * @file StormByte/safe/clonable.hxx
 * @brief Polymorphic clone/move whose pointers own storage on Base's heap.
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
		/**
		 * @brief `true` when @p SmartPointer is @ref StormByte::Safe::Shared<T> or @ref StormByte::Safe::Unique<T>.
		 * @tparam SmartPointer Candidate pointer type.
		 * @tparam T Pointee type.
		 *
		 * `std::shared_ptr<T>` and `std::unique_ptr<T>` are not accepted.
		 */
		template<typename SmartPointer, typename T>
		concept ValidSmartPointer =
			Type::SameAs<SmartPointer, Shared<T>> ||
			Type::SameAs<SmartPointer, Unique<T>>;

		/**
		 * @class Clonable
		 * @brief Polymorphic copy and move through a base.
		 * @tparam T Interface stored in the pointer.
		 * @tparam SmartPointer @ref StormByte::Safe::Shared<T> (default) or @ref StormByte::Safe::Unique<T>.
		 *
		 * Not an owner. @ref StormByte::Safe::Shared<T> and @ref StormByte::Safe::Unique<T> own the object on Base's heap.
		 * @ref Clonable is the interface: `Clone` and `Move` return that owner for
		 * the dynamic type, so the caller does not name the derived class.
		 * `MakePointer` forwards to `Shared::MakePointer` or `Unique::MakePointer`.
		 *
		 * `Clonable<T>` is `Clonable<T, Shared<T>>`. Unique ownership is
		 * `Clonable<T, Unique<T>>`. `std::shared_ptr` and `std::unique_ptr` are
		 * not a `PointerType`.
		 *
		 * Safe to derive from in another DLL. Storage goes through the exported
		 * @ref Heap::Allocate / @ref Heap::Free, so it is always Base's heap. Every
		 * module instantiates its own `Clonable<T>` (vtable and `typeinfo`);
		 * @ref STORMBYTE_PUBLIC_TYPE gives those copies default visibility on
		 * ELF and Mach-O, so `dynamic_cast` and `typeid` agree across modules
		 * even when the deriving module builds with hidden visibility (on ELF
		 * the loader also merges them into one object). @p T must be exported
		 * by its own module.
		 */
		template<class T, typename SmartPointer = Shared<T>>
		requires ValidSmartPointer<SmartPointer, T> class STORMBYTE_PUBLIC_TYPE Clonable {
			public:
				using PointerType = SmartPointer;	///< `SmartPointer` alias.

				/**
				 * @brief Allocates @p Target on Base's heap and returns it as `PointerType`.
				 * @tparam Target Concrete type (`T` or derived).
				 * @tparam Args Constructor argument types.
				 * @param args Forwarded to `Target`.
				 * @return @ref StormByte::Safe::Shared<T> or @ref StormByte::Safe::Unique<T> according to `PointerType`.
				 */
				template<class Target, typename... Args>
				static PointerType MakePointer(Args&&... args);

				/**
				 * @brief Default constructor.
				 */
				constexpr Clonable() = default;

				/**
				 * @brief Copy constructor.
				 */
				constexpr Clonable(const Clonable&) = default;

				/**
				 * @brief Move constructor.
				 */
				constexpr Clonable(Clonable&&) noexcept = default;

				/**
				 * @brief Copy assignment.
				 */
				constexpr Clonable& operator=(const Clonable&) = default;

				/**
				 * @brief Move assignment.
				 */
				constexpr Clonable& operator=(Clonable&&) noexcept = default;

				/**
				 * @brief Destructor.
				 */
				virtual constexpr ~Clonable() noexcept = default;

				/**
				 * @brief Deep copy into a new `PointerType`.
				 * @return Owning pointer to the clone.
				 */
				virtual PointerType Clone() const = 0;

				/**
				 * @brief Moves this object into a new `PointerType`.
				 * @return Owning pointer to the moved instance.
				 */
				virtual PointerType Move() = 0;
		};
	}
	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes Safe polymorphic clone interfaces, without certifying derived implementations.
		 * @tparam T Interface stored in the pointer.
		 * @tparam SmartPointer Safe shared or unique pointer to the interface.
		 */
		template<typename T, typename SmartPointer>
		requires Safe::ValidSmartPointer<SmartPointer, T> && IsSafe<T>::value && IsSafe<SmartPointer>::value
		struct IsSafe<Safe::Clonable<T, SmartPointer>>: std::true_type {};

		/**
		 * @brief Propagates conditional pointee or owner safety to Clonable.
		 * @tparam T Interface type.
		 * @tparam SmartPointer Safe shared or unique owner.
		 */
		template<typename T, typename SmartPointer>
		requires Safe::ValidSmartPointer<SmartPointer, T> && SafeComponent<T> &&
			SafeComponent<SmartPointer> && (MaybeSafe<T> || MaybeSafe<SmartPointer>)
		struct IsMaybeSafe<Safe::Clonable<T, SmartPointer>>: std::true_type {};
	}
}

#include <StormByte/safe/clonable.txx>
