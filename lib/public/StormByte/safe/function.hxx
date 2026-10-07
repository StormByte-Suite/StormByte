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
#include <StormByte/safe/owner.hxx>
#include <StormByte/type_traits/safe.hxx>
#include <StormByte/visibility.h>

#include <type_traits>
#include <utility>

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
		 * @namespace StormByte::Safe::Detail
		 * @brief Private implementation helpers for Safe callbacks.
		 */
		namespace Detail {
			/**
			 * @brief Callback arguments admitted across a DLL boundary.
			 * @tparam T Argument type, as written in the callback signature.
			 * @note References are synchronous borrows and must be const lvalue references.
			 */
			template<typename T>
			concept SafeCallbackArgument =
				(Type::SafeComponent<std::remove_cvref_t<T>> || Type::SafeValue<std::remove_cvref_t<T>>) &&
				(!std::is_reference_v<T> || (std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>));
		}

		/**
		 * @class Function
		 * @brief Copyable, creator-context callback with a typed signature.
		 * @tparam Signature Callback signature, such as @c void(double) or @c Size(Size).
		 *
		 * The callback context is destroyed by its creator-module @p Release
		 * function. @c Call returns @ref Status for ordinary failures; value-returning
		 * signatures receive caller-owned result storage explicitly. @c Call
		 * propagates @ref StormByte::Exception unchanged. Other exceptions are
		 * converted to @ref Status::Failure. The provider must keep its callback
		 * implementation valid until the function is destroyed.
		 *
		 * Value arguments are passed for the duration of the call. Const-reference
		 * arguments are borrowed only until @c Call returns and must not be
		 * retained by the provider. Function pointers use the platform's default
		 * C++ calling convention. Participants must use a compatible C++ ABI and
		 * calling convention. The provider module and Base must remain loaded until
		 * the callback is destroyed. Invocation thread safety and reentrancy are
		 * provider responsibilities. A callback must not destroy itself while
		 * running. This is not @c std::function.
		 */
		template<typename Signature>
		class Function;

		/**
		 * @brief Typed callback specialization for void-returning signatures.
		 * @tparam Args Argument types.
		 */
		template<typename... Args>
		requires (Detail::SafeCallbackArgument<Args> && ...)
		class STORMBYTE_PUBLIC_TYPE Function<void(Args...)> final {
			public:
				/**
				 * @brief Provider invocation function pointer.
				 */
				using Invoke = Status (*)(void*, Args...);

				/**
				 * @brief Provider context clone function; null reports failure.
				 */
				using Clone = void* (*)(const void*) noexcept;

				/**
				 * @brief Provider context destruction function pointer.
				 */
				using Release = void (*)(void*) noexcept;

				/**
				 * @brief Adopt provider-owned callback context.
				 * @param context Context allocated and owned by the provider.
				 * @param invoke Provider callback. @ref StormByte::Exception may escape;
				 *        other exceptions are caught and reported as @ref Status::Failure.
				 * @param clone Provider context clone callback; it must create independent provider-owned state.
				 * @param release Provider context destructor; it must not throw.
				 * @throws StormByte::Exception If an argument is null; ownership is not transferred.
				 */
				Function(void* context, Invoke invoke, Clone clone, Release release):
					m_owner(nullptr, nullptr, nullptr), m_invoke(invoke) {
					if (!context || !invoke || !clone || !release)
						throw StormByte::Exception("Safe function requires context, invoke, clone and release");
					m_owner = Detail::Owner(context, clone, release);
				}

				/**
				 * @brief Deep-copy provider-owned context.
				 * @param other Source callback.
				 * @throws StormByte::Exception Context cloning failed.
				 */
				Function(const Function& other) = default;

				/**
				 * @brief Deep-copy provider context with the strong guarantee.
				 * @param other Source callback.
				 * @return This callback.
				 * @throws StormByte::Exception Context cloning failed.
				 */
				Function& operator=(const Function& other) = default;

				/**
				 * @brief Transfer callback context.
				 * @param other Source callback.
				 */
				Function(Function&& other) noexcept:
					m_owner(std::move(other.m_owner)), m_invoke(std::exchange(other.m_invoke, nullptr)) {}

				/**
				 * @brief Release the current context and transfer another callback.
				 * @param other Source callback.
				 * @return This callback.
				 */
				Function& operator=(Function&& other) noexcept {
					if (this != &other) {
						m_owner = std::move(other.m_owner);
						m_invoke = std::exchange(other.m_invoke, nullptr);
					}
					return *this;
				}

				/**
				 * @brief Release context through its provider callback.
				 */
				~Function() noexcept = default;

				/**
				 * @brief Whether this callback still owns a context.
				 * @return False after move.
				 */
				bool HasValue() const noexcept {
					return m_owner.Get() != nullptr;
				}

				/**
				 * @brief Invoke the provider callback.
				 * @param args Typed callback arguments.
				 * @return Provider status, @ref Status::Failure for non-Safe exceptions,
				 *         or @ref Status::Missing after move.
				 * @throws StormByte::Exception Exceptions derived from Base's exception type.
				 */
				Status Call(Args... args) const {
					if (!HasValue())
						return Status::Missing;
					try {
						return m_invoke(m_owner.Get(), std::forward<Args>(args)...);
					} catch (const StormByte::Exception&) {
						throw;
					} catch (...) {
						return Status::Failure;
					}
				}

			private:
				Detail::Owner m_owner; ///< Context released in its creator module.
				Invoke m_invoke; ///< Typed invocation function in the creator module.
		};

		/**
		 * @brief Typed callback specialization for value-returning signatures.
		 * @tparam Return Safe callback result value.
		 * @tparam Args Safe callback argument types.
		 *
		 * The result is written to caller-owned @p output only by a successful
		 * provider invocation. The caller supplies that storage explicitly so no
		 * STL result wrapper or provider allocation crosses the boundary.
		 */
		template<typename Return, typename... Args>
		requires (!std::is_void_v<Return>) && (!std::is_reference_v<Return>) &&
			Detail::SafeCallbackArgument<Return> && (Detail::SafeCallbackArgument<Args> && ...) &&
			Type::CopyConstructible<Return> &&
			requires(Return& output, Return&& result) { { output = std::move(result) } noexcept; }
		class STORMBYTE_PUBLIC_TYPE Function<Return(Args...)> final {
			public:
				/**
				 * @brief Provider invocation function pointer.
				 */
				using Invoke = Status (*)(void*, Return*, Args...);
				/**
				 * @brief Provider context clone function pointer; null reports failure.
				 */
				using Clone = void* (*)(const void*) noexcept;
				/**
				 * @brief Provider context destruction function pointer.
				 */
				using Release = void (*)(void*) noexcept;

				/**
				 * @brief Adopt provider-owned callback context.
				 * @param context Context allocated and owned by the provider.
				 * @param invoke Provider callback; Safe exceptions may escape, other exceptions become Failure.
				 * @param clone Provider context clone callback; it must create independent provider-owned state.
				 * @param release Provider context destructor; it must not throw.
				 * @throws StormByte::Exception If an argument is null; ownership is not transferred.
				 */
				Function(void* context, Invoke invoke, Clone clone, Release release):
					m_owner(nullptr, nullptr, nullptr), m_invoke(invoke) {
					if (!context || !invoke || !clone || !release)
						throw StormByte::Exception("Safe function requires context, invoke, clone and release");
					m_owner = Detail::Owner(context, clone, release);
				}

				/**
				 * @brief Deep-copy provider context.
				 * @param other Source callback.
				 * @throws StormByte::Exception Context cloning failed.
				 */
				Function(const Function& other) = default;
				/**
				 * @brief Deep-copy provider context with the strong guarantee.
				 * @param other Source callback.
				 * @return This callback.
				 * @throws StormByte::Exception Context cloning failed.
				 */
				Function& operator=(const Function& other) = default;
				/**
				 * @brief Transfer callback context.
				 * @param other Source callback.
				 */
				Function(Function&& other) noexcept:
					m_owner(std::move(other.m_owner)), m_invoke(std::exchange(other.m_invoke, nullptr)) {}
				/**
				 * @brief Release current context and transfer.
				 * @param other Source callback.
				 * @return This callback.
				 */
				Function& operator=(Function&& other) noexcept {
					if (this != &other) {
						m_owner = std::move(other.m_owner);
						m_invoke = std::exchange(other.m_invoke, nullptr);
					}
					return *this;
				}
				/**
				 * @brief Release context through its provider callback.
				 */
				~Function() noexcept = default;

				/**
				 * @brief Whether this callback still owns a context.
				 * @return False after move.
				 */
				bool HasValue() const noexcept {
					return m_owner.Get() != nullptr;
				}

				/**
				 * @brief Invoke and write a successful result to caller-owned storage.
				 * @param output Result destination; unchanged unless the provider returns Success.
				 * @param args Typed callback arguments.
				 * @return Provider status, Failure for non-Safe exceptions, or Missing after move.
				 * @throws StormByte::Exception Exceptions derived from Base's exception type.
				 */
				Status Call(Return& output, Args... args) const {
					if (!HasValue())
						return Status::Missing;
					try {
						Return result(output);
						const Status status = m_invoke(m_owner.Get(), &result, std::forward<Args>(args)...);
						if (status == Status::Success)
							output = std::move(result);
						return status;
					} catch (const StormByte::Exception&) {
						throw;
					} catch (...) {
						return Status::Failure;
					}
				}

			private:
				Detail::Owner m_owner; ///< Context released in its creator module.
				Invoke m_invoke; ///< Typed invocation function in the creator module.
		};
	}
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
		 * @brief Classifies provider-implemented typed callbacks as conditional.
		 * @tparam Args Callback argument types.
		 */
		template<typename... Args>
		requires (Safe::Detail::SafeCallbackArgument<Args> && ...) &&
			((SafeComponent<std::remove_cvref_t<Args>>) && ...)
		struct IsMaybeSafe<Safe::Function<void(Args...)>>: std::true_type {};

		/**
		 * @brief Classifies provider-implemented value-returning callbacks as conditional.
		 * @tparam Return Callback result type.
		 * @tparam Args Callback argument types.
		 */
		template<typename Return, typename... Args>
		requires (!std::is_void_v<Return>) && Safe::Detail::SafeCallbackArgument<Return> &&
			(Safe::Detail::SafeCallbackArgument<Args> && ...) &&
			SafeComponent<std::remove_cvref_t<Return>> && (SafeComponent<std::remove_cvref_t<Args>> && ...) &&
			Type::CopyConstructible<Return> &&
			requires(Return& output, Return&& value) { { output = std::move(value) } noexcept; }
		struct IsMaybeSafe<Safe::Function<Return(Args...)>>: std::true_type {};
	}
}
