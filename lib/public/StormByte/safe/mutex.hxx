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

#include <StormByte/visibility.h>

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
		class ConditionVariable;

		/**
		 * @class Mutex
		 * @brief Non-recursive mutex. The gate lives on Base's heap.
		 *
		 * Not an alias of `std::mutex`. Not copyable or movable. `unlock` without ownership is undefined.
		 */
		class STORMBYTE_PUBLIC Mutex {
			public:
				/**
				 * @brief Unlocked mutex.
				 * @throws AllocationError The gate cannot be allocated.
				 */
				Mutex();

				Mutex(const Mutex&) = delete;

				Mutex(Mutex&&) = delete;

				/**
				 * @brief Destroys the gate on Base's heap.
				 * @note The owner unlocks before destruction.
				 */
				~Mutex() noexcept;

				Mutex& operator=(const Mutex&) = delete;

				Mutex& operator=(Mutex&&) = delete;

				/**
				 * @brief Acquire the mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				void lock();

				/**
				 * @brief Try to acquire the mutex.
				 * @return Whether this thread now owns it.
				 */
				bool try_lock() noexcept;

				/**
				 * @brief Release the mutex.
				 * @pre The calling thread owns it.
				 */
				void unlock() noexcept;

			private:
				friend class ConditionVariable;

				void* m_gate;	///< Base-owned gate. Never null while *this is alive.
		};
	}
}
