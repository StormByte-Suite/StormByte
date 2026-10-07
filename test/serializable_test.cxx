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

#include <StormByte/byte_size.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/expected.hxx>
#include <StormByte/helpers.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/list.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/set.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/unordered_map.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/serializable.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <array>
#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace StormByte;

struct EmptyWireValue {
	~EmptyWireValue() {}
};

namespace StormByte::Detail {
	template<>
	struct Codec<EmptyWireValue> {
		static ByteSize Size(const EmptyWireValue&) noexcept { return ByteSize{0}; }
		static Safe::Binary Write(const EmptyWireValue&) noexcept { return {}; }
		static Expected<EmptyWireValue, DeserializeError> Read(std::span<const std::byte>) noexcept {
			return EmptyWireValue{};
		}
	};
}

namespace {
	enum class Level : std::uint8_t {
		Info,
		Warning
	};

	struct Tag {
		int a;
		std::string b;
		bool operator==(const Tag& other) const noexcept {
			return a == other.a && b == other.b;
		}
	};

	std::size_t ByteCount(const Safe::Binary& buf) {
		return static_cast<std::size_t>(buf.size());
	}

	void CorruptByte(Safe::Binary& buf, std::size_t index, std::byte value) {
		if (index < ByteCount(buf))
			buf[ByteSize{index}] = value;
	}

	void FlipBit(Safe::Binary& buf, std::size_t byte_index, unsigned bit) {
		if (byte_index >= ByteCount(buf) || bit > 7)
			return;
		auto& byte = reinterpret_cast<unsigned char&>(buf[ByteSize{byte_index}]);
		byte ^= static_cast<unsigned char>(1u << bit);
	}

	Safe::Binary Truncate(const Safe::Binary& buf, std::size_t new_size) {
		if (new_size >= ByteCount(buf))
			return buf;
		return Safe::Binary(buf.data(), ByteSize{new_size});
	}

	Safe::Binary MakeStringVectorBuffer() {
		const std::vector<std::string> data{"Hello", "StormByte", "World"};
		return Serializable<std::vector<std::string>>(data).Serialize();
	}

	Safe::Binary MakeStringBuffer() {
		return Serializable<std::string>(std::string{"StormByte serialization test"}).Serialize();
	}

	Safe::Binary MakeBinaryPayload() {
		return Safe::Binary{
			std::byte{0},
			std::byte{1},
			std::byte{255},
			std::byte{128}
		};
	}

	Safe::Binary CountOnly(std::uint64_t count) {
		return Serializable<std::uint64_t>(count).Serialize();
	}

	template<typename T>
	int RoundTrip(const T& original) {
		const auto buffer = Serializable<T>(original).Serialize();
		ASSERT_EQUAL(Serializable<T>::Size(original), ByteSize{ByteCount(buffer)});
		const auto decoded = Serializable<T>::Deserialize(buffer);
		ASSERT_TRUE(static_cast<bool>(decoded));
		ASSERT_EQUAL(original, decoded.value());
		const auto again = Serializable<T>(decoded.value()).Serialize();
		ASSERT_EQUAL(buffer, again);
		RETURN_TEST(0);
	}
}

template<>
struct StormByte::Detail::Codec<Tag> {
	static ByteSize Size(const Tag& value) noexcept {
		return Serializable<int>::Size(value.a) + Serializable<std::string>::Size(value.b);
	}

	static Safe::Binary Write(const Tag& value) noexcept {
		Safe::Binary buffer;
		append_bytes(buffer, Serializable<int>(value.a).Serialize());
		append_bytes(buffer, Serializable<std::string>(value.b).Serialize());
		return buffer;
	}

	static Expected<Tag, DeserializeError> Read(std::span<const std::byte> data) noexcept {
		auto expected_a = Serializable<int>::Deserialize(data);
		if (!expected_a)
			return Unexpected(expected_a.error());
		const std::size_t a_size = static_cast<std::size_t>(Serializable<int>::Size(expected_a.value()));
		if (a_size > data.size())
			return Unexpected<DeserializeError>("Insufficient data for Tag::b");
		auto expected_b = Serializable<std::string>::Deserialize(data.subspan(a_size));
		if (!expected_b)
			return Unexpected(expected_b.error());
		return Tag{expected_a.value(), expected_b.value()};
	}
};

