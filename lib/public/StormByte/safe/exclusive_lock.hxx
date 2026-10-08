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

#include <StormByte/safe/memory_order.hxx>
#include <StormByte/safe/shared_mutex.hxx>
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
		 * @class ExclusiveLock
		 * @brief Owns at most one exclusive hold of a @ref SharedMutex.
		 *
		 * Header-only. The object is a pointer and an ownership flag. It does not contain a standard lock. @c lock and @c unlock take and drop the exclusive hold, so @ref ConditionVariableAny can wait on it. It does not take the shared hold. That is @ref SharedLock.
		 */
		class STORMBYTE_PUBLIC_TYPE ExclusiveLock {
			public:
				/**
				 * @brief Empty lock. Owns nothing.
				 */
				ExclusiveLock() noexcept
				:	m_mutex{nullptr}, m_owns{false} {}

				/**
				 * @brief Take the exclusive hold of @p mutex.
				 * @param mutex Shared mutex to acquire.
				 * @throws OperationError The exclusive lock translated a foreign failure.
				 */
				explicit ExclusiveLock(SharedMutex& mutex)
				:	m_mutex{&mutex}, m_owns{false} {
					mutex.lock();
					m_owns = true;
				}

				/**
				 * @brief Associate @p mutex without locking it.
				 * @param mutex Shared mutex to lock later.
				 * @param tag Deferred-lock tag.
				 */
				ExclusiveLock(SharedMutex& mutex, DeferLock tag) noexcept
				:	m_mutex{&mutex}, m_owns{false} {
					(void)tag;
				}

				/**
				 * @brief Try to take the exclusive hold of @p mutex.
				 * @param mutex Shared mutex to try.
				 * @param tag Try-lock tag.
				 */
				ExclusiveLock(SharedMutex& mutex, TryToLock tag) noexcept
				:	m_mutex{&mutex}, m_owns{mutex.try_lock()} {
					(void)tag;
				}

				/**
				 * @brief Assume the calling thread already holds @p mutex exclusively.
				 * @param mutex Shared mutex already held by this thread.
				 * @param tag Adopt tag.
				 */
				ExclusiveLock(SharedMutex& mutex, AdoptLock tag) noexcept
				:	m_mutex{&mutex}, m_owns{true} {
					(void)tag;
				}

				ExclusiveLock(const ExclusiveLock&) = delete;

				/**
				 * @brief Take a lock. @p other owns nothing afterwards.
				 * @param other Source.
				 */
				ExclusiveLock(ExclusiveLock&& other) noexcept
				:	m_mutex{other.m_mutex}, m_owns{other.m_owns} {
					other.m_mutex = nullptr;
					other.m_owns = false;
				}

				/**
				 * @brief Drop the exclusive hold if this lock owns it.
				 */
				~ExclusiveLock() noexcept {
					if (m_owns)
						m_mutex->unlock();
				}

				ExclusiveLock& operator=(const ExclusiveLock&) = delete;

				/**
				 * @brief Take a lock. Drops the previous exclusive hold when owned.
				 * @param other Source. Left not owning.
				 * @return This lock.
				 */
				ExclusiveLock& operator=(ExclusiveLock&& other) noexcept {
					if (this != &other) {
						if (m_owns)
							m_mutex->unlock();
						m_mutex = other.m_mutex;
						m_owns = other.m_owns;
						other.m_mutex = nullptr;
						other.m_owns = false;
					}
					return *this;
				}

				/**
				 * @brief Take the exclusive hold of the associated mutex.
				 * @pre A mutex is associated and this lock does not own it.
				 * @throws OperationError The exclusive lock translated a foreign failure.
				 */
				void lock() {
					m_mutex->lock();
					m_owns = true;
				}

				/**
				 * @brief Try to take the exclusive hold of the associated mutex.
				 * @return Whether this lock now owns it. False when no mutex is associated.
				 */
				bool try_lock() noexcept {
					if (m_mutex == nullptr)
						return false;
					m_owns = m_mutex->try_lock();
					return m_owns;
				}

				/**
				 * @brief Drop the exclusive hold.
				 * @pre This lock owns the exclusive hold.
				 */
				void unlock() noexcept {
					m_mutex->unlock();
					m_owns = false;
				}

				/**
				 * @brief Drop the association without unlocking.
				 * @return The previous mutex, or null.
				 */
				SharedMutex* release() noexcept {
					SharedMutex* released = m_mutex;
					m_mutex = nullptr;
					m_owns = false;
					return released;
				}

				/**
				 * @brief Associated mutex.
				 * @return The mutex, or null.
				 */
				SharedMutex* mutex() const noexcept {
					return m_mutex;
				}

				/**
				 * @brief Whether this lock owns the exclusive hold.
				 * @return Ownership flag.
				 */
				bool owns_lock() const noexcept {
					return m_owns;
				}

				/**
				 * @brief Whether this lock owns the exclusive hold.
				 * @return Ownership flag.
				 */
				explicit operator bool() const noexcept {
					return m_owns;
				}

				/**
				 * @brief Exchange two locks.
				 * @param other Other lock.
				 */
				void swap(ExclusiveLock& other) noexcept {
					SharedMutex* held = m_mutex;
					const bool owns = m_owns;
					m_mutex = other.m_mutex;
					m_owns = other.m_owns;
					other.m_mutex = held;
					other.m_owns = owns;
				}

			private:
				SharedMutex* m_mutex;	///< Associated shared mutex, or null.

				bool m_owns;			///< Whether this lock holds m_mutex exclusively.
		};

		/**
		 * @brief Exchange two exclusive locks.
		 * @param left First lock.
		 * @param right Second lock.
		 */
		inline void swap(ExclusiveLock& left, ExclusiveLock& right) noexcept {
			left.swap(right);
		}
	}
}
