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


#include <StormByte/byte_size.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/secure_binary.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef LINUX
	#include <sys/resource.h>
#endif

using namespace StormByte;

namespace {
	// Constructed before the page registry, populated in main, released during static teardown.
	Safe::SecureBinary StaticSecret;

	class LateSecureUse {
		public:
			LateSecureUse() = default;
			LateSecureUse(const LateSecureUse&) = delete;
			LateSecureUse(LateSecureUse&&) = delete;

			~LateSecureUse() noexcept {
				StaticSecret = Safe::SecureBinary{};
				const Safe::SecureBinary late(ByteSize{8192}, std::byte{0x5A});
				if (late.size() != ByteSize{8192} || late.front() != std::byte{0x5A})
					std::abort();
			}

			LateSecureUse& operator=(const LateSecureUse&) = delete;
			LateSecureUse& operator=(LateSecureUse&&) = delete;
	};

	LateSecureUse LateSecret;

	// Records every Discard call, then wipes like SecureBinary does.
	class SpyBinary final: public Safe::Binary {
		public:
			using Safe::Binary::Binary;
			using Safe::Binary::operator=;

			std::size_t calls = 0;
			std::size_t bytes = 0;
			bool wiped = true;
			std::vector<std::byte> last;

			std::span<std::byte> Block() noexcept {
				return Storage();
			}

			void Reset() noexcept {
				calls = 0;
				bytes = 0;
				last.clear();
			}

		protected:
			void Discard(std::span<std::byte> range) noexcept override {
				++calls;
				bytes += range.size();
				last.assign(range.begin(), range.end());
				Wipe(range);
				for (std::byte value : range)
					wiped = wiped && value == std::byte{0};
			}
	};

	// Counts owners per block: +1 on Acquire, -1 on Release. Every block must end at 0.
	std::map<const std::byte*, long>& Ledger() {
		static std::map<const std::byte*, long> ledger;
		return ledger;
	}

	bool LedgerBalanced() {
		return std::ranges::all_of(Ledger(), [](const auto& entry) { return entry.second == 0; });
	}

	long Owners(const std::byte* block) {
		const auto found = Ledger().find(block);
		return found == Ledger().end() ? 0 : found->second;
	}

	// Follows the SecureBinary pattern: fills and takes blocks in bodies, releases in the destructor.
	class OwnerSpy final: public Safe::Binary {
		public:
			using Safe::Binary::operator=;

			OwnerSpy() noexcept = default;

			OwnerSpy(OwnerSpy&& other) noexcept: Safe::Binary() {
				Safe::Binary::operator=(std::move(other));
			}

			OwnerSpy& operator=(OwnerSpy&& other) noexcept {
				Safe::Binary::operator=(std::move(other));
				return *this;
			}

			~OwnerSpy() noexcept override {
				if (capacity() != 0)
					--Ledger()[Storage().data()];
			}

		protected:
			void Acquire(std::span<std::byte> block) noexcept override {
				++Ledger()[block.data()];
			}

			void Release(std::span<std::byte> block) noexcept override {
				--Ledger()[block.data()];
			}
	};

	Safe::SecureBinary Secret() {
		return Safe::SecureBinary{std::byte{0xA1}, std::byte{0xB2}, std::byte{0xC3}, std::byte{0xD4}, std::byte{0xE5}};
	}

	template<typename B>
	concept HasHexDump = requires(const B& value) { value.HexDump(); };

	template<typename B>
	concept HasHexDumpColumns = requires(const B& value) { value.HexDump(Size{8}); };

	bool AllZero(const std::byte* from, std::size_t count) {
		for (std::size_t index = 0; index < count; ++index)
			if (from[index] != std::byte{0})
				return false;
		return true;
	}
}

// -------------------
// Shared contract (Binary and SecureBinary)
// -------------------

template<typename B>
Safe::Binary Plain(std::initializer_list<std::byte> list) {
	return Safe::Binary(list);
}