// -------------------
// Binary
// -------------------

int test_binary_empty() {
	return RoundTrip(Safe::Binary{});
}

int test_binary_payload() {
	return RoundTrip(MakeBinaryPayload());
}

int test_binary_shares_vector_byte_wire() {
	const Safe::Binary owned = MakeBinaryPayload();
	std::vector<std::byte> as_vector(owned.begin(), owned.end());
	ASSERT_EQUAL(Serializable<Safe::Binary>(owned).Serialize(), Serializable<std::vector<std::byte>>(as_vector).Serialize());
	RETURN_TEST(0);
}

int test_binary_span_and_blob() {
	const Safe::Binary data = MakeBinaryPayload();
	const auto buffer = Serializable<Safe::Binary>(data).Serialize();
	const auto from_blob = Serializable<Safe::Binary>::Deserialize(buffer);
	const auto from_span = Serializable<Safe::Binary>::Deserialize(buffer.span());
	ASSERT_TRUE(static_cast<bool>(from_blob));
	ASSERT_TRUE(static_cast<bool>(from_span));
	ASSERT_EQUAL(data, from_blob.value());
	ASSERT_EQUAL(data, from_span.value());
	RETURN_TEST(0);
}

int test_binary_trailing_garbage() {
	auto dirty = Serializable<Safe::Binary>(MakeBinaryPayload()).Serialize();
	dirty.push_back(std::byte{0xDE});
	dirty.push_back(std::byte{0xAD});
	const auto result = Serializable<Safe::Binary>::Deserialize(dirty);
	ASSERT_TRUE(static_cast<bool>(result));
	ASSERT_EQUAL(MakeBinaryPayload(), result.value());
	RETURN_TEST(0);
}

// -------------------
// Codec
// -------------------

int test_codec_custom_type() {
	return RoundTrip(Tag{7, "codec"});
}

int test_codec_empty_wire() {
	const auto buffer = Serializable<EmptyWireValue>(EmptyWireValue{}).Serialize();
	ASSERT_EMPTY(buffer);
	const auto decoded = Serializable<EmptyWireValue>::Deserialize(std::span<const std::byte>{});
	ASSERT_TRUE(static_cast<bool>(decoded));
	RETURN_TEST(0);
}

// -------------------
// Container
// -------------------

int test_array_roundtrip() {
	const std::array<int, 3> original{1, 2, 3};
	return RoundTrip(original);
}

int test_map_roundtrip() {
	std::map<std::string, int> original;
	original.insert_or_assign("a", 1);
	original.insert_or_assign("b", 2);
	return RoundTrip(original);
}

int test_queue_roundtrip() {
	std::queue<int> original;
	original.push(4);
	original.push(5);
	const auto buffer = Serializable<std::queue<int>>(original).Serialize();
	const auto decoded = Serializable<std::queue<int>>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	auto got = decoded.value();
	ASSERT_EQUAL(std::size_t{2}, got.size());
	ASSERT_EQUAL(4, got.front());
	got.pop();
	ASSERT_EQUAL(5, got.front());
	RETURN_TEST(0);
}

int test_set_roundtrip() {
	return RoundTrip(std::set<int>{4, 1, 4, 2});
}

int test_vector_roundtrip() {
	return RoundTrip(std::vector<std::string>{"Hello", "StormByte", "World"});
}

// -------------------
// Corruption
// -------------------

int test_corruption_binary_huge_size() {
	auto buf = Serializable<Safe::Binary>(MakeBinaryPayload()).Serialize();
	const std::uint64_t huge = static_cast<std::uint64_t>(-1);
	std::memcpy(buf.data(), &huge, sizeof(huge));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Binary>::Deserialize(buf)));
	RETURN_TEST(0);
}

