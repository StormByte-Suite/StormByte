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

#include <StormByte/safe/condition_variable.hxx>
#include <StormByte/safe/condition_variable_any.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/safe/mutex.hxx>
#include <StormByte/safe/shared_mutex.hxx>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <new>
#include <shared_mutex>

using namespace StormByte;

namespace {
	struct Gate {
		std::mutex mutex;
	};

	struct SharedGate {
		std::shared_mutex mutex;
	};

	struct Signal {
		std::condition_variable condition;
	};

	struct AnyGate {
		std::mutex mutex;
		std::condition_variable condition;
	};
}

Safe::Mutex::Mutex()
:	m_gate{nullptr} {
	void* raw = Safe::Heap::Allocate(sizeof(Gate));
	m_gate = new (raw) Gate;
}

Safe::Mutex::~Mutex() noexcept {
	static_cast<Gate*>(m_gate)->~Gate();
	Safe::Heap::Free(m_gate);
}

void Safe::Mutex::lock() {
	try {
		static_cast<Gate*>(m_gate)->mutex.lock();
	} catch (...) {
		Safe::Heap::RethrowException();
	}
}

bool Safe::Mutex::try_lock() noexcept {
	return static_cast<Gate*>(m_gate)->mutex.try_lock();
}

void Safe::Mutex::unlock() noexcept {
	static_cast<Gate*>(m_gate)->mutex.unlock();
}

Safe::SharedMutex::SharedMutex()
:	m_gate{nullptr} {
	void* raw = Safe::Heap::Allocate(sizeof(SharedGate));
	m_gate = new (raw) SharedGate;
}

Safe::SharedMutex::~SharedMutex() noexcept {
	static_cast<SharedGate*>(m_gate)->~SharedGate();
	Safe::Heap::Free(m_gate);
}

void Safe::SharedMutex::lock() {
	try {
		static_cast<SharedGate*>(m_gate)->mutex.lock();
	} catch (...) {
		Safe::Heap::RethrowException();
	}
}

bool Safe::SharedMutex::try_lock() noexcept {
	return static_cast<SharedGate*>(m_gate)->mutex.try_lock();
}

void Safe::SharedMutex::unlock() noexcept {
	static_cast<SharedGate*>(m_gate)->mutex.unlock();
}

void Safe::SharedMutex::lock_shared() {
	try {
		static_cast<SharedGate*>(m_gate)->mutex.lock_shared();
	} catch (...) {
		Safe::Heap::RethrowException();
	}
}

bool Safe::SharedMutex::try_lock_shared() noexcept {
	return static_cast<SharedGate*>(m_gate)->mutex.try_lock_shared();
}

void Safe::SharedMutex::unlock_shared() noexcept {
	static_cast<SharedGate*>(m_gate)->mutex.unlock_shared();
}

Safe::ConditionVariable::ConditionVariable()
:	m_signal{nullptr} {
	void* raw = Safe::Heap::Allocate(sizeof(Signal));
	m_signal = new (raw) Signal;
}

Safe::ConditionVariable::~ConditionVariable() noexcept {
	static_cast<Signal*>(m_signal)->~Signal();
	Safe::Heap::Free(m_signal);
}

void Safe::ConditionVariable::notify_one() noexcept {
	static_cast<Signal*>(m_signal)->condition.notify_one();
}

void Safe::ConditionVariable::notify_all() noexcept {
	static_cast<Signal*>(m_signal)->condition.notify_all();
}

void Safe::ConditionVariable::wait(UniqueLock& lock) {
	Gate& gate = *static_cast<Gate*>(lock.mutex()->m_gate);
	lock.Abandon();
	try {
		std::unique_lock<std::mutex> adopted(gate.mutex, std::adopt_lock);
		static_cast<Signal*>(m_signal)->condition.wait(adopted);
		adopted.release();
		lock.Claim();
	} catch (...) {
		gate.mutex.lock();
		lock.Claim();
		Safe::Heap::RethrowException();
	}
}

Safe::CvStatus Safe::ConditionVariable::WaitFor(UniqueLock& lock, std::int64_t nanos) {
	if (nanos <= 0)
		return CvStatus::Timeout;
	Gate& gate = *static_cast<Gate*>(lock.mutex()->m_gate);
	lock.Abandon();
	try {
		std::unique_lock<std::mutex> adopted(gate.mutex, std::adopt_lock);
		const auto status = static_cast<Signal*>(m_signal)->condition.wait_for(
			adopted,
			std::chrono::nanoseconds(nanos)
		);
		adopted.release();
		lock.Claim();
		if (status == std::cv_status::timeout)
			return CvStatus::Timeout;
		return CvStatus::NoTimeout;
	} catch (...) {
		gate.mutex.lock();
		lock.Claim();
		Safe::Heap::RethrowException();
	}
}

Safe::ConditionVariableAny::ConditionVariableAny()
:	m_gate{nullptr} {
	void* raw = Safe::Heap::Allocate(sizeof(AnyGate));
	m_gate = new (raw) AnyGate;
}

Safe::ConditionVariableAny::~ConditionVariableAny() noexcept {
	static_cast<AnyGate*>(m_gate)->~AnyGate();
	Safe::Heap::Free(m_gate);
}

void Safe::ConditionVariableAny::notify_one() noexcept {
	AnyGate& gate = *static_cast<AnyGate*>(m_gate);
	std::lock_guard<std::mutex> held(gate.mutex);
	gate.condition.notify_one();
}

void Safe::ConditionVariableAny::notify_all() noexcept {
	AnyGate& gate = *static_cast<AnyGate*>(m_gate);
	std::lock_guard<std::mutex> held(gate.mutex);
	gate.condition.notify_all();
}

void Safe::ConditionVariableAny::Enter() {
	try {
		static_cast<AnyGate*>(m_gate)->mutex.lock();
	} catch (...) {
		Safe::Heap::RethrowException();
	}
}

void Safe::ConditionVariableAny::Leave() noexcept {
	static_cast<AnyGate*>(m_gate)->mutex.unlock();
}

void Safe::ConditionVariableAny::Park() {
	AnyGate& gate = *static_cast<AnyGate*>(m_gate);
	try {
		std::unique_lock<std::mutex> adopted(gate.mutex, std::adopt_lock);
		gate.condition.wait(adopted);
		adopted.release();
	} catch (...) {
		gate.mutex.lock();
		Safe::Heap::RethrowException();
	}
}

Safe::CvStatus Safe::ConditionVariableAny::ParkFor(std::int64_t nanos) {
	AnyGate& gate = *static_cast<AnyGate*>(m_gate);
	try {
		std::unique_lock<std::mutex> adopted(gate.mutex, std::adopt_lock);
		const auto status = gate.condition.wait_for(adopted, std::chrono::nanoseconds(nanos));
		adopted.release();
		if (status == std::cv_status::timeout)
			return CvStatus::Timeout;
		return CvStatus::NoTimeout;
	} catch (...) {
		gate.mutex.lock();
		Safe::Heap::RethrowException();
	}
}
