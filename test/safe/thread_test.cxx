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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/function.hxx>
#include <StormByte/safe/thread.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <type_traits>

using namespace StormByte;

namespace {
	struct Counter {
		std::atomic<int>* value;
		std::atomic<int>* released;
		Safe::Status status;
	};

	struct Delta {
		std::atomic<int>* value;
	};

	void Touch(std::atomic<int>* value) {
		value->fetch_add(1);
	}

	void Add(std::atomic<int>* value, int delta) {
		value->fetch_add(delta);
	}

	void* CloneCounter(const void* raw) noexcept {
		auto* source = static_cast<const Counter*>(raw);
		return new Counter{source->value, source->released, source->status};
	}

	void ReleaseCounter(void* raw) noexcept {
		auto* context = static_cast<Counter*>(raw);
		if (context->released != nullptr)
			context->released->fetch_add(1);
		delete context;
	}

	Safe::Status InvokeCounter(void* raw) {
		auto* context = static_cast<Counter*>(raw);
		if (context->value != nullptr)
			context->value->fetch_add(1);
		return context->status;
	}

	Safe::Status InvokeThrow(void*) {
		throw Safe::Exception("from the thread");
	}

	void* CloneDelta(const void* raw) noexcept {
		auto* source = static_cast<const Delta*>(raw);
		return new Delta{source->value};
	}

	void ReleaseDelta(void* raw) noexcept {
		delete static_cast<Delta*>(raw);
	}

	Safe::Status InvokeDelta(void* raw, Size delta) {
		static_cast<Delta*>(raw)->value->fetch_add(static_cast<int>(static_cast<std::size_t>(delta)));
		return Safe::Status::Success;
	}

	Safe::Function<void()> MakeCounter(std::atomic<int>* value, std::atomic<int>* released, Safe::Status status) {
		return Safe::Function<void()>(new Counter{value, released, status}, InvokeCounter, CloneCounter, ReleaseCounter);
	}
}

// -------------------
// Detach
// -------------------

int test_detach_drops_joinable() {
	std::atomic<int> value(0);
	Safe::Thread thread(Touch, &value);
	ASSERT_TRUE(thread.joinable());
	thread.detach();
	ASSERT_FALSE(thread.joinable());
	ASSERT_TRUE(thread.get_id() == Safe::Thread::Id{});
	ASSERT_EQUAL(static_cast<Safe::Thread::native_handle_type>(0), thread.native_handle());
	bool threw = false;
	try {
		thread.join();
	}
	catch (const Safe::Exception&) {
		threw = true;
	}
	ASSERT_TRUE(threw);
	RETURN_TEST(0);
}

int test_detach_of_empty_throws() {
	Safe::Thread thread;
	bool threw = false;
	try {
		thread.detach();
	}
	catch (const Safe::Exception&) {
		threw = true;
	}
	ASSERT_TRUE(threw);
	RETURN_TEST(0);
}

// -------------------
// Function
// -------------------

int test_function_argument_reaches_call() {
	std::atomic<int> value(0);
	Safe::Function<void(Size)> function(new Delta{&value}, InvokeDelta, CloneDelta, ReleaseDelta);
	Safe::Thread thread(std::move(function), Size{5});
	thread.join();
	ASSERT_EQUAL(5, value.load());
	RETURN_TEST(0);
}

int test_function_base_exception_does_not_terminate() {
	std::atomic<int> released(0);
	Safe::Function<void()> function(new Counter{nullptr, &released, Safe::Status::Success}, InvokeThrow, CloneCounter, ReleaseCounter);
	Safe::Thread thread(std::move(function));
	thread.join();
	ASSERT_FALSE(thread.joinable());
	ASSERT_TRUE(released.load() >= 1);
	RETURN_TEST(0);
}

int test_function_call_runs_on_the_thread() {
	std::atomic<int> value(0);
	std::atomic<int> released(0);
	Safe::Function<void()> function = MakeCounter(&value, &released, Safe::Status::Success);
	Safe::Thread thread(std::move(function));
	ASSERT_FALSE(function.HasValue());
	ASSERT_TRUE(thread.joinable());
	thread.join();
	ASSERT_EQUAL(1, value.load());
	ASSERT_TRUE(released.load() >= 1);
	RETURN_TEST(0);
}

int test_function_failure_does_not_terminate() {
	std::atomic<int> value(0);
	std::atomic<int> released(0);
	Safe::Thread thread(MakeCounter(&value, &released, Safe::Status::Failure));
	thread.join();
	ASSERT_EQUAL(1, value.load());
	ASSERT_FALSE(thread.joinable());
	RETURN_TEST(0);
}

