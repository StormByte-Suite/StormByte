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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/hash.hxx>
#include <StormByte/safe/heap.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <compare>
#include <cstddef>
#include <functional>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

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
		 * @struct Monostate
		 * @brief Empty alternative. The stand-in for a default-constructed variant that holds no payload.
		 */
		struct Monostate {
			/**
			 * @brief Two empty alternatives are equal.
			 * @return Equality.
			 */
			friend constexpr bool operator==(Monostate, Monostate) noexcept {
				return true;
			}

			/**
			 * @brief Two empty alternatives compare equal.
			 * @return Equal ordering.
			 */
			friend constexpr std::strong_ordering operator<=>(Monostate, Monostate) noexcept {
				return std::strong_ordering::equal;
			}
		};
	}
}

template<>
struct StormByte::Type::IsSafe<StormByte::Safe::Monostate>: std::true_type {};

template<>
struct StormByte::Type::IsSafeValue<StormByte::Safe::Monostate>: std::true_type {};

namespace StormByte::Safe {
		/**
		 * @class Variant
		 * @brief `std::variant` stored on Base's heap.
		 * @tparam Ts Alternatives. Each one is a @ref StormByte::Type::SafeValue.
		 *
		 * Not a `std::variant`. The active alternative is constructed in a block from @ref Heap::Allocate and released with @ref Heap::Free. A move of this variant leaves the source valueless. A move from `std::variant` does not change its index. `emplace` builds the replacement first, so a throw keeps the previous alternative. `get` and `visit` are found by argument lookup; `std::get` and `std::visit` do not accept this type.
		 */
		template<Type::SafeValue... Ts>
		requires (sizeof...(Ts) > 0)
		class STORMBYTE_PUBLIC_TYPE Variant final {
			public:
				static constexpr std::size_t npos = static_cast<std::size_t>(-1);	///< No alternative.

				/**
				 * @brief Index of the single alternative constructible from @p T, or @ref npos.
				 * @tparam T Source type.
				 * @return Index, or @ref npos when none or more than one matches.
				 */
				template<class T>
				static constexpr std::size_t AlternativeIndex() noexcept;

				/**
				 * @brief Default-construct the first alternative.
				 * @throws AllocationError The block could not be allocated.
				 */
				Variant();

				/**
				 * @brief Copy an alternative into Base storage.
				 * @tparam T Alternative. Exactly one alternative is constructible from @p value.
				 * @param value Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class T>
				requires (Variant::template AlternativeIndex<T>() != npos)
				Variant(T&& value): m_storage(nullptr), m_index(npos) {
					constexpr std::size_t index = AlternativeIndex<T>();
					using Alternative = std::tuple_element_t<index, std::tuple<Ts...>>;
					m_storage = Make<Alternative>(std::forward<T>(value));
					m_index = index;
				}

				/**
				 * @brief Construct the alternative at @p I.
				 * @tparam I Alternative index.
				 * @tparam Args Constructor argument types.
				 * @param index In-place index tag.
				 * @param args Arguments forwarded to the alternative.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<std::size_t I, class... Args>
				explicit Variant(std::in_place_index_t<I> index, Args&&... args);

				/**
				 * @brief Construct the alternative @p T.
				 * @tparam T Alternative. It occurs once.
				 * @tparam Args Constructor argument types.
				 * @param tag In-place type tag.
				 * @param args Arguments forwarded to @p T.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class T, class... Args>
				explicit Variant(std::in_place_type_t<T> tag, Args&&... args);

				/**
				 * @brief Copy a caller-owned `std::variant`. A valueless source stays valueless.
				 * @param value Source. Its index is unchanged.
				 * @throws AllocationError The block could not be allocated.
				 */
				explicit Variant(const std::variant<Ts...>& value);

				/**
				 * @brief Move a caller-owned `std::variant` by copying the alternative into Base storage.
				 * @param value Source. Its index is unchanged; a held alternative is moved-from.
				 * @throws AllocationError The block could not be allocated.
				 */
				explicit Variant(std::variant<Ts...>&& value);

