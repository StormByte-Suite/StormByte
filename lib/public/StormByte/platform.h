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

#pragma once

// ---------------------------------------------------------------------------
// Operating system
// ---------------------------------------------------------------------------
#if defined(_WIN32) || defined(__CYGWIN__)
    #define WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define MACOS
    #define UNIX
#elif defined(__linux__)
    #define LINUX
    #define UNIX
#elif defined(__unix__) || defined(__unix)
    #define UNIX
#else
    #error "Unsupported operating system"
#endif

// ---------------------------------------------------------------------------
// Architecture (pointer / ILP model)
// ---------------------------------------------------------------------------
#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__) \
    || defined(__aarch64__) || defined(_M_X64) || defined(_M_ARM64) \
    || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8)
    #define BIT64
#elif defined(_WIN32) || defined(__i386__) || defined(__i386) \
    || defined(_M_IX86) || defined(__arm__) || defined(_M_ARM) \
    || (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4)
    #define BIT32
#else
    #error "Unsupported architecture (expected 32-bit or 64-bit)"
#endif

// ---------------------------------------------------------------------------
// Compiler
// ---------------------------------------------------------------------------
#if defined(__clang__)
    #define CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define GCC
#elif defined(_MSC_VER)
    #define MSVC
#endif

// ---------------------------------------------------------------------------
// Force the body into the caller. `inline` alone is only a hint.
// ---------------------------------------------------------------------------
#if defined(MSVC)
    #define STORMBYTE_FORCE_INLINE __forceinline
#elif defined(GCC) || defined(CLANG)
    #define STORMBYTE_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define STORMBYTE_FORCE_INLINE inline
#endif