template<typename B>
int test_shared_algorithms() {
	B bytes{std::byte{3}, std::byte{1}, std::byte{2}, std::byte{2}, std::byte{0}};
	const ByteSize owned = bytes.size();
	std::sort(bytes.begin(), bytes.end());
	ASSERT_TRUE((bytes == Plain<B>({std::byte{0}, std::byte{1}, std::byte{2}, std::byte{2}, std::byte{3}})));
	std::ranges::sort(bytes);
	ASSERT_TRUE(std::ranges::is_sorted(bytes));
	ASSERT_EQUAL(static_cast<std::ptrdiff_t>(2), std::count(bytes.begin(), bytes.end(), std::byte{2}));
	ASSERT_TRUE((std::ranges::binary_search(bytes, std::byte{3})));
	std::reverse(bytes.begin(), bytes.end());
	ASSERT_EQUAL(std::byte{3}, bytes.front());
	std::rotate(bytes.begin(), bytes.begin() + 1, bytes.end());
	ASSERT_EQUAL(std::byte{3}, bytes.back());
	std::replace(bytes.begin(), bytes.end(), std::byte{0}, std::byte{9});
	ASSERT_TRUE((std::ranges::find(bytes, std::byte{9}) != bytes.end()));
	std::reverse(bytes.rbegin(), bytes.rend());
	ASSERT_EQUAL(owned, bytes.size());
	B other(bytes);
	ASSERT_TRUE(std::ranges::equal(bytes, other));
	RETURN_TEST(0);
}

template<typename B>
int test_shared_construct() {
	B empty;
	ASSERT_TRUE(empty.empty());
	ASSERT_NULL(empty.data());
	ASSERT_TRUE(empty.begin() == empty.end());
	ASSERT_TRUE(empty.crbegin() == empty.crend());
	ASSERT_TRUE((empty.max_size() > ByteSize{0}));
	B filled(ByteSize{3}, std::byte{7});
	ASSERT_TRUE((filled == Plain<B>({std::byte{7}, std::byte{7}, std::byte{7}})));
	B zeroes(ByteSize{2});
	ASSERT_TRUE((zeroes == Plain<B>({std::byte{0}, std::byte{0}})));
	const std::byte raw[] = {std::byte{1}, std::byte{2}};
	B from_span(std::span<const std::byte>(raw, 2));
	B from_pointer(raw, ByteSize{2});
	ASSERT_TRUE(from_span == from_pointer);
	B from_zero(static_cast<const std::byte*>(nullptr), ByteSize{0});
	ASSERT_TRUE(from_zero.empty());
	ASSERT_THROWS(B(static_cast<const std::byte*>(nullptr), ByteSize{1}), Safe::OutOfBoundsError);
	B from_text(std::string_view("ab"));
	ASSERT_TRUE((from_text == Plain<B>({std::byte{'a'}, std::byte{'b'}})));
	B from_cstr("xy");
	ASSERT_EQUAL(ByteSize{2}, from_cstr.size());
	ASSERT_TRUE(B(static_cast<const char*>(nullptr)).empty());
	std::vector<std::byte> caller{std::byte{8}};
	B from_std(caller);
	ASSERT_EQUAL(static_cast<std::size_t>(1), caller.size());
	B from_moved(std::move(caller));
	ASSERT_TRUE(caller.empty());
	ASSERT_TRUE(from_std == from_moved);
	const std::list<int> range{1, 2, 3};
	B from_range(range);
	ASSERT_TRUE((from_range == Plain<B>({std::byte{1}, std::byte{2}, std::byte{3}})));
	B copy(from_span);
	ASSERT_TRUE(copy == from_span);
	copy = from_text;
	ASSERT_TRUE(copy == from_text);
	const B& self_copy = copy;
	copy = self_copy;
	ASSERT_TRUE(copy == from_text);
	B moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	moved = std::move(from_span);
	ASSERT_TRUE(from_span.empty());
	ASSERT_TRUE((moved == Plain<B>({std::byte{1}, std::byte{2}})));
	B& self_move = moved;
	moved = std::move(self_move);
	ASSERT_EQUAL(ByteSize{2}, moved.size());
	moved = {std::byte{1}};
	const std::span<const std::byte> view = moved;
	ASSERT_EQUAL(static_cast<std::size_t>(1), view.size());
	ASSERT_TRUE(moved.capacity() >= moved.size());
	RETURN_TEST(0);
}

