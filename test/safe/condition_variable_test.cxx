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

#include <StormByte/safe/condition_variable.hxx>
#include <StormByte/safe/mutex.hxx>
#include <StormByte/safe/unique_lock.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <type_traits>

using namespace StormByte;

namespace {
	void WaitUntil(const std::atomic<bool>& flag) {
		while (!flag.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

// -------------------
// Notify
// -------------------

int test_notify_all_wakes_every_waiter() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	std::atomic<int> awake(0);
	std::atomic<bool> started(false);
	std::thread first([&]() {
		Safe::UniqueLock lock(mutex);
		started.store(true);
		condition.wait(lock);
		awake.fetch_add(1);
	});
	std::thread second([&]() {
		Safe::UniqueLock lock(mutex);
		condition.wait(lock);
		awake.fetch_add(1);
	});
	WaitUntil(started);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	condition.notify_all();
	first.join();
	second.join();
	ASSERT_EQUAL(2, awake.load());
	RETURN_TEST(0);
}

int test_notify_one_wakes_one_waiter() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	std::atomic<int> awake(0);
	std::atomic<bool> started(false);
	bool release = false;
	std::thread first([&]() {
		Safe::UniqueLock lock(mutex);
		started.store(true);
		condition.wait(lock, [&]() { return release; });
		awake.fetch_add(1);
	});
	std::thread second([&]() {
		Safe::UniqueLock lock(mutex);
		condition.wait(lock, [&]() { return release; });
		awake.fetch_add(1);
	});
	WaitUntil(started);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	{
		Safe::UniqueLock lock(mutex);
		release = true;
	}
	condition.notify_one();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_EQUAL(1, awake.load());
	condition.notify_one();
	first.join();
	second.join();
	ASSERT_EQUAL(2, awake.load());
	RETURN_TEST(0);
}

// -------------------
// Predicate
// -------------------

int test_wait_for_predicate_succeeds() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	bool ready = false;
	std::atomic<bool> finished(false);
	std::thread waiter([&]() {
		Safe::UniqueLock lock(mutex);
		const bool ok = condition.wait_for(lock, std::chrono::seconds(2), [&]() { return ready; });
		finished.store(ok);
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	{
		Safe::UniqueLock lock(mutex);
		ready = true;
	}
	condition.notify_one();
	waiter.join();
	ASSERT_TRUE(finished.load());
	RETURN_TEST(0);
}

int test_wait_predicate_filters_early_wake() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	bool ready = false;
	std::atomic<bool> finished(false);
	std::thread waiter([&]() {
		Safe::UniqueLock lock(mutex);
		condition.wait(lock, [&]() { return ready; });
		finished.store(true);
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	condition.notify_all();
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	ASSERT_FALSE(finished.load());
	{
		Safe::UniqueLock lock(mutex);
		ready = true;
	}
	condition.notify_all();
	waiter.join();
	ASSERT_TRUE(finished.load());
	RETURN_TEST(0);
}

int test_wait_until_predicate_succeeds() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	bool ready = false;
	std::atomic<bool> finished(false);
	std::thread waiter([&]() {
		Safe::UniqueLock lock(mutex);
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		finished.store(condition.wait_until(lock, deadline, [&]() { return ready; }));
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(30));
	{
		Safe::UniqueLock lock(mutex);
		ready = true;
	}
	condition.notify_one();
	waiter.join();
	ASSERT_TRUE(finished.load());
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_not_copyable_or_movable() {
	static_assert(!std::is_copy_constructible_v<Safe::ConditionVariable>);
	static_assert(!std::is_copy_assignable_v<Safe::ConditionVariable>);
	static_assert(!std::is_move_constructible_v<Safe::ConditionVariable>);
	static_assert(!std::is_move_assignable_v<Safe::ConditionVariable>);
	RETURN_TEST(0);
}

// -------------------
// Timeout
// -------------------

int test_wait_for_predicate_times_out() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	Safe::UniqueLock lock(mutex);
	const bool ready = condition.wait_for(lock, std::chrono::milliseconds(40), []() { return false; });
	ASSERT_FALSE(ready);
	ASSERT_TRUE(lock.owns_lock());
	RETURN_TEST(0);
}

int test_wait_for_times_out() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	Safe::UniqueLock lock(mutex);
	const auto status = condition.wait_for(lock, std::chrono::milliseconds(40));
	ASSERT_TRUE(status == Safe::CvStatus::Timeout);
	ASSERT_TRUE(lock.owns_lock());
	RETURN_TEST(0);
}

int test_wait_until_past_deadline_times_out() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	Safe::UniqueLock lock(mutex);
	const auto deadline = std::chrono::steady_clock::now() - std::chrono::seconds(1);
	const auto status = condition.wait_until(lock, deadline);
	ASSERT_TRUE(status == Safe::CvStatus::Timeout);
	ASSERT_TRUE(lock.owns_lock());
	RETURN_TEST(0);
}

int test_wait_until_predicate_times_out() {
	Safe::Mutex mutex;
	Safe::ConditionVariable condition;
	Safe::UniqueLock lock(mutex);
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(40);
	const bool ready = condition.wait_until(lock, deadline, []() { return false; });
	ASSERT_FALSE(ready);
	ASSERT_TRUE(lock.owns_lock());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Notify
	// -------------------
	result += test_notify_all_wakes_every_waiter();
	result += test_notify_one_wakes_one_waiter();

	// -------------------
	// Predicate
	// -------------------
	result += test_wait_for_predicate_succeeds();
	result += test_wait_predicate_filters_early_wake();
	result += test_wait_until_predicate_succeeds();

	// -------------------
	// Special
	// -------------------
	result += test_not_copyable_or_movable();

	// -------------------
	// Timeout
	// -------------------
	result += test_wait_for_predicate_times_out();
	result += test_wait_for_times_out();
	result += test_wait_until_past_deadline_times_out();
	result += test_wait_until_predicate_times_out();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
