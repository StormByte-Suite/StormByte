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

#include <StormByte/byte_size.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/expected.hxx>
#include <StormByte/helpers.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/type_traits.hxx>
#include <StormByte/visibility.h>

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte suite.
 */
namespace StormByte {
	class DeserializeError;

	/**
	 * @namespace StormByte::Detail
	 * @brief Private specialization point for non-trivial types.
	 *
	 * Other StormByte modules (and only those) specialize @ref StormByte::Detail::Codec
	 * for their own types. Do not specialize @ref StormByte::Serializable itself.
	 */
	namespace Detail {
		/**
		 * @brief Compile-time false dependent on @p T.
		 * @tparam T Ignored; exists so `static_assert` is delayed to instantiation.
		 */
		template<typename T>
		constexpr bool codec_always_false_v = false;

		/**
		 * @struct Codec
		 * @brief Size / write / read for one type @p T.
		 * @tparam T Type being encoded.
		 *
		 * Primary template is intentionally unusable. Specialize the whole
		 * struct (all three members) for a type that is not optional, pair,
		 * container or trivially copyable.
		 *
		 * @code
		 * template<>
		 * struct StormByte::Detail::Codec<MyType> {
		 *     static ByteSize Size(const MyType& v) noexcept;
		 *     static Safe::Binary Write(const MyType& v) noexcept;
		 *     static Expected<MyType, DeserializeError> Read(std::span<const std::byte>) noexcept;
		 * };
		 * @endcode
		 *
		 * @note The three members must agree with each other. `Size(v)` must
		 *       equal `Write(v).size()` as a byte count. `Read` consumes a
		 *       prefix of that length; leftover bytes belong to the caller.
		 */
		template<typename T>
		struct Codec {
			/**
			 * @brief Serialized size of @p data.
			 * @param[in] data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static ByteSize Size(const T& data) noexcept {
				static_assert(codec_always_false_v<T>,
					"Specialize StormByte::Detail::Codec<T> instead of Serializable<T>");
				(void)data;
				return ByteSize{0};
			}

			/**
			 * @brief Encodes @p data to a little-endian blob.
			 * @param[in] data Value to encode.
			 * @return Blob owned by Base. No framing beyond what @p T itself needs.
			 */
			static Safe::Binary Write(const T& data) noexcept {
				static_assert(codec_always_false_v<T>,
					"Specialize StormByte::Detail::Codec<T> instead of Serializable<T>");
				(void)data;
				return {};
			}

			/**
			 * @brief Decodes @p T from the start of @p data.
			 * @param[in] data Input span; may be longer than one value.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static Expected<T, DeserializeError> Read(std::span<const std::byte> data) noexcept {
				static_assert(codec_always_false_v<T>,
					"Specialize StormByte::Detail::Codec<T> instead of Serializable<T>");
				(void)data;
				return Unexpected<DeserializeError>("Codec<T> is not specialized");
			}
		};

		/**
		 * @brief @ref StormByte::Detail::Codec specialization for `std::string`.
		 *
		 * Wire: `uint64` byte count (LE) + raw bytes. No encoding transform.
		 */
		template<>
		struct Codec<std::string> {
			/**
			 * @brief Serialized size of @p data.
			 * @param data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static STORMBYTE_PUBLIC ByteSize Size(const std::string& data) noexcept;

			/**
			 * @brief Encodes @p data.
			 * @param data Value to encode.
			 * @return Blob owned by Base.
			 */
			static STORMBYTE_PUBLIC Safe::Binary Write(const std::string& data) noexcept;

			/**
			 * @brief Decodes a string from the start of @p data.
			 * @param data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static STORMBYTE_PUBLIC Expected<std::string, DeserializeError> Read(std::span<const std::byte> data) noexcept;
		};

		/**
		 * @brief @ref StormByte::Detail::Codec specialization for `std::wstring`.
		 *
		 * Wire: `uint64` UTF-8 byte count (LE) + UTF-8 payload. Host `wchar_t`
		 * width (2 on Windows, 4 on POSIX) never appears on the wire.
		 */
		template<>
		struct Codec<std::wstring> {
			/**
			 * @brief Serialized size of @p data.
			 * @param data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static STORMBYTE_PUBLIC ByteSize Size(const std::wstring& data) noexcept;

			/**
			 * @brief Encodes @p data.
			 * @param data Value to encode.
			 * @return Blob owned by Base.
			 */
			static STORMBYTE_PUBLIC Safe::Binary Write(const std::wstring& data) noexcept;

