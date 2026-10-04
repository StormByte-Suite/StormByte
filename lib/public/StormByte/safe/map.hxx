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

#include <map>
#include <memory>
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
		requires requires(const K& left, const K& right) { left < right; }
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
						 * @brief Dereference an ordered entry.
						 * @return Safe pair copy or mutable entry proxy.
						 */
						reference operator*() const {
							K key{};
						V value{};
						if (m_owner->ReadEntry(static_cast<size_type>(m_index), key, value) != Status::Success)
							Detail::ThrowSafeConversionFailure("Safe map iterator read failed");
						if constexpr (IsConst)
							return value_type(std::move(key), std::move(value));
						else
							return EntryReference(key, value, MappedReference(*m_owner, key));
						}

						/**
						 * @brief Advance to the next ordered entry.
						 * @return This iterator.
						 */
						BasicIterator& operator++() noexcept { ++m_index; return *this; }

						/**
						 * @brief Advance to the next ordered entry.
						 * @return Previous iterator value.
						 */
						BasicIterator operator++(int) noexcept { auto copy = *this; ++*this; return copy; }

						/**
						 * @brief Move to the previous ordered entry.
						 * @return This iterator.
						 */
						BasicIterator& operator--() noexcept { --m_index; return *this; }

						/**
						 * @brief Move to the previous ordered entry.
						 * @return Previous iterator value.
						 */
						BasicIterator operator--(int) noexcept { auto copy = *this; --*this; return copy; }

						/**
						 * @brief Compare iterator position and owner.
						 * @tparam OtherConst Other iterator constness.
						 * @param other Iterator to compare.
						 * @return Whether both designate the same position.
						 */
						template<bool OtherConst>
						bool operator==(const BasicIterator<OtherConst>& other) const noexcept {
							return m_owner == other.m_owner && m_index == other.m_index;
						}

					private:
						friend class Iterable;
						template<bool> friend class BasicIterator;

						/**
						 * @brief Bind an iterator to a map owner and entry index.
						 * @param owner Map owner.
						 * @param index Ordered entry index.
						 */
						BasicIterator(Owner& owner, difference_type index) noexcept:
							m_owner(&owner), m_index(index) {}

						Owner* m_owner; ///< Map being traversed.
						difference_type m_index; ///< Ordered entry index.
				};

				using iterator = BasicIterator<false>; ///< Mutable bidirectional iterator.
				using const_iterator = BasicIterator<true>; ///< Read-only bidirectional iterator.

				/**
				 * @brief Construct an empty map.
				 */
				Iterable():
					m_owner(Store::Create(), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit) {}

				/**
				 * @brief Copy a caller-owned map into this module.
				 * @param values Source map.
				 */
				explicit Iterable(const Container& values):
					m_owner(Store::Create(values), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit) {}

				/**
				 * @brief Move entries from an STL rvalue into locally allocated nodes.
				 * @param values Source map; it is left valid and empty.
				 */
				explicit Iterable(Container&& values):
					m_owner(Store::CreateMove(values), &Store::Clone, &Store::Destroy),
					m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit) {}

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
				iterator begin() noexcept { return iterator(*this, 0); }

				/**
				 * @brief Return the end mutable iterator.
				 * @return End iterator.
				 */
				iterator end() noexcept { return iterator(*this, static_cast<difference_type>(size())); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Iterator to the first key-ordered entry.
				 */
				const_iterator begin() const noexcept { return const_iterator(*this, 0); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return End iterator.
				 */
				const_iterator end() const noexcept { return const_iterator(*this, static_cast<difference_type>(size())); }

				/**
				 * @brief Return the first read-only iterator.
				 * @return Iterator to the first entry.
				 */
				const_iterator cbegin() const noexcept { return begin(); }

				/**
				 * @brief Return the end read-only iterator.
				 * @return End iterator.
				 */
				const_iterator cend() const noexcept { return end(); }

				/**
				 * @brief Find an entry by key.
				 * @param key Key to search.
				 * @return Iterator to the entry or end.
				 */
				iterator find(const K& key) {
					for (auto current = begin(); current != end(); ++current) {
						auto entry = *current;
						if (!(entry.first < key) && !(key < entry.first))
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
						if (!(entry.first < key) && !(key < entry.first))
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
					K key{};
					V value{};
					if (ReadEntry(static_cast<size_type>(position.m_index), key, value) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map erase iterator is invalid");
					EraseKey(key);
					return iterator(*this, position.m_index);
				}

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
					Container output;
					const auto insert = [](void* context, const K& key, const V& value) noexcept {
						try {
							static_cast<Container*>(context)->insert_or_assign(key, value);
							return Status::Success;
						} catch (...) {
							return Status::Failure;
						}
					};
					if (m_visit(m_owner.Get(), &output, insert) != Status::Success)
						Detail::ThrowSafeConversionFailure("Safe map export failed");
					return output;
				}

			private:
				enum class Action { Get, GetAt, Set, Erase, Clear };
				using Dispatch = Status (*)(void*, Action, const StormByte::Size&, const K*, const V*, K*, V*) noexcept;
				using Count = StormByte::Size (*)(const void*) noexcept;
				using InsertEntry = Status (*)(void*, const K&, const V&) noexcept;
				using VisitEntries = Status (*)(const void*, void*, InsertEntry) noexcept;

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
					if (!m_owner.Get())
						m_owner = Detail::Owner(Store::Create(), &Store::Clone, &Store::Destroy);
				}

				Detail::Owner m_owner; ///< Opaque owner callbacks.
				Dispatch m_dispatch; ///< Creator-module dispatch callback.
				Count m_count; ///< Creator-module count callback.
				VisitEntries m_visit; ///< Creator-module traversal callback.
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
		requires requires(const K& left, const K& right) { left < right; }
		struct IsSafe<Safe::Iterable<std::map<K, V, Compare, Allocator>>>: std::true_type {};

		/**
		 * @brief Admits safe iterable maps as collection values.
		 * @tparam K Safe key type.
		 * @tparam V Safe mapped type.
		 * @tparam Compare Comparator type.
		 * @tparam Allocator Allocator type.
		 */
		template<SafeValue K, SafeValue V, class Compare, class Allocator>
		requires requires(const K& left, const K& right) { left < right; }
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
		requires requires(const K& left, const K& right) { left < right; }
		using Map = Iterable<std::map<K, V>>;
	}
}
