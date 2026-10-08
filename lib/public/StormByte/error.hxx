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

#include <concepts>
#include <string>
#include <system_error>
#include <type_traits>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Error
	 * @brief Error-code domains and the @ref StormByte::Error::Fault value type.
	 *
	 * Modules specialize @ref StormByte::Error::Domain for their enums. The category
	 * singleton for each enum lives in that module's `.cxx`.
	 * @ref StormByte::Exception is unrelated: throw / Expected stay there.
	 */
	namespace Error {
		/**
		 * @brief Description of one error-code enum.
		 * @tparam Enum Scoped enum used as `std::error_code` value.
		 *
		 * Specialize in the module that owns @p Enum.
		 * `Name` is a stable NUL-terminated tag.
		 */
		template<typename Enum>
		struct Domain;

		/**
		 * @brief Enum that has a @ref StormByte::Error::Domain specialization.
		 * @tparam Enum Candidate enum.
		 */
		template<typename Enum>
		concept Described = std::is_enum_v<Enum>
			&& requires(Enum e) {
				{ Domain<Enum>::Name } -> std::convertible_to<const char*>;
				{ Domain<Enum>::Message(e) } -> std::convertible_to<std::string>;
			};

		/**
		 * @class Category
		 * @brief `std::error_category` backed by @ref StormByte::Error::Domain.
		 * @tparam Enum Enum described by @ref StormByte::Error::Domain.
		 *
		 * Method bodies are in `error.txx`. Include that file only
		 * from a `.cxx` and keep the singleton there.
		 */
		template<Described Enum>
		class Category final: public std::error_category {
			public:
				/**
				 * @brief Category name (`Domain<Enum>::Name`).
				 * @return Stable C string.
				 */
				const char* name() const noexcept override;

				/**
				 * @brief Message for the enumerator stored in @p ev.
				 * @param ev Integer value of @p Enum.
				 * @return Text from `Domain<Enum>::Message`.
				 */
				std::string message(int ev) const override;
		};

		/**
		 * @enum Code
		 * @brief Suite-level codes that are not module-specific.
		 */
		enum class Code {
			Success = 0,	///< No error
			Unknown			///< Unclassified suite error
		};

		/**
		 * @brief Domain for @ref StormByte::Error::Code.
		 */
		template<>
		struct Domain<Code> {
			static constexpr const char* Name = "StormByte";	///< Stable category tag

			/**
			 * @brief Text for one suite enumerator.
			 * @param e Enumerator.
			 * @return Human-readable message.
			 */
			static std::string Message(Code e) {
				switch (e) {
					case Code::Success:
						return "Success";
					case Code::Unknown:
						return "Unknown StormByte error";
				}
				return "Unknown StormByte error";
			}
		};

		/**
		 * @brief Process-wide category for @ref StormByte::Error::Code.
		 * @return Category singleton (defined in error.cxx).
		 */
		STORMBYTE_PUBLIC const Category<Code>& category() noexcept;

		/**
		 * @brief Builds an `std::error_code` from @ref StormByte::Error::Code.
		 * @param e Suite enumerator.
		 * @return Code in the suite category.
		 *
		 * Lives in this namespace so ADL finds it for `std::error_code{Code}`.
		 */
		STORMBYTE_PUBLIC std::error_code make_error_code(Code e) noexcept;

		/**
		 * @class Fault
		 * @brief Held error: an `std::error_code` and a @ref StormByte::Safe::String message.
		 *
		 * Used as object state (`File`, tube `Fail`). Not thrown.
		 * `operator bool` is true when the code is an error.
		 */
		class STORMBYTE_PUBLIC Fault {
			public:
				/**
				 * @name Life
				 * @{
				 */

				/**
				 * @brief Success.
				 */
				Fault() noexcept;

				/**
				 * @brief From an `std::error_code`.
				 * @param code Code to hold.
				 */
				explicit Fault(const std::error_code& code);

				/**
				 * @brief From a described enumerator.
				 * @tparam Enum Enum described by @ref StormByte::Error::Domain.
				 * @param e Enumerator.
				 *
				 * Uses ADL `make_error_code(e)` so the module singleton
				 * category is the one stored in the code.
				 */
				template<Described Enum>
				explicit Fault(Enum e): Fault(make_error_code(e)) {}

				/**
				 * @brief Copy constructor.
				 * @param other Fault to copy.
				 */
				Fault(const Fault& other) = default;

				/**
				 * @brief Move constructor.
				 * @param other Fault to take.
				 */
				Fault(Fault&& other) noexcept = default;

				/**
				 * @brief Destructor.
				 */
				~Fault() noexcept = default;

				/**
				 * @brief Copy assignment.
				 * @param other Fault to copy.
				 * @return *this.
				 */
				Fault& operator=(const Fault& other) = default;

				/**
				 * @brief Move assignment.
				 * @param other Fault to take.
				 * @return *this.
				 */
				Fault& operator=(Fault&& other) noexcept = default;

				/** @} */

				/**
				 * @name Observers
				 * @{
				 */

				/**
				 * @brief Held code.
				 * @return Code.
				 */
				const std::error_code& code() const noexcept;

				/**
				 * @brief Owned message (`category: message`).
				 * @return NUL-terminated text owned by this object.
				 */
				const char* what() const noexcept;

				/**
				 * @brief Whether this is an error.
				 * @return true if the held code is non-zero.
				 */
				explicit operator bool() const noexcept;

				/** @} */

			private:
				std::error_code m_code;	///< Held code
				Safe::String m_what;			///< Owned message
		};
	}
}

namespace std {
	/**
	 * @brief Marks `StormByte::Error::Code` as an `std::error_code` enum.
	 */
	template<>
	struct is_error_code_enum<StormByte::Error::Code>: true_type {};
}