			/**
			 * @brief Decodes a wide string from the start of @p data.
			 * @param data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static STORMBYTE_PUBLIC Expected<std::wstring, DeserializeError> Read(std::span<const std::byte> data) noexcept;
		};

		/**
		 * @brief @ref StormByte::Detail::Codec specialization for `std::u16string`.
		 *
		 * Same UTF-8 wire as @ref StormByte::Detail::Codec<std::wstring>.
		 */
		template<>
		struct Codec<std::u16string> {
			/**
			 * @brief Serialized size of @p data.
			 * @param data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static STORMBYTE_PUBLIC ByteSize Size(const std::u16string& data) noexcept;

			/**
			 * @brief Encodes @p data.
			 * @param data Value to encode.
			 * @return Blob owned by Base.
			 */
			static STORMBYTE_PUBLIC Safe::Binary Write(const std::u16string& data) noexcept;

			/**
			 * @brief Decodes a UTF-16 string from the start of @p data.
			 * @param data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static STORMBYTE_PUBLIC Expected<std::u16string, DeserializeError> Read(std::span<const std::byte> data) noexcept;
		};

		/**
		 * @brief @ref StormByte::Detail::Codec specialization for `std::u32string`.
		 *
		 * Same UTF-8 wire as @ref StormByte::Detail::Codec<std::wstring>.
		 */
		template<>
		struct Codec<std::u32string> {
			/**
			 * @brief Serialized size of @p data.
			 * @param data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static STORMBYTE_PUBLIC ByteSize Size(const std::u32string& data) noexcept;

			/**
			 * @brief Encodes @p data.
			 * @param data Value to encode.
			 * @return Blob owned by Base.
			 */
			static STORMBYTE_PUBLIC Safe::Binary Write(const std::u32string& data) noexcept;

			/**
			 * @brief Decodes a UTF-32 string from the start of @p data.
			 * @param data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static STORMBYTE_PUBLIC Expected<std::u32string, DeserializeError> Read(std::span<const std::byte> data) noexcept;
		};

		/**
		 * @brief @ref StormByte::Detail::Codec specialization for @ref StormByte::Safe::String.
		 *
		 * Same wire as `std::string`: `uint64` byte count (LE) + raw bytes.
		 * Embedded NUL bytes are preserved. A null @ref StormByte::Safe::String is written as an empty payload
		 * (`operator std::string`). Decode always yields a non-null buffer
		 * (`""` when the payload is empty).
		 */
		template<>
		struct Codec<Safe::String> {
			/**
			 * @brief Serialized size of @p data.
			 * @param data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static STORMBYTE_PUBLIC ByteSize Size(const Safe::String& data) noexcept;

			/**
			 * @brief Encodes @p data.
			 * @param data Value to encode.
			 * @return Blob owned by Base.
			 */
			static STORMBYTE_PUBLIC Safe::Binary Write(const Safe::String& data) noexcept;

			/**
			 * @brief Decodes a @ref StormByte::Safe::String from the start of @p data.
			 * @param data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static STORMBYTE_PUBLIC Expected<Safe::String, DeserializeError> Read(std::span<const std::byte> data) noexcept;
		};

		/**
		 * @brief @ref StormByte::Detail::Codec specialization for @ref StormByte::Safe::WString.
		 *
		 * Same UTF-8 wire as `std::wstring`, preserving embedded NUL characters.
		 * A null @ref StormByte::Safe::WString
		 * is written as an empty payload. Decode always yields a non-null
		 * buffer (`L""` when the payload is empty).
		 */
		template<>
		struct Codec<Safe::WString> {
			/**
			 * @brief Serialized size of @p data.
			 * @param data Value to measure.
			 * @return Size in bytes as @ref ByteSize.
			 */
			static STORMBYTE_PUBLIC ByteSize Size(const Safe::WString& data) noexcept;

			/**
			 * @brief Encodes @p data.
			 * @param data Value to encode.
			 * @return Blob owned by Base.
			 */
			static STORMBYTE_PUBLIC Safe::Binary Write(const Safe::WString& data) noexcept;

			/**
			 * @brief Decodes a @ref StormByte::Safe::WString from the start of @p data.
			 * @param data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static STORMBYTE_PUBLIC Expected<Safe::WString, DeserializeError> Read(std::span<const std::byte> data) noexcept;
		};
	}

	/**
	 * @class Serializable
	 * @brief Encodes and decodes one value of type @p T.
	 * @tparam T Type to serialize. Dispatch uses the undecayed @p T.
	 *
	 * Dispatch order is intentional: optional / pair / container run
	 * *before* trivially-copyable. `std::optional<U>` with trivial `U` may
	 * report `is_trivially_copyable` and copying its object representation
	 * includes padding (observed on MSVC).
	 *
	 * Custom types: specialize @ref StormByte::Detail::Codec, then this class routes
	 * them through the “complex” path automatically.
	 *
	 * The blob is a @ref StormByte::Safe::Binary. The wire layout is unchanged.
	 *
	 * @note `Type::String` (`string` / `wstring` / `u16string` / `u32string`)
	 *       is excluded from @ref StormByte::Type::Container so those types hit Codec
	 *       instead of being encoded as a sequence of code units.
	 *       @ref StormByte::Safe::String and @ref StormByte::Safe::WString are not containers;
	 *       they hit @ref StormByte::Detail::Codec from the fallback in
	 *       `serializable.txx`.
	 */
	template<typename T>
	class Serializable {
		using DecayedT = std::remove_cvref_t<T>;	///< Value type stored by this wrapper.

