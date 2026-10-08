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

#include <StormByte/safe/exclusive_lock.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <type_traits>
#include <utility>

using namespace StormByte;

// -------------------
// Construct
// -------------------

int test_adopt_assumes_exclusive_hold() {
	Safe::SharedMutex mutex;
	mutex.lock();
	Safe::ExclusiveLock lock(mutex, Safe::AdoptLock{});
	ASSERT_TRUE(lock.owns_lock());
	ASSERT_EQUAL(&mutex, lock.mutex());
	lock.unlock();
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_TRUE(mutex.try_lock_shared());
	mutex.unlock_shared();
	RETURN_TEST(0);
}

int test_defer_does_not_lock() {
	Safe::SharedMutex mutex;
	Safe::ExclusiveLock lock(mutex, Safe::DeferLock{});
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_EQUAL(&mutex, lock.mutex());
	ASSERT_TRUE(mutex.try_lock_shared());
	mutex.unlock_shared();
	lock.lock();
	ASSERT_TRUE(lock.owns_lock());
	lock.unlock();
	ASSERT_FALSE(lock.owns_lock());
	RETURN_TEST(0);
}

int test_empty_owns_nothing() {
	Safe::ExclusiveLock lock;
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_FALSE(static_cast<bool>(lock));
	ASSERT_EQUAL(static_cast<Safe::SharedMutex*>(nullptr), lock.mutex());
	ASSERT_FALSE(lock.try_lock());
	ASSERT_EQUAL(static_cast<Safe::SharedMutex*>(nullptr), lock.release());
	RETURN_TEST(0);
}

int test_lock_blocks_shared_and_exclusive() {
	Safe::SharedMutex mutex;
	{
		Safe::ExclusiveLock lock(mutex);
		ASSERT_TRUE(lock.owns_lock());
		ASSERT_TRUE(static_cast<bool>(lock));
		ASSERT_EQUAL(&mutex, lock.mutex());
		ASSERT_FALSE(mutex.try_lock());
		ASSERT_FALSE(mutex.try_lock_shared());
	}
	ASSERT_TRUE(mutex.try_lock_shared());
	mutex.unlock_shared();
	RETURN_TEST(0);
}

int test_try_fails_while_shared() {
	Safe::SharedMutex mutex;
	mutex.lock_shared();
	Safe::ExclusiveLock lock(mutex, Safe::TryToLock{});
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_EQUAL(&mutex, lock.mutex());
	ASSERT_FALSE(lock.try_lock());
	mutex.unlock_shared();
	ASSERT_TRUE(lock.try_lock());
	ASSERT_TRUE(lock.owns_lock());
	RETURN_TEST(0);
}

// -------------------
// Move
// -------------------

int test_move_assign_releases_previous() {
	Safe::SharedMutex first;
	Safe::SharedMutex second;
	Safe::ExclusiveLock source(first);
	Safe::ExclusiveLock other(second);
	source = std::move(other);
	ASSERT_FALSE(other.owns_lock());
	ASSERT_TRUE(source.owns_lock());
	ASSERT_EQUAL(&second, source.mutex());
	ASSERT_TRUE(first.try_lock_shared());
	first.unlock_shared();
	RETURN_TEST(0);
}

int test_move_transfers_ownership() {
	Safe::SharedMutex mutex;
	Safe::ExclusiveLock source(mutex);
	Safe::ExclusiveLock moved(std::move(source));
	ASSERT_FALSE(source.owns_lock());
	ASSERT_EQUAL(static_cast<Safe::SharedMutex*>(nullptr), source.mutex());
	ASSERT_TRUE(moved.owns_lock());
	ASSERT_EQUAL(&mutex, moved.mutex());
	RETURN_TEST(0);
}

int test_release_keeps_exclusive_hold() {
	Safe::SharedMutex mutex;
	Safe::ExclusiveLock lock(mutex);
	Safe::SharedMutex* raw = lock.release();
	ASSERT_EQUAL(&mutex, raw);
	ASSERT_FALSE(lock.owns_lock());
	ASSERT_FALSE(mutex.try_lock_shared());
	mutex.unlock();
	ASSERT_TRUE(mutex.try_lock());
	mutex.unlock();
	RETURN_TEST(0);
}

int test_swap_exchanges_locks() {
	Safe::SharedMutex first;
	Safe::SharedMutex second;
	Safe::ExclusiveLock left(first);
	Safe::ExclusiveLock right(second, Safe::DeferLock{});
	left.swap(right);
	ASSERT_FALSE(left.owns_lock());
	ASSERT_EQUAL(&second, left.mutex());
	ASSERT_TRUE(right.owns_lock());
	ASSERT_EQUAL(&first, right.mutex());
	swap(left, right);
	ASSERT_TRUE(left.owns_lock());
	ASSERT_EQUAL(&first, left.mutex());
	ASSERT_FALSE(right.owns_lock());
	RETURN_TEST(0);
}

// -------------------
// Special
// -------------------

int test_not_copyable() {
	static_assert(!std::is_copy_constructible_v<Safe::ExclusiveLock>);
	static_assert(!std::is_copy_assignable_v<Safe::ExclusiveLock>);
	static_assert(std::is_move_constructible_v<Safe::ExclusiveLock>);
	static_assert(std::is_move_assignable_v<Safe::ExclusiveLock>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Construct
	// -------------------
	result += test_adopt_assumes_exclusive_hold();
	result += test_defer_does_not_lock();
	result += test_empty_owns_nothing();
	result += test_lock_blocks_shared_and_exclusive();
	result += test_try_fails_while_shared();

	// -------------------
	// Move
	// -------------------
	result += test_move_assign_releases_previous();
	result += test_move_transfers_ownership();
	result += test_release_keeps_exclusive_hold();
	result += test_swap_exchanges_locks();

	// -------------------
	// Special
	// -------------------
	result += test_not_copyable();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
