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

#pragma once

#include <StormByte/visibility.h>

#include <cstddef>

/**
 * @file safe/heap.hxx
 * @brief Private Base heap used by @ref StormByte::Safe::Shared, @ref StormByte::Safe::Unique and @ref StormByte::Safe::Clonable.
 *
 * Not installed. Not part of the public include tree. Implementation lives in `safe/heap.cxx` so allocation and release run on Base's CRT.
 */

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Owned values that cross a DLL without the caller's CRT.
	 */
	namespace Safe {
		/**
		 * @namespace StormByte::Safe::Heap
		 * @brief Allocate and free raw blocks on Base's heap.
		 *
		 * Public templates call @ref Allocate and @ref Free through matching declarations in the public headers. This header exists only for the translation unit that defines those functions. A failed allocation throws @ref AllocationError and does not allocate the exception message.
		 */
		namespace Heap {
			/**
			 * @brief Allocate @p bytes on Base's heap.
			 * @param bytes Block size in octets. Zero is forwarded to `operator new`.
			 * @return Address of the block. Never null.
			 * @throws AllocationError The allocator cannot satisfy the request. `std::bad_alloc` is not propagated.
			 * @throws Exception A StormByte exception already in flight is rethrown unchanged.
			 * @throws OperationError A foreign exception is translated. If that translation cannot allocate, @ref AllocationError is thrown instead.
			 */
			STORMBYTE_PUBLIC void* Allocate(std::size_t bytes);

			/**
			 * @brief Release a block obtained from @ref Allocate.
			 * @param pointer Block address, or a null pointer. A null pointer is ignored.
			 */
			STORMBYTE_PUBLIC void Free(void* pointer) noexcept;

			/**
			 * @brief Throw @ref ExpiredWeakPointerError.
			 * @throws ExpiredWeakPointerError Always. The body does not include the path.
			 */
			[[noreturn]] STORMBYTE_PUBLIC void ThrowExpiredWeakPointer();

			/**
			 * @brief Preserve an active StormByte exception or translate a foreign one.
			 * @pre Called from an active exception handler.
			 * @throws Exception A StormByte exception already in flight is rethrown unchanged.
			 * @throws AllocationError The active exception is `std::bad_alloc`, or translating it cannot allocate.
			 * @throws OperationError The active exception is foreign. The body is `what()`, or a fixed unknown-exception text.
			 */
			[[noreturn]] STORMBYTE_PUBLIC void RethrowException();
		}
	}
}
