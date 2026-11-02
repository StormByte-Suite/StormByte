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
 *    Foundation, either version 3 of the License, or any later version.
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
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>

#include <cstdlib>
#include <cerrno>
#include <exception>
#include <iostream>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

using StormByte::Safe::String;
using StormByte::Safe::WString;

template<class Text, class View, class Other>
constexpr bool TextContracts() {
	return std::is_nothrow_default_constructible_v<Text> &&
		std::is_nothrow_move_constructible_v<Text> &&
		std::is_nothrow_move_assignable_v<Text> &&
		!std::is_nothrow_constructible_v<Text, View> &&
		!std::is_nothrow_constructible_v<Text, const typename View::value_type*> &&
		!std::is_nothrow_copy_constructible_v<Text> &&
		!std::is_nothrow_copy_assignable_v<Text> &&
		!std::is_nothrow_constructible_v<Text, const Other&> &&
		!noexcept(static_cast<Other>(std::declval<const Text&>())) &&
		!noexcept(Text::ToLower(std::declval<View>())) &&
		!noexcept(Text::ToUpper(std::declval<View>())) &&
		!noexcept(Text::SanitizeNewlines(std::declval<View>())) &&
		!noexcept(Text::RemoveWhitespace(std::declval<View>())) &&
		!noexcept(std::declval<const Text&>().ToLower()) &&
		!noexcept(std::declval<const Text&>().ToUpper()) &&
		!noexcept(std::declval<const Text&>().SanitizeNewlines()) &&
		!noexcept(std::declval<const Text&>().RemoveWhitespace());
}

static_assert(TextContracts<String, std::string_view, WString>());
static_assert(TextContracts<WString, std::wstring_view, String>());

#ifndef __has_feature
#define __has_feature(feature) 0
#endif

#if defined(__linux__) && defined(__GLIBC__) && !defined(__SANITIZE_ADDRESS__) && !__has_feature(address_sanitizer)

namespace {
	bool armed = false;
	constexpr std::size_t threshold = 1024 * 1024;
	constexpr std::size_t slots = 64;
	void* allocations[slots]{};

	std::size_t Find(void* pointer) noexcept {
		if (pointer)
			for (std::size_t i = 0; i < slots; ++i)
				if (allocations[i] == pointer)
					return i;
		return slots;
	}

	void Track(void* pointer) noexcept {
		if (!armed || !pointer)
			return;
		for (void*& slot : allocations)
			if (!slot) {
				slot = pointer;
				return;
			}
		std::abort();
	}
}

// Interposition is confined to this single-threaded glibc test executable.
extern "C" void* __libc_malloc(std::size_t) noexcept;
extern "C" void* __libc_realloc(void*, std::size_t) noexcept;
extern "C" void __libc_free(void*) noexcept;

extern "C" void* malloc(std::size_t size) noexcept {
	if (armed && size >= threshold) {
		errno = ENOMEM;
		return nullptr;
	}
	void* result = __libc_malloc(size);
	Track(result);
	return result;
}

extern "C" void* realloc(void* pointer, std::size_t size) noexcept {
	const std::size_t index = Find(pointer);
	if (armed && size >= threshold) {
		errno = ENOMEM;
		return nullptr;
	}
	void* result = __libc_realloc(pointer, size);
	if (index != slots) {
		if (result || size == 0)
			allocations[index] = result;
	} else
		Track(result);
	return result;
}

extern "C" void free(void* pointer) noexcept {
	const std::size_t index = Find(pointer);
	if (index != slots)
		allocations[index] = nullptr;
	__libc_free(pointer);
}

template<class Operation>
int ExpectFailure(const char* name, Operation operation) {
	bool caught = false;
	armed = true;
	try {
		operation();
	} catch (const StormByte::Safe::AllocationError&) {
		caught = true;
	} catch (const std::bad_alloc&) {
		caught = true;
	}
	armed = false;
	if (!caught) {
		std::cerr << name << ": allocation failure did not reach caller\n";
		return 1;
	}
	for (void* pointer : allocations)
		if (pointer) {
			std::cerr << name << ": intermediate allocation leaked\n";
			return 1;
		}
	return 0;
}

template<class Text, class View>
int CheckText(View large, View small) {
	int errors = 0;
	errors += ExpectFailure("view construction", [&] { Text text(large); });
	errors += ExpectFailure("pointer construction", [&] { Text text(large.data()); });
	const Text source(large);
	errors += ExpectFailure("copy construction", [&] { Text text(source); });
	Text destination(small);
	errors += ExpectFailure("copy assignment", [&] { destination = source; });
	if (View(destination) != small) {
		std::cerr << "failed assignment changed destination\n";
		++errors;
	}
	errors += ExpectFailure("lowercase", [&] { source.ToLower(); });
	errors += ExpectFailure("uppercase", [&] { source.ToUpper(); });
	errors += ExpectFailure("newlines", [&] { source.SanitizeNewlines(); });
	errors += ExpectFailure("whitespace", [&] { source.RemoveWhitespace(); });
	errors += ExpectFailure("short reserve", [&] { destination.reserve(StormByte::Size{2 * threshold}); });
	if (View(destination) != small) {
		std::cerr << "failed reserve changed destination\n";
		++errors;
	}
	const std::basic_string<typename View::value_type> medium(128, large.front());
	Text reserved(View{medium});
	reserved.reserve(StormByte::Size{2 * threshold});
	errors += ExpectFailure("capacity-preserving copy", [&] { Text text(reserved); });
	return errors;
}

int main() {
	std::set_terminate([] { std::_Exit(86); });
	const std::string narrow(2 * threshold, 'x');
	const std::wstring wide(2 * threshold, L'x');
	int errors = CheckText<String>(std::string_view{narrow}, std::string_view{"small"});
	errors += CheckText<WString>(std::wstring_view{wide}, std::wstring_view{L"small"});
	const String text(narrow);
	const WString wtext(wide);
	errors += ExpectFailure("UTF-8 conversion constructor", [&] { String converted(wtext); });
	errors += ExpectFailure("wide conversion constructor", [&] { WString converted(text); });
	errors += ExpectFailure("UTF-8 conversion operator", [&] { static_cast<void>(static_cast<String>(wtext)); });
	errors += ExpectFailure("wide conversion operator", [&] { static_cast<void>(static_cast<WString>(text)); });
	return errors;
}

#else

int main() {
	std::cerr << "Allocation interposition requires Linux/glibc without AddressSanitizer; compile-time contracts were checked.\n";
	return 77;
}

#endif
