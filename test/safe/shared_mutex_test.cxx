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

#include <StormByte/safe/shared_mutex.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <type_traits>

using namespace StormByte;

// -------------------
// Exclusive
// -------------------

int test_exclusive_blocks_shared() {
	Safe::SharedMutex mutex;
	mutex.lock();
	ASSERT_FALSE(mutex.try_lock());
	ASSERT_FALSE(mutex.try_lock_shared());
	mutex.unlock();
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_exclusive_waits_for_shared() {
	Safe::SharedMutex mutex;
	std::atomic<bool> entered(false);
	mutex.lock_shared();
	std::thread waiter([&]() {
		mutex.lock();
		entered.store(true);
		mutex.unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	ASSERT_FALSE(entered.load());
	mutex.unlock_shared();
	waiter.join();
	ASSERT_TRUE(entered.load());
	RETURN_TEST(0);
}

// -------------------
// Shared
// -------------------

int test_shared_allows_another_shared() {
	Safe::SharedMutex mutex;
	mutex.lock_shared();
	ASSERT_TRUE(mutex.try_lock_shared());
	mutex.unlock_shared();
	ASSERT_FALSE(mutex.try_lock());
	mutex.unlock_shared();
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_shared_waits_for_exclusive() {
	Safe::SharedMutex mutex;
	std::atomic<int> entered(0);
	mutex.lock();
	std::thread first([&]() {
		mutex.lock_shared();
		entered.fetch_add(1);
		mutex.unlock_shared();
	});
	std::thread second([&]() {
		mutex.lock_shared();
		entered.fetch_add(1);
		mutex.unlock_shared();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	ASSERT_EQUAL(0, entered.load());
	mutex.unlock();
	first.join();
	second.join();
	ASSERT_EQUAL(2, entered.load());
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_not_copyable_or_movable() {
	static_assert(!std::is_copy_constructible_v<Safe::SharedMutex>);
	static_assert(!std::is_copy_assignable_v<Safe::SharedMutex>);
	static_assert(!std::is_move_constructible_v<Safe::SharedMutex>);
	static_assert(!std::is_move_assignable_v<Safe::SharedMutex>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Exclusive
	// -------------------
	result += test_exclusive_blocks_shared();
	result += test_exclusive_waits_for_shared();

	// -------------------
	// Shared
	// -------------------
	result += test_shared_allows_another_shared();
	result += test_shared_waits_for_exclusive();

	// -------------------
	// Special
	// -------------------
	result += test_not_copyable_or_movable();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
