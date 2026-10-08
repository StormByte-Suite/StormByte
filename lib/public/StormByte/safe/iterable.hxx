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

#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <iterator>
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
		/**
		 * @class Iterable
		 * @brief Gives a Safe component the iterators of its category.
		 * @tparam Component Safe component that already stores the elements.
		 *
		 * The derived type does not write cursors and does not keep a second container. @ref m_container is the storage. A random-access component gets an index cursor. A bidirectional component gets an ordered cursor. A type that is not a Safe component is rejected. A maybe-safe mark is the consumer's promise that its own allocations stay on Base's heap.
		 */
		template<Type::SafeComponent Component>
		requires requires(Component& component, const Component& readonly) {
			typename Component::value_type;
			{ component.begin() };
			{ component.end() };
			{ readonly.begin() };
			{ readonly.end() };
			{ readonly.size() } -> Type::ConvertibleTo<std::size_t>;
		}
		class STORMBYTE_PUBLIC_TYPE Iterable {
				using InnerIterator = decltype(std::declval<Component&>().begin());
				using InnerConstIterator = decltype(std::declval<const Component&>().begin());

			public:
				using container_type = Component; ///< Wrapped Safe component.
				using value_type = typename container_type::value_type; ///< Element type.
				using size_type = std::size_t; ///< Element count type.
				using difference_type = std::ptrdiff_t; ///< Cursor distance type.
				static constexpr bool random_access = std::random_access_iterator<InnerIterator>; ///< Whether the suite selects the index cursor.

				/**
				 * @class BasicIterator
				 * @brief Suite cursor. The category follows the wrapped component.
				 * @tparam IsConst Whether dereference is read-only.
				 */
				template<bool IsConst>
				class BasicIterator final {
						using Inner = std::conditional_t<IsConst, InnerConstIterator, InnerIterator>;

					public:
						using iterator_category = std::conditional_t<random_access, std::random_access_iterator_tag, std::bidirectional_iterator_tag>; ///< Iterator category.
						using iterator_concept = iterator_category; ///< C++20 iterator concept.
						using value_type = typename Iterable::value_type; ///< Element type.
						using difference_type = std::ptrdiff_t; ///< Cursor distance type.
						using reference = decltype(*std::declval<Inner>()); ///< Element reference.
						using pointer = void; ///< The cursor does not expose a raw component pointer.

						/**
						 * @brief Construct a singular cursor.
						 */
						BasicIterator() = default;

						/**
						 * @brief Convert a mutable cursor to a read-only cursor.
						 * @param other Mutable cursor.
						 */
						template<bool OtherConst>
						requires IsConst && (!OtherConst)
						BasicIterator(const BasicIterator<OtherConst>& other) noexcept;

						/**
						 * @brief Return the current element.
						 * @return Element reference. Valid until the component reallocates or erases it.
						 */
						reference operator*() const;

						/**
						 * @brief Advance one element.
						 * @return This cursor.
						 */
						BasicIterator& operator++();

						/**
						 * @brief Advance one element.
						 * @return Previous cursor value.
						 */
						BasicIterator operator++(int);

						/**
						 * @brief Move to the previous element.
						 * @return This cursor.
						 */
						BasicIterator& operator--();

						/**
						 * @brief Move to the previous element.
						 * @return Previous cursor value.
						 */
						BasicIterator operator--(int);

						/**
						 * @brief Advance by an offset. Random-access components only.
						 * @param offset Element offset.
						 * @return This cursor.
						 */
						BasicIterator& operator+=(difference_type offset) requires random_access;

						/**
						 * @brief Retreat by an offset. Random-access components only.
						 * @param offset Element offset.
						 * @return This cursor.
						 */
						BasicIterator& operator-=(difference_type offset) requires random_access;

						/**
						 * @brief Return the element at an offset. Random-access components only.
						 * @param offset Element offset.
						 * @return Element reference.
						 */
						reference operator[](difference_type offset) const requires random_access;

						/**
						 * @brief Return the cursor advanced by an offset. Random-access components only.
						 * @param offset Element offset.
						 * @return Advanced cursor.
						 */
						BasicIterator operator+(difference_type offset) const requires random_access;

						/**
						 * @brief Advance a cursor from the offset. Required by random_access_iterator.
						 * @param offset Element offset.
						 * @param cursor Cursor to advance.
						 * @return Advanced cursor.
						 */
						friend BasicIterator operator+(difference_type offset, const BasicIterator& cursor) requires random_access {
							return cursor + offset;
						}

						/**
						 * @brief Return the cursor retreated by an offset. Random-access components only.
						 * @param offset Element offset.
						 * @return Retreated cursor.
						 */
						BasicIterator operator-(difference_type offset) const requires random_access;

						/**
						 * @brief Return the distance to another cursor. Random-access components only.
						 * @param other Cursor to measure against.
						 * @return Element distance.
						 */
						template<bool OtherConst>
						difference_type operator-(const BasicIterator<OtherConst>& other) const requires random_access;

						/**
						 * @brief Compare cursor position.
						 * @param other Cursor to compare.
						 * @return Whether both designate the same element.
						 */
						template<bool OtherConst>
						bool operator==(const BasicIterator<OtherConst>& other) const;

						/**
						 * @brief Order two cursors. Random-access components only.
						 * @param other Cursor to compare.
						 * @return Whether this cursor precedes @p other.
						 */
						template<bool OtherConst>
						bool operator<(const BasicIterator<OtherConst>& other) const requires random_access;

						/**
						 * @brief Order two cursors. Random-access components only.
						 * @param other Cursor to compare.
						 * @return Whether this cursor follows @p other.
						 */
						template<bool OtherConst>
						bool operator>(const BasicIterator<OtherConst>& other) const requires random_access;

						/**
						 * @brief Order two cursors. Random-access components only.
						 * @param other Cursor to compare.
						 * @return Whether this cursor precedes or equals @p other.
						 */
						template<bool OtherConst>
						bool operator<=(const BasicIterator<OtherConst>& other) const requires random_access;

						/**
						 * @brief Order two cursors. Random-access components only.
						 * @param other Cursor to compare.
						 * @return Whether this cursor follows or equals @p other.
						 */
						template<bool OtherConst>
						bool operator>=(const BasicIterator<OtherConst>& other) const requires random_access;

					private:
						friend class Iterable;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind the cursor to a component iterator.
						 * @param current Component cursor.
						 */
						explicit BasicIterator(Inner current) noexcept;

						Inner m_current{}; ///< Component cursor. Not part of the public type.
				};

				using iterator = BasicIterator<false>; ///< Mutable cursor.
				using const_iterator = BasicIterator<true>; ///< Read-only cursor.

				/**
				 * @brief Construct an empty component.
				 */
				Iterable() = default;

				/**
				 * @brief Copy the component.
				 * @param other Source.
				 */
				Iterable(const Iterable& other) = default;

				/**
				 * @brief Move the component. @p other is left empty.
				 * @param other Source.
				 */
				Iterable(Iterable&& other) noexcept = default;

				/**
				 * @brief Take ownership of an existing component.
				 * @param container Component to store.
				 */
				explicit Iterable(container_type container) noexcept;

				/**
				 * @brief Destroy the component through its Safe operations.
				 */
				~Iterable() = default;

				/**
				 * @brief Copy-assign the component.
				 * @param other Source.
				 * @return This object.
				 */
				Iterable& operator=(const Iterable& other) = default;

				/**
				 * @brief Move-assign the component. @p other is left empty.
				 * @param other Source.
				 * @return This object.
				 */
				Iterable& operator=(Iterable&& other) noexcept = default;

				/**
				 * @brief Return a mutable cursor to the first element.
				 * @return Mutable cursor.
				 */
				iterator begin() noexcept;

				/**
				 * @brief Return the mutable end cursor.
				 * @return End cursor.
				 */
				iterator end() noexcept;

				/**
				 * @brief Return a read-only cursor to the first element.
				 * @return Read-only cursor.
				 */
				const_iterator begin() const noexcept;

				/**
				 * @brief Return the read-only end cursor.
				 * @return End cursor.
				 */
				const_iterator end() const noexcept;

				/**
				 * @brief Return a read-only cursor to the first element.
				 * @return Read-only cursor.
				 */
				const_iterator cbegin() const noexcept;

				/**
				 * @brief Return the read-only end cursor.
				 * @return End cursor.
				 */
				const_iterator cend() const noexcept;

				/**
				 * @brief Test whether the component has no elements.
				 * @return Whether the component is empty.
				 */
				bool empty() const noexcept;

				/**
				 * @brief Return the element count.
				 * @return Element count.
				 */
				size_type size() const noexcept;

			protected:
				/**
				 * @brief Return the stored component.
				 * @return Mutable component.
				 */
				container_type& Container() noexcept;

				/**
				 * @brief Return the stored component.
				 * @return Read-only component.
				 */
				const container_type& Container() const noexcept;

				container_type m_container{}; ///< Stored Safe component. This is the only storage.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes an iterable facade of an already safe component.
		 * @tparam Component Safe component.
		 */
		template<SafeComponent Component>
		requires IsSafe<std::remove_const_t<Component>>::value
		struct IsSafe<Safe::Iterable<Component>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from the wrapped component.
		 * @tparam Component Safe component.
		 */
		template<SafeComponent Component>
		requires MaybeSafe<Component>
		struct IsMaybeSafe<Safe::Iterable<Component>>: std::true_type {};

		/**
		 * @brief Admits an iterable facade as a collection value.
		 * @tparam Component Safe component.
		 */
		template<SafeComponent Component>
		struct IsSafeValue<Safe::Iterable<Component>>: std::true_type {};
	}
}

#include <StormByte/safe/iterable.txx>
