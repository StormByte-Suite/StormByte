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

#include <StormByte/byte_size.hxx>

#include <array>
#include <cstdint>
#include <cstdio>

namespace StormByte {
	namespace {
		void FormatIec(std::uint64_t bytes, char* buffer, std::size_t buffer_size) noexcept {
			static constexpr std::array<const char*, 7> units{
				"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"
			};
			std::uint64_t value = bytes;
			std::size_t unit = 0;
			std::uint64_t remainder = 0;
			while (value >= 1024 && unit + 1 < units.size()) {
				remainder = value % 1024;
				value /= 1024;
				++unit;
			}

			if (unit == 0) {
				std::snprintf(buffer, buffer_size, "%llu %s",
					static_cast<unsigned long long>(value), units[unit]);
				return;
			}

			unsigned hundredths = static_cast<unsigned>((remainder * 100ull + 512ull) / 1024ull);
			if (hundredths >= 100) {
				++value;
				hundredths = 0;
			}
			std::snprintf(buffer, buffer_size, "%llu.%02u %s",
				static_cast<unsigned long long>(value), hundredths, units[unit]);
		}
	}

	ByteSize::operator Safe::String() const noexcept {
		char buffer[32];
		FormatIec(m_value, buffer, sizeof(buffer));
		return Safe::String(buffer);
	}

	ByteSize::operator Safe::WString() const noexcept {
		return Safe::WString(static_cast<Safe::String>(*this));
	}

	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator+=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator-=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator*=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator/=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize& ByteSize::operator%=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize::operator char() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator signed char() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator unsigned char() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator short() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator unsigned short() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator int() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator unsigned int() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator long() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator long long() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator wchar_t() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator char8_t() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator char16_t() const noexcept;
	template STORMBYTE_INSTANTIATE ByteSize::operator char32_t() const noexcept;

	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(ByteSize, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE bool operator==(char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(signed char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(wchar_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(char8_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(char16_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(char32_t, ByteSize) noexcept;

	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(ByteSize, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(signed char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(wchar_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char8_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char16_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char32_t, ByteSize) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(ByteSize, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator+(char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(signed char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(unsigned char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(unsigned short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(unsigned int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(unsigned long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(unsigned long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(wchar_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(char8_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(char16_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator+(char32_t, ByteSize) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(ByteSize, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator-(char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(signed char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(unsigned char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(unsigned short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(unsigned int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(unsigned long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(unsigned long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(wchar_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(char8_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(char16_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator-(char32_t, ByteSize) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(ByteSize, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator*(char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(signed char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(unsigned char, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(unsigned short, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(unsigned int, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(unsigned long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(unsigned long long, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(wchar_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(char8_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(char16_t, ByteSize) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator*(char32_t, ByteSize) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator/(ByteSize, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, signed char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE ByteSize operator%(ByteSize, char32_t) noexcept;
}
