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

using namespace StormByte;

Exception::Exception() noexcept = default;

Exception::Exception(std::string_view message) {
	m_what.append("StormByte: ");
	m_what.append(message);
}

Exception::Exception(const Exception& other) = default;

Exception::Exception(Exception&& other) noexcept = default;

Exception::~Exception() noexcept = default;

Exception& Exception::operator=(const Exception& other) = default;

Exception& Exception::operator=(Exception&& other) noexcept = default;

const char* Exception::what() const noexcept {
	return m_what.c_str();
}

DeserializeError::DeserializeError(std::string_view message)
	: Exception(message) {}

DeserializeError::DeserializeError(const DeserializeError& other) = default;

DeserializeError::DeserializeError(DeserializeError&& other) noexcept = default;

DeserializeError::~DeserializeError() noexcept = default;

DeserializeError& DeserializeError::operator=(const DeserializeError& other) = default;

DeserializeError& DeserializeError::operator=(DeserializeError&& other) noexcept = default;

OperationError::OperationError(std::string_view message)
	: Exception(message) {}

OperationError::OperationError(const OperationError& other) = default;

OperationError::OperationError(OperationError&& other) noexcept = default;

OperationError::~OperationError() noexcept = default;

OperationError& OperationError::operator=(const OperationError& other) = default;

OperationError& OperationError::operator=(OperationError&& other) noexcept = default;

Base64Error::Base64Error(std::string_view message)
	: Exception(message) {}

Base64Error::Base64Error(const Base64Error& other) = default;

Base64Error::Base64Error(Base64Error&& other) noexcept = default;

Base64Error::~Base64Error() noexcept = default;

Base64Error& Base64Error::operator=(const Base64Error& other) = default;

Base64Error& Base64Error::operator=(Base64Error&& other) noexcept = default;