template<typename B>
int test_shared_mutate() {
	B bytes;
	const std::byte raw[] = {std::byte{1}, std::byte{2}};
	bytes.append(std::span<const std::byte>(raw, 2));
	bytes.append(raw, ByteSize{1});
	bytes.append(static_cast<const std::byte*>(nullptr), ByteSize{0});
	B extra{std::byte{9}};
	bytes.append(extra);
	B taken{std::byte{8}};
	bytes.append(std::move(taken));
	ASSERT_TRUE(taken.empty());
	bytes += std::span<const std::byte>(raw, 1);
	bytes += B{std::byte{3}};
	B moved_add{std::byte{4}};
	bytes += std::move(moved_add);
	bytes.push_back(std::byte{5});
	ASSERT_EQUAL(std::byte{6}, bytes.emplace_back(6));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{1}, std::byte{9}, std::byte{8}, std::byte{1}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}})));
	bytes.pop_back();
	auto inserted = bytes.insert(bytes.begin(), std::byte{0});
	ASSERT_EQUAL(std::byte{0}, *inserted);
	inserted = bytes.insert(bytes.begin() + 2, ByteSize{2}, std::byte{7});
	ASSERT_EQUAL(bytes.begin() + 2, inserted);
	inserted = bytes.insert(bytes.end(), {std::byte{0xA}, std::byte{0xB}});
	ASSERT_EQUAL(std::byte{0xA}, *inserted);
	ASSERT_TRUE((bytes.insert(bytes.begin() + 1, ByteSize{0}, std::byte{1}) == bytes.begin() + 1));
	ASSERT_TRUE((bytes.insert(bytes.begin() + 1, std::span<const std::byte>{}) == bytes.begin() + 1));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{0}, std::byte{1}, std::byte{7}, std::byte{7}, std::byte{2}, std::byte{1}, std::byte{9}, std::byte{8}, std::byte{1}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{0xA}, std::byte{0xB}})));
	auto next = bytes.erase(bytes.begin());
	ASSERT_EQUAL(std::byte{1}, *next);
	next = bytes.erase(bytes.begin() + 1, bytes.begin() + 3);
	ASSERT_EQUAL(std::byte{2}, *next);
	ASSERT_TRUE(bytes.erase(bytes.begin(), bytes.begin()) == bytes.begin());
	bytes.erase(bytes.end() - 2, bytes.end());
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{1}, std::byte{9}, std::byte{8}, std::byte{1}, std::byte{3}, std::byte{4}, std::byte{5}})));
	bytes.assign(ByteSize{2}, std::byte{7});
	ASSERT_TRUE((bytes == Plain<B>({std::byte{7}, std::byte{7}})));
	bytes.assign(std::span<const std::byte>(raw, 2));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}})));
	bytes.assign({std::byte{3}, std::byte{1}});
	ASSERT_TRUE((bytes == Plain<B>({std::byte{3}, std::byte{1}})));
	bytes.assign(raw, raw + 2);
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}})));
	bytes.resize(ByteSize{4});
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{0}, std::byte{0}})));
	bytes.resize(ByteSize{3}, std::byte{9});
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{0}})));
	bytes.resize(ByteSize{5}, std::byte{9});
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{0}, std::byte{9}, std::byte{9}})));
	bytes.reserve(ByteSize{32});
	ASSERT_TRUE((bytes.capacity() >= ByteSize{32}));
	bytes.reserve(ByteSize{1});
	ASSERT_TRUE((bytes.capacity() >= ByteSize{32}));
	bytes.shrink_to_fit();
	ASSERT_EQUAL(bytes.size(), bytes.capacity());
	ASSERT_EQUAL(ByteSize{5}, bytes.size());
	const B& read = bytes;
	ASSERT_EQUAL(bytes.front(), read.front());
	ASSERT_EQUAL(bytes.back(), read.back());
	ASSERT_EQUAL(bytes[ByteSize{1}], read.at(ByteSize{1}));
	ASSERT_THROWS(bytes.at(bytes.size()), Safe::OutOfBoundsError);
	ASSERT_THROWS(bytes.append(static_cast<const std::byte*>(nullptr), ByteSize{1}), Safe::OutOfBoundsError);
	B other{std::byte{1}};
	bytes.swap(other);
	ASSERT_EQUAL(ByteSize{1}, bytes.size());
	swap(bytes, other);
	ASSERT_EQUAL(ByteSize{5}, bytes.size());
	bytes.clear();
	ASSERT_TRUE(bytes.empty());
	ASSERT_TRUE((bytes.capacity() > ByteSize{0}));
	bytes.shrink_to_fit();
	ASSERT_EQUAL(ByteSize{0}, bytes.capacity());
	ASSERT_TRUE((B{std::byte{1}, std::byte{2}} < B{std::byte{1}, std::byte{3}}));
	RETURN_TEST(0);
}