				/**
				 * @brief Copy the active alternative into a new Base block.
				 * @param other Source.
				 * @throws AllocationError The block could not be allocated.
				 */
				Variant(const Variant& other);

				/**
				 * @brief Take the active block. @p other becomes valueless.
				 * @param other Source.
				 */
				Variant(Variant&& other) noexcept;

				/**
				 * @brief Destroy the active alternative and release its block.
				 */
				~Variant() noexcept;

				/**
				 * @brief Copy-assign. The replacement is built first.
				 * @param other Source.
				 * @return This variant.
				 * @throws AllocationError The block could not be allocated. The previous alternative is kept.
				 */
				Variant& operator=(const Variant& other);

				/**
				 * @brief Move-assign. The block is taken. @p other becomes valueless.
				 * @param other Source.
				 * @return This variant.
				 */
				Variant& operator=(Variant&& other) noexcept;

				/**
				 * @brief Assign an alternative. The replacement is built first.
				 * @tparam T Alternative.
				 * @param value Source.
				 * @return This variant.
				 * @throws AllocationError The block could not be allocated. The previous alternative is kept.
				 */
				template<class T>
				requires (Variant::template AlternativeIndex<T>() != npos)
				Variant& operator=(T&& value) {
					emplace<std::tuple_element_t<Variant::template AlternativeIndex<T>(), std::tuple<Ts...>>>(std::forward<T>(value));
					return *this;
				}

				/**
				 * @brief Index of the active alternative, or @ref npos when valueless.
				 * @return Index.
				 */
				std::size_t index() const noexcept;

				/**
				 * @brief Whether a throwing operation left this variant without an alternative.
				 * @return Valueless state. `emplace` does not enter it.
				 */
				bool valueless_by_exception() const noexcept;

				/**
				 * @brief Construct the alternative at @p I, replacing the current one only after construction.
				 * @tparam I Alternative index.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to the alternative.
				 * @return The new alternative.
				 * @throws AllocationError The block could not be allocated. The previous alternative is kept.
				 */
				template<std::size_t I, class... Args>
				auto& emplace(Args&&... args);

				/**
				 * @brief Construct @p T, replacing the current alternative only after construction.
				 * @tparam T Alternative. It occurs once.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p T.
				 * @return The new alternative.
				 * @throws AllocationError The block could not be allocated. The previous alternative is kept.
				 */
				template<class T, class... Args>
				T& emplace(Args&&... args);

				/**
				 * @brief Exchange storage. Both blocks stay on Base's heap.
				 * @param other Other variant.
				 */
				void swap(Variant& other) noexcept;

				/**
				 * @brief Content equality. Two valueless variants are equal.
				 * @param other Other variant.
				 * @return Whether the active alternatives are equal.
				 */
				bool operator==(const Variant& other) const;

				/**
				 * @brief Index order, then alternative order when the indices match.
				 * @param other Other variant.
				 * @return Ordering.
				 */
				std::strong_ordering operator<=>(const Variant& other) const;

				/**
				 * @brief Copy the active alternative into caller-owned STL storage.
				 * @return A `std::variant` owned by the caller.
				 * @throws BadVariantAccess This variant is valueless.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::variant<Ts...>() const {
					if (valueless_by_exception())
						throw BadVariantAccess("wrong alternative");
					return Visit([](const auto& alternative) -> std::variant<Ts...> {
						return alternative;
					});
				}

			private:
				/**
				 * @brief Allocate a Base block and construct @p Alternative in it.
				 * @tparam Alternative Alternative type.
				 * @tparam Args Constructor argument types.
				 * @param args Arguments forwarded to @p Alternative.
				 * @return Pointer to the constructed alternative.
				 * @throws AllocationError The block could not be allocated.
				 */
				template<class Alternative, class... Args>
				static Alternative* Make(Args&&... args);

				/**
				 * @brief Destroy the active alternative and release its block. Valueless is a no-op.
				 */
				void Release() noexcept;

