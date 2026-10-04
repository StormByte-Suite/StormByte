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

#pragma once

#include <StormByte/safe/string.hxx>
#include <StormByte/platform.h>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/visibility.h>

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class Clock
	 * @brief Thread-safe aggregate of independently timed operation samples.
	 *
	 * Not a telemetry type and not inherited by leaves.
	 * Concurrent samples may share this Clock; each sample owns its own start time
	 * and completion is accumulated under the shared state lock. A Sample token
	 * itself has one owner and must not be accessed concurrently.
	 */
	class STORMBYTE_PUBLIC Clock final {
		private:
			/**
			 * @struct State
			 * @brief Shared aggregate counters for completed samples.
			 */
			struct State;

		public:
			/**
			 * @struct Values
			 * @brief Coherent snapshot of completed measurements.
			 */
			struct Values {
				std::uint64_t Count{}; ///< Number of completed samples.
				std::chrono::microseconds Time{}; ///< Sum of completed sample durations.
				std::chrono::microseconds MeanDuration{}; ///< Mean duration, or zero when empty.
			};

			/**
			 * @class Sample
			 * @brief One independently timed interval contributing to a shared Clock.
			 *
			 * Each sample owns its start time. Samples from the same Clock may overlap
			 * or nest on any threads without replacing one another. Stop is idempotent;
			 * an active sample is recorded on explicit Stop or destruction. A Sample
			 * is a single-owner token and must not be accessed concurrently by multiple
			 * threads. It may be moved to the thread that will stop it.
			 */
			class STORMBYTE_PUBLIC Sample final {
				public:
					/**
					 * @brief Construct an empty, inactive sample token.
				 */
					Sample() noexcept = default;

					/**
					 * @brief Copy construction is disabled for single-owner sample tokens.
					 * @param other Source sample; it remains owned by its current token.
					 */
					Sample(const Sample& other) = delete;

					/**
					 * @brief Copy assignment is disabled for single-owner sample tokens.
					 * @param other Source sample.
					 * @return No value; this operation is deleted.
					 */
					Sample& operator=(const Sample& other) = delete;

					/**
					 * @brief Transfer responsibility for recording the active sample.
					 * @param other Source sample; it becomes inactive.
					 */
					Sample(Sample&& other) noexcept;

					/**
					 * @brief Record this sample and take responsibility for another.
					 * @param other Source sample; it becomes inactive.
					 * @return This sample token.
					 */
					Sample& operator=(Sample&& other) noexcept;

					/**
					 * @brief Record this sample if it is still active.
					 */
					~Sample() noexcept;

					/**
					 * @brief Whether this token owns a measurement that will be recorded.
					 * @return False for an empty token, after Stop, or when best-effort creation failed.
					 */
					bool Active() const noexcept;

					/**
					 * @brief Complete and record this sample once.
					 * @return This sample's elapsed microseconds; repeated calls return the same value.
					 */
					std::chrono::microseconds Stop() noexcept;

				private:
					friend class Clock;

					/**
					 * @brief Start an interval sample against shared aggregate state.
					 * @param state Shared state updated when the sample completes.
					 */
					explicit Sample(Safe::Shared<State> state) noexcept;

					Safe::Shared<State> m_state; ///< Keeps the aggregate state alive until this sample ends.
					std::chrono::steady_clock::time_point m_start{}; ///< This sample's independent start.
					std::chrono::microseconds m_elapsed{}; ///< Cached duration after Stop.
					bool m_active{false}; ///< Whether this token still contributes a sample.
			};

			/**
			 * @brief Construct zeroed shared clock state.
			 */
			Clock() noexcept;

			/**
			 * @brief Copy constructor is deleted.
			 */
			Clock(const Clock&) = delete;

			/**
			 * @brief Move constructor is defaulted.
			 */
			Clock(Clock&&) noexcept;

			/**
			 * @brief Copy assignment is deleted.
			 * @return Reference to this clock.
			 */
			Clock& operator=(const Clock&) = delete;

			/**
			 * @brief Move assignment is defaulted.
			 * @return Reference to this clock.
			 */
			Clock& operator=(Clock&&) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Clock() noexcept;

			/**
			 * @brief Start an independent interval sample.
			 * @return Move-only token; Stop it explicitly or let its destructor record it.
			 */
			Sample Measure();

			/**
			 * @brief Read Count, Time and MeanDuration under one lock.
			 * @return Coherent completed-sample snapshot.
			 */
			Values GetValues() const noexcept;

			/**
			 * @brief Number of completed samples.
			 * @return Completed operation count.
			 */
			std::uint64_t Count() const noexcept;

			/**
			 * @brief Cumulative duration of completed samples.
			 * @return Elapsed microseconds.
			 */
			std::chrono::microseconds Time() const noexcept;

			/**
			 * @brief Mean duration across completed samples.
			 * @return Average microseconds per operation, or 0 when Count() == 0.
			 */
			std::chrono::microseconds MeanDuration() const noexcept;

		private:
			mutable std::mutex m_state_lock; ///< Protects lazy state creation and clock moves.
			Safe::Shared<State> m_state; ///< Shared aggregate state retained by active samples.
	};

	/**
	 * @class Telemetry
	 * @brief Abstract session telemetry base with a protected clock drawer.
	 *
	 * Modules derive and extend this class by adding their own domain counters
	 * or metrics. Time series and stopwatches are managed in a protected,
	 * named clock drawer. Use @ref MeasureClock to obtain an independent
	 * concurrent sample; the returned token owns its own start time.
	 */
	class STORMBYTE_PUBLIC Telemetry {
		public:
			/**
			 * @brief Virtual destructor. Defined out-of-line for DLL boundaries.
			 */
			virtual ~Telemetry() noexcept;

			/**
			 * @brief Flatten counters into an owned StormByte Safe::String.
			 * @return Formatted telemetry string.
			 */
			virtual operator Safe::String() const = 0;

			/**
			 * @brief Flatten counters into a caller-owned standard string.
			 * @return Formatted standard string allocated on caller heap.
			 */
			STORMBYTE_FORCE_INLINE operator std::string() const {
				return static_cast<std::string>(static_cast<Safe::String>(*this));
			}

		protected:
			/**
			 * @brief Construct an empty telemetry drawer.
			 */
			Telemetry() noexcept;

			/**
			 * @brief Copy constructor is deleted.
			 */
			Telemetry(const Telemetry&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Object to move from.
			 */
			Telemetry(Telemetry&& other) noexcept;

			/**
			 * @brief Copy assignment is deleted.
			 * @return Reference to this telemetry object.
			 */
			Telemetry& operator=(const Telemetry&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Object to move from.
			 * @return Reference to this telemetry object.
			 */
			Telemetry& operator=(Telemetry&& other) noexcept;

			/**
			 * @brief Access or create a named clock in the drawer.
			 * @param name Unique clock name.
			 * @return Reference to the requested aggregate clock. Use MeasureClock to time an operation.
			 * @throws StormByte::AllocationError If a new name or clock cannot be allocated.
			 * @throws StormByte::OperationError If clock creation fails for another reason.
			 */
			class Clock& Clock(std::string_view name);

			/**
			 * @brief Start an independent sample on a named aggregate clock.
			 * @param name Stable metric name.
			 * @return Active sample token; inactive if telemetry storage allocation fails.
			 * @note Best-effort and non-throwing so it can be used in no-throw instrumentation paths.
			 */
			class Clock::Sample MeasureClock(std::string_view name) noexcept;

			/**
			 * @brief Read-only access to a named clock in the drawer.
			 * @param name Clock name.
			 * @return Reference to the clock if found, or a static empty clock if missing.
			 */
			const class Clock& Clock(std::string_view name) const;

		private:
			struct Store;
			Safe::Unique<Store> m_store;	///< PIMPL store for clock drawer.
	};
}

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Coherent clock snapshots contain only fixed-width numeric duration values.
		 */
		template<> struct IsSafe<Clock::Values>: std::true_type {};

		/**
		 * @brief Clock samples carry provider-managed shared state and require compatible STL ABI.
		 */
		template<> struct IsMaybeSafe<Clock::Sample>: std::true_type {};
	}
}
