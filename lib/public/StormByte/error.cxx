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

#include <StormByte/error.txx>

#include <format>

using namespace StormByte;

const Error::Category<Error::Code>& Error::category() noexcept {
	static Category<Code> instance;
	return instance;
}

std::error_code Error::make_error_code(Error::Code e) noexcept {
	return std::error_code(static_cast<int>(e), Error::category());
}

Error::Fault::Fault() noexcept
: Fault(make_error_code(Error::Code::Success)) {}

Error::Fault::Fault(const std::error_code& code)
: m_code(code), m_what(std::format("{}: {}", code.category().name(), code.message())) {}

const std::error_code& Error::Fault::code() const noexcept {
	return m_code;
}

const char* Error::Fault::what() const noexcept {
	const char* text = static_cast<const char*>(m_what);
	return text ? text : "";
}

Error::Fault::operator bool() const noexcept {
	return static_cast<bool>(m_code);
}
