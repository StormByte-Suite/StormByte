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
		/**
		 * @class SharedMutex
		 * @brief Non-recursive shared mutex. The gate lives on Base's heap.
		 *
		 * Not an alias of `std::shared_mutex`. Not copyable or movable. One exclusive owner, or several shared owners. `unlock` and `unlock_shared` without that ownership are undefined.
		 */
		class STORMBYTE_PUBLIC SharedMutex {
			public:
				/**
				 * @brief Unlocked shared mutex.
				 * @throws AllocationError The gate cannot be allocated.
				 */
				SharedMutex();

				SharedMutex(const SharedMutex&) = delete;

				SharedMutex(SharedMutex&&) = delete;

				/**
				 * @brief Destroys the gate on Base's heap.
				 * @note Every owner unlocks before destruction.
				 */
				~SharedMutex() noexcept;

				SharedMutex& operator=(const SharedMutex&) = delete;

				SharedMutex& operator=(SharedMutex&&) = delete;

				/**
				 * @brief Acquire exclusive ownership.
				 * @throws OperationError A foreign failure is translated.
				 */
				void lock();

				/**
				 * @brief Try to acquire exclusive ownership.
				 * @return Whether this thread now owns it exclusively.
				 */
				bool try_lock() noexcept;

				/**
				 * @brief Release exclusive ownership.
				 * @pre The calling thread owns it exclusively.
				 */
				void unlock() noexcept;

				/**
				 * @brief Acquire shared ownership.
				 * @throws OperationError A foreign failure is translated.
				 */
				void lock_shared();

				/**
				 * @brief Try to acquire shared ownership.
				 * @return Whether this thread now owns it shared.
				 */
				bool try_lock_shared() noexcept;

				/**
				 * @brief Release shared ownership.
				 * @pre The calling thread owns it shared.
				 */
				void unlock_shared() noexcept;

			private:
				void* m_gate;	///< Base-owned gate. Never null while *this is alive.
		};
	}
}
