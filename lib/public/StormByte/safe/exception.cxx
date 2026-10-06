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

using namespace StormByte::Safe;

Exception::Exception() noexcept = default;

Exception::Exception(std::string_view message)
	: StormByte::Exception(StormByte::Exception::Path{"Safe"}, "{}", message) {}

Exception::Exception(const String& message)
	: StormByte::Exception(StormByte::Exception::Path{"Safe"}, "{}", static_cast<std::string_view>(message)) {}

Exception::Exception(const Exception& other) = default;

Exception::Exception(Exception&& other) noexcept = default;

Exception::~Exception() noexcept = default;

Exception& Exception::operator=(const Exception& other) = default;

Exception& Exception::operator=(Exception&& other) noexcept = default;

AllocationError::AllocationError() noexcept = default;

AllocationError::AllocationError(const AllocationError& other) noexcept = default;

AllocationError::AllocationError(AllocationError&& other) noexcept = default;

AllocationError::~AllocationError() noexcept = default;

AllocationError& AllocationError::operator=(const AllocationError& other) noexcept = default;

AllocationError& AllocationError::operator=(AllocationError&& other) noexcept = default;

const char* AllocationError::what() const noexcept {
	return "StormByte.Safe: Memory allocation failed";
}

ExpiredWeakPointerError::ExpiredWeakPointerError(std::string_view message)
	: Exception(message) {}

ExpiredWeakPointerError::ExpiredWeakPointerError(const String& message)
	: Exception(message) {}

ExpiredWeakPointerError::ExpiredWeakPointerError(const ExpiredWeakPointerError& other) = default;

ExpiredWeakPointerError::ExpiredWeakPointerError(ExpiredWeakPointerError&& other) noexcept = default;

ExpiredWeakPointerError::~ExpiredWeakPointerError() noexcept = default;

ExpiredWeakPointerError& ExpiredWeakPointerError::operator=(const ExpiredWeakPointerError& other) = default;

ExpiredWeakPointerError& ExpiredWeakPointerError::operator=(ExpiredWeakPointerError&& other) noexcept = default;

OutOfBoundsError::OutOfBoundsError(std::string_view message)
	: Exception(message) {}

OutOfBoundsError::OutOfBoundsError(const String& message)
	: Exception(message) {}

OutOfBoundsError::OutOfBoundsError(const OutOfBoundsError& other) = default;

OutOfBoundsError::OutOfBoundsError(OutOfBoundsError&& other) noexcept = default;

OutOfBoundsError::~OutOfBoundsError() noexcept = default;

OutOfBoundsError& OutOfBoundsError::operator=(const OutOfBoundsError& other) = default;

OutOfBoundsError& OutOfBoundsError::operator=(OutOfBoundsError&& other) noexcept = default;
