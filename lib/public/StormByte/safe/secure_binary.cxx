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


#include <StormByte/safe/secure_binary.hxx>
#include <StormByte/safe/detail/page_lock.hxx>

using StormByte::ByteSize;
using StormByte::Safe::Binary;
using StormByte::Safe::SecureBinary;

// Bodies fill the bytes after Binary is built, so growth already reaches the Discard override.
SecureBinary::SecureBinary() noexcept = default;

SecureBinary::SecureBinary(const ByteSize& count, std::byte value): Binary() {
	assign(count, value);
}

SecureBinary::SecureBinary(const ByteSize& count): Binary() {
	resize(count);
}

SecureBinary::SecureBinary(std::span<const std::byte> bytes): Binary() {
	assign(bytes);
}

SecureBinary::SecureBinary(const std::byte* bytes, const ByteSize& count): Binary() {
	append(bytes, count);
}

SecureBinary::SecureBinary(std::initializer_list<std::byte> list): Binary() {
	assign(list);
}

SecureBinary::SecureBinary(std::string_view text): Binary() {
	assign(std::span<const std::byte>(reinterpret_cast<const std::byte*>(text.data()), text.size()));
}

SecureBinary::SecureBinary(const char* text)
	: SecureBinary(text != nullptr ? std::string_view(text) : std::string_view{}) {}

SecureBinary::SecureBinary(const SecureBinary& other): Binary() {
	assign(other.span());
}

SecureBinary::SecureBinary(SecureBinary&& other) noexcept: Binary() {
	// Taken in the body, where Acquire reaches this class, so the block never leaves the locked pages.
	Binary::operator=(std::move(other));
}

SecureBinary::~SecureBinary() noexcept {
	const std::span<std::byte> block = Storage();
	Wipe(block);
	StormByte::Safe::Detail::UnlockPages(block);
}

SecureBinary& SecureBinary::operator=(const SecureBinary& other) {
	Binary::operator=(other);
	return *this;
}

SecureBinary& SecureBinary::operator=(SecureBinary&& other) noexcept {
	Binary::operator=(std::move(other));
	return *this;
}

SecureBinary& SecureBinary::operator=(std::initializer_list<std::byte> list) {
	assign(list);
	return *this;
}

void SecureBinary::Discard(std::span<std::byte> bytes) noexcept {
	Wipe(bytes);
}

bool SecureBinary::ConstantTimeCompare() const noexcept {
	return true;
}

void SecureBinary::Acquire(std::span<std::byte> block) noexcept {
	StormByte::Safe::Detail::LockPages(block);
}

void SecureBinary::Release(std::span<std::byte> block) noexcept {
	StormByte::Safe::Detail::UnlockPages(block);
}
