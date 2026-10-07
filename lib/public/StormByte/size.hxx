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

#include <StormByte/safe/hash.hxx>
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
 * @file StormByte/size.hxx
 * @brief Host-width-stable count (`uint64_t`), implicit only to `std::size_t`.
 */

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/** @namespace StormByte::Safe
	 * @brief Types safe to pass across a DLL boundary.
	 */
	namespace Safe {
		class String;
		class WString;
	}
	class ByteSize;

	/**
	 * @class Size
	 * @brief Count of abstract units (`uint64_t` storage).
	 *
	 * Not a byte length. Not `std::size_t`. Not `std::ptrdiff_t`.
	 * The value that crosses a DLL is always 64-bit unsigned.
	 *
	 * Arithmetic always yields @ref Size. Integer operands are absorbed.
	 * `std::size_t calc = size1 + 3 * size2` does the math in `uint64_t`
	 * and converts once at the end.
	 *
	 * Overflow of `uint64_t` and a negative result: `assert` when
	 * assertions are on; wrap modulo 2⁶⁴ when they are not.
	 * Division by zero: `assert` when assertions are on; undefined otherwise.
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
	 * Mixed comparison with any @ref Type::Integral is a hidden friend
	 * so `s == 5`, `5u == s` and `ASSERT_EQUAL` are not ambiguous
	 * with the built-in candidates. Comparison with
	 * @ref StormByte::ByteSize lives in `byte_size.hxx`.
	 *
	 * `operator Safe::String` / `operator Safe::WString` print decimal digits.
	 */
	class STORMBYTE_PUBLIC Size final {
		public:
			/**
			 * @brief Zero.
			 */
			constexpr Size() noexcept: m_value(0) {}

			/**
			 * @brief From an integer count.
			 * @tparam T Integral type.
			 * @param value Count. Negative is undefined (`assert` when on).
			 */
			template<Type::Integral T>
			constexpr Size(T value) noexcept: m_value(FromIntegral(value)) {}

			/**
			 * @brief Copy constructor.
			 * @param other Count to copy.
			 */
			constexpr Size(const Size& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Count to take.
			 */
			constexpr Size(Size&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~Size() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Count to copy.
			 * @return @c *this.
			 */
			constexpr Size& operator=(const Size& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Count to take.
			 * @return @c *this.
			 */
			constexpr Size& operator=(Size&& other) noexcept = default;

			/**
			 * @brief Assign an integer count.
			 * @tparam T Integral type.
			 * @param value Count. Negative is undefined (`assert` when on).
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr Size& operator=(T value) noexcept {
				m_value = FromIntegral(value);
				return *this;
			}

			/**
			 * @brief Implicit host `size_t` (clamped).
			 * @return Count as `std::size_t`.
			 */
			constexpr operator std::size_t() const noexcept {
				constexpr auto max = static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max());
				return static_cast<std::size_t>(m_value > max ? max : m_value);
			}

			/**
			 * @brief Explicit conversion to an integral other than `std::size_t`.
			 * @tparam T Integral type, not `std::size_t`.
			 * @return Count clamped to `std::numeric_limits<T>::max()`.
			 */
			template<Type::Integral T>
			requires (!Type::SameAs<T, std::size_t>)
			constexpr explicit operator T() const noexcept {
				constexpr auto max = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
				return static_cast<T>(m_value > max ? max : m_value);
			}

			/**
			 * @brief Decimal text owned by Base.
			 * @return Digits, no unit suffix.
			 */
			explicit operator Safe::String() const noexcept;

			/**
			 * @brief Decimal wide text owned by Base.
			 * @return Digits, no unit suffix.
			 */
			explicit operator Safe::WString() const noexcept;

			/**
			 * @brief Decimal text on the caller heap.
			 * @return Digits, no unit suffix.
			 */
			STORMBYTE_FORCE_INLINE operator std::string() const {
				return std::to_string(m_value);
			}

			/**
			 * @brief Numeric equality.
			 * @param lhs Left count.
			 * @param rhs Right count.
			 * @return Whether the counts are equal.
			 */
			friend constexpr bool operator==(Size lhs, Size rhs) noexcept {
				return lhs.m_value == rhs.m_value;
			}

			/**
			 * @brief Mixed equality.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr bool operator==(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed equality.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr bool operator==(T lhs, Size rhs) noexcept;

			/**
			 * @brief Numeric order.
			 * @param lhs Left count.
			 * @param rhs Right count.
			 * @return Ordering.
			 */
			friend constexpr std::strong_ordering operator<=>(Size lhs, Size rhs) noexcept {
				return lhs.m_value <=> rhs.m_value;
			}

			/**
			 * @brief Mixed order.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr std::strong_ordering operator<=>(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed order.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr std::strong_ordering operator<=>(T lhs, Size rhs) noexcept;

			/**
			 * @brief Prefix increment.
			 * @return @c *this after adding one.
			 */
			constexpr Size& operator++() noexcept {
				*this += Size(Raw{}, 1);
				return *this;
			}

			/**
			 * @brief Postfix increment.
			 * @return Copy before adding one.
			 */
			constexpr Size operator++(int) noexcept {
				const Size old = *this;
				++*this;
				return old;
			}

			/**
			 * @brief Prefix decrement.
			 * @return @c *this after subtracting one.
			 */
			constexpr Size& operator--() noexcept {
				*this -= Size(Raw{}, 1);
				return *this;
			}

			/**
			 * @brief Postfix decrement.
			 * @return Copy before subtracting one.
			 */
			constexpr Size operator--(int) noexcept {
				const Size old = *this;
				--*this;
				return old;
			}

			/**
			 * @brief Sum.
			 * @param lhs Left addend.
			 * @param rhs Right addend.
			 * @return Sum as @ref Size.
			 */
			friend constexpr Size operator+(Size lhs, Size rhs) noexcept {
				return Size(Raw{}, Add(lhs.m_value, rhs.m_value));
			}

			/**
			 * @brief Difference.
			 * @param lhs Minuend.
			 * @param rhs Subtrahend.
			 * @return Difference as @ref Size.
			 */
			friend constexpr Size operator-(Size lhs, Size rhs) noexcept {
				return Size(Raw{}, Sub(lhs.m_value, rhs.m_value));
			}

			/**
			 * @brief Product.
			 * @param lhs Left factor.
			 * @param rhs Right factor.
			 * @return Product as @ref Size.
			 */
			friend constexpr Size operator*(Size lhs, Size rhs) noexcept {
				return Size(Raw{}, Mul(lhs.m_value, rhs.m_value));
			}

			/**
			 * @brief Quotient.
			 * @param lhs Dividend.
			 * @param rhs Divisor.
			 * @return Quotient as @ref Size.
			 */
			friend constexpr Size operator/(Size lhs, Size rhs) noexcept {
				return Size(Raw{}, Div(lhs.m_value, rhs.m_value));
			}

			/**
			 * @brief Remainder.
			 * @param lhs Dividend.
			 * @param rhs Divisor.
			 * @return Remainder as @ref Size.
			 */
			friend constexpr Size operator%(Size lhs, Size rhs) noexcept {
				return Size(Raw{}, Mod(lhs.m_value, rhs.m_value));
			}

			/**
			 * @brief Add @p other.
			 * @param other Addend.
			 * @return @c *this.
			 */
			constexpr Size& operator+=(Size other) noexcept {
				m_value = Add(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Subtract @p other.
			 * @param other Subtrahend.
			 * @return @c *this.
			 */
			constexpr Size& operator-=(Size other) noexcept {
				m_value = Sub(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Multiply by @p other.
			 * @param other Factor.
			 * @return @c *this.
			 */
			constexpr Size& operator*=(Size other) noexcept {
				m_value = Mul(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Divide by @p other.
			 * @param other Divisor.
			 * @return @c *this.
			 */
			constexpr Size& operator/=(Size other) noexcept {
				m_value = Div(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Remainder with @p other.
			 * @param other Divisor.
			 * @return @c *this.
			 */
			constexpr Size& operator%=(Size other) noexcept {
				m_value = Mod(m_value, other.m_value);
				return *this;
			}

			/**
			 * @brief Mixed sum.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator+(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed sum.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator+(T lhs, Size rhs) noexcept;

			/**
			 * @brief Mixed difference.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator-(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed difference.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator-(T lhs, Size rhs) noexcept;

			/**
			 * @brief Mixed product.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator*(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed product.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator*(T lhs, Size rhs) noexcept;

			/**
			 * @brief Mixed quotient.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator/(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed quotient.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator/(T lhs, Size rhs) noexcept;

			/**
			 * @brief Mixed remainder.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator%(Size lhs, T rhs) noexcept;

			/**
			 * @brief Mixed remainder.
			 * @tparam T Integral type.
			 */
			template<Type::Integral T>
			friend constexpr Size operator%(T lhs, Size rhs) noexcept;

			/**
			 * @brief Add an integer.
			 * @tparam T Integral type.
			 * @param value Addend.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr Size& operator+=(T value) noexcept {
				*this = *this + value;
				return *this;
			}

			/**
			 * @brief Subtract an integer.
			 * @tparam T Integral type.
			 * @param value Subtrahend.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr Size& operator-=(T value) noexcept {
				*this = *this - value;
				return *this;
			}

			/**
			 * @brief Multiply by an integer.
			 * @tparam T Integral type.
			 * @param value Factor.
			 * @return @c *this.
			 */
			template<Type::Integral T>
			constexpr Size& operator*=(T value) noexcept {
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
			constexpr Size& operator/=(T value) noexcept {
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
			constexpr Size& operator%=(T value) noexcept {
				*this = *this % value;
				return *this;
			}

		private:
			std::uint64_t m_value;	///< Unit count

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
			explicit constexpr Size(Raw, std::uint64_t value) noexcept: m_value(value) {}

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
			 * @return @ref Size.
			 */
			template<Type::Integral T>
			static constexpr Size MixAdd(std::uint64_t lhs, T rhs) noexcept {
				if constexpr (Type::Signed<T>) {
					if (rhs < T{0}) {
						const auto mag = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(rhs);
						return Size(Raw{}, Sub(lhs, mag));
					}
				}
				return Size(Raw{}, Add(lhs, static_cast<std::uint64_t>(rhs)));
			}

			/**
			 * @brief Subtract a possibly signed operand.
			 * @tparam T Integral type.
			 * @param lhs Left count.
			 * @param rhs Operand.
			 * @return @ref Size.
			 */
			template<Type::Integral T>
			static constexpr Size MixSub(std::uint64_t lhs, T rhs) noexcept {
				if constexpr (Type::Signed<T>) {
					if (rhs < T{0}) {
						const auto mag = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(rhs);
						return Size(Raw{}, Add(lhs, mag));
					}
				}
				return Size(Raw{}, Sub(lhs, static_cast<std::uint64_t>(rhs)));
			}

			/**
			 * @brief Multiply by a possibly signed operand.
			 * @tparam T Integral type.
			 * @param lhs Left count.
			 * @param rhs Operand.
			 * @return @ref Size.
			 */
			template<Type::Integral T>
			static constexpr Size MixMul(std::uint64_t lhs, T rhs) noexcept {
				if constexpr (Type::Signed<T>)
					assert(rhs >= T{0});
				return Size(Raw{}, Mul(lhs, static_cast<std::uint64_t>(rhs)));
			}
	};

	/**
	 * @brief Mixed equality.
	 * @tparam T Integral type.
	 * @param lhs Left count.
	 * @param rhs Right integer.
	 * @return Whether the numeric values match.
	 */
	template<Type::Integral T>
	constexpr bool operator==(Size lhs, T rhs) noexcept {
		return lhs == Size(rhs);
	}

	/**
	 * @brief Mixed equality.
	 * @tparam T Integral type.
	 * @param lhs Left integer.
	 * @param rhs Right count.
	 * @return Whether the numeric values match.
	 */
	template<Type::Integral T>
	constexpr bool operator==(T lhs, Size rhs) noexcept {
		return Size(lhs) == rhs;
	}

	/**
	 * @brief Mixed order.
	 * @tparam T Integral type.
	 * @param lhs Left count.
	 * @param rhs Right integer.
	 * @return Ordering.
	 */
	template<Type::Integral T>
	constexpr std::strong_ordering operator<=>(Size lhs, T rhs) noexcept {
		return lhs <=> Size(rhs);
	}

	/**
	 * @brief Mixed order.
	 * @tparam T Integral type.
	 * @param lhs Left integer.
	 * @param rhs Right count.
	 * @return Ordering.
	 */
	template<Type::Integral T>
	constexpr std::strong_ordering operator<=>(T lhs, Size rhs) noexcept {
		return Size(lhs) <=> rhs;
	}

	/**
	 * @brief Mixed sum.
	 * @tparam T Integral type.
	 * @param lhs Left addend.
	 * @param rhs Right addend.
	 * @return Sum as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator+(Size lhs, T rhs) noexcept {
		return Size::MixAdd(lhs.m_value, rhs);
	}

	/**
	 * @brief Mixed sum.
	 * @tparam T Integral type.
	 * @param lhs Left addend.
	 * @param rhs Right addend.
	 * @return Sum as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator+(T lhs, Size rhs) noexcept {
		return Size::MixAdd(rhs.m_value, lhs);
	}

	/**
	 * @brief Mixed difference.
	 * @tparam T Integral type.
	 * @param lhs Minuend.
	 * @param rhs Subtrahend.
	 * @return Difference as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator-(Size lhs, T rhs) noexcept {
		return Size::MixSub(lhs.m_value, rhs);
	}

	/**
	 * @brief Mixed difference.
	 * @tparam T Integral type.
	 * @param lhs Minuend.
	 * @param rhs Subtrahend.
	 * @return Difference as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator-(T lhs, Size rhs) noexcept {
		return Size(lhs) - rhs;
	}

	/**
	 * @brief Mixed product.
	 * @tparam T Integral type.
	 * @param lhs Left factor.
	 * @param rhs Right factor.
	 * @return Product as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator*(Size lhs, T rhs) noexcept {
		return Size::MixMul(lhs.m_value, rhs);
	}

	/**
	 * @brief Mixed product.
	 * @tparam T Integral type.
	 * @param lhs Left factor.
	 * @param rhs Right factor.
	 * @return Product as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator*(T lhs, Size rhs) noexcept {
		return Size::MixMul(rhs.m_value, lhs);
	}

	/**
	 * @brief Mixed quotient.
	 * @tparam T Integral type.
	 * @param lhs Dividend.
	 * @param rhs Divisor.
	 * @return Quotient as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator/(Size lhs, T rhs) noexcept {
		return lhs / Size(rhs);
	}

	/**
	 * @brief Mixed quotient.
	 * @tparam T Integral type.
	 * @param lhs Dividend.
	 * @param rhs Divisor.
	 * @return Quotient as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator/(T lhs, Size rhs) noexcept {
		return Size(lhs) / rhs;
	}

	/**
	 * @brief Mixed remainder.
	 * @tparam T Integral type.
	 * @param lhs Dividend.
	 * @param rhs Divisor.
	 * @return Remainder as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator%(Size lhs, T rhs) noexcept {
		return lhs % Size(rhs);
	}

	/**
	 * @brief Mixed remainder.
	 * @tparam T Integral type.
	 * @param lhs Dividend.
	 * @param rhs Divisor.
	 * @return Remainder as @ref Size.
	 */
	template<Type::Integral T>
	constexpr Size operator%(T lhs, Size rhs) noexcept {
		return Size(lhs) % rhs;
	}

	/**
	 * @brief Writes the decimal count.
	 * @param stream Destination.
	 * @param size Count.
	 * @return @p stream.
	 */
	inline std::ostream& operator<<(std::ostream& stream, Size size) {
		stream << static_cast<std::string>(size);
		return stream;
	}

	/// @cond
	extern template STORMBYTE_PUBLIC Size& Size::operator=(char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size& Size::operator+=(char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator+=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size& Size::operator-=(char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator-=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size& Size::operator*=(char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator*=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size& Size::operator/=(char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator/=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size& Size::operator%=(char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size& Size::operator%=(char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size::operator char() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator signed char() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator unsigned char() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator short() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator unsigned short() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator int() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator unsigned int() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator long() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator long long() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator wchar_t() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator char8_t() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator char16_t() const noexcept;
	extern template STORMBYTE_PUBLIC Size::operator char32_t() const noexcept;

	extern template STORMBYTE_PUBLIC bool operator==(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC bool operator==(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC bool operator==(char32_t, Size) noexcept;

	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC std::strong_ordering operator<=>(char32_t, Size) noexcept;

	extern template STORMBYTE_PUBLIC Size operator+(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size operator+(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator+(char32_t, Size) noexcept;

	extern template STORMBYTE_PUBLIC Size operator-(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size operator-(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator-(char32_t, Size) noexcept;

	extern template STORMBYTE_PUBLIC Size operator*(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size operator*(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator*(char32_t, Size) noexcept;

	extern template STORMBYTE_PUBLIC Size operator/(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size operator/(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator/(char32_t, Size) noexcept;

	extern template STORMBYTE_PUBLIC Size operator%(Size, char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, signed char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, unsigned char) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, unsigned short) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, unsigned int) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, unsigned long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, unsigned long long) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, wchar_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, char8_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, char16_t) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(Size, char32_t) noexcept;

	extern template STORMBYTE_PUBLIC Size operator%(char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(signed char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(unsigned char, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(unsigned short, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(unsigned int, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(unsigned long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(unsigned long long, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(wchar_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(char8_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(char16_t, Size) noexcept;
	extern template STORMBYTE_PUBLIC Size operator%(char32_t, Size) noexcept;
	/// @endcond
}

template<>
struct StormByte::Safe::Hash<StormByte::Size> {
	/**
	 * @brief Hash @p value.
	 * @param value Size.
	 * @return Hash.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::Size& value) const noexcept {
		const auto widened = static_cast<std::uint64_t>(static_cast<std::size_t>(value));
		return HashBytes(std::as_bytes(std::span<const std::uint64_t>(&widened, 1)));
	}
};

template<>
struct std::hash<StormByte::Size> {
	/**
	 * @brief Hash @p value.
	 * @param value Size.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Size& value) const noexcept {
		return StormByte::Safe::Hash<StormByte::Size>{}(value);
	}
};