int test_corruption_binary_no_crash() {
	const auto clean = Serializable<Safe::Binary>(MakeBinaryPayload()).Serialize();
	for (std::size_t i = 0; i < ByteCount(clean); ++i) {
		for (unsigned bit = 0; bit < 8; ++bit) {
			auto buf = clean;
			FlipBit(buf, i, bit);
			(void)Serializable<Safe::Binary>::Deserialize(buf);
		}
		for (int value = 0; value < 256; value += 31) {
			auto buf = clean;
			CorruptByte(buf, i, static_cast<std::byte>(value));
			(void)Serializable<Safe::Binary>::Deserialize(buf);
		}
	}
	std::mt19937 rng(0xB10A);
	std::uniform_int_distribution<std::size_t> pos_dist(0, ByteCount(clean) - 1);
	std::uniform_int_distribution<int> val_dist(0, 255);
	for (int i = 0; i < 400; ++i) {
		auto buf = clean;
		for (int c = 0; c < 1 + (i % 4); ++c)
			CorruptByte(buf, pos_dist(rng), std::byte{static_cast<unsigned char>(val_dist(rng))});
		(void)Serializable<Safe::Binary>::Deserialize(buf);
	}
	RETURN_TEST(0);
}

int test_corruption_empty_buffer() {
	ASSERT_FALSE(static_cast<bool>(Serializable<int>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::string>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::vector<std::string>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::pair<int, double>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Binary>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::String>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::WString>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::List<int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Vector<int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Map<Safe::String, int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::UnorderedMap<Safe::String, int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Queue<int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Set<int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Optional<int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Pair<int, int>>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<Size>::Deserialize(Safe::Binary{})));
	ASSERT_FALSE(static_cast<bool>(Serializable<ByteSize>::Deserialize(Safe::Binary{})));
	RETURN_TEST(0);
}

int test_corruption_huge_container_and_string() {
	auto vector_buf = MakeStringVectorBuffer();
	const std::uint64_t huge = static_cast<std::uint64_t>(-1);
	std::memcpy(vector_buf.data(), &huge, sizeof(huge));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::vector<std::string>>::Deserialize(vector_buf)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Vector<Safe::String>>::Deserialize(vector_buf)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::List<Safe::String>>::Deserialize(vector_buf)));
	auto string_buf = MakeStringBuffer();
	std::memcpy(string_buf.data(), &huge, sizeof(huge));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::string>::Deserialize(string_buf)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::String>::Deserialize(string_buf)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::WString>::Deserialize(string_buf)));
	const auto claimed = CountOnly(huge);
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Map<Safe::String, int>>::Deserialize(claimed)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::UnorderedMap<Safe::String, int>>::Deserialize(claimed)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Set<int>>::Deserialize(claimed)));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::set<int>>::Deserialize(claimed)));
	RETURN_TEST(0);
}

int test_corruption_no_crash_text() {
	const auto clean = MakeStringVectorBuffer();
	for (std::size_t i = 0; i < ByteCount(clean); ++i) {
		for (unsigned bit = 0; bit < 8; ++bit) {
			auto buf = clean;
			FlipBit(buf, i, bit);
			(void)Serializable<std::vector<std::string>>::Deserialize(buf);
			(void)Serializable<Safe::Vector<Safe::String>>::Deserialize(buf);
			(void)Serializable<Safe::List<Safe::String>>::Deserialize(buf);
			(void)Serializable<Safe::Set<Safe::String>>::Deserialize(buf);
		}
	}
	const auto text = MakeStringBuffer();
	for (std::size_t i = 0; i < ByteCount(text); ++i) {
		for (int value = 0; value < 256; value += 31) {
			auto buf = text;
			CorruptByte(buf, i, static_cast<std::byte>(value));
			(void)Serializable<std::string>::Deserialize(buf);
			(void)Serializable<Safe::String>::Deserialize(buf);
		}
	}
	std::mt19937 rng(0xC0FFEE);
	std::uniform_int_distribution<std::size_t> pos_dist(0, ByteCount(clean) - 1);
	std::uniform_int_distribution<int> val_dist(0, 255);
	for (int i = 0; i < 400; ++i) {
		auto buf = clean;
		for (int c = 0; c < 1 + (i % 4); ++c)
			CorruptByte(buf, pos_dist(rng), static_cast<std::byte>(val_dist(rng)));
		(void)Serializable<std::vector<std::string>>::Deserialize(buf);
		(void)Serializable<Safe::Map<Safe::String, Safe::String>>::Deserialize(buf);
		(void)Serializable<Safe::UnorderedMap<Safe::String, Safe::String>>::Deserialize(buf);
		(void)Serializable<Safe::Set<Safe::String>>::Deserialize(buf);
	}
	auto doubled = clean;
	for (std::size_t i = 0; i < 4; ++i)
		doubled[ByteSize{i}] = std::byte{0xFF};
	for (std::size_t i = 0; i < 4; ++i)
		doubled[ByteSize{ByteCount(doubled) - 1 - i}] = std::byte{0xAA};
	(void)Serializable<std::vector<std::string>>::Deserialize(doubled);
	(void)Serializable<std::string>::Deserialize(clean);
	(void)Serializable<Safe::Queue<Safe::String>>::Deserialize(clean);
	(void)Serializable<Safe::Set<Safe::String>>::Deserialize(clean);
	RETURN_TEST(0);
}