template<typename B>
int test_shared_self_aliasing() {
	B bytes{std::byte{1}, std::byte{2}, std::byte{3}};
	bytes.shrink_to_fit();
	bytes.append(bytes);
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{3}, std::byte{1}, std::byte{2}, std::byte{3}})));
	bytes.shrink_to_fit();
	bytes.append(std::span<const std::byte>(bytes.data() + 4, 2));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{3}, std::byte{1}, std::byte{2}, std::byte{3}, std::byte{2}, std::byte{3}})));
	bytes.assign({std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
	bytes.shrink_to_fit();
	// Source before, after and around the insertion point, each forcing a reallocation.
	bytes.insert(bytes.cbegin() + 3, std::span<const std::byte>(bytes.data(), 2));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{3}, std::byte{1}, std::byte{2}, std::byte{4}})));
	bytes.assign({std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
	bytes.shrink_to_fit();
	bytes.insert(bytes.cbegin(), std::span<const std::byte>(bytes.data() + 2, 2));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{3}, std::byte{4}, std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}})));
	bytes.assign({std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
	bytes.shrink_to_fit();
	bytes.insert(bytes.cbegin() + 2, std::span<const std::byte>(bytes.data() + 1, 2));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{1}, std::byte{2}, std::byte{2}, std::byte{3}, std::byte{3}, std::byte{4}})));
	bytes.assign(std::span<const std::byte>(bytes.data() + 3, 3));
	ASSERT_TRUE((bytes == Plain<B>({std::byte{3}, std::byte{3}, std::byte{4}})));
	bytes.assign(bytes.span());
	ASSERT_EQUAL(ByteSize{3}, bytes.size());
	bytes.append(std::move(bytes));
	ASSERT_EQUAL(ByteSize{3}, bytes.size());
	RETURN_TEST(0);
}

template<typename B>
int test_shared_insert_iterators() {
	B bytes;
	std::istringstream stream("abc");
	auto at = bytes.insert(bytes.cbegin(), std::istream_iterator<char>(stream), std::istream_iterator<char>());
	ASSERT_TRUE(at == bytes.begin());
	ASSERT_TRUE(bytes == Safe::Binary(std::string_view("abc")));
	const std::list<int> forward{9, 8};
	at = bytes.insert(bytes.cbegin() + 1, forward.begin(), forward.end());
	ASSERT_EQUAL(std::byte{9}, *at);
	ASSERT_TRUE((bytes == Plain<B>({std::byte{'a'}, std::byte{9}, std::byte{8}, std::byte{'b'}, std::byte{'c'}})));
	ASSERT_TRUE(bytes.insert(bytes.cbegin() + 2, forward.end(), forward.end()) == bytes.begin() + 2);
	bytes.assign(forward.begin(), forward.end());
	ASSERT_TRUE((bytes == Plain<B>({std::byte{9}, std::byte{8}})));
	RETURN_TEST(0);
}

template<typename B>
int test_shared_comparisons() {
	const B left{std::byte{1}, std::byte{2}};
	const B right{std::byte{1}, std::byte{3}};
	const B shorter{std::byte{1}};
	ASSERT_TRUE(left == left);
	ASSERT_TRUE(left != right);
	ASSERT_TRUE(left != shorter);
	ASSERT_TRUE(left < right);
	ASSERT_TRUE((left <=> right) == std::strong_ordering::less);
	ASSERT_TRUE((left <=> shorter) == std::strong_ordering::greater);
	const std::span<const std::byte> view = left.span();
	ASSERT_TRUE(left == view);
	ASSERT_TRUE(view == left);
	ASSERT_TRUE(left != right.span());
	ASSERT_TRUE(right.span() != left);
	ASSERT_TRUE((view <=> right) == std::strong_ordering::less);
	ASSERT_TRUE((B{} == B{}));
	ASSERT_TRUE((B{} == std::span<const std::byte>{}));
	RETURN_TEST(0);
}

