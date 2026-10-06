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

#include <utility>

namespace StormByte {
	namespace Safe {
		template<Type::SafeValue First, Type::SafeValue Second>
		Pair<First, Second>::Pair(const First& firstValue, const Second& secondValue):
			first(firstValue), second(secondValue) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		Pair<First, Second>::Pair(First&& firstValue, Second&& secondValue) noexcept(
			std::is_nothrow_move_constructible_v<First> &&
			std::is_nothrow_move_constructible_v<Second>):
			first(std::move(firstValue)), second(std::move(secondValue)) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class U, class V>
		requires Type::ConstructibleFrom<First, U> && Type::ConstructibleFrom<Second, V>
		Pair<First, Second>::Pair(U&& firstValue, V&& secondValue):
			first(std::forward<U>(firstValue)), second(std::forward<V>(secondValue)) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class... FirstArgs, class... SecondArgs>
		Pair<First, Second>::Pair(std::piecewise_construct_t tag, std::tuple<FirstArgs...> firstArgs, std::tuple<SecondArgs...> secondArgs):
			Pair(tag, firstArgs, secondArgs, std::index_sequence_for<FirstArgs...>{}, std::index_sequence_for<SecondArgs...>{}) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::ConstructibleFrom<First, const OtherFirst&> &&
			Type::ConstructibleFrom<Second, const OtherSecond&>
		Pair<First, Second>::Pair(const std::pair<OtherFirst, OtherSecond>& other):
			first(other.first), second(other.second) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::ConstructibleFrom<First, OtherFirst> &&
			Type::ConstructibleFrom<Second, OtherSecond>
		Pair<First, Second>::Pair(std::pair<OtherFirst, OtherSecond>&& other):
			first(std::move(other.first)), second(std::move(other.second)) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::ConstructibleFrom<First, const OtherFirst&> &&
			Type::ConstructibleFrom<Second, const OtherSecond&>
		Pair<First, Second>::Pair(const Pair<OtherFirst, OtherSecond>& other):
			first(other.first), second(other.second) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::ConstructibleFrom<First, OtherFirst> &&
			Type::ConstructibleFrom<Second, OtherSecond>
		Pair<First, Second>::Pair(Pair<OtherFirst, OtherSecond>&& other):
			first(std::move(other.first)), second(std::move(other.second)) {}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::AssignableFrom<First&, const OtherFirst&> &&
			Type::AssignableFrom<Second&, const OtherSecond&>
		Pair<First, Second>& Pair<First, Second>::operator=(const Pair<OtherFirst, OtherSecond>& other) {
			first = other.first;
			second = other.second;
			return *this;
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::AssignableFrom<First&, OtherFirst> &&
			Type::AssignableFrom<Second&, OtherSecond>
		Pair<First, Second>& Pair<First, Second>::operator=(Pair<OtherFirst, OtherSecond>&& other) {
			first = std::move(other.first);
			second = std::move(other.second);
			return *this;
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::AssignableFrom<First&, const OtherFirst&> &&
			Type::AssignableFrom<Second&, const OtherSecond&>
		Pair<First, Second>& Pair<First, Second>::operator=(const std::pair<OtherFirst, OtherSecond>& other) {
			first = other.first;
			second = other.second;
			return *this;
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class OtherFirst, class OtherSecond>
		requires Type::AssignableFrom<First&, OtherFirst> &&
			Type::AssignableFrom<Second&, OtherSecond>
		Pair<First, Second>& Pair<First, Second>::operator=(std::pair<OtherFirst, OtherSecond>&& other) {
			first = std::move(other.first);
			second = std::move(other.second);
			return *this;
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		void Pair<First, Second>::swap(Pair& other) noexcept(Type::Swappable<First> && Type::Swappable<Second>) {
			using std::swap;
			swap(first, other.first);
			swap(second, other.second);
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		bool Pair<First, Second>::operator==(const Pair& other) const requires Type::EqualityComparable<First> && Type::EqualityComparable<Second> {
			return first == other.first && second == other.second;
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		auto Pair<First, Second>::operator<=>(const Pair& other) const requires Type::ThreeWayComparable<First> && Type::ThreeWayComparable<Second> {
			if (auto order = first <=> other.first; order != 0)
				return order;
			return second <=> other.second;
		}

		template<Type::SafeValue First, Type::SafeValue Second>
		template<class... FirstArgs, class... SecondArgs, std::size_t... FirstIndexes, std::size_t... SecondIndexes>
		Pair<First, Second>::Pair(std::piecewise_construct_t, std::tuple<FirstArgs...>& firstArgs, std::tuple<SecondArgs...>& secondArgs,
			std::index_sequence<FirstIndexes...>, std::index_sequence<SecondIndexes...>):
			first(std::get<FirstIndexes>(std::move(firstArgs))...),
			second(std::get<SecondIndexes>(std::move(secondArgs))...) {}

		template<Type::SafeValue First, Type::SafeValue Second, class SecondReference>
		PairReference<First, Second, SecondReference>::PairReference(const First& firstValue, const Second& secondValue, SecondReference secondReference):
			first(firstValue), second(std::move(secondReference)), m_snapshot(firstValue, secondValue) {}

		template<Type::SafeValue First, Type::SafeValue Second, class SecondReference>
		PairReference<First, Second, SecondReference>::operator const typename PairReference<First, Second, SecondReference>::value_type&() const {
			m_snapshot.second = static_cast<Second>(second);
			return m_snapshot;
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr First& get(Pair<First, Second>& pair) noexcept {
			return pair.first;
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr Second& get(Pair<First, Second>& pair) noexcept {
			return pair.second;
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr const First& get(const Pair<First, Second>& pair) noexcept {
			return pair.first;
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr const Second& get(const Pair<First, Second>& pair) noexcept {
			return pair.second;
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr First&& get(Pair<First, Second>&& pair) noexcept {
			return std::move(pair.first);
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr Second&& get(Pair<First, Second>&& pair) noexcept {
			return std::move(pair.second);
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 0)
		constexpr const First&& get(const Pair<First, Second>&& pair) noexcept {
			return std::move(pair.first);
		}

		template<std::size_t Index, Type::SafeValue First, Type::SafeValue Second>
		requires (Index == 1)
		constexpr const Second&& get(const Pair<First, Second>&& pair) noexcept {
			return std::move(pair.second);
		}
	}
}
