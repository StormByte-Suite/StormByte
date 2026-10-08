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

#include <StormByte/exception.hxx>
#include <StormByte/safe/function.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <stdexcept>
#include <utility>

using namespace StormByte;

namespace {
	struct Context {
		int value;
		static int live;
		explicit Context(int n): value(n) { ++live; }
		~Context() { --live; }
	};
	int Context::live = 0;

	void* CloneContext(const void* pointer) noexcept {
		return new Context(static_cast<const Context*>(pointer)->value);
	}

	void ReleaseContext(void* pointer) noexcept {
		delete static_cast<Context*>(pointer);
	}

	Safe::Status Invoke(void* pointer, int argument) {
		static_cast<Context*>(pointer)->value += argument;
		return Safe::Status::Success;
	}

	Safe::Status Fail(void*, int) {
		throw std::runtime_error("foreign");
	}

	Safe::Status Escape(void*, int) {
		throw StormByte::Exception("safe");
	}

	Safe::Status Produce(void* pointer, int* output, int argument) {
		*output = static_cast<Context*>(pointer)->value + argument;
		return Safe::Status::Success;
	}
}

// -------------------
// Life
// -------------------

int test_reject_copy_move_and_call() {
	using Callback = Safe::Function<void(int)>;
	ASSERT_THROWS(Callback(nullptr, Invoke, CloneContext, ReleaseContext), StormByte::Exception);
	Context* rejected_invoke = new Context(1);
	Context* rejected_clone = new Context(1);
	Context* rejected_release = new Context(1);
	ASSERT_THROWS(Callback(rejected_invoke, nullptr, CloneContext, ReleaseContext), StormByte::Exception);
	ASSERT_THROWS(Callback(rejected_clone, Invoke, nullptr, ReleaseContext), StormByte::Exception);
	ASSERT_THROWS(Callback(rejected_release, Invoke, CloneContext, nullptr), StormByte::Exception);
	delete rejected_invoke;
	delete rejected_clone;
	delete rejected_release;
	ASSERT_EQUAL(0, Context::live);
	Callback function(new Context(4), Invoke, CloneContext, ReleaseContext);
	ASSERT_TRUE(function.HasValue());
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Success), static_cast<int>(function.Call(3)));
	Callback copy(function);
	ASSERT_EQUAL(2, Context::live);
	Callback moved(std::move(copy));
	ASSERT_FALSE(copy.HasValue());
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Missing), static_cast<int>(copy.Call(1)));
	Callback assigned(new Context(1), Invoke, CloneContext, ReleaseContext);
	assigned = std::move(moved);
	ASSERT_FALSE(moved.HasValue());
	Callback foreign(new Context(1), Fail, CloneContext, ReleaseContext);
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Failure), static_cast<int>(foreign.Call(1)));
	Callback escaping(new Context(1), Escape, CloneContext, ReleaseContext);
	ASSERT_THROWS(escaping.Call(1), StormByte::Exception);
	using Producer = Safe::Function<int(int)>;
	int output = 0;
	Producer producer(new Context(5), Produce, CloneContext, ReleaseContext);
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Success), static_cast<int>(producer.Call(output, 2)));
	ASSERT_EQUAL(7, output);
	Producer moved_producer(std::move(producer));
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Missing), static_cast<int>(producer.Call(output, 1)));
	ASSERT_EQUAL(7, output);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Life
	// -------------------
	result += test_reject_copy_move_and_call();

	if (result == 0 && Context::live == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
