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
	struct Clock::State {
		mutable std::mutex lock;
		std::chrono::microseconds time{};
		std::uint64_t count{};
	};

	struct Telemetry::Store {
		ThreadLock lock;
		std::map<Safe::String, StormByte::Clock> clocks;
	};
}

using namespace StormByte;

// --- StormByte::Clock ---

Clock::Clock() noexcept:
	m_state{} {}

Clock::Clock(Clock&& other) noexcept {
	std::lock_guard lock(other.m_state_lock);
	m_state = std::move(other.m_state);
}

Clock& Clock::operator=(Clock&& other) noexcept {
	if (this != &other) {
		std::scoped_lock lock(m_state_lock, other.m_state_lock);
		m_state = std::move(other.m_state);
	}
	return *this;
}

Clock::~Clock() noexcept = default;

Clock::Sample::Sample(Safe::Shared<State> state) noexcept:
	m_state{std::move(state)},
	m_start{std::chrono::steady_clock::now()},
	m_elapsed{},
	m_active{static_cast<bool>(m_state)} {}

Clock::Sample::Sample(Sample&& other) noexcept:
	m_state{std::move(other.m_state)},
	m_start{other.m_start},
	m_elapsed{other.m_elapsed},
	m_active{std::exchange(other.m_active, false)} {}

Clock::Sample& Clock::Sample::operator=(Sample&& other) noexcept {
	if (this != &other) {
		(void)Stop();
		m_state = std::move(other.m_state);
		m_start = other.m_start;
		m_elapsed = other.m_elapsed;
		m_active = std::exchange(other.m_active, false);
	}
	return *this;
}

Clock::Sample::~Sample() noexcept {
	(void)Stop();
}

std::chrono::microseconds Clock::Sample::Stop() noexcept {
	if (!m_active)
		return m_elapsed;
	const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::steady_clock::now() - m_start);
	m_elapsed = elapsed.count() < 0 ? std::chrono::microseconds::zero() : elapsed;
	if (m_state) {
		std::lock_guard lock(m_state->lock);
		m_state->time += m_elapsed;
		++m_state->count;
	}
	m_active = false;
	m_state.reset();
	return m_elapsed;
}

Clock::Sample Clock::Measure() {
	Safe::Shared<State> state;
	{
		std::lock_guard lock(m_state_lock);
		if (!m_state)
			m_state = Safe::Heap::MakeShared<State>();
		state = m_state;
	}
	return Sample{std::move(state)};
}

Clock::Values Clock::GetValues() const noexcept {
	Safe::Shared<State> state;
	{
		std::lock_guard lock(m_state_lock);
		state = m_state;
	}
	if (!state)
		return {};
	std::lock_guard lock(state->lock);
	const auto mean = state->count == 0
		? std::chrono::microseconds::rep{0}
		: static_cast<std::chrono::microseconds::rep>(
			static_cast<std::uint64_t>(state->time.count()) / state->count);
	return Values{
		state->count,
		state->time,
		std::chrono::microseconds{mean}
	};
}

std::uint64_t Clock::Count() const noexcept {
	return GetValues().Count;
}

std::chrono::microseconds Clock::Time() const noexcept {
	return GetValues().Time;
}

std::chrono::microseconds Clock::MeanDuration() const noexcept {
	return GetValues().MeanDuration;
}

// --- StormByte::Telemetry ---

Telemetry::Telemetry() noexcept:
	m_store{Safe::Heap::MakeUnique<Store>()} {}

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
		m_store = Safe::Heap::MakeUnique<Store>();
	m_store->lock.Lock();
	auto& clock = m_store->clocks[Safe::String(name)];
	m_store->lock.Unlock();
	return clock;
}

const Clock& Telemetry::Clock(const std::string_view name) const noexcept {
	static const StormByte::Clock empty_clock;
	if (!m_store)
		return empty_clock;
	m_store->lock.Lock();
	const auto it = m_store->clocks.find(Safe::String(name));
	const bool found = (it != m_store->clocks.end());
	const StormByte::Clock* result = found ? &it->second : &empty_clock;
	m_store->lock.Unlock();
	return *result;
}

Clock::Sample Telemetry::MeasureClock(const std::string_view name) {
	return Clock(name).Measure();
}
