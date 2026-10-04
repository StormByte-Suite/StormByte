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

#include <StormByte/safe/callback.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/queue.hxx>

#ifdef WINDOWS
	#ifdef SafeCollectionsProducer_EXPORTS
		#define SAFE_COLLECTIONS_PRODUCER_PUBLIC __declspec(dllexport)
	#else
		#define SAFE_COLLECTIONS_PRODUCER_PUBLIC __declspec(dllimport)
	#endif
	#ifdef SafeCollectionsConsumer_EXPORTS
		#define SAFE_COLLECTIONS_CONSUMER_PUBLIC __declspec(dllexport)
	#else
		#define SAFE_COLLECTIONS_CONSUMER_PUBLIC __declspec(dllimport)
	#endif
#else
	#define SAFE_COLLECTIONS_PRODUCER_PUBLIC __attribute__((visibility("default")))
	#define SAFE_COLLECTIONS_CONSUMER_PUBLIC __attribute__((visibility("default")))
#endif

/**
 * @namespace SafeCollectionsFixture
 * @brief Test-only interfaces between independent producer and consumer DLLs.
 */
namespace SafeCollectionsFixture {
	using Text = StormByte::Safe::String;	///< Base-owned UTF-8 value.
	using Sequence = StormByte::Safe::Vector<Text>;	///< Opaque text sequence.
	using Dictionary = StormByte::Safe::Map<Text, Text>;	///< Opaque text dictionary.
	using MaybeText = StormByte::Safe::Optional<Text>;	///< Opaque optional text.
	using Nested = StormByte::Safe::Vector<Sequence>;	///< Nested opaque sequences.
	using TokenQueue = StormByte::Safe::Queue<Text>;	///< Opaque text FIFO.

	/**
	 * @brief Construct and populate text storage in the producer DLL.
	 * @return Owned sequence.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC Sequence MakeSequence();

	/**
	 * @brief Construct nodes in the producer DLL.
	 * @return Owned dictionary.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC Dictionary MakeDictionary();

	/**
	 * @brief Construct nontrivial optional state in the producer DLL.
	 * @return Owned optional.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC MaybeText MakeOptional();

	/**
	 * @brief Construct nested storage in the producer DLL.
	 * @return Owned nested collection.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC Nested MakeNested();

	/**
	 * @brief Construct a callback with a producer-owned STL context.
	 * @return Move-only callback.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC StormByte::Safe::Callback MakeCallback();

	/**
	 * @brief Create a token queue in the producer DLL using String::Explode.
	 * @return Owned token queue.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC TokenQueue MakeTokenQueue();

	/**
	 * @brief Throw a Base exception from the producer DLL.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC void ThrowProducerException();

	/**
	 * @brief Number of producer contexts still alive.
	 * @return Fixed-width count.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC std::uint64_t LiveContexts() noexcept;

	/**
	 * @brief Number of contexts destroyed by the producer.
	 * @return Fixed-width count.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC std::uint64_t DestroyedContexts() noexcept;

	/**
	 * @brief Copy, move, modify, read and destroy producer values in another DLL.
	 * @return True when every ownership and behavior check passes.
	 */
	SAFE_COLLECTIONS_CONSUMER_PUBLIC bool ExerciseCollections();
}