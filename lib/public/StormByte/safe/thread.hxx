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

#include <StormByte/exception.hxx>
#include <StormByte/safe/function.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/visibility.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

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
		 * @namespace StormByte::Safe::this_thread
		 * @brief Operations on the calling execution. The names match `std::this_thread`.
		 */
		namespace this_thread {}

		/**
		 * @namespace StormByte::Safe::Detail
		 * @brief Private helpers. Not a supported API.
		 */
		namespace Detail {
			/**
			 * @brief Whether @p T is a void-returning @ref Function.
			 * @tparam T Candidate.
			 */
			template <class T>
			struct IsVoidFunction: std::false_type {};

			/**
			 * @brief A void-returning @ref Function.
			 * @tparam Args Argument types.
			 */
			template <class... Args>
			struct IsVoidFunction<Function<void(Args...)>>: std::true_type {};

			/**
			 * @brief Start a thread whose body lives in Base.
			 * @param entry Entry. Not null. Base calls it and then returns.
			 * @param argument Argument passed to @p entry. May be null.
			 * @return Base-owned thread. Never null.
			 * @throws AllocationError The thread cannot be started.
			 */
			STORMBYTE_PUBLIC void* ThreadStart(void (*entry)(void*), void* argument);

			/**
			 * @brief Destroy a thread from @ref ThreadStart.
			 * @param thread Thread address. A joinable thread terminates the process. Null is ignored.
			 */
			STORMBYTE_PUBLIC void ThreadDestroy(void* thread) noexcept;

			/**
			 * @brief Whether the thread still owns an execution.
			 * @param thread Thread address. Not null.
			 * @return Joinable flag.
			 */
			STORMBYTE_PUBLIC bool ThreadJoinable(void* thread) noexcept;

			/**
			 * @brief Wait until the execution returns.
			 * @param thread Thread address. Not null. Must be joinable.
			 * @throws Exception The thread is not joinable.
			 */
			STORMBYTE_PUBLIC void ThreadJoin(void* thread);

			/**
			 * @brief Detach the execution.
			 * @param thread Thread address. Not null. Must be joinable.
			 * @throws Exception The thread is not joinable.
			 */
			STORMBYTE_PUBLIC void ThreadDetach(void* thread);

			/**
			 * @brief Identifier of the owned execution.
			 * @param thread Thread address. Not null.
			 * @return Zero when the object is not joinable.
			 */
			STORMBYTE_PUBLIC std::uint64_t ThreadId(void* thread) noexcept;

			/**
			 * @brief Native handle as an integer. Not a CRT thread type.
			 * @param thread Thread address. Not null.
			 * @return Zero when the object is not joinable.
			 */
			STORMBYTE_PUBLIC std::uintptr_t ThreadNative(void* thread) noexcept;

			/**
			 * @brief Identifier of the calling execution.
			 * @return Non-zero identifier.
			 */
			STORMBYTE_PUBLIC std::uint64_t ThreadSelf() noexcept;

			/**
			 * @brief Hint of concurrent executions.
			 * @return Zero when the hint is not available.
			 */
			STORMBYTE_PUBLIC unsigned ThreadHardware() noexcept;

			/**
			 * @brief Offer the calling execution.
			 */
			STORMBYTE_PUBLIC void ThreadYield() noexcept;

			/**
			 * @brief Sleep the calling execution.
			 * @param nanoseconds Duration. Zero returns immediately.
			 */
			STORMBYTE_PUBLIC void ThreadSleep(std::uint64_t nanoseconds) noexcept;

			/**
			 * @brief Decayed callable and arguments, allocated on Base's heap.
			 * @tparam Callable Decayed callable.
			 * @tparam Args Decayed arguments.
			 */
			template <class Callable, class... Args>
			struct ThreadState {
				Callable callable;				///< Entry.
				std::tuple<Args...> arguments;	///< Arguments captured before the execution starts.

				/**
				 * @brief Capture the entry.
				 * @param fn Entry.
				 * @param args Arguments.
				 */
				STORMBYTE_FORCE_INLINE ThreadState(Callable&& fn, Args&&... args)
				:	callable(std::move(fn)), arguments(std::move(args)...) {}

				/**
				 * @brief Invoke and destroy the state.
				 * @param raw State address. Not null.
				 */
				STORMBYTE_FORCE_INLINE static void Run(void* raw) {
					ThreadState* state = static_cast<ThreadState*>(raw);
					std::apply(std::move(state->callable), std::move(state->arguments));
					state->~ThreadState();
					Heap::Free(raw);
				}
			};

			/**
			 * @brief @ref Function and its arguments, allocated on Base's heap.
			 * @tparam Args Argument types of the void function.
			 *
			 * @c Call runs in the new execution. Its @ref Status stays there. A Base exception does not escape: an uncaught exception would terminate the process.
			 */
			template <class... Args>
			struct FunctionState {
				Function<void(Args...)> function;	///< Creator-owned callback.
				std::tuple<Args...> arguments;		///< Arguments captured before the execution starts.

				/**
				 * @brief Capture the callback and its arguments.
				 * @param fn Callback. Moved.
				 * @param args Arguments.
				 */
				STORMBYTE_FORCE_INLINE FunctionState(Function<void(Args...)>&& fn, Args&&... args)
				:	function(std::move(fn)), arguments(std::move(args)...) {}

				/**
				 * @brief Invoke @c Call and destroy the state.
				 * @param raw State address. Not null.
				 */
				STORMBYTE_FORCE_INLINE static void Run(void* raw) {
					FunctionState* state = static_cast<FunctionState*>(raw);
					try {
						std::apply([&state](Args&... args) {
							(void)state->function.Call(args...);
						}, state->arguments);
					}
					catch (const StormByte::Exception&) {}
					state->~FunctionState();
					Heap::Free(raw);
				}
			};
		}

		/**
		 * @class Thread
		 * @brief Execution owned by Base.
		 *
		 * Not an alias of `std::thread`. The native thread is started and joined in Base. Destroying a joinable thread terminates the process, as `std::thread` does.
		 * A @ref Function is stored by value and invoked with @c Call. The creator-module release stays with the function.
		 */
		class STORMBYTE_PUBLIC Thread {
			public:
				/**
				 * @class Id
				 * @brief Identifier of an execution. Zero is not an execution.
				 */
				class STORMBYTE_PUBLIC Id {
					friend class Thread;

					public:
						/**
						 * @brief Not an execution.
						 */
						inline Id() noexcept
						:	m_value{0} {}

						/**
						 * @brief Compare two identifiers.
						 * @param other Other identifier.
						 * @return Whether they name the same execution.
						 */
						inline bool operator==(const Id& other) const noexcept {
							return m_value == other.m_value;
						}

						/**
						 * @brief Compare two identifiers.
						 * @param other Other identifier.
						 * @return Whether they name different executions.
						 */
						inline bool operator!=(const Id& other) const noexcept {
							return m_value != other.m_value;
						}

						/**
						 * @brief Order two identifiers.
						 * @param other Other identifier.
						 * @return Whether this identifier is less.
						 */
						inline bool operator<(const Id& other) const noexcept {
							return m_value < other.m_value;
						}

						/**
						 * @brief Order two identifiers.
						 * @param other Other identifier.
						 * @return Whether this identifier is less or equal.
						 */
						inline bool operator<=(const Id& other) const noexcept {
							return m_value <= other.m_value;
						}

						/**
						 * @brief Order two identifiers.
						 * @param other Other identifier.
						 * @return Whether this identifier is greater.
						 */
						inline bool operator>(const Id& other) const noexcept {
							return m_value > other.m_value;
						}

						/**
						 * @brief Order two identifiers.
						 * @param other Other identifier.
						 * @return Whether this identifier is greater or equal.
						 */
						inline bool operator>=(const Id& other) const noexcept {
							return m_value >= other.m_value;
						}

						/**
						 * @brief Value used by a hash.
						 * @return Raw identifier. Zero is not an execution.
						 */
						inline std::uint64_t Value() const noexcept {
							return m_value;
						}

					private:
						/**
						 * @brief Identifier from Base.
						 * @param value Raw identifier.
						 */
						inline explicit Id(std::uint64_t value) noexcept
						:	m_value{value} {}

						std::uint64_t m_value;	///< Zero is not an execution.
				};

				/**
				 * @brief Native handle as an integer. Not a CRT thread type.
				 */
				using native_handle_type = std::uintptr_t;

				/**
				 * @brief Not an execution.
				 */
				inline Thread() noexcept
				:	m_thread{nullptr} {}

				/**
				 * @brief Start @p callable in a Base-owned execution.
				 * @tparam Callable Callable. Invoked in the new execution. A @ref Function uses the other constructor.
				 * @tparam Args Argument types, decayed and stored before the execution starts.
				 * @param callable Entry.
				 * @param args Arguments.
				 * @throws AllocationError The execution or its state cannot be allocated.
				 */
				template <class Callable, class... Args>
				STORMBYTE_FORCE_INLINE explicit Thread(Callable&& callable, Args&&... args) requires (!Detail::IsVoidFunction<std::decay_t<Callable>>::value)
				:	Thread() {
					using State = Detail::ThreadState<std::decay_t<Callable>, std::decay_t<Args>...>;
					void* raw = Heap::Allocate(sizeof(State));
					State* state = ::new (raw) State(std::forward<Callable>(callable), std::forward<Args>(args)...);
					m_thread = Detail::ThreadStart(&State::Run, state);
				}

				/**
				 * @brief Start a void @ref Function in a Base-owned execution.
				 * @tparam Args Argument types. Captured before the execution starts.
				 * @param function Callback. Moved. Its release stays in the creator module.
				 * @param args Arguments passed to @c Call.
				 * @throws AllocationError The execution or its state cannot be allocated.
				 *
				 * @c Status is not returned. A Base exception is caught in the execution.
				 */
				template <class... Args>
				STORMBYTE_FORCE_INLINE explicit Thread(Function<void(Args...)> function, Args&&... args)
				:	Thread() {
					using State = Detail::FunctionState<std::decay_t<Args>...>;
					void* raw = Heap::Allocate(sizeof(State));
					State* state = ::new (raw) State(std::move(function), std::forward<Args>(args)...);
					m_thread = Detail::ThreadStart(&State::Run, state);
				}

				Thread(const Thread&) = delete;

				/**
				 * @brief Take an execution.
				 * @param other Source. Left not joinable.
				 */
				inline Thread(Thread&& other) noexcept
				:	Thread() {
					swap(other);
				}

				/**
				 * @brief Destroy the handle. A joinable execution terminates the process.
				 */
				inline ~Thread() noexcept {
					Detail::ThreadDestroy(m_thread);
				}

				Thread& operator=(const Thread&) = delete;

				/**
				 * @brief Take an execution. A joinable destination terminates the process.
				 * @param other Source. Left not joinable.
				 * @return This thread.
				 */
				inline Thread& operator=(Thread&& other) noexcept {
					if (this != &other) {
						Detail::ThreadDestroy(m_thread);
						m_thread = nullptr;
						swap(other);
					}
					return *this;
				}

				/**
				 * @brief Exchange two executions.
				 * @param other Other thread.
				 */
				inline void swap(Thread& other) noexcept {
					void* mine = m_thread;
					m_thread = other.m_thread;
					other.m_thread = mine;
				}

				/**
				 * @brief Whether this object owns an execution.
				 * @return Joinable flag.
				 */
				inline bool joinable() const noexcept {
					return m_thread != nullptr && Detail::ThreadJoinable(m_thread);
				}

				/**
				 * @brief Wait until the execution returns.
				 * @throws Exception The thread is not joinable.
				 */
				inline void join() {
					Detail::ThreadJoin(m_thread);
				}

				/**
				 * @brief Detach the execution.
				 * @throws Exception The thread is not joinable.
				 */
				inline void detach() {
					Detail::ThreadDetach(m_thread);
				}

				/**
				 * @brief Identifier of the owned execution.
				 * @return Zero identifier when this object is not joinable.
				 */
				inline Id get_id() const noexcept {
					if (m_thread == nullptr)
						return Id{};
					return Id{Detail::ThreadId(m_thread)};
				}

				/**
				 * @brief Native handle as an integer.
				 * @return Zero when this object is not joinable.
				 */
				inline native_handle_type native_handle() noexcept {
					if (m_thread == nullptr)
						return 0;
					return Detail::ThreadNative(m_thread);
				}

				/**
				 * @brief Hint of concurrent executions.
				 * @return Zero when the hint is not available.
				 */
				inline static unsigned hardware_concurrency() noexcept {
					return Detail::ThreadHardware();
				}

				/**
				 * @brief Identifier of the calling execution.
				 * @return Non-zero identifier.
				 */
				inline static Id Calling() noexcept {
					return Id{Detail::ThreadSelf()};
				}

			private:
				void* m_thread;	///< Base-owned thread. Not a `std::thread` in the caller.
		};

		/**
		 * @brief Exchange two executions.
		 * @param left Left thread.
		 * @param right Right thread.
		 */
		inline void swap(Thread& left, Thread& right) noexcept {
			left.swap(right);
		}

		namespace this_thread {
			/**
			 * @brief Identifier of the calling execution.
			 * @return Non-zero identifier.
			 */
			inline Thread::Id get_id() noexcept {
				return Thread::Calling();
			}

			/**
			 * @brief Offer the calling execution.
			 */
			inline void yield() noexcept {
				Detail::ThreadYield();
			}

			/**
			 * @brief Sleep the calling execution.
			 * @tparam Rep Duration representation.
			 * @tparam Period Duration period.
			 * @param time Duration. A negative duration returns immediately.
			 */
			template <class Rep, class Period>
			STORMBYTE_FORCE_INLINE void sleep_for(const std::chrono::duration<Rep, Period>& time) noexcept {
				if (time <= std::chrono::duration<Rep, Period>::zero())
					return;
				const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
				if (nanos <= 0)
					return;
				Detail::ThreadSleep(static_cast<std::uint64_t>(nanos));
			}

			/**
			 * @brief Sleep until a time point.
			 * @tparam Clock Clock.
			 * @tparam Duration Duration.
			 * @param time Absolute time.
			 */
			template <class Clock, class Duration>
			STORMBYTE_FORCE_INLINE void sleep_until(const std::chrono::time_point<Clock, Duration>& time) noexcept {
				sleep_for(time - Clock::now());
			}
		}
	}
}
