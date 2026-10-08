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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/test_handlers.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>

using namespace StormByte;

// -------------------
// Allocate
// -------------------

int test_allocate_and_free() {
	void* block = Safe::Heap::Allocate(16);
	ASSERT_NOT_NULL(block);
	auto* bytes = static_cast<std::uint8_t*>(block);
	bytes[0] = 7;
	bytes[15] = 9;
	ASSERT_EQUAL(std::uint8_t{7}, bytes[0]);
	ASSERT_EQUAL(std::uint8_t{9}, bytes[15]);
	Safe::Heap::Free(block);
	Safe::Heap::Free(nullptr);
	RETURN_TEST(0);
}

int test_allocate_rejects_bad_alloc() {
	try {
		throw std::bad_alloc();
	}
	catch (...) {
		ASSERT_THROWS(Safe::Heap::RethrowException(), Safe::AllocationError);
	}
	RETURN_TEST(0);
}

// -------------------
// Throw
// -------------------

int test_throw_expired_and_rethrow() {
	ASSERT_THROWS(Safe::Heap::ThrowExpiredWeakPointer(), Safe::ExpiredWeakPointerError);
	try {
		throw Safe::OutOfBoundsError("slot");
	}
	catch (...) {
		ASSERT_THROWS(Safe::Heap::RethrowException(), Safe::OutOfBoundsError);
	}
	try {
		throw std::runtime_error("foreign");
	}
	catch (...) {
		ASSERT_THROWS(Safe::Heap::RethrowException(), OperationError);
	}
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Allocate
	// -------------------
	result += test_allocate_and_free();
	result += test_allocate_rejects_bad_alloc();

	// -------------------
	// Throw
	// -------------------
	result += test_throw_expired_and_rethrow();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
