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

#include <new>
#include <utility>

// Out-of-line implementation of StormByte::Safe::Heap factories.
// See safe/pointers.hxx for documentation of each member.

namespace StormByte {
	namespace Safe {
		namespace Heap {
			template<class T, class... Args>
			Shared<T> MakeShared(Args&&... args) {
				void* const memory = Allocate(sizeof(T));
				T* object = nullptr;
				try {
					object = ::new (memory) T(std::forward<Args>(args)...);
				} catch (...) {
					Free(memory);
					throw;
				}
				try {
					return Shared<T>(typename Shared<T>::Adopt{}, object);
				} catch (...) {
					object->~T();
					Free(memory);
					throw;
				}
			}

			template<class T, class... Args>
			Unique<T> MakeUnique(Args&&... args) {
				void* const memory = Allocate(sizeof(T));
				T* object = nullptr;
				try {
					object = ::new (memory) T(std::forward<Args>(args)...);
				} catch (...) {
					Free(memory);
					throw;
				}
				return Unique<T>(typename Unique<T>::Adopt{}, object);
			}
		}
	}
}
