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

#include <StormByte/safe/owner.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits/safe.hxx>

#include <map>

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
		 * @class Map
		 * @brief Opaque ordered dictionary with creator-module storage and callbacks.
		 * @tparam K Safe value with a strict weak ordering through operator<.
		 * @tparam V Safe value.
		 *
		 * Copy duplicates entries in the creator; move leaves an empty readable map.
		 * Assign a fresh Map before modifying a moved-from object. Read operations
		 * copy values, never lend references. Set inserts or replaces transactionally.
		 * Failure leaves the map unchanged. Missing leaves outputs unchanged.
		 * Indices follow key order and may change after modification. GetAt is linear.
		 * Requires external synchronization, loaded creator/Base modules and compatible
		 * C++ ABI. No custom comparators or STL storage cross the public boundary.
		 */
		template<Type::SafeValue K, Type::SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		class STORMBYTE_PUBLIC_TYPE Map final {
			public:
				/**
				 * @brief Construct an empty map in the calling module.
				 */
				STORMBYTE_FORCE_INLINE Map();

				/**
				 * @brief Copy entries from a caller-owned STL map.
				 * @param values Source map; its allocation remains with its owner.
				 */
				explicit Map(const std::map<K, V>& values);

				/**
				 * @brief Copy entries from an STL rvalue without adopting its allocation.
				 * @param values Source map; it remains valid and unchanged.
				 */
				explicit Map(std::map<K, V>&& values);

				/**
				 * @brief Deep copy in the original module.
				 * @param other Source.
				 */
				Map(const Map& other) = default;

				/**
				 * @brief Transfer state; source becomes empty.
				 * @param other Source.
				 */
				Map(Map&& other) noexcept = default;

				/**
				 * @brief Destroy nodes in the creator module.
				 */
				~Map() noexcept = default;

				/**
				 * @brief Deep copy with strong guarantee.
				 * @param other Source.
				 * @return This map.
				 */
				Map& operator=(const Map& other) = default;

				/**
				 * @brief Release old state and transfer.
				 * @param other Source.
				 * @return This map.
				 */
				Map& operator=(Map&& other) noexcept = default;

				/**
				 * @brief Number of entries.
				 * @return Count, including zero after move.
				 */
				StormByte::Size Size() const noexcept;

				/**
				 * @brief Copy a value found by key.
				 * @param key Lookup key.
				 * @param output Destination.
				 * @return Operation status.
				 */
				Status Get(const K& key, V& output) const noexcept;

				/**
				 * @brief Copy an entry at its ordered index.
				 * @param index Position in key order.
				 * @param key Destination key.
				 * @param value Destination value; must not alias key.
				 * @return Operation status.
				 */
				Status GetAt(const StormByte::Size& index, K& key, V& value) const noexcept;

				/**
				 * @brief Insert or replace a copied value.
				 * @param key Entry key.
				 * @param value Source value.
				 * @return Operation status.
				 */
				Status Set(const K& key, const V& value) noexcept;

				/**
				 * @brief Remove a key.
				 * @param key Lookup key.
				 * @return Missing if absent, otherwise Success or Failure.
				 */
				Status Erase(const K& key) noexcept;

				/**
				 * @brief Destroy all entries locally.
				 * @return Operation status.
				 */
				Status Clear() noexcept;

				/**
				 * @brief Copy entries into caller-owned STL storage.
				 * @return A std::map allocated and destroyed in the caller module.
				 */
				STORMBYTE_FORCE_INLINE explicit operator std::map<K, V>() const;

			private:
				/**
				 * @brief Creator-local implementation.
				 */
				struct STORMBYTE_PRIVATE Store;

				/**
				 * @brief Internal operation identifier.
				 */
				enum class Action {
					Get, ///< Copy an entry value by key.
					GetAt, ///< Copy an entry by ordered index.
					Set, ///< Insert or replace an entry.
					Erase, ///< Remove an entry by key.
					Clear ///< Remove all entries.
				};

				/**
				 * @brief Creator-local operation callback.
				 */
				using Dispatch = Status (*)(void*, Action, const StormByte::Size&, const K*, const V*, K*, V*) noexcept;

				/**
				 * @brief Creator-local count callback.
				 */
				using Count = StormByte::Size (*)(const void*) noexcept;

				/**
				 * @brief Caller-module insertion callback used by STL conversion.
				 */
				using InsertEntry = Status (*)(void*, const K&, const V&) noexcept;

				/**
				 * @brief Creator-module ordered traversal callback.
				 */
				using VisitEntries = Status (*)(const void*, void*, InsertEntry) noexcept;
				VisitEntries m_visit; ///< Ordered traversal in the creator module.

				Detail::Owner m_owner;	///< State with creator-module lifetime callbacks.
				Dispatch m_dispatch;		///< Creator-module operations.
				Count m_count;			///< Creator-module count.
		};
	}

	/**
	 * @namespace StormByte::Type
	 * @brief Named concepts and small type utilities used across the suite.
	 */
	namespace Type {
		/**
		 * @brief Recognizes opaque Safe dictionaries.
		 * @tparam K Safe ordered key.
		 * @tparam V Safe value.
		 */
		template<SafeValue K, SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		struct IsSafe<Safe::Map<K, V>>: std::true_type {};

		/**
		 * @brief Admits nested opaque dictionaries.
		 * @tparam K Safe ordered key.
		 * @tparam V Safe value.
		 */
		template<SafeValue K, SafeValue V>
		requires requires(const K& left, const K& right) { left < right; }
		struct IsSafeValue<Safe::Map<K, V>>: std::true_type {};
	}
}

#include <StormByte/safe/map.txx>