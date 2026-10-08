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

#include <StormByte/thread_lock.hxx>
using namespace StormByte;
ThreadLock::~ThreadLock() noexcept = default;
void ThreadLock::Lock() noexcept {
	const auto this_id = std::this_thread::get_id();
	// Fast path: if current thread already owns the lock, return immediately.
	{
		std::unique_lock owner_lock(m_thread_owner_mutex);
		if (m_owner_thread_id && *m_owner_thread_id == this_id) {
			return;
		}
	}

	// Acquire the main mutex (blocks other threads). After acquiring, set ownership.
	m_main_mutex.lock();
	{
		std::unique_lock owner_lock(m_thread_owner_mutex);
		m_owner_thread_id = this_id;
	}
}

void ThreadLock::Unlock() noexcept {
	const auto this_id = std::this_thread::get_id();
	std::unique_lock owner_lock(m_thread_owner_mutex);
	if (!m_owner_thread_id || *m_owner_thread_id != this_id) {
		// Not the owner: per design Unlock() is a no-op for non-owning threads.
		// Return without changing state.
		return;
	}

	// Always fully release ownership and unlock the main mutex.
	m_owner_thread_id = std::nullopt;
	owner_lock.unlock();
	m_main_mutex.unlock();
}
