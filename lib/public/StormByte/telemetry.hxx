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
#include <optional>
#include <string>
#include <string_view>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class Clock
	 * @brief Stopwatch for measuring execution time of operations.
	 *
	 * Not a telemetry type and not inherited by leaves.
	 *
	 * @note Not safe for concurrent Start/Stop on the same instance;
	 * Telemetry::Clock(name) is safe to call concurrently (distinct or same key)
	 * for map access only.
	 */
	class STORMBYTE_PUBLIC Clock final {
		public:
			/**
			 * @brief Construct zeroed clock counters.
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
			 * @brief Start measuring an interval.
			 *
			 * Stores t0 = steady_clock::now(). A second Start() without
			 * Stop() replaces t0 (no nesting).
			 */
			void Start() noexcept;

			/**
			 * @brief Stop measuring and accumulate elapsed time.
			 *
			 * When active, adds elapsed duration to Time, increments Count by 1,
			 * and clears t0. If no interval was started, this is a no-op.
			 */
			void Stop() noexcept;

			/**
			 * @brief Number of completed Start/Stop intervals.
			 * @return Completed operation count.
			 */
			std::uint64_t Count() const noexcept;

			/**
			 * @brief Cumulative duration of completed intervals.
			 * @return Elapsed microseconds.
			 */
			std::chrono::microseconds Time() const noexcept;

			/**
			 * @brief Mean duration across completed intervals.
			 * @return Average microseconds per operation, or 0 when Count() == 0.
			 */
			std::chrono::microseconds MeanDuration() const noexcept;

		private:
			std::optional<std::chrono::steady_clock::time_point> m_start;	///< Active start point, if running.
			std::chrono::microseconds m_time;	///< Accumulated microseconds.
			std::uint64_t m_count;	///< Total completed intervals.
	};

	/**
	 * @class Telemetry
	 * @brief Abstract session telemetry base with a protected clock drawer.
	 *
	 * Modules derive and extend this class by adding their own domain counters
	 * or metrics. Time series and stopwatches are managed in a protected,
	 * named clock drawer.
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
			 * @return Reference to the requested clock.
			 */
			class Clock& Clock(std::string_view name) noexcept;

			/**
			 * @brief Read-only access to a named clock in the drawer.
			 * @param name Clock name.
			 * @return Reference to the clock if found, or a static empty clock if missing.
			 */
			const class Clock& Clock(std::string_view name) const noexcept;

		private:
			struct Store;
			Safe::Unique<Store> m_store;	///< PIMPL store for clock drawer.
	};
}
