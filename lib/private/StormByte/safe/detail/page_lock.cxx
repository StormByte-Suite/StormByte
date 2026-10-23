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
#include <StormByte/safe/detail/page_lock.hxx>

#include <cstdint>
#include <mutex>
#include <unordered_map>

#ifdef WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
#else
	#include <sys/mman.h>
	#include <unistd.h>
#endif

namespace StormByte::Safe::Detail {
	namespace {
		struct Registry {
			std::mutex mutex;
			std::unordered_map<std::uintptr_t, std::size_t> holders; ///< Page address to number of blocks holding it.
		};

		// Leaked on purpose: static blocks may still release pages during static destruction.
		Registry& Pages() noexcept {
			static Registry* registry = new Registry;
			return *registry;
		}

		std::size_t PageSize() noexcept {
			static const std::size_t size = [] {
#ifdef WINDOWS
				SYSTEM_INFO info;
				GetSystemInfo(&info);
				return static_cast<std::size_t>(info.dwPageSize);
#else
				const long size = sysconf(_SC_PAGESIZE);
				return size > 0 ? static_cast<std::size_t>(size) : std::size_t{4096};
#endif
			}();
			return size;
		}

		// Results are ignored: locking is best effort, and unlocking a page whose lock failed is harmless.
		void SystemLock(std::uintptr_t address, std::size_t length) noexcept {
#ifdef WINDOWS
			VirtualLock(reinterpret_cast<void*>(address), length);
#else
			mlock(reinterpret_cast<const void*>(address), length);
#endif
		}

		void SystemUnlock(std::uintptr_t address, std::size_t length) noexcept {
#ifdef WINDOWS
			VirtualUnlock(reinterpret_cast<void*>(address), length);
#else
			munlock(reinterpret_cast<const void*>(address), length);
#endif
		}

		// Walks the pages under bytes and makes one system call per run of pages whose holder count crosses zero.
		void Update(std::span<std::byte> bytes, bool lock) noexcept {
			if (bytes.empty())
				return;
			const std::size_t page = PageSize();
			const std::uintptr_t start = reinterpret_cast<std::uintptr_t>(bytes.data());
			const std::uintptr_t first = start - start % page;
			const std::uintptr_t last = start + bytes.size();

			Registry& registry = Pages();
			std::lock_guard guard(registry.mutex);
			std::uintptr_t run = 0;
			std::size_t run_pages = 0;
			const auto flush = [&] {
				if (run_pages != 0)
					lock ? SystemLock(run, run_pages * page) : SystemUnlock(run, run_pages * page);
				run_pages = 0;
			};
			for (std::uintptr_t address = first; address < last; address += page) {
				bool crossed = false;
				if (lock) {
					try {
						crossed = registry.holders[address]++ == 0;
					}
					catch (...) {
						// Out of memory for the bookkeeping: leave the page unlocked and uncounted.
					}
				}
				else if (const auto found = registry.holders.find(address); found != registry.holders.end()) {
					if (--found->second == 0) {
						registry.holders.erase(found);
						crossed = true;
					}
				}
				if (!crossed) {
					flush();
					continue;
				}
				if (run_pages == 0)
					run = address;
				++run_pages;
			}
			flush();
		}
	}

	void LockPages(std::span<std::byte> bytes) noexcept {
		Update(bytes, true);
	}

	void UnlockPages(std::span<std::byte> bytes) noexcept {
		Update(bytes, false);
	}
}
