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

#include <StormByte/type_traits.hxx>

using namespace StormByte;

/**
 * @brief Bitwise OR for unsigned scoped enumerations.
 * @tparam E Enumeration type (`Type::UnsignedEnum`).
 * @param a Left operand.
 * @param b Right operand.
 * @return `E` with bits of @p a or @p b.
 */
template <Type::UnsignedEnum E>
constexpr E operator|(E a, E b) noexcept {
	return static_cast<E>(Type::ToUnderlying(a) | Type::ToUnderlying(b));
}

/**
 * @brief Bitwise AND for unsigned scoped enumerations.
 * @tparam E Enumeration type (`Type::UnsignedEnum`).
 * @param a Left operand.
 * @param b Right operand.
 * @return `E` with bits of @p a and @p b.
 */
template <Type::UnsignedEnum E>
constexpr E operator&(E a, E b) noexcept {
	return static_cast<E>(Type::ToUnderlying(a) & Type::ToUnderlying(b));
}

/**
 * @brief Bitwise XOR for unsigned scoped enumerations.
 * @tparam E Enumeration type (`Type::UnsignedEnum`).
 * @param a Left operand.
 * @param b Right operand.
 * @return `E` with bits of @p a xor @p b.
 */
template <Type::UnsignedEnum E>
constexpr E operator^(E a, E b) noexcept {
	return static_cast<E>(Type::ToUnderlying(a) ^ Type::ToUnderlying(b));
}

/**
 * @brief Bitwise NOT for unsigned scoped enumerations.
 * @tparam E Enumeration type (`Type::UnsignedEnum`).
 * @param a Operand.
 * @return `E` with bits of @p a inverted.
 */
template <Type::UnsignedEnum E>
constexpr E operator~(E a) noexcept {
	return static_cast<E>(~Type::ToUnderlying(a));
}

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	/**
	 * @class Bitmask
	 * @brief CRTP wrapper that stores an unsigned enum as a flag set.
	 * @tparam Derived Concrete bitmask type.
	 * @tparam E Flag enumeration (`Type::UnsignedEnum`).
	 *
	 * @code
	 * enum class MyFlags : uint8_t { FlagA = 0x01, FlagB = 0x02 };
	 * class MyBitmask : public Bitmask<MyBitmask, MyFlags> {
	 * public:
	 *     using Bitmask<MyBitmask, MyFlags>::Bitmask;
	 * };
	 * MyBitmask mask(MyFlags::FlagA);
	 * mask |= MyBitmask(MyFlags::FlagB);
	 * @endcode
	 */
	template<typename Derived, Type::UnsignedEnum E>
	class Bitmask {
		public:
			/**
			 * @brief Empty mask (`static_cast<E>(0)`).
			 */
			constexpr Bitmask() noexcept: m_value(static_cast<E>(0)) {}

			/**
			 * @brief Mask from a single enumerator (or already combined value).
			 * @param value Initial flags.
			 */
			constexpr Bitmask(E value) noexcept: m_value(value) {}

			/**
			 * @brief Copy constructor.
			 * @param other Source mask.
			 */
			constexpr Bitmask(const Bitmask& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Source mask.
			 */
			constexpr Bitmask(Bitmask&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			constexpr virtual ~Bitmask() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source mask.
			 * @return `*this`.
			 */
			constexpr Bitmask& operator=(const Bitmask& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Source mask.
			 * @return `*this`.
			 */
			constexpr Bitmask& operator=(Bitmask&& other) noexcept = default;

			/**
			 * @brief Equality.
			 * @param other Other mask.
			 * @return `true` when both store the same `E`.
			 */
			constexpr bool operator==(const Bitmask& other) const noexcept;

			/**
			 * @brief Inequality.
			 * @param other Other mask.
			 * @return `true` when the stored `E` differs.
			 */
			constexpr bool operator!=(const Bitmask& other) const noexcept;

			/**
			 * @brief Bitwise OR; returns `Derived`.
			 * @param other Other mask.
			 * @return New `Derived` with union of flags.
			 */
			constexpr Derived operator|(const Bitmask& other) const noexcept;

			/**
			 * @brief Bitwise AND; returns `Derived`.
			 * @param other Other mask.
			 * @return New `Derived` with intersection of flags.
			 */
			constexpr Derived operator&(const Bitmask& other) const noexcept;

			/**
			 * @brief Bitwise XOR; returns `Derived`.
			 * @param other Other mask.
			 * @return New `Derived` with toggled flags.
			 */
			constexpr Derived operator^(const Bitmask& other) const noexcept;

			/**
			 * @brief Bitwise NOT; returns `Derived`.
			 * @return New `Derived` with inverted flags.
			 */
			constexpr Derived operator~() const noexcept;

			/**
			 * @brief OR-assign.
			 * @param other Other mask.
			 * @return `*this`.
			 */
			constexpr Bitmask& operator|=(const Bitmask& other) noexcept;

			/**
			 * @brief AND-assign.
			 * @param other Other mask.
			 * @return `*this`.
			 */
			constexpr Bitmask& operator&=(const Bitmask& other) noexcept;

			/**
			 * @brief XOR-assign.
			 * @param other Other mask.
			 * @return `*this`.
			 */
			constexpr Bitmask& operator^=(const Bitmask& other) noexcept;

			/**
			 * @brief Sets every bit in @p value.
			 * @param value Flags to add.
			 */
			constexpr void Add(E value) noexcept;

			/**
			 * @brief Clears every bit in @p value.
			 * @param value Flags to remove.
			 */
			constexpr void Remove(E value) noexcept;

			/**
			 * @brief Stored enumerators.
			 * @return Current `E`.
			 */
			constexpr E Value() const noexcept;

			/**
			 * @brief `true` when every bit in @p value is set.
			 * @param value Flags to test.
			 */
			constexpr bool Has(E value) const noexcept;

			/**
			 * @brief `true` when every bit in @p other is set.
			 * @param other Other mask.
			 */
			constexpr bool Has(const Bitmask& other) const noexcept;

			/**
			 * @brief `true` when any bit in @p value is set.
			 * @param value Flags to test.
			 */
			constexpr bool HasAny(E value) const noexcept;

			/**
			 * @brief `true` when any bit in @p other is set.
			 * @param other Other mask.
			 */
			constexpr bool HasAny(const Bitmask& other) const noexcept;

			/**
			 * @brief `true` when no bit in @p value is set.
			 * @param value Flags to test.
			 */
			constexpr bool HasNone(E value) const noexcept;

			/**
			 * @brief `true` when no bit in @p other is set.
			 * @param other Other mask.
			 */
			constexpr bool HasNone(const Bitmask& other) const noexcept;

		protected:
			E m_value;	///< Packed flags.
	};
}

#include <StormByte/bitmask.txx>
