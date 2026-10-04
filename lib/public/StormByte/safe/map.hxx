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

#include <StormByte/safe/iterable.hxx>
#include <StormByte/safe/pair.hxx>

#include <concepts>
#include <map>
#include <memory>
#include <optional>
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
		 * @class Iterable<std::map<K, V>>
		 * @brief Opaque safe range over an ordered map.
		 * @tparam K Safe key type with strict weak ordering.
		 * @tparam V Safe mapped-value type.
		 * @tparam Compare Map comparison type.
		 * @tparam Allocator Map allocator type.
		 *
		 * Nodes and allocator state stay in the creator module. Dereference
		 * returns a caller-module entry proxy: first is immutable and second
		 * writes through a callback. No node iterator or reference escapes.
		 */
		template<Type::SafeValue K, Type::SafeValue V, class Compare, class Allocator>
		requires std::strict_weak_order<Compare, const K&, const K&>
		class STORMBYTE_PUBLIC_TYPE Iterable<std::map<K, V, Compare, Allocator>> final {
				using Container = std::map<K, V, Compare, Allocator>;

			public:
				using key_type = K; ///< Key type.
				using mapped_type = V; ///< Mapped type.
				using value_type = Pair<K, V>; ///< Safe entry value type.
				using size_type = std::size_t; ///< Entry count type.
				using difference_type = std::ptrdiff_t; ///< Iterator distance type.

				/**
				 * @class MappedReference
				 * @brief Callback-backed mutable proxy to a mapped value.
				 */
				class MappedReference final {
					public:
						/**
						 * @brief Read the mapped value.
						 * @return Mapped value copy.
						 */
					operator V() const {
						V output{};
						if (m_owner->ReadKey(m_key, output) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe map read failed");
						return output;
					}

					/**
					 * @brief Replace the mapped value.
					 * @param value Replacement value.
					 * @return This proxy.
					 */
					MappedReference& operator=(const V& value) {
						if (m_owner->WriteKey(m_key, value) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe map write failed");
						return *this;
					}

						/**
						 * @brief Assign through a const mapped-value proxy.
						 * @param value Replacement value.
						 * @return This proxy.
						 */
						const MappedReference& operator=(const V& value) const {
							if (m_owner->WriteKey(m_key, value) != Status::Success)
								Detail::ThrowSafeConversionFailure("Safe map write failed");
							return *this;
						}

					/**
					 * @brief Compare the mapped value with a value.
					 * @param left Mapped-value proxy.
					 * @param right Value to compare.
					 * @return Whether the values are equal.
					 */
					friend bool operator==(MappedReference left, const V& right) {
						return static_cast<V>(left) == right;
					}

					/**
					 * @brief Compare the mapped value with a convertible value.
					 * @tparam Other Other operand type.
					 * @param left Mapped-value proxy.
					 * @param right Value convertible to V.
					 * @return Whether the values are equal.
					 */
					template<class Other>
					requires requires(const Other& value) { V(value); } &&
						(!std::same_as<std::remove_cvref_t<Other>, MappedReference>)
					friend bool operator==(MappedReference left, const Other& right) {
						return static_cast<V>(left) == V(right);
					}

					/**
					 * @brief Compare a convertible value with the mapped value.
					 * @tparam Other Other operand type.
					 * @param left Value convertible to V.
					 * @param right Mapped-value proxy.
					 * @return Whether the values are equal.
					 */
					template<class Other>
					requires requires(const Other& value) { V(value); } &&
						(!std::same_as<std::remove_cvref_t<Other>, MappedReference>)
					friend bool operator==(const Other& left, MappedReference right) {
						return V(left) == static_cast<V>(right);
					}

				private:
					friend class Iterable;

					/**
					 * @brief Bind a mapped-value proxy to an owner and copied key.
					 * @param owner Map owner.
					 * @param key Copied lookup key.
					 */
					MappedReference(Iterable& owner, const K& key): m_owner(&owner), m_key(key) {}

					Iterable* m_owner; ///< Owner receiving writes.
					K m_key; ///< Key copied into the caller module.
				};

				using EntryReference = PairReference<K, V, MappedReference>; ///< Callback-backed map entry proxy.

				/**
				 * @class BasicIterator
				 * @brief Ordered bidirectional iterator over callback-backed map nodes.
			 * @tparam IsConst Whether mapped-value writes are disabled.
				 */
				template<bool IsConst>
				class BasicIterator final {
						using Owner = std::conditional_t<IsConst, const Iterable, Iterable>;

					public:
						using iterator_category = std::bidirectional_iterator_tag; ///< Iterator category.
						using iterator_concept = std::bidirectional_iterator_tag; ///< C++20 iterator concept.
						using value_type = typename Iterable::value_type; ///< Entry value type.
						using difference_type = std::ptrdiff_t; ///< Iterator distance type.
						using reference = std::conditional_t<IsConst, value_type, EntryReference>; ///< Dereference result.
						using pointer = void; ///< Map-node pointers are not exposed.

						/**
						 * @class ArrowProxy
						 * @brief Owns an entry snapshot or write-through proxy for one arrow expression.
						 * @note Its pointer is valid only for the full expression. Mutable mapped
						 *       assignments still dispatch through the creator callback.
						 */
						class ArrowProxy final {
							public:
								/**
								 * @brief Store a caller-owned entry snapshot/proxy.
								 * @param entry Entry obtained from iterator dereference.
								 */
								explicit ArrowProxy(reference entry): m_entry(std::move(entry)) {}

								/**
								 * @brief Access the owned entry snapshot/proxy.
								 * @return Pointer valid until this arrow proxy expires.
								 */
								reference* operator->() noexcept { return &m_entry; }

								/**
								 * @brief Access a const owned entry snapshot/proxy.
								 * @return Pointer valid until this arrow proxy expires.
								 */
								const reference* operator->() const noexcept { return &m_entry; }

							private:
								reference m_entry; ///< Caller-owned entry object; mapped proxy retains callback.
						};

						/**
						 * @brief Construct a singular iterator.
					 */
						BasicIterator() requires std::default_initializable<K>:
							m_owner(nullptr), m_index(0), m_key(), m_atEnd(true) {}

						/**
						 * @brief Convert a mutable iterator to a const iterator.
						 * @param other Mutable iterator.
						 */
						template<bool OtherConst>
						requires IsConst && (!OtherConst)
						BasicIterator(const BasicIterator<OtherConst>& other):
							m_owner(other.m_owner), m_index(other.m_index), m_key(other.m_key), m_atEnd(other.m_atEnd) {}

						/**
						 * @brief Dereference an ordered entry.
						 * @return Safe pair copy or mutable entry proxy.
						 */
						reference operator*() const {
							V value{};
							if (!m_owner || m_atEnd || m_owner->ReadKey(m_key, value) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe map iterator read failed");
						if constexpr (IsConst)
								return value_type(m_key, std::move(value));
						else
								return EntryReference(m_key, value, MappedReference(*m_owner, m_key));
						}

						/**
						 * @brief Access an entry through a short-lived caller-owned proxy.
						 * @return Arrow proxy valid for the full expression only.
						 */
						ArrowProxy operator->() const { return ArrowProxy(operator*()); }

						/**
						 * @brief Advance to the next ordered entry.
						 * @return This iterator.
						 */
						BasicIterator& operator++() {
							if (!m_owner || m_atEnd)
								Detail::ThrowSafeConversionFailure("Safe map iterator increment is invalid");
							const auto index = m_owner->FindIndex(m_key);
							if (index == m_owner->size())
								Detail::ThrowSafeConversionFailure("Safe map iterator key was erased");
							const auto next = index + 1;
							if (next >= m_owner->size()) {
								m_index = static_cast<difference_type>(m_owner->size());
								m_atEnd = true;
								return *this;
							}
						K nextKey{};
						V nextValue{};
						if (m_owner->ReadEntry(next, nextKey, nextValue) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe map iterator increment failed");
						m_index = static_cast<difference_type>(next);
						m_key = std::move(nextKey);
						return *this;
					}

						/**
						 * @brief Advance to the next ordered entry.
						 * @return Previous iterator value.
						 */
						BasicIterator operator++(int) { auto copy = *this; ++*this; return copy; }

						/**
						 * @brief Move to the previous ordered entry.
						 * @return This iterator.
						 */
						BasicIterator& operator--() {
							if (!m_owner || m_owner->empty())
								Detail::ThrowSafeConversionFailure("Safe map iterator decrement is invalid");
						std::size_t index = m_atEnd ? m_owner->size() - 1 : m_owner->FindIndex(m_key);
						if (index == 0 || index >= m_owner->size())
							Detail::ThrowSafeConversionFailure("Safe map iterator decrement is invalid");
						--index;
						V value{};
						if (m_owner->ReadEntry(index, m_key, value) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe map iterator decrement failed");
						m_index = static_cast<difference_type>(index);
						m_atEnd = false;
						return *this;
					}

						/**
						 * @brief Move to the previous ordered entry.
						 * @return Previous iterator value.
						 */
						BasicIterator operator--(int) { auto copy = *this; --*this; return copy; }

						/**
						 * @brief Compare iterator position and owner.
						 * @tparam OtherConst Other iterator constness.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same position.
						 */
						template<bool OtherConst>
						bool operator==(const BasicIterator<OtherConst>& other) const {
							if (m_owner != other.m_owner)
								return false;
							if (m_atEnd || other.m_atEnd)
								return m_atEnd && other.m_atEnd;
							return !m_owner->KeyLess(m_key, other.m_key) && !m_owner->KeyLess(other.m_key, m_key);
						}

					private:
						friend class Iterable;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind an iterator to a map owner and entry index.
						 * @param owner Map owner.
						 * @param index Ordered entry index.
						 */
						BasicIterator(Owner& owner, difference_type index)
							requires std::default_initializable<K>:
							m_owner(&owner), m_index(index), m_key(), m_atEnd(index >= static_cast<difference_type>(owner.size())) {
							if (!m_atEnd) {
								V value{};
								if (owner.ReadEntry(static_cast<size_type>(index), m_key, value) != Status::Success)
									Detail::ThrowSafeConversionFailure("Safe map iterator construction failed");
							}
						}

						/**
						 * @brief Bind an iterator to a stable key identity.
						 * @param owner Map being traversed.
						 * @param key Key identity copied into the iterator.
						 * @param atEnd Whether to construct the end sentinel.
						 */
						BasicIterator(Owner& owner, const K& key, bool atEnd = false):
							m_owner(&owner), m_index(0), m_key(key), m_atEnd(atEnd) {
							if (!m_atEnd) {
								m_index = static_cast<difference_type>(owner.FindIndex(m_key));
								if (static_cast<std::size_t>(m_index) >= owner.size())
									Detail::ThrowSafeConversionFailure("Safe map iterator key is absent");
							}
							else {
								m_index = static_cast<difference_type>(owner.size());
							}
						}

						Owner* m_owner; ///< Map being traversed.
						difference_type m_index; ///< Ordered entry index.
						K m_key; ///< Caller-owned key identity, stable across unrelated mutations.
						bool m_atEnd; ///< Whether this is the map's end sentinel.
				};

				using iterator = BasicIterator<false>; ///< Mutable bidirectional iterator.
				using const_iterator = BasicIterator<true>; ///< Read-only bidirectional iterator.

				/**
				 * @brief Construct an empty map.
				 */
				Iterable():
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit), m_compare(&Store::CompareKeys) {}

				/**
				 * @brief Copy a caller-owned map into this module.
				 * @param values Source map.
				 */
				explicit Iterable(const Container& values):
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::Create(values), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit), m_compare(&Store::CompareKeys) {}

				/**
				 * @brief Move entries from an STL rvalue into locally allocated nodes.
				 * @param values Source map; it is left valid and empty.
				 */
				explicit Iterable(Container&& values):
					m_create(&Store::Create), m_clone(&Store::Clone), m_destroy(&Store::Destroy),
					m_owner(Store::CreateMove(values), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit), m_compare(&Store::CompareKeys) {}

				/**
				 * @brief Deep-copy an iterable.
				 * @param other Source iterable.
				 */
				Iterable(const Iterable& other) = default;

				/**
				 * @brief Transfer creator callbacks; source becomes empty.
				 * @param other Source iterable.
				 */
				Iterable(Iterable&& other) noexcept = default;

				/**
				 * @brief Release creator-owned map nodes.
				 */
				~Iterable() noexcept = default;

				/**
				 * @brief Deep-copy another iterable.
				 * @param other Source iterable.
				 * @return This iterable.
				 */
				Iterable& operator=(const Iterable& other) = default;

				/**
				 * @brief Transfer another iterable's creator callbacks.
				 * @param other Source iterable.
				 * @return This iterable.
				 */
				Iterable& operator=(Iterable&& other) noexcept = default;

				/**
				 * @brief Copy-assign from caller-owned std::map storage.
				 * @param values Source map; it remains unchanged.
				 * @return This map.
				 */
				Iterable& operator=(const Container& values) {
					Iterable replacement(values);
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Move elements from caller-owned std::map storage.
				 * @param values Source map, empty after successful transfer.
				 * @return This map.
				 */
				Iterable& operator=(Container&& values) {
					Iterable replacement(std::move(values));
					*this = std::move(replacement);
					return *this;
				}

				/**
				 * @brief Return the entry count.
				 * @return Number of entries.
				 */
				size_type size() const noexcept { return static_cast<size_type>(m_count(m_owner.Get())); }

				/**
				 * @brief Test whether the map is empty.
				 * @return Whether there are no entries.
				 */
				bool empty() const noexcept { return size() == 0; }

				/**
				 * @brief Return the first mutable iterator.
				 * @return Iterator to the first key-ordered entry.
				 */
				iterator begin() { return iterator(*this, 0); }

				/**
				 * @brief Return the end mutable iterator.
				 * @return End iterator.
				 */
				iterator end() { return iterator(*this, static_cast<difference_type>(size())); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Iterator to the first key-ordered entry.
				 */
				const_iterator begin() const { return const_iterator(*this, 0); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return End iterator.
				 */
				const_iterator end() const { return const_iterator(*this, static_cast<difference_type>(size())); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Iterator to the first entry.
				 */
				const_iterator cbegin() const { return begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const { return end(); }

				/**
				 * @brief Find an entry by key.
				 * @param key Key to search.
				 * @return Iterator to the entry or end.
				 */
				iterator find(const K& key) {
					for (auto current = begin(); current != end(); ++current) {
						auto entry = *current;
						if (!KeyLess(entry.first, key) && !KeyLess(key, entry.first))
							return current;
					}
					return end();
				}

				/**
				 * @brief Find an entry by key in a const map.
				 * @param key Key to search.
				 * @return Read-only iterator to the entry or end.
				 */
				const_iterator find(const K& key) const {
					for (auto current = begin(); current != end(); ++current) {
						const auto entry = *current;
						if (!KeyLess(entry.first, key) && !KeyLess(key, entry.first))
							return current;
					}
					return end();
				}

				/**
				 * @brief Test whether a key exists.
				 * @param key Key to search.
				 * @return Whether the key is present.
				 */
				bool contains(const K& key) const { return find(key) != end(); }

				/**
				 * @brief Return one when a key exists, otherwise zero.
				 * @param key Key to search.
				 * @return Zero or one.
				 */
				size_type count(const K& key) const { return contains(key) ? 1 : 0; }

				/**
				 * @brief Find the first entry not ordered before key.
				 * @param key Search key.
				 * @return Lower-bound iterator.
				 */
				iterator lower_bound(const K& key) {
					for (auto current = begin(); current != end(); ++current) {
						const auto entry = *current;
						if (!KeyLess(entry.first, key))
							return current;
					}
					return end();
				}

				/**
				 * @brief Find the first entry ordered after key.
				 * @param key Search key.
				 * @return Upper-bound iterator.
				 */
				iterator upper_bound(const K& key) {
					for (auto current = begin(); current != end(); ++current) {
						const auto entry = *current;
						if (KeyLess(key, entry.first))
							return current;
					}
					return end();
				}

				/**
				 * @brief Find the first entry not ordered before key in a const map.
				 * @param key Search key.
				 * @return Const lower-bound iterator.
				 */
				const_iterator lower_bound(const K& key) const {
					for (auto current = begin(); current != end(); ++current) {
						const auto entry = *current;
						if (!KeyLess(entry.first, key))
							return current;
					}
					return end();
				}

				/**
				 * @brief Find the first entry ordered after key in a const map.
				 * @param key Search key.
				 * @return Const upper-bound iterator.
				 */
				const_iterator upper_bound(const K& key) const {
					for (auto current = begin(); current != end(); ++current) {
						const auto entry = *current;
						if (KeyLess(key, entry.first))
							return current;
					}
					return end();
				}

				/**
				 * @brief Return the lower and upper bound of key.
				 * @param key Search key.
				 * @return Pair of equal-range iterators.
				 */
				auto equal_range(const K& key) { return std::pair(lower_bound(key), upper_bound(key)); }

				/**
				 * @brief Return the lower and upper bound of key in a const map.
				 * @param key Search key.
				 * @return Pair of const equal-range iterators.
				 */
				auto equal_range(const K& key) const { return std::pair(lower_bound(key), upper_bound(key)); }

				/**
				 * @brief Access a mapped value by key.
				 * @param key Key to find.
				 * @return Mutable mapped-value proxy.
				 * @throws StormByte::Exception The key is absent.
				 */
				MappedReference at(const K& key) {
					if (!contains(key))
						Detail::ThrowSafeConversionFailure("Safe map key is absent");
					return MappedReference(*this, key);
				}

				/**
				 * @brief Access a mapped value by key in a const map.
				 * @param key Key to find.
				 * @return Mapped value copy.
				 * @throws StormByte::Exception The key is absent.
				 */
				V at(const K& key) const {
					V value{};
					if (ReadKey(key, value) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map key is absent");
					return value;
				}

				/**
				 * @brief Insert a default mapped value if the key is absent.
				 * @param key Key to insert or find.
				 * @return Mutable mapped-value proxy.
				 */
				MappedReference operator[](const K& key) {
					if (!contains(key) && WriteKey(key, V{}) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map subscript insert failed");
					return MappedReference(*this, key);
				}

				/**
				 * @brief Insert or assign an entry.
				 * @param key Entry key.
				 * @param value Mapped value.
				 * @return Iterator to entry and whether it was inserted.
				 */
				std::pair<iterator, bool> insert_or_assign(const K& key, const V& value) {
					const bool inserted = !contains(key);
					if (WriteKey(key, value) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map insert_or_assign failed");
					return {find(key), inserted};
				}

				/**
				 * @brief Insert an entry without replacing an existing mapped value.
				 * @param entry Entry to insert.
				 * @return Iterator to the matching entry and whether insertion occurred.
				 */
				std::pair<iterator, bool> insert(const std::pair<K, V>& entry) {
					return try_emplace(entry.first, entry.second);
				}

				/**
				 * @brief Insert a moved entry without replacing an existing value.
				 * @param entry Entry to insert.
				 * @return Iterator to the matching entry and whether insertion occurred.
				 */
				std::pair<iterator, bool> insert(std::pair<K, V>&& entry) {
					return try_emplace(entry.first, std::move(entry.second));
				}

				/**
				 * @brief Construct and insert a mapped value only if key is absent.
				 * @tparam Args Mapped-value constructor argument types.
				 * @param key Key to insert or find.
				 * @param args Arguments forwarded to V when inserting.
				 * @return Iterator to the matching entry and whether insertion occurred.
				 */
				template<class... Args>
				std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) {
					if (auto existing = find(key); existing != end())
						return {existing, false};
					V value(std::forward<Args>(args)...);
					if (WriteKey(key, value) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map try_emplace failed");
					return {find(key), true};
				}

				/**
				 * @brief Construct an entry and insert it if its key is absent.
				 * @tparam Args Mapped-value constructor argument types.
				 * @param key Entry key.
				 * @param args Arguments forwarded to V.
				 * @return Iterator to the matching entry and whether insertion occurred.
				 */
				template<class... Args>
				std::pair<iterator, bool> emplace(const K& key, Args&&... args) {
					return try_emplace(key, std::forward<Args>(args)...);
				}

				/**
				 * @brief Erase an entry by key.
				 * @param key Key to erase.
				 * @return One if an entry was removed, otherwise zero.
				 */
				size_type erase(const K& key) {
					const Status result = EraseKey(key);
					if (result == Status::Failure)
						Detail::ThrowSafeConversionFailure("Safe map erase failed");
					return result == Status::Success ? 1 : 0;
				}

				/**
				 * @brief Erase an entry by iterator.
				 * @param position Iterator to erase.
				 * @return Iterator to the next entry.
				 */
				iterator erase(const_iterator position) {
					if (position.m_atEnd)
						Detail::ThrowSafeConversionFailure("Safe map erase iterator is invalid");
					const auto index = FindIndex(position.m_key);
					if (index == size())
						Detail::ThrowSafeConversionFailure("Safe map erase iterator is invalid");
					iterator next(*this, static_cast<difference_type>(index + 1));
					if (EraseKey(position.m_key) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map erase iterator failed");
					return next;
				}

				/**
				 * @brief Erase a range of entries.
				 * @param first First entry to erase.
				 * @param last Iterator past the final erased entry.
				 * @return Iterator following the erased range.
				 */
				iterator erase(const_iterator first, const_iterator last) {
					iterator current(*this, first.m_key, first.m_atEnd);
					while (current != last)
						current = erase(current);
					return current;
				}

				/**
				 * @brief Exchange map storage and creator callbacks.
				 * @param other Map to exchange with.
				 */
				void swap(Iterable& other) noexcept {
					if (this == &other)
						return;
					Iterable temporary(std::move(*this));
					*this = std::move(other);
					other = std::move(temporary);
				}

				/**
				 * @brief Exchange two Safe maps.
				 * @param left First map.
				 * @param right Second map.
				 */
				friend void swap(Iterable& left, Iterable& right) noexcept { left.swap(right); }

				/**
				 * @brief Remove all entries.
				 */
				void clear() {
					EnsureOwner();
					if (m_dispatch(m_owner.Get(), Action::Clear, StormByte::Size(0), nullptr, nullptr, nullptr, nullptr) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map clear failed");
				}

				/**
				 * @brief Copy entries into caller-owned STL storage.
				 * @return A std::map allocated in the caller module.
				 */
				STORMBYTE_FORCE_INLINE explicit operator Container() const {
					std::optional<Container> output;
					const auto create = [](void* context, const Compare& compare) noexcept {
						try {
							static_cast<std::optional<Container>*>(context)->emplace(compare);
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					};
					if (m_prepareOutput(m_owner.Get(), &output, create) != Status::Success || !output)
						Detail::ThrowSafeConversionFailure("Safe map export failed");
					const auto insert = [](void* context, const K& key, const V& value) noexcept {
						try {
							static_cast<Container*>(context)->insert_or_assign(key, value);
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					};
					if (m_visit(m_owner.Get(), &*output, insert) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map export failed");
					return std::move(*output);
				}

			private:
				enum class Action { Get, GetAt, Set, Erase, Clear };
				using Dispatch = Status (*)(void*, Action, const StormByte::Size&, const K*, const V*, K*, V*) noexcept;
				using Count = StormByte::Size (*)(const void*) noexcept;
				using InsertEntry = Status (*)(void*, const K&, const V&) noexcept;
				using VisitEntries = Status (*)(const void*, void*, InsertEntry) noexcept;
				using CreateOutputContainer = Status (*)(void*, const Compare&) noexcept;
				using PrepareOutput = Status (*)(const void*, void*, CreateOutputContainer) noexcept;
				using CompareKeyOperation = Status (*)(const void*, const K&, const K&, bool*, bool*) noexcept;
				using FindKeyOperation = std::size_t (*)(const void*, const K&) noexcept;
				using CreateState = void* (*)(); ///< Creator-module empty-store callback type.
				using CloneState = Detail::Owner::Clone; ///< Creator-module clone callback type.
				using DestroyState = Detail::Owner::Destroy; ///< Creator-module destroy callback type.

				struct Store {
					Container values;

					static void* Create() { return std::make_unique<Store>().release(); }

					static void* Create(const Container& source) {
						auto store = std::make_unique<Store>();
						store->values = Container(source.key_comp());
						for (const auto& [key, value]: source)
							store->values.emplace(key, value);
						return store.release();
					}

					/**
					 * @brief Move mapped values into locally allocated nodes.
					 * @param source Source map, cleared after successful transfer.
					 * @return Opaque store pointer.
					 */
					static void* CreateMove(Container& source) {
						auto store = std::make_unique<Store>();
						store->values = Container(source.key_comp());
						for (auto& [key, value]: source)
							store->values.emplace(key, std::move(value));
						source.clear();
						return store.release();
					}

					static void* Clone(const void* state) noexcept {
						try {
							return std::make_unique<Store>(*static_cast<const Store*>(state)).release();
						} catch (...) {
							return nullptr;
						}
					}

					static void Destroy(void* state) noexcept {
						std::unique_ptr<Store> owner(static_cast<Store*>(state));
					}

					static StormByte::Size Count(const void* state) noexcept {
						return state ? StormByte::Size(static_cast<const Store*>(state)->values.size()) : StormByte::Size(0);
					}

					/**
					 * @brief Compare keys with this store's creator-owned comparator.
					 * @param state Opaque creator store.
					 * @param left First key.
					 * @param right Second key.
					 * @param leftBefore Output whether left precedes right.
					 * @param rightBefore Output whether right precedes left.
					 * @return Success or Failure if the comparator throws.
					 */
					static Status CompareKeys(const void* state, const K& left, const K& right,
						bool* leftBefore, bool* rightBefore) noexcept {
						if (!state)
							return Status::Failure;
						try {
							const Compare compare = static_cast<const Store*>(state)->values.key_comp();
							*leftBefore = compare(left, right);
							*rightBefore = compare(right, left);
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					}

					/**
					 * @brief Construct caller-owned map storage with the creator comparator.
					 * @param state Opaque creator store.
					 * @param context Caller-owned output context.
					 * @param create Caller-module map construction callback.
					 * @return Success or Failure.
					 */
					static Status PrepareOutput(const void* state, void* context, CreateOutputContainer create) noexcept {
						if (!state) {
							if constexpr (std::is_empty_v<Compare> && std::default_initializable<Compare>) {
								try {
									return create(context, Compare{});
								} catch (...) {
									return Status::Failure;
								}
							} else {
								return Status::Failure;
							}
						}
						try {
							const Compare compare = static_cast<const Store*>(state)->values.key_comp();
							return create(context, compare);
						} catch (...) {
							return Status::Failure;
						}
					}

					/**
					 * @brief Find a key's current ordered index using the creator comparator.
					 * @param state Opaque creator store.
					 * @param key Key to locate.
					 * @return Key index or container size when absent or comparison fails.
					 */
					static std::size_t FindIndex(const void* state, const K& key) noexcept {
						if (!state)
							return 0;
						try {
							const auto& values = static_cast<const Store*>(state)->values;
							const auto found = values.find(key);
							return found == values.end() ? values.size() : static_cast<std::size_t>(std::distance(values.begin(), found));
						} catch (...) {
							return static_cast<const Store*>(state)->values.size();
						}
					}

					static Status Visit(const void* state, void* context, InsertEntry insert) noexcept {
						if (!state)
							return Status::Success;
						try {
							for (const auto& [key, value]: static_cast<const Store*>(state)->values)
								if (insert(context, key, value) != Status::Success)
									return Status::Failure;
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					}

					static Status Apply(void* state, Action action, const StormByte::Size& index,
						const K* key, const V* input, K* outputKey, V* outputValue) noexcept {
						if (!state)
							return action == Action::Get || action == Action::GetAt || action == Action::Erase
								? Status::Missing : Status::Failure;
						auto& values = static_cast<Store*>(state)->values;
						try {
							if (action == Action::GetAt) {
								if (index >= StormByte::Size(values.size()))
									return Status::Missing;
								auto entry = values.begin();
								std::advance(entry, static_cast<std::size_t>(index));
								*outputKey = entry->first;
								*outputValue = entry->second;
								return Status::Success;
							}
							if (action == Action::Get) {
								auto entry = values.find(*key);
								if (entry == values.end())
									return Status::Missing;
								*outputValue = entry->second;
								return Status::Success;
							}
							if (action == Action::Clear) {
								values.clear();
								return Status::Success;
							}
							if (action == Action::Erase && values.find(*key) == values.end())
								return Status::Missing;
							Container copy(values);
							if (action == Action::Set)
								copy.insert_or_assign(*key, *input);
							else
								copy.erase(*key);
							values.swap(copy);
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					}
				};

				/**
				 * @brief Query key order through the creator-owned comparator callback.
				 * @param left First key.
				 * @param right Second key.
				 * @return Whether left precedes right.
				 * @throws StormByte::Exception The creator comparator failed.
				 */
				bool KeyLess(const K& left, const K& right) const {
					bool leftBefore = false;
					bool rightBefore = false;
					if (m_compare(m_owner.Get(), left, right, &leftBefore, &rightBefore) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map key comparison failed");
					return leftBefore;
				}

				/**
				 * @brief Find a key's current ordered index through the creator callback.
				 * @param key Key to locate.
				 * @return Current index or size when absent.
				 */
				std::size_t FindIndex(const K& key) const noexcept { return m_findKey(m_owner.Get(), key); }

				/**
				 * @brief Test whether a key is currently stored.
				 * @param key Key to locate.
				 * @return Whether the creator lookup found the key.
				 */
				bool HasKey(const K& key) const noexcept { return FindIndex(key) < size(); }

				Status ReadKey(const K& key, V& output) const noexcept {
					return m_dispatch(m_owner.Get(), Action::Get, StormByte::Size(0), &key, nullptr, nullptr, &output);
				}

				Status ReadEntry(size_type index, K& key, V& value) const noexcept {
					return m_dispatch(m_owner.Get(), Action::GetAt, StormByte::Size(index), nullptr, nullptr, &key, &value);
				}

				Status WriteKey(const K& key, const V& value) noexcept {
					try {
						EnsureOwner();
					} catch (...) {
						return Status::Failure;
					}
					return m_dispatch(m_owner.Get(), Action::Set, StormByte::Size(0), &key, &value, nullptr, nullptr);
				}

				Status EraseKey(const K& key) noexcept {
					return m_dispatch(m_owner.Get(), Action::Erase, StormByte::Size(0), &key, nullptr, nullptr, nullptr);
				}

				void EnsureOwner() {
					if (!m_owner.Get()) {
						if constexpr (std::is_empty_v<Compare>)
							m_owner = Detail::Owner(m_create(), m_clone, m_destroy);
						else
							Detail::ThrowSafeConversionFailure("Safe map with stateful comparator cannot be reused after move");
					}
				}

				CreateState m_create; ///< Creator-module empty-store callback.
				CloneState m_clone; ///< Creator-module deep-clone callback.
				DestroyState m_destroy; ///< Creator-module release callback.
				Detail::Owner m_owner; ///< Opaque owner callbacks.
				Dispatch m_dispatch; ///< Creator-module dispatch callback.
				Count m_count; ///< Creator-module count callback.
				VisitEntries m_visit; ///< Creator-module traversal callback.
				CompareKeyOperation m_compare; ///< Creator-module comparator callback.
				FindKeyOperation m_findKey{&Store::FindIndex}; ///< Creator-module key lookup callback.
				PrepareOutput m_prepareOutput{&Store::PrepareOutput}; ///< Caller map construction dispatcher.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes safe iterable map containers.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Comparator type.
		 * @tparam Allocator Allocator type.
		 */
		template<SafeValue K, SafeValue V, class Compare, class Allocator>
		requires IsSafe<K>::value && IsSafe<V>::value && std::strict_weak_order<Compare, const K&, const K&>
		struct IsSafe<Safe::Iterable<std::map<K, V, Compare, Allocator>>>: std::true_type {};

		/**
		 * @brief Propagates conditional key or mapped-value safety to Safe maps.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Comparator type.
		 * @tparam Allocator Allocator type.
		 */
		template<SafeValue K, SafeValue V, class Compare, class Allocator>
		requires (MaybeSafe<K> || MaybeSafe<V>) && std::strict_weak_order<Compare, const K&, const K&>
		struct IsMaybeSafe<Safe::Iterable<std::map<K, V, Compare, Allocator>>>: std::true_type {};

		/**
		 * @brief Admits safe iterable maps as collection values.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Comparator type.
		 * @tparam Allocator Allocator type.
		 */
		template<SafeValue K, SafeValue V, class Compare, class Allocator>
		requires std::strict_weak_order<Compare, const K&, const K&>
		struct IsSafeValue<Safe::Iterable<std::map<K, V, Compare, Allocator>>>: std::true_type {};
	}
}


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
		 * @brief Safe ordered map alias backed by the shared opaque iterable.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped-value type.
		 */
		template<Type::SafeValue K, Type::SafeValue V>
		requires std::strict_weak_order<std::less<K>, const K&, const K&>
		using Map = Iterable<std::map<K, V>>;
	}
}