		public:
			/**
			 * @brief Binds @p data for a later @ref StormByte::Serializable::Serialize.
			 * @param[in] data Value to encode. Must outlive this object.
			 */
			Serializable(const DecayedT& data) noexcept : m_data(data) {}

			/**
			 * @brief Copy is deleted: the wrapper only holds a reference.
			 */
			Serializable(const Serializable& other) noexcept = delete;

			/**
			 * @brief Move is deleted: the wrapper only holds a reference.
			 */
			Serializable(Serializable&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Serializable() noexcept = default;

			/**
			 * @brief Copy assignment is deleted.
			 */
			Serializable& operator=(const Serializable& other) noexcept = delete;

			/**
			 * @brief Move assignment is deleted.
			 */
			Serializable& operator=(Serializable&& other) noexcept = delete;

			/**
			 * @brief Encodes the bound value to a little-endian blob.
			 * @return Blob for this one value, owned by Base. No outer framing.
			 */
			Safe::Binary Serialize() const noexcept;

			/**
			 * @brief Decodes one @p T from the start of @p data.
			 * @param[in] data Input span; may be longer than one value.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static Expected<T, DeserializeError> Deserialize(std::span<const std::byte> data) noexcept;

			/**
			 * @brief Decodes one @p T from a @ref StormByte::Safe::Binary.
			 * @param[in] data Input blob.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			static Expected<T, DeserializeError> Deserialize(const Safe::Binary& data) noexcept;

			/**
			 * @brief Serialized size of @p data.
			 * @param[in] data Value to measure.
			 * @return @ref ByteSize of @ref StormByte::Serializable::Serialize for the same value.
			 */
			static ByteSize Size(const DecayedT& data) noexcept;

		private:
			const DecayedT& m_data;	///< Referenced value. Not owned.

			/**
			 * @brief Encodes a trivially copyable value.
			 *
			 * On a big-endian host the object representation is byte-swapped
			 * before the copy. `bool` is one byte and is not swapped.
			 *
			 * @tparam U Defaulted to @p T; keeps this a member *template* so
			 *           explicit instantiation of `Serializable<T>` does not
			 *           try to instantiate it for a `T` that fails the
			 *           `requires` clause (observed with clang-cl/MSVC ABI).
			 * @return Blob of `sizeof(T)` bytes owned by Base.
			 */
			template<typename U = T>
			Safe::Binary SerializeTrivial() const noexcept
			requires Type::TriviallyCopyable<U>;

			/**
			 * @brief Encodes a container: `uint64` count (LE) then each element.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @return Blob owned by Base.
			 */
			template<typename U = T>
			Safe::Binary SerializeContainer() const noexcept
			requires Type::Container<U>;

			/**
			 * @brief Encodes a pair: first, then second. No separator.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @return Blob owned by Base.
			 */
			template<typename U = T>
			Safe::Binary SerializePair() const noexcept
			requires Type::Pair<U>;

			/**
			 * @brief Encodes an optional: `bool has_value`, then the value if set.
			 *
			 * Never copies the `optional` object representation (padding).
			 *
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @return Blob owned by Base.
			 */
			template<typename U = T>
			Safe::Binary SerializeOptional() const noexcept
			requires Type::Optional<U>;

			/**
			 * @brief Encodes a FIFO: `uint64` count (LE), then elements in pop order.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @return Blob owned by Base.
			 */
		template<typename U = T>
		Safe::Binary SerializeQueue() const noexcept
			requires Type::Queue<U>;

			/**
			 * @brief Serialized size of a container.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Container to measure.
			 * @return `8` plus the sum of element sizes, as @ref ByteSize.
			 */
			template<typename U = T>
			static ByteSize SizeContainer(const DecayedT& data) noexcept
			requires Type::Container<U>;

			/**
			 * @brief Serialized size of a pair.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Pair to measure.
			 * @return Sum of member sizes as @ref ByteSize.
			 */
			template<typename U = T>
			static ByteSize SizePair(const DecayedT& data) noexcept
			requires Type::Pair<U>;

			/**
			 * @brief Serialized size of an optional.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Optional to measure.
			 * @return `sizeof(bool)` plus the value size when engaged, as @ref ByteSize.
			 */
			template<typename U = T>
			static ByteSize SizeOptional(const DecayedT& data) noexcept
			requires Type::Optional<U>;

			/**
			 * @brief Serialized size of a FIFO.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Queue to measure.
			 * @return `8` plus the sum of element sizes, as @ref ByteSize.
			 */
		template<typename U = T>
		static ByteSize SizeQueue(const DecayedT& data) noexcept
			requires Type::Queue<U>;

			/**
			 * @brief Decodes a trivially copyable value.
			 *
			 * `bool` only accepts the bytes `0` and `1`. Any other value is
			 * rejected (loading it into a `bool` is undefined).
			 *
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Input span.
			 * @return Value, or @ref StormByte::DeserializeError.
			 */
			template<typename U = T>
			static Expected<T, DeserializeError> DeserializeTrivial(std::span<const std::byte> data) noexcept
			requires Type::TriviallyCopyable<U>;

			/**
			 * @brief Decodes a container: count, then that many elements.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Input span.
			 * @return Container, or @ref StormByte::DeserializeError.
			 */
			template<typename U = T>
			static Expected<T, DeserializeError> DeserializeContainer(std::span<const std::byte> data) noexcept
			requires Type::Container<U>;

			/**
			 * @brief Decodes a pair: first, then second.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Input span.
			 * @return Pair, or @ref StormByte::DeserializeError.
			 */
			template<typename U = T>
			static Expected<T, DeserializeError> DeserializePair(std::span<const std::byte> data) noexcept
			requires Type::Pair<U>;

			/**
			 * @brief Decodes an optional: `bool`, then the value if set.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Input span.
			 * @return Optional, or @ref StormByte::DeserializeError.
			 */
			template<typename U = T>
			static Expected<T, DeserializeError> DeserializeOptional(std::span<const std::byte> data) noexcept
			requires Type::Optional<U>;

			/**
			 * @brief Decodes a FIFO from a count and ordered element sequence.
			 * @tparam U Defaulted to @p T; see @ref StormByte::Serializable::SerializeTrivial.
			 * @param[in] data Input span.
			 * @return Queue, or @ref StormByte::DeserializeError.
			 */
		template<typename U = T>
		static Expected<T, DeserializeError> DeserializeQueue(std::span<const std::byte> data) noexcept
			requires Type::Queue<U>;
	};

