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

#include <StormByte/exception.hxx>

#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Optional<T>::Optional(): m_value() {}

		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Optional<T>::Optional(std::nullopt_t): Optional() {}

		template<Type::SafeValue T>
		Optional<T>::Optional(const T& value): Optional() {
			emplace(value);
		}

		template<Type::SafeValue T>
		Optional<T>::Optional(T&& value): Optional() {
			emplace(std::move(value));
		}

		template<Type::SafeValue T>
		Optional<T>& Optional<T>::operator=(const T& value) {
			Optional replacement(value);
			*this = std::move(replacement);
			return *this;
		}

		template<Type::SafeValue T>
		Optional<T>& Optional<T>::operator=(T&& value) {
			Optional replacement(std::move(value));
			*this = std::move(replacement);
			return *this;
		}

		template<Type::SafeValue T>
		Optional<T>::Optional(const std::optional<T>& value): m_value() {
			if (value)
				emplace(*value);
		}

		template<Type::SafeValue T>
		Optional<T>::Optional(std::optional<T>&& value): Optional() {
			if (value)
				emplace(std::move(*value));
			value.reset();
		}

		template<Type::SafeValue T>
		STORMBYTE_FORCE_INLINE Optional<T>::operator std::optional<T>() const {
			if (!has_value())
				return std::nullopt;
			return std::optional<T>(value());
		}
	}
}