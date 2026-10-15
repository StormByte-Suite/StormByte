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

#include <StormByte/platform.h>
#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/thread.hxx>

#include <chrono>
#include <cstring>
#include <exception>
#include <functional>
#include <new>
#include <thread>
#include <utility>
#ifdef UNIX
#include <pthread.h>
#endif

namespace StormByte::Safe::Detail {
#ifdef UNIX
	namespace {
		constexpr std::size_t ThreadStackBytes = 8u * 1024u * 1024u;

		struct Launch {
			void (*entry)(void*);
			void* argument;
		};

		void* RunLaunch(void* raw) {
			auto* launch = static_cast<Launch*>(raw);
			void (*entry)(void*) = launch->entry;
			void* argument = launch->argument;
			launch->~Launch();
			Heap::Free(raw);
			try {
				entry(argument);
			}
			catch (...) {
				std::terminate();
			}
			return nullptr;
		}

		template <typename Handle>
		std::uintptr_t AsHandle(Handle handle) noexcept {
			static_assert(sizeof(Handle) <= sizeof(std::uintptr_t));
			std::uintptr_t value = 0;
			std::memcpy(&value, &handle, sizeof(handle));
			return value;
		}
	}

	struct OwnedThread {
		pthread_t handle;

		bool joinable;

		explicit OwnedThread(void (*entry)(void*), void* argument)
		:	handle{}, joinable{false} {
			void* raw = Heap::Allocate(sizeof(Launch));
			auto* launch = ::new (raw) Launch{entry, argument};
			pthread_attr_t attr;
			if (pthread_attr_init(&attr) != 0) {
				launch->~Launch();
				Heap::Free(raw);
				throw AllocationError();
			}
			const int sized = pthread_attr_setstacksize(&attr, ThreadStackBytes);
			const int created = sized == 0 ? pthread_create(&handle, &attr, RunLaunch, launch) : sized;
			pthread_attr_destroy(&attr);
			if (created != 0) {
				launch->~Launch();
				Heap::Free(raw);
				throw AllocationError();
			}
			joinable = true;
		}
	};

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
		if (owned->joinable)
			std::terminate();
		owned->~OwnedThread();
		Heap::Free(thread);
	}

	bool ThreadJoinable(void* thread) noexcept {
		return static_cast<OwnedThread*>(thread)->joinable;
	}

	void ThreadJoin(void* thread) {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->joinable)
			throw Exception("Thread is not joinable");
		if (pthread_join(owned->handle, nullptr) != 0)
			throw Exception("Thread is not joinable");
		owned->joinable = false;
	}

	void ThreadDetach(void* thread) {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->joinable)
			throw Exception("Thread is not joinable");
		if (pthread_detach(owned->handle) != 0)
			throw Exception("Thread is not joinable");
		owned->joinable = false;
	}

	std::uint64_t ThreadId(void* thread) noexcept {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->joinable)
			return 0;
		return static_cast<std::uint64_t>(AsHandle(owned->handle));
	}

	std::uintptr_t ThreadNative(void* thread) noexcept {
		OwnedThread* owned = static_cast<OwnedThread*>(thread);
		if (owned == nullptr || !owned->joinable)
			return 0;
		return AsHandle(owned->handle);
	}

	std::uint64_t ThreadSelf() noexcept {
		return static_cast<std::uint64_t>(AsHandle(pthread_self()));
	}
#else
	struct OwnedThread {
		std::thread thread;

		explicit OwnedThread(void (*entry)(void*), void* argument)
		:	thread(entry, argument) {}
	};

	std::uint64_t Identify(const std::thread::id& id) noexcept {
		return static_cast<std::uint64_t>(std::hash<std::thread::id>{}(id));
	}

	std::uintptr_t AsHandle(std::thread::native_handle_type handle) noexcept {
		static_assert(sizeof(handle) <= sizeof(std::uintptr_t));
		std::uintptr_t value = 0;
		std::memcpy(&value, &handle, sizeof(handle));
		return value;
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
		return AsHandle(owned->thread.native_handle());
	}

	std::uint64_t ThreadSelf() noexcept {
		return Identify(std::this_thread::get_id());
	}
#endif

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
