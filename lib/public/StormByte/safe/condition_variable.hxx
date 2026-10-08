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

#include <StormByte/safe/unique_lock.hxx>
#include <StormByte/visibility.h>

#include <chrono>
#include <cstdint>

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
		 * @class ConditionVariable
		 * @brief Wakes threads waiting on a @ref Mutex.
		 *
		 * Accepts only @ref UniqueLock. The signal lives on Base's heap. A spurious wake is valid. The predicate overloads filter it in the caller.
		 */
		class STORMBYTE_PUBLIC ConditionVariable {
			public:
				/**
				 * @brief Condition with no waiters.
				 * @throws AllocationError The signal cannot be allocated.
				 */
				ConditionVariable();

				ConditionVariable(const ConditionVariable&) = delete;

				ConditionVariable(ConditionVariable&&) = delete;

				/**
				 * @brief Destroys the signal on Base's heap.
				 * @note No thread may be waiting.
				 */
				~ConditionVariable() noexcept;

				ConditionVariable& operator=(const ConditionVariable&) = delete;

				ConditionVariable& operator=(ConditionVariable&&) = delete;

				/**
				 * @brief Wake one waiter.
				 */
				void notify_one() noexcept;

				/**
				 * @brief Wake every waiter.
				 */
				void notify_all() noexcept;

				/**
				 * @brief Wait until notified. May wake spuriously.
				 * @param lock Lock that owns the mutex. Dropped while waiting and retaken before return.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				void wait(UniqueLock& lock);

				/**
				 * @brief Wait until @p pred is true.
				 * @tparam Predicate Callable invoked with the lock held. Its result is tested as a bool.
				 * @param lock Lock that owns the mutex.
				 * @param pred Predicate. Evaluated in the caller.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError The unpredicated wait translated a foreign failure.
				 */
				template <class Predicate>
				void wait(UniqueLock& lock, Predicate pred) {
					while (!pred())
						wait(lock);
				}

				/**
				 * @brief Wait until notified or @p rel elapses.
				 * @tparam Rep Tick representation of @p rel.
				 * @tparam Period Tick period of @p rel.
				 * @param lock Lock that owns the mutex.
				 * @param rel Relative timeout.
				 * @return Whether the wait timed out. A spurious wake is @ref CvStatus::NoTimeout.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Rep, class Period>
				CvStatus wait_for(UniqueLock& lock, const std::chrono::duration<Rep, Period>& rel) {
					return WaitFor(lock, std::chrono::duration_cast<std::chrono::nanoseconds>(rel).count());
				}

				/**
				 * @brief Wait until @p pred is true or @p rel elapses.
				 * @tparam Rep Tick representation of @p rel.
				 * @tparam Period Tick period of @p rel.
				 * @tparam Predicate Callable invoked with the lock held. Its result is tested as a bool.
				 * @param lock Lock that owns the mutex.
				 * @param rel Relative timeout.
				 * @param pred Predicate. Evaluated in the caller.
				 * @return Whether @p pred is true on return.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Rep, class Period, class Predicate>
				bool wait_for(UniqueLock& lock, const std::chrono::duration<Rep, Period>& rel, Predicate pred) {
					const auto deadline = std::chrono::steady_clock::now() + rel;
					while (!pred()) {
						if (wait_until(lock, deadline) == CvStatus::Timeout)
							return pred();
					}
					return true;
				}

				/**
				 * @brief Wait until notified or @p abs is reached.
				 * @tparam Clock Clock of @p abs.
				 * @tparam Duration Duration of @p abs.
				 * @param lock Lock that owns the mutex.
				 * @param abs Absolute time.
				 * @return Whether the wait timed out.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Clock, class Duration>
				CvStatus wait_until(UniqueLock& lock, const std::chrono::time_point<Clock, Duration>& abs) {
					const auto rel = abs - Clock::now();
					if (rel <= Clock::duration::zero())
						return CvStatus::Timeout;
					return wait_for(lock, rel);
				}

				/**
				 * @brief Wait until @p pred is true or @p abs is reached.
				 * @tparam Clock Clock of @p abs.
				 * @tparam Duration Duration of @p abs.
				 * @tparam Predicate Callable invoked with the lock held. Its result is tested as a bool.
				 * @param lock Lock that owns the mutex.
				 * @param abs Absolute time.
				 * @param pred Predicate. Evaluated in the caller.
				 * @return Whether @p pred is true on return.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Clock, class Duration, class Predicate>
				bool wait_until(UniqueLock& lock, const std::chrono::time_point<Clock, Duration>& abs, Predicate pred) {
					while (!pred()) {
						if (wait_until(lock, abs) == CvStatus::Timeout)
							return pred();
					}
					return true;
				}

			private:
				/**
				 * @brief Timed wait on the Base signal.
				 * @param lock Owning lock.
				 * @param nanos Steady relative timeout, in nanoseconds. Zero or negative times out.
				 * @return Wait result.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				CvStatus WaitFor(UniqueLock& lock, std::int64_t nanos);

				void* m_signal;	///< Base-owned signal. Never null while *this is alive.
		};
	}
}
