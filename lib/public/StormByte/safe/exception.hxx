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

#include <StormByte/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/visibility.h>

#include <format>
#include <string_view>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Safe
	 * @brief Owned values that cross a DLL without the caller's CRT.
	 */
	namespace Safe {
		/**
		 * @class Exception
		 * @brief Base exception for Safe. Leaf text is `StormByte.Safe: message`.
		 *
		 * The body does not include the path. This type applies the `Safe` segment. Copy, move and the destructor are defined in the Base DLL. A leaf that must not allocate uses the empty constructor and overrides `what()`. The message constructor takes a view. A @ref String converts to that view.
		 */
		class STORMBYTE_PUBLIC Exception: public StormByte::Exception {
			public:
				/**
				 * @brief Copy a body into `StormByte.Safe: message`.
				 * @param message Body. Not a path and not a format string.
				 */
				explicit Exception(std::string_view message);

				/**
				 * @brief Format a body into `StormByte.Safe: formatted`.
				 * @tparam Args Format argument types.
				 * @param fmt Format string. Body only.
				 * @param args Format arguments.
				 */
				template <typename... Args>
				Exception(std::format_string<Args...> fmt, Args&&... args)
					: StormByte::Exception(StormByte::Exception::Path{"Safe"}, fmt, std::forward<Args>(args)...) {}

				/**
				 * @brief Copy constructor. Defined in the Base DLL.
				 * @param other Exception to copy.
				 */
				Exception(const Exception& other);

				/**
				 * @brief Move constructor. Defined in the Base DLL.
				 * @param other Exception to take.
				 */
				Exception(Exception&& other) noexcept;

				/**
				 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
				 */
				~Exception() noexcept override;

				/**
				 * @brief Copy assignment. Defined in the Base DLL.
				 * @param other Exception to copy.
				 * @return This exception.
				 */
				Exception& operator=(const Exception& other);

				/**
				 * @brief Move assignment. Defined in the Base DLL.
				 * @param other Exception to take.
				 * @return This exception.
				 */
				Exception& operator=(Exception&& other) noexcept;

			protected:
				/**
				 * @brief Construct without allocating message storage.
				 *
				 * Used by a leaf that overrides `what()` with a static string.
				 */
				Exception() noexcept;
		};

		/**
		 * @class AllocationError
		 * @brief Memory allocation failed under Safe.
		 *
		 * Constructing this exception does not allocate. `what()` returns static text. The path is part of that literal because this leaf does not fill the owned message.
		 */
		class STORMBYTE_PUBLIC AllocationError: public Exception {
			public:
				/**
				 * @brief Construct an allocation failure. No message storage.
				 */
				AllocationError() noexcept;

				/**
				 * @brief Copy constructor. Defined in the Base DLL.
				 * @param other Exception to copy.
				 */
				AllocationError(const AllocationError& other) noexcept;

				/**
				 * @brief Move constructor. Defined in the Base DLL.
				 * @param other Exception to take.
				 */
				AllocationError(AllocationError&& other) noexcept;

				/**
				 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
				 */
				~AllocationError() noexcept override;

				/**
				 * @brief Copy assignment. Defined in the Base DLL.
				 * @param other Exception to copy.
				 * @return This exception.
				 */
				AllocationError& operator=(const AllocationError& other) noexcept;

				/**
				 * @brief Move assignment. Defined in the Base DLL.
				 * @param other Exception to take.
				 * @return This exception.
				 */
				AllocationError& operator=(AllocationError&& other) noexcept;

				/**
				 * @brief Return the static message.
				 * @return `StormByte.Safe: Memory allocation failed`.
				 */
				const char* what() const noexcept override;
		};

		/**
		 * @class ExpiredWeakPointerError
		 * @brief A shared owner cannot be obtained from an empty or expired observer.
		 *
		 * The message is the body only. The path `Safe` is applied by @ref Exception.
		 */
		class STORMBYTE_PUBLIC ExpiredWeakPointerError: public Exception {
			public:
				/**
				 * @brief Copy a body into `StormByte.Safe: message`.
				 * @param message Body. Not a path and not a format string.
				 */
				explicit ExpiredWeakPointerError(std::string_view message);

				/**
				 * @brief Format a body into `StormByte.Safe: formatted`.
				 * @tparam Args Format argument types.
				 * @param fmt Format string. Body only.
				 * @param args Format arguments.
				 */
				template <typename... Args>
				ExpiredWeakPointerError(std::format_string<Args...> fmt, Args&&... args)
					: Exception(fmt, std::forward<Args>(args)...) {}

				/**
				 * @brief Copy constructor. Defined in the Base DLL.
				 * @param other Exception to copy.
				 */
				ExpiredWeakPointerError(const ExpiredWeakPointerError& other);

				/**
				 * @brief Move constructor. Defined in the Base DLL.
				 * @param other Exception to take.
				 */
				ExpiredWeakPointerError(ExpiredWeakPointerError&& other) noexcept;

				/**
				 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
				 */
				~ExpiredWeakPointerError() noexcept override;

				/**
				 * @brief Copy assignment. Defined in the Base DLL.
				 * @param other Exception to copy.
				 * @return This exception.
				 */
				ExpiredWeakPointerError& operator=(const ExpiredWeakPointerError& other);

				/**
				 * @brief Move assignment. Defined in the Base DLL.
				 * @param other Exception to take.
				 * @return This exception.
				 */
				ExpiredWeakPointerError& operator=(ExpiredWeakPointerError&& other) noexcept;
		};

		/**
		 * @class OutOfBoundsError
		 * @brief An index or range is out of bounds.
		 *
		 * The message is the body only. The path `Safe` is applied by @ref Exception.
		 */
		class STORMBYTE_PUBLIC OutOfBoundsError: public Exception {
			public:
				/**
				 * @brief Copy a body into `StormByte.Safe: message`.
				 * @param message Body. Not a path and not a format string.
				 */
				explicit OutOfBoundsError(std::string_view message);

				/**
				 * @brief Format a body into `StormByte.Safe: formatted`.
				 * @tparam Args Format argument types.
				 * @param fmt Format string. Body only.
				 * @param args Format arguments.
				 */
				template <typename... Args>
				OutOfBoundsError(std::format_string<Args...> fmt, Args&&... args)
					: Exception(fmt, std::forward<Args>(args)...) {}

				/**
				 * @brief Copy constructor. Defined in the Base DLL.
				 * @param other Exception to copy.
				 */
				OutOfBoundsError(const OutOfBoundsError& other);

				/**
				 * @brief Move constructor. Defined in the Base DLL.
				 * @param other Exception to take.
				 */
				OutOfBoundsError(OutOfBoundsError&& other) noexcept;

				/**
				 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
				 */
				~OutOfBoundsError() noexcept override;

				/**
				 * @brief Copy assignment. Defined in the Base DLL.
				 * @param other Exception to copy.
				 * @return This exception.
				 */
				OutOfBoundsError& operator=(const OutOfBoundsError& other);

				/**
				 * @brief Move assignment. Defined in the Base DLL.
				 * @param other Exception to take.
				 * @return This exception.
				 */
				OutOfBoundsError& operator=(OutOfBoundsError&& other) noexcept;
		};

		/**
		 * @class BadOptionalAccess
		 * @brief An empty Optional was dereferenced.
		 *
		 * Constructing this exception does not allocate. `what()` returns static text. The path is part of that literal because this leaf does not fill the owned message.
		 */
		class STORMBYTE_PUBLIC BadOptionalAccess: public Exception {
			public:
				/**
				 * @brief Construct an empty-optional failure. No message storage.
				 */
				BadOptionalAccess() noexcept;

				/**
				 * @brief Copy constructor. Defined in the Base DLL.
				 * @param other Exception to copy.
				 */
				BadOptionalAccess(const BadOptionalAccess& other) noexcept;

				/**
				 * @brief Move constructor. Defined in the Base DLL.
				 * @param other Exception to take.
				 */
				BadOptionalAccess(BadOptionalAccess&& other) noexcept;

				/**
				 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
				 */
				~BadOptionalAccess() noexcept override;

				/**
				 * @brief Copy assignment. Defined in the Base DLL.
				 * @param other Exception to copy.
				 * @return This exception.
				 */
				BadOptionalAccess& operator=(const BadOptionalAccess& other) noexcept;

				/**
				 * @brief Move assignment. Defined in the Base DLL.
				 * @param other Exception to take.
				 * @return This exception.
				 */
				BadOptionalAccess& operator=(BadOptionalAccess&& other) noexcept;

				/**
				 * @brief Return the static message.
				 * @return `StormByte.Safe: Optional has no value`.
				 */
				const char* what() const noexcept override;
		};
	}
}