int test_missing_function_does_not_terminate() {
	std::atomic<int> released(0);
	Safe::Function<void()> function = MakeCounter(nullptr, &released, Safe::Status::Success);
	Safe::Function<void()> kept(std::move(function));
	ASSERT_FALSE(function.HasValue());
	Safe::Thread thread(std::move(function));
	thread.join();
	ASSERT_FALSE(thread.joinable());
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Success), static_cast<int>(kept.Call()));
	RETURN_TEST(0);
}

// -------------------
// Id
// -------------------

int test_id_orders_and_compares() {
	Safe::Thread::Id empty;
	ASSERT_EQUAL(static_cast<std::uint64_t>(0), empty.Value());
	std::atomic<Safe::Thread::Id> seen;
	Safe::Thread thread([&seen]() {
		seen.store(Safe::this_thread::get_id());
	});
	const Safe::Thread::Id owned = thread.get_id();
	thread.join();
	const Safe::Thread::Id inside = seen.load();
	ASSERT_TRUE(owned == inside);
	ASSERT_FALSE(owned != inside);
	ASSERT_FALSE(owned == empty);
	ASSERT_TRUE(owned != empty);
	ASSERT_TRUE((owned < empty) || (empty < owned));
	ASSERT_TRUE(owned <= owned);
	ASSERT_TRUE(owned >= owned);
	ASSERT_FALSE(owned < owned);
	ASSERT_FALSE(owned > owned);
	RETURN_TEST(0);
}

int test_two_threads_have_different_ids() {
	Safe::Thread::Id first_id;
	Safe::Thread::Id second_id;
	Safe::Thread first([&first_id]() {
		first_id = Safe::this_thread::get_id();
	});
	Safe::Thread second([&second_id]() {
		second_id = Safe::this_thread::get_id();
	});
	const Safe::Thread::Id first_owned = first.get_id();
	const Safe::Thread::Id second_owned = second.get_id();
	first.join();
	second.join();
	ASSERT_TRUE(first_owned == first_id);
	ASSERT_TRUE(second_owned == second_id);
	ASSERT_TRUE(first_owned != second_owned);
	RETURN_TEST(0);
}

// -------------------
// Join
// -------------------

int test_join_of_empty_throws() {
	Safe::Thread thread;
	bool threw = false;
	try {
		thread.join();
	}
	catch (const Safe::Exception&) {
		threw = true;
	}
	ASSERT_TRUE(threw);
	RETURN_TEST(0);
}

int test_join_runs_callable_and_second_join_throws() {
	std::atomic<int> value(0);
	Safe::Thread thread(Touch, &value);
	ASSERT_TRUE(thread.joinable());
	ASSERT_TRUE(thread.get_id() != Safe::Thread::Id{});
	ASSERT_TRUE(thread.native_handle() != 0);
	thread.join();
	ASSERT_EQUAL(1, value.load());
	ASSERT_FALSE(thread.joinable());
	ASSERT_TRUE(thread.get_id() == Safe::Thread::Id{});
	ASSERT_EQUAL(static_cast<Safe::Thread::native_handle_type>(0), thread.native_handle());
	bool threw = false;
	try {
		thread.join();
	}
	catch (const Safe::Exception&) {
		threw = true;
	}
	ASSERT_TRUE(threw);
	RETURN_TEST(0);
}

int test_join_sees_arguments() {
	std::atomic<int> value(1);
	Safe::Thread thread(Add, &value, 41);
	thread.join();
	ASSERT_EQUAL(42, value.load());
	RETURN_TEST(0);
}

// -------------------
// Life
// -------------------

int test_default_is_not_joinable() {
	Safe::Thread thread;
	ASSERT_FALSE(thread.joinable());
	ASSERT_TRUE(thread.get_id() == Safe::Thread::Id{});
	ASSERT_EQUAL(static_cast<Safe::Thread::native_handle_type>(0), thread.native_handle());
	RETURN_TEST(0);
}

int test_hardware_concurrency_is_callable() {
	const unsigned count = Safe::Thread::hardware_concurrency();
	ASSERT_TRUE(count == Safe::Thread::hardware_concurrency());
	(void)count;
	RETURN_TEST(0);
}

int test_lambda_runs() {
	std::atomic<int> value(0);
	Safe::Thread thread([&value]() {
		value.store(7);
	});
	thread.join();
	ASSERT_EQUAL(7, value.load());
	RETURN_TEST(0);
}

int test_move_assign_transfers() {
	std::atomic<int> value(0);
	Safe::Thread source(Touch, &value);
	Safe::Thread destination;
	destination = std::move(source);
	ASSERT_FALSE(source.joinable());
	ASSERT_TRUE(destination.joinable());
	destination.join();
	ASSERT_EQUAL(1, value.load());
	RETURN_TEST(0);
}

