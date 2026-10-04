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

#include <StormByte/safe/owner.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/type_traits/safe.hxx>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
	 */
	namespace Safe {
		/**
		 * @class Callback
		 * @brief Move-only callback with explicit owned context and creator-module release.
		 *
		 * Invoke borrows UTF-8 text for the duration of the call. Retained text must be
		 * copied into a Safe owner. Function pointers use the project's default C++
		 * calling convention; all participants must agree. No exceptions may escape
		 * Invoke or Release. Keep the callback's module and Base loaded until release.
		 * Thread safety and reentrancy are the provider's responsibility. A callback
		 * must not destroy itself during invocation. This is not std::function.
		 */
		class STORMBYTE_PUBLIC Callback final {
			public:
				/**
				 * @brief Invoke with owned context and borrowed text.
				 */
				using Invoke = Status (*)(void*, const String&) noexcept;

				/**
				 * @brief Release the context in its creator module exactly once.
				 */
				using Release = void (*)(void*) noexcept;

				/**
				 * @brief Adopt a non-null context with non-null callbacks.
				 * @param context Owned context.
				 * @param invoke Invocation callback.
				 * @param release Destruction callback.
				 * @throws StormByte::Exception Invalid arguments; ownership is not transferred.
				 */
				Callback(void* context, Invoke invoke, Release release);

				Callback(const Callback&) = delete;
				Callback& operator=(const Callback&) = delete;

				/**
				 * @brief Transfer context; source becomes empty.
				 * @param other Source.
				 */
				Callback(Callback&& other) noexcept;

				/**
				 * @brief Release old context and transfer.
				 * @param other Source.
				 * @return This callback.
				 */
				Callback& operator=(Callback&& other) noexcept;

				/**
				 * @brief Invoke the creator-module release callback.
				 */
				~Callback() noexcept;

				/**
				 * @brief Test whether a context is held.
				 * @return False after move.
				 */
				bool HasValue() const noexcept;

				/**
				 * @brief Invoke with borrowed UTF-8 text.
				 * @param value Text valid for this call.
				 * @return Provider status, or Missing after move.
				 */
				Status Call(const String& value) const noexcept;

			private:
				Detail::Owner m_owner;	///< Context released only in the provider module.
				Invoke m_invoke;		///< Provider invocation callback.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes explicit callback ownership, not a copyable collection value.
		 */
		template<>
		struct IsSafe<Safe::Callback>: std::true_type {};
	}
}