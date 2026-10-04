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
#include <StormByte/test_handlers.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

static_assert(StormByte::Type::IsSafe<StormByte::Clock::Values>::value);
static_assert(StormByte::Type::MaybeSafe<StormByte::Clock::Sample>);
static_assert(!StormByte::Type::SafeValue<StormByte::Clock::Sample>);

namespace {
	class Dummy final : public StormByte::Telemetry {
		public:
			Dummy() noexcept = default;
			~Dummy() noexcept override = default;

			Dummy(Dummy&&) noexcept = default;
			Dummy& operator=(Dummy&&) noexcept = default;

			void Beat(const std::string_view name = "x") noexcept {
				auto sample = MeasureClock(name);
				std::this_thread::sleep_for(std::chrono::microseconds(50));
				(void)sample.Stop();
			}

			StormByte::Clock::Sample Measure(const std::string_view name = "x") {
				return MeasureClock(name);
			}

			StormByte::Clock::Values Values(const std::string_view name = "x") const noexcept {
				return Clock(name).GetValues();
			}

			std::uint64_t Beats(const std::string_view name = "x") const noexcept {
				return Clock(name).Count();
			}

			std::chrono::microseconds Time(const std::string_view name = "x") const noexcept {
				return Clock(name).Time();
			}

			std::chrono::microseconds MeanDuration(const std::string_view name = "x") const noexcept {
				return Clock(name).MeanDuration();
			}

			operator StormByte::Safe::String() const override {
				return StormByte::Safe::String("dummy");
			}
	};
}

static int test_clock_two_beats() {
	Dummy dummy;
	dummy.Beat();
	dummy.Beat();

	ASSERT_EQUAL("test_clock_two_beats", 2ull, dummy.Beats());
	ASSERT_TRUE("test_clock_two_beats", dummy.Time().count() >= 0);
	ASSERT_TRUE("test_clock_two_beats", dummy.MeanDuration().count() >= 0);
	return 0;
}

static int test_empty_sample_stop_is_noop() {
	StormByte::Clock clock;
	StormByte::Clock::Sample sample;

	ASSERT_FALSE("test_empty_sample_stop_is_noop", sample.Active());
	ASSERT_EQUAL("test_empty_sample_stop_is_noop", 0ll, sample.Stop().count());
	ASSERT_EQUAL("test_empty_sample_stop_is_noop", 0ull, clock.Count());
	ASSERT_EQUAL("test_empty_sample_stop_is_noop", 0ll, clock.Time().count());
	return 0;
}

static int test_nested_samples_are_independent() {
	StormByte::Clock clock;
	auto outer = clock.Measure();
	ASSERT_TRUE("test_nested_samples_are_independent", outer.Active());
	std::this_thread::sleep_for(std::chrono::microseconds(20));
	auto inner = clock.Measure();
	ASSERT_TRUE("test_nested_samples_are_independent", inner.Active());
	std::this_thread::sleep_for(std::chrono::microseconds(20));
	const auto inner_elapsed = inner.Stop();
	const auto outer_elapsed = outer.Stop();

	ASSERT_EQUAL("test_nested_samples_are_independent", 2ull, clock.Count());
	ASSERT_FALSE("test_nested_samples_are_independent", inner.Active());
	ASSERT_FALSE("test_nested_samples_are_independent", outer.Active());
	ASSERT_TRUE("test_nested_samples_are_independent", inner_elapsed.count() > 0);
	ASSERT_TRUE("test_nested_samples_are_independent", outer_elapsed >= inner_elapsed);
	ASSERT_EQUAL("test_nested_samples_are_independent", outer_elapsed, outer.Stop());
	return 0;
}

static int test_const_clock_missing_and_lazy_insert() {
	Dummy dummy;
	const Dummy& const_ref = dummy;

	ASSERT_EQUAL("test_const_clock_missing_and_lazy_insert", 0ull, const_ref.Beats("missing"));
	ASSERT_EQUAL("test_const_clock_missing_and_lazy_insert", 0ll, const_ref.Time("missing").count());
	ASSERT_EQUAL("test_const_clock_missing_and_lazy_insert", 0ll, const_ref.MeanDuration("missing").count());

	dummy.Beat("missing");
	ASSERT_EQUAL("test_const_clock_missing_and_lazy_insert", 1ull, const_ref.Beats("missing"));
	ASSERT_TRUE("test_const_clock_missing_and_lazy_insert", const_ref.Time("missing").count() >= 0);
	return 0;
}

static int test_string_conversions() {
	Dummy dummy;
	const StormByte::Safe::String cstr = static_cast<StormByte::Safe::String>(dummy);
	ASSERT_EQUAL("test_string_conversions", StormByte::Safe::String("dummy"), cstr);

	const std::string str = static_cast<std::string>(dummy);
	ASSERT_EQUAL("test_string_conversions", std::string("dummy"), str);
	return 0;
}

static int test_move_semantics() {
	Dummy dummy1;
	dummy1.Beat("m1");
	ASSERT_EQUAL("test_move_semantics", 1ull, dummy1.Beats("m1"));

	Dummy dummy2 = std::move(dummy1);
	ASSERT_EQUAL("test_move_semantics", 1ull, dummy2.Beats("m1"));
	ASSERT_EQUAL("test_move_semantics", 0ull, dummy1.Beats("m1"));

	dummy1.Beat("m1");
	ASSERT_EQUAL("test_move_semantics", 1ull, dummy1.Beats("m1"));

	Dummy dummy3;
	dummy3 = std::move(dummy2);
	ASSERT_EQUAL("test_move_semantics", 1ull, dummy3.Beats("m1"));
	ASSERT_EQUAL("test_move_semantics", 0ull, dummy2.Beats("m1"));
	return 0;
}

static int test_multithread_same_key_independent_samples() {
	Dummy dummy;
	constexpr int thread_count = 8;
	constexpr int iterations = 100;

	std::vector<std::thread> threads;
	for (int thread_index = 0; thread_index < thread_count; ++thread_index) {
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
	ASSERT_EQUAL("test_multithread_same_key_independent_samples", static_cast<std::uint64_t>(thread_count * iterations), values.Count);
	ASSERT_TRUE("test_multithread_same_key_independent_samples", values.Time.count() > 0);
	ASSERT_TRUE("test_multithread_same_key_independent_samples", values.MeanDuration.count() > 0);
	return 0;
}

int main() {
	int result = 0;

	result += test_clock_two_beats();
	result += test_empty_sample_stop_is_noop();
	result += test_nested_samples_are_independent();
	result += test_const_clock_missing_and_lazy_insert();
	result += test_string_conversions();
	result += test_move_semantics();
	result += test_multithread_same_key_independent_samples();

	if (result == 0)
		std::cout << "All telemetry tests passed!" << std::endl;
	else
		std::cout << result << " telemetry tests failed." << std::endl;

	return result;
}
