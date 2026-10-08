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

#include <StormByte/safe/atomic.hxx>
#include <StormByte/test_handlers.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <type_traits>

using namespace StormByte;

namespace {
	enum class Phase : unsigned char {
		Idle,
		Running,
		Done
	};

	void WaitUntil(const Safe::Atomic<bool>& flag) {
		while (!flag.load(Safe::MemoryOrder::Acquire))
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

// -------------------
// Bool
// -------------------

int test_bool_compare_exchange_strong_fails_and_updates_expected() {
	Safe::Atomic<bool> value(false);
	bool expected = true;
	ASSERT_FALSE(value.compare_exchange_strong(expected, true));
	ASSERT_FALSE(expected);
	ASSERT_FALSE(value.load());
	RETURN_TEST(0);
}

int test_bool_compare_exchange_strong_succeeds() {
	Safe::Atomic<bool> value(false);
	bool expected = false;
	ASSERT_TRUE(value.compare_exchange_strong(expected, true, Safe::MemoryOrder::Release, Safe::MemoryOrder::Acquire));
	ASSERT_TRUE(value.load(Safe::MemoryOrder::Acquire));
	RETURN_TEST(0);
}

int test_bool_compare_exchange_weak_succeeds() {
	Safe::Atomic<bool> value(false);
	bool expected = false;
	bool stored = false;
	for (int attempt = 0; attempt < 8 && !stored; ++attempt)
		stored = value.compare_exchange_weak(expected, true);
	ASSERT_TRUE(stored);
	ASSERT_TRUE(static_cast<bool>(value));
	RETURN_TEST(0);
}

int test_bool_exchange_returns_previous() {
	Safe::Atomic<bool> value(false);
	ASSERT_FALSE(value.exchange(true));
	ASSERT_TRUE(value.load());
	RETURN_TEST(0);
}

int test_bool_store_and_load() {
	Safe::Atomic<bool> value;
	ASSERT_FALSE(value.load());
	value.store(true, Safe::MemoryOrder::Relaxed);
	ASSERT_TRUE(value.load(Safe::MemoryOrder::Relaxed));
	value = false;
	ASSERT_FALSE(value.load());
	RETURN_TEST(0);
}

// -------------------
// Enum
// -------------------

int test_enum_round_trip() {
	Safe::Atomic<Phase> phase(Phase::Idle);
	ASSERT_TRUE(phase.load() == Phase::Idle);
	phase.store(Phase::Running, Safe::MemoryOrder::Release);
	ASSERT_TRUE(phase.load(Safe::MemoryOrder::Acquire) == Phase::Running);
	ASSERT_TRUE(phase.exchange(Phase::Done) == Phase::Running);
	Phase expected = Phase::Done;
	ASSERT_TRUE(phase.compare_exchange_strong(expected, Phase::Idle));
	ASSERT_TRUE(phase.load() == Phase::Idle);
	RETURN_TEST(0);
}

// -------------------
// Integer
// -------------------

int test_int64_fetch_and_operators() {
	Safe::Atomic<std::int64_t> value(std::int64_t{7});
	ASSERT_EQUAL(std::int64_t{7}, value.fetch_add(std::int64_t{3}));
	ASSERT_EQUAL(std::int64_t{10}, value.load());
	ASSERT_EQUAL(std::int64_t{10}, value.fetch_sub(std::int64_t{4}));
	ASSERT_EQUAL(std::int64_t{6}, value.load());
	ASSERT_EQUAL(std::int64_t{6}, value++);
	ASSERT_EQUAL(std::int64_t{7}, value.load());
	ASSERT_EQUAL(std::int64_t{8}, ++value);
	ASSERT_EQUAL(std::int64_t{8}, value--);
	ASSERT_EQUAL(std::int64_t{6}, --value);
	value += std::int64_t{4};
	ASSERT_EQUAL(std::int64_t{10}, value.load());
	value -= std::int64_t{1};
	ASSERT_EQUAL(std::int64_t{9}, value.load());
	RETURN_TEST(0);
}

int test_int64_fetch_max_keeps_signed_order() {
	Safe::Atomic<std::int64_t> value(std::int64_t{-2});
	ASSERT_EQUAL(std::int64_t{-2}, value.fetch_max(std::int64_t{-8}));
	ASSERT_EQUAL(std::int64_t{-2}, value.load());
	ASSERT_EQUAL(std::int64_t{-2}, value.fetch_max(std::int64_t{4}, Safe::MemoryOrder::Release));
	ASSERT_EQUAL(std::int64_t{4}, value.load(Safe::MemoryOrder::Acquire));
	RETURN_TEST(0);
}

int test_int64_fetch_min_keeps_signed_order() {
	Safe::Atomic<std::int64_t> value(std::int64_t{-2});
	ASSERT_EQUAL(std::int64_t{-2}, value.fetch_min(std::int64_t{4}));
	ASSERT_EQUAL(std::int64_t{-2}, value.load());
	ASSERT_EQUAL(std::int64_t{-2}, value.fetch_min(std::int64_t{-8}, Safe::MemoryOrder::AcqRel));
	ASSERT_EQUAL(std::int64_t{-8}, value.load());
	RETURN_TEST(0);
}

int test_size_fetch_bitwise() {
	Safe::Atomic<std::size_t> value(std::size_t{0x0f});
	ASSERT_EQUAL(std::size_t{0x0f}, value.fetch_or(std::size_t{0xf0}));
	ASSERT_EQUAL(std::size_t{0xff}, value.load());
	ASSERT_EQUAL(std::size_t{0xff}, value.fetch_and(std::size_t{0x3c}));
	ASSERT_EQUAL(std::size_t{0x3c}, value.load());
	ASSERT_EQUAL(std::size_t{0x3c}, value.fetch_xor(std::size_t{0x0f}));
	ASSERT_EQUAL(std::size_t{0x33}, value.load());
	value |= std::size_t{0x0f};
	value &= std::size_t{0x0f};
	value ^= std::size_t{0x06};
	ASSERT_EQUAL(std::size_t{0x09}, value.load());
	RETURN_TEST(0);
}

int test_size_fetch_max_and_min() {
	Safe::Atomic<std::size_t> value(std::size_t{8});
	ASSERT_EQUAL(std::size_t{8}, value.fetch_max(std::size_t{3}));
	ASSERT_EQUAL(std::size_t{8}, value.load());
	ASSERT_EQUAL(std::size_t{8}, value.fetch_max(std::size_t{12}));
	ASSERT_EQUAL(std::size_t{12}, value.load());
	ASSERT_EQUAL(std::size_t{12}, value.fetch_min(std::size_t{20}));
	ASSERT_EQUAL(std::size_t{12}, value.load());
	ASSERT_EQUAL(std::size_t{12}, value.fetch_min(std::size_t{1}));
	ASSERT_EQUAL(std::size_t{1}, value.load());
	RETURN_TEST(0);
}

int test_size_wraps_on_its_width() {
	Safe::Atomic<std::uint8_t> value(std::uint8_t{255});
	ASSERT_EQUAL(std::uint8_t{255}, value.fetch_add(std::uint8_t{2}));
	ASSERT_EQUAL(std::uint8_t{1}, value.load());
	RETURN_TEST(0);
}

// -------------------
// Pointer
// -------------------

int test_pointer_fetch_advances_by_elements() {
	int cells[4] = {1, 2, 3, 4};
	Safe::Atomic<int*> cursor(cells);
	int* previous = cursor.fetch_add(2);
	ASSERT_TRUE(previous == cells);
	ASSERT_TRUE(cursor.load() == cells + 2);
	ASSERT_TRUE(cursor.fetch_sub(1) == cells + 2);
	ASSERT_TRUE(cursor.load() == cells + 1);
	RETURN_TEST(0);
}

int test_pointer_fetch_max_and_min() {
	int cells[4] = {1, 2, 3, 4};
	Safe::Atomic<int*> cursor(cells + 1);
	ASSERT_TRUE(cursor.fetch_max(cells) == cells + 1);
	ASSERT_TRUE(cursor.load() == cells + 1);
	ASSERT_TRUE(cursor.fetch_max(cells + 3) == cells + 1);
	ASSERT_TRUE(cursor.load() == cells + 3);
	ASSERT_TRUE(cursor.fetch_min(cells + 3) == cells + 3);
	ASSERT_TRUE(cursor.load() == cells + 3);
	ASSERT_TRUE(cursor.fetch_min(cells) == cells + 3);
	ASSERT_TRUE(cursor.load() == cells);
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_explicit_value_constructor() {
	Safe::Atomic<std::size_t> value(std::size_t{42});
	ASSERT_EQUAL(std::size_t{42}, value.load(Safe::MemoryOrder::Relaxed));
	RETURN_TEST(0);
}

int test_lock_free_is_reported() {
	Safe::Atomic<std::size_t> value;
	ASSERT_TRUE(value.is_lock_free());
	RETURN_TEST(0);
}

int test_not_copyable_or_movable() {
	static_assert(!std::is_copy_constructible_v<Safe::Atomic<std::size_t>>);
	static_assert(!std::is_copy_assignable_v<Safe::Atomic<std::size_t>>);
	static_assert(!std::is_move_constructible_v<Safe::Atomic<std::size_t>>);
	static_assert(!std::is_move_assignable_v<Safe::Atomic<std::size_t>>);
	RETURN_TEST(0);
}

// -------------------
// Wait
// -------------------

int test_wait_wakes_on_notify_all() {
	Safe::Atomic<std::size_t> generation(std::size_t{0});
	Safe::Atomic<bool> started(false);
	std::atomic<int> awake(0);
	std::thread first([&]() {
		const auto captured = generation.load(Safe::MemoryOrder::Acquire);
		started.store(true, Safe::MemoryOrder::Release);
		generation.wait(captured, Safe::MemoryOrder::Acquire);
		awake.fetch_add(1);
	});
	std::thread second([&]() {
		const auto captured = generation.load(Safe::MemoryOrder::Acquire);
		generation.wait(captured, Safe::MemoryOrder::Acquire);
		awake.fetch_add(1);
	});
	WaitUntil(started);
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	generation.fetch_add(std::size_t{1}, Safe::MemoryOrder::Release);
	generation.notify_all();
	first.join();
	second.join();
	ASSERT_EQUAL(2, awake.load());
	RETURN_TEST(0);
}

int test_wait_wakes_on_notify_one() {
	Safe::Atomic<std::size_t> generation(std::size_t{4});
	Safe::Atomic<bool> started(false);
	std::thread waiter([&]() {
		const auto captured = generation.load(Safe::MemoryOrder::Acquire);
		started.store(true, Safe::MemoryOrder::Release);
		generation.wait(captured, Safe::MemoryOrder::Acquire);
	});
	WaitUntil(started);
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	generation.store(std::size_t{5}, Safe::MemoryOrder::Release);
	generation.notify_one();
	waiter.join();
	ASSERT_EQUAL(std::size_t{5}, generation.load());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Bool
	// -------------------
	result += test_bool_compare_exchange_strong_fails_and_updates_expected();
	result += test_bool_compare_exchange_strong_succeeds();
	result += test_bool_compare_exchange_weak_succeeds();
	result += test_bool_exchange_returns_previous();
	result += test_bool_store_and_load();

	// -------------------
	// Enum
	// -------------------
	result += test_enum_round_trip();

	// -------------------
	// Integer
	// -------------------
	result += test_int64_fetch_and_operators();
	result += test_int64_fetch_max_keeps_signed_order();
	result += test_int64_fetch_min_keeps_signed_order();
	result += test_size_fetch_bitwise();
	result += test_size_fetch_max_and_min();
	result += test_size_wraps_on_its_width();

	// -------------------
	// Pointer
	// -------------------
	result += test_pointer_fetch_advances_by_elements();
	result += test_pointer_fetch_max_and_min();

	// -------------------
	// Special
	// -------------------
	result += test_explicit_value_constructor();
	result += test_lock_free_is_reported();
	result += test_not_copyable_or_movable();

	// -------------------
	// Wait
	// -------------------
	result += test_wait_wakes_on_notify_all();
	result += test_wait_wakes_on_notify_one();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
