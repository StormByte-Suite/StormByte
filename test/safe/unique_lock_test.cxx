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

#include <StormByte/safe/mutex.hxx>
#include <StormByte/safe/unique_lock.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <type_traits>

using namespace StormByte;

// -------------------
// Adopt
// -------------------

int test_adopt_does_not_lock_again() {
	Safe::Mutex mutex;
	mutex.lock();
	Safe::UniqueLock lock(mutex, Safe::adopt_lock);
	ASSERT_TRUE(lock.owns_lock());
	std::atomic<bool> acquired(false);
	std::thread waiter([&]() {
		acquired.store(mutex.try_lock());
		if (acquired.load())
			mutex.unlock();
	});
	waiter.join();
	ASSERT_FALSE(acquired.load());
	lock.unlock();
	RETURN_TEST(0);
}

// -------------------
// Defer
// -------------------

int test_defer_does_not_own() {
	Safe::Mutex mutex;
	Safe::UniqueLock lock(mutex, Safe::defer_lock);
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_TRUE(static_cast<bool>(lock.mutex() == &mutex));
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_defer_then_lock() {
	Safe::Mutex mutex;
	Safe::UniqueLock lock(mutex, Safe::defer_lock);
	lock.lock();
	ASSERT_TRUE(lock.owns_lock());
	ASSERT_FALSE(mutex.try_lock());
	lock.unlock();
	ASSERT_FALSE(lock.owns_lock());
	RETURN_TEST(0);
}

// -------------------
// Destructor
// -------------------

int test_destructor_unlocks() {
	Safe::Mutex mutex;
	{
		Safe::UniqueLock lock(mutex);
		ASSERT_TRUE(lock.owns_lock());
	}
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_destructor_of_deferred_does_not_unlock() {
	Safe::Mutex mutex;
	mutex.lock();
	{
		Safe::UniqueLock lock(mutex, Safe::defer_lock);
	}
	ASSERT_FALSE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

// -------------------
// Empty
// -------------------

int test_empty_lock_owns_nothing() {
	Safe::UniqueLock lock;
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_FALSE(static_cast<bool>(lock));
	ASSERT_TRUE(lock.mutex() == nullptr);
	ASSERT_FALSE(lock.try_lock());
	RETURN_TEST(0);
}

// -------------------
// Move
// -------------------

int test_move_assign_unlocks_previous() {
	Safe::Mutex first;
	Safe::Mutex second;
	Safe::UniqueLock lock(first);
	Safe::UniqueLock other(second);
	lock = std::move(other);
	ASSERT_TRUE(lock.owns_lock());
	ASSERT_TRUE(lock.mutex() == &second);
	ASSERT_FALSE(other.owns_lock());
	ASSERT_TRUE(first.try_lock());
	first.unlock();
	lock.unlock();
	RETURN_TEST(0);
}

int test_move_construct_transfers_ownership() {
	Safe::Mutex mutex;
	Safe::UniqueLock source(mutex);
	Safe::UniqueLock lock(std::move(source));
	ASSERT_TRUE(lock.owns_lock());
	ASSERT_FALSE(source.owns_lock());
	ASSERT_TRUE(source.mutex() == nullptr);
	ASSERT_FALSE(mutex.try_lock());
	lock.unlock();
	RETURN_TEST(0);
}

// -------------------
// Release
// -------------------

int test_release_drops_association_without_unlock() {
	Safe::Mutex mutex;
	Safe::UniqueLock lock(mutex);
	Safe::Mutex* released = lock.release();
	ASSERT_TRUE(released == &mutex);
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_TRUE(lock.mutex() == nullptr);
	ASSERT_FALSE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_not_copyable() {
	static_assert(!std::is_copy_constructible_v<Safe::UniqueLock>);
	static_assert(!std::is_copy_assignable_v<Safe::UniqueLock>);
	static_assert(std::is_move_constructible_v<Safe::UniqueLock>);
	static_assert(std::is_move_assignable_v<Safe::UniqueLock>);
	RETURN_TEST(0);
}

// -------------------
// Swap
// -------------------

int test_free_swap_exchanges_locks() {
	Safe::Mutex first;
	Safe::Mutex second;
	Safe::UniqueLock left(first);
	Safe::UniqueLock right(second, Safe::defer_lock);
	swap(left, right);
	ASSERT_FALSE(left.owns_lock());
	ASSERT_TRUE(left.mutex() == &second);
	ASSERT_TRUE(right.owns_lock());
	ASSERT_TRUE(right.mutex() == &first);
	right.unlock();
	RETURN_TEST(0);
}

int test_member_swap_exchanges_locks() {
	Safe::Mutex first;
	Safe::Mutex second;
	Safe::UniqueLock left(first, Safe::defer_lock);
	Safe::UniqueLock right(second);
	left.swap(right);
	ASSERT_TRUE(left.owns_lock());
	ASSERT_TRUE(left.mutex() == &second);
	ASSERT_FALSE(right.owns_lock());
	ASSERT_TRUE(right.mutex() == &first);
	left.unlock();
	RETURN_TEST(0);
}

// -------------------
// Try
// -------------------

int test_try_to_lock_fails_when_owned() {
	Safe::Mutex mutex;
	mutex.lock();
	Safe::UniqueLock lock(mutex, Safe::try_to_lock);
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_TRUE(lock.mutex() == &mutex);
	mutex.unlock();
	RETURN_TEST(0);
}

int test_try_to_lock_succeeds_when_free() {
	Safe::Mutex mutex;
	Safe::UniqueLock lock(mutex, Safe::try_to_lock);
	ASSERT_TRUE(lock.owns_lock());
	ASSERT_TRUE(static_cast<bool>(lock));
	lock.unlock();
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Adopt
	// -------------------
	result += test_adopt_does_not_lock_again();

	// -------------------
	// Defer
	// -------------------
	result += test_defer_does_not_own();
	result += test_defer_then_lock();

	// -------------------
	// Destructor
	// -------------------
	result += test_destructor_of_deferred_does_not_unlock();
	result += test_destructor_unlocks();

	// -------------------
	// Empty
	// -------------------
	result += test_empty_lock_owns_nothing();

	// -------------------
	// Move
	// -------------------
	result += test_move_assign_unlocks_previous();
	result += test_move_construct_transfers_ownership();

	// -------------------
	// Release
	// -------------------
	result += test_release_drops_association_without_unlock();

	// -------------------
	// Special
	// -------------------
	result += test_not_copyable();

	// -------------------
	// Swap
	// -------------------
	result += test_free_swap_exchanges_locks();
	result += test_member_swap_exchanges_locks();

	// -------------------
	// Try
	// -------------------
	result += test_try_to_lock_fails_when_owned();
	result += test_try_to_lock_succeeds_when_free();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
