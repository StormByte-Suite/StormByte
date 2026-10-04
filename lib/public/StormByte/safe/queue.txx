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

namespace StormByte {
	namespace Safe {
		/**
		 * @struct Queue<T>::Store
		 * @brief Creator-module queue storage and operation callbacks.
		 * @tparam T Safe queued value type.
		 */
		template<Type::SafeValue T>
		struct Queue<T>::Store {
			std::queue<T> values; ///< Queue nodes confined to the creator module.

			/**
			 * @brief Allocate an empty queue locally.
			 * @return Opaque state.
			 */
			static void* Create() {
				return std::make_unique<Store>().release();
			}

			/**
			 * @brief Copy an STL queue into local storage.
			 * @param source Caller-owned queue.
			 * @return New state.
			 */
			static void* Create(const std::queue<T>& source) {
				auto store = std::make_unique<Store>();
				store->values = source;
				return store.release();
			}

			/**
			 * @brief Move elements into queue storage owned by this module.
			 * @param source Source queue, consumed after successful transfer.
			 * @return New owner state.
			 */
			static void* CreateMove(std::queue<T>& source) {
				auto store = std::make_unique<Store>();
				while (!source.empty()) {
					store->values.push(std::move(source.front()));
					source.pop();
				}
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
			 * @brief Read or modify local storage, containing all exceptions.
			 * @param state Owned state.
			 * @param action Operation.
			 * @param input Optional input value.
			 * @param output Optional output value.
			 * @return Operation status.
			 */
			static Status Apply(void* state, Action action, const T* input, T* output) noexcept {
				if (!state)
					return action == Action::Front || action == Action::Pop ? Status::Missing : Status::Failure;
				auto& values = static_cast<Store*>(state)->values;
				if ((action == Action::Front || action == Action::Pop) && values.empty())
					return Status::Missing;
				try {
					if (action == Action::Front) {
						T copy(values.front());
						*output = std::move(copy);
					}
					else if (action == Action::Push) {
						auto copy = values;
						copy.push(*input);
						values.swap(copy);
					}
					else if (action == Action::Pop)
						values.pop();
					return Status::Success;
				} catch (...) {
					return Status::Failure;
				}
			}
		};

		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Queue<T>::Queue():
			m_owner(Store::Create(), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count) {}

		template<Type::SafeValue T>
		Queue<T>::Queue(const std::queue<T>& values):
			m_owner(Store::Create(values), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count) {}

		template<Type::SafeValue T>
		Queue<T>::Queue(std::queue<T>&& values):
			m_owner(Store::CreateMove(values), &Store::Clone, &Store::Destroy),
			m_dispatch(&Store::Apply), m_count(&Store::Count) {}

		template<Type::SafeValue T>
		std::size_t Queue<T>::size() const noexcept {
			return static_cast<std::size_t>(m_count(m_owner.Get()));
		}

		template<Type::SafeValue T>
		void Queue<T>::EnsureOwner() {
			if (!m_owner.Get())
				m_owner = Detail::Owner(Store::Create(), &Store::Clone, &Store::Destroy);
		}

		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Queue<T>::operator std::queue<T>() const {
			std::queue<T> output;
			Queue<T> copy(*this);
			T value{};
			while (!copy.empty()) {
				value = copy.front();
				output.push(std::move(value));
				copy.pop();
			}
			return output;
		}
	}
}