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

#include <StormByte/uuid.hxx>

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#ifdef WINDOWS
#include <windows.h>
#include <bcrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

using namespace StormByte;

namespace {
	inline bool FillRandomBytes(uint8_t* out, size_t len) noexcept {
#ifdef WINDOWS
		NTSTATUS status = BCryptGenRandom(nullptr, out, static_cast<ULONG>(len), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		return status == 0;
#else
		int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		if (fd < 0) return false;
		ssize_t rd = read(fd, out, len);
		close(fd);
		return rd == (ssize_t)len;
#endif
	}
}

Safe::String StormByte::GenerateUUIDv4() noexcept {
	std::array<uint8_t, 16> b{};
	if (!FillRandomBytes(b.data(), b.size())) {
		static thread_local std::mt19937_64 rng((std::random_device())());
		static thread_local std::uniform_int_distribution<uint64_t> dist;
		uint64_t r1 = dist(rng);
		uint64_t r2 = dist(rng);
		std::memcpy(b.data(), &r1, 8);
		std::memcpy(b.data() + 8, &r2, 8);
	}

	b[6] = static_cast<uint8_t>((b[6] & 0x0F) | 0x40);
	b[8] = static_cast<uint8_t>((b[8] & 0x3F) | 0x80);
	char buf[37];
	std::snprintf(buf, sizeof(buf),
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		b[0], b[1], b[2], b[3],
		b[4], b[5],
		b[6], b[7],
		b[8], b[9],
		b[10], b[11], b[12], b[13], b[14], b[15]
	);
	return Safe::String(buf);
}
