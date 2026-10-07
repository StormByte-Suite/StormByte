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

#include <StormByte/safe/atomic.hxx>
#include <StormByte/safe/heap.hxx>

#include <atomic>
#include <cstring>
#include <new>

using namespace StormByte;

namespace {
	struct Word {
		std::atomic<std::uint64_t> bits;
	};

	std::uint64_t Mask(std::size_t bytes) noexcept {
		if (bytes >= 8)
			return ~std::uint64_t{0};
		return (std::uint64_t{1} << (bytes * 8)) - 1;
	}

	std::uint64_t Read(const void* value, std::size_t bytes) noexcept {
		std::uint64_t incoming = 0;
		std::memcpy(&incoming, value, bytes);
		return incoming & Mask(bytes);
	}

	void Write(void* out, std::size_t bytes, std::uint64_t value) noexcept {
		const std::uint64_t masked = value & Mask(bytes);
		std::memcpy(out, &masked, bytes);
	}

	std::memory_order Map(Safe::MemoryOrder order) noexcept {
		switch (order) {
			case Safe::MemoryOrder::Relaxed:
				return std::memory_order_relaxed;
			case Safe::MemoryOrder::Consume:
			case Safe::MemoryOrder::Acquire:
				return std::memory_order_acquire;
			case Safe::MemoryOrder::Release:
				return std::memory_order_release;
			case Safe::MemoryOrder::AcqRel:
				return std::memory_order_acq_rel;
			case Safe::MemoryOrder::SeqCst:
				return std::memory_order_seq_cst;
		}
		return std::memory_order_seq_cst;
	}

	Safe::MemoryOrder Failure(Safe::MemoryOrder order) noexcept {
		if (order == Safe::MemoryOrder::Release || order == Safe::MemoryOrder::AcqRel)
			return Safe::MemoryOrder::Acquire;
		return order;
	}

	std::uint64_t Apply(std::uint64_t current, std::int64_t delta, int op, std::uint64_t mask) noexcept {
		const std::uint64_t width = current & mask;
		std::uint64_t next = width;
		switch (op) {
			case 0:
				next = width + static_cast<std::uint64_t>(delta);
				break;
			case 1:
				next = width - static_cast<std::uint64_t>(delta);
				break;
			case 2:
				next = width & static_cast<std::uint64_t>(delta);
				break;
			case 3:
				next = width | static_cast<std::uint64_t>(delta);
				break;
			default:
				next = width ^ static_cast<std::uint64_t>(delta);
				break;
		}
		return (current & ~mask) | (next & mask);
	}
}

void* Safe::Detail::WordCreate(std::size_t bytes) {
	(void)bytes;
	void* raw = Safe::Heap::Allocate(sizeof(Word));
	return new (raw) Word;
}

void Safe::Detail::WordDestroy(void* word) noexcept {
	if (word == nullptr)
		return;
	static_cast<Word*>(word)->~Word();
	Safe::Heap::Free(word);
}

void Safe::Detail::WordLoad(void* word, std::size_t bytes, void* out, MemoryOrder order) noexcept {
	const std::uint64_t value = static_cast<Word*>(word)->bits.load(Map(order));
	Write(out, bytes, value);
}

void Safe::Detail::WordStore(void* word, std::size_t bytes, const void* value, MemoryOrder order) noexcept {
	static_cast<Word*>(word)->bits.store(Read(value, bytes), Map(order));
}

void Safe::Detail::WordExchange(void* word, std::size_t bytes, const void* desired, void* previous, MemoryOrder order) noexcept {
	const std::uint64_t old = static_cast<Word*>(word)->bits.exchange(Read(desired, bytes), Map(order));
	Write(previous, bytes, old);
}

bool Safe::Detail::WordCompareExchange(void* word, std::size_t bytes, void* expected, const void* desired, MemoryOrder success, MemoryOrder failure, bool weak) noexcept {
	auto& bits = static_cast<Word*>(word)->bits;
	const std::uint64_t want = Read(expected, bytes);
	const std::uint64_t next = Read(desired, bytes);
	std::uint64_t current = bits.load(std::memory_order_relaxed);
	if (weak && (current & std::uint64_t{1}) == (want & std::uint64_t{1}) && current != want)
		return false;
	for (;;) {
		if ((current & Mask(bytes)) != want) {
			Write(expected, bytes, current);
			return false;
		}
		if (bits.compare_exchange_weak(current, next, Map(success), Map(Failure(failure))))
			return true;
	}
}

void Safe::Detail::WordFetchOp(void* word, std::size_t bytes, std::int64_t delta, void* previous, MemoryOrder order, int op) noexcept {
	auto& bits = static_cast<Word*>(word)->bits;
	const std::uint64_t mask = Mask(bytes);
	std::uint64_t current = bits.load(std::memory_order_relaxed);
	for (;;) {
		const std::uint64_t next = Apply(current, delta, op, mask);
		if (bits.compare_exchange_weak(current, next, Map(order), std::memory_order_relaxed)) {
			Write(previous, bytes, current);
			return;
		}
	}
}

void Safe::Detail::WordWait(void* word, std::size_t bytes, const void* old, MemoryOrder order) noexcept {
	const std::uint64_t captured = Read(old, bytes);
	static_cast<Word*>(word)->bits.wait(captured, Map(order));
}

void Safe::Detail::WordNotifyOne(void* word) noexcept {
	static_cast<Word*>(word)->bits.notify_one();
}

void Safe::Detail::WordNotifyAll(void* word) noexcept {
	static_cast<Word*>(word)->bits.notify_all();
}

bool Safe::Detail::WordLockFree(void* word) noexcept {
	return static_cast<Word*>(word)->bits.is_lock_free();
}