int test_corruption_truncated() {
	const auto binary = Serializable<Safe::Binary>(MakeBinaryPayload()).Serialize();
	const auto text = MakeStringBuffer();
	const auto vector_buf = MakeStringVectorBuffer();
	const auto tag = Serializable<Tag>(Tag{7, "codec"}).Serialize();
	const Safe::String owned("StormByte serialization test");
	const auto safe_text = Serializable<Safe::String>(owned).Serialize();
	const Safe::List<int> numbers{1, 2, 3};
	const auto list_buf = Serializable<Safe::List<int>>(numbers).Serialize();
	Safe::Queue<int> fifo;
	fifo.push(9);
	fifo.push(8);
	const auto queue_buf = Serializable<Safe::Queue<int>>(fifo).Serialize();
	const Safe::Set<int> unique{1, 6, 7, 6};
	const auto set_buf = Serializable<Safe::Set<int>>(unique).Serialize();
	const Safe::Optional<int> present(7);
	const auto optional_buf = Serializable<Safe::Optional<int>>(present).Serialize();
	for (std::size_t len = 0; len < ByteCount(binary); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Binary>::Deserialize(Truncate(binary, len))));
	for (std::size_t len = 0; len < ByteCount(text); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<std::string>::Deserialize(Truncate(text, len))));
	for (std::size_t len = 0; len < ByteCount(safe_text); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Safe::String>::Deserialize(Truncate(safe_text, len))));
	for (std::size_t len = 0; len < ByteCount(vector_buf); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<std::vector<std::string>>::Deserialize(Truncate(vector_buf, len))));
	for (std::size_t len = 0; len < ByteCount(list_buf); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Safe::List<int>>::Deserialize(Truncate(list_buf, len))));
	for (std::size_t len = 0; len < ByteCount(queue_buf); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Queue<int>>::Deserialize(Truncate(queue_buf, len))));
	for (std::size_t len = 0; len < ByteCount(set_buf); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Set<int>>::Deserialize(Truncate(set_buf, len))));
	for (std::size_t len = 0; len < ByteCount(optional_buf); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Optional<int>>::Deserialize(Truncate(optional_buf, len))));
	for (std::size_t len = 0; len < ByteCount(tag); ++len)
		ASSERT_FALSE(static_cast<bool>(Serializable<Tag>::Deserialize(Truncate(tag, len))));
	RETURN_TEST(0);
}

// -------------------
// Optional
// -------------------

int test_optional_empty_and_filled() {
	const std::optional<Safe::Binary> empty;
	const auto empty_buf = Serializable<std::optional<Safe::Binary>>(empty).Serialize();
	const auto empty_got = Serializable<std::optional<Safe::Binary>>::Deserialize(empty_buf);
	ASSERT_TRUE(static_cast<bool>(empty_got));
	ASSERT_FALSE(empty_got.value().has_value());
	const std::optional<Safe::Binary> filled = MakeBinaryPayload();
	return RoundTrip(filled);
}

