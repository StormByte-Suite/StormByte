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
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <type_traits>

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
		 * @brief Mix a byte range with FNV-1a. Empty is a defined value.
		 * @param bytes Bytes. May be empty.
		 * @return Hash. The same bytes produce the same value in every module.
		 */
		STORMBYTE_FORCE_INLINE std::size_t HashBytes(std::span<const std::byte> bytes) noexcept {
			constexpr std::uint64_t offset = 14695981039346656037ull;
			constexpr std::uint64_t prime = 1099511628211ull;
			std::uint64_t value = offset;
			for (const std::byte byte : bytes) {
				value ^= static_cast<std::uint64_t>(byte);
				value *= prime;
			}
			return static_cast<std::size_t>(value);
		}

		/**
		 * @brief Mix an existing hash with another. Order matters.
		 * @param seed Hash so far.
		 * @param next Next hash.
		 * @return Combined hash.
		 */
		STORMBYTE_FORCE_INLINE std::size_t HashCombine(std::size_t seed, std::size_t next) noexcept {
			return seed ^ (next + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2));
		}

		/**
		 * @class Hash
		 * @brief Cross-module hash. Not `std::hash`.
		 * @tparam T Hashed type.
		 *
		 * Integral, enumeration, floating-point and pointer keys are closed here. `String`, `WString`, `Size`, `ByteSize`, `Pair`, `Optional` and `Binary` are closed in their own headers. A type without a specialization is not a key of `UnorderedMap`.
		 */
		template<class T>
		struct Hash;

		/**
		 * @brief Hash of an integral or enumeration value.
		 * @tparam T Integral or enumeration type.
		 */
		template<class T>
		requires std::is_integral_v<T> || std::is_enum_v<T>
		struct STORMBYTE_PUBLIC_TYPE Hash<T> {
			/**
			 * @brief Hash @p value.
			 * @param value Value.
			 * @return Hash.
			 */
			STORMBYTE_FORCE_INLINE std::size_t operator()(T value) const noexcept {
				std::uint64_t widened = 0;
				if constexpr (std::is_same_v<T, bool>)
					widened = value ? 1ull : 0ull;
				else if constexpr (std::is_enum_v<T>) {
					using Underlying = std::underlying_type_t<T>;
					widened = static_cast<std::uint64_t>(static_cast<std::make_unsigned_t<Underlying>>(static_cast<Underlying>(value)));
				} else
					widened = static_cast<std::uint64_t>(static_cast<std::make_unsigned_t<T>>(value));
				return HashBytes(std::as_bytes(std::span<const std::uint64_t>(&widened, 1)));
			}
		};

		/**
		 * @brief Hash of a floating-point value. The sign bit is included. Padding is not.
		 * @tparam T Floating-point type.
		 */
		template<class T>
		requires std::is_floating_point_v<T>
		struct STORMBYTE_PUBLIC_TYPE Hash<T> {
			/**
			 * @brief Hash @p value.
			 * @param value Value. A NaN is hashed as its bit pattern, not as a canonical NaN.
			 * @return Hash.
			 */
			STORMBYTE_FORCE_INLINE std::size_t operator()(T value) const noexcept {
				unsigned char bytes[sizeof(T)] = {};
				std::size_t used = sizeof(T);
				if constexpr (std::is_same_v<T, long double> && sizeof(long double) > 10 && std::numeric_limits<long double>::digits == 64)
					used = 10;
				std::memcpy(bytes, &value, used);
				return HashBytes(std::span<const std::byte>(reinterpret_cast<const std::byte*>(bytes), used));
			}
		};

		/**
		 * @brief Hash of a pointer identity. The pointee is not read.
		 * @tparam T Pointee type.
		 */
		template<class T>
		struct STORMBYTE_PUBLIC_TYPE Hash<T*> {
			/**
			 * @brief Hash @p value.
			 * @param value Pointer. May be null.
			 * @return Hash.
			 */
			STORMBYTE_FORCE_INLINE std::size_t operator()(T* value) const noexcept {
				return Hash<std::uintptr_t>{}(reinterpret_cast<std::uintptr_t>(value));
			}
		};
	}
}
