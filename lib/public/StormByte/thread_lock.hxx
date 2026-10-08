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

#include <mutex>
#include <optional>
#include <thread>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class ThreadLock
	 * @brief Mutex with owner-thread reentry.
	 *
	 * The owning thread may call `Lock()` again without blocking.
	 * Another thread blocks in `Lock()` until the owner calls `Unlock()`.
	 * `Unlock()` from a thread that does not own the lock is a no-op.
	 */
	class STORMBYTE_PUBLIC ThreadLock final {
		public:
			/**
			 * @brief Unlocked lock, no owner.
			 */
			ThreadLock() noexcept = default;

			/**
			 * @brief Not copyable.
			 */
			ThreadLock(const ThreadLock&) = delete;

			/**
			 * @brief Not movable (`std::mutex` is not movable on Windows).
			 */
			ThreadLock(ThreadLock&&) = delete;

			/**
			 * @brief Releases internal mutexes.
			 * @note The owner should `Unlock()` before destruction if it still holds the lock.
			 */
			~ThreadLock() noexcept;

			/**
			 * @brief Not copy-assignable.
			 */
			ThreadLock& operator=(const ThreadLock&) = delete;

			/**
			 * @brief Not move-assignable.
			 */
			ThreadLock& operator=(ThreadLock&&) = delete;

			/**
			 * @brief Acquire the lock.
			 *
			 * Returns immediately if this thread already owns it.
			 * Otherwise blocks until ownership is taken.
			 */
			void Lock() noexcept;

			/**
			 * @brief Release ownership.
			 *
			 * No-op if this thread is not the owner.
			 */
			void Unlock() noexcept;

		private:
			std::optional<std::thread::id> m_owner_thread_id;	///< Owner, or empty when free.
			std::mutex m_main_mutex;							///< Blocks non-owners.
			std::mutex m_thread_owner_mutex;					///< Guards `m_owner_thread_id`.
	};
}