// -------------------
// Discard hook
// -------------------

int test_hook_growth_discards_old_block() {
	SpyBinary spy;
	spy.push_back(std::byte{1});
	ASSERT_EQUAL(static_cast<std::size_t>(0), spy.calls);
	spy.push_back(std::byte{2});
	ASSERT_EQUAL(static_cast<std::size_t>(1), spy.calls);
	ASSERT_EQUAL(static_cast<std::size_t>(1), spy.last.size());
	ASSERT_EQUAL(std::byte{1}, spy.last[0]);
	spy.Reset();
	const ByteSize capacity = spy.capacity();
	spy.append(std::span<const std::byte>(spy.data(), static_cast<std::size_t>(spy.size())));
	spy.append(std::span<const std::byte>(spy.data(), static_cast<std::size_t>(spy.size())));
	ASSERT_TRUE(spy.capacity() > capacity);
	ASSERT_TRUE(spy.calls >= 1);
	ASSERT_TRUE((spy == Safe::Binary{std::byte{1}, std::byte{2}, std::byte{1}, std::byte{2}, std::byte{1}, std::byte{2}, std::byte{1}, std::byte{2}}));
	ASSERT_TRUE(spy.wiped);
	RETURN_TEST(0);
}

int test_hook_shrinking_operations_discard_dropped_bytes() {
	SpyBinary spy{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}};
	spy.Reset();
	spy.pop_back();
	ASSERT_EQUAL(static_cast<std::size_t>(1), spy.bytes);
	ASSERT_EQUAL(std::byte{6}, spy.last[0]);

	spy.Reset();
	spy.erase(spy.begin() + 1);
	ASSERT_EQUAL(static_cast<std::size_t>(1), spy.bytes);
	ASSERT_TRUE((spy == Safe::Binary{std::byte{1}, std::byte{3}, std::byte{4}, std::byte{5}}));

	spy.Reset();
	spy.resize(ByteSize{2});
	ASSERT_EQUAL(static_cast<std::size_t>(2), spy.bytes);
	ASSERT_EQUAL(std::byte{4}, spy.last[0]);
	ASSERT_EQUAL(std::byte{5}, spy.last[1]);

	spy.Reset();
	spy.assign({std::byte{9}});
	ASSERT_EQUAL(static_cast<std::size_t>(2), spy.bytes);

	spy.Reset();
	spy.clear();
	ASSERT_EQUAL(static_cast<std::size_t>(1), spy.bytes);
	ASSERT_EQUAL(std::byte{9}, spy.last[0]);
	ASSERT_TRUE(spy.wiped);
	RETURN_TEST(0);
}

int test_hook_reallocation_and_move_discard_whole_block() {
	SpyBinary spy{std::byte{1}, std::byte{2}};
	spy.reserve(ByteSize{32});
	spy.Reset();
	spy.shrink_to_fit();
	ASSERT_EQUAL(static_cast<std::size_t>(1), spy.calls);
	ASSERT_EQUAL(static_cast<std::size_t>(32), spy.bytes);
	ASSERT_EQUAL(ByteSize{2}, spy.capacity());

	spy.Reset();
	spy.reserve(ByteSize{8});
	ASSERT_EQUAL(static_cast<std::size_t>(2), spy.bytes);
	ASSERT_EQUAL(ByteSize{8}, spy.Block().size());

	spy.Reset();
	spy = Safe::Binary{std::byte{7}};
	ASSERT_EQUAL(static_cast<std::size_t>(8), spy.bytes);
	ASSERT_EQUAL(ByteSize{1}, spy.size());
	ASSERT_TRUE(spy.wiped);
	RETURN_TEST(0);
}

