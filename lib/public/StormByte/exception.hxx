/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This is the base for StormByte Suite libraries.
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
#include <iterator>
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
	 * `what()` is `StormByte: message`, or `StormByte.path: message` when a parent passes the segments under `StormByte` (`Safe`, `Crypto.Crypter`). Segments are joined with a dot. The body is a @ref StormByte::Safe::String and does not repeat the path.
	 *
	 * `std::format_to` runs in the caller's translation unit and appends into the owned string. It does not build a `std::string`. A parent passes @ref Path, a `std::string_view` that lives for the constructor call and is not stored. A bare string is not a path: that would be ambiguous with the format constructor.
	 *
	 * A parent prepends its own segment and forwards the format and the arguments. It does not format. A final leaf adds no segment: it inherits the parent constructors. Copy, move and the destructor of each named type are defined in that module's `.cxx`, so the `typeinfo` is unique across a DLL.
	 *
	 * A leaf that must not allocate uses the empty constructor and overrides @ref what. That leaf does not fill `m_what`. The message constructor takes a view. A @ref Safe::String converts to that view, so a second constructor would be ambiguous with a literal.
	 */
	class STORMBYTE_PUBLIC Exception {
		public:
			/**
			 * @brief Copies text into `StormByte: message`.
			 * @param message Exception text. Not a format string. A @ref Safe::String converts to this view.
			 */
			explicit Exception(std::string_view message);

			/**
			 * @brief Constructs with `std::format_to`. Text is `StormByte: formatted`.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 * @note With zero arguments the format string is the message as-is.
			 */
			template <typename... Args>
			Exception(std::format_string<Args...> fmt, Args&&... args)
				: Exception(Path{}, fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Copy constructor. Defined in the Base DLL.
			 * @param other Exception to copy.
			 */
			Exception(const Exception& other);

			/**
			 * @brief Move constructor. Defined in the Base DLL.
			 * @param other Exception to take. @p other stays valid.
			 */
			Exception(Exception&& other) noexcept;

			/**
			 * @brief Destructor. Defined in the Base DLL: this anchors the `typeinfo`.
			 */
			virtual ~Exception() noexcept;

			/**
			 * @brief Copy assignment. Defined in the Base DLL.
			 * @param other Exception to copy.
			 * @return This exception.
			 */
			Exception& operator=(const Exception& other);

			/**
			 * @brief Move assignment. Defined in the Base DLL.
			 * @param other Exception to take. @p other stays valid.
			 * @return This exception.
			 */
			Exception& operator=(Exception&& other) noexcept;

			/**
			 * @brief Message pointer. A leaf may override this and return static storage.
			 * @return NUL-terminated message. Never null.
			 */
			virtual const char* what() const noexcept;

		protected:
			/**
			 * @brief Segments under `StormByte`, already joined with a dot.
			 *
			 * Exists so a path cannot be mistaken for a format string. The view must live for the constructor call. It is not stored.
			 */
			struct Path {
				std::string_view text;	///< Joined segments. Empty means the root.

				/**
				 * @brief Wraps @p value.
				 * @param value Joined segments, or empty.
				 */
				explicit constexpr Path(std::string_view value = {}) noexcept: text(value) {}
			};

			/**
			 * @brief Construct without allocating message storage.
			 *
			 * Used by a leaf that overrides @ref what with a static string.
			 */
			Exception() noexcept;

			/**
			 * @brief Stores `StormByte.path: message`, or `StormByte: message` when @p path is empty.
			 * @tparam Args Format argument types.
			 * @param path Segments under `StormByte`.
			 * @param fmt Format string. The body only, not the path.
			 * @param args Format arguments.
			 * @note With zero arguments the format string is the message as-is. The body is appended to `m_what`. No `std::string` is created.
			 */
			template <typename... Args>
			Exception(Path path, std::format_string<Args...> fmt, Args&&... args) {
				if (path.text.empty())
					m_what.append("StormByte: ");
				else {
					m_what.append("StormByte.");
					m_what.append(path.text);
					m_what.append(": ");
				}
				if constexpr (sizeof...(Args) == 0)
					m_what.append(fmt.get());
				else {
					struct Append {
						using iterator_concept = std::output_iterator_tag;
						using iterator_category = std::output_iterator_tag;
						using difference_type = std::ptrdiff_t;
						using value_type = void;
						using pointer = void;
						using reference = void;

						Safe::String* target;	///< Message receiving each formatted byte.

						Append& operator=(char value) {
							target->push_back(value);
							return *this;
						}

						Append& operator*() { return *this; }
						Append& operator++() { return *this; }
						Append operator++(int) { return *this; }
					};
					std::format_to(Append{&m_what}, fmt, std::forward<Args>(args)...);
				}
			}

		private:
			Safe::String m_what;	///< Owned message. Empty when @ref what is overridden.
	};

	/**
	 * @class DeserializeError
	 * @brief Deserialization failed. Leaf of the root: `StormByte: …`.
	 *
	 * The message is the body only. No extra path segment is added.
	 */
	class STORMBYTE_PUBLIC DeserializeError: public Exception {
		public:
			/**
			 * @brief Copy a body into `StormByte: message`.
			 * @param message Body. Not a path and not a format string.
			 */
			explicit DeserializeError(std::string_view message);

			/**
			 * @brief Format a body into `StormByte: formatted`.
			 * @tparam Args Format argument types.
			 * @param fmt Format string. Body only.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			DeserializeError(std::format_string<Args...> fmt, Args&&... args)
				: Exception(fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Copy constructor. Defined in the Base DLL.
			 * @param other Exception to copy.
			 */
			DeserializeError(const DeserializeError& other);

			/**
			 * @brief Move constructor. Defined in the Base DLL.
			 * @param other Exception to take.
			 */
			DeserializeError(DeserializeError&& other) noexcept;

			/**
			 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
			 */
			~DeserializeError() noexcept override;

			/**
			 * @brief Copy assignment. Defined in the Base DLL.
			 * @param other Exception to copy.
			 * @return This exception.
			 */
			DeserializeError& operator=(const DeserializeError& other);

			/**
			 * @brief Move assignment. Defined in the Base DLL.
			 * @param other Exception to take.
			 * @return This exception.
			 */
			DeserializeError& operator=(DeserializeError&& other) noexcept;
	};

	/**
	 * @class OperationError
	 * @brief An operation failed with an exception outside the StormByte hierarchy. Leaf of the root: `StormByte: …`.
	 *
	 * The message is the body only. No extra path segment is added.
	 */
	class STORMBYTE_PUBLIC OperationError: public Exception {
		public:
			/**
			 * @brief Copy a body into `StormByte: message`.
			 * @param message Body. Not a path and not a format string.
			 */
			explicit OperationError(std::string_view message);

			/**
			 * @brief Format a body into `StormByte: formatted`.
			 * @tparam Args Format argument types.
			 * @param fmt Format string. Body only.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			OperationError(std::format_string<Args...> fmt, Args&&... args)
				: Exception(fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Copy constructor. Defined in the Base DLL.
			 * @param other Exception to copy.
			 */
			OperationError(const OperationError& other);

			/**
			 * @brief Move constructor. Defined in the Base DLL.
			 * @param other Exception to take.
			 */
			OperationError(OperationError&& other) noexcept;

			/**
			 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
			 */
			~OperationError() noexcept override;

			/**
			 * @brief Copy assignment. Defined in the Base DLL.
			 * @param other Exception to copy.
			 * @return This exception.
			 */
			OperationError& operator=(const OperationError& other);

			/**
			 * @brief Move assignment. Defined in the Base DLL.
			 * @param other Exception to take.
			 * @return This exception.
			 */
			OperationError& operator=(OperationError&& other) noexcept;
	};

	/**
	 * @class Base64Error
	 * @brief Base64 encode or decode failed. Leaf of the root: `StormByte: …`.
	 *
	 * The message is the body only. No extra path segment is added.
	 */
	class STORMBYTE_PUBLIC Base64Error: public Exception {
		public:
			/**
			 * @brief Copy a body into `StormByte: message`.
			 * @param message Body. Not a path and not a format string.
			 */
			explicit Base64Error(std::string_view message);

			/**
			 * @brief Format a body into `StormByte: formatted`.
			 * @tparam Args Format argument types.
			 * @param fmt Format string. Body only.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Base64Error(std::format_string<Args...> fmt, Args&&... args)
				: Exception(fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Copy constructor. Defined in the Base DLL.
			 * @param other Exception to copy.
			 */
			Base64Error(const Base64Error& other);

			/**
			 * @brief Move constructor. Defined in the Base DLL.
			 * @param other Exception to take.
			 */
			Base64Error(Base64Error&& other) noexcept;

			/**
			 * @brief Destructor. Defined in the Base DLL so `catch` matches across modules.
			 */
			~Base64Error() noexcept override;

			/**
			 * @brief Copy assignment. Defined in the Base DLL.
			 * @param other Exception to copy.
			 * @return This exception.
			 */
			Base64Error& operator=(const Base64Error& other);

			/**
			 * @brief Move assignment. Defined in the Base DLL.
			 * @param other Exception to take.
			 * @return This exception.
			 */
			Base64Error& operator=(Base64Error&& other) noexcept;
	};
}