int test_move_construct_transfers() {
	std::atomic<int> value(0);
	Safe::Thread source(Touch, &value);
	Safe::Thread destination(std::move(source));
	ASSERT_FALSE(source.joinable());
	ASSERT_TRUE(destination.joinable());
	destination.join();
	ASSERT_EQUAL(1, value.load());
	RETURN_TEST(0);
}

int test_not_copyable_and_is_movable() {
	static_assert(!std::is_copy_constructible_v<Safe::Thread>);
	static_assert(!std::is_copy_assignable_v<Safe::Thread>);
	static_assert(std::is_move_constructible_v<Safe::Thread>);
	static_assert(std::is_move_assignable_v<Safe::Thread>);
	RETURN_TEST(0);
}

int test_several_threads_all_join() {
	constexpr int count = 8;
	std::atomic<int> value(0);
	Safe::Thread threads[count];
	for (int i = 0; i < count; ++i)
		threads[i] = Safe::Thread(Touch, &value);
	for (int i = 0; i < count; ++i)
		threads[i].join();
	ASSERT_EQUAL(count, value.load());
	RETURN_TEST(0);
}

int test_swap_exchanges_joinable() {
	std::atomic<int> left_value(0);
	std::atomic<int> right_value(0);
	Safe::Thread left(Add, &left_value, 1);
	Safe::Thread right(Add, &right_value, 2);
	const Safe::Thread::Id left_id = left.get_id();
	left.swap(right);
	ASSERT_TRUE(right.get_id() == left_id);
	swap(left, right);
	ASSERT_TRUE(left.get_id() == left_id);
	left.join();
	right.join();
	ASSERT_EQUAL(1, left_value.load());
	ASSERT_EQUAL(2, right_value.load());
	RETURN_TEST(0);
}

// -------------------
// This thread
// -------------------

int test_calling_matches_this_thread() {
	ASSERT_TRUE(Safe::Thread::Calling() == Safe::this_thread::get_id());
	RETURN_TEST(0);
}

int test_sleep_for_negative_returns() {
	const auto start = std::chrono::steady_clock::now();
	Safe::this_thread::sleep_for(std::chrono::milliseconds(-1));
	const auto elapsed = std::chrono::steady_clock::now() - start;
	ASSERT_TRUE(elapsed < std::chrono::milliseconds(50));
	RETURN_TEST(0);
}

int test_sleep_for_waits() {
	const auto start = std::chrono::steady_clock::now();
	Safe::this_thread::sleep_for(std::chrono::milliseconds(20));
	const auto elapsed = std::chrono::steady_clock::now() - start;
	ASSERT_TRUE(elapsed >= std::chrono::milliseconds(10));
	RETURN_TEST(0);
}

int test_sleep_until_waits() {
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
	Safe::this_thread::sleep_until(deadline);
	ASSERT_TRUE(std::chrono::steady_clock::now() >= deadline - std::chrono::milliseconds(5));
	RETURN_TEST(0);
}

int test_this_thread_id_is_stable_and_nonzero() {
	const Safe::Thread::Id first = Safe::this_thread::get_id();
	const Safe::Thread::Id second = Safe::this_thread::get_id();
	ASSERT_TRUE(first == second);
	ASSERT_TRUE(first != Safe::Thread::Id{});
	ASSERT_TRUE(first.Value() != 0);
	RETURN_TEST(0);
}

int test_yield_returns() {
	Safe::this_thread::yield();
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Detach
	// -------------------
	result += test_detach_drops_joinable();
	result += test_detach_of_empty_throws();

	// -------------------
	// Function
	// -------------------
	result += test_function_argument_reaches_call();
	result += test_function_base_exception_does_not_terminate();
	result += test_function_call_runs_on_the_thread();
	result += test_function_failure_does_not_terminate();
	result += test_missing_function_does_not_terminate();

	// -------------------
	// Id
	// -------------------
	result += test_id_orders_and_compares();
	result += test_two_threads_have_different_ids();

	// -------------------
	// Join
	// -------------------
	result += test_join_of_empty_throws();
	result += test_join_runs_callable_and_second_join_throws();
	result += test_join_sees_arguments();

	// -------------------
	// Life
	// -------------------
	result += test_default_is_not_joinable();
	result += test_hardware_concurrency_is_callable();
	result += test_lambda_runs();
	result += test_move_assign_transfers();
	result += test_move_construct_transfers();
	result += test_not_copyable_and_is_movable();
	result += test_several_threads_all_join();
	result += test_swap_exchanges_joinable();

	// -------------------
	// This thread
	// -------------------
	result += test_calling_matches_this_thread();
	result += test_sleep_for_negative_returns();
	result += test_sleep_for_waits();
	result += test_sleep_until_waits();
	result += test_this_thread_id_is_stable_and_nonzero();
	result += test_yield_returns();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