int test_hook_every_acquired_block_is_released_once() {
	{
		OwnerSpy spy;
		for (int value = 0; value < 100; ++value)
			spy.push_back(static_cast<std::byte>(value));
		ASSERT_EQUAL(1L, Owners(spy.data()));
		spy.shrink_to_fit();
		spy.reserve(ByteSize{512});
		ASSERT_EQUAL(1L, Owners(spy.data()));

		OwnerSpy moved(std::move(spy));
		ASSERT_EQUAL(1L, Owners(moved.data()));
		OwnerSpy assigned;
		assigned.push_back(std::byte{1});
		assigned = std::move(moved);
		ASSERT_EQUAL(1L, Owners(assigned.data()));

		OwnerSpy other;
		other.resize(ByteSize{3}, std::byte{2});
		assigned.swap(other);
		ASSERT_EQUAL(1L, Owners(assigned.data()));
		ASSERT_EQUAL(1L, Owners(other.data()));
		OwnerSpy& alias = other;
		other.swap(alias);
		ASSERT_EQUAL(1L, Owners(other.data()));

		Safe::Binary plain{std::byte{9}};
		assigned.swap(plain);
		ASSERT_EQUAL(0L, Owners(plain.data()));
		ASSERT_EQUAL(1L, Owners(assigned.data()));
		plain = std::move(other);
		ASSERT_EQUAL(0L, Owners(plain.data()));
		Safe::Binary constructed(std::move(assigned));
		ASSERT_EQUAL(0L, Owners(constructed.data()));
		assigned = std::move(constructed);
		ASSERT_EQUAL(1L, Owners(assigned.data()));
	}
	ASSERT_TRUE(LedgerBalanced());
	RETURN_TEST(0);
}

// -------------------
// Zeroing
// -------------------

int test_secure_slack_is_zero_after_shrinking() {
	Safe::SecureBinary secret = Secret();
	secret.reserve(ByteSize{16});
	const std::byte* block = secret.data();

	secret.pop_back();
	ASSERT_TRUE(AllZero(block + 4, 1));
	secret.erase(secret.begin() + 1, secret.begin() + 3);
	ASSERT_TRUE((secret == Safe::Binary{std::byte{0xA1}, std::byte{0xD4}}));
	ASSERT_TRUE(AllZero(block + 2, 2));
	secret.resize(ByteSize{1});
	ASSERT_TRUE(AllZero(block + 1, 1));
	secret.assign({std::byte{'k'}});
	ASSERT_EQUAL(block, secret.data());
	secret.clear();
	ASSERT_TRUE(AllZero(block, 1));
	RETURN_TEST(0);
}

int test_secure_assign_and_insert_keep_slack_clean() {
	Safe::SecureBinary secret = Secret();
	secret.reserve(ByteSize{16});
	const std::byte* block = secret.data();
	secret.insert(secret.cbegin() + 2, std::span<const std::byte>(secret.data(), 2));
	ASSERT_EQUAL(ByteSize{7}, secret.size());
	ASSERT_EQUAL(block, secret.data());
	secret.assign(std::span<const std::byte>(secret.data() + 5, 2));
	ASSERT_TRUE((secret == Safe::Binary{std::byte{0xD4}, std::byte{0xE5}}));
	ASSERT_TRUE(AllZero(block + 2, 5));
	RETURN_TEST(0);
}

int test_secure_vector_rvalue_is_wiped() {
	std::vector<std::byte> source{std::byte{1}, std::byte{2}, std::byte{3}};
	Safe::SecureBinary secret{std::move(source)};
	ASSERT_TRUE(source.empty());
	ASSERT_EQUAL(ByteSize{3}, secret.size());
	RETURN_TEST(0);
}

// -------------------
// Semantics
// -------------------

int test_secure_constructors_and_copies() {
	const Safe::SecureBinary secret = Secret();
	Safe::SecureBinary copy(secret);
	ASSERT_TRUE(copy == secret);
	ASSERT_EQUAL(copy.size(), copy.capacity());
	Safe::SecureBinary moved(std::move(copy));
	ASSERT_TRUE(copy.empty());
	ASSERT_TRUE(moved == secret);
	Safe::SecureBinary assigned;
	assigned = secret;
	ASSERT_TRUE(assigned == secret);
	assigned = {std::byte{1}};
	ASSERT_EQUAL(ByteSize{1}, assigned.size());
	ASSERT_TRUE(Safe::SecureBinary("abc") == Safe::Binary(std::string_view("abc")));
	ASSERT_TRUE(Safe::SecureBinary(static_cast<const char*>(nullptr)).empty());
	ASSERT_TRUE((Safe::SecureBinary(ByteSize{3}) == Safe::Binary(ByteSize{3})));
	ASSERT_TRUE((Safe::SecureBinary(ByteSize{2}, std::byte{7}) == Safe::Binary(ByteSize{2}, std::byte{7})));
	const std::list<int> range{1, 2, 3};
	ASSERT_TRUE(Safe::SecureBinary(range) == Safe::Binary(range));
	ASSERT_THROWS(Safe::SecureBinary(static_cast<const std::byte*>(nullptr), ByteSize{1}), Safe::OutOfBoundsError);
	RETURN_TEST(0);
}

