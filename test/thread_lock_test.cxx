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

#include <StormByte/test_handlers.h>
#include <StormByte/thread_lock.hxx>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

using namespace StormByte;

namespace {
	void WaitUntil(const std::atomic<bool>& flag) {
		while (!flag.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

// -------------------
// Blocking
// -------------------

int test_other_thread_blocks_until_unlock() {
	ThreadLock lock;
	lock.Lock();
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		lock.Lock();
		acquired.store(true);
		lock.Unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_FALSE(acquired.load());
	lock.Unlock();
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

int test_relock_after_unlock() {
	ThreadLock lock;
	lock.Lock();
	lock.Unlock();
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		lock.Lock();
		acquired.store(true);
		lock.Unlock();
	});
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

// -------------------
// Reentry
// -------------------

int test_owner_reentry_is_not_counted() {
	ThreadLock lock;
	lock.Lock();
	lock.Lock();
	std::atomic<bool> started(false);
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		started.store(true);
		lock.Lock();
		acquired.store(true);
		lock.Unlock();
	});
	WaitUntil(started);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_FALSE(acquired.load());
	lock.Unlock();
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

int test_owner_reentry_does_not_block_self() {
	ThreadLock lock;
	ASSERT_NO_THROW(lock.Lock());
	ASSERT_NO_THROW(lock.Lock());
	ASSERT_NO_THROW(lock.Lock());
	lock.Unlock();
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_not_copyable_or_movable() {
	static_assert(!std::is_copy_constructible_v<ThreadLock>);
	static_assert(!std::is_copy_assignable_v<ThreadLock>);
	static_assert(!std::is_move_constructible_v<ThreadLock>);
	static_assert(!std::is_move_assignable_v<ThreadLock>);
	static_assert(std::is_nothrow_default_constructible_v<ThreadLock>);
	RETURN_TEST(0);
}

// -------------------
// Unlock
// -------------------

int test_double_unlock_is_noop() {
	ThreadLock lock;
	lock.Lock();
	lock.Unlock();
	ASSERT_NO_THROW(lock.Unlock());
	lock.Lock();
	lock.Unlock();
	RETURN_TEST(0);
}

int test_unlock_before_lock_is_noop() {
	ThreadLock lock;
	ASSERT_NO_THROW(lock.Unlock());
	lock.Lock();
	lock.Unlock();
	RETURN_TEST(0);
}

int test_unlock_from_non_owner_is_noop() {
	ThreadLock lock;
	lock.Lock();
	std::thread thief([&]() {
		lock.Unlock();
	});
	thief.join();
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		lock.Lock();
		acquired.store(true);
		lock.Unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_FALSE(acquired.load());
	lock.Unlock();
	waiter.join();
	ASSERT_TRUE(acquired.load());
	RETURN_TEST(0);
}

// -------------------
// Writers
// -------------------

int test_many_writers_do_not_interleave() {
	ThreadLock lock;
	std::string shared;
	constexpr int thread_count = 8;
	constexpr int iterations = 200;
	constexpr int token_size = 8;
	std::vector<std::thread> threads;
	for (int t = 0; t < thread_count; ++t) {
		threads.emplace_back([t, &lock, &shared]() {
			const char c = static_cast<char>('A' + (t % 26));
			const std::string token(static_cast<std::size_t>(token_size), c);
			for (int i = 0; i < iterations; ++i) {
				lock.Lock();
				shared.append(token);
				lock.Unlock();
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
	// Reentry
	// -------------------
	result += test_owner_reentry_does_not_block_self();
	result += test_owner_reentry_is_not_counted();

	// -------------------
	// Special
	// -------------------
	result += test_not_copyable_or_movable();

	// -------------------
	// Unlock
	// -------------------
	result += test_double_unlock_is_noop();
	result += test_unlock_before_lock_is_noop();
	result += test_unlock_from_non_owner_is_noop();

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