	// Explicit-instantiation declarations: suppress implicit instantiation
	// of Serializable<T> for these T in every consumer TU. Definitions are
	// in serializable.cxx. STORMBYTE_PUBLIC here is the export/import of
	// those instantiations; the class template itself has no visibility
	// so other modules can instantiate Serializable<TheirType> in their DLL.
	//
	// This block MUST come before `#include <StormByte/serializable.txx>`:
	// those bodies name Serializable<bool> and Serializable<std::uint64_t>
	// as non-dependent types. If the .txx is parsed first, those two are
	// instantiated in this header before the extern declarations.
	/// @cond
	extern template class STORMBYTE_PUBLIC Serializable<bool>;
	extern template class STORMBYTE_PUBLIC Serializable<char>;
	extern template class STORMBYTE_PUBLIC Serializable<signed char>;
	extern template class STORMBYTE_PUBLIC Serializable<unsigned char>;
	extern template class STORMBYTE_PUBLIC Serializable<wchar_t>;
	extern template class STORMBYTE_PUBLIC Serializable<char8_t>;
	extern template class STORMBYTE_PUBLIC Serializable<char16_t>;
	extern template class STORMBYTE_PUBLIC Serializable<char32_t>;
	extern template class STORMBYTE_PUBLIC Serializable<short>;
	extern template class STORMBYTE_PUBLIC Serializable<unsigned short>;
	extern template class STORMBYTE_PUBLIC Serializable<int>;
	extern template class STORMBYTE_PUBLIC Serializable<unsigned int>;
	extern template class STORMBYTE_PUBLIC Serializable<long>;
	extern template class STORMBYTE_PUBLIC Serializable<unsigned long>;
	extern template class STORMBYTE_PUBLIC Serializable<long long>;
	extern template class STORMBYTE_PUBLIC Serializable<unsigned long long>;
	extern template class STORMBYTE_PUBLIC Serializable<float>;
	extern template class STORMBYTE_PUBLIC Serializable<double>;
	extern template class STORMBYTE_PUBLIC Serializable<long double>;
	extern template class STORMBYTE_PUBLIC Serializable<std::string>;
	extern template class STORMBYTE_PUBLIC Serializable<std::wstring>;
	extern template class STORMBYTE_PUBLIC Serializable<std::u16string>;
	extern template class STORMBYTE_PUBLIC Serializable<std::u32string>;
	extern template class STORMBYTE_PUBLIC Serializable<Safe::String>;
	extern template class STORMBYTE_PUBLIC Serializable<Safe::WString>;
	extern template class STORMBYTE_PUBLIC Serializable<Safe::Binary>;
	/// @endcond
}

#include <StormByte/serializable.txx>