int test_optional_invalid_flag() {
	Safe::Binary buffer(ByteSize{1}, std::byte{2});
	ASSERT_FALSE(static_cast<bool>(Serializable<std::optional<int>>::Deserialize(buffer)));
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Optional<int>>::Deserialize(buffer)));
	RETURN_TEST(0);
}

// -------------------
// Pair
// -------------------

int test_pair_roundtrip() {
	return RoundTrip(std::pair<int, std::string>{42, "answer"});
}

// -------------------
// Safe
// -------------------

int test_safe_bytesize_roundtrip() {
	ASSERT_EQUAL(0, RoundTrip(ByteSize{0}));
	ASSERT_EQUAL(0, RoundTrip(ByteSize{1024}));
	ASSERT_EQUAL(0, RoundTrip(4 * MiB));
	const auto buffer = Serializable<ByteSize>(ByteSize{1}).Serialize();
	ASSERT_EQUAL(Serializable<std::uint64_t>(std::uint64_t{1}).Serialize(), buffer);
	RETURN_TEST(0);
}

int test_safe_list_matches_vector_wire() {
	const Safe::List<int> owned{1, 6, 7};
	const std::vector<int> standard{1, 6, 7};
	ASSERT_EQUAL(Serializable<std::vector<int>>(standard).Serialize(), Serializable<Safe::List<int>>(owned).Serialize());
	const auto decoded = Serializable<Safe::List<int>>::Deserialize(Serializable<Safe::List<int>>(owned).Serialize());
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{3}, decoded.value().size());
	ASSERT_EQUAL(1, decoded.value().front());
	ASSERT_EQUAL(7, decoded.value().back());
	RETURN_TEST(0);
}

int test_safe_list_roundtrip() {
	ASSERT_EQUAL(0, RoundTrip(Safe::List<int>{}));
	return RoundTrip(Safe::List<Safe::String>{Safe::String("one"), Safe::String("two")});
}

int test_safe_map_empty() {
	return RoundTrip(Safe::Map<Safe::String, int>{});
}

int test_safe_map_roundtrip() {
	Safe::Map<Safe::String, Safe::String> original;
	original.insert_or_assign(Safe::String("key"), Safe::String("value"));
	original.insert_or_assign(Safe::String("other"), Safe::String("row"));
	const auto buffer = Serializable<Safe::Map<Safe::String, Safe::String>>(original).Serialize();
	const auto decoded = Serializable<Safe::Map<Safe::String, Safe::String>>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{2}, decoded.value().size());
	ASSERT_EQUAL(std::string_view{"value"}, std::string_view{decoded.value().at(Safe::String("key"))});
	ASSERT_EQUAL(std::string_view{"row"}, std::string_view{decoded.value().at(Safe::String("other"))});
	RETURN_TEST(0);
}

