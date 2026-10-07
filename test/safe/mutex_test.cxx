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

#include <StormByte/safe/mutex.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

using namespace StormByte;

// -------------------
// Blocking
// -------------------

int test_other_thread_blocks_until_unlock() {
	Safe::Mutex mutex;
	mutex.lock();
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		mutex.lock();
		acquired.store(true);
		mutex.unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_FALSE(acquired.load());
	mutex.unlock();
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

int test_relock_after_unlock() {
	Safe::Mutex mutex;
	mutex.lock();
	mutex.unlock();
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		mutex.lock();
		acquired.store(true);
		mutex.unlock();
	});
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_not_copyable_or_movable() {
	static_assert(!std::is_copy_constructible_v<Safe::Mutex>);
	static_assert(!std::is_copy_assignable_v<Safe::Mutex>);
	static_assert(!std::is_move_constructible_v<Safe::Mutex>);
	static_assert(!std::is_move_assignable_v<Safe::Mutex>);
	RETURN_TEST(0);
}

// -------------------
// Try
// -------------------

int test_try_lock_fails_when_owned() {
	Safe::Mutex mutex;
	mutex.lock();
	std::atomic<bool> failed(false);
	std::thread contender([&]() {
		failed.store(!mutex.try_lock());
	});
	contender.join();
	ASSERT_TRUE(failed.load());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_try_lock_succeeds_when_free() {
	Safe::Mutex mutex;
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_try_lock_then_other_thread_acquires() {
	Safe::Mutex mutex;
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		const bool taken = mutex.try_lock();
		if (taken)
			mutex.unlock();
		acquired.store(taken);
	});
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

// -------------------
// Writers
// -------------------

int test_many_writers_do_not_interleave() {
	Safe::Mutex mutex;
	std::string shared;
	constexpr int thread_count = 8;
	constexpr int iterations = 200;
	constexpr int token_size = 8;
	std::vector<std::thread> threads;
	for (int t = 0; t < thread_count; ++t) {
		threads.emplace_back([t, &mutex, &shared]() {
			const char c = static_cast<char>('A' + (t % 26));
			const std::string token(static_cast<std::size_t>(token_size), c);
			for (int i = 0; i < iterations; ++i) {
				mutex.lock();
				shared.append(token);
				mutex.unlock();
			}
		});
	}
	for (auto& thread : threads)
		thread.join();
	const std::size_t expected_len = static_cast<std::size_t>(thread_count) * iterations * token_size;
	ASSERT_EQUAL(expected_len, shared.size());
	for (std::size_t pos = 0; pos < shared.size(); pos += static_cast<std::size_t>(token_size)) {
		const char first = shared[pos];
		for (std::size_t k = 1; k < static_cast<std::size_t>(token_size); ++k)
			ASSERT_EQUAL(first, shared[pos + k]);
	}
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Blocking
	// -------------------
	result += test_other_thread_blocks_until_unlock();
	result += test_relock_after_unlock();

	// -------------------
	// Special
	// -------------------
	result += test_not_copyable_or_movable();

	// -------------------
	// Try
	// -------------------
	result += test_try_lock_fails_when_owned();
	result += test_try_lock_succeeds_when_free();
	result += test_try_lock_then_other_thread_acquires();

	// -------------------
	// Writers
	// -------------------
	result += test_many_writers_do_not_interleave();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
