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

#include <StormByte/telemetry.hxx>
#include <StormByte/thread_lock.hxx>

#include <map>
#include <mutex>

namespace StormByte {
	struct Telemetry::Store {
		ThreadLock lock;
		std::map<CString, StormByte::Clock> clocks;
	};
}

using namespace StormByte;

// --- StormByte::Clock ---

Clock::Clock() noexcept:
	m_start{std::nullopt},
	m_time{std::chrono::microseconds::zero()},
	m_count{0} {}

Clock::Clock(Clock&&) noexcept = default;

Clock& Clock::operator=(Clock&&) noexcept = default;

Clock::~Clock() noexcept = default;

void Clock::Start() noexcept {
	m_start = std::chrono::steady_clock::now();
}

void Clock::Stop() noexcept {
	if (!m_start)
		return;
	const auto now = std::chrono::steady_clock::now();
	const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - *m_start);
	m_time += (elapsed.count() < 0 ? std::chrono::microseconds::zero() : elapsed);
	++m_count;
	m_start.reset();
}

std::uint64_t Clock::Count() const noexcept {
	return m_count;
}

std::chrono::microseconds Clock::Time() const noexcept {
	return m_time;
}

std::chrono::microseconds Clock::MeanDuration() const noexcept {
	if (m_count == 0)
		return std::chrono::microseconds::zero();
	return m_time / m_count;
}

// --- StormByte::Telemetry ---

Telemetry::Telemetry() noexcept:
	m_store{Heap::MakeUnique<Store>()} {}

Telemetry::Telemetry(Telemetry&& other) noexcept:
	m_store{std::move(other.m_store)} {}

Telemetry& Telemetry::operator=(Telemetry&& other) noexcept {
	if (this != &other)
		m_store = std::move(other.m_store);
	return *this;
}

Telemetry::~Telemetry() noexcept = default;

Clock& Telemetry::Clock(const std::string_view name) noexcept {
	if (!m_store)
		m_store = Heap::MakeUnique<Store>();
	m_store->lock.Lock();
	auto& clock = m_store->clocks[CString(name)];
	m_store->lock.Unlock();
	return clock;
}

const Clock& Telemetry::Clock(const std::string_view name) const noexcept {
	static const StormByte::Clock empty_clock;
	if (!m_store)
		return empty_clock;
	m_store->lock.Lock();
	const auto it = m_store->clocks.find(CString(name));
	const bool found = (it != m_store->clocks.end());
	const StormByte::Clock* result = found ? &it->second : &empty_clock;
	m_store->lock.Unlock();
	return *result;
}