int test_safe_optional_matches_std_wire() {
	using Optional = Safe::Optional<Safe::String>;
	static_assert(Type::Optional<Optional>);
	const Optional present(Safe::String("safe optional"));
	const auto present_buffer = Serializable<Optional>(present).Serialize();
	const auto standard_buffer = Serializable<std::optional<Safe::String>>(std::optional<Safe::String>(Safe::String("safe optional"))).Serialize();
	ASSERT_EQUAL(present_buffer, standard_buffer);
	const auto decoded = Serializable<Optional>::Deserialize(present_buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_TRUE(decoded.value().has_value());
	ASSERT_EQUAL(std::string_view{"safe optional"}, std::string_view{*decoded.value()});
	const Optional empty;
	const auto empty_buffer = Serializable<Optional>(empty).Serialize();
	const auto empty_decoded = Serializable<Optional>::Deserialize(empty_buffer);
	ASSERT_TRUE(static_cast<bool>(empty_decoded));
	ASSERT_FALSE(empty_decoded.value().has_value());
	RETURN_TEST(0);
}

int test_safe_pair_roundtrip() {
	using Pair = Safe::Pair<Safe::String, Safe::String>;
	const Pair original(Safe::String("key"), Safe::String("value"));
	const auto buffer = Serializable<Pair>(original).Serialize();
	const auto decoded = Serializable<Pair>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(original, decoded.value());
	const auto standard = Serializable<std::pair<Safe::String, Safe::String>>(std::pair<Safe::String, Safe::String>(original.first, original.second)).Serialize();
	ASSERT_EQUAL(standard, buffer);
	RETURN_TEST(0);
}

int test_safe_queue_fifo_and_wire() {
	Safe::Queue<int> original;
	original.push(9);
	original.push(8);
	std::queue<int> standard;
	standard.push(9);
	standard.push(8);
	const auto buffer = Serializable<Safe::Queue<int>>(original).Serialize();
	ASSERT_EQUAL(Serializable<std::queue<int>>(standard).Serialize(), buffer);
	const auto decoded = Serializable<Safe::Queue<int>>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	auto got = decoded.value();
	ASSERT_EQUAL(Size{2}, got.size());
	ASSERT_EQUAL(9, got.front());
	got.pop();
	ASSERT_EQUAL(8, got.front());
	RETURN_TEST(0);
}

int test_safe_queue_rejects_huge_count() {
	const auto claimed = CountOnly(static_cast<std::uint64_t>(1'048'576) + 1);
	ASSERT_FALSE(static_cast<bool>(Serializable<Safe::Queue<int>>::Deserialize(claimed)));
	ASSERT_FALSE(static_cast<bool>(Serializable<std::queue<int>>::Deserialize(claimed)));
	RETURN_TEST(0);
}

int test_safe_set_matches_std_wire() {
	const Safe::Set<int> owned{1, 6, 7, 6};
	const std::set<int> standard{1, 6, 7, 6};
	ASSERT_EQUAL(Serializable<std::set<int>>(standard).Serialize(), Serializable<Safe::Set<int>>(owned).Serialize());
	const auto decoded = Serializable<Safe::Set<int>>::Deserialize(Serializable<Safe::Set<int>>(owned).Serialize());
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{3}, decoded.value().size());
	ASSERT_TRUE(decoded.value().contains(1));
	ASSERT_TRUE(decoded.value().contains(6));
	ASSERT_TRUE(decoded.value().contains(7));
	RETURN_TEST(0);
}

int test_safe_set_roundtrip() {
	ASSERT_EQUAL(0, RoundTrip(Safe::Set<int>{}));
	return RoundTrip(Safe::Set<Safe::String>{Safe::String("one"), Safe::String("two"), Safe::String("one")});
}

int test_safe_size_roundtrip() {
	ASSERT_EQUAL(0, RoundTrip(Size{0}));
	ASSERT_EQUAL(0, RoundTrip(Size{42}));
	const auto buffer = Serializable<Size>(Size{7}).Serialize();
	ASSERT_EQUAL(Serializable<std::uint64_t>(std::uint64_t{7}).Serialize(), buffer);
	RETURN_TEST(0);
}

int test_safe_string_embedded_nul() {
	const char raw[] = {'A', '\0', 'B'};
	const Safe::String original(std::string_view{raw, 3});
	const auto buffer = Serializable<Safe::String>(original).Serialize();
	const auto decoded = Serializable<Safe::String>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{3}, decoded.value().size());
	ASSERT_EQUAL('\0', decoded.value()[Size{1}]);
	ASSERT_EQUAL(Serializable<std::string>(std::string(raw, 3)).Serialize(), buffer);
	RETURN_TEST(0);
}

