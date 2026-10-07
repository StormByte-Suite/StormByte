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

#include <StormByte/safe/condition_variable.hxx>
#include <StormByte/visibility.h>

#include <chrono>
#include <cstdint>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	namespace Safe {
		/**
		 * @class ConditionVariableAny
		 * @brief Wakes a thread that waits with any lock it can drop and retake.
		 *
		 * Not a @ref ConditionVariable. That one accepts only @ref UniqueLock. This one accepts @ref UniqueLock, @ref SharedLock and any other lock with @c lock() and @c unlock(). The signal lives on Base's heap. A spurious wake is valid. The predicate overloads filter it in the caller.
		 */
		class STORMBYTE_PUBLIC ConditionVariableAny {
			public:
				/**
				 * @brief Condition with no waiters.
				 * @throws AllocationError The signal cannot be allocated.
				 */
				ConditionVariableAny();

				ConditionVariableAny(const ConditionVariableAny&) = delete;

				ConditionVariableAny(ConditionVariableAny&&) = delete;

				/**
				 * @brief Destroys the signal on Base's heap.
				 * @note No thread may be waiting.
				 */
				~ConditionVariableAny() noexcept;

				ConditionVariableAny& operator=(const ConditionVariableAny&) = delete;

				ConditionVariableAny& operator=(ConditionVariableAny&&) = delete;

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
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @param lock Lock held by the caller. Dropped while waiting and retaken before return.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Lock>
				requires requires(Lock& lock) { lock.lock(); lock.unlock(); }
				void wait(Lock& lock) {
					Enter();
					lock.unlock();
					try {
						Park();
						Leave();
						lock.lock();
					} catch (...) {
						Leave();
						lock.lock();
						throw;
					}
				}

				/**
				 * @brief Wait until @p pred is true.
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @tparam Predicate Callable invoked with the lock held. Its result is tested as a bool.
				 * @param lock Lock held by the caller.
				 * @param pred Predicate. Evaluated in the caller.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError The unpredicated wait translated a foreign failure.
				 */
				template <class Lock, class Predicate>
				requires requires(Lock& lock) { lock.lock(); lock.unlock(); }
				void wait(Lock& lock, Predicate pred) {
					while (!pred())
						wait(lock);
				}

				/**
				 * @brief Wait until notified or @p rel elapses.
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @tparam Rep Tick representation of @p rel.
				 * @tparam Period Tick period of @p rel.
				 * @param lock Lock held by the caller.
				 * @param rel Relative timeout.
				 * @return Whether the wait timed out. A spurious wake is @ref CvStatus::NoTimeout.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Lock, class Rep, class Period>
				requires requires(Lock& lock) { lock.lock(); lock.unlock(); }
				CvStatus wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& rel) {
					return WaitFor(lock, std::chrono::duration_cast<std::chrono::nanoseconds>(rel).count());
				}

				/**
				 * @brief Wait until @p pred is true or @p rel elapses.
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @tparam Rep Tick representation of @p rel.
				 * @tparam Period Tick period of @p rel.
				 * @tparam Predicate Callable invoked with the lock held. Its result is tested as a bool.
				 * @param lock Lock held by the caller.
				 * @param rel Relative timeout.
				 * @param pred Predicate. Evaluated in the caller.
				 * @return Whether @p pred is true on return.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Lock, class Rep, class Period, class Predicate>
				requires requires(Lock& lock) { lock.lock(); lock.unlock(); }
				bool wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& rel, Predicate pred) {
					const auto deadline = std::chrono::steady_clock::now() + rel;
					while (!pred()) {
						if (wait_until(lock, deadline) == CvStatus::Timeout)
							return pred();
					}
					return true;
				}

				/**
				 * @brief Wait until notified or @p abs is reached.
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @tparam Clock Clock of @p abs.
				 * @tparam Duration Duration of @p abs.
				 * @param lock Lock held by the caller.
				 * @param abs Absolute time.
				 * @return Whether the wait timed out.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Lock, class Clock, class Duration>
				requires requires(Lock& lock) { lock.lock(); lock.unlock(); }
				CvStatus wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& abs) {
					const auto rel = abs - Clock::now();
					if (rel <= Clock::duration::zero())
						return CvStatus::Timeout;
					return wait_for(lock, rel);
				}

				/**
				 * @brief Wait until @p pred is true or @p abs is reached.
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @tparam Clock Clock of @p abs.
				 * @tparam Duration Duration of @p abs.
				 * @tparam Predicate Callable invoked with the lock held. Its result is tested as a bool.
				 * @param lock Lock held by the caller.
				 * @param abs Absolute time.
				 * @param pred Predicate. Evaluated in the caller.
				 * @return Whether @p pred is true on return.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Lock, class Clock, class Duration, class Predicate>
				requires requires(Lock& lock) { lock.lock(); lock.unlock(); }
				bool wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& abs, Predicate pred) {
					while (!pred()) {
						if (wait_until(lock, abs) == CvStatus::Timeout)
							return pred();
					}
					return true;
				}

			private:
				/**
				 * @brief Timed wait that drops and retakes @p lock.
				 * @tparam Lock Lock with @c lock() and @c unlock().
				 * @param lock Lock held by the caller.
				 * @param nanos Steady relative timeout, in nanoseconds. Zero or negative times out.
				 * @return Wait result.
				 * @pre @p lock owns its mutex.
				 * @throws OperationError A foreign failure is translated.
				 */
				template <class Lock>
				CvStatus WaitFor(Lock& lock, std::int64_t nanos) {
					if (nanos <= 0)
						return CvStatus::Timeout;
					Enter();
					lock.unlock();
					try {
						const CvStatus status = ParkFor(nanos);
						Leave();
						lock.lock();
						return status;
					} catch (...) {
						Leave();
						lock.lock();
						throw;
					}
				}

				/**
				 * @brief Lock the Base gate so a notify cannot pass before the waiter parks.
				 * @throws OperationError A foreign failure is translated.
				 */
				void Enter();

				/**
				 * @brief Unlock the Base gate.
				 * @pre @ref Enter has locked it and @ref Park has returned.
				 */
				void Leave() noexcept;

				/**
				 * @brief Park on the Base signal. The gate stays locked.
				 * @pre @ref Enter has locked the gate and the caller has dropped its own lock.
				 * @throws OperationError A foreign failure is translated.
				 */
				void Park();

				/**
				 * @brief Park until notified or @p nanos elapse. The gate stays locked.
				 * @param nanos Steady relative timeout, in nanoseconds.
				 * @return Wait result.
				 * @pre @ref Enter has locked the gate and the caller has dropped its own lock.
				 * @throws OperationError A foreign failure is translated.
				 */
				CvStatus ParkFor(std::int64_t nanos);

				void* m_gate;	///< Base-owned mutex and signal. Never null while *this is alive.
		};
	}
}
