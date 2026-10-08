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
#include <StormByte/safe/callback.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
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

	void* Clone(const void* pointer) noexcept {
		return new Context(static_cast<const Context*>(pointer)->value);
	}

	void Release(void* pointer) noexcept {
		delete static_cast<Context*>(pointer);
	}

	Safe::Status Invoke(void* pointer, const Safe::String& text) noexcept {
		static_cast<Context*>(pointer)->value += static_cast<int>(text.size());
		return Safe::Status::Success;
	}
}

// -------------------
// Life
// -------------------

int test_call_copy_and_move() {
	Safe::Callback callback(new Context(1), Invoke, Clone, Release);
	ASSERT_TRUE(callback.HasValue());
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Success), static_cast<int>(callback.Call(Safe::String("ab"))));
	Safe::Callback copy(callback);
	ASSERT_TRUE(copy.HasValue());
	ASSERT_EQUAL(2, Context::live);
	Safe::Callback moved(std::move(copy));
	ASSERT_FALSE(copy.HasValue());
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Missing), static_cast<int>(copy.Call(Safe::String("x"))));
	ASSERT_EQUAL(static_cast<int>(Safe::Status::Success), static_cast<int>(moved.Call(Safe::String("x"))));
	Safe::Callback assigned(new Context(1), Invoke, Clone, Release);
	assigned = std::move(moved);
	ASSERT_FALSE(moved.HasValue());
	ASSERT_TRUE(assigned.HasValue());
	RETURN_TEST(0);
}

int test_reject_null_arguments() {
	Context* context = new Context(1);
	ASSERT_THROWS(Safe::Callback(nullptr, Invoke, Clone, Release), StormByte::Exception);
	ASSERT_THROWS(Safe::Callback(context, nullptr, Clone, Release), StormByte::Exception);
	ASSERT_THROWS(Safe::Callback(context, Invoke, nullptr, Release), StormByte::Exception);
	ASSERT_THROWS(Safe::Callback(context, Invoke, Clone, nullptr), StormByte::Exception);
	delete context;
	ASSERT_EQUAL(0, Context::live);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Life
	// -------------------
	result += test_call_copy_and_move();
	result += test_reject_null_arguments();

	if (result == 0 && Context::live == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