				/**
				 * @brief Call @p visitor with the alternative at @p I, or continue.
				 * @tparam I Alternative index.
				 * @tparam Visitor Callable.
				 * @param visitor Callable.
				 * @return Whatever @p visitor returns. A `void` visitor is not stored.
				 * @throws BadVariantAccess This variant is valueless or the index is past the pack.
				 */
				template<std::size_t I, class Visitor>
				decltype(auto) VisitIndex(Visitor&& visitor) const;

				/**
				 * @brief Call @p visitor with the active alternative.
				 * @tparam Visitor Callable.
				 * @param visitor Callable.
				 * @return Whatever @p visitor returns.
				 * @throws BadVariantAccess This variant is valueless.
				 */
				template<class Visitor>
				decltype(auto) Visit(Visitor&& visitor) const;

				void* m_storage;	///< Base block of the active alternative, or null when valueless.
				std::size_t m_index;	///< Active alternative, or @ref npos.

				template<std::size_t I, Type::SafeValue... Us>
				friend auto* get_if(Variant<Us...>* value) noexcept;

				template<std::size_t I, Type::SafeValue... Us>
				friend const auto* get_if(const Variant<Us...>* value) noexcept;

				template<class T, Type::SafeValue... Us>
				friend T* get_if(Variant<Us...>* value) noexcept;

				template<class T, Type::SafeValue... Us>
				friend const T* get_if(const Variant<Us...>* value) noexcept;

				template<class Visitor, Type::SafeValue... Us>
				friend decltype(auto) visit(Visitor&& visitor, Variant<Us...>& value);

				template<class Visitor, Type::SafeValue... Us>
				friend decltype(auto) visit(Visitor&& visitor, const Variant<Us...>& value);
		};

		/**
		 * @brief Whether @p value holds @p T.
		 * @tparam T Alternative.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Match. A valueless variant does not hold @p T.
		 */
		template<class T, Type::SafeValue... Ts>
		bool holds_alternative(const Variant<Ts...>& value) noexcept;

		/**
		 * @brief Read the alternative at @p I.
		 * @tparam I Alternative index.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Alternative.
		 * @throws BadVariantAccess The index is not active.
		 */
		template<std::size_t I, Type::SafeValue... Ts>
		auto& get(Variant<Ts...>& value);

		/**
		 * @brief Read the alternative at @p I.
		 * @tparam I Alternative index.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Alternative.
		 * @throws BadVariantAccess The index is not active.
		 */
		template<std::size_t I, Type::SafeValue... Ts>
		const auto& get(const Variant<Ts...>& value);

		/**
		 * @brief Read the alternative @p T.
		 * @tparam T Alternative. It occurs once.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Alternative.
		 * @throws BadVariantAccess @p T is not active.
		 */
		template<class T, Type::SafeValue... Ts>
		T& get(Variant<Ts...>& value);

		/**
		 * @brief Read the alternative @p T.
		 * @tparam T Alternative. It occurs once.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Alternative.
		 * @throws BadVariantAccess @p T is not active.
		 */
		template<class T, Type::SafeValue... Ts>
		const T& get(const Variant<Ts...>& value);

		/**
		 * @brief Pointer to the alternative at @p I, or null.
		 * @tparam I Alternative index.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Pointer, or null when the index is not active.
		 */
		template<std::size_t I, Type::SafeValue... Ts>
		auto* get_if(Variant<Ts...>* value) noexcept;

		/**
		 * @brief Pointer to the alternative at @p I, or null.
		 * @tparam I Alternative index.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Pointer, or null when the index is not active.
		 */
		template<std::size_t I, Type::SafeValue... Ts>
		const auto* get_if(const Variant<Ts...>* value) noexcept;

		/**
		 * @brief Pointer to the alternative @p T, or null.
		 * @tparam T Alternative. It occurs once.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Pointer, or null when @p T is not active.
		 */
		template<class T, Type::SafeValue... Ts>
		T* get_if(Variant<Ts...>* value) noexcept;

