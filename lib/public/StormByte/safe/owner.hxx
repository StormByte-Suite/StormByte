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

#include <StormByte/visibility.h>

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
		 * @enum Status
		 * @brief Result of a collection operation; exceptions never leave its owner callback.
		 */
		enum class Status : unsigned char {
			Success,	///< Operation completed.
			Missing,	///< Index or key is absent.
			Failure		///< Allocation, copying or comparison failed; no exception crosses the callback.
		};

		/**
		 * @namespace StormByte::Safe::Detail
		 * @brief Private helpers. Not a supported API.
		 */
		namespace Detail {
			/**
			 * @brief Throw Base's exception for a failed Safe conversion.
			 * @param message Failure description.
			 */
			STORMBYTE_PUBLIC void ThrowSafeConversionFailure(const char* message);

			/**
			 * @class Owner
			 * @brief Opaque unique state and creator-module lifetime callbacks.
			 * @note The creator and Base must remain loaded until every copy is destroyed.
			 */
			class STORMBYTE_PUBLIC Owner final {
				public:
					/**
					 * @brief Creator-module deep-copy callback; null reports failure.
					 */
					using Clone = void* (*)(const void*) noexcept;

					/**
					 * @brief Creator-module destruction callback.
					 */
					using Destroy = void (*)(void*) noexcept;

					/**
					 * @brief Adopt state and callbacks.
					 * @param state Owned state.
					 * @param clone Copy callback.
					 * @param destroy Release callback.
					 */
					Owner(void* state, Clone clone, Destroy destroy) noexcept;

					/**
					 * @brief Deep copy in the original module.
					 * @param other Source.
					 * @throws StormByte::Exception Copy failure.
					 */
					Owner(const Owner& other);

					/**
					 * @brief Transfer state; source becomes empty.
					 * @param other Source.
					 */
					Owner(Owner&& other) noexcept;

					/**
					 * @brief Release exclusively through the original callback.
					 */
					~Owner() noexcept;

					/**
					 * @brief Strong-guarantee deep copy.
					 * @param other Source.
					 * @return This owner.
					 */
					Owner& operator=(const Owner& other);

					/**
					 * @brief Release old state and transfer.
					 * @param other Source.
					 * @return This owner.
					 */
					Owner& operator=(Owner&& other) noexcept;

					/**
					 * @brief Borrow state internally.
					 * @return State, or null after move.
					 */
					void* Get() const noexcept;

				private:
					void* m_state;		///< Opaque state allocated in the creator module.
					Clone m_clone;		///< Callback in that module.
					Destroy m_destroy;	///< Callback in that module.
			};
		}
	}
}