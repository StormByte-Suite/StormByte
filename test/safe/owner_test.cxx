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
#include <StormByte/safe/owner.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <utility>

using namespace StormByte;

namespace {
	struct Box {
		int value;
		static int live;
		explicit Box(int n): value(n) { ++live; }
		~Box() { --live; }
	};
	int Box::live = 0;

	void* CloneBox(const void* pointer) noexcept {
		return new Box(static_cast<const Box*>(pointer)->value);
	}

	void DestroyBox(void* pointer) noexcept {
		delete static_cast<Box*>(pointer);
	}
}

// -------------------
// Life
// -------------------

int test_construct_empty_and_reject() {
	Safe::Owner empty;
	ASSERT_NULL(empty.Get());
	Box* box = new Box(1);
	ASSERT_THROWS(Safe::Owner(box, CloneBox, nullptr), StormByte::Exception);
	ASSERT_EQUAL(1, Box::live);
	delete box;
	ASSERT_EQUAL(0, Box::live);
	RETURN_TEST(0);
}

int test_copy_clone_and_move_only() {
	Safe::Owner source(new Box(4), CloneBox, DestroyBox);
	ASSERT_NOT_NULL(source.Get());
	ASSERT_EQUAL(4, static_cast<Box*>(source.Get())->value);
	Safe::Owner copy(source);
	ASSERT_TRUE(source.Get() != copy.Get());
	ASSERT_EQUAL(4, static_cast<Box*>(copy.Get())->value);
	ASSERT_EQUAL(2, Box::live);
	Safe::Owner moved(std::move(source));
	ASSERT_NULL(source.Get());
	ASSERT_NOT_NULL(moved.Get());
	Safe::Owner move_only(new Box(8), nullptr, DestroyBox);
	Safe::Owner taken(std::move(move_only));
	ASSERT_NULL(move_only.Get());
	ASSERT_EQUAL(8, static_cast<Box*>(taken.Get())->value);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Life
	// -------------------
	result += test_construct_empty_and_reject();
	result += test_copy_clone_and_move_only();

	if (result == 0 && Box::live == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
