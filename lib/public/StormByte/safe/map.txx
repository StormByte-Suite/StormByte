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

#include <StormByte/exception.hxx>

#include <map>
#include <memory>
#include <utility>

namespace StormByte {
	namespace Safe {
		/**
		 * @struct Map<K, V>::Store
		 * @brief Creator-module ordered storage and operation callbacks.
		 * @tparam K Safe ordered key type.
		 * @tparam V Safe mapped value type.
		 */
		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		struct Map<K, V>::Store {
			std::map<K, V> values;	///< Nodes confined to the creator module.

			/**
			 * @brief Allocate empty state locally.
			 * @return Opaque state.
			 */
			static void* Create() {
				return std::make_unique<Store>().release();
			}

			/**
			 * @brief Copy an STL map into local storage.
			 * @param source Caller-owned map.
			 * @return New state.
			 */
			static void* Create(const std::map<K, V>& source) {
				auto store = std::make_unique<Store>();
				store->values = source;
				return store.release();
			}

			/**
			 * @brief Deep copy in this module, containing exceptions.
			 * @param state Source.
			 * @return New state, or null on failure.
			 */
			static void* Clone(const void* state) noexcept {
				try {
					return std::make_unique<Store>(*static_cast<const Store*>(state)).release();
				} catch (...) {
					return nullptr;
				}
			}

			/**
			 * @brief Destroy locally.
			 * @param state Owned state.
			 */
			static void Destroy(void* state) noexcept {
				std::unique_ptr<Store> owner(static_cast<Store*>(state));
			}

			/**
			 * @brief Count locally.
			 * @param state State, or null after move.
			 * @return Entry count.
			 */
			static StormByte::Size Count(const void* state) noexcept {
				return state ? StormByte::Size(static_cast<const Store*>(state)->values.size()) : StormByte::Size(0);
			}

			/**
			 * @brief Visit ordered entries through a caller-module insertion callback.
			 * @param state State, or null after move.
			 * @param context Caller-owned output context.
			 * @param insert Caller-module insertion callback.
			 * @return Operation status.
			 */
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

			/**
			 * @brief Read or transactionally modify local nodes, containing exceptions.
			 * @param state Owned state.
			 * @param action Operation.
			 * @param index Ordered position.
			 * @param key Lookup key.
			 * @param input Source value.
			 * @param outputKey Destination key.
			 * @param outputValue Destination value.
			 * @return Operation status.
			 */
			static Status Apply(void* state, Action action, const StormByte::Size& index, const K* key, const V* input, K* outputKey, V* outputValue) noexcept {
				if (!state)
					return action == Action::Set || action == Action::Clear ? Status::Failure : Status::Missing;
				auto& values = static_cast<Store*>(state)->values;
				try {
					if (action == Action::GetAt) {
						if (index >= StormByte::Size(values.size()))
							return Status::Missing;
						auto entry = values.begin();
						for (std::size_t position = 0; position < static_cast<std::size_t>(index); ++position)
							++entry;
						K keyCopy(entry->first);
						V valueCopy(entry->second);
						*outputKey = std::move(keyCopy);
						*outputValue = std::move(valueCopy);
					}
					else if (action == Action::Get) {
						auto entry = values.find(*key);
						if (entry == values.end())
							return Status::Missing;
						V copy(entry->second);
						*outputValue = std::move(copy);
					}
					else if (action == Action::Clear)
						values.clear();
					else {
						if (action == Action::Erase && values.find(*key) == values.end())
							return Status::Missing;
						auto copy = values;
						if (action == Action::Set)
							copy.insert_or_assign(*key, *input);
						else
							copy.erase(*key);
						values.swap(copy);
					}
					return Status::Success;
				} catch (...) {
					return Status::Failure;
				}
			}
		};

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		STORMBYTE_FORCE_INLINE Map<K, V>::Map():
			m_owner(Store::Create(), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit) {}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Map<K, V>::Map(const std::map<K, V>& values):
			m_owner(Store::Create(values), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count), m_visit(&Store::Visit) {}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Map<K, V>::Map(std::map<K, V>&& values): Map(static_cast<const std::map<K, V>&>(values)) {}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		StormByte::Size Map<K, V>::Size() const noexcept {
			return m_count(m_owner.Get());
		}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Status Map<K, V>::Get(const K& key, V& output) const noexcept {
			return m_dispatch(m_owner.Get(), Action::Get, StormByte::Size(0), &key, nullptr, nullptr, &output);
		}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Status Map<K, V>::GetAt(const StormByte::Size& index, K& key, V& value) const noexcept {
			return m_dispatch(m_owner.Get(), Action::GetAt, index, nullptr, nullptr, &key, &value);
		}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Status Map<K, V>::Set(const K& key, const V& value) noexcept {
			return m_dispatch(m_owner.Get(), Action::Set, StormByte::Size(0), &key, &value, nullptr, nullptr);
		}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Status Map<K, V>::Erase(const K& key) noexcept {
			return m_dispatch(m_owner.Get(), Action::Erase, StormByte::Size(0), &key, nullptr, nullptr, nullptr);
		}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		Status Map<K, V>::Clear() noexcept {
			return m_dispatch(m_owner.Get(), Action::Clear, StormByte::Size(0), nullptr, nullptr, nullptr, nullptr);
		}

		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		STORMBYTE_FORCE_INLINE Map<K, V>::operator std::map<K, V>() const {
			std::map<K, V> output;
			const auto insert = [](void* context, const K& key, const V& value) noexcept {
				try {
					static_cast<std::map<K, V>*>(context)->insert_or_assign(key, value);
					return Status::Success;
				} catch (...) {
					return Status::Failure;
				}
			};
			if (m_visit(m_owner.Get(), &output, insert) != Status::Success)
				throw StormByte::Exception("Safe map export failed");
			return output;
		}
	}
}