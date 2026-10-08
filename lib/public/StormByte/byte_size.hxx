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

#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ostream>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

/**
 * @file StormByte/byte_size.hxx
 * @brief IEC octet length (`uint64_t`), implicit only to `std::size_t`.
 */

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class ByteSize
	 * @brief Count of octets (`uint64_t` storage).
	 *
	 * This is the byte-length type. @ref Size is an abstract unit count.
	 * The value that crosses a DLL is always 64-bit unsigned.
	 *
	 * Implicit conversion exists only to `std::size_t` (clamp to
	 * `size_t::max`). Every other integral destination is `explicit`
	 * and clamps to `T::max`. There is no `operator bool` and no `Value()`.
	 *
	 * On hosts where `std::size_t` is `unsigned long` or
	 * `unsigned long long`, that conversion is the implicit
	 * `operator std::size_t`. Those two names are not given a second
	 * `extern template` of `operator T()`.
	 *
	 * Area products (`ByteSize * ByteSize`) are deleted. Scaling by
	 * @ref Size or by an integer is allowed and yields @ref ByteSize.
	 *
	 * Mixed comparison with any @ref Type::Integral is a hidden friend
	 * so `n == 5`, `5u == n` and `ASSERT_EQUAL` are not ambiguous
	 * with the built-in candidates. Comparison with @ref Size is also
	 * a hidden friend and compares the numeric counts.
	 *
	 * `1 * KiB` uses the free @ref StormByte::KiB constant (a @ref ByteSize
	 * of 1024 octets). `ByteSize::KiB(n)` is the same scale via @ref Unit.
	 *
	 * Overflow of `uint64_t` and a negative result: `assert` when
	 * assertions are on; wrap modulo 2⁶⁴ when they are not.
	 * Division by zero: `assert` when assertions are on; undefined otherwise.
	 *
	 * `operator Safe::String` / `operator Safe::WString` print an IEC human-readable
	 * length (`1.50 KiB`). `B`/`KiB`/`MiB`/`GiB`/`TiB`/`PiB`/`EiB` are IEC.
	 * `KB`/`MB`/`GB`/`TB`/`PB`/`EB` are SI (10³).
	 */
	class STORMBYTE_PUBLIC ByteSize final {
		public:
			/**
			 * @brief One-octet scale used by the named factories.
			 */
			struct Unit {
				std::uint64_t bytes;	///< Octets in one unit

				/**
				 * @brief Scale @p count units into a @ref ByteSize.
				 * @param count Number of units.
				 * @return Product as @ref ByteSize.
				 */
				constexpr ByteSize operator()(Size count) const noexcept {
					return ByteSize(Raw{}, Mul(static_cast<std::uint64_t>(static_cast<std::size_t>(count)), bytes));
				}
			};

			static constexpr Unit B{1ull};										///< 2^0 octets
			static constexpr Unit KiB{1024ull};									///< 2^10 octets
			static constexpr Unit MiB{1024ull * 1024};							///< 2^20 octets
			static constexpr Unit GiB{1024ull * 1024 * 1024};					///< 2^30 octets
			static constexpr Unit TiB{1024ull * 1024 * 1024 * 1024};			///< 2^40 octets
			static constexpr Unit PiB{1024ull * 1024 * 1024 * 1024 * 1024};		///< 2^50 octets
			static constexpr Unit EiB{1024ull * 1024 * 1024 * 1024 * 1024 * 1024};	///< 2^60 octets
			static constexpr Unit KB{1000ull};									///< 10^3 octets
			static constexpr Unit MB{1000ull * 1000};							///< 10^6 octets
			static constexpr Unit GB{1000ull * 1000 * 1000};					///< 10^9 octets
			static constexpr Unit TB{1000ull * 1000 * 1000 * 1000};				///< 10^12 octets
			static constexpr Unit PB{1000ull * 1000 * 1000 * 1000 * 1000};		///< 10^15 octets
			static constexpr Unit EB{1000ull * 1000 * 1000 * 1000 * 1000 * 1000};	///< 10^18 octets

			/**
			 * @brief Zero octets.
			 */
			constexpr ByteSize() noexcept: m_value(0) {}

			/**
			 * @brief From an integer octet count.
			 * @tparam T Integral type.
			 * @param value Octets. Negative is undefined (`assert` when on).
			 */
			template<Type::Integral T>
			constexpr ByteSize(T value) noexcept: m_value(FromIntegral(value)) {}

			/**
			 * @brief From an abstract unit count interpreted as octets.
			 * @param count Unit count used as an octet count.
			 */
			constexpr explicit ByteSize(Size count) noexcept: m_value(static_cast<std::uint64_t>(static_cast<std::size_t>(count))) {}

			/**
			 * @brief Copy constructor.
			 * @param other Length to copy.
			 */
			constexpr ByteSize(const ByteSize& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Length to take.
			 */
			constexpr ByteSize(ByteSize&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~ByteSize() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Length to copy.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator=(const ByteSize& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Length to take.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator=(ByteSize&& other) noexcept = default;

			/**
			 * @brief Assign an integer octet count.
			 * @tparam T Integral type.
			 * @param value Octets. Negative is undefined (`assert` when on).
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr ByteSize& operator=(T value) noexcept {
				m_value = FromIntegral(value);
				return *this;
			}

			/**
			 * @brief Implicit host `size_t` (clamped).
			 * @return Octets as `std::size_t`.
			 */
			constexpr operator std::size_t() const noexcept {
				constexpr auto max = static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max());
				return static_cast<std::size_t>(m_value > max ? max : m_value);
			}

			/**
			 * @brief Explicit conversion to an integral other than `std::size_t`.
			 * @tparam T Integral type, not `std::size_t`.
			 * @return Octets clamped to `std::numeric_limits<T>::max()`.
			 */
			template<Type::Integral T>
			requires (!Type::SameAs<T, std::size_t>)
			constexpr explicit operator T() const noexcept {
				constexpr auto max = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
				return static_cast<T>(m_value > max ? max : m_value);
			}

			/**
			 * @brief IEC human-readable text owned by Base.
			 * @return Formatted length.
			 */
			explicit operator Safe::String() const noexcept;

			/**
			 * @brief IEC human-readable wide text owned by Base.
			 * @return Formatted length.
			 */
			explicit operator Safe::WString() const noexcept;

			/**
			 * @brief IEC human-readable text on the caller heap.
			 * @return Formatted length.
			 */
			STORMBYTE_FORCE_INLINE operator std::string() const {
				// By name. static_cast<Safe::String> is Safe::String(std::string) on GCC,
				// and that calls this operator again.
				return operator Safe::String().operator std::string();
			}

			/**
			 * @brief Numeric equality.
			 * @param lhs Left length.
			 * @param rhs Right length.
			 * @return Whether the lengths are equal.
			 */
			friend constexpr bool operator==(ByteSize lhs, ByteSize rhs) noexcept {
				return lhs.m_value == rhs.m_value;
			}

			/**
			 * @brief Mixed equality with an integer.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr bool operator==(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Mixed equality with an integer.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr bool operator==(T lhs, ByteSize rhs) noexcept;

			/**
			 * @brief Mixed equality with @ref Size.
			 * @param lhs Left length.
			 * @param rhs Right unit count.
			 * @return Whether the numeric values match.
			 */
			friend constexpr bool operator==(ByteSize lhs, Size rhs) noexcept {
				return lhs.m_value == static_cast<std::uint64_t>(static_cast<std::size_t>(rhs));
			}

			/**
			 * @brief Mixed equality with @ref Size.
			 * @param lhs Left unit count.
			 * @param rhs Right length.
			 * @return Whether the numeric values match.
			 */
			friend constexpr bool operator==(Size lhs, ByteSize rhs) noexcept {
				return rhs == lhs;
			}

			/**
			 * @brief Numeric order.
			 * @param lhs Left length.
			 * @param rhs Right length.
			 * @return Ordering.
			 */
			friend constexpr std::strong_ordering operator<=>(ByteSize lhs, ByteSize rhs) noexcept {
				return lhs.m_value <=> rhs.m_value;
			}

			/**
			 * @brief Mixed order with an integer.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr std::strong_ordering operator<=>(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Mixed order with an integer.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr std::strong_ordering operator<=>(T lhs, ByteSize rhs) noexcept;

			/**
			 * @brief Mixed order with @ref Size.
			 * @param lhs Left length.
			 * @param rhs Right unit count.
			 * @return Ordering.
			 */
			friend constexpr std::strong_ordering operator<=>(ByteSize lhs, Size rhs) noexcept {
				return lhs.m_value <=> static_cast<std::uint64_t>(static_cast<std::size_t>(rhs));
			}

			/**
			 * @brief Mixed order with @ref Size.
			 * @param lhs Left unit count.
			 * @param rhs Right length.
			 * @return Ordering.
			 */
			friend constexpr std::strong_ordering operator<=>(Size lhs, ByteSize rhs) noexcept {
				return 0 <=> (rhs <=> lhs);
			}

			/**
			 * @brief Prefix increment.
			 * @return @c *this after adding one octet.
			 */
			constexpr ByteSize& operator++() noexcept {
				*this += ByteSize(Raw{}, 1);
				return *this;
			}

			/**
			 * @brief Postfix increment.
			 * @return Copy before adding one octet.
			 */
			constexpr ByteSize operator++(int) noexcept {
				const ByteSize old = *this;
				++*this;
				return old;
			}

			/**
			 * @brief Prefix decrement.
			 * @return @c *this after subtracting one octet.
			 */
			constexpr ByteSize& operator--() noexcept {
				*this -= ByteSize(Raw{}, 1);
				return *this;
			}

			/**
			 * @brief Postfix decrement.
			 * @return Copy before subtracting one octet.
			 */
			constexpr ByteSize operator--(int) noexcept {
				const ByteSize old = *this;
				--*this;
				return old;
			}

			/**
			 * @brief Sum of lengths.
			 * @param lhs Left addend.
			 * @param rhs Right addend.
			 * @return Sum as @ref ByteSize.
			 */
			friend constexpr ByteSize operator+(ByteSize lhs, ByteSize rhs) noexcept {
				return ByteSize(Raw{}, Add(lhs.m_value, rhs.m_value));
			}

			/**
			 * @brief Difference of lengths.
			 * @param lhs Minuend.
			 * @param rhs Subtrahend.
			 * @return Difference as @ref ByteSize.
			 */
			friend constexpr ByteSize operator-(ByteSize lhs, ByteSize rhs) noexcept {
				return ByteSize(Raw{}, Sub(lhs.m_value, rhs.m_value));
			}

			ByteSize operator*(ByteSize) const = delete;
			ByteSize& operator*=(ByteSize) = delete;
			ByteSize operator/(ByteSize) const = delete;
			ByteSize& operator/=(ByteSize) = delete;
			ByteSize operator%(ByteSize) const = delete;
			ByteSize& operator%=(ByteSize) = delete;

			/**
			 * @brief Scale by a unit count.
			 * @param lhs Length.
			 * @param rhs Factor.
			 * @return Product as @ref ByteSize.
			 */
			friend constexpr ByteSize operator*(ByteSize lhs, Size rhs) noexcept {
				return ByteSize(Raw{}, Mul(lhs.m_value, static_cast<std::uint64_t>(static_cast<std::size_t>(rhs))));
			}

			/**
			 * @brief Scale by a unit count.
			 * @param lhs Factor.
			 * @param rhs Length.
			 * @return Product as @ref ByteSize.
			 */
			friend constexpr ByteSize operator*(Size lhs, ByteSize rhs) noexcept {
				return rhs * lhs;
			}

			/**
			 * @brief Divide by a unit count.
			 * @param lhs Length.
			 * @param rhs Divisor.
			 * @return Quotient as @ref ByteSize.
			 */
			friend constexpr ByteSize operator/(ByteSize lhs, Size rhs) noexcept {
				return ByteSize(Raw{}, Div(lhs.m_value, static_cast<std::uint64_t>(static_cast<std::size_t>(rhs))));
			}

			/**
			 * @brief Remainder with a unit count.
			 * @param lhs Length.
			 * @param rhs Divisor.
			 * @return Remainder as @ref ByteSize.
			 */
			friend constexpr ByteSize operator%(ByteSize lhs, Size rhs) noexcept {
				return ByteSize(Raw{}, Mod(lhs.m_value, static_cast<std::uint64_t>(static_cast<std::size_t>(rhs))));
			}

			/**
			 * @brief Add @p other octets.
			 * @param other Addend.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator+=(ByteSize other) noexcept {
				m_value = Add(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Subtract @p other octets.
			 * @param other Subtrahend.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator-=(ByteSize other) noexcept {
				m_value = Sub(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Scale by a unit count.
			 * @param other Factor.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator*=(Size other) noexcept {
				*this = *this * other;
				return *this;
			}

			/**
			 * @brief Divide by a unit count.
			 * @param other Divisor.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator/=(Size other) noexcept {
				*this = *this / other;
				return *this;
			}

			/**
			 * @brief Remainder with a unit count.
			 * @param other Divisor.
			 * @return @c *this.
			 */
			constexpr ByteSize& operator%=(Size other) noexcept {
				*this = *this % other;
				return *this;
			}

			/**
			 * @brief Mixed sum.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator+(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Mixed sum.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator+(T lhs, ByteSize rhs) noexcept;

			/**
			 * @brief Mixed difference.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator-(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Mixed difference.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator-(T lhs, ByteSize rhs) noexcept;

			/**
			 * @brief Mixed product.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator*(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Mixed product.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator*(T lhs, ByteSize rhs) noexcept;

			/**
			 * @brief Mixed quotient.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator/(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Mixed remainder.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr ByteSize operator%(ByteSize lhs, T rhs) noexcept;

			/**
			 * @brief Add an integer octet count.
			 * @tparam T Integral type.
			 * @param value Addend.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr ByteSize& operator+=(T value) noexcept {
				*this = *this + value;
				return *this;
			}

			/**
			 * @brief Subtract an integer octet count.
			 * @tparam T Integral type.
			 * @param value Subtrahend.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr ByteSize& operator-=(T value) noexcept {
				*this = *this - value;
				return *this;
			}

			/**
			 * @brief Scale by an integer.
			 * @tparam T Integral type.
			 * @param value Factor.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr ByteSize& operator*=(T value) noexcept {
				*this = *this * value;
				return *this;
			}

			/**
			 * @brief Divide by an integer.
			 * @tparam T Integral type.
			 * @param value Divisor.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr ByteSize& operator/=(T value) noexcept {
				*this = *this / value;
				return *this;
			}

			/**
			 * @brief Remainder with an integer.
			 * @tparam T Integral type.
			 * @param value Divisor.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr ByteSize& operator%=(T value) noexcept {
				*this = *this % value;
				return *this;
			}

		private:
			std::uint64_t m_value;	///< Octet count

			/**
			 * @brief Tag for a pre-normalized store.
			 */
			struct Raw {
				explicit constexpr Raw() noexcept = default;
			};

			/**
			 * @brief Private raw store.
			 * @param value Already-normalized count.
			 */
			explicit constexpr ByteSize(Raw, std::uint64_t value) noexcept: m_value(value) {}

			/**
			 * @brief Convert a construction operand.
			 * @tparam T Integral type.
			 * @param value Operand.
			 * @return Non-negative `uint64_t`.
			 */
			template<Type::Integral T>
			static constexpr std::uint64_t FromIntegral(T value) noexcept {
				if constexpr (Type::Signed<T>)
					assert(value >= T{0});
				return static_cast<std::uint64_t>(value);
			}

			/**
			 * @brief Saturated-debug add.
			 * @param lhs Left.
			 * @param rhs Right.
			 * @return Sum or wrap.
			 */
			static constexpr std::uint64_t Add(std::uint64_t lhs, std::uint64_t rhs) noexcept {
				assert(lhs <= std::numeric_limits<std::uint64_t>::max() - rhs);
				return lhs + rhs;
			}

			/**
			 * @brief Saturated-debug sub.
			 * @param lhs Left.
			 * @param rhs Right.
			 * @return Difference or wrap.
			 */
			static constexpr std::uint64_t Sub(std::uint64_t lhs, std::uint64_t rhs) noexcept {
				assert(lhs >= rhs);
				return lhs - rhs;
			}

			/**
			 * @brief Saturated-debug mul.
			 * @param lhs Left.
			 * @param rhs Right.
			 * @return Product or wrap.
			 */
			static constexpr std::uint64_t Mul(std::uint64_t lhs, std::uint64_t rhs) noexcept {
				assert(rhs == 0 || lhs <= std::numeric_limits<std::uint64_t>::max() / rhs);
				return lhs * rhs;
			}

			/**
			 * @brief Quotient.
			 * @param lhs Dividend.
			 * @param rhs Divisor.
			 * @return Quotient.
			 */
			static constexpr std::uint64_t Div(std::uint64_t lhs, std::uint64_t rhs) noexcept {
				assert(rhs != 0);
				return lhs / rhs;
			}

			/**
			 * @brief Remainder.
			 * @param lhs Dividend.
			 * @param rhs Divisor.
			 * @return Remainder.
			 */
			static constexpr std::uint64_t Mod(std::uint64_t lhs, std::uint64_t rhs) noexcept {
				assert(rhs != 0);
				return lhs % rhs;
			}

			/**
			 * @brief Add a possibly signed operand.
			 * @tparam T Integral type.
			 * @param lhs Left count.
			 * @param rhs Operand.
			 * @return @ref ByteSize.
			 */
			template<Type::Integral T>
			static constexpr ByteSize MixAdd(std::uint64_t lhs, T rhs) noexcept {
				if constexpr (Type::Signed<T>) {
					if (rhs < T{0}) {
						const auto mag = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(rhs);
						return ByteSize(Raw{}, Sub(lhs, mag));
					}
				}
				return ByteSize(Raw{}, Add(lhs, static_cast<std::uint64_t>(rhs)));
			}

			/**
			 * @brief Subtract a possibly signed operand.
			 * @tparam T Integral type.
			 * @param lhs Left count.
			 * @param rhs Operand.
			 * @return @ref ByteSize.
			 */
			template<Type::Integral T>
			static constexpr ByteSize MixSub(std::uint64_t lhs, T rhs) noexcept {
				if constexpr (Type::Signed<T>) {
					if (rhs < T{0}) {
						const auto mag = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(rhs);
						return ByteSize(Raw{}, Add(lhs, mag));
					}
				}
				return ByteSize(Raw{}, Sub(lhs, static_cast<std::uint64_t>(rhs)));
			}

			/**
			 * @brief Multiply by a possibly signed operand.
			 * @tparam T Integral type.
			 * @param lhs Left count.
			 * @param rhs Operand.
			 * @return @ref ByteSize.
			 */
			template<Type::Integral T>
			static constexpr ByteSize MixMul(std::uint64_t lhs, T rhs) noexcept {
				if constexpr (Type::Signed<T>)
					assert(rhs >= T{0});
				return ByteSize(Raw{}, Mul(lhs, static_cast<std::uint64_t>(rhs)));
			}
	};

	/**
	 * @brief Mixed equality.
	 * @tparam T Integral type.
	 * @param lhs Left length.
	 * @param rhs Right integer.
	 * @return Whether the numeric values match.
	 */
	template<Type::Integral T>
	constexpr bool operator==(ByteSize lhs, T rhs) noexcept {
		return lhs == ByteSize(rhs);
	}

	/**
	 * @brief Mixed equality.
	 * @tparam T Integral type.
	 * @param lhs Left integer.
	 * @param rhs Right length.
	 * @return Whether the numeric values match.
	 */
	template<Type::Integral T>
	constexpr bool operator==(T lhs, ByteSize rhs) noexcept {
		return ByteSize(lhs) == rhs;
	}

	/**
	 * @brief Mixed order.
	 * @tparam T Integral type.
	 * @param lhs Left length.
	 * @param rhs Right integer.
	 * @return Ordering.
	 */
	template<Type::Integral T>
	constexpr std::strong_ordering operator<=>(ByteSize lhs, T rhs) noexcept {
		return lhs <=> ByteSize(rhs);
	}

	/**
	 * @brief Mixed order.
	 * @tparam T Integral type.
	 * @param lhs Left integer.
	 * @param rhs Right length.
	 * @return Ordering.
	 */
	template<Type::Integral T>
	constexpr std::strong_ordering operator<=>(T lhs, ByteSize rhs) noexcept {
		return ByteSize(lhs) <=> rhs;
	}

	/**
	 * @brief Mixed sum.
	 * @tparam T Integral type.
	 * @param lhs Left addend.
	 * @param rhs Right addend.
	 * @return Sum as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator+(ByteSize lhs, T rhs) noexcept {
		return ByteSize::MixAdd(lhs.m_value, rhs);
	}

	/**
	 * @brief Mixed sum.
	 * @tparam T Integral type.
	 * @param lhs Left addend.
	 * @param rhs Right addend.
	 * @return Sum as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator+(T lhs, ByteSize rhs) noexcept {
		return ByteSize::MixAdd(rhs.m_value, lhs);
	}

	/**
	 * @brief Mixed difference.
	 * @tparam T Integral type.
	 * @param lhs Minuend.
	 * @param rhs Subtrahend.
	 * @return Difference as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator-(ByteSize lhs, T rhs) noexcept {
		return ByteSize::MixSub(lhs.m_value, rhs);
	}

	/**
	 * @brief Mixed difference.
	 * @tparam T Integral type.
	 * @param lhs Minuend.
	 * @param rhs Subtrahend.
	 * @return Difference as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator-(T lhs, ByteSize rhs) noexcept {
		return ByteSize(lhs) - rhs;
	}

	/**
	 * @brief Mixed product.
	 * @tparam T Integral type.
	 * @param lhs Left factor.
	 * @param rhs Right factor.
	 * @return Product as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator*(ByteSize lhs, T rhs) noexcept {
		return ByteSize::MixMul(lhs.m_value, rhs);
	}

	/**
	 * @brief Mixed product.
	 * @tparam T Integral type.
	 * @param lhs Left factor.
	 * @param rhs Right factor.
	 * @return Product as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator*(T lhs, ByteSize rhs) noexcept {
		return ByteSize::MixMul(rhs.m_value, lhs);
	}

	/**
	 * @brief Mixed quotient.
	 * @tparam T Integral type.
	 * @param lhs Dividend.
	 * @param rhs Divisor.
	 * @return Quotient as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator/(ByteSize lhs, T rhs) noexcept {
		return ByteSize(ByteSize::Raw{}, ByteSize::Div(lhs.m_value, ByteSize::FromIntegral(rhs)));
	}

	/**
	 * @brief Mixed remainder.
	 * @tparam T Integral type.
	 * @param lhs Dividend.
	 * @param rhs Divisor.
	 * @return Remainder as @ref ByteSize.
	 */
	template<Type::Integral T>
	constexpr ByteSize operator%(ByteSize lhs, T rhs) noexcept {
		return ByteSize(ByteSize::Raw{}, ByteSize::Mod(lhs.m_value, ByteSize::FromIntegral(rhs)));
	}

	/**
	 * @brief Writes the IEC human-readable length.
	 * @param stream Destination.
	 * @param size Length.
	 * @return @p stream.
	 */
	inline std::ostream& operator<<(std::ostream& stream, ByteSize size) {
		stream << static_cast<std::string>(size);
		return stream;
	}

	/// @cond
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator+=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator-=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator*=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator/=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize& ByteSize::operator%=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize::operator char() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator signed char() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator unsigned char() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator short() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator unsigned short() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator int() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator unsigned int() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator long() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator long long() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator wchar_t() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator char8_t() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator char16_t() const noexcept;
	extern template STORMBYTE_PUBLIC ByteSize::operator char32_t() const noexcept;

	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(ByteSize, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC bool operator==(char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(signed char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(wchar_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(char8_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(char16_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(char32_t, ByteSize) noexcept;

	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(ByteSize, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(signed char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(wchar_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char8_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char16_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char32_t, ByteSize) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(ByteSize, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator+(char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(signed char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(unsigned char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(unsigned short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(unsigned int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(unsigned long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(unsigned long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(wchar_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(char8_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(char16_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator+(char32_t, ByteSize) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(ByteSize, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator-(char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(signed char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(unsigned char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(unsigned short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(unsigned int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(unsigned long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(unsigned long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(wchar_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(char8_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(char16_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator-(char32_t, ByteSize) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(ByteSize, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator*(char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(signed char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(unsigned char, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(unsigned short, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(unsigned int, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(unsigned long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(unsigned long long, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(wchar_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(char8_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(char16_t, ByteSize) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator*(char32_t, ByteSize) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator/(ByteSize, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, signed char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC ByteSize operator%(ByteSize, char32_t) noexcept;
	/// @endcond

	/**
	 * @brief One octet.
	 */
	inline constexpr ByteSize B{1ull};

	/**
	 * @brief 2^10 octets. `1 * KiB` is 1024.
	 */
	inline constexpr ByteSize KiB{1024ull};

	/**
	 * @brief 2^20 octets.
	 */
	inline constexpr ByteSize MiB{1024ull * 1024};

	/**
	 * @brief 2^30 octets.
	 */
	inline constexpr ByteSize GiB{1024ull * 1024 * 1024};

	/**
	 * @brief 2^40 octets.
	 */
	inline constexpr ByteSize TiB{1024ull * 1024 * 1024 * 1024};

	/**
	 * @brief 2^50 octets.
	 */
	inline constexpr ByteSize PiB{1024ull * 1024 * 1024 * 1024 * 1024};

	/**
	 * @brief 2^60 octets.
	 */
	inline constexpr ByteSize EiB{1024ull * 1024 * 1024 * 1024 * 1024 * 1024};

	/**
	 * @brief 10^3 octets.
	 */
	inline constexpr ByteSize KB{1000ull};

	/**
	 * @brief 10^6 octets.
	 */
	inline constexpr ByteSize MB{1000ull * 1000};

	/**
	 * @brief 10^9 octets.
	 */
	inline constexpr ByteSize GB{1000ull * 1000 * 1000};

	/**
	 * @brief 10^12 octets.
	 */
	inline constexpr ByteSize TB{1000ull * 1000 * 1000 * 1000};

	/**
	 * @brief 10^15 octets.
	 */
	inline constexpr ByteSize PB{1000ull * 1000 * 1000 * 1000 * 1000};

	/**
	 * @brief 10^18 octets.
	 */
	inline constexpr ByteSize EB{1000ull * 1000 * 1000 * 1000 * 1000 * 1000};

}

template<>
struct StormByte::Safe::Hash<StormByte::ByteSize> {
	/**
	 * @brief Hashes @p value.
	 * @param value Octet count.
	 * @return FNV-1a of the 64-bit count. The same value in every module.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::ByteSize& value) const noexcept {
		const auto widened = static_cast<std::uint64_t>(static_cast<std::size_t>(value));
		return Safe::HashBytes(std::as_bytes(std::span<const std::uint64_t>(&widened, 1)));
	}
};

template<>
struct std::hash<StormByte::ByteSize> {
	/**
	 * @brief Hashes @p value.
	 * @param value Octet count.
	 * @return Hash of this module's `std::hash<std::uint64_t>`.
	 */
	std::size_t operator()(const StormByte::ByteSize& value) const noexcept {
		return StormByte::Safe::Hash<StormByte::ByteSize>{}(value);
	}
};
