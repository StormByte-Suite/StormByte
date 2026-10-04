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

#include <memory>
#include <utility>
#include <vector>

namespace StormByte {
	namespace Safe {
		/**
		 * @struct Vector<T>::Store
		 * @brief Creator-module vector storage and operation callbacks.
		 * @tparam T Safe element type.
		 */
		template<Type::SafeValue T>
		struct Vector<T>::Store {
			std::vector<T> values;	///< Storage confined to the creator module.

			/**
			 * @brief Allocate empty state locally.
			 * @return Opaque state.
			 */
			static void* Create() {
				return std::make_unique<Store>().release();
			}

			/**
			 * @brief Copy an STL vector into local storage.
			 * @param source Caller-owned vector.
			 * @return New state.
			 */
			static void* Create(const std::vector<T>& source) {
				auto store = std::make_unique<Store>();
				store->values = source;
				return store.release();
			}

			/**
			 * @brief Deep copy locally.
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
			 * @brief Release locally.
			 * @param state Owned state.
			 */
			static void Destroy(void* state) noexcept {
				std::unique_ptr<Store> owner(static_cast<Store*>(state));
			}

			/**
			 * @brief Count locally.
			 * @param state State.
			 * @return Number of elements.
			 */
			static StormByte::Size Count(const void* state) noexcept {
				return state ? StormByte::Size(static_cast<const Store*>(state)->values.size()) : StormByte::Size(0);
			}

			/**
			 * @brief Read or transactionally modify local storage; contain all exceptions.
			 * @param state State.
			 * @param action Operation.
			 * @param index Position.
			 * @param input Optional source.
			 * @param output Optional destination.
			 * @return Operation status.
			 */
			static Status Apply(void* state, Action action, const StormByte::Size& index, const T* input, T* output) noexcept {
				if (!state)
					return action == Action::Get || action == Action::Erase || action == Action::Set ? Status::Missing : Status::Failure;
				auto& values = static_cast<Store*>(state)->values;
				if ((action == Action::Get || action == Action::Set || action == Action::Erase) && index >= StormByte::Size(values.size()))
					return Status::Missing;
				try {
					const auto position = static_cast<std::size_t>(index);
					if (action == Action::Get) {
						T copy(values[position]);
						*output = std::move(copy);
					}
					else if (action == Action::Clear)
						values.clear();
					else {
						auto copy = values;
						if (action == Action::PushBack)
							copy.push_back(*input);
						else if (action == Action::Set)
							copy[position] = *input;
						else
							copy.erase(copy.begin() + static_cast<typename std::vector<T>::difference_type>(position));
						values.swap(copy);
					}
					return Status::Success;
				} catch (...) {
					return Status::Failure;
				}
			}
		};

		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Vector<T>::Vector():
			m_owner(Store::Create(), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count) {}

		template<Type::SafeValue T>
		Vector<T>::Vector(const std::vector<T>& values):
			m_owner(Store::Create(values), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count) {}

		template<Type::SafeValue T>
		Vector<T>::Vector(std::vector<T>&& values): Vector(static_cast<const std::vector<T>&>(values)) {}

		template<Type::SafeValue T>
		StormByte::Size Vector<T>::Size() const noexcept {
			return m_count(m_owner.Get());
		}

		template<Type::SafeValue T>
		Status Vector<T>::Get(const StormByte::Size& index, T& output) const noexcept {
			return m_dispatch(m_owner.Get(), Action::Get, index, nullptr, &output);
		}

		template<Type::SafeValue T>
		Status Vector<T>::PushBack(const T& value) noexcept {
			return m_dispatch(m_owner.Get(), Action::PushBack, StormByte::Size(0), &value, nullptr);
		}

		template<Type::SafeValue T>
		Status Vector<T>::Set(const StormByte::Size& index, const T& value) noexcept {
			return m_dispatch(m_owner.Get(), Action::Set, index, &value, nullptr);
		}

		template<Type::SafeValue T>
		Status Vector<T>::Erase(const StormByte::Size& index) noexcept {
			return m_dispatch(m_owner.Get(), Action::Erase, index, nullptr, nullptr);
		}

		template<Type::SafeValue T>
		Status Vector<T>::Clear() noexcept {
			return m_dispatch(m_owner.Get(), Action::Clear, StormByte::Size(0), nullptr, nullptr);
		}

		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Vector<T>::operator std::vector<T>() const {
			std::vector<T> output;
			const auto count = static_cast<std::size_t>(Size());
			output.reserve(count);
			for (std::size_t index = 0; index < count; ++index) {
				output.emplace_back();
				if (Get(StormByte::Size(index), output.back()) != Status::Success) {
					output.pop_back();
					Detail::ThrowSafeConversionFailure("Safe vector export failed");
				}
			}
			return output;
		}
	}
}