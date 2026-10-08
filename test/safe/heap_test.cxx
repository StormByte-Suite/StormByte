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

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
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

int test_allocate_fundamental_alignment() {
	for (std::size_t size: {std::size_t{1}, sizeof(std::max_align_t), std::size_t{257}}) {
		std::unique_ptr<void, decltype(&Safe::Heap::Free)> owned(Safe::Heap::Allocate(size), Safe::Heap::Free);
		void* block = owned.get();
		ASSERT_NOT_NULL(block);
		ASSERT_EQUAL(std::uintptr_t{0}, reinterpret_cast<std::uintptr_t>(block) % alignof(std::max_align_t));
		auto* bytes = static_cast<std::uint8_t*>(block);
		for (std::size_t index = 0; index < size; ++index)
			bytes[index] = static_cast<std::uint8_t>(index % 251);
		for (std::size_t index = 0; index < size; ++index)
			ASSERT_EQUAL(static_cast<std::uint8_t>(index % 251), bytes[index]);
	}
	RETURN_TEST(0);
}

int test_allocate_zero_and_independent_blocks() {
	std::unique_ptr<void, decltype(&Safe::Heap::Free)> zero(Safe::Heap::Allocate(0), Safe::Heap::Free);
	ASSERT_NOT_NULL(zero.get());
	std::unique_ptr<void, decltype(&Safe::Heap::Free)> first_owner(Safe::Heap::Allocate(32), Safe::Heap::Free);
	std::unique_ptr<void, decltype(&Safe::Heap::Free)> second_owner(Safe::Heap::Allocate(64), Safe::Heap::Free);
	void* first = first_owner.get();
	void* second = second_owner.get();
	ASSERT_NOT_NULL(first);
	ASSERT_NOT_NULL(second);
	ASSERT_TRUE(first != second);
	auto* first_bytes = static_cast<std::uint8_t*>(first);
	auto* second_bytes = static_cast<std::uint8_t*>(second);
	for (std::size_t index = 0; index < 32; ++index)
		first_bytes[index] = static_cast<std::uint8_t>(index);
	for (std::size_t index = 0; index < 64; ++index)
		second_bytes[index] = 255;
	zero.reset();
	second_owner.reset();
	for (std::size_t index = 0; index < 32; ++index)
		ASSERT_EQUAL(static_cast<std::uint8_t>(index), first_bytes[index]);
	first_owner.reset();
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
	result += test_allocate_fundamental_alignment();
	result += test_allocate_zero_and_independent_blocks();
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