int test_secure_equality_is_symmetric_through_base() {
	const Safe::SecureBinary secret = Secret();
	const Safe::Binary plain(secret.span());
	const Safe::Binary& base = secret;
	ASSERT_TRUE(secret == plain);
	ASSERT_TRUE(plain == secret);
	ASSERT_TRUE(base == plain);
	ASSERT_TRUE(secret == secret.span());
	Safe::SecureBinary other = Secret();
	other.back() = std::byte{0};
	ASSERT_TRUE(secret != other);
	ASSERT_TRUE(plain != other);
	other.pop_back();
	ASSERT_FALSE(secret == other);
	ASSERT_TRUE((secret <=> other) == std::strong_ordering::greater);
	RETURN_TEST(0);
}

int test_secure_cross_type_moves_hand_over_the_block() {
	Safe::SecureBinary secret = Secret();
	const std::byte* block = secret.data();
	Safe::Binary plain(std::move(secret));
	ASSERT_TRUE(secret.empty());
	ASSERT_EQUAL(block, plain.data());

	Safe::SecureBinary back(std::move(plain));
	ASSERT_TRUE(plain.empty());
	ASSERT_EQUAL(block, back.data());

	Safe::Binary other{std::byte{1}};
	back.swap(other);
	ASSERT_EQUAL(block, other.data());
	ASSERT_EQUAL(ByteSize{1}, back.size());
	RETURN_TEST(0);
}

