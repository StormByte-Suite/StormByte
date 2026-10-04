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

#include <StormByte/safe/owner.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits/safe.hxx>
#include <StormByte/visibility.h>

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
	 */
	namespace Safe {
		/**
		 * @namespace StormByte::Safe::Detail
		 * @brief Private implementation helpers for Safe containers.
		 */
		namespace Detail {
			/**
			 * @brief Sequence containers admitted by Safe::Iterable.
			 * @tparam Container Candidate standard-style sequence container.
			 */
			template<class Container>
			concept SafeRandomAccessContainer = requires {
				typename Container::value_type;
				typename Container::iterator;
				typename Container::const_iterator;
			} && Type::SafeValue<typename Container::value_type> &&
				std::move_constructible<typename Container::value_type> &&
				std::random_access_iterator<typename Container::iterator> &&
				std::random_access_iterator<typename Container::const_iterator> &&
				requires(Container container, typename Container::value_type value, std::size_t index) {
					{ container.size() } -> std::convertible_to<std::size_t>;
					container[index];
					container.push_back(value);
					container.erase(container.begin());
					container.clear();
				};
		}

		/**
		 * @class Iterable
		 * @brief Opaque safe owner for a random-access sequence container.
		 * @tparam Container Standard-style random-access sequence whose value_type is a SafeValue.
		 *
		 * The container and all of its allocations stay in the module that creates
		 * each object. Iterators carry callback-backed proxies, never references,
		 * pointers, or iterators into the owner module. Modifying algorithms operate
		 * through those proxies and cannot change the container's size except through
		 * the explicit container operations. Creator modules must remain loaded.
		 */
		template<class Container>
		class STORMBYTE_PUBLIC_TYPE Iterable final {
			public:
				static_assert(Detail::SafeRandomAccessContainer<Container>,
					"Safe::Iterable requires a SafeValue random-access sequence container");
				using value_type = typename Container::value_type; ///< Stored value type.
				using size_type = std::size_t; ///< Element count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.

				/**
				 * @class Reference
				 * @brief Mutable callback-backed proxy to one element.
			 */
				class Reference final {
					public:
						/**
						 * @brief Copy a proxy binding.
						 * @param other Source proxy.
						 */
						Reference(const Reference& other) = default;

						/**
						 * @brief Move a proxy binding.
						 * @param other Source proxy.
						 */
						Reference(Reference&& other) = default;

						/**
						 * @brief Read the current element value.
						 * @return Reference to this proxy's caller-module snapshot.
					 */
					operator const value_type&() const {
						if (m_owner->ReadAt(m_index, m_value) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe iterable read failed");
						return m_value;
					}

					/**
					 * @brief Copy-assign a value through the owner callback.
					 * @param value Replacement value.
					 * @return This proxy.
					 */
					Reference& operator=(const value_type& value) {
						Write(value);
						return *this;
					}

					/**
					 * @brief Copy-assign through a const proxy for indirectly-writable iterators.
					 * @param value Replacement value.
					 * @return This proxy.
					 */
					const Reference& operator=(const value_type& value) const {
						Write(value);
						return *this;
					}

					/**
					 * @brief Copy-assign from another proxy.
					 * @param other Source proxy.
					 * @return This proxy.
					 */
					Reference& operator=(const Reference& other) {
						return *this = static_cast<value_type>(other);
					}

					/**
					 * @brief Compare the proxy value with a convertible value.
					 * @tparam Other Other operand type.
					 * @param left Proxy operand.
					 * @param right Value convertible to value_type.
					 * @return Whether the values are equal.
					 */
					template<class Other>
					requires requires(const Other& value) { value_type(value); }
					friend bool operator==(Reference left, const Other& right) {
						return static_cast<value_type>(left) == value_type(right);
					}

					/**
					 * @brief Compare a convertible value with the proxy value.
					 * @tparam Other Other operand type.
					 * @param left Value convertible to value_type.
					 * @param right Proxy operand.
					 * @return Whether the values are equal.
					 */
					template<class Other>
					requires requires(const Other& value) { value_type(value); }
					friend bool operator==(const Other& left, Reference right) {
						return value_type(left) == static_cast<value_type>(right);
					}

					/**
					 * @brief Order the proxy value before a convertible value.
					 * @tparam Other Other operand type.
					 * @param left Proxy operand.
					 * @param right Value convertible to value_type.
					 * @return Whether left precedes right.
					 */
					template<class Other>
					requires requires(const Other& value) { value_type(value); }
					friend bool operator<(Reference left, const Other& right) {
						return static_cast<value_type>(left) < value_type(right);
					}

					/**
					 * @brief Order a convertible value before the proxy value.
					 * @tparam Other Other operand type.
					 * @param left Value convertible to value_type.
					 * @param right Proxy operand.
					 * @return Whether left precedes right.
					 */
					template<class Other>
					requires requires(const Other& value) { value_type(value); }
					friend bool operator<(const Other& left, Reference right) {
						return value_type(left) < static_cast<value_type>(right);
					}

					/**
					 * @brief Order a proxy value after a convertible value.
					 * @tparam Other Other operand type.
					 * @param left Proxy operand.
					 * @param right Value convertible to value_type.
					 * @return Whether left follows right.
					 */
					template<class Other>
						requires requires(const Other& value) { value_type(value); } &&
							(!std::same_as<std::remove_cvref_t<Other>, Reference>)
					friend bool operator>(Reference left, const Other& right) {
						return value_type(right) < static_cast<value_type>(left);
					}

					/**
					 * @brief Order a convertible value after a proxy value.
					 * @tparam Other Other operand type.
					 * @param left Value convertible to value_type.
					 * @param right Proxy operand.
					 * @return Whether left follows right.
					 */
					template<class Other>
						requires requires(const Other& value) { value_type(value); } &&
							(!std::same_as<std::remove_cvref_t<Other>, Reference>)
					friend bool operator>(const Other& left, Reference right) {
						return static_cast<value_type>(right) < value_type(left);
					}

					/**
					 * @brief Compare a proxy value with less-or-equal ordering.
					 * @tparam Other Other operand type.
					 * @param left Proxy operand.
					 * @param right Value convertible to value_type.
					 * @return Whether left does not follow right.
					 */
					template<class Other>
						requires requires(const Other& value) { value_type(value); } &&
							(!std::same_as<std::remove_cvref_t<Other>, Reference>)
					friend bool operator<=(Reference left, const Other& right) {
						return !(value_type(right) < static_cast<value_type>(left));
					}

					/**
					 * @brief Compare a convertible value with less-or-equal ordering.
					 * @tparam Other Other operand type.
					 * @param left Value convertible to value_type.
					 * @param right Proxy operand.
					 * @return Whether left does not follow right.
					 */
					template<class Other>
						requires requires(const Other& value) { value_type(value); } &&
							(!std::same_as<std::remove_cvref_t<Other>, Reference>)
					friend bool operator<=(const Other& left, Reference right) {
						return !(static_cast<value_type>(right) < value_type(left));
					}

					/**
					 * @brief Compare a proxy value with greater-or-equal ordering.
					 * @tparam Other Other operand type.
					 * @param left Proxy operand.
					 * @param right Value convertible to value_type.
					 * @return Whether left does not precede right.
					 */
					template<class Other>
						requires requires(const Other& value) { value_type(value); } &&
							(!std::same_as<std::remove_cvref_t<Other>, Reference>)
					friend bool operator>=(Reference left, const Other& right) {
						return !(static_cast<value_type>(left) < value_type(right));
					}

					/**
					 * @brief Compare a convertible value with greater-or-equal ordering.
					 * @tparam Other Other operand type.
					 * @param left Value convertible to value_type.
					 * @param right Proxy operand.
					 * @return Whether left does not precede right.
					 */
					template<class Other>
						requires requires(const Other& value) { value_type(value); } &&
							(!std::same_as<std::remove_cvref_t<Other>, Reference>)
					friend bool operator>=(const Other& left, Reference right) {
						return !(value_type(left) < static_cast<value_type>(right));
					}

					/**
					 * @brief Compare two proxy values in descending order.
					 * @param left First proxy.
					 * @param right Second proxy.
					 * @return Whether left follows right.
					 */
					friend bool operator>(Reference left, Reference right) {
						return static_cast<value_type>(right) < static_cast<value_type>(left);
					}

					/**
					 * @brief Compare two proxy values with less-or-equal ordering.
					 * @param left First proxy.
					 * @param right Second proxy.
					 * @return Whether left does not follow right.
					 */
					friend bool operator<=(Reference left, Reference right) {
						return !(right < left);
					}

					/**
					 * @brief Compare two proxy values with greater-or-equal ordering.
					 * @param left First proxy.
					 * @param right Second proxy.
					 * @return Whether left does not precede right.
					 */
					friend bool operator>=(Reference left, Reference right) {
						return !(left < right);
					}

					/**
					 * @brief Compare two proxy values for equality.
					 * @param left First proxy.
					 * @param right Second proxy.
					 * @return Whether the values are equal.
					 */
					friend bool operator==(Reference left, Reference right) {
						return static_cast<value_type>(left) == static_cast<value_type>(right);
					}

					/**
					 * @brief Order two proxy values.
					 * @param left First proxy.
					 * @param right Second proxy.
					 * @return Whether left precedes right.
					 */
					friend bool operator<(Reference left, Reference right) {
						return static_cast<value_type>(left) < static_cast<value_type>(right);
					}

					/**
					 * @brief Swap the values denoted by two proxies.
					 * @param left First proxy.
					 * @param right Second proxy.
					 */
					friend void swap(Reference left, Reference right) {
						value_type leftValue = left;
						value_type rightValue = right;
						left = rightValue;
						right = leftValue;
					}

				private:
					friend class Iterable;

					/**
					 * @brief Bind a proxy to an owner and element index.
					 * @param owner Sequence owner.
					 * @param index Element index.
					 */
					Reference(Iterable& owner, difference_type index):
						m_owner(&owner), m_index(static_cast<size_type>(index)), m_value(owner.ReadValue(m_index)) {}

					/**
					 * @brief Write a value and refresh the local snapshot.
					 * @param value Replacement value.
					 */
					void Write(const value_type& value) const {
						if (m_owner->WriteAt(m_index, value) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe iterable write failed");
						m_value = value;
					}

					Iterable* m_owner; ///< Sequence receiving writes.
					size_type m_index; ///< Element index.
					mutable value_type m_value; ///< Caller-module value snapshot.
				};

				/**
				 * @class BasicIterator
				 * @brief Random-access iterator that stores only an owner pointer and index.
			 * @tparam IsConst Whether writes are disabled.
			 */
				template<bool IsConst>
				class BasicIterator final {
						using Owner = std::conditional_t<IsConst, const Iterable, Iterable>;

					public:
						using iterator_category = std::random_access_iterator_tag; ///< Iterator category.
						using iterator_concept = std::random_access_iterator_tag; ///< C++20 iterator concept.
						using value_type = typename Iterable::value_type; ///< Element type.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using reference = std::conditional_t<IsConst, value_type, Reference>; ///< Dereference result.
						using pointer = void; ///< Owner-module pointers are not exposed.

						/**
						 * @brief Construct a singular iterator.
					 */
						BasicIterator() noexcept: m_owner(nullptr), m_index(0) {}

						/**
						 * @brief Convert a mutable iterator to a const iterator.
						 * @param other Mutable iterator.
						 */
						template<bool OtherConst>
						requires IsConst && (!OtherConst)
						BasicIterator(const BasicIterator<OtherConst>& other) noexcept:
							m_owner(other.m_owner), m_index(other.m_index) {}

						/**
						 * @brief Dereference the iterator.
						 * @return Element copy or mutable proxy.
						 */
						reference operator*() const {
							if constexpr (IsConst)
								return m_owner->ReadValue(static_cast<size_type>(m_index));
							else
								return Reference(*m_owner, m_index);
						}

						/**
						 * @brief Access an element relative to this iterator.
						 * @param offset Relative element offset.
						 * @return Element copy or mutable proxy.
						 */
						reference operator[](difference_type offset) const {
							return *(*this + offset);
						}

						/**
						 * @brief Advance to the next element.
						 * @return This iterator.
						 */
						BasicIterator& operator++() noexcept { ++m_index; return *this; }

						/**
						 * @brief Advance to the next element.
						 * @return Previous iterator value.
						 */
						BasicIterator operator++(int) noexcept { auto copy = *this; ++*this; return copy; }

						/**
						 * @brief Move to the previous element.
						 * @return This iterator.
						 */
						BasicIterator& operator--() noexcept { --m_index; return *this; }

						/**
						 * @brief Move to the previous element.
						 * @return Previous iterator value.
						 */
						BasicIterator operator--(int) noexcept { auto copy = *this; --*this; return copy; }

						/**
						 * @brief Advance by an offset.
						 * @param offset Number of positions.
						 * @return This iterator.
						 */
						BasicIterator& operator+=(difference_type offset) noexcept { m_index += offset; return *this; }

						/**
						 * @brief Retreat by an offset.
						 * @param offset Number of positions.
						 * @return This iterator.
						 */
						BasicIterator& operator-=(difference_type offset) noexcept { m_index -= offset; return *this; }

						/**
						 * @brief Return an iterator advanced by an offset.
						 * @param offset Number of positions.
						 * @return Advanced iterator.
						 */
						BasicIterator operator+(difference_type offset) const noexcept { auto copy = *this; return copy += offset; }

						/**
						 * @brief Return an iterator retreated by an offset.
						 * @param offset Number of positions.
						 * @return Retreated iterator.
						 */
						BasicIterator operator-(difference_type offset) const noexcept { auto copy = *this; return copy -= offset; }

						/**
						 * @brief Return the distance to another iterator.
						 * @param other Iterator to compare.
						 * @return Signed element distance.
						 */
						difference_type operator-(const BasicIterator& other) const noexcept { return m_index - other.m_index; }

						/**
						 * @brief Compare iterator positions.
						 * @param other Iterator to compare.
						 * @return Whether owner and index match.
						 */
						bool operator==(const BasicIterator& other) const noexcept { return m_owner == other.m_owner && m_index == other.m_index; }

						/**
						 * @brief Order iterator positions.
						 * @param other Iterator to compare.
						 * @return Index ordering.
						 */
						std::strong_ordering operator<=>(const BasicIterator& other) const noexcept { return m_index <=> other.m_index; }

						/**
						 * @brief Return an iterator advanced by an offset.
						 * @param offset Number of positions.
						 * @param iterator Starting iterator.
						 * @return Advanced iterator.
						 */
						friend BasicIterator operator+(difference_type offset, BasicIterator iterator) noexcept { return iterator += offset; }

					private:
						friend class Iterable;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind an iterator to an owner and index.
						 * @param owner Sequence owner.
						 * @param index Element index.
						 */
						BasicIterator(Owner& owner, difference_type index) noexcept:
							m_owner(&owner), m_index(index) {}

						Owner* m_owner; ///< Sequence being traversed.
						difference_type m_index; ///< Current element index.
				};

				using iterator = BasicIterator<false>; ///< Mutable random-access iterator.
				using const_iterator = BasicIterator<true>; ///< Read-only random-access iterator.
				using reference = Reference; ///< Mutable callback-backed reference.
				using const_reference = value_type; ///< Read-only value copy.

				/**
				 * @brief Construct an empty sequence in the calling module.
				 */
				Iterable(): m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_capacity(&Store::Capacity) {}

				/**
				 * @brief Construct count default-inserted elements.
				 * @param count Number of elements.
				 */
				explicit Iterable(size_type count)
					requires std::default_initializable<value_type> && requires { Container(count); }:
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(Container(count)), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_capacity(&Store::Capacity) {}

				/**
				 * @brief Construct count copies of value.
				 * @param count Number of elements.
				 * @param value Value to copy.
				 */
				Iterable(size_type count, const value_type& value)
					requires requires { Container(count, value); }:
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(Container(count, value)), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_capacity(&Store::Capacity) {}

				/**
				 * @brief Construct from caller-owned initializer-list values.
				 * @param values Source values.
				 */
				Iterable(std::initializer_list<value_type> values)
					requires requires { Container(values); }:
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(Container(values)), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_capacity(&Store::Capacity) {}

				/**
				 * @brief Copy a container into creator-owned storage.
				 * @param values Source container.
				 */
				explicit Iterable(const Container& values):
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(values), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_capacity(&Store::Capacity) {}

				/**
				 * @brief Move elements from an STL rvalue without adopting its allocation.
				 * @param values Source container; it is left valid and empty.
				 */
				explicit Iterable(Container&& values):
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::CreateMove(values), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_capacity(&Store::Capacity) {}

				/**
				 * @brief Deep-copy another iterable in its creator module.
				 * @param other Source iterable.
				 */
				Iterable(const Iterable& other) = default;

				/**
				 * @brief Transfer owner callbacks; source becomes empty.
				 * @param other Source iterable.
				 */
				Iterable(Iterable&& other) noexcept = default;

				/**
				 * @brief Release creator-owned sequence storage.
				 */
				~Iterable() noexcept = default;

				/**
				 * @brief Deep-copy another iterable.
				 * @param other Source iterable.
				 * @return This iterable.
				 */
				Iterable& operator=(const Iterable& other) = default;

				/**
				 * @brief Transfer another iterable's owner callbacks.
				 * @param other Source iterable.
				 * @return This iterable.
				 */
				Iterable& operator=(Iterable&& other) noexcept = default;

				/**
				 * @brief Copy-assign from caller-owned sequence storage.
				 * @param values Source container; it remains unchanged.
				 * @return This iterable.
				 */
				Iterable& operator=(const Container& values) {
					Iterable replacement(values);
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Move elements from caller-owned sequence storage.
				 * @param values Source container, empty after a successful transfer.
				 * @return This iterable.
				 */
				Iterable& operator=(Container&& values) {
					Iterable replacement(std::move(values));
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Assign from an initializer list.
				 * @param values Replacement elements.
				 * @return This iterable.
				 */
				Iterable& operator=(std::initializer_list<value_type> values)
					requires requires { Container(values); } {
					return *this = Container(values);
				}

				/**
				 * @brief Return the number of elements.
				 * @return Element count.
				 */
				size_type size() const noexcept { return static_cast<size_type>(m_count(m_owner.Get())); }

				/**
				 * @brief Test whether the sequence is empty.
				 * @return Whether no elements are stored.
				 */
				bool empty() const noexcept { return size() == 0; }

				/**
				 * @brief Return the underlying capacity when supported by Container.
				 * @return Allocated element slots.
				 */
				size_type capacity() const noexcept
					requires requires(const Container& container) { container.capacity(); } {
					return static_cast<size_type>(m_capacity(m_owner.Get()));
				}

				/**
				 * @brief Return the first mutable iterator.
				 * @return Iterator to the first element.
				 */
				iterator begin() noexcept { return iterator(*this, 0); }

				/**
				 * @brief Return the end mutable iterator.
				 * @return End iterator.
				 */
				iterator end() noexcept { return iterator(*this, static_cast<difference_type>(size())); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Iterator to the first element.
				 */
				const_iterator begin() const noexcept { return const_iterator(*this, 0); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return End iterator.
				 */
				const_iterator end() const noexcept { return const_iterator(*this, static_cast<difference_type>(size())); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Iterator to the first element.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Return a mutable proxy at an unchecked position.
				 * @param index Element index.
				 * @return Mutable reference proxy.
				 */
				reference operator[](size_type index) { return Reference(*this, static_cast<difference_type>(index)); }

				/**
				 * @brief Return a value copy at an unchecked position.
				 * @param index Element index.
				 * @return Element copy.
				 */
				value_type operator[](size_type index) const { return ReadValue(index); }

				/**
				 * @brief Return a mutable proxy at a checked position.
				 * @param index Element index.
				 * @return Mutable reference proxy.
				 * @throws StormByte::Exception The index is out of range.
				 */
				reference at(size_type index) {
					if (index >= size())
						Detail::ThrowSafeConversionFailure("Safe iterable index is out of range");
					return Reference(*this, static_cast<difference_type>(index));
				}

				/**
				 * @brief Return a value copy at a checked position.
				 * @param index Element index.
				 * @return Element copy.
				 * @throws StormByte::Exception The index is out of range.
				 */
				value_type at(size_type index) const { return ReadValue(index); }

				/**
				 * @brief Return a mutable proxy to the first element.
				 * @return First element proxy.
				 */
				reference front() { return at(0); }

				/**
				 * @brief Return a copy of the first element.
				 * @return First element copy.
				 */
				value_type front() const { return at(0); }

				/**
				 * @brief Return a mutable proxy to the last element.
				 * @return Last element proxy.
				 */
				reference back() { return at(size() - 1); }

				/**
				 * @brief Return a copy of the last element.
				 * @return Last element copy.
				 */
				value_type back() const { return at(size() - 1); }

				/**
				 * @brief Append an element copy.
				 * @param value Value to append.
				 * @throws StormByte::Exception Allocation or copying failed.
				 */
				void push_back(const value_type& value) {
					EnsureOwner();
					if (m_dispatch(m_owner.Get(), Action::PushBack, StormByte::Size(0), &value, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable append failed");
				}

				/**
				 * @brief Append an rvalue element through creator-owned storage.
				 * @param value Value to move from; its state becomes moved-from.
				 */
				void push_back(value_type&& value) {
					value_type snapshot(std::move(value));
					push_back(static_cast<const value_type&>(snapshot));
				}

				/**
				 * @brief Construct and append an element.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to value_type.
				 * @return Mutable callback-backed proxy to the inserted element.
				 */
				template<class... Args>
				reference emplace_back(Args&&... args) {
					value_type value(std::forward<Args>(args)...);
					push_back(std::move(value));
					return back();
				}

				/**
				 * @brief Insert a value before position.
				 * @param position Insertion position.
				 * @param value Value to insert.
				 * @return Iterator to the inserted value.
				 */
				iterator insert(const_iterator position, const value_type& value)
					requires requires(Container& container, const value_type& item) { container.insert(container.begin(), item); } {
					const difference_type index = position - cbegin();
					if (m_dispatch(m_owner.Get(), Action::Insert, StormByte::Size(index), &value, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable insert failed");
					return begin() + index;
				}

				/**
				 * @brief Construct a value before position.
				 * @tparam Args Constructor argument types.
				 * @param position Insertion position.
				 * @param args Arguments forwarded to value_type.
				 * @return Iterator to the inserted value.
				 */
				template<class... Args>
				iterator emplace(const_iterator position, Args&&... args)
					requires requires(Container& container, const value_type& item) { container.insert(container.begin(), item); } {
					value_type value(std::forward<Args>(args)...);
					return insert(position, value);
				}

				/**
				 * @brief Insert count copies before position.
				 * @param position Insertion position.
				 * @param count Number of copies.
				 * @param value Value to copy.
				 * @return Iterator to the first inserted value.
				 */
				iterator insert(const_iterator position, size_type count, const value_type& value)
					requires requires(Container& container, const value_type& item) { container.insert(container.begin(), item); } {
					const difference_type index = position - cbegin();
					Container replacement = static_cast<Container>(*this);
					for (size_type inserted = 0; inserted < count; ++inserted)
						replacement.insert(replacement.begin() + index + static_cast<difference_type>(inserted), value);
					*this = std::move(replacement);
					return begin() + index;
				}

				/**
				 * @brief Insert initializer-list values before position.
				 * @param position Insertion position.
				 * @param values Values to insert.
				 * @return Iterator to the first inserted value.
				 */
				iterator insert(const_iterator position, std::initializer_list<value_type> values)
					requires requires(Container& container, const value_type& item) { container.insert(container.begin(), item); } {
					const difference_type index = position - cbegin();
					const difference_type first = index;
					Container replacement = static_cast<Container>(*this);
					difference_type inserted = 0;
					for (const auto& value: values) {
						replacement.insert(replacement.begin() + index + inserted, value);
						++inserted;
					}
					*this = std::move(replacement);
					return begin() + first;
				}

				/**
				 * @brief Remove the final element.
				 * @throws StormByte::Exception The sequence is empty or removal failed.
				 */
				void pop_back()
					requires requires(Container& container) { container.pop_back(); } {
					if (empty() || m_dispatch(m_owner.Get(), Action::PopBack, StormByte::Size(0), nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable pop_back failed");
				}

				/**
				 * @brief Resize, default-inserting or removing elements.
				 * @param count New element count.
			 */
				void resize(size_type count)
					requires requires(Container& container) { container.resize(std::size_t{}); } {
					if (m_dispatch(m_owner.Get(), Action::Resize, StormByte::Size(count), nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable resize failed");
				}

				/**
				 * @brief Resize, filling newly added elements with value.
				 * @param count New element count.
				 * @param value Fill value.
				 */
				void resize(size_type count, const value_type& value)
					requires requires(Container& container, const value_type& item) { container.resize(std::size_t{}, item); } {
					if (m_dispatch(m_owner.Get(), Action::Resize, StormByte::Size(count), &value, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable resize failed");
				}

				/**
				 * @brief Replace all elements with count copies of value.
				 * @param count Number of copies.
				 * @param value Fill value.
				 */
				void assign(size_type count, const value_type& value)
					requires requires { Container(count, value); } {
					*this = Container(count, value);
				}

				/**
				 * @brief Replace all elements from an initializer list.
				 * @param values Replacement values.
				 */
				void assign(std::initializer_list<value_type> values)
					requires requires { Container(values); } {
					*this = Container(values);
				}

				/**
				 * @brief Request capacity for at least count elements.
				 * @param count Requested capacity.
				 */
				void reserve(size_type count)
					requires requires(Container& container) { container.reserve(std::size_t{}); } {
					if (m_dispatch(m_owner.Get(), Action::Reserve, StormByte::Size(count), nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable reserve failed");
				}

				/**
				 * @brief Request release of unused capacity.
				 */
				void shrink_to_fit()
					requires requires(Container& container) { container.shrink_to_fit(); } {
					if (m_dispatch(m_owner.Get(), Action::ShrinkToFit, StormByte::Size(0), nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable shrink_to_fit failed");
				}

				/**
				 * @brief Remove all elements.
				 */
				void clear() {
					EnsureOwner();
					if (m_dispatch(m_owner.Get(), Action::Clear, StormByte::Size(0), nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable clear failed");
				}

				/**
				 * @brief Erase one element and return its former successor.
				 * @param position Iterator to the element.
				 * @return Iterator to the next element.
				 */
				iterator erase(const_iterator position) {
					const difference_type index = position - cbegin();
					EraseAt(static_cast<size_type>(index));
					return begin() + index;
				}

				/**
				 * @brief Erase a range and return its former end position.
				 * @param first Start iterator.
				 * @param last End iterator.
				 * @return Iterator to the first remaining element after the erased range.
				 */
				iterator erase(const_iterator first, const_iterator last) {
					const difference_type start = first - cbegin();
					const difference_type finish = last - cbegin();
					for (difference_type i = finish; i > start; --i)
						EraseAt(static_cast<size_type>(start));
					return begin() + start;
				}

				/**
				 * @brief Copy elements into a container allocated by the caller.
				 * @return Container copy.
				 */
				STORMBYTE_FORCE_INLINE explicit operator Container() const {
					Container output;
					for (size_type index = 0; index < size(); ++index)
						output.push_back(ReadValue(index));
					return output;
				}

				/**
				 * @brief Compare Safe sequences by value.
				 * @param left First sequence.
				 * @param right Second sequence.
				 * @return Whether both contain equal elements in order.
				 */
				friend bool operator==(const Iterable& left, const Iterable& right)
					requires Type::EqualityComparable<Container> {
					return static_cast<Container>(left) == static_cast<Container>(right);
				}

				/**
				 * @brief Order Safe sequences lexicographically.
				 * @param left First sequence.
				 * @param right Second sequence.
				 * @return Comparison category of the underlying sequence.
				 */
				friend auto operator<=>(const Iterable& left, const Iterable& right)
					requires Type::ThreeWayComparable<Container> {
					return static_cast<Container>(left) <=> static_cast<Container>(right);
				}

			private:
				enum class Action {
					Get,
					PushBack,
					Insert,
					PopBack,
					Resize,
					Reserve,
					ShrinkToFit,
					Set,
					Erase,
					Clear
				};

				using Dispatch = Status (*)(void*, Action, const StormByte::Size&, const value_type*, value_type*) noexcept;
				using Count = StormByte::Size (*)(const void*) noexcept;

				/**
				 * @brief Creator-module storage and callbacks.
			 */
				struct Store {
					Container values; ///< Container storage owned by this module.

					/**
					 * @brief Allocate an empty store.
				 * @return Opaque store pointer.
				 */
					static void* Create() { return std::make_unique<Store>().release(); }

					/**
					 * @brief Copy a caller container into this module.
					 * @param source Source container.
					 * @return Opaque store pointer.
				 */
					static void* Create(const Container& source) {
						auto store = std::make_unique<Store>();
						for (const auto& value: source)
							store->values.push_back(value);
						return store.release();
					}

					/**
					 * @brief Move elements from an rvalue container without adopting its allocation.
					 * @param source Source container, cleared after successful transfer.
					 * @return Opaque store pointer.
					 */
					static void* CreateMove(Container& source) {
						auto store = std::make_unique<Store>();
						for (auto& value: source)
							store->values.push_back(std::move(value));
						source.clear();
						return store.release();
					}

					/**
					 * @brief Clone store, containing exceptions.
					 * @param state Source store.
					 * @return Clone or null on failure.
					 */
					static void* Clone(const void* state) noexcept {
						try {
							return std::make_unique<Store>(*static_cast<const Store*>(state)).release();
						} catch (...) {
							return nullptr;
						}
					}

					/**
					 * @brief Destroy store in its creator module.
					 * @param state Store to destroy.
					 */
					static void Destroy(void* state) noexcept {
						std::unique_ptr<Store> owner(static_cast<Store*>(state));
					}

					/**
					 * @brief Return element count.
					 * @param state Store, or null after move.
					 * @return Element count.
					 */
					static StormByte::Size Count(const void* state) noexcept {
						return state ? StormByte::Size(static_cast<const Store*>(state)->values.size()) : StormByte::Size(0);
					}

					/**
					 * @brief Return capacity when Container publishes it, otherwise its size.
					 * @param state Opaque store, or null after move.
					 * @return Creator-side element capacity.
					 */
					static StormByte::Size Capacity(const void* state) noexcept {
						if (!state)
							return StormByte::Size(0);
						const auto& values = static_cast<const Store*>(state)->values;
						if constexpr (requires { values.capacity(); })
							return StormByte::Size(values.capacity());
						else
							return StormByte::Size(values.size());
					}

					/**
					 * @brief Read or transactionally modify the stored container.
					 * @param state Opaque store.
					 * @param action Requested operation.
					 * @param index Element index.
					 * @param input Optional input value.
					 * @param output Optional output location.
					 * @return Operation status.
					 */
					static Status Apply(void* state, Action action, const StormByte::Size& index, const value_type* input, value_type* output) noexcept {
						if (!state)
							return action == Action::Get ? Status::Missing : Status::Failure;
						auto& values = static_cast<Store*>(state)->values;
						if ((action == Action::Get || action == Action::Set || action == Action::Erase || action == Action::PopBack) && index >= StormByte::Size(values.size()))
							return Status::Missing;
						if (action == Action::Insert && index > StormByte::Size(values.size()))
							return Status::Missing;
						try {
							const auto offset = static_cast<difference_type>(index);
							if (action == Action::Get) {
								value_type copy(values[static_cast<size_type>(offset)]);
								*output = std::move(copy);
							}
							else {
								Container copy(values);
								if (action == Action::PushBack)
									copy.push_back(*input);
								else if (action == Action::Insert) {
									if constexpr (requires { copy.insert(copy.begin() + offset, *input); })
										copy.insert(copy.begin() + offset, *input);
									else
										return Status::Failure;
								}
								else if (action == Action::PopBack) {
									if constexpr (requires { copy.pop_back(); })
										copy.pop_back();
									else
										return Status::Failure;
								}
								else if (action == Action::Resize) {
									if (input) {
										if constexpr (requires { copy.resize(static_cast<size_type>(index), *input); })
											copy.resize(static_cast<size_type>(index), *input);
										else
											return Status::Failure;
									} else {
										if constexpr (requires { copy.resize(static_cast<size_type>(index)); })
											copy.resize(static_cast<size_type>(index));
										else
											return Status::Failure;
									}
								}
								else if (action == Action::Reserve) {
									if constexpr (requires { copy.reserve(static_cast<size_type>(index)); })
										copy.reserve(static_cast<size_type>(index));
									else
										return Status::Failure;
								}
								else if (action == Action::ShrinkToFit) {
									if constexpr (requires { copy.shrink_to_fit(); })
										copy.shrink_to_fit();
									else
										return Status::Failure;
								}
								else if (action == Action::Set)
									copy[static_cast<size_type>(offset)] = *input;
								else if (action == Action::Erase)
									copy.erase(copy.begin() + offset);
								else
									copy.clear();
								values.swap(copy);
							}
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					}
				};
				/**
				 * @brief Read a value and throw if its index is invalid.
				 * @param index Element index.
				 * @return Value copy.
				 */
				value_type ReadValue(size_type index) const {
					value_type output{};
					if (ReadAt(index, output) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable index is out of range");
					return output;
				}

				/**
				 * @brief Read a value through the owner callback.
				 * @param index Element index.
				 * @param output Copy destination.
				 * @return Operation status.
				 */
				Status ReadAt(size_type index, value_type& output) const noexcept {
					return m_dispatch(m_owner.Get(), Action::Get, StormByte::Size(index), nullptr, &output);
				}

				/**
				 * @brief Write a value through the owner callback.
				 * @param index Element index.
				 * @param value Source value.
				 * @return Operation status.
				 */
				Status WriteAt(size_type index, const value_type& value) noexcept {
					EnsureOwner();
					return m_dispatch(m_owner.Get(), Action::Set, StormByte::Size(index), &value, nullptr);
				}

				/**
				 * @brief Erase a value through the owner callback.
				 * @param index Element index.
				 */
				void EraseAt(size_type index) {
					if (m_dispatch(m_owner.Get(), Action::Erase, StormByte::Size(index), nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe iterable erase failed");
				}

				/**
				 * @brief Recreate empty creator-owned storage after move.
				 */
				void EnsureOwner() {
					if (!m_owner.Get())
						m_owner = Detail::Owner(m_create(), m_clone, m_destroy);
				}

				using Create = void* (*)(); ///< Creator-module empty-store callback type.
				using Clone = Detail::Owner::Clone; ///< Creator-module clone callback type.
				using Destroy = Detail::Owner::Destroy; ///< Creator-module destroy callback type.
				Create m_create; ///< Creator-module empty-store callback.
				Clone m_clone; ///< Creator-module deep-clone callback.
				Destroy m_destroy; ///< Creator-module release callback.
				Detail::Owner m_owner; ///< Opaque owner callbacks.
				Dispatch m_dispatch; ///< Creator-module operation callback.
				Count m_count; ///< Creator-module count callback.
				Count m_capacity; ///< Creator-module capacity callback.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes safe opaque iterable containers.
		 * @tparam Container Admitted random-access container type.
		 */
		template<class Container>
		requires Safe::Detail::SafeRandomAccessContainer<Container> && Type::IsSafe<typename Container::value_type>::value
		struct IsSafe<Safe::Iterable<Container>>: std::true_type {};

		/**
		 * @brief Propagates conditional safety from a collection's element type.
		 * @tparam Container Admitted random-access container type.
		 */
		template<class Container>
		requires Safe::Detail::SafeRandomAccessContainer<Container> && MaybeSafe<typename Container::value_type>
		struct IsMaybeSafe<Safe::Iterable<Container>>: std::true_type {};

		/**
		 * @brief Admits safe opaque iterable containers as collection values.
		 * @tparam Container Admitted random-access container type.
		 */
		template<class Container>
		requires Safe::Detail::SafeRandomAccessContainer<Container>
		struct IsSafeValue<Safe::Iterable<Container>>: std::true_type {};
	}
}

#include <StormByte/safe/map.hxx>
