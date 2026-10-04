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

#include <StormByte/safe/owner.hxx>
#include <StormByte/exception.hxx>

#include <utility>

using namespace StormByte::Safe::Detail;

void StormByte::Safe::Detail::ThrowSafeConversionFailure(const char* message) {
	throw StormByte::Exception(message);
}

Owner::Owner(void* state, Clone clone, Destroy destroy) noexcept:
	m_state(state), m_clone(clone), m_destroy(destroy) {}

Owner::Owner() noexcept:
	m_state(nullptr), m_clone(nullptr), m_destroy(nullptr) {}

Owner::Owner(const Owner& other):
	m_state(other.m_state ? other.m_clone(other.m_state) : nullptr),
	m_clone(other.m_clone), m_destroy(other.m_destroy) {
	if (other.m_state && !m_state)
		throw StormByte::Exception("Safe collection copy failed");
}

Owner::Owner(Owner&& other) noexcept:
	m_state(std::exchange(other.m_state, nullptr)),
	m_clone(other.m_clone), m_destroy(other.m_destroy) {}

Owner::~Owner() noexcept {
	if (m_state)
		m_destroy(m_state);
}

Owner& Owner::operator=(const Owner& other) {
	if (this != &other) {
		Owner copy(other);
		*this = std::move(copy);
	}
	return *this;
}

Owner& Owner::operator=(Owner&& other) noexcept {
	if (this != &other) {
		if (m_state)
			m_destroy(m_state);
		m_state = std::exchange(other.m_state, nullptr);
		m_clone = other.m_clone;
		m_destroy = other.m_destroy;
	}
	return *this;
}

void* Owner::Get() const noexcept {
		return m_state;
}
