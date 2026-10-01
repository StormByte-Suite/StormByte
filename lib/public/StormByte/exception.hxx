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
#include <StormByte/visibility.h>

#include <format>
#include <string>
#include <string_view>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class Exception
	 * @brief Base exception type for the suite.
	 *
	 * `what()` is `StormByte: message`, or `StormByte.path: message` when
	 * a module parent passes the segments under `StormByte` (`Crypto.Crypter`).
	 * Segments are joined with a dot. The message is a @ref StormByte::Safe::String.
		 * `std::format` runs in the caller's translation unit. The result is copied
		 * into an owned string. A parent passes @ref StormByte::Exception::Path, a
	 * `std::string_view` that lives for the constructor call and is not stored.
	 * A bare string is not a path: that would be ambiguous with the format constructor.
	 *
	 * A parent prepends its own segment and forwards the format and the
	 * arguments. It does not format. A final leaf adds no segment: it inherits
	 * the parent constructors. The destructor of each named type is defined in
	 * that module's `.cxx`, so the `typeinfo` is unique across a DLL.
	 */
	class STORMBYTE_PUBLIC Exception {
		public:
			/**
			 * @brief Copies text into `StormByte: message`.
			 * @param message Exception text. Not a format string.
			 */
			explicit Exception(std::string_view message);

			/**
			 * @brief Copies owned text into `StormByte: message`.
			 * @param message Exception text. Not a format string.
			 */
			explicit Exception(const Safe::String& message);

		protected:
			/**
			 * @brief Segments under `StormByte`, already joined with a dot.
			 *
			 * Exists so a path cannot be mistaken for a format string.
			 * The view must live for the constructor call. It is not stored.
			 */
			struct Path {
				std::string_view text;	///< Joined segments. Empty means the root.

				/**
				 * @brief Wraps @p text.
				 * @param text Joined segments, or empty.
				 */
				explicit constexpr Path(std::string_view text = {}) noexcept: text(text) {}
			};

		public:
			/**
			 * @brief Constructs with `std::format`. Text is `StormByte: formatted`.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 * @note With zero arguments the format string is the message as-is.
			 */
			template <typename... Args>
			Exception(std::format_string<Args...> fmt, Args&&... args)
				: Exception(Path{}, fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Copy constructor.
			 * @param e Exception to copy.
			 */
			Exception(const Exception& e) = default;

			/**
			 * @brief Move constructor.
			 * @param e Exception to take.
			 */
			Exception(Exception&& e) noexcept = default;

			/**
			 * @brief Destructor. Defined in the Base DLL: this anchors the `typeinfo`.
			 */
			virtual ~Exception() noexcept;

			/**
			 * @brief Copy assignment.
			 * @param e Exception to copy.
			 * @return *this.
			 */
			Exception& operator=(const Exception& e) = default;

			/**
			 * @brief Move assignment.
			 * @param e Exception to take.
			 * @return *this.
			 */
			Exception& operator=(Exception&& e) noexcept = default;

			/**
			 * @brief Message pointer.
			 * @return NUL-terminated message owned by this object.
			 */
			virtual const char* what() const noexcept;

		protected:
			/**
			 * @brief Stores `StormByte.path: message`, or `StormByte: message` when @p path is empty.
			 * @tparam Args Format argument types.
			 * @param path Segments under `StormByte`.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 * @note With zero arguments the format string is the message as-is.
			 */
			template <typename... Args>
			Exception(Path path, std::format_string<Args...> fmt, Args&&... args) {
				const std::string body = sizeof...(Args) == 0
					? std::string(fmt.get())
					: std::format(fmt, std::forward<Args>(args)...);
				if (path.text.empty())
					Assign("StormByte", body);
				else {
					const std::string full = std::string("StormByte.") + std::string(path.text);
					Assign(full, body);
				}
			}

		private:
			/**
			 * @brief Copy @p path + `: ` + @p body into @ref m_what.
			 * @param path Dotted path, including `StormByte`.
			 * @param body Message body. Already formatted, or raw.
			 */
			void Assign(std::string_view path, std::string_view body) {
				std::string full;
				full.reserve(path.size() + 2 + body.size());
				full.append(path);
				full.append(": ");
				full.append(body);
				m_what = Safe::String(std::string_view(full));
			}

			Safe::String m_what;	///< Owned message
	};

	/**
	 * @class DeserializeError
	 * @brief Thrown when deserialization fails. Leaf of the root: `StormByte: …`.
	 */
	class STORMBYTE_PUBLIC DeserializeError: public Exception {
		public:
			using Exception::Exception;

			/**
			 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
			 */
			~DeserializeError() noexcept override;
	};

	/**
	 * @class OutOfBoundsError
	 * @brief Thrown when an index or range is out of bounds. Leaf of the root: `StormByte: …`.
	 */
	class STORMBYTE_PUBLIC OutOfBoundsError: public Exception {
		public:
			using Exception::Exception;

			/**
			 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
			 */
			~OutOfBoundsError() noexcept override;
	};

	/**
	 * @class Base64Error
	 * @brief Thrown when Base64 encode or decode fails. Leaf of the root: `StormByte: …`.
	 */
	class STORMBYTE_PUBLIC Base64Error: public Exception {
		public:
			using Exception::Exception;

			/**
			 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
			 */
			~Base64Error() noexcept override;
	};
}
