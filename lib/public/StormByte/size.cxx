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

#include <StormByte/size.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>

#include <cstdio>
#include <cwchar>

namespace StormByte {
	Size::operator Safe::String() const noexcept {
		char buf[32];
		const int n = std::snprintf(buf, sizeof(buf), "%zu", static_cast<std::size_t>(*this));
		if (n <= 0)
			return Safe::String("0");
		return Safe::String(buf);
	}

	Size::operator Safe::WString() const noexcept {
		wchar_t buf[32];
		const int n = std::swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%zu", static_cast<std::size_t>(*this));
		if (n <= 0)
			return Safe::WString(L"0");
		return Safe::WString(buf);
	}

	template STORMBYTE_INSTANTIATE Size& Size::operator=(char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size& Size::operator+=(char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator+=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size& Size::operator-=(char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator-=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size& Size::operator*=(char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator*=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size& Size::operator/=(char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator/=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size& Size::operator%=(char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size& Size::operator%=(char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size::operator char() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator signed char() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator unsigned char() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator short() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator unsigned short() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator int() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator unsigned int() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator long() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator long long() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator wchar_t() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator char8_t() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator char16_t() const noexcept;
	template STORMBYTE_INSTANTIATE Size::operator char32_t() const noexcept;

	template STORMBYTE_INSTANTIATE bool operator==(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE bool operator==(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE bool operator==(char32_t, Size) noexcept;

	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE std::strong_ordering operator<=>(char32_t, Size) noexcept;

	template STORMBYTE_INSTANTIATE Size operator+(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size operator+(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator+(char32_t, Size) noexcept;

	template STORMBYTE_INSTANTIATE Size operator-(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size operator-(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator-(char32_t, Size) noexcept;

	template STORMBYTE_INSTANTIATE Size operator*(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size operator*(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator*(char32_t, Size) noexcept;

	template STORMBYTE_INSTANTIATE Size operator/(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size operator/(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator/(char32_t, Size) noexcept;

	template STORMBYTE_INSTANTIATE Size operator%(Size, char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, signed char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, unsigned char) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, unsigned short) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, unsigned int) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, unsigned long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, unsigned long long) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, wchar_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, char8_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, char16_t) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(Size, char32_t) noexcept;

	template STORMBYTE_INSTANTIATE Size operator%(char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(signed char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(unsigned char, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(unsigned short, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(unsigned int, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(unsigned long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(unsigned long long, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(wchar_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(char8_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(char16_t, Size) noexcept;
	template STORMBYTE_INSTANTIATE Size operator%(char32_t, Size) noexcept;
}
