/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/telemetry.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/set.hxx>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <new>

#ifndef __has_feature
#define __has_feature(feature) 0
#endif

#if defined(__linux__) && defined(__GLIBC__) && !defined(__SANITIZE_ADDRESS__) && !__has_feature(address_sanitizer)

namespace {
	bool exhausted = false;
	std::size_t denied = 0;

	bool Reject() noexcept {
		if (!exhausted)
			return false;
		++denied;
		errno = ENOMEM;
		return true;
	}

	class Telemetry final: public StormByte::Telemetry {
		public:
			using StormByte::Telemetry::MeasureClock;
			operator StormByte::Safe::String() const override { return {}; }
	};

	struct Cleanup {
		Telemetry& Metrics;
		bool& Active;
		bool* AllocationFree = nullptr;
		~Cleanup() noexcept {
			const auto before = denied;
			Active = Metrics.MeasureClock("first cleanup sample needing storage").Active();
			if (AllocationFree)
				*AllocationFree = denied == before;
		}
	};
}

extern "C" void* __libc_malloc(std::size_t) noexcept;
extern "C" void* __libc_calloc(std::size_t, std::size_t) noexcept;
extern "C" void* __libc_realloc(void*, std::size_t) noexcept;
extern "C" void* __libc_memalign(std::size_t, std::size_t) noexcept;

extern "C" void* malloc(std::size_t size) noexcept {
	return Reject() ? nullptr : __libc_malloc(size);
}
extern "C" void* calloc(std::size_t count, std::size_t size) noexcept {
	return Reject() ? nullptr : __libc_calloc(count, size);
}
extern "C" void* realloc(void* pointer, std::size_t size) noexcept {
	return Reject() ? nullptr : __libc_realloc(pointer, size);
}
extern "C" int posix_memalign(void** pointer, std::size_t alignment, std::size_t size) noexcept {
	if (Reject())
		return ENOMEM;
	void* allocation = __libc_memalign(alignment, size);
	if (!allocation)
		return ENOMEM;
	*pointer = allocation;
	return 0;
}
extern "C" void* aligned_alloc(std::size_t alignment, std::size_t size) noexcept {
	return Reject() ? nullptr : __libc_memalign(alignment, size);
}

int main() {
	Telemetry normal;
	Telemetry unwinding;
	bool normal_active = true;
	bool unwind_active = true;
	bool unwind_allocation_free = false;
	// Initialize libc++abi exception state before denying its initial calloc.
	try { throw std::bad_alloc{}; } catch (const std::bad_alloc&) {}
	denied = 0;
	exhausted = true;
	{
		Telemetry allocation_free;
	}
	exhausted = false;
	if (denied != 0)
		return 1;
	using Map = StormByte::Safe::Map<int, int>;
	static_assert(!noexcept(StormByte::Safe::Set<int>{}));
	static_assert(!noexcept(Map{}));
	static_assert(!noexcept(Map(std::declval<Map&&>())));
	unsigned caught = 0;
	Map source{{1, 2}};
	exhausted = true;
	try {
		Map empty;
	} catch (const StormByte::Safe::AllocationError&) {
		++caught;
	}
	try {
		Map compared{std::less<int>{}};
	} catch (const StormByte::Safe::AllocationError&) {
		++caught;
	}
	try {
		Map moved{std::move(source)};
	} catch (const StormByte::Safe::AllocationError&) {
		++caught;
	}
	try {
		StormByte::Safe::Set<int> empty;
	} catch (const StormByte::Safe::AllocationError&) {
		++caught;
	}
	exhausted = false;
	if (caught != 4 || source.at(1) != 2)
		return 1;
	Map target{{3, 4}};
	denied = 0;
	exhausted = true;
	target = std::move(source);
	const bool assignment_allocation_free = denied == 0;
	exhausted = false;
	if (!assignment_allocation_free || !source.empty() || target.at(1) != 2)
		return 1;
	source.emplace(5, 6);
	exhausted = true;
	{
		Cleanup cleanup{normal, normal_active};
	}
	const bool normal_rejected = denied > 0;
	denied = 0;
	try {
		Cleanup cleanup{unwinding, unwind_active, &unwind_allocation_free};
		throw std::bad_alloc{};
	} catch (const std::bad_alloc&) {
	}
	exhausted = false;
	if (normal_active || unwind_active || !normal_rejected || !unwind_allocation_free)
		return 1;
	if (!normal.MeasureClock("recovered").Active())
		return 1;
	std::puts("Normal cleanup OOM and allocation-free unwind cleanup passed.");
	return 0;
}

#else
int main() {
	std::puts("Allocation interposition requires Linux/glibc without AddressSanitizer.");
	return 77;
}
#endif
