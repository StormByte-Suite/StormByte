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
#include <vector>

using namespace StormByte;

// -------------------
// Reentry
// -------------------

int test_owner_may_lock_again() {
	int result = 0;
	ThreadLock lock;
	lock.Lock();
	lock.Lock();
	std::atomic<bool> other_acquired(false);
	std::thread t([&]() {
		lock.Lock();
		other_acquired.store(true);
		lock.Unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_FALSE("test_owner_may_lock_again", other_acquired.load());
	lock.Unlock();
	t.join();
	ASSERT_TRUE("test_owner_may_lock_again", other_acquired.load());
	RETURN_TEST("test_owner_may_lock_again", result);
}

// -------------------
// Unlock
// -------------------

int test_unlock_before_lock_is_noop() {
	int result = 0;
	ThreadLock lock;
	lock.Unlock();
	lock.Lock();
	lock.Unlock();
	RETURN_TEST("test_unlock_before_lock_is_noop", result);
}

int test_unlock_from_non_owner_is_noop() {
	int result = 0;
	ThreadLock lock;
	lock.Lock();
	std::atomic<bool> acquired(false);
	std::thread thief([&]() {
		lock.Unlock();
	});
	thief.join();
	std::thread waiter([&]() {
		lock.Lock();
		acquired.store(true);
		lock.Unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_FALSE("test_unlock_from_non_owner_is_noop", acquired.load());
	lock.Unlock();
	waiter.join();
	ASSERT_TRUE("test_unlock_from_non_owner_is_noop", acquired.load());
	RETURN_TEST("test_unlock_from_non_owner_is_noop", result);
}

// -------------------
// Blocking
// -------------------

int test_other_thread_blocks_until_unlock() {
	int result = 0;
	ThreadLock lock;
	lock.Lock();
	std::atomic<bool> acquired(false);
	std::thread t([&]() {
		lock.Lock();
		acquired.store(true);
		lock.Unlock();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_FALSE("test_other_thread_blocks_until_unlock", acquired.load());
	lock.Unlock();
	t.join();
	ASSERT_TRUE("test_other_thread_blocks_until_unlock", acquired.load());
	RETURN_TEST("test_other_thread_blocks_until_unlock", result);
}

// -------------------
// Writers
// -------------------

int test_many_writers_do_not_interleave() {
	int result = 0;
	ThreadLock lock;
	std::string shared;
	const int thread_count = 8;
	static constexpr int iterations = 200;
	static constexpr int token_size = 8;
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
	for (auto& th : threads)
		th.join();
	const std::size_t expected_len = static_cast<std::size_t>(thread_count) * iterations * token_size;
	ASSERT_EQUAL("test_many_writers_do_not_interleave", expected_len, shared.size());
	for (std::size_t pos = 0; pos < shared.size(); pos += static_cast<std::size_t>(token_size)) {
		const char first = shared[pos];
		for (std::size_t k = 1; k < static_cast<std::size_t>(token_size); ++k)
			ASSERT_EQUAL("test_many_writers_do_not_interleave", first, shared[pos + k]);
	}
	RETURN_TEST("test_many_writers_do_not_interleave", result);
}

int main() {
	int result = 0;

	// -------------------
	// Reentry
	// -------------------
	result += test_owner_may_lock_again();

	// -------------------
	// Unlock
	// -------------------
	result += test_unlock_before_lock_is_noop();
	result += test_unlock_from_non_owner_is_noop();

	// -------------------
	// Blocking
	// -------------------
	result += test_other_thread_blocks_until_unlock();

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
