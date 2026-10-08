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
#include <StormByte/type_traits/categories.hxx>
#include <StormByte/visibility.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

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
		 * @namespace StormByte::Safe::Detail
		 * @brief Private helpers. Not a supported API.
		 */
		namespace Detail {
			/**
			 * @brief Allocate a zeroed word on Base's heap.
			 * @param bytes Width of the value. One, two, four or eight.
			 * @return Base-owned word. Never null.
			 * @throws AllocationError The word cannot be allocated.
			 */
			STORMBYTE_PUBLIC void* WordCreate(std::size_t bytes);

			/**
			 * @brief Destroy a word from @ref WordCreate.
			 * @param word Word address. Null is ignored.
			 */
			STORMBYTE_PUBLIC void WordDestroy(void* word) noexcept;

			/**
			 * @brief Load the low @p bytes of a word.
			 * @param word Word address.
			 * @param bytes Value width.
			 * @param out Destination of @p bytes. Not null.
			 * @param order Load order.
			 */
			STORMBYTE_PUBLIC void WordLoad(void* word, std::size_t bytes, void* out, MemoryOrder order) noexcept;

			/**
			 * @brief Store @p bytes into the low part of a word.
			 * @param word Word address.
			 * @param bytes Value width.
			 * @param value Source of @p bytes. Not null.
			 * @param order Store order.
			 */
			STORMBYTE_PUBLIC void WordStore(void* word, std::size_t bytes, const void* value, MemoryOrder order) noexcept;

			/**
			 * @brief Replace the value and return the previous one.
			 * @param word Word address.
			 * @param bytes Value width.
			 * @param desired New value, @p bytes long.
			 * @param previous Destination of the previous value.
			 * @param order Order.
			 */
			STORMBYTE_PUBLIC void WordExchange(void* word, std::size_t bytes, const void* desired, void* previous, MemoryOrder order) noexcept;

			/**
			 * @brief Compare-exchange the low @p bytes.
			 * @param word Word address.
			 * @param bytes Value width.
			 * @param expected Expected value. Replaced with the actual value on failure.
			 * @param desired Value to store on success.
			 * @param success Order on success.
			 * @param failure Order on failure. Release and acq-rel are promoted to acquire.
			 * @param weak Whether a spurious failure is allowed.
			 * @return Whether the exchange happened.
			 */
			STORMBYTE_PUBLIC bool WordCompareExchange(void* word, std::size_t bytes, void* expected, const void* desired, MemoryOrder success, MemoryOrder failure, bool weak) noexcept;

			/**
			 * @brief Integer operation on the low @p bytes.
			 * @param word Word address.
			 * @param bytes Value width.
			 * @param delta Operand, already scaled for a pointer.
			 * @param previous Destination of the previous value.
			 * @param order Order.
			 * @param op Zero add, one subtract, two and, three or, four xor.
			 */
			STORMBYTE_PUBLIC void WordFetchOp(void* word, std::size_t bytes, std::int64_t delta, void* previous, MemoryOrder order, int op) noexcept;

			/**
			 * @brief Wait while the low @p bytes still equal @p old.
			 * @param word Word address.
			 * @param bytes Value width.
			 * @param old Captured value.
			 * @param order Load order used by the wait.
			 */
			STORMBYTE_PUBLIC void WordWait(void* word, std::size_t bytes, const void* old, MemoryOrder order) noexcept;

			/**
			 * @brief Wake one waiter on @p word.
			 * @param word Word address.
			 */
			STORMBYTE_PUBLIC void WordNotifyOne(void* word) noexcept;

			/**
			 * @brief Wake every waiter on @p word.
			 * @param word Word address.
			 */
			STORMBYTE_PUBLIC void WordNotifyAll(void* word) noexcept;

			/**
			 * @brief Whether Base's word is lock-free.
			 * @param word Word address.
			 * @return Lock-free flag of the Base word, not of the caller's `std::atomic`.
			 */
			STORMBYTE_PUBLIC bool WordLockFree(void* word) noexcept;
		}

		/**
		 * @class Atomic
		 * @brief Atomic word of one, two, four or eight bytes.
		 *
		 * Not an alias of `std::atomic`. The template copies bytes in the caller. The word lives on Base's heap.
		 * @tparam T Trivially copyable integer, bool, enum or object pointer.
		 */
		template <class T>
		class Atomic {
			static_assert(std::is_trivially_copyable_v<T>);
			static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

			public:
				/**
				 * @brief Zero-initialized value.
				 * @throws AllocationError The word cannot be allocated.
				 */
				Atomic()
				:	m_word{Detail::WordCreate(sizeof(T))} {}

				/**
				 * @brief Value-initialized word.
				 * @param desired Initial value.
				 * @throws AllocationError The word cannot be allocated.
				 */
				explicit Atomic(T desired)
				:	Atomic() {
					store(desired, MemoryOrder::Relaxed);
				}

				Atomic(const Atomic&) = delete;

				Atomic(Atomic&&) = delete;

				/**
				 * @brief Destroys the word on Base's heap.
				 */
				~Atomic() noexcept {
					Detail::WordDestroy(m_word);
				}

				Atomic& operator=(const Atomic&) = delete;

				Atomic& operator=(Atomic&&) = delete;

				/**
				 * @brief Store a value.
				 * @param desired New value.
				 * @return The stored value.
				 */
				T operator=(T desired) noexcept {
					store(desired);
					return desired;
				}

				/**
				 * @brief Whether Base's word is lock-free.
				 * @return Lock-free flag of the Base word.
				 */
				bool is_lock_free() const noexcept {
					return Detail::WordLockFree(m_word);
				}

				/**
				 * @brief Load the value.
				 * @param order Load order.
				 * @return Current value.
				 */
				T load(MemoryOrder order = MemoryOrder::SeqCst) const noexcept {
					T out{};
					Detail::WordLoad(m_word, sizeof(T), &out, order);
					return out;
				}

				/**
				 * @brief Store a value.
				 * @param desired New value.
				 * @param order Store order.
				 */
				void store(T desired, MemoryOrder order = MemoryOrder::SeqCst) noexcept {
					Detail::WordStore(m_word, sizeof(T), &desired, order);
				}

				/**
				 * @brief Replace the value.
				 * @param desired New value.
				 * @param order Order.
				 * @return Previous value.
				 */
				T exchange(T desired, MemoryOrder order = MemoryOrder::SeqCst) noexcept {
					T previous{};
					Detail::WordExchange(m_word, sizeof(T), &desired, &previous, order);
					return previous;
				}

				/**
				 * @brief Weak compare-exchange.
				 * @param expected Expected value. Replaced with the actual value on failure.
				 * @param desired Value to store on success.
				 * @param success Order on success.
				 * @param failure Order on failure. Release and acq-rel are promoted to acquire.
				 * @return Whether the exchange happened.
				 */
				bool compare_exchange_weak(T& expected, T desired, MemoryOrder success, MemoryOrder failure) noexcept {
					return Detail::WordCompareExchange(m_word, sizeof(T), &expected, &desired, success, failure, true);
				}

				/**
				 * @brief Weak compare-exchange with one order.
				 * @param expected Expected value.
				 * @param desired Value to store on success.
				 * @param order Success order. Failure is the matching acquire or relaxed order.
				 * @return Whether the exchange happened.
				 */
				bool compare_exchange_weak(T& expected, T desired, MemoryOrder order = MemoryOrder::SeqCst) noexcept {
					return compare_exchange_weak(expected, desired, order, FailureOrder(order));
				}

				/**
				 * @brief Strong compare-exchange.
				 * @param expected Expected value. Replaced with the actual value on failure.
				 * @param desired Value to store on success.
				 * @param success Order on success.
				 * @param failure Order on failure. Release and acq-rel are promoted to acquire.
				 * @return Whether the exchange happened.
				 */
				bool compare_exchange_strong(T& expected, T desired, MemoryOrder success, MemoryOrder failure) noexcept {
					return Detail::WordCompareExchange(m_word, sizeof(T), &expected, &desired, success, failure, false);
				}

				/**
				 * @brief Strong compare-exchange with one order.
				 * @param expected Expected value.
				 * @param desired Value to store on success.
				 * @param order Success order.
				 * @return Whether the exchange happened.
				 */
				bool compare_exchange_strong(T& expected, T desired, MemoryOrder order = MemoryOrder::SeqCst) noexcept {
					return compare_exchange_strong(expected, desired, order, FailureOrder(order));
				}

				/**
				 * @brief Wait while the value is still @p old.
				 * @param old Captured value.
				 * @param order Load order used by the wait.
				 */
				void wait(T old, MemoryOrder order = MemoryOrder::SeqCst) const noexcept {
					Detail::WordWait(m_word, sizeof(T), &old, order);
				}

				/**
				 * @brief Wake one waiter.
				 */
				void notify_one() noexcept {
					Detail::WordNotifyOne(m_word);
				}

				/**
				 * @brief Wake every waiter.
				 */
				void notify_all() noexcept {
					Detail::WordNotifyAll(m_word);
				}

				/**
				 * @brief Add and return the previous value.
				 * @tparam Delta `T` for an integer. An integer convertible to `std::ptrdiff_t` for an object pointer, counted in elements.
				 * @param delta Addend.
				 * @param order Order.
				 * @return Previous value.
				 *
				 * A member template so an explicit class instantiation does not emit it. On Windows `ptrdiff_t` is `long long`, and MSVC does not mangle a requires clause. An integer literal is `int`, not `ptrdiff_t`, so the pointer path accepts the conversion.
				 */
				template <class Delta>
				T fetch_add(Delta delta, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires ((Type::Integral<T> && std::is_same_v<Delta, T>) || (Type::Pointer<T> && sizeof(std::remove_pointer_t<T>) > 0 && std::is_integral_v<Delta> && std::is_convertible_v<Delta, std::ptrdiff_t>)) {
					if constexpr (Type::Pointer<T>)
						return Fetch(static_cast<std::int64_t>(static_cast<std::ptrdiff_t>(delta)) * static_cast<std::int64_t>(sizeof(std::remove_pointer_t<T>)), order, 0);
					else
						return Fetch(static_cast<std::int64_t>(delta), order, 0);
				}

				/**
				 * @brief Subtract and return the previous value.
				 * @tparam Delta `T` for an integer. An integer convertible to `std::ptrdiff_t` for an object pointer, counted in elements.
				 * @param delta Subtrahend.
				 * @param order Order.
				 * @return Previous value.
				 */
				template <class Delta>
				T fetch_sub(Delta delta, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires ((Type::Integral<T> && std::is_same_v<Delta, T>) || (Type::Pointer<T> && sizeof(std::remove_pointer_t<T>) > 0 && std::is_integral_v<Delta> && std::is_convertible_v<Delta, std::ptrdiff_t>)) {
					if constexpr (Type::Pointer<T>)
						return Fetch(static_cast<std::int64_t>(static_cast<std::ptrdiff_t>(delta)) * static_cast<std::int64_t>(sizeof(std::remove_pointer_t<T>)), order, 1);
					else
						return Fetch(static_cast<std::int64_t>(delta), order, 1);
				}

				/**
				 * @brief Bitwise and, returning the previous value.
				 * @param bits Mask.
				 * @param order Order.
				 * @return Previous value.
				 */
				T fetch_and(T bits, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Type::Integral<T> {
					return Fetch(static_cast<std::int64_t>(bits), order, 2);
				}

				/**
				 * @brief Bitwise or, returning the previous value.
				 * @param bits Mask.
				 * @param order Order.
				 * @return Previous value.
				 */
				T fetch_or(T bits, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Type::Integral<T> {
					return Fetch(static_cast<std::int64_t>(bits), order, 3);
				}

				/**
				 * @brief Bitwise xor, returning the previous value.
				 * @param bits Mask.
				 * @param order Order.
				 * @return Previous value.
				 */
				T fetch_xor(T bits, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Type::Integral<T> {
					return Fetch(static_cast<std::int64_t>(bits), order, 4);
				}

				/**
				 * @brief Replace the value with the greater of it and @p value.
				 * @param value Candidate.
				 * @param order Order.
				 * @return Previous value.
				 *
				 * One function for an integer and an object pointer. MSVC does not mangle a requires clause, so a second overload with the same signature is a redefinition.
				 */
				T fetch_max(T value, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires (Type::Integral<T> || (Type::Pointer<T> && sizeof(std::remove_pointer_t<T>) > 0)) {
					return Extremum(value, order);
				}

				/**
				 * @brief Replace the value with the lesser of it and @p value.
				 * @param value Candidate.
				 * @param order Order.
				 * @return Previous value.
				 */
				T fetch_min(T value, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires (Type::Integral<T> || (Type::Pointer<T> && sizeof(std::remove_pointer_t<T>) > 0)) {
					return Extremum(value, order, false);
				}

				/**
				 * @brief Pre-increment.
				 * @return New value.
				 */
				T operator++() noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_add(T{1}) + T{1});
				}

				/**
				 * @brief Post-increment.
				 * @return Previous value.
				 */
				T operator++(int) noexcept requires Type::Integral<T> {
					return fetch_add(T{1});
				}

				/**
				 * @brief Pre-decrement.
				 * @return New value.
				 */
				T operator--() noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_sub(T{1}) - T{1});
				}

				/**
				 * @brief Post-decrement.
				 * @return Previous value.
				 */
				T operator--(int) noexcept requires Type::Integral<T> {
					return fetch_sub(T{1});
				}

				/**
				 * @brief Add and return the new value.
				 * @param delta Addend.
				 * @return New value.
				 */
				T operator+=(T delta) noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_add(delta) + delta);
				}

				/**
				 * @brief Subtract and return the new value.
				 * @param delta Subtrahend.
				 * @return New value.
				 */
				T operator-=(T delta) noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_sub(delta) - delta);
				}

				/**
				 * @brief And and return the new value.
				 * @param bits Mask.
				 * @return New value.
				 */
				T operator&=(T bits) noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_and(bits) & bits);
				}

				/**
				 * @brief Or and return the new value.
				 * @param bits Mask.
				 * @return New value.
				 */
				T operator|=(T bits) noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_or(bits) | bits);
				}

				/**
				 * @brief Xor and return the new value.
				 * @param bits Mask.
				 * @return New value.
				 */
				T operator^=(T bits) noexcept requires Type::Integral<T> {
					return static_cast<T>(fetch_xor(bits) ^ bits);
				}

				/**
				 * @brief Sequentially consistent load.
				 * @return Current value.
				 */
				operator T() const noexcept {
					return load();
				}

			private:
				/**
				 * @brief Failure order implied by a single success order.
				 * @param success Success order.
				 * @return Acquire for release and acq-rel, otherwise relaxed. Sequential consistency stays.
				 */
				static MemoryOrder FailureOrder(MemoryOrder success) noexcept {
					if (success == MemoryOrder::AcqRel || success == MemoryOrder::Release)
						return MemoryOrder::Acquire;
					if (success == MemoryOrder::SeqCst)
						return MemoryOrder::SeqCst;
					return MemoryOrder::Relaxed;
				}

				/**
				 * @brief Run a fetch operation and return the previous value.
				 * @param delta Operand.
				 * @param order Order.
				 * @param op Operation code understood by @ref Detail::WordFetchOp.
				 * @return Previous value.
				 */
				T Fetch(std::int64_t delta, MemoryOrder order, int op) noexcept {
					T previous{};
					Detail::WordFetchOp(m_word, sizeof(T), delta, &previous, order, op);
					return previous;
				}

				/**
				 * @brief Store the extreme of the current value and @p value.
				 * @param value Candidate.
				 * @param order Order of the successful exchange.
				 * @param maximum Whether the greater value wins. False keeps the lesser.
				 * @return Previous value.
				 */
				T Extremum(T value, MemoryOrder order, bool maximum = true) noexcept {
					T current = load(MemoryOrder::Relaxed);
					for (;;) {
						const bool replace = maximum ? (current < value) : (value < current);
						if (!replace)
							return current;
						if (compare_exchange_weak(current, value, order))
							return current;
					}
				}

				void* m_word;	///< Base-owned word. Not a `std::atomic` in the caller.
		};

		/// @cond
		extern template class STORMBYTE_PUBLIC Atomic<bool>;
		extern template class STORMBYTE_PUBLIC Atomic<char>;
		extern template class STORMBYTE_PUBLIC Atomic<signed char>;
		extern template class STORMBYTE_PUBLIC Atomic<unsigned char>;
		extern template class STORMBYTE_PUBLIC Atomic<char8_t>;
		extern template class STORMBYTE_PUBLIC Atomic<wchar_t>;
		extern template class STORMBYTE_PUBLIC Atomic<char16_t>;
		extern template class STORMBYTE_PUBLIC Atomic<char32_t>;
		extern template class STORMBYTE_PUBLIC Atomic<short>;
		extern template class STORMBYTE_PUBLIC Atomic<unsigned short>;
		extern template class STORMBYTE_PUBLIC Atomic<int>;
		extern template class STORMBYTE_PUBLIC Atomic<unsigned int>;
		extern template class STORMBYTE_PUBLIC Atomic<long>;
		extern template class STORMBYTE_PUBLIC Atomic<unsigned long>;
		extern template class STORMBYTE_PUBLIC Atomic<long long>;
		extern template class STORMBYTE_PUBLIC Atomic<unsigned long long>;
		/// @endcond
	}
}
