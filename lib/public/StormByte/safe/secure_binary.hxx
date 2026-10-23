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

#include <StormByte/safe/binary.hxx>

#include <cstddef>
#include <initializer_list>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

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
		 * @class SecureBinary
		 * @brief @ref Binary for secrets: every byte it drops or releases is zeroed first.
		 *
		 * Same API as @ref Binary. Erased, popped and truncated bytes, previous contents on assign, the old block when the storage grows or shrinks, and the whole block in the destructor are zeroed with writes the compiler cannot drop. Equality runs in constant time for equal sizes.
		 *
		 * Each block it owns is also locked in memory (`mlock`, `VirtualLock` on Windows) so it is not written to swap. This is best effort: when the process limit is reached (`RLIMIT_MEMLOCK`, the Windows minimum working set) the block stays unlocked and everything else still works. Pages shared with other blocks are counted and stay locked until the last of them goes.
		 *
		 * Not covered: a move, swap or `append(&&)` with a plain @ref Binary hands the block over as is. The secret is then no longer wiped or locked by this type. Copies made through @ref span() or a `const Binary&` are the caller's responsibility.
		 *
		 * There is no conversion to `std::vector`, no `HexDump`, and no @ref StormByte::Serializable support: each would copy the secret into memory that is never wiped.
		 */
		class STORMBYTE_PUBLIC SecureBinary final: public Binary {
			public:
				/**
				 * @brief Construct an empty sequence.
				 */
				SecureBinary() noexcept;

				/**
				 * @brief Construct @p count bytes filled with @p value.
				 * @param count Element count.
				 * @param value Fill byte.
				 */
				SecureBinary(const ByteSize& count, std::byte value);

				/**
				 * @brief Construct @p count zeroed bytes.
				 * @param count Element count.
				 */
				explicit SecureBinary(const ByteSize& count);

				/**
				 * @brief Copy bytes from a contiguous span.
				 * @param bytes Source view. Not owned and not wiped.
				 */
				explicit SecureBinary(std::span<const std::byte> bytes);

				/**
				 * @brief Copy @p count bytes starting at @p bytes.
				 * @param bytes Source pointer. May be null when @p count is zero.
				 * @param count Byte count.
				 * @throws OutOfBoundsError @p bytes is null and @p count is not zero.
				 */
				SecureBinary(const std::byte* bytes, const ByteSize& count);

				/**
				 * @brief Copy from an initializer list.
				 * @param list Source bytes.
				 */
				SecureBinary(std::initializer_list<std::byte> list);

				/**
				 * @brief Copy character bytes from a string view. No trailing NUL.
				 * @param text Source characters.
				 */
				explicit SecureBinary(std::string_view text);

				/**
				 * @brief Copy character bytes from a C string. No trailing NUL.
				 * @param text Source. A null pointer yields an empty sequence.
				 */
				explicit SecureBinary(const char* text);

				/**
				 * @brief Copy bytes from a caller-owned vector. The vector is unchanged.
				 * @param bytes Source.
				 */
				STORMBYTE_FORCE_INLINE explicit SecureBinary(const std::vector<std::byte>& bytes): SecureBinary(std::span<const std::byte>(bytes.data(), bytes.size())) {}

				/**
				 * @brief Copy bytes from a caller-owned vector, then zero and release it.
				 * @param bytes Source. Its bytes are zeroed before it is cleared. Reallocations the vector made earlier are not covered.
				 */
				STORMBYTE_FORCE_INLINE explicit SecureBinary(std::vector<std::byte>&& bytes): SecureBinary(std::span<const std::byte>(bytes.data(), bytes.size())) {
					Wipe(std::span<std::byte>(bytes.data(), bytes.capacity()));
					bytes.clear();
					bytes.shrink_to_fit();
				}

				/**
				 * @brief Copy from an input range of byte-convertible values.
				 * @tparam R Range type satisfying @ref Type::ByteInputRange.
				 * @param range Source range.
				 */
				template<Type::ByteInputRange R>
				explicit SecureBinary(const R& range);

				/**
				 * @brief Consume an rvalue range. Takes the block when @p R is an rvalue @ref Binary or derived.
				 * @tparam R Range type satisfying @ref Type::ByteInputRange.
				 * @param range Source range.
				 */
				template<Type::ByteInputRange R>
				explicit SecureBinary(R&& range);

				/**
				 * @brief Copy construct. The copy is allocated once.
				 * @param other Source sequence.
				 */
				SecureBinary(const SecureBinary& other);

				/**
				 * @brief Move construct. @p other is left empty.
				 * @param other Source sequence.
				 */
				SecureBinary(SecureBinary&& other) noexcept;

				/**
				 * @brief Zero the whole block, then release it.
				 */
				~SecureBinary() noexcept override;

				/**
				 * @brief Copy assign. Previous contents are zeroed.
				 * @param other Source sequence.
				 * @return This sequence.
				 */
				SecureBinary& operator=(const SecureBinary& other);

				/**
				 * @brief Move assign. The previous block is zeroed. @p other is left empty.
				 * @param other Source sequence.
				 * @return This sequence.
				 */
				SecureBinary& operator=(SecureBinary&& other) noexcept;

				/**
				 * @brief Replace contents with an initializer list. Previous contents are zeroed.
				 * @param list Source bytes.
				 * @return This sequence.
				 */
				SecureBinary& operator=(std::initializer_list<std::byte> list);

				/**
				 * @brief Not available: the vector would hold the secret in memory that is never wiped. Use @ref span().
				 */
				explicit operator std::vector<std::byte>() const& = delete;

				/**
				 * @brief Not available: the vector would hold the secret in memory that is never wiped. Use @ref span().
				 */
				explicit operator std::vector<std::byte>() && = delete;

				/**
				 * @brief Not available: the dump would hold the secret in memory that is never wiped.
				 */
				String HexDump() const = delete;

				/**
				 * @brief Not available: the dump would hold the secret in memory that is never wiped.
				 * @param columns Unused.
				 */
				String HexDump(Size columns) const = delete;

			protected:
				/**
				 * @brief Zero @p bytes.
				 * @param bytes Bytes being dropped.
				 */
				void Discard(std::span<std::byte> bytes) noexcept override;

				/**
				 * @brief Equality with this object never stops at the first difference.
				 * @return Always true.
				 */
				bool ConstantTimeCompare() const noexcept override;

				/**
				 * @brief Best effort: lock the pages of @p block in memory so they are not written to swap.
				 * @param block Block now owned by this object.
				 */
				void Acquire(std::span<std::byte> block) noexcept override;

				/**
				 * @brief Unlock the pages of @p block once no other block on the same pages is locked.
				 * @param block Block no longer owned by this object.
				 */
				void Release(std::span<std::byte> block) noexcept override;
		};

		template<Type::ByteInputRange R>
		SecureBinary::SecureBinary(const R& range): SecureBinary() {
			if constexpr (requires { std::size(range); })
				reserve(ByteSize{std::size(range)});
			for (auto&& byte : range)
				push_back(static_cast<std::byte>(byte));
		}

		template<Type::ByteInputRange R>
		SecureBinary::SecureBinary(R&& range): SecureBinary() {
			using U = std::remove_cvref_t<R>;
			if constexpr (Type::DerivedFrom<U, Binary> && !Type::LvalueReference<R>) {
				Binary::operator=(std::move(static_cast<Binary&>(range)));
			} else {
				if constexpr (requires { std::size(range); })
					reserve(ByteSize{std::size(range)});
				for (auto&& byte : range)
					push_back(static_cast<std::byte>(byte));
			}
		}
	}
}