int test_safe_string_null_and_empty() {
	const Safe::String null;
	const Safe::String empty("");
	const auto null_buffer = Serializable<Safe::String>(null).Serialize();
	const auto empty_buffer = Serializable<Safe::String>(empty).Serialize();
	ASSERT_EQUAL(empty_buffer, null_buffer);
	ASSERT_EQUAL(Serializable<std::string>(std::string{}).Serialize(), null_buffer);
	const auto decoded = Serializable<Safe::String>::Deserialize(null_buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_FALSE(static_cast<bool>(decoded.value()));
	ASSERT_TRUE(decoded.value().empty());
	ASSERT_EQUAL(Size{0}, decoded.value().size());
	RETURN_TEST(0);
}

int test_safe_unordered_map_roundtrip() {
	Safe::UnorderedMap<Safe::String, int> original;
	original.insert_or_assign(Safe::String("a"), 1);
	original.insert_or_assign(Safe::String("b"), 2);
	const auto buffer = Serializable<Safe::UnorderedMap<Safe::String, int>>(original).Serialize();
	const auto decoded = Serializable<Safe::UnorderedMap<Safe::String, int>>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{2}, decoded.value().size());
	ASSERT_EQUAL(1, decoded.value().at(Safe::String("a")));
	ASSERT_EQUAL(2, decoded.value().at(Safe::String("b")));
	const auto again = Serializable<Safe::UnorderedMap<Safe::String, int>>(decoded.value()).Serialize();
	const auto reread = Serializable<Safe::UnorderedMap<Safe::String, int>>::Deserialize(again);
	ASSERT_TRUE(static_cast<bool>(reread));
	ASSERT_EQUAL(1, reread.value().at(Safe::String("a")));
	ASSERT_EQUAL(2, reread.value().at(Safe::String("b")));
	const Safe::UnorderedMap<Safe::String, int> empty;
	const auto empty_decoded = Serializable<Safe::UnorderedMap<Safe::String, int>>::Deserialize(Serializable<Safe::UnorderedMap<Safe::String, int>>(empty).Serialize());
	ASSERT_TRUE(static_cast<bool>(empty_decoded));
	ASSERT_EQUAL(Size{0}, empty_decoded.value().size());
	RETURN_TEST(0);
}

int test_safe_vector_empty() {
	return RoundTrip(Safe::Vector<int>{});
}

int test_safe_vector_roundtrip() {
	Safe::Vector<Safe::String> original;
	original.push_back(Safe::String("one"));
	original.push_back(Safe::String("two"));
	const auto buffer = Serializable<Safe::Vector<Safe::String>>(original).Serialize();
	const std::vector<Safe::String> standard{Safe::String("one"), Safe::String("two")};
	ASSERT_EQUAL(Serializable<std::vector<Safe::String>>(standard).Serialize(), buffer);
	const auto decoded = Serializable<Safe::Vector<Safe::String>>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{2}, decoded.value().size());
	ASSERT_EQUAL(std::string_view{"one"}, std::string_view{decoded.value()[Size{0}]});
	ASSERT_EQUAL(std::string_view{"two"}, std::string_view{decoded.value()[Size{1}]});
	RETURN_TEST(0);
}

int test_safe_wstring_embedded_nul() {
	const wchar_t raw[] = {L'A', L'\0', L'B'};
	const Safe::WString original(std::wstring_view{raw, 3});
	const auto buffer = Serializable<Safe::WString>(original).Serialize();
	const auto decoded = Serializable<Safe::WString>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_EQUAL(Size{3}, decoded.value().size());
	ASSERT_EQUAL(L'\0', decoded.value()[Size{1}]);
	ASSERT_EQUAL(Serializable<std::wstring>(std::wstring(raw, 3)).Serialize(), buffer);
	RETURN_TEST(0);
}

int test_safe_wstring_roundtrip() {
	const Safe::WString original(L"ancho");
	const auto buffer = Serializable<Safe::WString>(original).Serialize();
	const auto decoded = Serializable<Safe::WString>::Deserialize(buffer);
	ASSERT_TRUE(static_cast<bool>(decoded));
	ASSERT_TRUE(decoded.value() == L"ancho");
	const Safe::WString empty(L"");
	const auto empty_buffer = Serializable<Safe::WString>(empty).Serialize();
	ASSERT_EQUAL(Serializable<Safe::WString>(Safe::WString{}).Serialize(), empty_buffer);
	const auto empty_decoded = Serializable<Safe::WString>::Deserialize(empty_buffer);
	ASSERT_TRUE(static_cast<bool>(empty_decoded));
	ASSERT_FALSE(static_cast<bool>(empty_decoded.value()));
	ASSERT_TRUE(empty_decoded.value().empty());
	ASSERT_EQUAL(Size{0}, empty_decoded.value().size());
	RETURN_TEST(0);
}

// -------------------
// Text
// -------------------

