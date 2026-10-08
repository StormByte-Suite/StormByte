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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/thread.hxx>

#include <chrono>
#include <exception>
#include <functional>
#include <new>
#include <thread>
#include <utility>

namespace StormByte::Safe::Detail {
	struct OwnedThread {
		std::thread thread;

		explicit OwnedThread(void (*entry)(void*), void* argument)
		:	thread(entry, argument) {}
	};

	std::uint64_t Identify(const std::thread::id& id) noexcept {
		return static_cast<std::uint64_t>(std::hash<std::thread::id>{}(id));
	}

	void* ThreadStart(void (*entry)(void*), void* argument) {
		void* raw = Heap::Allocate(sizeof(OwnedThread));
		try {
			return ::new (raw) OwnedThread(entry, argument);
		}
		catch (...) {
			Heap::Free(raw);
			throw AllocationError();
		}
	}

	void ThreadDestroy(void* thread) noexcept {
		if (thread == nullptr)
			return;
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned->thread.joinable())
			std::terminate();
		owned->~OwnedThread();
		Heap::Free(thread);
	}

	bool ThreadJoinable(void* thread) noexcept {
		return static_cast<OwnedThread*>(thread)->thread.joinable();
	}

	void ThreadJoin(void* thread) {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->thread.joinable())
			throw Exception("Thread is not joinable");
		owned->thread.join();
	}

	void ThreadDetach(void* thread) {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->thread.joinable())
			throw Exception("Thread is not joinable");
		owned->thread.detach();
	}

	std::uint64_t ThreadId(void* thread) noexcept {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->thread.joinable())
			return 0;
		return Identify(owned->thread.get_id());
	}

	std::uintptr_t ThreadNative(void* thread) noexcept {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->thread.joinable())
			return 0;
		return static_cast<std::uintptr_t>(owned->thread.native_handle());
	}

	std::uint64_t ThreadSelf() noexcept {
		return Identify(std::this_thread::get_id());
	}

	unsigned ThreadHardware() noexcept {
		return std::thread::hardware_concurrency();
	}

	void ThreadYield() noexcept {
		std::this_thread::yield();
	}

	void ThreadSleep(std::uint64_t nanoseconds) noexcept {
		std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
	}
}
