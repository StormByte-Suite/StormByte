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
	bool exclusive_acquired = false;
	bool shared_acquired = false;
	std::thread contender([&]() {
		exclusive_acquired = mutex.try_lock();
		if (exclusive_acquired)
			mutex.unlock();
		shared_acquired = mutex.try_lock_shared();
		if (shared_acquired)
			mutex.unlock_shared();
	});
	contender.join();
	mutex.unlock();
	ASSERT_FALSE(exclusive_acquired);
	ASSERT_FALSE(shared_acquired);
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_exclusive_rejected_until_shared_unlock() {
	Safe::SharedMutex mutex;
	bool entered = false;
	mutex.lock_shared();
	std::thread waiter([&]() {
		entered = mutex.try_lock();
		if (entered)
			mutex.unlock();
	});
	waiter.join();
	mutex.unlock_shared();
	ASSERT_FALSE(entered);
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

// -------------------
// Shared
// -------------------

int test_shared_allows_another_shared() {
	Safe::SharedMutex mutex;
	mutex.lock_shared();
	bool shared_acquired = false;
	bool exclusive_acquired = false;
	std::thread reader([&]() {
		shared_acquired = mutex.try_lock_shared();
		if (shared_acquired)
			mutex.unlock_shared();
		exclusive_acquired = mutex.try_lock();
		if (exclusive_acquired)
			mutex.unlock();
	});
	reader.join();
	mutex.unlock_shared();
	ASSERT_TRUE(shared_acquired);
	ASSERT_FALSE(exclusive_acquired);
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_shared_rejected_until_exclusive_unlock() {
	Safe::SharedMutex mutex;
	bool first_entered = false;
	bool second_entered = false;
	mutex.lock();
	std::thread first([&]() {
		first_entered = mutex.try_lock_shared();
		if (first_entered)
			mutex.unlock_shared();
	});
	std::thread second([&]() {
		second_entered = mutex.try_lock_shared();
		if (second_entered)
			mutex.unlock_shared();
	});
	first.join();
	second.join();
	mutex.unlock();
	ASSERT_FALSE(first_entered);
	ASSERT_FALSE(second_entered);
	ASSERT_TRUE(mutex.try_lock_shared());
	mutex.unlock_shared();
	RETURN_TEST(0);
}

// -------------------
// Reuse
// -------------------

int test_repeated_shared_and_exclusive_reuse() {
	Safe::SharedMutex mutex;
	for (int iteration = 0; iteration < 3; ++iteration) {
		bool shared_acquired = false;
		std::thread reader([&]() {
			shared_acquired = mutex.try_lock_shared();
			if (shared_acquired)
				mutex.unlock_shared();
		});
		reader.join();
		bool exclusive_acquired = false;
		std::thread writer([&]() {
			exclusive_acquired = mutex.try_lock();
			if (exclusive_acquired)
				mutex.unlock();
		});
		writer.join();
		ASSERT_TRUE(shared_acquired);
		ASSERT_TRUE(exclusive_acquired);
	}
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
	result += test_exclusive_rejected_until_shared_unlock();

	// -------------------
	// Shared
	// -------------------
	result += test_shared_allows_another_shared();
	result += test_shared_rejected_until_exclusive_unlock();

	// -------------------
	// Reuse
	// -------------------
	result += test_repeated_shared_and_exclusive_reuse();

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