int test_secure_insert_ranges_and_aliasing() {
	Safe::SecureBinary secret{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
	secret.insert(secret.cbegin() + 1, std::span<const std::byte>(secret.data() + 2, 2));
	ASSERT_TRUE((secret == Safe::Binary{std::byte{1}, std::byte{3}, std::byte{4}, std::byte{2}, std::byte{3}, std::byte{4}}));
	secret.insert(secret.cbegin() + 2, std::span<const std::byte>(secret.data() + 1, 3));
	ASSERT_TRUE((secret == Safe::Binary{std::byte{1}, std::byte{3}, std::byte{3}, std::byte{4}, std::byte{2}, std::byte{4}, std::byte{2}, std::byte{3}, std::byte{4}}));
	secret.append(secret);
	ASSERT_EQUAL(ByteSize{18}, secret.size());

	Safe::SecureBinary input;
	std::istringstream stream("abc");
	input.insert(input.cbegin(), std::istream_iterator<char>(stream), std::istream_iterator<char>());
	ASSERT_TRUE(input == Safe::Binary(std::string_view("abc")));
	const std::list<int> forward{9, 8};
	input.insert(input.cbegin() + 1, forward.begin(), forward.end());
	ASSERT_TRUE((input == Safe::Binary{std::byte{'a'}, std::byte{9}, std::byte{8}, std::byte{'b'}, std::byte{'c'}}));
	input.insert(input.cend(), ByteSize{2}, std::byte{0xFF});
	ASSERT_EQUAL(std::byte{0xFF}, input.back());
	RETURN_TEST(0);
}

#ifdef LINUX
namespace {
	// Locked memory of this process, in KiB, as the kernel reports it.
	long LockedKiB() {
		std::ifstream status("/proc/self/status");
		std::string line;
		while (std::getline(status, line))
			if (line.rfind("VmLck:", 0) == 0)
				return std::stol(line.substr(6));
		return -1;
	}
}

int test_secure_pages_are_locked_best_effort() {
	constexpr std::size_t block = 256 * 1024;
	rlimit limit{};
	const long before = LockedKiB();
	if (before < 0 || getrlimit(RLIMIT_MEMLOCK, &limit) != 0
		|| (limit.rlim_cur != RLIM_INFINITY && limit.rlim_cur < block * 2 + static_cast<rlim_t>(before) * 1024)) {
		std::cout << "  (skipped: RLIMIT_MEMLOCK too low to lock " << block / 1024 << " KiB)" << std::endl;
		RETURN_TEST(0);
	}
	{
		Safe::SecureBinary secret(ByteSize{block}, std::byte{0x5A});
		ASSERT_TRUE(LockedKiB() >= before + static_cast<long>(block / 1024));
		Safe::SecureBinary moved(std::move(secret));
		ASSERT_TRUE(LockedKiB() >= before + static_cast<long>(block / 1024));
		moved.push_back(std::byte{1});
		ASSERT_TRUE(LockedKiB() >= before + static_cast<long>(block / 1024));
		Safe::Binary plain(std::move(moved));
		ASSERT_TRUE(LockedKiB() < before + static_cast<long>(block / 1024));
	}
	ASSERT_EQUAL(before, LockedKiB());
	RETURN_TEST(0);
}
#endif

int test_secure_api_surface() {
	static_assert(Type::IsSafe<Safe::SecureBinary>::value);
	static_assert(Type::IsSafeValue<Safe::SecureBinary>::value);
	static_assert(std::is_base_of_v<Safe::Binary, Safe::SecureBinary>);
	static_assert(std::has_virtual_destructor_v<Safe::Binary>);
	static_assert(!std::is_constructible_v<std::vector<std::byte>, const Safe::SecureBinary&>);
	static_assert(!std::is_constructible_v<std::vector<std::byte>, Safe::SecureBinary&&>);
	static_assert(HasHexDump<Safe::Binary>);
	static_assert(!HasHexDump<Safe::SecureBinary>);
	static_assert(!HasHexDumpColumns<Safe::SecureBinary>);
	static_assert(std::is_convertible_v<const Safe::SecureBinary&, std::span<const std::byte>>);
	RETURN_TEST(0);
}

int test_secure_page_registry_reuse_and_threads() {
	for (unsigned cycle = 0; cycle < 100; ++cycle) {
		const Safe::SecureBinary secret(ByteSize{16}, std::byte{0x5A});
		ASSERT_EQUAL(std::byte{0x5A}, secret.front());
	}
	std::vector<std::thread> threads;
	for (unsigned worker = 0; worker < 4; ++worker)
		threads.emplace_back([] {
			for (unsigned cycle = 0; cycle < 100; ++cycle) {
				Safe::SecureBinary secret(ByteSize{8192}, std::byte{0xA5});
				secret.reserve(ByteSize{16384});
				secret.shrink_to_fit();
			}
		});
	for (auto& thread : threads)
		thread.join();
	StaticSecret = Safe::SecureBinary(ByteSize{32}, std::byte{0x42});
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Shared contract (Binary and SecureBinary)
	// -------------------
	result += test_shared_algorithms<Safe::Binary>();
	result += test_shared_algorithms<Safe::SecureBinary>();
	result += test_shared_construct<Safe::Binary>();
	result += test_shared_construct<Safe::SecureBinary>();
	result += test_shared_mutate<Safe::Binary>();
	result += test_shared_mutate<Safe::SecureBinary>();
	result += test_shared_self_aliasing<Safe::Binary>();
	result += test_shared_self_aliasing<Safe::SecureBinary>();
	result += test_shared_insert_iterators<Safe::Binary>();
	result += test_shared_insert_iterators<Safe::SecureBinary>();
	result += test_shared_comparisons<Safe::Binary>();
	result += test_shared_comparisons<Safe::SecureBinary>();

	// -------------------
	// Discard hook
	// -------------------
	result += test_hook_growth_discards_old_block();
	result += test_hook_shrinking_operations_discard_dropped_bytes();
	result += test_hook_reallocation_and_move_discard_whole_block();
	result += test_hook_every_acquired_block_is_released_once();

	// -------------------
	// Zeroing
	// -------------------
	result += test_secure_slack_is_zero_after_shrinking();
	result += test_secure_assign_and_insert_keep_slack_clean();
	result += test_secure_vector_rvalue_is_wiped();

	// -------------------
	// Semantics
	// -------------------
#ifdef LINUX
	result += test_secure_pages_are_locked_best_effort();
#endif
	result += test_secure_constructors_and_copies();
	result += test_secure_equality_is_symmetric_through_base();
	result += test_secure_cross_type_moves_hand_over_the_block();
	result += test_secure_insert_ranges_and_aliasing();
	result += test_secure_api_surface();
	result += test_secure_page_registry_reuse_and_threads();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
