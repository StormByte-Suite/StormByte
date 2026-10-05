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
#include <StormByte/safe/function.hxx>

#include <string>
#include <string_view>

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
	/**
	 * @class ProviderException
	 * @brief Test exception whose destructor is anchored in the producer DLL.
	 */
	class SAFE_COLLECTIONS_PRODUCER_PUBLIC ProviderException: public StormByte::Exception {
		public:
			using StormByte::Exception::Exception;
			/**
			 * @brief Destructor defined in the producer DLL.
			 */
			~ProviderException() noexcept override;
	};

	/**
	 * @class MaybeValue
	 * @brief Consumer-registered value whose resource operations are implemented by the provider.
	 */
	class SAFE_COLLECTIONS_PRODUCER_PUBLIC MaybeValue final {
		public:
			/** @brief Construct an empty value. */
			MaybeValue();
			/** @brief Construct from copied text. @param value Text value. */
			explicit MaybeValue(std::string_view value);
			/** @brief Copy construct in the provider module. @param other Source value. */
			MaybeValue(const MaybeValue& other);
			/** @brief Move construct in the provider module. @param other Source value. */
			MaybeValue(MaybeValue&& other) noexcept;
			/** @brief Destroy resources in the provider module. */
			~MaybeValue() noexcept;
			/** @brief Copy assign in the provider module. @param other Source. @return This value. */
			MaybeValue& operator=(const MaybeValue& other);
			/** @brief Move assign in the provider module. @param other Source. @return This value. */
			MaybeValue& operator=(MaybeValue&& other) noexcept;
			/** @brief Borrow the value text. @return View valid while this object remains unchanged. */
			std::string_view Text() const noexcept;

		private:
			std::string m_text; ///< Provider-owned text resource.
	};

}

STORMBYTE_DECLARE_MAYBE_SAFE(SafeCollectionsFixture::MaybeValue);

namespace SafeCollectionsFixture {

	/**
	 * @brief Enum fixture used to verify Safe optional values across modules.
	 */
	using MaybeValues = StormByte::Safe::Vector<MaybeValue>; ///< Conditionally Safe provider-defined values.
	using MaybeOwners = StormByte::Safe::Vector<StormByte::Safe::Owner>; ///< Opaque provider-owned states.

	enum class OptionalTestLevel : std::uint8_t {
		Info,
		Warning
	};

	using Text = StormByte::Safe::String;	///< Base-owned UTF-8 value.
	using Sequence = StormByte::Safe::Vector<Text>;	///< Opaque text sequence.
	using Dictionary = StormByte::Safe::Map<Text, Text>;	///< Opaque text dictionary.
	using MaybeText = StormByte::Safe::Optional<Text>;	///< Opaque optional text.
	using MaybeLevel = StormByte::Safe::Optional<OptionalTestLevel>;	///< Opaque enum optional.
	using Nested = StormByte::Safe::Vector<Sequence>;	///< Nested opaque sequences.
	using TokenQueue = StormByte::Safe::Queue<Text>;	///< Opaque text FIFO.
	using CallbackFunction = StormByte::Safe::Function<void(double)>; ///< Function signature for callbacks.
	using SizeSelector = StormByte::Safe::Function<StormByte::Size(StormByte::Size)>; ///< Typed size selector.

	/**
	 * @brief Construct and populate text storage in the producer DLL.
	 * @return Owned sequence.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC Sequence MakeSequence();

	/**
	 * @brief Create provider-owned MaybeSafe values in a Safe collection.
	 * @return Owned values.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC MaybeValues MakeMaybeValues();

	/**
	 * @brief Create opaque MaybeValue states owned and cloned by the producer.
	 * @return Opaque owners.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC MaybeOwners MakeMaybeOwners();

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
	 * @brief Construct enum optional storage in the producer DLL.
	 * @return Owned enum optional.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC MaybeLevel MakeOptionalLevel();

	/**
	 * @brief Construct nested storage in the producer DLL.
	 * @return Owned nested collection.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC Nested MakeNested();

	/**
	 * @brief Construct a callback with a clonable producer-owned STL context.
	 * @return Copyable callback whose copies receive independent contexts.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC StormByte::Safe::Callback MakeCallback();

	/**
	 * @brief Construct a typed progress callback with clonable producer-owned context.
	 * @return Copyable callback whose copies receive independent contexts.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC CallbackFunction MakeProgressCallback();

	/**
	 * @brief Construct a typed Size-to-Size selector with clonable producer context.
	 * @return Copyable selector whose copies receive independent contexts.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC SizeSelector MakeSizeSelector();

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
	 * @brief Throw a derived provider exception across the producer/consumer boundary.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC void ThrowProviderException();

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
	 * @brief Number of progress callback contexts still alive.
	 * @return Fixed-width count.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC std::uint64_t LiveProgressContexts() noexcept;

	/**
	 * @brief Number of progress callback contexts destroyed by the producer.
	 * @return Fixed-width count.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC std::uint64_t DestroyedProgressContexts() noexcept;

	/**
	 * @brief Last percentage passed to the typed progress callback.
	 * @return Percentage captured by the producer.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC double LastProgress() noexcept;

	/**
	 * @brief Number of MaybeValue objects currently alive.
	 * @return Fixed-width count.
	 */
	SAFE_COLLECTIONS_PRODUCER_PUBLIC std::uint64_t LiveMaybeValues() noexcept;

	/**
	 * @brief Copy, move, modify, read and destroy producer values in another DLL.
	 * @return True when every ownership and behavior check passes.
	 */
		SAFE_COLLECTIONS_CONSUMER_PUBLIC bool ExerciseCollections();
}