		/**
		 * @brief Pointer to the alternative @p T, or null.
		 * @tparam T Alternative. It occurs once.
		 * @tparam Ts Alternatives.
		 * @param value Variant.
		 * @return Pointer, or null when @p T is not active.
		 */
		template<class T, Type::SafeValue... Ts>
		const T* get_if(const Variant<Ts...>* value) noexcept;

		/**
		 * @brief Call @p visitor with the active alternative.
		 * @tparam Visitor Callable.
		 * @tparam Ts Alternatives.
		 * @param visitor Callable.
		 * @param value Variant.
		 * @return Whatever @p visitor returns.
		 * @throws BadVariantAccess @p value is valueless.
		 */
		template<class Visitor, Type::SafeValue... Ts>
		decltype(auto) visit(Visitor&& visitor, Variant<Ts...>& value);

		/**
		 * @brief Call @p visitor with the active alternative.
		 * @tparam Visitor Callable.
		 * @tparam Ts Alternatives.
		 * @param visitor Callable.
		 * @param value Variant.
		 * @return Whatever @p visitor returns.
		 * @throws BadVariantAccess @p value is valueless.
		 */
		template<class Visitor, Type::SafeValue... Ts>
		decltype(auto) visit(Visitor&& visitor, const Variant<Ts...>& value);

		/**
		 * @brief Swaps two variants.
		 * @tparam Ts Alternatives.
		 * @param left First variant.
		 * @param right Second variant.
		 */
		template<Type::SafeValue... Ts>
		void swap(Variant<Ts...>& left, Variant<Ts...>& right) noexcept;
}

template<StormByte::Type::SafeValue... Ts>
struct StormByte::Type::IsSafe<StormByte::Safe::Variant<Ts...>>: std::true_type {};

template<StormByte::Type::SafeValue... Ts>
struct StormByte::Type::IsSafeValue<StormByte::Safe::Variant<Ts...>>: std::true_type {};

template<StormByte::Type::SafeValue... Ts>
struct StormByte::Safe::Hash<StormByte::Safe::Variant<Ts...>> {
	/**
	 * @brief Hash the index and the active alternative. A valueless variant hashes as the index only.
	 * @param value Variant.
	 * @return Hash.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(const StormByte::Safe::Variant<Ts...>& value) const noexcept {
		std::size_t hash = Hash<std::size_t>{}(value.index());
		if (value.valueless_by_exception())
			return hash;
		std::size_t current = 0;
		auto step = [&](auto* unused) {
			using Alternative = std::remove_pointer_t<decltype(unused)>;
			if (current == value.index()) {
				const Alternative* alternative = get_if<Alternative>(&value);
				if (alternative != nullptr)
					hash = HashCombine(hash, Hash<Alternative>{}(*alternative));
			}
			++current;
		};
		(step(static_cast<Ts*>(nullptr)), ...);
		return hash;
	}
};

template<>
struct StormByte::Safe::Hash<StormByte::Safe::Monostate> {
	/**
	 * @brief Hash an empty alternative.
	 * @return Constant hash.
	 */
	STORMBYTE_FORCE_INLINE std::size_t operator()(StormByte::Safe::Monostate) const noexcept {
		return HashBytes({});
	}
};

template<StormByte::Type::SafeValue... Ts>
struct std::hash<StormByte::Safe::Variant<Ts...>> {
	/**
	 * @brief Hashes @p value.
	 * @param value Variant.
	 * @return Hash.
	 */
	std::size_t operator()(const StormByte::Safe::Variant<Ts...>& value) const noexcept {
		return StormByte::Safe::Hash<StormByte::Safe::Variant<Ts...>>{}(value);
	}
};

template<>
struct std::hash<StormByte::Safe::Monostate> {
	/**
	 * @brief Hashes an empty alternative.
	 * @return Hash.
	 */
	std::size_t operator()(StormByte::Safe::Monostate value) const noexcept {
		return StormByte::Safe::Hash<StormByte::Safe::Monostate>{}(value);
	}
};

#include <StormByte/safe/variant.txx>