int test_text_std_strings() {
	ASSERT_EQUAL(0, RoundTrip(std::string{"StormByte"}));
	ASSERT_EQUAL(0, RoundTrip(std::wstring{L"StormByte"}));
	ASSERT_EQUAL(0, RoundTrip(std::u16string{u"StormByte"}));
	ASSERT_EQUAL(0, RoundTrip(std::u32string{U"StormByte"}));
	ASSERT_EQUAL(0, RoundTrip(std::string{}));
	RETURN_TEST(0);
}

// -------------------
// Trivial
// -------------------

int test_trivial_bool_and_invalid() {
	ASSERT_EQUAL(0, RoundTrip(true));
	ASSERT_EQUAL(0, RoundTrip(false));
	Safe::Binary bad(ByteSize{1}, std::byte{2});
	ASSERT_FALSE(static_cast<bool>(Serializable<bool>::Deserialize(bad)));
	RETURN_TEST(0);
}

int test_trivial_endian_and_widths() {
	const auto one = Serializable<std::uint32_t>(std::uint32_t{1}).Serialize();
	ASSERT_EQUAL(ByteSize{4}, ByteSize{ByteCount(one)});
	ASSERT_EQUAL(std::byte{1}, one[ByteSize{0}]);
	ASSERT_EQUAL(std::byte{0}, one[ByteSize{3}]);
	ASSERT_EQUAL(0, RoundTrip(std::uint8_t{255}));
	ASSERT_EQUAL(0, RoundTrip(std::uint16_t{0xBEEF}));
	ASSERT_EQUAL(0, RoundTrip(std::int64_t{-7}));
	ASSERT_EQUAL(0, RoundTrip(3.5));
	ASSERT_EQUAL(0, RoundTrip(1.25f));
	ASSERT_EQUAL(0, RoundTrip(Level::Warning));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Binary
	// -------------------
	result += test_binary_empty();
	result += test_binary_payload();
	result += test_binary_shares_vector_byte_wire();
	result += test_binary_span_and_blob();
	result += test_binary_trailing_garbage();

	// -------------------
	// Codec
	// -------------------
	result += test_codec_custom_type();
	result += test_codec_empty_wire();

	// -------------------
	// Container
	// -------------------
	result += test_array_roundtrip();
	result += test_map_roundtrip();
	result += test_queue_roundtrip();
	result += test_set_roundtrip();
	result += test_vector_roundtrip();

	// -------------------
	// Corruption
	// -------------------
	result += test_corruption_binary_huge_size();
	result += test_corruption_binary_no_crash();
	result += test_corruption_empty_buffer();
	result += test_corruption_huge_container_and_string();
	result += test_corruption_no_crash_text();
	result += test_corruption_truncated();

	// -------------------
	// Optional
	// -------------------
	result += test_optional_empty_and_filled();
	result += test_optional_invalid_flag();

	// -------------------
	// Pair
	// -------------------
	result += test_pair_roundtrip();

	// -------------------
	// Safe
	// -------------------
	result += test_safe_bytesize_roundtrip();
	result += test_safe_list_matches_vector_wire();
	result += test_safe_list_roundtrip();
	result += test_safe_map_empty();
	result += test_safe_map_roundtrip();
	result += test_safe_optional_matches_std_wire();
	result += test_safe_pair_roundtrip();
	result += test_safe_queue_fifo_and_wire();
	result += test_safe_queue_rejects_huge_count();
	result += test_safe_set_matches_std_wire();
	result += test_safe_set_roundtrip();
	result += test_safe_size_roundtrip();
	result += test_safe_string_embedded_nul();
	result += test_safe_string_null_and_empty();
	result += test_safe_unordered_map_roundtrip();
	result += test_safe_vector_empty();
	result += test_safe_vector_roundtrip();
	result += test_safe_wstring_embedded_nul();
	result += test_safe_wstring_roundtrip();

	// -------------------
	// Text
	// -------------------
	result += test_text_std_strings();

	// -------------------
	// Trivial
	// -------------------
	result += test_trivial_bool_and_invalid();
	result += test_trivial_endian_and_widths();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
