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

#include <StormByte/safe/string.hxx>
#include <StormByte/telemetry.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

static_assert(StormByte::Type::IsSafe<StormByte::Clock::Values>::value);
static_assert(StormByte::Type::MaybeSafe<StormByte::Clock::Sample>);
static_assert(!StormByte::Type::SafeValue<StormByte::Clock::Sample>);

namespace {
	class Dummy final: public StormByte::Telemetry {
		public:
			Dummy() noexcept = default;
			~Dummy() noexcept override = default;

			Dummy(const Dummy&) = delete;
			Dummy(Dummy&&) noexcept = default;
			Dummy& operator=(const Dummy&) = delete;
			Dummy& operator=(Dummy&&) noexcept = default;

			void Beat(std::string_view name = "x") noexcept {
				auto sample = MeasureClock(name);
				std::this_thread::sleep_for(std::chrono::microseconds(50));
				(void)sample.Stop();
			}

			StormByte::Clock::Sample Measure(std::string_view name = "x") noexcept {
				return MeasureClock(name);
			}

			StormByte::Clock::Values Values(std::string_view name = "x") const noexcept {
				return Clock(name).GetValues();
			}

			std::uint64_t Beats(std::string_view name = "x") const noexcept {
				return Clock(name).Count();
			}

			std::chrono::microseconds Time(std::string_view name = "x") const noexcept {
				return Clock(name).Time();
			}

			std::chrono::microseconds MeanDuration(std::string_view name = "x") const noexcept {
				return Clock(name).MeanDuration();
			}

			operator StormByte::Safe::String() const override {
				return StormByte::Safe::String("dummy");
			}
	};
}

// -------------------
// Clock
// -------------------

int test_clock_destructor_records() {
	StormByte::Clock clock;
	{
		auto sample = clock.Measure();
		ASSERT_TRUE(sample.Active());
		std::this_thread::sleep_for(std::chrono::microseconds(20));
	}
	ASSERT_EQUAL(1ull, clock.Count());
	ASSERT_TRUE(clock.Time().count() >= 0);
	RETURN_TEST(0);
}

int test_clock_empty_snapshot() {
	const StormByte::Clock clock;
	ASSERT_EQUAL(0ull, clock.Count());
	ASSERT_EQUAL(0ll, clock.Time().count());
	ASSERT_EQUAL(0ll, clock.MeanDuration().count());
	const auto values = clock.GetValues();
	ASSERT_EQUAL(0ull, values.Count);
	ASSERT_EQUAL(0ll, values.Time.count());
	ASSERT_EQUAL(0ll, values.MeanDuration.count());
	RETURN_TEST(0);
}

int test_clock_empty_sample_stop_is_noop() {
	StormByte::Clock clock;
	StormByte::Clock::Sample sample;
	ASSERT_FALSE(sample.Active());
	ASSERT_EQUAL(0ll, sample.Stop().count());
	ASSERT_EQUAL(0ll, sample.Stop().count());
	ASSERT_EQUAL(0ull, clock.Count());
	RETURN_TEST(0);
}

int test_clock_nested_samples_are_independent() {
	StormByte::Clock clock;
	auto outer = clock.Measure();
	ASSERT_TRUE(outer.Active());
	std::this_thread::sleep_for(std::chrono::microseconds(20));
	auto inner = clock.Measure();
	ASSERT_TRUE(inner.Active());
	std::this_thread::sleep_for(std::chrono::microseconds(20));
	const auto inner_elapsed = inner.Stop();
	const auto outer_elapsed = outer.Stop();
	ASSERT_EQUAL(2ull, clock.Count());
	ASSERT_FALSE(inner.Active());
	ASSERT_FALSE(outer.Active());
	ASSERT_TRUE(inner_elapsed.count() > 0);
	ASSERT_TRUE(outer_elapsed >= inner_elapsed);
	ASSERT_EQUAL(outer_elapsed, outer.Stop());
	const auto values = clock.GetValues();
	ASSERT_EQUAL(values.Count, clock.Count());
	ASSERT_EQUAL(values.Time, clock.Time());
	ASSERT_EQUAL(values.MeanDuration, clock.MeanDuration());
	RETURN_TEST(0);
}

int test_clock_not_copyable() {
	static_assert(!std::is_copy_constructible_v<StormByte::Clock>);
	static_assert(!std::is_copy_assignable_v<StormByte::Clock>);
	static_assert(!std::is_copy_constructible_v<StormByte::Clock::Sample>);
	static_assert(!std::is_copy_assignable_v<StormByte::Clock::Sample>);
	static_assert(std::is_move_constructible_v<StormByte::Clock::Sample>);
	RETURN_TEST(0);
}

int test_clock_sample_move_transfers_ownership() {
	StormByte::Clock clock;
	auto original = clock.Measure();
	ASSERT_TRUE(original.Active());
	StormByte::Clock::Sample taken(std::move(original));
	ASSERT_FALSE(original.Active());
	ASSERT_TRUE(taken.Active());
	std::this_thread::sleep_for(std::chrono::microseconds(20));
	const auto elapsed = taken.Stop();
	ASSERT_TRUE(elapsed.count() > 0);
	ASSERT_EQUAL(1ull, clock.Count());
	ASSERT_EQUAL(elapsed, taken.Stop());
	RETURN_TEST(0);
}

int test_clock_two_beats() {
	Dummy dummy;
	dummy.Beat();
	dummy.Beat();
	ASSERT_EQUAL(2ull, dummy.Beats());
	ASSERT_TRUE(dummy.Time().count() >= 0);
	ASSERT_TRUE(dummy.MeanDuration().count() >= 0);
	ASSERT_EQUAL(dummy.Time() / 2, dummy.MeanDuration());
	RETURN_TEST(0);
}

// -------------------
// Drawer
// -------------------

int test_drawer_missing_clock_is_empty() {
	Dummy dummy;
	const Dummy& view = dummy;
	ASSERT_EQUAL(0ull, view.Beats("missing"));
	ASSERT_EQUAL(0ll, view.Time("missing").count());
	ASSERT_EQUAL(0ll, view.MeanDuration("missing").count());
	dummy.Beat("missing");
	ASSERT_EQUAL(1ull, view.Beats("missing"));
	ASSERT_EQUAL(0ull, view.Beats("other"));
	RETURN_TEST(0);
}

int test_drawer_move_moves_counters() {
	Dummy source;
	source.Beat("m1");
	ASSERT_EQUAL(1ull, source.Beats("m1"));
	Dummy taken(std::move(source));
	ASSERT_EQUAL(1ull, taken.Beats("m1"));
	ASSERT_EQUAL(0ull, source.Beats("m1"));
	source.Beat("m1");
	ASSERT_EQUAL(1ull, source.Beats("m1"));
	Dummy assigned;
	assigned = std::move(taken);
	ASSERT_EQUAL(1ull, assigned.Beats("m1"));
	ASSERT_EQUAL(0ull, taken.Beats("m1"));
	RETURN_TEST(0);
}

int test_drawer_multithread_same_key() {
	Dummy dummy;
	constexpr int thread_count = 8;
	constexpr int iterations = 100;
	std::vector<std::thread> threads;
	for (int index = 0; index < thread_count; ++index) {
		threads.emplace_back([&dummy]() {
			for (int i = 0; i < iterations; ++i) {
				auto sample = dummy.Measure("shared-key");
				if (!sample.Active())
					return;
				std::this_thread::sleep_for(std::chrono::microseconds(10));
				(void)sample.Stop();
			}
		});
	}
	for (auto& thread : threads)
		thread.join();
	const auto values = dummy.Values("shared-key");
	ASSERT_EQUAL(static_cast<std::uint64_t>(thread_count * iterations), values.Count);
	ASSERT_TRUE(values.Time.count() > 0);
	ASSERT_TRUE(values.MeanDuration.count() > 0);
	ASSERT_EQUAL(values.Time / static_cast<std::int64_t>(values.Count), values.MeanDuration);
	RETURN_TEST(0);
}

int test_drawer_string_conversions() {
	const Dummy dummy;
	const StormByte::Safe::String owned = static_cast<StormByte::Safe::String>(dummy);
	ASSERT_EQUAL(std::string_view{"dummy"}, std::string_view{owned});
	const std::string text = static_cast<std::string>(dummy);
	ASSERT_EQUAL(std::string{"dummy"}, text);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Clock
	// -------------------
	result += test_clock_destructor_records();
	result += test_clock_empty_sample_stop_is_noop();
	result += test_clock_empty_snapshot();
	result += test_clock_nested_samples_are_independent();
	result += test_clock_not_copyable();
	result += test_clock_sample_move_transfers_ownership();
	result += test_clock_two_beats();

	// -------------------
	// Drawer
	// -------------------
	result += test_drawer_missing_clock_is_empty();
	result += test_drawer_move_moves_counters();
	result += test_drawer_multithread_same_key();
	result += test_drawer_string_conversions();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
